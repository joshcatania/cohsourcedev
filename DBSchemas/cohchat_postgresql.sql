CREATE SCHEMA dbo;


-- Tables
CREATE TABLE dbo.channels(
	name varchar(32) PRIMARY KEY NOT NULL,
	data text NULL
);

CREATE TABLE dbo.parse_versions(
	users_crc int NULL,
	channels_crc int NULL,
	user_gmail_crc int NULL
);

CREATE TABLE dbo.user_gmail(
	user_gmail_id int PRIMARY KEY NOT NULL,
	data text NULL
);

CREATE TABLE dbo.users(
	user_id int PRIMARY KEY NOT NULL,
	data text NULL
);


INSERT INTO dbo.parse_versions (users_crc, channels_crc, user_gmail_crc) VALUES (0,0,0);


-- Stored Procedures
CREATE OR REPLACE PROCEDURE dbo.SP_delete_channel(
	IN name text
)
LANGUAGE 'sql'
AS $$
	DELETE FROM dbo.channels 
	WHERE name = SP_delete_channel.name;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_insert_or_update_channel(
	IN name text, 
	IN data text
)
LANGUAGE 'sql'
AS $$
  INSERT INTO dbo.channels (name, data) 
  VALUES (SP_insert_or_update_channel.name, SP_insert_or_update_channel.data)
  ON CONFLICT (name)
  DO UPDATE SET
    data = SP_insert_or_update_channel.data;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_insert_or_update_user(
	IN user_id integer, 
	IN data text
)
LANGUAGE 'sql'
AS $$
  INSERT INTO dbo.users (user_id, data) 
  VALUES (SP_insert_or_update_user.user_id, SP_insert_or_update_user.data)
  ON CONFLICT (user_id)
  DO UPDATE SET
    data = SP_insert_or_update_user.data;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_insert_or_update_user_gmail(
	IN user_gmail_id integer, 
	IN data text
)
LANGUAGE 'sql'
AS $$
  INSERT INTO dbo.user_gmail (user_gmail_id, data) 
  VALUES (SP_insert_or_update_user_gmail.user_gmail_id, SP_insert_or_update_user_gmail.data)
  ON CONFLICT (user_gmail_id)
  DO UPDATE SET
    data = SP_insert_or_update_user_gmail.data;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_write_channels_crc(
	IN crc integer
)
LANGUAGE 'sql'
AS $$
	UPDATE dbo.parse_versions set channels_crc = crc;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_write_user_gmail_crc(
	IN crc integer
)
LANGUAGE 'sql'
AS $$
	UPDATE dbo.parse_versions set user_gmail_crc = crc;
$$;

CREATE OR REPLACE PROCEDURE dbo.SP_write_users_crc(
	IN crc integer
)
LANGUAGE 'sql'
AS $$
	UPDATE dbo.parse_versions set users_crc = crc;
$$;