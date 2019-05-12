CREATE SCHEMA dbo;


CREATE TABLE dbo.auction_ents(
	ent_id int NOT NULL,
	shard_name varchar(50) NOT NULL,
	data text NULL,
	updated timestamp NOT NULL DEFAULT('1970-01-01 00:00:00'),
  CONSTRAINT PK_ents PRIMARY KEY (ent_id, shard_name)
);

CREATE TABLE dbo.history(
	identifier varchar(255) PRIMARY KEY NOT NULL,
	data text NULL
);

CREATE TABLE dbo.parse_versions(
	shards_crc int NULL,
	ents_crc int NULL,
	history_crc int NULL
);

CREATE TABLE dbo.shards(
	data text NOT NULL
);


INSERT INTO dbo.parse_versions (shards_crc, ents_crc, history_crc) VALUES (0,0,0);


CREATE INDEX IX_ents ON dbo.auction_ents (ent_id);
CREATE INDEX IX_ents_1 ON dbo.auction_ents (shard_name); 


CREATE OR REPLACE PROCEDURE dbo.SP_delete_ent(
  IN ent_id integer,
  IN shard_name text
)
LANGUAGE 'sql'
AS $$
  DELETE FROM dbo.auction_ents WHERE ent_id = SP_delete_ent.ent_id AND shard_name = SP_delete_ent.shard_name;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_insert_or_update_ent(
  IN ent_id integer,
  IN shard_name text,
  IN data text
)
LANGUAGE 'sql'
AS $$
  INSERT INTO dbo.auction_ents (ent_id, shard_name, data, updated) 
  VALUES (SP_insert_or_update_ent.ent_id, SP_insert_or_update_ent.shard_name, SP_insert_or_update_ent.data, now())
  ON CONFLICT (ent_id, shard_name)
  DO UPDATE SET data = SP_insert_or_update_ent.data, updated = now();
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_insert_or_update_history(
  IN identifier text,
  IN data text
)
LANGUAGE 'sql'
AS $$
  INSERT INTO dbo.history (identifier, data) 
  VALUES (SP_insert_or_update_history.identifier, SP_insert_or_update_history.data)
  ON CONFLICT (identifier)
  DO UPDATE SET data = SP_insert_or_update_history.data;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_insert_shard(
  IN data text
)
LANGUAGE 'sql'
AS $$
  SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
	BEGIN TRANSACTION;
    INSERT INTO dbo.shards (data) VALUES (SP_insert_shard.data);
  COMMIT;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_write_ents_crc(
	IN crc integer
)
LANGUAGE 'sql'
AS $$
	UPDATE dbo.parse_versions set ents_crc = crc;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_write_history_crc(
	IN crc integer
)
LANGUAGE 'sql'
AS $$
	UPDATE dbo.parse_versions set history_crc = crc;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_write_shards_crc(
	IN crc integer
)
LANGUAGE 'sql'
AS $$
	UPDATE dbo.parse_versions set shards_crc = crc;
$$;