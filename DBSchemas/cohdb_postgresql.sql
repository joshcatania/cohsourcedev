CREATE SCHEMA dbo;

CREATE OR REPLACE FUNCTION public.constraint_exists(
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


-- Rest of schema is generated from Attributes and Templates when DbServer starts.