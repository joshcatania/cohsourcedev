CREATE SCHEMA IF NOT EXISTS dbo;


CREATE TABLE dbo.block_msg(
	uid int NOT NULL PRIMARY KEY,
	reason int NOT NULL DEFAULT(0),
	msg varchar(255) NULL
);

CREATE TABLE dbo.gm_illegal_login(
	account varchar(32) NULL,
	ip varchar(16) NULL
);

CREATE TABLE dbo.server(
	id int NOT NULL PRIMARY KEY,
	name text NULL,
	ip varchar(16) NULL,
	inner_ip varchar(16) NULL,
	ageLimit int NULL,
	pk_flag int NULL,
	server_group_id int NULL
);

CREATE TABLE dbo.user_account(
	account varchar(14) NOT NULL PRIMARY KEY,
	uid int NULL,
	pay_stat int NOT NULL DEFAULT(0),
	login_flag int NOT NULL DEFAULT(0),
	warn_flag int NOT NULL DEFAULT(0),
	block_flag int NOT NULL DEFAULT(0),
	block_flag2 int NOT NULL DEFAULT(0),
	last_login timestamp with time zone NULL,
	last_logout timestamp with time zone NULL,
	subscription_flag int NOT NULL DEFAULT(0),
	last_world smallint NULL DEFAULT(0),
	last_game int NULL,
	last_ip varchar(15) NULL,
	block_end_date timestamp with time zone NULL,
	queue_level int NOT NULL DEFAULT(0),
	product_id int NULL,
	loyalty_points int NULL DEFAULT(0),
	loyalty_legacy_points int NULL DEFAULT(0),
	forum_id int NULL
);

CREATE TABLE dbo.user_auth(
	account varchar(14) NOT NULL PRIMARY KEY,
	password bytea NOT NULL CHECK (length(password) = 128),
	salt int NOT NULL DEFAULT(0),
	hash_type smallint NOT NULL DEFAULT(0)
);

CREATE TABLE dbo.user_count(
	record_time timestamp with time zone NOT NULL DEFAULT(now()),
	server_id smallint NOT NULL,
	world_user int NOT NULL,
	limit_user int NOT NULL,
	auth_user int NOT NULL,
	wait_user int NOT NULL,
	dayofweek smallint NOT NULL DEFAULT(date_part('dow', now())),
	product_id int NULL
);

CREATE TABLE dbo.user_data(
	uid int NOT NULL PRIMARY KEY,
	user_data bytea NULL DEFAULT('') CHECK (length(user_data) <= 16),
	user_data_new bytea NULL DEFAULT('') CHECK (length(user_data_new) <= 112),
	user_game_data bytea NOT NULL DEFAULT('') CHECK (length(user_game_data) <= 16),
	user_game_data_new bytea NOT NULL DEFAULT('') CHECK (length(user_game_data_new) <= 112)
);

CREATE TABLE dbo.user_info(
	account varchar(14) NOT NULL PRIMARY KEY,
	ssn varchar(11) NULL
);

CREATE TABLE dbo.user_server_group(
	uid int NOT NULL,
	server_group_id int NOT NULL,
  CONSTRAINT PK_user_server_groups PRIMARY KEY (uid, server_group_id)
);

CREATE TABLE dbo.worldstatus(
	idx int NOT NULL PRIMARY KEY,
	status int NULL
);


CREATE INDEX IX_user_account ON dbo.user_account (uid);


CREATE OR REPLACE PROCEDURE dbo.ap_GPwd(
  IN name text,
  INOUT password bytea,
  INOUT hash_type integer,
  INOUT salt smallint
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
  SELECT ap_GPwd.password = password, ap_GPwd.salt = salt, ap_GPwd.hash_type = hash_type 
  FROM dbo.user_auth WHERE account = ap_GPwd.name;
END;
$$;


CREATE OR REPLACE PROCEDURE dbo.ap_GStat(
  IN account text,
  INOUT uid integer,
  INOUT pay_stat integer,
  INOUT login_flag integer,
  INOUT warn_flag integer,
  INOUT block_flag integer,
  INOUT block_flag2 integer,
  INOUT subscription_flag integer,
  INOUT lastworld smallint,
  INOUT block_end_date timestamp with time zone,
  INOUT queueLevel integer,
  INOUT loyalty integer,
  INOUT loyaltylegacy integer
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
	SELECT ap_GStat.uid = uid,
	  ap_GStat.pay_stat = pay_stat,
	  ap_GStat.login_flag = login_flag,
	  ap_GStat.warn_flag = warn_flag,
	  ap_GStat.block_flag = block_flag,
	  ap_GStat.block_flag2 = block_flag2,
	  ap_GStat.subscription_flag = subscription_flag,
	  ap_GStat.lastworld = last_world,
	  ap_GStat.block_end_date = block_end_date,
	  ap_GStat.queuelevel = queue_level,
	  ap_GStat.loyalty = loyalty_points,
	  ap_GStat.loyaltylegacy = loyalty_legacy_points
	FROM dbo.user_account
	WHERE account = ap_GStat.account;
END;
$$;

CREATE OR REPLACE PROCEDURE dbo.ap_SLog(
  IN uid integer,
  IN login timestamp without time zone,
  IN logout timestamp without time zone,
  IN game_id integer,
  IN world_id integer,
  IN ip text
)
LANGUAGE 'sql'
AS $$
	UPDATE dbo.user_account SET
	    last_login = ap_SLog.login,
	    last_logout = ap_SLog.logout,
	    last_game = ap_SLog.game_id,
	    last_world = ap_SLog.world_id,
	    last_ip = ap_SLog.ip
	WHERE uid = ap_SLog.uid;
$$;

CREATE OR REPLACE PROCEDURE dbo.get_server_groups(
  IN uid integer
)
LANGUAGE 'sql'
AS $$
	SELECT server_group_id FROM dbo.user_server_group
	WHERE uid = get_server_groups.uid;
$$;

CREATE OR REPLACE PROCEDURE dbo.sp_LogUserNumbers(
  IN record_time timestamp with time zone,
  IN server_id integer,
  IN world_user integer,
  IN limit_user integer,
  IN auth_user integer,
  IN wait_user integer
)
LANGUAGE 'sql'
AS $$
/*INSERT INTO user_count
(
	record_time,
	server_id,
	world_user,
	limit_user,
	auth_user,
	wait_user
)
VALUES
(
	@record_time,
	@server_id,
	@world_user,
	@limit_user,
	@auth_user,
	@wait_user
) */
$$;
