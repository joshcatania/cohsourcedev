USE [master]
GO

CREATE DATABASE [cohacctsrv]
 CONTAINMENT = NONE
 ON  PRIMARY 
( NAME = N'cohacctsrv', FILENAME = N'C:\MSSQL\MSSQL14.SQLEXPRESS\MSSQL\DATA\cohacctsrv.mdf' , SIZE = 8256KB , MAXSIZE = UNLIMITED, FILEGROWTH = 1024KB )
 LOG ON 
( NAME = N'cohacctsrv_log', FILENAME = N'C:\MSSQL\MSSQL14.SQLEXPRESS\MSSQL\DATA\cohacctsrv_log.ldf' , SIZE = 1344KB , MAXSIZE = 2048GB , FILEGROWTH = 10%)
GO
ALTER DATABASE [cohacctsrv] SET COMPATIBILITY_LEVEL = 110
GO
IF (1 = FULLTEXTSERVICEPROPERTY('IsFullTextInstalled'))
begin
EXEC [cohacctsrv].[dbo].[sp_fulltext_database] @action = 'enable'
end
GO
ALTER DATABASE [cohacctsrv] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [cohacctsrv] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [cohacctsrv] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [cohacctsrv] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [cohacctsrv] SET ARITHABORT OFF 
GO
ALTER DATABASE [cohacctsrv] SET AUTO_CLOSE OFF 
GO
ALTER DATABASE [cohacctsrv] SET AUTO_CREATE_STATISTICS ON 
GO
ALTER DATABASE [cohacctsrv] SET AUTO_SHRINK OFF 
GO
ALTER DATABASE [cohacctsrv] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [cohacctsrv] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [cohacctsrv] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [cohacctsrv] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [cohacctsrv] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [cohacctsrv] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [cohacctsrv] SET RECURSIVE_TRIGGERS OFF 
GO
ALTER DATABASE [cohacctsrv] SET  ENABLE_BROKER 
GO
ALTER DATABASE [cohacctsrv] SET AUTO_UPDATE_STATISTICS_ASYNC OFF 
GO
ALTER DATABASE [cohacctsrv] SET DATE_CORRELATION_OPTIMIZATION OFF 
GO
ALTER DATABASE [cohacctsrv] SET TRUSTWORTHY OFF 
GO
ALTER DATABASE [cohacctsrv] SET ALLOW_SNAPSHOT_ISOLATION OFF 
GO
ALTER DATABASE [cohacctsrv] SET PARAMETERIZATION SIMPLE 
GO
ALTER DATABASE [cohacctsrv] SET READ_COMMITTED_SNAPSHOT OFF 
GO
ALTER DATABASE [cohacctsrv] SET HONOR_BROKER_PRIORITY OFF 
GO
ALTER DATABASE [cohacctsrv] SET RECOVERY FULL 
GO
ALTER DATABASE [cohacctsrv] SET  MULTI_USER 
GO
ALTER DATABASE [cohacctsrv] SET PAGE_VERIFY CHECKSUM  
GO
ALTER DATABASE [cohacctsrv] SET DB_CHAINING OFF 
GO
ALTER DATABASE [cohacctsrv] SET FILESTREAM( NON_TRANSACTED_ACCESS = OFF ) 
GO
ALTER DATABASE [cohacctsrv] SET TARGET_RECOVERY_TIME = 0 SECONDS 
GO
EXEC sys.sp_db_vardecimal_storage_format N'cohacctsrv', N'ON'
GO

USE [cohacctsrv]
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[product_type] (
	[product_type_id] [int] NOT NULL,
	[name] [varchar](128) NULL
)
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[product] (
	[sku_id] [char](8) NOT NULL,
	[name] [varchar](128) NULL,
	[product_type_id] [int] NOT NULL,
	[grant_limit] [int] NULL,
	[expiration_seconds] [int] NULL
)
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[account] (
  [auth_id] [int] NOT NULL,
  [name] [varchar](14) NULL,
  [loyalty_bits] [binary](16) NULL,
  [last_loyalty_point_count] [smallint] NULL,
  [loyalty_points_spent] [smallint] NULL,
  [last_email_date] [smalldatetime] NULL,
  [last_num_emails_sent] [smallint] NULL,
  [free_xfer_date] [smalldatetime] NULL
)
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[game_log] (
  [order_id] [uniqueidentifier] NOT NULL,
  [auth_id] [int] NOT NULL,
  [sku_id] [char](8) NOT NULL,
  [transaction_date] [datetime] NULL,
  [shard_id] [tinyint] NULL,
  [ent_id] [int] NULL,
  [granted] [int] NULL,
  [claimed] [int] NULL,
  [csr_did_it] [bit] NULL,
  [saved] [bit] NULL,
  [parent_order_id] [uniqueidentifier] NULL
)
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[inventory] (
  [auth_id] [int] NOT NULL,
  [sku_id] [char](8) NOT NULL,
  [granted_total] [int] NULL,
  [claimed_total] [int] NULL,
  [saved_total] [int] NULL,
  [expires] [datetime] NULL
)
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[mtx_log] (
  [order_id] [uniqueidentifier] NOT NULL,
  [auth_id] [int] NOT NULL,
  [sku_id] [char](8) NOT NULL,
  [transaction_date] [datetime] NULL,
  [quantity] [int] NULL,
  [points] [int] NULL
)
GO

