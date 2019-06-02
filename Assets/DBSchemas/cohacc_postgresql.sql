CREATE SCHEMA dbo;

CREATE TABLE dbo.account(
	auth_id int NOT NULL PRIMARY KEY,
	name varchar(14) NULL DEFAULT(''),
	loyalty_bits bytea NULL DEFAULT('0'),
	last_loyalty_point_count smallint NULL DEFAULT(0),
	loyalty_points_spent smallint NULL DEFAULT(0),
	last_email_date timestamp NULL DEFAULT('2000-01-01 00:00:00'),
	last_num_emails_sent smallint NULL DEFAULT(0),
	free_xfer_date timestamp NULL DEFAULT('2000-01-01 00:00:00'),
  CONSTRAINT c_loyalty_bits_length CHECK (length(loyalty_bits) <= 16)
);

CREATE TABLE dbo.product_type(
	product_type_id int NOT NULL PRIMARY KEY,
	name varchar(128) NULL
);

CREATE TABLE dbo.product(
	sku_id char(8) NOT NULL PRIMARY KEY,
	name varchar(128) NULL,
	product_type_id int NULL,
	grant_limit int NULL,
	expiration_seconds int NULL,
  CONSTRAINT FK_product_product_type FOREIGN KEY (product_type_id) REFERENCES dbo.product_type(product_type_id)
);

CREATE TABLE dbo.game_log(
	order_id uuid NOT NULL PRIMARY KEY,
	auth_id int NULL,
	sku_id char(8) NULL,
	transaction_date timestamp NULL,
	shard_id smallint NULL,
	ent_id int NULL,
	granted int NULL,
	claimed int NULL,
	csr_did_it boolean NULL,
	parent_order_id uuid NULL,
	saved int NULL,
  CONSTRAINT FK_game_log_account FOREIGN KEY (auth_id) REFERENCES dbo.account(auth_id),
  CONSTRAINT FK_game_log_product FOREIGN KEY (sku_id) REFERENCES dbo.product (sku_id)
);

CREATE TABLE dbo.inventory(
	auth_id int NOT NULL,
	sku_id char(8) NOT NULL,
	granted_total int NULL,
	claimed_total int NULL,
	saved_total int NULL DEFAULT(0),
	expires timestamp NULL,
  CONSTRAINT PK_inventory PRIMARY KEY (auth_id, sku_id),
  CONSTRAINT FK_inventory_account FOREIGN KEY (auth_id) REFERENCES dbo.account (auth_id),
  CONSTRAINT FK_inventory_product FOREIGN KEY (sku_id) REFERENCES dbo.product (sku_id)
);

CREATE TABLE dbo.mtx_log(
	order_id uuid NOT NULL PRIMARY KEY,
	auth_id int NULL,
	sku_id char(8) NULL,
	transaction_date timestamp NULL,
	quantity int NULL,
	points int NULL,
  CONSTRAINT FK_mtx_log_account FOREIGN KEY (auth_id) REFERENCES dbo.account (auth_id),
  CONSTRAINT FK_mtx_log_product FOREIGN KEY (sku_id) REFERENCES dbo.product (sku_id)
);

CREATE TYPE dbo.TVP_game_transaction AS(
	order_id uuid,
	auth_id int,
	sku_id char(8),
	transaction_date timestamp,
	shard_id smallint,
	ent_id int,
	granted int,
	claimed int,
	csr_did_it smallint
);

CREATE TYPE dbo.TVP_product AS (
  sku_id char(8), 
  name varchar(128), 
  product_type_id integer, 
  grant_limit integer, 
  expiration_seconds integer
);

CREATE TYPE dbo.TVP_product_type AS (
  product_type_id integer,
  name varchar(128)
);

