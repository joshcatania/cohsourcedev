CREATE SCHEMA dbo;

CREATE OR REPLACE FUNCTION dbo.constraint_exists(
  name text
)
RETURNS boolean
LANGUAGE 'sql'
AS $$
select (select 1 
  from pg_constraint 
  where conname = LOWER(name)
) is not null
$$;


-- SQL was previously sent from mapserver\container\containeremail.c
CREATE OR REPLACE PROCEDURE dbo.mapserver_delete_message(
	IN container_id integer,
	IN recipient_id integer
)
LANGUAGE 'plpgsql'
AS $$
BEGIN
	UPDATE dbo.Recipients SET State = 0
	WHERE ContainerId = mapserver_delete_message.container_id
	AND Recipient = mapserver_delete_message.recipient_id;
	
	IF EXISTS(
		SELECT 1 FROM dbo.Recipients
		GROUP BY State, ContainerId
		HAVING (ContainerId = mapserver_delete_message.container_id) 
		AND NOT (State = 0)
	) THEN 
		DELETE FROM dbo.Recipients WHERE ContainerId = mapserver_delete_message.container_id;
		DELETE FROM dbo.Email WHERE ContainerId = mapserver_delete_message.container_id;
	END IF;
END
$$;


-- Rest of schema is generated from Attributes and Templates when DbServer starts.