ALTER TABLE [dbo].[account] ADD  DEFAULT ('') FOR [name]
GO
ALTER TABLE [dbo].[account] ADD  DEFAULT (0) FOR [loyalty_bits]
GO
ALTER TABLE [dbo].[account] ADD  DEFAULT (0) FOR [last_loyalty_point_count]
GO
ALTER TABLE [dbo].[account] ADD  DEFAULT (0) FOR [loyalty_points_spent]
GO
ALTER TABLE [dbo].[account] ADD  DEFAULT (0) FOR [last_num_emails_sent]
GO
ALTER TABLE [dbo].[inventory] ADD  DEFAULT (0) FOR [saved_total]
GO

/****** Object:  StoredProcedure [dbo].[SP_add_game_transaction]    Script Date: 4/30/2019 5:43:50 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE [dbo].[SP_add_game_transaction]
	-- Add the parameters for the stored procedure here
	@order_id uniqueidentifier,
	@auth_id int,
	@sku_id char(8),
	@transaction_date datetime,
	@csr_did_it bit,
	@shard_id tinyint = NULL,
	@ent_id int = NULL,
	@granted int = NULL,
	@claimed int = NULL,
	@parent_order_id uniqueidentifier = NULL
AS
DECLARE
	@ncount int
BEGIN
	SET NOCOUNT ON;
	SET XACT_ABORT ON;

	BEGIN TRANSACTION;
	
	DECLARE @found int = 0;
	SELECT @found=COUNT(*) FROM dbo.game_log WHERE order_id=@order_id;
	
	IF @found = 0
	BEGIN
		DECLARE @expiration_seconds int = NULL;		
		SELECT @expiration_seconds=expiration_seconds FROM dbo.product WHERE sku_id=@sku_id;

		INSERT INTO dbo.game_log (order_id, auth_id, sku_id, transaction_date, csr_did_it, shard_id, ent_id, granted, claimed, parent_order_id)
			VALUES (@order_id, @auth_id, @sku_id, @transaction_date, @csr_did_it, @shard_id, @ent_id, @granted, @claimed, @parent_order_id);

		MERGE INTO inventory AS target
			USING (SELECT @auth_id, @sku_id, ISNULL(@granted,0), ISNULL(@claimed,0)) AS source (auth_id, sku_id, granted, claimed)
			ON (target.auth_id = source.auth_id) AND (target.sku_id = source.sku_id)
			WHEN NOT MATCHED BY TARGET THEN
				INSERT (auth_id, sku_id, granted_total, claimed_total, expires)
				VALUES (source.auth_id, source.sku_id, source.granted, source.claimed,
					DATEADD(second, @expiration_seconds * source.granted, GETDATE()))
			WHEN MATCHED THEN
				UPDATE SET
					granted_total = target.granted_total + source.granted,
					claimed_total = target.claimed_total + source.claimed,
					expires = DATEADD(second, @expiration_seconds * source.granted, GETDATE());
	END
	
	SELECT sku_id, granted_total, claimed_total, saved_total, expires FROM dbo.inventory WHERE auth_id=@auth_id AND sku_id=@sku_id;
	
	COMMIT TRANSACTION;
END

GO
/****** Object:  StoredProcedure [dbo].[SP_add_micro_transaction]    Script Date: 4/30/2019 5:43:50 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE [dbo].[SP_add_micro_transaction]
      @order_id uniqueidentifier,
      @auth_id int,
      @sku_id char(8),
      @transaction_date datetime,
      @quantity int,
      @points int
AS
BEGIN
      SET NOCOUNT ON;
      SET XACT_ABORT ON;
      
      BEGIN TRANSACTION;
      
      DECLARE @found int = 0;
      SELECT @found=COUNT(*) FROM dbo.mtx_log WHERE order_id=@order_id;
      
      IF @found = 0
      BEGIN
            DECLARE @expiration_seconds int = NULL;         
            SELECT @expiration_seconds=expiration_seconds FROM dbo.product WHERE sku_id=@sku_id;
      
            INSERT INTO dbo.mtx_log (order_id, auth_id, sku_id, transaction_date, quantity, points)
                  VALUES (@order_id, @auth_id, @sku_id, @transaction_date, @quantity, @points);

            MERGE INTO inventory AS target
                  USING (SELECT @auth_id, @sku_id, @quantity) AS source (auth_id, sku_id, quantity)
                  ON (target.auth_id = source.auth_id) AND (target.sku_id = source.sku_id)
                  WHEN NOT MATCHED BY TARGET THEN
                        INSERT (auth_id, sku_id, granted_total, claimed_total, expires)
                        VALUES (source.auth_id, source.sku_id, source.quantity, 0,
                              DATEADD(second, @expiration_seconds * source.quantity, GETDATE()))                   
                  WHEN MATCHED THEN
                        UPDATE SET granted_total = target.granted_total + source.quantity,
                              expires = DATEADD(second, @expiration_seconds * source.quantity, GETDATE());
      END
      
      SELECT sku_id, granted_total, claimed_total, saved_total, expires FROM dbo.inventory WHERE auth_id=@auth_id AND sku_id=@sku_id;
      
      COMMIT TRANSACTION;
END

GO
/****** Object:  StoredProcedure [dbo].[SP_find_or_create_account]    Script Date: 4/30/2019 5:43:50 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE [dbo].[SP_find_or_create_account] 
	@auth_id int,
	@name varchar(14) out,
	@loyalty_bits binary(16) out,
	@last_loyalty_point_count smallint out,
	@loyalty_points_spent smallint out,
	@last_email_date smalldatetime out,
	@last_num_emails_sent smallint out,
	@free_xfer_date smalldatetime out
AS
BEGIN
	SET NOCOUNT ON;
	DECLARE @found int = 0;
	
	SELECT @found=COUNT(*) FROM dbo.account WHERE auth_id=@auth_id;
	IF @found =0
	BEGIN
		INSERT INTO dbo.account (auth_id) VALUES (@auth_id);
	END
	
	IF @name IS NOT NULL
	BEGIN
		UPDATE dbo.account SET name=@name WHERE auth_id=@auth_id;
	END
	
	SELECT
		@name=name,
		@loyalty_bits=loyalty_bits,
		@last_loyalty_point_count=last_loyalty_point_count,
		@loyalty_points_spent=loyalty_points_spent,
		@last_email_date=last_email_date,
		@last_num_emails_sent=last_num_emails_sent,
		@free_xfer_date=free_xfer_date
		FROM dbo.account WHERE auth_id=@auth_id;
	
	SELECT sku_id, granted_total, claimed_total, saved_total, expires FROM dbo.inventory WHERE auth_id=@auth_id;
END

GO
/****** Object:  StoredProcedure [dbo].[SP_read_unsaved_game_transactions]    Script Date: 4/30/2019 5:43:50 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE [dbo].[SP_read_unsaved_game_transactions]
      @auth_id int,
      @shard_id tinyint = NULL,
      @ent_id int = NULL
AS
BEGIN
      SET NOCOUNT ON;
      SET XACT_ABORT ON;
      
      SELECT order_id, auth_id, sku_id, transaction_date, shard_id, ent_id, granted, claimed, csr_did_it
            FROM game_log
            WHERE auth_id=@auth_id AND claimed IS NOT NULL AND saved=0 AND parent_order_id IS NULL
                  AND (shard_id=@shard_id OR @shard_id IS NULL)
                  AND (ent_id=@ent_id OR @ent_id IS NULL);
END

GO
/****** Object:  StoredProcedure [dbo].[SP_revert_game_transaction]    Script Date: 4/30/2019 5:43:50 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE [dbo].[SP_revert_game_transaction]
	@auth_id int,
	@search_id uniqueidentifier
AS
BEGIN
	SET NOCOUNT ON;
	SET XACT_ABORT ON;

	DECLARE @order_id uniqueidentifier;
	DECLARE @sku_id char(8);
	DECLARE @granted int;
	DECLARE @claimed int;
	DECLARE @expiration_seconds int = NULL;		

	BEGIN TRANSACTION;
	
	DECLARE transaction_cursor CURSOR LOCAL FAST_FORWARD
		FOR SELECT order_id, sku_id, granted, claimed FROM dbo.game_log WHERE (order_id=@search_id OR parent_order_id=@search_id) AND auth_id=@auth_id AND saved=0;
			
	OPEN transaction_cursor;
	FETCH NEXT FROM transaction_cursor INTO @order_id, @sku_id, @granted, @claimed;
	WHILE @@FETCH_STATUS = 0	
	BEGIN
		SELECT @expiration_seconds=expiration_seconds FROM dbo.product WHERE sku_id=@sku_id;	
	
		DELETE FROM dbo.game_log WHERE order_id=@order_id;

		UPDATE inventory SET
			granted_total = granted_total - ISNULL(@granted,0),
			claimed_total = claimed_total - ISNULL(@claimed,0),
			expires = DATEADD(second, -@expiration_seconds * @granted, expires)
			WHERE auth_id=@auth_id AND sku_id=@sku_id;	
				
		SELECT sku_id, granted_total, claimed_total, saved_total, expires FROM dbo.inventory WHERE auth_id=@auth_id AND sku_id=@sku_id;

		FETCH NEXT FROM transaction_cursor INTO @order_id, @sku_id, @granted, @claimed;
	END

	CLOSE transaction_cursor;
	DEALLOCATE transaction_cursor;		

	COMMIT TRANSACTION;
END

GO
/****** Object:  StoredProcedure [dbo].[SP_save_game_transaction]    Script Date: 4/30/2019 5:43:50 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE [dbo].[SP_save_game_transaction]
	@auth_id int,
	@search_id uniqueidentifier
AS
BEGIN
	SET NOCOUNT ON;
	SET XACT_ABORT ON;

	DECLARE @order_id uniqueidentifier;
	DECLARE @claimed int;
	DECLARE @sku_id char(8);

	BEGIN TRANSACTION;
	
	DECLARE transaction_cursor CURSOR LOCAL FAST_FORWARD
		FOR SELECT order_id, sku_id, claimed FROM dbo.game_log WHERE (order_id=@search_id OR parent_order_id=@search_id) AND auth_id=@auth_id AND saved=0
		--FOR UPDATE OF saved;

	OPEN transaction_cursor;
	FETCH NEXT FROM transaction_cursor INTO @order_id, @sku_id, @claimed;
	WHILE @@FETCH_STATUS = 0	
	BEGIN
		UPDATE dbo.game_log SET saved=1 WHERE order_id=@order_id;
		IF @claimed > 0
		BEGIN
			UPDATE dbo.inventory SET saved_total = saved_total + @claimed WHERE auth_id=@auth_id AND sku_id=@sku_id;
		END
		-- Note: we don't save the number of saved grants, only saved claims.

		SELECT sku_id, granted_total, claimed_total, saved_total, expires FROM dbo.inventory WHERE auth_id=@auth_id AND sku_id=@sku_id;

		FETCH NEXT FROM transaction_cursor INTO @order_id, @sku_id, @claimed;
	END

	CLOSE transaction_cursor;
	DEALLOCATE transaction_cursor;		

	COMMIT TRANSACTION;
END

GO
/****** Object:  StoredProcedure [dbo].[SP_update_account]    Script Date: 4/30/2019 5:43:50 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE [dbo].[SP_update_account]
	@auth_id int,
	@loyalty_bits binary(16),
	@last_loyalty_point_count smallint,
	@loyalty_points_spent smallint,
	@last_email_date smalldatetime,
	@last_num_emails_sent smallint,
	@free_xfer_date smalldatetime
AS
BEGIN
	SET NOCOUNT ON;

	UPDATE dbo.account SET
		loyalty_bits=@loyalty_bits,
		last_loyalty_point_count=@last_loyalty_point_count,
		loyalty_points_spent=@loyalty_points_spent,
		last_email_date=@last_email_date,
		last_num_emails_sent=@last_num_emails_sent,
		free_xfer_date=@free_xfer_date
		WHERE auth_id=@auth_id;
		
	IF @@ROWCOUNT = 0 RAISERROR('No such account "%d"', 1, 0, @auth_id);
END

GO


USE [master]
GO
ALTER DATABASE [cohacctsrv] SET  READ_WRITE 
GO