CREATE OR REPLACE PROCEDURE dbo.merge_products_from_bins(
	IN bin_products dbo.TVP_product
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
	INSERT INTO dbo.product SELECT * FROM bin_products
	ON CONFLICT (sku_id)
	DO UPDATE SET name = bin_products.name, product_type_id = bin_products.product_type_id, 
		grant_limit = bin_products.grant_limit, expiration_seconds = bin_products.expiration_seconds;

	DELETE FROM dbo.product
  WHERE NOT EXISTS (SELECT 1 FROM bin_products bins WHERE bins.sku_id = product.sku_id);
END;
$$;

CREATE OR REPLACE PROCEDURE dbo.merge_product_types_from_bins(
  IN product_type_ids integer[],
  IN names text[]
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
  CREATE TEMPORARY TABLE tmp_product_type (product_type_id integer, name varchar(128));
  
  INSERT INTO tmp_product_type (product_type_id, name)
    SELECT * FROM unnest(product_type_ids, names);
  
  DELETE FROM dbo.product_type
  WHERE NOT EXISTS (SELECT 1 FROM tmp_product_type tmp WHERE tmp.product_type_id = product_type.product_type_id);
END;
$$;

CREATE OR REPLACE PROCEDURE dbo.merge_product_types()
LANGUAGE 'plpgsql'
AS $$
BEGIN
	INSERT INTO dbo.product_type SELECT * FROM tmp_product_type
  ON CONFLICT (product_type_id)
  DO UPDATE SET name = excluded.name;
  
  DELETE FROM dbo.product_type
  WHERE NOT EXISTS (SELECT 1 FROM tmp_product_type WHERE tmp_product_type.product_type_id = product_type.product_type_id);
	
	DROP TABLE tmp_product_type;
END;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_add_game_transaction(
	IN order_id uuid,
	IN auth_id int,
	IN sku_id char(8),
	IN transaction_date timestamp,
	IN csr_did_it boolean,
	IN shard_id smallint DEFAULT NULL,
	IN ent_id int DEFAULT NULL,
	IN granted int DEFAULT NULL,
	IN claimed int DEFAULT NULL,
	IN parent_order_id uuid DEFAULT NULL
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
	IF NOT EXISTS(
		SELECT 1 FROM dbo.game_log WHERE order_id = SP_add_game_transaction.order_id
	) THEN
		SP_add_game_transaction.granted = COALESCE(SP_add_game_transaction.granted, 0);
		SP_add_game_transaction.claimed = COALESCE(SP_add_game_transaction.claimed, 0);

		INSERT INTO dbo.game_log (order_id, auth_id, sku_id, transaction_date, csr_did_it, shard_id, ent_id, granted, claimed, parent_order_id)
		VALUES (SP_add_game_transaction.order_id, SP_add_game_transaction.auth_id, SP_add_game_transaction.sku_id, 
				SP_add_game_transaction.transaction_date, SP_add_game_transaction.csr_did_it, SP_add_game_transaction.shard_id, 
				SP_add_game_transaction.ent_id, SP_add_game_transaction.granted, SP_add_game_transaction.claimed, SP_add_game_transaction.parent_order_id);
	
		INSERT INTO dbo.inventory (auth_id, sku_id, granted_total, claimed_total)
		VALUES (SP_add_game_transaction.auth_id, SP_add_game_transaction.sku_id, 
				SP_add_game_transaction.granted, SP_add_game_transaction.claimed)
		ON CONFLICT (auth_id, sku_id)
		DO UPDATE SET
			granted_total = granted_total + SP_add_game_transaction.granted,
			claimed_total = claimed_total + SP_add_game_transaction.claimed,
			expires = NULL;
	END IF;

	SELECT sku_id, granted_total, claimed_total, saved_total, expires FROM dbo.inventory 
	WHERE auth_id = SP_add_game_transaction.auth_id AND sku_id = SP_add_game_transaction.sku_id;
END
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_add_micro_transaction(
  IN order_id uuid,
  IN auth_id integer,
  IN sku_id char(8),
  IN transaction_date timestamp,
  IN quantity integer,
  IN points integer
)
LANGUAGE 'plpgsql'
AS $$
DECLARE
	product_expires_in_seconds integer;
	product_expires timestamp;
BEGIN
	IF NOT EXISTS(
		SELECT 1 FROM dbo.mtx_log WHERE order_id = SP_add_micro_transaction.order_id
	) THEN  
    INSERT INTO dbo.mtx_log (order_id, auth_id, sku_id, transaction_date, quantity, points)
    VALUES (SP_add_micro_transaction.order_id, SP_add_micro_transaction.auth_id, 
    SP_add_micro_transaction.sku_id, SP_add_micro_transaction.transaction_date, 
    SP_add_micro_transaction.quantity, SP_add_micro_transaction.points);
		
    SELECT product_expires_in_seconds = expiration_seconds FROM dbo.product WHERE sku_id = SP_add_micro_transaction.sku_id;
		product_expires = NOW() + ((product_expires_in_seconds * SP_add_game_transaction.quantity) || ' seconds')::interval;
		INSERT INTO dbo.inventory (auth_id, sku_id, granted_total, claimed_total, expires)
		VALUES (SP_add_game_transaction.auth_id, SP_add_game_transaction.sku_id, 
				SP_add_game_transaction.quantity, 0, product_expires)
		ON CONFLICT (auth_id, sku_id)
		DO UPDATE SET
			granted_total = granted_total + SP_add_game_transaction.quantity,
			expires = product_expires;
  END IF;
END;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_add_multi_game_transaction(
	IN game_transactions dbo.TVP_game_transaction,
	IN parent_order_id uuid
)
LANGUAGE 'plpgsql'
AS $$
DECLARE
	csr refcursor;
	f_order_id uuid;
	f_auth_id integer;
	f_sku_id CHAR(8);
	f_transaction_date timestamp;
	f_shard_id smallint;
	f_ent_id integer;
	f_granted integer;
	f_claimed integer;
	f_csr_did_it smallint;
BEGIN
	IF NOT EXISTS(
		SELECT 1 FROM dbo.game_log WHERE parent_order_id = SP_add_multi_game_transaction.parent_order_id
	) THEN
		OPEN csr FOR SELECT order_id, auth_id, sku_id, transaction_date, shard_id, ent_id, granted, claimed, csr_did_it 
			FROM SP_add_multi_game_transaction.game_transactions; 
		
		LOOP
			FETCH NEXT FROM csr INTO f_order_id, f_auth_id, f_sku_id, f_transaction_date, f_shard_id, 
				f_ent_id, f_granted, f_claimed, f_csr_did_it;
			EXIT WHEN NOT FOUND;
			SELECT dbo.SP_add_game_transaction(f_order_id, f_auth_id, f_sku_id, f_transaction_date, f_csr_did_it, f_shard_id, 
						f_ent_id, f_granted, f_claimed, SP_add_multi_game_transaction.parent_order_id);
		END LOOP;
	END IF;	
END;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_find_or_create_account(
	IN auth_id integer,
	INOUT name varchar(14),
	INOUT loyalty_bits bytea,
	INOUT last_loyalty_point_count smallint,
	INOUT loyalty_points_spent smallint,
	INOUT last_email_date timestamp,
	INOUT last_num_emails_sent smallint,
	INOUT free_xfer_date timestamp
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
	INSERT INTO dbo.account (auth_id, name) 
	VALUES (SP_find_or_create_account.auth_id, SP_find_or_create_account.name)
	ON CONFLICT (auth_id) 
	DO UPDATE SET
		name = COALESCE(SP_find_or_create_account.name, name);
	SELECT
		SP_find_or_create_account.name = name,
		SP_find_or_create_account.loyalty_bits = loyalty_bits,
		SP_find_or_create_account.last_loyalty_point_count = last_loyalty_point_count,
		SP_find_or_create_account.loyalty_points_spent = loyalty_points_spent,
		SP_find_or_create_account.last_email_date = last_email_date,
		SP_find_or_create_account.last_num_emails_sent = last_num_emails_sent,
		SP_find_or_create_account.free_xfer_date = free_xfer_date
	FROM dbo.account WHERE auth_id = SP_find_or_create_account.auth_id;
END;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_read_unsaved_game_transactions(
   IN auth_id int,
   IN shard_id smallint = NULL,
   IN ent_id int = NULL
)
LANGUAGE 'sql'
AS $$
    SELECT order_id, auth_id, sku_id, transaction_date, shard_id, ent_id, granted, claimed, csr_did_it
	FROM dbo.game_log
	WHERE auth_id = SP_read_unsaved_game_transactions.auth_id 
		AND claimed IS NOT NULL AND saved = 0 AND parent_order_id IS NULL
		AND (shard_id = SP_read_unsaved_game_transactions.shard_id OR SP_read_unsaved_game_transactions.shard_id IS NULL)
		AND (ent_id = SP_read_unsaved_game_transactions.ent_id OR SP_read_unsaved_game_transactions.ent_id IS NULL);
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_revert_game_transaction(
	IN auth_id integer,
	IN search_id uuid
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
	CREATE TEMPORARY TABLE tmp_revert ON COMMIT DROP AS
		SELECT game_log.order_id, game_log.sku_id, game_log.granted, game_log.claimed, product.expiration_seconds 
		FROM dbo.game_log 
		INNER JOIN dbo.product ON (game_log.sku_id = product.sku_id)
		WHERE (game_log.order_id = SP_revert_game_transaction.search_id 
			   OR game_log.parent_order_id = SP_revert_game_transaction.search_id) 
		AND game_log.auth_id = SP_revert_game_transaction.auth_id 
		AND game_log.saved = 0;
	
	DELETE FROM dbo.game_log gl
	USING tmp_revert tmp 
	WHERE gl.order_id = tmp.order_id;
	
	UPDATE dbo.inventory inv
	SET	granted_total = granted_total - COALESCE(tmp.granted, 0),
		claimed_total = claimed_total - COALESCE(tmp.claimed, 0),
		expires = expires - ((tmp.expiration_seconds * tmp.granted) | 'seconds')::interval
	FROM dbo.inventory
	INNER JOIN tmp_revert tmp
	ON inv.sku_id = tmp.sku_id
	WHERE inv.auth_id = SP_revert_game_transaction.auth_id;	
END;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_save_game_transaction(
	IN auth_id integer,
	IN search_id uuid
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
	CREATE TEMPORARY TABLE tmp_save ON COMMIT DROP AS
		SELECT order_id, sku_id, claimed FROM dbo.game_log 
		WHERE (order_id = SP_save_game_transaction.search_id OR parent_order_id = SP_save_game_transaction.search_id) 
			AND auth_id = SP_save_game_transaction.auth_id AND saved = 0;
			
	UPDATE dbo.game_log game SET saved = 1 
	FROM dbo.game_log RIGHT JOIN tmp_save tmp ON game.order_id = tmp.order_id;
	
	UPDATE dbo.inventory inv SET saved_total = saved_total + tmp.claimed 
	FROM dbo.inventory INNER JOIN tmp_save tmp ON inv.sku_id = tmp.sku_id
	WHERE inv.auth_id = SP_save_game_transaction.auth_id
	AND tmp.claimed > 0;	
END;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_update_account(
	IN auth_id integer,
	IN loyalty_bits bytea,
	IN last_loyalty_point_count smallint,
	IN loyalty_points_spent smallint,
	IN last_email_date timestamp,
	IN last_num_emails_sent smallint,
	IN free_xfer_date timestamp
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
	UPDATE dbo.account SET
		loyalty_bits = SP_update_account.loyalty_bits,
		last_loyalty_point_count = SP_update_account.last_loyalty_point_count,
		loyalty_points_spent = SP_update_account.loyalty_points_spent,
		last_email_date = SP_update_account.last_email_date,
		last_num_emails_sent = SP_update_account.last_num_emails_sent,
		free_xfer_date = SP_update_account.free_xfer_date
		WHERE auth_id = SP_update_account.auth_id;
		
	IF NOT FOUND THEN RAISE NOTICE 'No such account %', auth_id; END IF;
END;
$$;