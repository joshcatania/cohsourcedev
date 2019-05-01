USE [master]
GO
/****** Object:  Database [cohdb]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE DATABASE [cohdb]
 CONTAINMENT = NONE
 ON  PRIMARY 
( NAME = N'cohdb', FILENAME = N'C:\MSSQL\MSSQL14.SQLEXPRESS\MSSQL\DATA\cohdb.mdf' , SIZE = 8256KB , MAXSIZE = UNLIMITED, FILEGROWTH = 1024KB )
 LOG ON 
( NAME = N'cohdb_log', FILENAME = N'C:\MSSQL\MSSQL14.SQLEXPRESS\MSSQL\DATA\cohdb_log.ldf' , SIZE = 1344KB , MAXSIZE = 2048GB , FILEGROWTH = 10%)
GO
ALTER DATABASE [cohdb] SET COMPATIBILITY_LEVEL = 110
GO
IF (1 = FULLTEXTSERVICEPROPERTY('IsFullTextInstalled'))
begin
EXEC [cohdb].[dbo].[sp_fulltext_database] @action = 'enable'
end
GO
ALTER DATABASE [cohdb] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [cohdb] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [cohdb] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [cohdb] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [cohdb] SET ARITHABORT OFF 
GO
ALTER DATABASE [cohdb] SET AUTO_CLOSE OFF 
GO
ALTER DATABASE [cohdb] SET AUTO_CREATE_STATISTICS ON 
GO
ALTER DATABASE [cohdb] SET AUTO_SHRINK OFF 
GO
ALTER DATABASE [cohdb] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [cohdb] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [cohdb] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [cohdb] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [cohdb] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [cohdb] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [cohdb] SET RECURSIVE_TRIGGERS OFF 
GO
ALTER DATABASE [cohdb] SET  ENABLE_BROKER 
GO
ALTER DATABASE [cohdb] SET AUTO_UPDATE_STATISTICS_ASYNC OFF 
GO
ALTER DATABASE [cohdb] SET DATE_CORRELATION_OPTIMIZATION OFF 
GO
ALTER DATABASE [cohdb] SET TRUSTWORTHY OFF 
GO
ALTER DATABASE [cohdb] SET ALLOW_SNAPSHOT_ISOLATION OFF 
GO
ALTER DATABASE [cohdb] SET PARAMETERIZATION SIMPLE 
GO
ALTER DATABASE [cohdb] SET READ_COMMITTED_SNAPSHOT OFF 
GO
ALTER DATABASE [cohdb] SET HONOR_BROKER_PRIORITY OFF 
GO
ALTER DATABASE [cohdb] SET RECOVERY FULL 
GO
ALTER DATABASE [cohdb] SET  MULTI_USER 
GO
ALTER DATABASE [cohdb] SET PAGE_VERIFY CHECKSUM  
GO
ALTER DATABASE [cohdb] SET DB_CHAINING OFF 
GO
ALTER DATABASE [cohdb] SET FILESTREAM( NON_TRANSACTED_ACCESS = OFF ) 
GO
ALTER DATABASE [cohdb] SET TARGET_RECOVERY_TIME = 0 SECONDS 
GO
EXEC sys.sp_db_vardecimal_storage_format N'cohdb', N'ON'
GO
USE [cohdb]
GO
/****** Object:  Table [dbo].[AccountInventory]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AccountInventory](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[SkuId0] [int] NULL,
	[SkuId1] [int] NULL,
	[Granted] [int] NULL,
	[Claimed] [int] NULL,
	[Expires] [int] NULL,
 CONSTRAINT [PK_AccountInventory] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Appearance]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Appearance](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[BodyType] [int] NULL,
	[ColorSkin] [int] NULL,
	[BodyScale] [real] NULL,
	[BoneScale] [real] NULL,
	[HeadScale] [real] NULL,
	[ShoulderScale] [real] NULL,
	[ChestScale] [real] NULL,
	[WaistScale] [real] NULL,
	[HipScale] [real] NULL,
	[LegScale] [real] NULL,
	[ConvertedScale] [int] NULL,
	[HeadScales] [int] NULL,
	[BrowScales] [int] NULL,
	[CheekScales] [int] NULL,
	[ChinScales] [int] NULL,
	[CraniumScales] [int] NULL,
	[JawScales] [int] NULL,
	[NoseScales] [int] NULL,
	[SuperPrimary] [int] NULL,
	[SuperSecondary] [int] NULL,
	[SuperPrimary2] [int] NULL,
	[SuperSecondary2] [int] NULL,
	[SuperTertiary] [int] NULL,
	[SuperQuaternary] [int] NULL,
	[SuperPrimaryAux1] [int] NULL,
	[SuperSecondaryAux1] [int] NULL,
	[SuperPrimary2Aux1] [int] NULL,
	[SuperSecondary2Aux1] [int] NULL,
	[SuperTertiaryAux1] [int] NULL,
	[SuperQuaternaryAux1] [int] NULL,
	[PowerColorPrimary1] [int] NULL,
	[PowerColorPrimary2] [int] NULL,
	[PowerColorSecondary1] [int] NULL,
	[PowerColorSecondary2] [int] NULL,
	[SuperPrimaryAux2] [int] NULL,
	[SuperSecondaryAux2] [int] NULL,
	[SuperPrimary2Aux2] [int] NULL,
	[SuperSecondary2Aux2] [int] NULL,
	[SuperTertiaryAux2] [int] NULL,
	[SuperQuaternaryAux2] [int] NULL,
	[SuperPrimaryAux3] [int] NULL,
	[SuperSecondaryAux3] [int] NULL,
	[SuperPrimary2Aux3] [int] NULL,
	[SuperSecondary2Aux3] [int] NULL,
	[SuperTertiaryAux3] [int] NULL,
	[SuperQuaternaryAux3] [int] NULL,
	[SuperPrimaryAux4] [int] NULL,
	[SuperSecondaryAux4] [int] NULL,
	[SuperPrimary2Aux4] [int] NULL,
	[SuperSecondary2Aux4] [int] NULL,
	[SuperTertiaryAux4] [int] NULL,
	[SuperQuaternaryAux4] [int] NULL,
	[SuperPrimaryAux5] [int] NULL,
	[SuperSecondaryAux5] [int] NULL,
	[SuperPrimary2Aux5] [int] NULL,
	[SuperSecondary2Aux5] [int] NULL,
	[SuperTertiaryAux5] [int] NULL,
	[SuperQuaternaryAux5] [int] NULL,
	[PrimaryPowerToken] [int] NULL,
	[SecondaryPowerToken] [int] NULL,
	[SuperColorSet] [int] NULL,
 CONSTRAINT [PK_Appearance] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[ArenaEvents]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ArenaEvents](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[UniqueId] [int] NULL,
	[LastUpdateDbid] [int] NULL,
	[LastUpdateCmd] [int] NULL,
	[LastUpdateName] [nvarchar](64) NULL,
	[EventType] [int] NULL,
	[TeamType] [int] NULL,
	[Weight] [int] NULL,
	[Duration] [int] NULL,
	[Custom] [int] NULL,
	[Listed] [int] NULL,
	[Phase] [int] NULL,
	[Round] [int] NULL,
	[StartTime] [int] NULL,
	[RoundStart] [int] NULL,
	[Scheduled] [int] NULL,
	[ServerEvent] [int] NULL,
	[DemandPlay] [int] NULL,
	[Sanctioned] [int] NULL,
	[VerifySanctioned] [int] NULL,
	[InviteOnly] [int] NULL,
	[NumSides] [int] NULL,
	[MinPlayers] [int] NULL,
	[MaxPlayers] [int] NULL,
	[TacticalStart] [int] NULL,
	[NoEnd] [int] NULL,
	[NoPool] [int] NULL,
	[NoStealth] [int] NULL,
	[NoTravel] [int] NULL,
	[NoObserve] [int] NULL,
	[Description] [nvarchar](512) NULL,
	[CreatorId] [int] NULL,
	[CreatorSg] [int] NULL,
	[OtherSg] [int] NULL,
	[MapName] [nvarchar](520) NULL,
	[CreatorName] [nvarchar](256) NULL,
	[CancelReason] [nvarchar](256) NULL,
	[CannotStart] [int] NULL,
	[EventProblems0] [nvarchar](64) NULL,
	[EventProblems1] [nvarchar](64) NULL,
	[EventProblems2] [nvarchar](64) NULL,
	[EventProblems3] [nvarchar](64) NULL,
	[EventProblems4] [nvarchar](64) NULL,
	[RoundType0] [int] NULL,
	[RoundType1] [int] NULL,
	[RoundType2] [int] NULL,
	[RoundType3] [int] NULL,
	[RoundType4] [int] NULL,
	[RoundType5] [int] NULL,
	[RoundType6] [int] NULL,
	[RoundType7] [int] NULL,
	[RoundType8] [int] NULL,
	[RoundType9] [int] NULL,
	[RoundType10] [int] NULL,
	[Entryfee] [int] NULL,
	[PetBattle] [int] NULL,
	[MapType] [int] NULL,
	[NoTravelSuppression] [int] NULL,
	[NoDiminishingReturns] [int] NULL,
	[VictoryValue] [int] NULL,
	[NoHealDecay] [int] NULL,
	[NoInspirations] [int] NULL,
	[EnableTempPowers] [int] NULL,
	[TournamentType] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[ArenaPlayers]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[ArenaPlayers](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[PrizeMoney] [int] NULL,
	[Connected] [int] NULL,
	[LastConnected] [int] NULL,
	[Wins0] [int] NULL,
	[Wins1] [int] NULL,
	[Wins2] [int] NULL,
	[Wins3] [int] NULL,
	[Wins4] [int] NULL,
	[Wins5] [int] NULL,
	[Wins6] [int] NULL,
	[Wins7] [int] NULL,
	[Draws0] [int] NULL,
	[Draws1] [int] NULL,
	[Draws2] [int] NULL,
	[Draws3] [int] NULL,
	[Draws4] [int] NULL,
	[Draws5] [int] NULL,
	[Draws6] [int] NULL,
	[Draws7] [int] NULL,
	[Losses0] [int] NULL,
	[Losses1] [int] NULL,
	[Losses2] [int] NULL,
	[Losses3] [int] NULL,
	[Losses4] [int] NULL,
	[Losses5] [int] NULL,
	[Losses6] [int] NULL,
	[Losses7] [int] NULL,
	[Rating0] [real] NULL,
	[Rating1] [real] NULL,
	[Rating2] [real] NULL,
	[Rating3] [real] NULL,
	[Rating4] [real] NULL,
	[Rating5] [real] NULL,
	[Rating6] [real] NULL,
	[Rating7] [real] NULL,
	[Provisional0] [int] NULL,
	[Provisional1] [int] NULL,
	[Provisional2] [int] NULL,
	[Provisional3] [int] NULL,
	[Provisional4] [int] NULL,
	[Provisional5] [int] NULL,
	[Provisional6] [int] NULL,
	[Provisional7] [int] NULL,
	[RaidParticipant] [varchar](8) NULL,
	[Wins8] [int] NULL,
	[Wins9] [int] NULL,
	[Draws8] [int] NULL,
	[Draws9] [int] NULL,
	[Losses8] [int] NULL,
	[Losses9] [int] NULL,
	[Rating8] [real] NULL,
	[Rating9] [real] NULL,
	[Provisional8] [int] NULL,
	[Provisional9] [int] NULL,
	[ArenaReward] [nvarchar](256) NULL,
	[WinsTotal] [int] NULL,
	[DrawsTotal] [int] NULL,
	[LossesTotal] [int] NULL,
	[WinsByEvent0] [int] NULL,
	[WinsByEvent1] [int] NULL,
	[WinsByEvent2] [int] NULL,
	[WinsByEvent3] [int] NULL,
	[WinsByEvent4] [int] NULL,
	[DrawsByEvent0] [int] NULL,
	[DrawsByEvent1] [int] NULL,
	[DrawsByEvent2] [int] NULL,
	[DrawsByEvent3] [int] NULL,
	[DrawsByEvent4] [int] NULL,
	[LossesByEvent0] [int] NULL,
	[LossesByEvent1] [int] NULL,
	[LossesByEvent2] [int] NULL,
	[LossesByEvent3] [int] NULL,
	[LossesByEvent4] [int] NULL,
	[WinsByTeam0] [int] NULL,
	[WinsByTeam1] [int] NULL,
	[WinsByTeam2] [int] NULL,
	[DrawsByTeam0] [int] NULL,
	[DrawsByTeam1] [int] NULL,
	[DrawsByTeam2] [int] NULL,
	[LossesByTeam0] [int] NULL,
	[LossesByTeam1] [int] NULL,
	[LossesByTeam2] [int] NULL,
	[WinsByVictoryType0] [int] NULL,
	[WinsByVictoryType1] [int] NULL,
	[WinsByVictoryType2] [int] NULL,
	[WinsByVictoryType3] [int] NULL,
	[WinsByVictoryType4] [int] NULL,
	[DrawsByVictoryType0] [int] NULL,
	[DrawsByVictoryType1] [int] NULL,
	[DrawsByVictoryType2] [int] NULL,
	[DrawsByVictoryType3] [int] NULL,
	[DrawsByVictoryType4] [int] NULL,
	[LossesByVictoryType0] [int] NULL,
	[LossesByVictoryType1] [int] NULL,
	[LossesByVictoryType2] [int] NULL,
	[LossesByVictoryType3] [int] NULL,
	[LossesByVictoryType4] [int] NULL,
	[WinsByWeightClass0] [int] NULL,
	[WinsByWeightClass1] [int] NULL,
	[WinsByWeightClass2] [int] NULL,
	[WinsByWeightClass3] [int] NULL,
	[WinsByWeightClass4] [int] NULL,
	[WinsByWeightClass5] [int] NULL,
	[WinsByWeightClass6] [int] NULL,
	[WinsByWeightClass7] [int] NULL,
	[WinsByWeightClass8] [int] NULL,
	[WinsByWeightClass9] [int] NULL,
	[WinsByWeightClass10] [int] NULL,
	[DrawsByWeightClass0] [int] NULL,
	[DrawsByWeightClass1] [int] NULL,
	[DrawsByWeightClass2] [int] NULL,
	[DrawsByWeightClass3] [int] NULL,
	[DrawsByWeightClass4] [int] NULL,
	[DrawsByWeightClass5] [int] NULL,
	[DrawsByWeightClass6] [int] NULL,
	[DrawsByWeightClass7] [int] NULL,
	[DrawsByWeightClass8] [int] NULL,
	[DrawsByWeightClass9] [int] NULL,
	[DrawsByWeightClass10] [int] NULL,
	[LossesByWeightClass0] [int] NULL,
	[LossesByWeightClass1] [int] NULL,
	[LossesByWeightClass2] [int] NULL,
	[LossesByWeightClass3] [int] NULL,
	[LossesByWeightClass4] [int] NULL,
	[LossesByWeightClass5] [int] NULL,
	[LossesByWeightClass6] [int] NULL,
	[LossesByWeightClass7] [int] NULL,
	[LossesByWeightClass8] [int] NULL,
	[LossesByWeightClass9] [int] NULL,
	[LossesByWeightClass10] [int] NULL,
	[WinsByEvent5] [int] NULL,
	[WinsByEvent6] [int] NULL,
	[WinsByEvent7] [int] NULL,
	[WinsByEvent8] [int] NULL,
	[WinsByEvent9] [int] NULL,
	[DrawsByEvent5] [int] NULL,
	[DrawsByEvent6] [int] NULL,
	[DrawsByEvent7] [int] NULL,
	[DrawsByEvent8] [int] NULL,
	[DrawsByEvent9] [int] NULL,
	[LossesByEvent5] [int] NULL,
	[LossesByEvent6] [int] NULL,
	[LossesByEvent7] [int] NULL,
	[LossesByEvent8] [int] NULL,
	[LossesByEvent9] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[AttackerParticipants]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AttackerParticipants](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[DbId] [int] NULL,
 CONSTRAINT [PK_AttackerParticipants] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[AttribMods]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AttribMods](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Idx] [int] NULL,
	[CategoryName] [int] NULL,
	[PowerSetName] [int] NULL,
	[PowerName] [int] NULL,
	[Duration] [real] NULL,
	[Magnitude] [real] NULL,
	[Timer] [real] NULL,
	[Chance] [real] NULL,
	[UiD] [int] NULL,
	[PetFlags] [int] NULL,
	[CustomFXToken] [int] NULL,
	[PrimaryTint] [int] NULL,
	[SecondaryTint] [int] NULL,
	[SuppressedByStacking] [tinyint] NULL,
 CONSTRAINT [PK_AttribMods] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Attributes]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Attributes](
	[Id] [int] NOT NULL,
	[Name] [varchar](128) NULL,
PRIMARY KEY CLUSTERED 
(
	[Id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[AutoCommands]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AutoCommands](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[ExecutionTime] [int] NULL,
	[Command] [nvarchar](512) NULL,
	[Creator] [nvarchar](64) NULL,
	[CreationTime] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[BadgeMonitor]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BadgeMonitor](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[iIdx] [int] NULL,
	[iOrder] [int] NULL,
 CONSTRAINT [PK_BadgeMonitor] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Badges]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Badges](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Owned] [varchar](2104) NULL,
	[c001] [int] NULL,
	[c002] [int] NULL,
	[c003] [int] NULL,
	[c004] [int] NULL,
	[c005] [int] NULL,
	[c006] [int] NULL,
	[c007] [int] NULL,
	[c008] [int] NULL,
	[c009] [int] NULL,
	[c010] [int] NULL,
	[c011] [int] NULL,
	[c012] [int] NULL,
	[c013] [int] NULL,
	[c014] [int] NULL,
	[c015] [int] NULL,
	[c016] [int] NULL,
	[c017] [int] NULL,
	[c018] [int] NULL,
	[c019] [int] NULL,
	[c020] [int] NULL,
	[c021] [int] NULL,
	[c022] [int] NULL,
	[c023] [int] NULL,
	[c024] [int] NULL,
	[c025] [int] NULL,
	[c026] [int] NULL,
	[c027] [int] NULL,
	[c028] [int] NULL,
	[c029] [int] NULL,
	[c030] [int] NULL,
	[c031] [int] NULL,
	[c032] [int] NULL,
	[c033] [int] NULL,
	[c034] [int] NULL,
	[c035] [int] NULL,
	[c036] [int] NULL,
	[c037] [int] NULL,
	[c038] [int] NULL,
	[c039] [int] NULL,
	[c040] [int] NULL,
	[c041] [int] NULL,
	[c042] [int] NULL,
	[c043] [int] NULL,
	[c044] [int] NULL,
	[c045] [int] NULL,
	[c046] [int] NULL,
	[c047] [int] NULL,
	[c048] [int] NULL,
	[c049] [int] NULL,
	[c050] [int] NULL,
	[c051] [int] NULL,
	[c052] [int] NULL,
	[c053] [int] NULL,
	[c054] [int] NULL,
	[c055] [int] NULL,
	[c056] [int] NULL,
	[c057] [int] NULL,
	[c058] [int] NULL,
	[c059] [int] NULL,
	[c060] [int] NULL,
	[c061] [int] NULL,
	[c062] [int] NULL,
	[c063] [int] NULL,
	[c064] [int] NULL,
	[c065] [int] NULL,
	[c066] [int] NULL,
	[c067] [int] NULL,
	[c068] [int] NULL,
	[c069] [int] NULL,
	[c070] [int] NULL,
	[c071] [int] NULL,
	[c072] [int] NULL,
	[c073] [int] NULL,
	[c074] [int] NULL,
	[c075] [int] NULL,
	[c076] [int] NULL,
	[c077] [int] NULL,
	[c078] [int] NULL,
	[c079] [int] NULL,
	[c080] [int] NULL,
	[c081] [int] NULL,
	[c082] [int] NULL,
	[c083] [int] NULL,
	[c084] [int] NULL,
	[c085] [int] NULL,
	[c086] [int] NULL,
	[c087] [int] NULL,
	[c088] [int] NULL,
	[c089] [int] NULL,
	[c090] [int] NULL,
	[c091] [int] NULL,
	[c092] [int] NULL,
	[c093] [int] NULL,
	[c094] [int] NULL,
	[c095] [int] NULL,
	[c096] [int] NULL,
	[c097] [int] NULL,
	[c098] [int] NULL,
	[c099] [int] NULL,
	[c100] [int] NULL,
	[c101] [int] NULL,
	[c102] [int] NULL,
	[c103] [int] NULL,
	[c104] [int] NULL,
	[c105] [int] NULL,
	[c106] [int] NULL,
	[c107] [int] NULL,
	[c108] [int] NULL,
	[c109] [int] NULL,
	[c110] [int] NULL,
	[c111] [int] NULL,
	[c112] [int] NULL,
	[c113] [int] NULL,
	[c114] [int] NULL,
	[c115] [int] NULL,
	[c116] [int] NULL,
	[c117] [int] NULL,
	[c118] [int] NULL,
	[c119] [int] NULL,
	[c120] [int] NULL,
	[c121] [int] NULL,
	[c122] [int] NULL,
	[c123] [int] NULL,
	[c124] [int] NULL,
	[c125] [int] NULL,
	[c126] [int] NULL,
	[c127] [int] NULL,
	[c128] [int] NULL,
	[c129] [int] NULL,
	[c130] [int] NULL,
	[c131] [int] NULL,
	[c132] [int] NULL,
	[c133] [int] NULL,
	[c134] [int] NULL,
	[c135] [int] NULL,
	[c136] [int] NULL,
	[c137] [int] NULL,
	[c138] [int] NULL,
	[c139] [int] NULL,
	[c140] [int] NULL,
	[c141] [int] NULL,
	[c142] [int] NULL,
	[c143] [int] NULL,
	[c144] [int] NULL,
	[c145] [int] NULL,
	[c146] [int] NULL,
	[c147] [int] NULL,
	[c148] [int] NULL,
	[c149] [int] NULL,
	[c150] [int] NULL,
	[c151] [int] NULL,
	[c152] [int] NULL,
	[c153] [int] NULL,
	[c154] [int] NULL,
	[c155] [int] NULL,
	[c156] [int] NULL,
	[c157] [int] NULL,
	[c158] [int] NULL,
	[c159] [int] NULL,
	[c160] [int] NULL,
	[c161] [int] NULL,
	[c162] [int] NULL,
	[c163] [int] NULL,
	[c164] [int] NULL,
	[c165] [int] NULL,
	[c166] [int] NULL,
	[c167] [int] NULL,
	[c168] [int] NULL,
	[c169] [int] NULL,
	[c170] [int] NULL,
	[c171] [int] NULL,
	[c172] [int] NULL,
	[c173] [int] NULL,
	[c174] [int] NULL,
	[c175] [int] NULL,
	[c176] [int] NULL,
	[c177] [int] NULL,
	[c178] [int] NULL,
	[c179] [int] NULL,
	[c180] [int] NULL,
	[c181] [int] NULL,
	[c182] [int] NULL,
	[c183] [int] NULL,
	[c184] [int] NULL,
	[c185] [int] NULL,
	[c186] [int] NULL,
	[c187] [int] NULL,
	[c188] [int] NULL,
	[c189] [int] NULL,
	[c190] [int] NULL,
	[c191] [int] NULL,
	[c192] [int] NULL,
	[c193] [int] NULL,
	[c194] [int] NULL,
	[c195] [int] NULL,
	[c196] [int] NULL,
	[c197] [int] NULL,
	[c198] [int] NULL,
	[c199] [int] NULL,
	[c200] [int] NULL,
	[c201] [int] NULL,
	[c202] [int] NULL,
	[c203] [int] NULL,
	[c204] [int] NULL,
	[c205] [int] NULL,
	[c206] [int] NULL,
	[c207] [int] NULL,
	[c208] [int] NULL,
	[c209] [int] NULL,
	[c210] [int] NULL,
	[c211] [int] NULL,
	[c212] [int] NULL,
	[c213] [int] NULL,
	[c214] [int] NULL,
	[c215] [int] NULL,
	[c216] [int] NULL,
	[c217] [int] NULL,
	[c218] [int] NULL,
	[c219] [int] NULL,
	[c220] [int] NULL,
	[c221] [int] NULL,
	[c222] [int] NULL,
	[c223] [int] NULL,
	[c224] [int] NULL,
	[c225] [int] NULL,
	[c226] [int] NULL,
	[c227] [int] NULL,
	[c228] [int] NULL,
	[c229] [int] NULL,
	[c230] [int] NULL,
	[c231] [int] NULL,
	[c232] [int] NULL,
	[c233] [int] NULL,
	[c234] [int] NULL,
	[c235] [int] NULL,
	[c236] [int] NULL,
	[c237] [int] NULL,
	[c238] [int] NULL,
	[c239] [int] NULL,
	[c240] [int] NULL,
	[c241] [int] NULL,
	[c242] [int] NULL,
	[c243] [int] NULL,
	[c244] [int] NULL,
	[c245] [int] NULL,
	[c246] [int] NULL,
	[c247] [int] NULL,
	[c248] [int] NULL,
	[c249] [int] NULL,
	[c250] [int] NULL,
	[c251] [int] NULL,
	[c252] [int] NULL,
	[c253] [int] NULL,
	[c254] [int] NULL,
	[c255] [int] NULL,
	[c256] [int] NULL,
	[c257] [int] NULL,
	[c258] [int] NULL,
	[c259] [int] NULL,
	[c260] [int] NULL,
	[c261] [int] NULL,
	[c262] [int] NULL,
	[c263] [int] NULL,
	[c264] [int] NULL,
	[c265] [int] NULL,
	[c266] [int] NULL,
	[c267] [int] NULL,
	[c268] [int] NULL,
	[c269] [int] NULL,
	[c270] [int] NULL,
	[c271] [int] NULL,
	[c272] [int] NULL,
	[c273] [int] NULL,
	[c274] [int] NULL,
	[c275] [int] NULL,
	[c276] [int] NULL,
	[c277] [int] NULL,
	[c278] [int] NULL,
	[c279] [int] NULL,
	[c280] [int] NULL,
	[c281] [int] NULL,
	[c282] [int] NULL,
	[c283] [int] NULL,
	[c284] [int] NULL,
	[c285] [int] NULL,
	[c286] [int] NULL,
	[c287] [int] NULL,
	[c288] [int] NULL,
	[c289] [int] NULL,
	[c290] [int] NULL,
	[c291] [int] NULL,
	[c292] [int] NULL,
	[c293] [int] NULL,
	[c294] [int] NULL,
	[c295] [int] NULL,
	[c296] [int] NULL,
	[c297] [int] NULL,
	[c298] [int] NULL,
	[c299] [int] NULL,
	[c300] [int] NULL,
	[c301] [int] NULL,
	[c302] [int] NULL,
	[c303] [int] NULL,
	[c304] [int] NULL,
	[c305] [int] NULL,
	[c306] [int] NULL,
	[c307] [int] NULL,
	[c308] [int] NULL,
	[c309] [int] NULL,
	[c310] [int] NULL,
	[c311] [int] NULL,
	[c312] [int] NULL,
	[c313] [int] NULL,
	[c314] [int] NULL,
	[c315] [int] NULL,
	[c316] [int] NULL,
	[c317] [int] NULL,
	[c318] [int] NULL,
	[c319] [int] NULL,
	[c320] [int] NULL,
	[c321] [int] NULL,
	[c322] [int] NULL,
	[c323] [int] NULL,
	[c324] [int] NULL,
	[c325] [int] NULL,
	[c326] [int] NULL,
	[c327] [int] NULL,
	[c328] [int] NULL,
	[c329] [int] NULL,
	[c330] [int] NULL,
	[c331] [int] NULL,
	[c332] [int] NULL,
	[c333] [int] NULL,
	[c334] [int] NULL,
	[c335] [int] NULL,
	[c336] [int] NULL,
	[c337] [int] NULL,
	[c338] [int] NULL,
	[c339] [int] NULL,
	[c340] [int] NULL,
	[c341] [int] NULL,
	[c342] [int] NULL,
	[c343] [int] NULL,
	[c344] [int] NULL,
	[c345] [int] NULL,
	[c346] [int] NULL,
	[c347] [int] NULL,
	[c348] [int] NULL,
	[c349] [int] NULL,
	[c350] [int] NULL,
	[c351] [int] NULL,
	[c352] [int] NULL,
	[c353] [int] NULL,
	[c354] [int] NULL,
	[c355] [int] NULL,
	[c356] [int] NULL,
	[c357] [int] NULL,
	[c358] [int] NULL,
	[c359] [int] NULL,
	[c360] [int] NULL,
	[c361] [int] NULL,
	[c362] [int] NULL,
	[c363] [int] NULL,
	[c364] [int] NULL,
	[c365] [int] NULL,
	[c366] [int] NULL,
	[c367] [int] NULL,
	[c368] [int] NULL,
	[c369] [int] NULL,
	[c370] [int] NULL,
	[c371] [int] NULL,
	[c372] [int] NULL,
	[c373] [int] NULL,
	[c374] [int] NULL,
	[c375] [int] NULL,
	[c376] [int] NULL,
	[c377] [int] NULL,
	[c378] [int] NULL,
	[c379] [int] NULL,
	[c380] [int] NULL,
	[c381] [int] NULL,
	[c382] [int] NULL,
	[c383] [int] NULL,
	[c384] [int] NULL,
	[c385] [int] NULL,
	[c386] [int] NULL,
	[c387] [int] NULL,
	[c388] [int] NULL,
	[c389] [int] NULL,
	[c390] [int] NULL,
	[c391] [int] NULL,
	[c392] [int] NULL,
	[c393] [int] NULL,
	[c394] [int] NULL,
	[c395] [int] NULL,
	[c396] [int] NULL,
	[c397] [int] NULL,
	[c398] [int] NULL,
	[c399] [int] NULL,
	[c400] [int] NULL,
	[c401] [int] NULL,
	[c402] [int] NULL,
	[c403] [int] NULL,
	[c404] [int] NULL,
	[c405] [int] NULL,
	[c406] [int] NULL,
	[c407] [int] NULL,
	[c408] [int] NULL,
	[c409] [int] NULL,
	[c410] [int] NULL,
	[c411] [int] NULL,
	[c412] [int] NULL,
	[c413] [int] NULL,
	[c414] [int] NULL,
	[c415] [int] NULL,
	[c416] [int] NULL,
	[c417] [int] NULL,
	[c418] [int] NULL,
	[c419] [int] NULL,
	[c420] [int] NULL,
	[c421] [int] NULL,
	[c422] [int] NULL,
	[c423] [int] NULL,
	[c424] [int] NULL,
	[c425] [int] NULL,
	[c426] [int] NULL,
	[c427] [int] NULL,
	[c428] [int] NULL,
	[c429] [int] NULL,
	[c430] [int] NULL,
	[c431] [int] NULL,
	[c432] [int] NULL,
	[c433] [int] NULL,
	[c434] [int] NULL,
	[c435] [int] NULL,
	[c436] [int] NULL,
	[c437] [int] NULL,
	[c438] [int] NULL,
	[c439] [int] NULL,
	[c440] [int] NULL,
	[c441] [int] NULL,
	[c442] [int] NULL,
	[c443] [int] NULL,
	[c444] [int] NULL,
	[c445] [int] NULL,
	[c446] [int] NULL,
	[c447] [int] NULL,
	[c448] [int] NULL,
	[c449] [int] NULL,
	[c450] [int] NULL,
	[c451] [int] NULL,
	[c452] [int] NULL,
	[c453] [int] NULL,
	[c454] [int] NULL,
	[c455] [int] NULL,
	[c456] [int] NULL,
	[c457] [int] NULL,
	[c458] [int] NULL,
	[c459] [int] NULL,
	[c460] [int] NULL,
	[c461] [int] NULL,
	[c462] [int] NULL,
	[c463] [int] NULL,
	[c464] [int] NULL,
	[c465] [int] NULL,
	[c466] [int] NULL,
	[c467] [int] NULL,
	[c468] [int] NULL,
	[c469] [int] NULL,
	[c470] [int] NULL,
	[c471] [int] NULL,
	[c472] [int] NULL,
	[c473] [int] NULL,
	[c474] [int] NULL,
	[c475] [int] NULL,
	[c476] [int] NULL,
	[c477] [int] NULL,
	[c478] [int] NULL,
	[c479] [int] NULL,
	[c480] [int] NULL,
	[c481] [int] NULL,
	[c482] [int] NULL,
	[c483] [int] NULL,
	[c484] [int] NULL,
	[c485] [int] NULL,
	[c486] [int] NULL,
	[c487] [int] NULL,
	[c488] [int] NULL,
	[c489] [int] NULL,
	[c490] [int] NULL,
	[c491] [int] NULL,
	[c492] [int] NULL,
	[c493] [int] NULL,
	[c494] [int] NULL,
	[c495] [int] NULL,
	[c496] [int] NULL,
	[c497] [int] NULL,
	[c498] [int] NULL,
	[c499] [int] NULL,
	[c500] [int] NULL,
	[c501] [int] NULL,
	[c502] [int] NULL,
	[c503] [int] NULL,
	[c504] [int] NULL,
	[c505] [int] NULL,
	[c506] [int] NULL,
	[c507] [int] NULL,
	[c508] [int] NULL,
	[c509] [int] NULL,
	[c510] [int] NULL,
	[c511] [int] NULL,
	[c512] [int] NULL,
	[c513] [int] NULL,
	[c514] [int] NULL,
	[c515] [int] NULL,
	[c516] [int] NULL,
	[c517] [int] NULL,
	[c518] [int] NULL,
	[c519] [int] NULL,
	[c520] [int] NULL,
	[c521] [int] NULL,
	[c522] [int] NULL,
	[c523] [int] NULL,
	[c524] [int] NULL,
	[c525] [int] NULL,
	[c526] [int] NULL,
	[c527] [int] NULL,
	[c528] [int] NULL,
	[c529] [int] NULL,
	[c530] [int] NULL,
	[c531] [int] NULL,
	[c532] [int] NULL,
	[c533] [int] NULL,
	[c534] [int] NULL,
	[c535] [int] NULL,
	[c536] [int] NULL,
	[c537] [int] NULL,
	[c538] [int] NULL,
	[c539] [int] NULL,
	[c540] [int] NULL,
	[c541] [int] NULL,
	[c542] [int] NULL,
	[c543] [int] NULL,
	[c544] [int] NULL,
	[c545] [int] NULL,
	[c546] [int] NULL,
	[c547] [int] NULL,
	[c548] [int] NULL,
	[c549] [int] NULL,
	[c550] [int] NULL,
	[c551] [int] NULL,
	[c552] [int] NULL,
	[c553] [int] NULL,
	[c554] [int] NULL,
	[c555] [int] NULL,
	[c556] [int] NULL,
	[c557] [int] NULL,
	[c558] [int] NULL,
	[c559] [int] NULL,
	[c560] [int] NULL,
	[c561] [int] NULL,
	[c562] [int] NULL,
	[c563] [int] NULL,
	[c564] [int] NULL,
	[c565] [int] NULL,
	[c566] [int] NULL,
	[c567] [int] NULL,
	[c568] [int] NULL,
	[c569] [int] NULL,
	[c570] [int] NULL,
	[c571] [int] NULL,
	[c572] [int] NULL,
	[c573] [int] NULL,
	[c574] [int] NULL,
	[c575] [int] NULL,
	[c576] [int] NULL,
	[c577] [int] NULL,
	[c578] [int] NULL,
	[c579] [int] NULL,
	[c580] [int] NULL,
	[c581] [int] NULL,
	[c582] [int] NULL,
	[c583] [int] NULL,
	[c584] [int] NULL,
	[c585] [int] NULL,
	[c586] [int] NULL,
	[c587] [int] NULL,
	[c588] [int] NULL,
	[c589] [int] NULL,
	[c590] [int] NULL,
	[c591] [int] NULL,
	[c592] [int] NULL,
	[c593] [int] NULL,
	[c594] [int] NULL,
	[c595] [int] NULL,
	[c596] [int] NULL,
	[c597] [int] NULL,
	[c598] [int] NULL,
	[c599] [int] NULL,
	[c600] [int] NULL,
	[c601] [int] NULL,
	[c602] [int] NULL,
	[c603] [int] NULL,
	[c604] [int] NULL,
	[c605] [int] NULL,
	[c606] [int] NULL,
	[c607] [int] NULL,
	[c608] [int] NULL,
	[c609] [int] NULL,
	[c610] [int] NULL,
	[c611] [int] NULL,
	[c612] [int] NULL,
	[c613] [int] NULL,
	[c614] [int] NULL,
	[c615] [int] NULL,
	[c616] [int] NULL,
	[c617] [int] NULL,
	[c618] [int] NULL,
	[c619] [int] NULL,
	[c620] [int] NULL,
	[c621] [int] NULL,
	[c622] [int] NULL,
	[c623] [int] NULL,
	[c624] [int] NULL,
	[c625] [int] NULL,
	[c626] [int] NULL,
	[c627] [int] NULL,
	[c628] [int] NULL,
	[c629] [int] NULL,
	[c630] [int] NULL,
	[c631] [int] NULL,
	[c632] [int] NULL,
	[c633] [int] NULL,
	[c634] [int] NULL,
	[c635] [int] NULL,
	[c636] [int] NULL,
	[c637] [int] NULL,
	[c638] [int] NULL,
	[c639] [int] NULL,
	[c640] [int] NULL,
	[c641] [int] NULL,
	[c642] [int] NULL,
	[c643] [int] NULL,
	[c644] [int] NULL,
	[c645] [int] NULL,
	[c646] [int] NULL,
	[c647] [int] NULL,
	[c648] [int] NULL,
	[c649] [int] NULL,
	[c650] [int] NULL,
	[c651] [int] NULL,
	[c652] [int] NULL,
	[c653] [int] NULL,
	[c654] [int] NULL,
	[c655] [int] NULL,
	[c656] [int] NULL,
	[c657] [int] NULL,
	[c658] [int] NULL,
	[c659] [int] NULL,
	[c660] [int] NULL,
	[c661] [int] NULL,
	[c662] [int] NULL,
	[c663] [int] NULL,
	[c664] [int] NULL,
	[c665] [int] NULL,
	[c666] [int] NULL,
	[c667] [int] NULL,
	[c668] [int] NULL,
	[c669] [int] NULL,
	[c670] [int] NULL,
	[c671] [int] NULL,
	[c672] [int] NULL,
	[c673] [int] NULL,
	[c674] [int] NULL,
	[c675] [int] NULL,
	[c676] [int] NULL,
	[c677] [int] NULL,
	[c678] [int] NULL,
	[c679] [int] NULL,
	[c680] [int] NULL,
	[c681] [int] NULL,
	[c682] [int] NULL,
	[c683] [int] NULL,
	[c684] [int] NULL,
	[c685] [int] NULL,
	[c686] [int] NULL,
	[c687] [int] NULL,
	[c688] [int] NULL,
	[c689] [int] NULL,
	[c690] [int] NULL,
	[c691] [int] NULL,
	[c692] [int] NULL,
	[c693] [int] NULL,
	[c694] [int] NULL,
	[c695] [int] NULL,
	[c696] [int] NULL,
	[c697] [int] NULL,
	[c698] [int] NULL,
	[c699] [int] NULL,
	[c700] [int] NULL,
	[c701] [int] NULL,
	[c702] [int] NULL,
	[c703] [int] NULL,
	[c704] [int] NULL,
	[c705] [int] NULL,
	[c706] [int] NULL,
	[c707] [int] NULL,
	[c708] [int] NULL,
	[c709] [int] NULL,
	[c710] [int] NULL,
	[c711] [int] NULL,
	[c712] [int] NULL,
	[c713] [int] NULL,
	[c714] [int] NULL,
	[c715] [int] NULL,
	[c716] [int] NULL,
	[c717] [int] NULL,
	[c718] [int] NULL,
	[c719] [int] NULL,
	[c720] [int] NULL,
	[c721] [int] NULL,
	[c722] [int] NULL,
	[c723] [int] NULL,
	[c724] [int] NULL,
	[c725] [int] NULL,
	[c726] [int] NULL,
	[c727] [int] NULL,
	[c728] [int] NULL,
	[c729] [int] NULL,
	[c730] [int] NULL,
	[c731] [int] NULL,
	[c732] [int] NULL,
	[c733] [int] NULL,
	[c734] [int] NULL,
	[c735] [int] NULL,
	[c736] [int] NULL,
	[c737] [int] NULL,
	[c738] [int] NULL,
	[c739] [int] NULL,
	[c740] [int] NULL,
	[c741] [int] NULL,
	[c742] [int] NULL,
	[c743] [int] NULL,
	[c744] [int] NULL,
	[c745] [int] NULL,
	[c746] [int] NULL,
	[c747] [int] NULL,
	[c748] [int] NULL,
	[c749] [int] NULL,
	[c750] [int] NULL,
	[c751] [int] NULL,
	[c752] [int] NULL,
	[c753] [int] NULL,
	[c754] [int] NULL,
	[c755] [int] NULL,
	[c756] [int] NULL,
	[c757] [int] NULL,
	[c758] [int] NULL,
	[c759] [int] NULL,
	[c760] [int] NULL,
	[c761] [int] NULL,
	[c762] [int] NULL,
	[c763] [int] NULL,
	[c764] [int] NULL,
	[c765] [int] NULL,
	[c766] [int] NULL,
	[c767] [int] NULL,
	[c768] [int] NULL,
	[c769] [int] NULL,
	[c770] [int] NULL,
	[c771] [int] NULL,
	[c772] [int] NULL,
	[c773] [int] NULL,
	[c774] [int] NULL,
	[c775] [int] NULL,
	[c776] [int] NULL,
	[c777] [int] NULL,
	[c778] [int] NULL,
	[c779] [int] NULL,
	[c780] [int] NULL,
	[c781] [int] NULL,
	[c782] [int] NULL,
	[c783] [int] NULL,
	[c784] [int] NULL,
	[c785] [int] NULL,
	[c786] [int] NULL,
	[c787] [int] NULL,
	[c788] [int] NULL,
	[c789] [int] NULL,
	[c790] [int] NULL,
	[c791] [int] NULL,
	[c792] [int] NULL,
	[c793] [int] NULL,
	[c794] [int] NULL,
	[c795] [int] NULL,
	[c796] [int] NULL,
	[c797] [int] NULL,
	[c798] [int] NULL,
	[c799] [int] NULL,
	[c800] [int] NULL,
	[c801] [int] NULL,
	[c802] [int] NULL,
	[c803] [int] NULL,
	[c804] [int] NULL,
	[c805] [int] NULL,
	[c806] [int] NULL,
	[c807] [int] NULL,
	[c808] [int] NULL,
	[c809] [int] NULL,
	[c810] [int] NULL,
	[c811] [int] NULL,
	[c812] [int] NULL,
	[c813] [int] NULL,
	[c814] [int] NULL,
	[c815] [int] NULL,
	[c816] [int] NULL,
	[c817] [int] NULL,
	[c818] [int] NULL,
	[c819] [int] NULL,
	[c820] [int] NULL,
	[c821] [int] NULL,
	[c822] [int] NULL,
	[c823] [int] NULL,
	[c824] [int] NULL,
	[c825] [int] NULL,
	[c826] [int] NULL,
	[c827] [int] NULL,
	[c828] [int] NULL,
	[c829] [int] NULL,
	[c830] [int] NULL,
	[c831] [int] NULL,
	[c832] [int] NULL,
	[c833] [int] NULL,
	[c834] [int] NULL,
	[c835] [int] NULL,
	[c836] [int] NULL,
	[c837] [int] NULL,
	[c838] [int] NULL,
	[c839] [int] NULL,
	[c840] [int] NULL,
	[c841] [int] NULL,
	[c842] [int] NULL,
	[c843] [int] NULL,
	[c844] [int] NULL,
	[c845] [int] NULL,
	[c846] [int] NULL,
	[c847] [int] NULL,
	[c848] [int] NULL,
	[c849] [int] NULL,
	[c850] [int] NULL,
	[c851] [int] NULL,
	[c852] [int] NULL,
	[c853] [int] NULL,
	[c854] [int] NULL,
	[c855] [int] NULL,
	[c856] [int] NULL,
	[c857] [int] NULL,
	[c858] [int] NULL,
	[c859] [int] NULL,
	[c860] [int] NULL,
	[c861] [int] NULL,
 CONSTRAINT [PK_Badges] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[Badges01]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Badges01](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[c862] [int] NULL,
	[c863] [int] NULL,
	[c864] [int] NULL,
	[c865] [int] NULL,
	[c866] [int] NULL,
	[c867] [int] NULL,
	[c868] [int] NULL,
	[c869] [int] NULL,
	[c870] [int] NULL,
	[c871] [int] NULL,
	[c872] [int] NULL,
	[c873] [int] NULL,
	[c874] [int] NULL,
	[c875] [int] NULL,
	[c876] [int] NULL,
	[c877] [int] NULL,
	[c878] [int] NULL,
	[c879] [int] NULL,
	[c880] [int] NULL,
	[c881] [int] NULL,
	[c882] [int] NULL,
	[c883] [int] NULL,
	[c884] [int] NULL,
	[c885] [int] NULL,
	[c886] [int] NULL,
	[c887] [int] NULL,
	[c888] [int] NULL,
	[c889] [int] NULL,
	[c890] [int] NULL,
	[c891] [int] NULL,
	[c892] [int] NULL,
	[c893] [int] NULL,
	[c894] [int] NULL,
	[c895] [int] NULL,
	[c896] [int] NULL,
	[c897] [int] NULL,
	[c898] [int] NULL,
	[c899] [int] NULL,
	[c900] [int] NULL,
	[c901] [int] NULL,
	[c902] [int] NULL,
	[c903] [int] NULL,
	[c904] [int] NULL,
	[c905] [int] NULL,
	[c906] [int] NULL,
	[c907] [int] NULL,
	[c908] [int] NULL,
	[c909] [int] NULL,
	[c910] [int] NULL,
	[c911] [int] NULL,
	[c912] [int] NULL,
	[c913] [int] NULL,
	[c914] [int] NULL,
	[c915] [int] NULL,
	[c916] [int] NULL,
	[c917] [int] NULL,
	[c918] [int] NULL,
	[c919] [int] NULL,
	[c920] [int] NULL,
	[c921] [int] NULL,
	[c922] [int] NULL,
	[c923] [int] NULL,
	[c924] [int] NULL,
	[c925] [int] NULL,
	[c926] [int] NULL,
	[c927] [int] NULL,
	[c928] [int] NULL,
	[c929] [int] NULL,
	[c930] [int] NULL,
	[c931] [int] NULL,
	[c932] [int] NULL,
	[c933] [int] NULL,
	[c934] [int] NULL,
	[c935] [int] NULL,
	[c936] [int] NULL,
	[c937] [int] NULL,
	[c938] [int] NULL,
	[c939] [int] NULL,
	[c940] [int] NULL,
	[c941] [int] NULL,
	[c942] [int] NULL,
	[c943] [int] NULL,
	[c944] [int] NULL,
	[c945] [int] NULL,
	[c946] [int] NULL,
	[c947] [int] NULL,
	[c948] [int] NULL,
	[c949] [int] NULL,
	[c950] [int] NULL,
	[c951] [int] NULL,
	[c952] [int] NULL,
	[c953] [int] NULL,
	[c954] [int] NULL,
	[c955] [int] NULL,
	[c956] [int] NULL,
	[c957] [int] NULL,
	[c958] [int] NULL,
	[c959] [int] NULL,
	[c960] [int] NULL,
	[c961] [int] NULL,
	[c962] [int] NULL,
	[c963] [int] NULL,
	[c964] [int] NULL,
	[c965] [int] NULL,
	[c966] [int] NULL,
	[c967] [int] NULL,
	[c968] [int] NULL,
	[c969] [int] NULL,
	[c970] [int] NULL,
	[c971] [int] NULL,
	[c972] [int] NULL,
	[c973] [int] NULL,
	[c974] [int] NULL,
	[c975] [int] NULL,
	[c976] [int] NULL,
	[c977] [int] NULL,
	[c978] [int] NULL,
	[c979] [int] NULL,
	[c980] [int] NULL,
	[c981] [int] NULL,
	[c982] [int] NULL,
	[c983] [int] NULL,
	[c984] [int] NULL,
	[c985] [int] NULL,
	[c986] [int] NULL,
	[c987] [int] NULL,
	[c988] [int] NULL,
	[c989] [int] NULL,
	[c990] [int] NULL,
	[c991] [int] NULL,
	[c992] [int] NULL,
	[c993] [int] NULL,
	[c994] [int] NULL,
	[c995] [int] NULL,
	[c996] [int] NULL,
	[c997] [int] NULL,
	[c998] [int] NULL,
	[c999] [int] NULL,
	[c1000] [int] NULL,
	[c1001] [int] NULL,
	[c1002] [int] NULL,
	[c1003] [int] NULL,
	[c1004] [int] NULL,
	[c1005] [int] NULL,
	[c1006] [int] NULL,
	[c1007] [int] NULL,
	[c1008] [int] NULL,
	[c1009] [int] NULL,
	[c1010] [int] NULL,
	[c1011] [int] NULL,
	[c1012] [int] NULL,
	[c1013] [int] NULL,
	[c1014] [int] NULL,
	[c1015] [int] NULL,
	[c1016] [int] NULL,
	[c1017] [int] NULL,
	[c1018] [int] NULL,
	[c1019] [int] NULL,
	[c1020] [int] NULL,
	[c1021] [int] NULL,
	[c1022] [int] NULL,
	[c1023] [int] NULL,
	[c1024] [int] NULL,
	[c1025] [int] NULL,
	[c1026] [int] NULL,
	[c1027] [int] NULL,
	[c1028] [int] NULL,
	[c1029] [int] NULL,
	[c1030] [int] NULL,
	[c1031] [int] NULL,
	[c1032] [int] NULL,
	[c1033] [int] NULL,
	[c1034] [int] NULL,
	[c1035] [int] NULL,
	[c1036] [int] NULL,
	[c1037] [int] NULL,
	[c1038] [int] NULL,
	[c1039] [int] NULL,
	[c1040] [int] NULL,
	[c1041] [int] NULL,
	[c1042] [int] NULL,
	[c1043] [int] NULL,
	[c1044] [int] NULL,
	[c1045] [int] NULL,
	[c1046] [int] NULL,
	[c1047] [int] NULL,
	[c1048] [int] NULL,
	[c1049] [int] NULL,
	[c1050] [int] NULL,
	[c1051] [int] NULL,
	[c1052] [int] NULL,
	[c1053] [int] NULL,
	[c1054] [int] NULL,
	[c1055] [int] NULL,
	[c1056] [int] NULL,
	[c1057] [int] NULL,
	[c1058] [int] NULL,
	[c1059] [int] NULL,
	[c1060] [int] NULL,
	[c1061] [int] NULL,
	[c1062] [int] NULL,
	[c1063] [int] NULL,
	[c1064] [int] NULL,
	[c1065] [int] NULL,
	[c1066] [int] NULL,
	[c1067] [int] NULL,
	[c1068] [int] NULL,
	[c1069] [int] NULL,
	[c1070] [int] NULL,
	[c1071] [int] NULL,
	[c1072] [int] NULL,
	[c1073] [int] NULL,
	[c1074] [int] NULL,
	[c1075] [int] NULL,
	[c1076] [int] NULL,
	[c1077] [int] NULL,
	[c1078] [int] NULL,
	[c1079] [int] NULL,
	[c1080] [int] NULL,
	[c1081] [int] NULL,
	[c1082] [int] NULL,
	[c1083] [int] NULL,
	[c1084] [int] NULL,
	[c1085] [int] NULL,
	[c1086] [int] NULL,
	[c1087] [int] NULL,
	[c1088] [int] NULL,
	[c1089] [int] NULL,
	[c1090] [int] NULL,
	[c1091] [int] NULL,
	[c1092] [int] NULL,
	[c1093] [int] NULL,
	[c1094] [int] NULL,
	[c1095] [int] NULL,
	[c1096] [int] NULL,
	[c1097] [int] NULL,
	[c1098] [int] NULL,
	[c1099] [int] NULL,
	[c1100] [int] NULL,
	[c1101] [int] NULL,
	[c1102] [int] NULL,
	[c1103] [int] NULL,
	[c1104] [int] NULL,
	[c1105] [int] NULL,
	[c1106] [int] NULL,
	[c1107] [int] NULL,
	[c1108] [int] NULL,
	[c1109] [int] NULL,
	[c1110] [int] NULL,
	[c1111] [int] NULL,
	[c1112] [int] NULL,
	[c1113] [int] NULL,
	[c1114] [int] NULL,
	[c1115] [int] NULL,
	[c1116] [int] NULL,
	[c1117] [int] NULL,
	[c1118] [int] NULL,
	[c1119] [int] NULL,
	[c1120] [int] NULL,
	[c1121] [int] NULL,
	[c1122] [int] NULL,
	[c1123] [int] NULL,
	[c1124] [int] NULL,
	[c1125] [int] NULL,
	[c1126] [int] NULL,
	[c1127] [int] NULL,
	[c1128] [int] NULL,
	[c1129] [int] NULL,
	[c1130] [int] NULL,
	[c1131] [int] NULL,
	[c1132] [int] NULL,
	[c1133] [int] NULL,
	[c1134] [int] NULL,
	[c1135] [int] NULL,
	[c1136] [int] NULL,
	[c1137] [int] NULL,
	[c1138] [int] NULL,
	[c1139] [int] NULL,
	[c1140] [int] NULL,
	[c1141] [int] NULL,
	[c1142] [int] NULL,
	[c1143] [int] NULL,
	[c1144] [int] NULL,
	[c1145] [int] NULL,
	[c1146] [int] NULL,
	[c1147] [int] NULL,
	[c1148] [int] NULL,
	[c1149] [int] NULL,
	[c1150] [int] NULL,
	[c1151] [int] NULL,
	[c1152] [int] NULL,
	[c1153] [int] NULL,
	[c1154] [int] NULL,
	[c1155] [int] NULL,
	[c1156] [int] NULL,
	[c1157] [int] NULL,
	[c1158] [int] NULL,
	[c1159] [int] NULL,
	[c1160] [int] NULL,
	[c1161] [int] NULL,
	[c1162] [int] NULL,
	[c1163] [int] NULL,
	[c1164] [int] NULL,
	[c1165] [int] NULL,
	[c1166] [int] NULL,
	[c1167] [int] NULL,
	[c1168] [int] NULL,
	[c1169] [int] NULL,
	[c1170] [int] NULL,
	[c1171] [int] NULL,
	[c1172] [int] NULL,
	[c1173] [int] NULL,
	[c1174] [int] NULL,
	[c1175] [int] NULL,
	[c1176] [int] NULL,
	[c1177] [int] NULL,
	[c1178] [int] NULL,
	[c1179] [int] NULL,
	[c1180] [int] NULL,
	[c1181] [int] NULL,
	[c1182] [int] NULL,
	[c1183] [int] NULL,
	[c1184] [int] NULL,
	[c1185] [int] NULL,
	[c1186] [int] NULL,
	[c1187] [int] NULL,
	[c1188] [int] NULL,
	[c1189] [int] NULL,
	[c1190] [int] NULL,
	[c1191] [int] NULL,
	[c1192] [int] NULL,
	[c1193] [int] NULL,
	[c1194] [int] NULL,
	[c1195] [int] NULL,
	[c1196] [int] NULL,
	[c1197] [int] NULL,
	[c1198] [int] NULL,
	[c1199] [int] NULL,
	[c1200] [int] NULL,
	[c1201] [int] NULL,
	[c1202] [int] NULL,
	[c1203] [int] NULL,
	[c1204] [int] NULL,
	[c1205] [int] NULL,
	[c1206] [int] NULL,
	[c1207] [int] NULL,
	[c1208] [int] NULL,
	[c1209] [int] NULL,
	[c1210] [int] NULL,
	[c1211] [int] NULL,
	[c1212] [int] NULL,
	[c1213] [int] NULL,
	[c1214] [int] NULL,
	[c1215] [int] NULL,
	[c1216] [int] NULL,
	[c1217] [int] NULL,
	[c1218] [int] NULL,
	[c1219] [int] NULL,
	[c1220] [int] NULL,
	[c1221] [int] NULL,
	[c1222] [int] NULL,
	[c1223] [int] NULL,
	[c1224] [int] NULL,
	[c1225] [int] NULL,
	[c1226] [int] NULL,
	[c1227] [int] NULL,
	[c1228] [int] NULL,
	[c1229] [int] NULL,
	[c1230] [int] NULL,
	[c1231] [int] NULL,
	[c1232] [int] NULL,
	[c1233] [int] NULL,
	[c1234] [int] NULL,
	[c1235] [int] NULL,
	[c1236] [int] NULL,
	[c1237] [int] NULL,
	[c1238] [int] NULL,
	[c1239] [int] NULL,
	[c1240] [int] NULL,
	[c1241] [int] NULL,
	[c1242] [int] NULL,
	[c1243] [int] NULL,
	[c1244] [int] NULL,
	[c1245] [int] NULL,
	[c1246] [int] NULL,
	[c1247] [int] NULL,
	[c1248] [int] NULL,
	[c1249] [int] NULL,
	[c1250] [int] NULL,
	[c1251] [int] NULL,
	[c1252] [int] NULL,
	[c1253] [int] NULL,
	[c1254] [int] NULL,
	[c1255] [int] NULL,
	[c1256] [int] NULL,
	[c1257] [int] NULL,
	[c1258] [int] NULL,
	[c1259] [int] NULL,
	[c1260] [int] NULL,
	[c1261] [int] NULL,
	[c1262] [int] NULL,
	[c1263] [int] NULL,
	[c1264] [int] NULL,
	[c1265] [int] NULL,
	[c1266] [int] NULL,
	[c1267] [int] NULL,
	[c1268] [int] NULL,
	[c1269] [int] NULL,
	[c1270] [int] NULL,
	[c1271] [int] NULL,
	[c1272] [int] NULL,
	[c1273] [int] NULL,
	[c1274] [int] NULL,
	[c1275] [int] NULL,
	[c1276] [int] NULL,
	[c1277] [int] NULL,
	[c1278] [int] NULL,
	[c1279] [int] NULL,
	[c1280] [int] NULL,
	[c1281] [int] NULL,
	[c1282] [int] NULL,
	[c1283] [int] NULL,
	[c1284] [int] NULL,
	[c1285] [int] NULL,
	[c1286] [int] NULL,
	[c1287] [int] NULL,
	[c1288] [int] NULL,
	[c1289] [int] NULL,
	[c1290] [int] NULL,
	[c1291] [int] NULL,
	[c1292] [int] NULL,
	[c1293] [int] NULL,
	[c1294] [int] NULL,
	[c1295] [int] NULL,
	[c1296] [int] NULL,
	[c1297] [int] NULL,
	[c1298] [int] NULL,
	[c1299] [int] NULL,
	[c1300] [int] NULL,
	[c1301] [int] NULL,
	[c1302] [int] NULL,
	[c1303] [int] NULL,
	[c1304] [int] NULL,
	[c1305] [int] NULL,
	[c1306] [int] NULL,
	[c1307] [int] NULL,
	[c1308] [int] NULL,
	[c1309] [int] NULL,
	[c1310] [int] NULL,
	[c1311] [int] NULL,
	[c1312] [int] NULL,
	[c1313] [int] NULL,
	[c1314] [int] NULL,
	[c1315] [int] NULL,
	[c1316] [int] NULL,
	[c1317] [int] NULL,
	[c1318] [int] NULL,
	[c1319] [int] NULL,
	[c1320] [int] NULL,
	[c1321] [int] NULL,
	[c1322] [int] NULL,
	[c1323] [int] NULL,
	[c1324] [int] NULL,
	[c1325] [int] NULL,
	[c1326] [int] NULL,
	[c1327] [int] NULL,
	[c1328] [int] NULL,
	[c1329] [int] NULL,
	[c1330] [int] NULL,
	[c1331] [int] NULL,
	[c1332] [int] NULL,
	[c1333] [int] NULL,
	[c1334] [int] NULL,
	[c1335] [int] NULL,
	[c1336] [int] NULL,
	[c1337] [int] NULL,
	[c1338] [int] NULL,
	[c1339] [int] NULL,
	[c1340] [int] NULL,
	[c1341] [int] NULL,
	[c1342] [int] NULL,
	[c1343] [int] NULL,
	[c1344] [int] NULL,
	[c1345] [int] NULL,
	[c1346] [int] NULL,
	[c1347] [int] NULL,
	[c1348] [int] NULL,
	[c1349] [int] NULL,
	[c1350] [int] NULL,
	[c1351] [int] NULL,
	[c1352] [int] NULL,
	[c1353] [int] NULL,
	[c1354] [int] NULL,
	[c1355] [int] NULL,
	[c1356] [int] NULL,
	[c1357] [int] NULL,
	[c1358] [int] NULL,
	[c1359] [int] NULL,
	[c1360] [int] NULL,
	[c1361] [int] NULL,
	[c1362] [int] NULL,
	[c1363] [int] NULL,
	[c1364] [int] NULL,
	[c1365] [int] NULL,
	[c1366] [int] NULL,
	[c1367] [int] NULL,
	[c1368] [int] NULL,
	[c1369] [int] NULL,
	[c1370] [int] NULL,
	[c1371] [int] NULL,
	[c1372] [int] NULL,
	[c1373] [int] NULL,
	[c1374] [int] NULL,
	[c1375] [int] NULL,
	[c1376] [int] NULL,
	[c1377] [int] NULL,
	[c1378] [int] NULL,
	[c1379] [int] NULL,
	[c1380] [int] NULL,
	[c1381] [int] NULL,
	[c1382] [int] NULL,
	[c1383] [int] NULL,
	[c1384] [int] NULL,
	[c1385] [int] NULL,
	[c1386] [int] NULL,
	[c1387] [int] NULL,
	[c1388] [int] NULL,
	[c1389] [int] NULL,
	[c1390] [int] NULL,
	[c1391] [int] NULL,
	[c1392] [int] NULL,
	[c1393] [int] NULL,
	[c1394] [int] NULL,
	[c1395] [int] NULL,
	[c1396] [int] NULL,
	[c1397] [int] NULL,
	[c1398] [int] NULL,
	[c1399] [int] NULL,
	[c1400] [int] NULL,
	[c1401] [int] NULL,
	[c1402] [int] NULL,
	[c1403] [int] NULL,
	[c1404] [int] NULL,
	[c1405] [int] NULL,
	[c1406] [int] NULL,
	[c1407] [int] NULL,
	[c1408] [int] NULL,
	[c1409] [int] NULL,
	[c1410] [int] NULL,
	[c1411] [int] NULL,
	[c1412] [int] NULL,
	[c1413] [int] NULL,
	[c1414] [int] NULL,
	[c1415] [int] NULL,
	[c1416] [int] NULL,
	[c1417] [int] NULL,
	[c1418] [int] NULL,
	[c1419] [int] NULL,
	[c1420] [int] NULL,
	[c1421] [int] NULL,
	[c1422] [int] NULL,
	[c1423] [int] NULL,
	[c1424] [int] NULL,
	[c1425] [int] NULL,
	[c1426] [int] NULL,
	[c1427] [int] NULL,
	[c1428] [int] NULL,
	[c1429] [int] NULL,
	[c1430] [int] NULL,
	[c1431] [int] NULL,
	[c1432] [int] NULL,
	[c1433] [int] NULL,
	[c1434] [int] NULL,
	[c1435] [int] NULL,
	[c1436] [int] NULL,
	[c1437] [int] NULL,
	[c1438] [int] NULL,
	[c1439] [int] NULL,
	[c1440] [int] NULL,
	[c1441] [int] NULL,
	[c1442] [int] NULL,
	[c1443] [int] NULL,
	[c1444] [int] NULL,
	[c1445] [int] NULL,
	[c1446] [int] NULL,
	[c1447] [int] NULL,
	[c1448] [int] NULL,
	[c1449] [int] NULL,
	[c1450] [int] NULL,
	[c1451] [int] NULL,
	[c1452] [int] NULL,
	[c1453] [int] NULL,
	[c1454] [int] NULL,
	[c1455] [int] NULL,
	[c1456] [int] NULL,
	[c1457] [int] NULL,
	[c1458] [int] NULL,
	[c1459] [int] NULL,
	[c1460] [int] NULL,
	[c1461] [int] NULL,
	[c1462] [int] NULL,
	[c1463] [int] NULL,
	[c1464] [int] NULL,
	[c1465] [int] NULL,
	[c1466] [int] NULL,
	[c1467] [int] NULL,
	[c1468] [int] NULL,
	[c1469] [int] NULL,
	[c1470] [int] NULL,
	[c1471] [int] NULL,
	[c1472] [int] NULL,
	[c1473] [int] NULL,
	[c1474] [int] NULL,
	[c1475] [int] NULL,
	[c1476] [int] NULL,
	[c1477] [int] NULL,
	[c1478] [int] NULL,
	[c1479] [int] NULL,
	[c1480] [int] NULL,
	[c1481] [int] NULL,
	[c1482] [int] NULL,
	[c1483] [int] NULL,
	[c1484] [int] NULL,
	[c1485] [int] NULL,
	[c1486] [int] NULL,
	[c1487] [int] NULL,
	[c1488] [int] NULL,
	[c1489] [int] NULL,
	[c1490] [int] NULL,
	[c1491] [int] NULL,
	[c1492] [int] NULL,
	[c1493] [int] NULL,
	[c1494] [int] NULL,
	[c1495] [int] NULL,
	[c1496] [int] NULL,
	[c1497] [int] NULL,
	[c1498] [int] NULL,
	[c1499] [int] NULL,
	[c1500] [int] NULL,
	[c1501] [int] NULL,
	[c1502] [int] NULL,
	[c1503] [int] NULL,
	[c1504] [int] NULL,
	[c1505] [int] NULL,
	[c1506] [int] NULL,
	[c1507] [int] NULL,
	[c1508] [int] NULL,
	[c1509] [int] NULL,
	[c1510] [int] NULL,
	[c1511] [int] NULL,
	[c1512] [int] NULL,
	[c1513] [int] NULL,
	[c1514] [int] NULL,
	[c1515] [int] NULL,
	[c1516] [int] NULL,
	[c1517] [int] NULL,
	[c1518] [int] NULL,
	[c1519] [int] NULL,
	[c1520] [int] NULL,
	[c1521] [int] NULL,
	[c1522] [int] NULL,
	[c1523] [int] NULL,
	[c1524] [int] NULL,
	[c1525] [int] NULL,
	[c1526] [int] NULL,
	[c1527] [int] NULL,
	[c1528] [int] NULL,
	[c1529] [int] NULL,
	[c1530] [int] NULL,
	[c1531] [int] NULL,
	[c1532] [int] NULL,
	[c1533] [int] NULL,
	[c1534] [int] NULL,
	[c1535] [int] NULL,
	[c1536] [int] NULL,
	[c1537] [int] NULL,
	[c1538] [int] NULL,
	[c1539] [int] NULL,
	[c1540] [int] NULL,
	[c1541] [int] NULL,
	[c1542] [int] NULL,
	[c1543] [int] NULL,
	[c1544] [int] NULL,
	[c1545] [int] NULL,
	[c1546] [int] NULL,
	[c1547] [int] NULL,
	[c1548] [int] NULL,
	[c1549] [int] NULL,
	[c1550] [int] NULL,
	[c1551] [int] NULL,
	[c1552] [int] NULL,
	[c1553] [int] NULL,
	[c1554] [int] NULL,
	[c1555] [int] NULL,
	[c1556] [int] NULL,
	[c1557] [int] NULL,
	[c1558] [int] NULL,
	[c1559] [int] NULL,
	[c1560] [int] NULL,
	[c1561] [int] NULL,
	[c1562] [int] NULL,
	[c1563] [int] NULL,
	[c1564] [int] NULL,
	[c1565] [int] NULL,
	[c1566] [int] NULL,
	[c1567] [int] NULL,
	[c1568] [int] NULL,
	[c1569] [int] NULL,
	[c1570] [int] NULL,
	[c1571] [int] NULL,
	[c1572] [int] NULL,
	[c1573] [int] NULL,
	[c1574] [int] NULL,
	[c1575] [int] NULL,
	[c1576] [int] NULL,
	[c1577] [int] NULL,
	[c1578] [int] NULL,
	[c1579] [int] NULL,
	[c1580] [int] NULL,
	[c1581] [int] NULL,
	[c1582] [int] NULL,
	[c1583] [int] NULL,
	[c1584] [int] NULL,
	[c1585] [int] NULL,
	[c1586] [int] NULL,
	[c1587] [int] NULL,
	[c1588] [int] NULL,
	[c1589] [int] NULL,
	[c1590] [int] NULL,
	[c1591] [int] NULL,
	[c1592] [int] NULL,
	[c1593] [int] NULL,
	[c1594] [int] NULL,
	[c1595] [int] NULL,
	[c1596] [int] NULL,
	[c1597] [int] NULL,
	[c1598] [int] NULL,
	[c1599] [int] NULL,
	[c1600] [int] NULL,
	[c1601] [int] NULL,
	[c1602] [int] NULL,
	[c1603] [int] NULL,
	[c1604] [int] NULL,
	[c1605] [int] NULL,
	[c1606] [int] NULL,
	[c1607] [int] NULL,
	[c1608] [int] NULL,
	[c1609] [int] NULL,
	[c1610] [int] NULL,
	[c1611] [int] NULL,
	[c1612] [int] NULL,
	[c1613] [int] NULL,
	[c1614] [int] NULL,
	[c1615] [int] NULL,
	[c1616] [int] NULL,
	[c1617] [int] NULL,
	[c1618] [int] NULL,
	[c1619] [int] NULL,
	[c1620] [int] NULL,
	[c1621] [int] NULL,
	[c1622] [int] NULL,
	[c1623] [int] NULL,
	[c1624] [int] NULL,
	[c1625] [int] NULL,
	[c1626] [int] NULL,
	[c1627] [int] NULL,
	[c1628] [int] NULL,
	[c1629] [int] NULL,
	[c1630] [int] NULL,
	[c1631] [int] NULL,
	[c1632] [int] NULL,
	[c1633] [int] NULL,
	[c1634] [int] NULL,
	[c1635] [int] NULL,
	[c1636] [int] NULL,
	[c1637] [int] NULL,
	[c1638] [int] NULL,
	[c1639] [int] NULL,
	[c1640] [int] NULL,
	[c1641] [int] NULL,
	[c1642] [int] NULL,
	[c1643] [int] NULL,
	[c1644] [int] NULL,
	[c1645] [int] NULL,
	[c1646] [int] NULL,
	[c1647] [int] NULL,
	[c1648] [int] NULL,
	[c1649] [int] NULL,
	[c1650] [int] NULL,
	[c1651] [int] NULL,
	[c1652] [int] NULL,
	[c1653] [int] NULL,
	[c1654] [int] NULL,
	[c1655] [int] NULL,
	[c1656] [int] NULL,
	[c1657] [int] NULL,
	[c1658] [int] NULL,
	[c1659] [int] NULL,
	[c1660] [int] NULL,
	[c1661] [int] NULL,
	[c1662] [int] NULL,
	[c1663] [int] NULL,
	[c1664] [int] NULL,
	[c1665] [int] NULL,
	[c1666] [int] NULL,
	[c1667] [int] NULL,
	[c1668] [int] NULL,
	[c1669] [int] NULL,
	[c1670] [int] NULL,
	[c1671] [int] NULL,
	[c1672] [int] NULL,
	[c1673] [int] NULL,
	[c1674] [int] NULL,
	[c1675] [int] NULL,
	[c1676] [int] NULL,
	[c1677] [int] NULL,
	[c1678] [int] NULL,
	[c1679] [int] NULL,
	[c1680] [int] NULL,
	[c1681] [int] NULL,
	[c1682] [int] NULL,
	[c1683] [int] NULL,
	[c1684] [int] NULL,
	[c1685] [int] NULL,
	[c1686] [int] NULL,
	[c1687] [int] NULL,
	[c1688] [int] NULL,
	[c1689] [int] NULL,
	[c1690] [int] NULL,
	[c1691] [int] NULL,
	[c1692] [int] NULL,
	[c1693] [int] NULL,
	[c1694] [int] NULL,
	[c1695] [int] NULL,
	[c1696] [int] NULL,
	[c1697] [int] NULL,
	[c1698] [int] NULL,
	[c1699] [int] NULL,
	[c1700] [int] NULL,
	[c1701] [int] NULL,
	[c1702] [int] NULL,
	[c1703] [int] NULL,
	[c1704] [int] NULL,
	[c1705] [int] NULL,
	[c1706] [int] NULL,
	[c1707] [int] NULL,
	[c1708] [int] NULL,
	[c1709] [int] NULL,
	[c1710] [int] NULL,
	[c1711] [int] NULL,
	[c1712] [int] NULL,
	[c1713] [int] NULL,
	[c1714] [int] NULL,
	[c1715] [int] NULL,
	[c1716] [int] NULL,
	[c1717] [int] NULL,
	[c1718] [int] NULL,
	[c1719] [int] NULL,
	[c1720] [int] NULL,
	[c1721] [int] NULL,
	[c1722] [int] NULL,
	[c1723] [int] NULL,
	[c1724] [int] NULL,
	[c1725] [int] NULL,
	[c1726] [int] NULL,
	[c1727] [int] NULL,
	[c1728] [int] NULL,
	[c1729] [int] NULL,
	[c1730] [int] NULL,
	[c1731] [int] NULL,
	[c1732] [int] NULL,
	[c1733] [int] NULL,
	[c1734] [int] NULL,
	[c1735] [int] NULL,
	[c1736] [int] NULL,
	[c1737] [int] NULL,
	[c1738] [int] NULL,
	[c1739] [int] NULL,
	[c1740] [int] NULL,
	[c1741] [int] NULL,
	[c1742] [int] NULL,
	[c1743] [int] NULL,
	[c1744] [int] NULL,
	[c1745] [int] NULL,
	[c1746] [int] NULL,
	[c1747] [int] NULL,
	[c1748] [int] NULL,
	[c1749] [int] NULL,
	[c1750] [int] NULL,
	[c1751] [int] NULL,
	[c1752] [int] NULL,
	[c1753] [int] NULL,
	[c1754] [int] NULL,
	[c1755] [int] NULL,
	[c1756] [int] NULL,
	[c1757] [int] NULL,
	[c1758] [int] NULL,
	[c1759] [int] NULL,
	[c1760] [int] NULL,
	[c1761] [int] NULL,
	[c1762] [int] NULL,
	[c1763] [int] NULL,
	[c1764] [int] NULL,
	[c1765] [int] NULL,
	[c1766] [int] NULL,
	[c1767] [int] NULL,
	[c1768] [int] NULL,
	[c1769] [int] NULL,
	[c1770] [int] NULL,
	[c1771] [int] NULL,
	[c1772] [int] NULL,
	[c1773] [int] NULL,
	[c1774] [int] NULL,
	[c1775] [int] NULL,
	[c1776] [int] NULL,
	[c1777] [int] NULL,
	[c1778] [int] NULL,
	[c1779] [int] NULL,
	[c1780] [int] NULL,
	[c1781] [int] NULL,
	[c1782] [int] NULL,
	[c1783] [int] NULL,
	[c1784] [int] NULL,
	[c1785] [int] NULL,
	[c1786] [int] NULL,
	[c1787] [int] NULL,
	[c1788] [int] NULL,
	[c1789] [int] NULL,
	[c1790] [int] NULL,
	[c1791] [int] NULL,
	[c1792] [int] NULL,
	[c1793] [int] NULL,
	[c1794] [int] NULL,
	[c1795] [int] NULL,
	[c1796] [int] NULL,
	[c1797] [int] NULL,
	[c1798] [int] NULL,
	[c1799] [int] NULL,
	[c1800] [int] NULL,
	[c1801] [int] NULL,
	[c1802] [int] NULL,
	[c1803] [int] NULL,
	[c1804] [int] NULL,
	[c1805] [int] NULL,
	[c1806] [int] NULL,
	[c1807] [int] NULL,
	[c1808] [int] NULL,
	[c1809] [int] NULL,
	[c1810] [int] NULL,
	[c1811] [int] NULL,
	[c1812] [int] NULL,
	[c1813] [int] NULL,
	[c1814] [int] NULL,
	[c1815] [int] NULL,
	[c1816] [int] NULL,
	[c1817] [int] NULL,
	[c1818] [int] NULL,
	[c1819] [int] NULL,
	[c1820] [int] NULL,
	[c1821] [int] NULL,
	[c1822] [int] NULL,
	[c1823] [int] NULL,
	[c1824] [int] NULL,
	[c1825] [int] NULL,
	[c1826] [int] NULL,
	[c1827] [int] NULL,
	[c1828] [int] NULL,
	[c1829] [int] NULL,
	[c1830] [int] NULL,
	[c1831] [int] NULL,
	[c1832] [int] NULL,
	[c1833] [int] NULL,
	[c1834] [int] NULL,
	[c1835] [int] NULL,
	[c1836] [int] NULL,
	[c1837] [int] NULL,
	[c1838] [int] NULL,
	[c1839] [int] NULL,
	[c1840] [int] NULL,
	[c1841] [int] NULL,
	[c1842] [int] NULL,
	[c1843] [int] NULL,
	[c1844] [int] NULL,
	[c1845] [int] NULL,
	[c1846] [int] NULL,
	[c1847] [int] NULL,
	[c1848] [int] NULL,
	[c1849] [int] NULL,
	[c1850] [int] NULL,
	[c1851] [int] NULL,
	[c1852] [int] NULL,
	[c1853] [int] NULL,
	[c1854] [int] NULL,
	[c1855] [int] NULL,
	[c1856] [int] NULL,
	[c1857] [int] NULL,
	[c1858] [int] NULL,
	[c1859] [int] NULL,
	[c1860] [int] NULL,
	[c1861] [int] NULL,
 CONSTRAINT [PK_Badges01] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Badges02]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Badges02](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[c1862] [int] NULL,
	[c1863] [int] NULL,
	[c1864] [int] NULL,
	[c1865] [int] NULL,
	[c1866] [int] NULL,
	[c1867] [int] NULL,
	[c1868] [int] NULL,
	[c1869] [int] NULL,
	[c1870] [int] NULL,
	[c1871] [int] NULL,
	[c1872] [int] NULL,
	[c1873] [int] NULL,
	[c1874] [int] NULL,
	[c1875] [int] NULL,
	[c1876] [int] NULL,
	[c1877] [int] NULL,
	[c1878] [int] NULL,
	[c1879] [int] NULL,
	[c1880] [int] NULL,
	[c1881] [int] NULL,
	[c1882] [int] NULL,
	[c1883] [int] NULL,
	[c1884] [int] NULL,
	[c1885] [int] NULL,
	[c1886] [int] NULL,
	[c1887] [int] NULL,
	[c1888] [int] NULL,
	[c1889] [int] NULL,
	[c1890] [int] NULL,
	[c1891] [int] NULL,
	[c1892] [int] NULL,
	[c1893] [int] NULL,
	[c1894] [int] NULL,
	[c1895] [int] NULL,
	[c1896] [int] NULL,
	[c1897] [int] NULL,
	[c1898] [int] NULL,
	[c1899] [int] NULL,
	[c1900] [int] NULL,
	[c1901] [int] NULL,
	[c1902] [int] NULL,
	[c1903] [int] NULL,
	[c1904] [int] NULL,
	[c1905] [int] NULL,
	[c1906] [int] NULL,
	[c1907] [int] NULL,
	[c1908] [int] NULL,
	[c1909] [int] NULL,
	[c1910] [int] NULL,
	[c1911] [int] NULL,
	[c1912] [int] NULL,
	[c1913] [int] NULL,
	[c1914] [int] NULL,
	[c1915] [int] NULL,
	[c1916] [int] NULL,
	[c1917] [int] NULL,
	[c1918] [int] NULL,
	[c1919] [int] NULL,
	[c1920] [int] NULL,
	[c1921] [int] NULL,
	[c1922] [int] NULL,
	[c1923] [int] NULL,
	[c1924] [int] NULL,
	[c1925] [int] NULL,
	[c1926] [int] NULL,
	[c1927] [int] NULL,
	[c1928] [int] NULL,
	[c1929] [int] NULL,
	[c1930] [int] NULL,
	[c1931] [int] NULL,
	[c1932] [int] NULL,
	[c1933] [int] NULL,
	[c1934] [int] NULL,
	[c1935] [int] NULL,
	[c1936] [int] NULL,
	[c1937] [int] NULL,
	[c1938] [int] NULL,
	[c1939] [int] NULL,
	[c1940] [int] NULL,
	[c1941] [int] NULL,
	[c1942] [int] NULL,
	[c1943] [int] NULL,
	[c1944] [int] NULL,
	[c1945] [int] NULL,
	[c1946] [int] NULL,
	[c1947] [int] NULL,
	[c1948] [int] NULL,
	[c1949] [int] NULL,
	[c1950] [int] NULL,
	[c1951] [int] NULL,
	[c1952] [int] NULL,
	[c1953] [int] NULL,
	[c1954] [int] NULL,
	[c1955] [int] NULL,
	[c1956] [int] NULL,
	[c1957] [int] NULL,
	[c1958] [int] NULL,
	[c1959] [int] NULL,
	[c1960] [int] NULL,
	[c1961] [int] NULL,
	[c1962] [int] NULL,
	[c1963] [int] NULL,
	[c1964] [int] NULL,
	[c1965] [int] NULL,
	[c1966] [int] NULL,
	[c1967] [int] NULL,
	[c1968] [int] NULL,
	[c1969] [int] NULL,
	[c1970] [int] NULL,
	[c1971] [int] NULL,
	[c1972] [int] NULL,
	[c1973] [int] NULL,
	[c1974] [int] NULL,
	[c1975] [int] NULL,
	[c1976] [int] NULL,
	[c1977] [int] NULL,
	[c1978] [int] NULL,
	[c1979] [int] NULL,
	[c1980] [int] NULL,
	[c1981] [int] NULL,
	[c1982] [int] NULL,
	[c1983] [int] NULL,
	[c1984] [int] NULL,
	[c1985] [int] NULL,
	[c1986] [int] NULL,
	[c1987] [int] NULL,
	[c1988] [int] NULL,
	[c1989] [int] NULL,
	[c1990] [int] NULL,
	[c1991] [int] NULL,
	[c1992] [int] NULL,
	[c1993] [int] NULL,
	[c1994] [int] NULL,
	[c1995] [int] NULL,
	[c1996] [int] NULL,
	[c1997] [int] NULL,
	[c1998] [int] NULL,
	[c1999] [int] NULL,
	[c2000] [int] NULL,
	[c2001] [int] NULL,
	[c2002] [int] NULL,
	[c2003] [int] NULL,
	[c2004] [int] NULL,
	[c2005] [int] NULL,
	[c2006] [int] NULL,
	[c2007] [int] NULL,
	[c2008] [int] NULL,
	[c2009] [int] NULL,
	[c2010] [int] NULL,
	[c2011] [int] NULL,
	[c2012] [int] NULL,
	[c2013] [int] NULL,
	[c2014] [int] NULL,
	[c2015] [int] NULL,
	[c2016] [int] NULL,
	[c2017] [int] NULL,
	[c2018] [int] NULL,
	[c2019] [int] NULL,
	[c2020] [int] NULL,
	[c2021] [int] NULL,
	[c2022] [int] NULL,
	[c2023] [int] NULL,
	[c2024] [int] NULL,
	[c2025] [int] NULL,
	[c2026] [int] NULL,
	[c2027] [int] NULL,
	[c2028] [int] NULL,
	[c2029] [int] NULL,
	[c2030] [int] NULL,
	[c2031] [int] NULL,
	[c2032] [int] NULL,
	[c2033] [int] NULL,
	[c2034] [int] NULL,
	[c2035] [int] NULL,
	[c2036] [int] NULL,
	[c2037] [int] NULL,
	[c2038] [int] NULL,
	[c2039] [int] NULL,
	[c2040] [int] NULL,
	[c2041] [int] NULL,
	[c2042] [int] NULL,
	[c2043] [int] NULL,
	[c2044] [int] NULL,
	[c2045] [int] NULL,
	[c2046] [int] NULL,
	[c2047] [int] NULL,
	[c2048] [int] NULL,
	[c2049] [int] NULL,
	[c2050] [int] NULL,
	[c2051] [int] NULL,
	[c2052] [int] NULL,
	[c2053] [int] NULL,
	[c2054] [int] NULL,
	[c2055] [int] NULL,
	[c2056] [int] NULL,
	[c2057] [int] NULL,
	[c2058] [int] NULL,
	[c2059] [int] NULL,
	[c2060] [int] NULL,
	[c2061] [int] NULL,
	[c2062] [int] NULL,
	[c2063] [int] NULL,
	[c2064] [int] NULL,
	[c2065] [int] NULL,
	[c2066] [int] NULL,
	[c2067] [int] NULL,
	[c2068] [int] NULL,
	[c2069] [int] NULL,
	[c2070] [int] NULL,
	[c2071] [int] NULL,
	[c2072] [int] NULL,
	[c2073] [int] NULL,
	[c2074] [int] NULL,
	[c2075] [int] NULL,
	[c2076] [int] NULL,
	[c2077] [int] NULL,
	[c2078] [int] NULL,
	[c2079] [int] NULL,
	[c2080] [int] NULL,
	[c2081] [int] NULL,
	[c2082] [int] NULL,
	[c2083] [int] NULL,
	[c2084] [int] NULL,
	[c2085] [int] NULL,
	[c2086] [int] NULL,
	[c2087] [int] NULL,
	[c2088] [int] NULL,
	[c2089] [int] NULL,
	[c2090] [int] NULL,
	[c2091] [int] NULL,
	[c2092] [int] NULL,
	[c2093] [int] NULL,
	[c2094] [int] NULL,
	[c2095] [int] NULL,
	[c2096] [int] NULL,
	[c2097] [int] NULL,
	[c2098] [int] NULL,
	[c2099] [int] NULL,
	[c2100] [int] NULL,
	[c2101] [int] NULL,
	[c2102] [int] NULL,
	[c2103] [int] NULL,
	[c2104] [int] NULL,
	[c2105] [int] NULL,
	[c2106] [int] NULL,
	[c2107] [int] NULL,
	[c2108] [int] NULL,
	[c2109] [int] NULL,
	[c2110] [int] NULL,
	[c2111] [int] NULL,
	[c2112] [int] NULL,
	[c2113] [int] NULL,
	[c2114] [int] NULL,
	[c2115] [int] NULL,
	[c2116] [int] NULL,
	[c2117] [int] NULL,
	[c2118] [int] NULL,
	[c2119] [int] NULL,
	[c2120] [int] NULL,
	[c2121] [int] NULL,
	[c2122] [int] NULL,
	[c2123] [int] NULL,
	[c2124] [int] NULL,
	[c2125] [int] NULL,
	[c2126] [int] NULL,
	[c2127] [int] NULL,
	[c2128] [int] NULL,
	[c2129] [int] NULL,
	[c2130] [int] NULL,
	[c2131] [int] NULL,
	[c2132] [int] NULL,
	[c2133] [int] NULL,
	[c2134] [int] NULL,
	[c2135] [int] NULL,
	[c2136] [int] NULL,
	[c2137] [int] NULL,
	[c2138] [int] NULL,
	[c2139] [int] NULL,
	[c2140] [int] NULL,
	[c2141] [int] NULL,
	[c2142] [int] NULL,
	[c2143] [int] NULL,
	[c2144] [int] NULL,
	[c2145] [int] NULL,
	[c2146] [int] NULL,
	[c2147] [int] NULL,
	[c2148] [int] NULL,
	[c2149] [int] NULL,
	[c2150] [int] NULL,
	[c2151] [int] NULL,
	[c2152] [int] NULL,
	[c2153] [int] NULL,
	[c2154] [int] NULL,
	[c2155] [int] NULL,
	[c2156] [int] NULL,
	[c2157] [int] NULL,
	[c2158] [int] NULL,
	[c2159] [int] NULL,
	[c2160] [int] NULL,
	[c2161] [int] NULL,
	[c2162] [int] NULL,
	[c2163] [int] NULL,
	[c2164] [int] NULL,
	[c2165] [int] NULL,
	[c2166] [int] NULL,
	[c2167] [int] NULL,
	[c2168] [int] NULL,
	[c2169] [int] NULL,
	[c2170] [int] NULL,
	[c2171] [int] NULL,
	[c2172] [int] NULL,
	[c2173] [int] NULL,
	[c2174] [int] NULL,
	[c2175] [int] NULL,
	[c2176] [int] NULL,
	[c2177] [int] NULL,
	[c2178] [int] NULL,
	[c2179] [int] NULL,
	[c2180] [int] NULL,
	[c2181] [int] NULL,
	[c2182] [int] NULL,
	[c2183] [int] NULL,
	[c2184] [int] NULL,
	[c2185] [int] NULL,
	[c2186] [int] NULL,
	[c2187] [int] NULL,
	[c2188] [int] NULL,
	[c2189] [int] NULL,
	[c2190] [int] NULL,
	[c2191] [int] NULL,
	[c2192] [int] NULL,
	[c2193] [int] NULL,
	[c2194] [int] NULL,
	[c2195] [int] NULL,
	[c2196] [int] NULL,
	[c2197] [int] NULL,
	[c2198] [int] NULL,
	[c2199] [int] NULL,
	[c2200] [int] NULL,
	[c2201] [int] NULL,
	[c2202] [int] NULL,
	[c2203] [int] NULL,
	[c2204] [int] NULL,
	[c2205] [int] NULL,
	[c2206] [int] NULL,
	[c2207] [int] NULL,
	[c2208] [int] NULL,
	[c2209] [int] NULL,
	[c2210] [int] NULL,
	[c2211] [int] NULL,
	[c2212] [int] NULL,
	[c2213] [int] NULL,
	[c2214] [int] NULL,
	[c2215] [int] NULL,
	[c2216] [int] NULL,
	[c2217] [int] NULL,
	[c2218] [int] NULL,
	[c2219] [int] NULL,
	[c2220] [int] NULL,
	[c2221] [int] NULL,
	[c2222] [int] NULL,
	[c2223] [int] NULL,
	[c2224] [int] NULL,
	[c2225] [int] NULL,
	[c2226] [int] NULL,
	[c2227] [int] NULL,
	[c2228] [int] NULL,
	[c2229] [int] NULL,
	[c2230] [int] NULL,
	[c2231] [int] NULL,
	[c2232] [int] NULL,
	[c2233] [int] NULL,
	[c2234] [int] NULL,
	[c2235] [int] NULL,
	[c2236] [int] NULL,
	[c2237] [int] NULL,
	[c2238] [int] NULL,
	[c2239] [int] NULL,
	[c2240] [int] NULL,
	[c2241] [int] NULL,
	[c2242] [int] NULL,
	[c2243] [int] NULL,
	[c2244] [int] NULL,
	[c2245] [int] NULL,
	[c2246] [int] NULL,
	[c2247] [int] NULL,
	[c2248] [int] NULL,
	[c2249] [int] NULL,
	[c2250] [int] NULL,
	[c2251] [int] NULL,
	[c2252] [int] NULL,
	[c2253] [int] NULL,
	[c2254] [int] NULL,
	[c2255] [int] NULL,
	[c2256] [int] NULL,
	[c2257] [int] NULL,
	[c2258] [int] NULL,
	[c2259] [int] NULL,
	[c2260] [int] NULL,
	[c2261] [int] NULL,
	[c2262] [int] NULL,
	[c2263] [int] NULL,
	[c2264] [int] NULL,
	[c2265] [int] NULL,
	[c2266] [int] NULL,
	[c2267] [int] NULL,
	[c2268] [int] NULL,
	[c2269] [int] NULL,
	[c2270] [int] NULL,
	[c2271] [int] NULL,
	[c2272] [int] NULL,
	[c2273] [int] NULL,
	[c2274] [int] NULL,
	[c2275] [int] NULL,
	[c2276] [int] NULL,
	[c2277] [int] NULL,
	[c2278] [int] NULL,
	[c2279] [int] NULL,
	[c2280] [int] NULL,
	[c2281] [int] NULL,
	[c2282] [int] NULL,
	[c2283] [int] NULL,
	[c2284] [int] NULL,
	[c2285] [int] NULL,
	[c2286] [int] NULL,
	[c2287] [int] NULL,
	[c2288] [int] NULL,
	[c2289] [int] NULL,
	[c2290] [int] NULL,
	[c2291] [int] NULL,
	[c2292] [int] NULL,
	[c2293] [int] NULL,
	[c2294] [int] NULL,
	[c2295] [int] NULL,
	[c2296] [int] NULL,
	[c2297] [int] NULL,
	[c2298] [int] NULL,
	[c2299] [int] NULL,
	[c2300] [int] NULL,
	[c2301] [int] NULL,
	[c2302] [int] NULL,
	[c2303] [int] NULL,
	[c2304] [int] NULL,
	[c2305] [int] NULL,
	[c2306] [int] NULL,
	[c2307] [int] NULL,
	[c2308] [int] NULL,
	[c2309] [int] NULL,
	[c2310] [int] NULL,
	[c2311] [int] NULL,
	[c2312] [int] NULL,
	[c2313] [int] NULL,
	[c2314] [int] NULL,
	[c2315] [int] NULL,
	[c2316] [int] NULL,
	[c2317] [int] NULL,
	[c2318] [int] NULL,
	[c2319] [int] NULL,
	[c2320] [int] NULL,
	[c2321] [int] NULL,
	[c2322] [int] NULL,
	[c2323] [int] NULL,
	[c2324] [int] NULL,
	[c2325] [int] NULL,
	[c2326] [int] NULL,
	[c2327] [int] NULL,
	[c2328] [int] NULL,
	[c2329] [int] NULL,
	[c2330] [int] NULL,
	[c2331] [int] NULL,
	[c2332] [int] NULL,
	[c2333] [int] NULL,
	[c2334] [int] NULL,
	[c2335] [int] NULL,
	[c2336] [int] NULL,
	[c2337] [int] NULL,
	[c2338] [int] NULL,
	[c2339] [int] NULL,
	[c2340] [int] NULL,
	[c2341] [int] NULL,
	[c2342] [int] NULL,
	[c2343] [int] NULL,
	[c2344] [int] NULL,
	[c2345] [int] NULL,
	[c2346] [int] NULL,
	[c2347] [int] NULL,
	[c2348] [int] NULL,
	[c2349] [int] NULL,
	[c2350] [int] NULL,
	[c2351] [int] NULL,
	[c2352] [int] NULL,
	[c2353] [int] NULL,
	[c2354] [int] NULL,
	[c2355] [int] NULL,
	[c2356] [int] NULL,
	[c2357] [int] NULL,
	[c2358] [int] NULL,
	[c2359] [int] NULL,
	[c2360] [int] NULL,
	[c2361] [int] NULL,
	[c2362] [int] NULL,
	[c2363] [int] NULL,
	[c2364] [int] NULL,
	[c2365] [int] NULL,
	[c2366] [int] NULL,
	[c2367] [int] NULL,
	[c2368] [int] NULL,
	[c2369] [int] NULL,
	[c2370] [int] NULL,
	[c2371] [int] NULL,
	[c2372] [int] NULL,
	[c2373] [int] NULL,
	[c2374] [int] NULL,
	[c2375] [int] NULL,
	[c2376] [int] NULL,
	[c2377] [int] NULL,
	[c2378] [int] NULL,
	[c2379] [int] NULL,
	[c2380] [int] NULL,
	[c2381] [int] NULL,
	[c2382] [int] NULL,
	[c2383] [int] NULL,
	[c2384] [int] NULL,
	[c2385] [int] NULL,
	[c2386] [int] NULL,
	[c2387] [int] NULL,
	[c2388] [int] NULL,
	[c2389] [int] NULL,
	[c2390] [int] NULL,
	[c2391] [int] NULL,
	[c2392] [int] NULL,
	[c2393] [int] NULL,
	[c2394] [int] NULL,
	[c2395] [int] NULL,
	[c2396] [int] NULL,
	[c2397] [int] NULL,
	[c2398] [int] NULL,
	[c2399] [int] NULL,
	[c2400] [int] NULL,
	[c2401] [int] NULL,
	[c2402] [int] NULL,
	[c2403] [int] NULL,
	[c2404] [int] NULL,
	[c2405] [int] NULL,
	[c2406] [int] NULL,
	[c2407] [int] NULL,
	[c2408] [int] NULL,
	[c2409] [int] NULL,
	[c2410] [int] NULL,
	[c2411] [int] NULL,
	[c2412] [int] NULL,
	[c2413] [int] NULL,
	[c2414] [int] NULL,
	[c2415] [int] NULL,
	[c2416] [int] NULL,
	[c2417] [int] NULL,
	[c2418] [int] NULL,
	[c2419] [int] NULL,
	[c2420] [int] NULL,
	[c2421] [int] NULL,
	[c2422] [int] NULL,
	[c2423] [int] NULL,
	[c2424] [int] NULL,
	[c2425] [int] NULL,
	[c2426] [int] NULL,
	[c2427] [int] NULL,
	[c2428] [int] NULL,
	[c2429] [int] NULL,
	[c2430] [int] NULL,
	[c2431] [int] NULL,
	[c2432] [int] NULL,
	[c2433] [int] NULL,
	[c2434] [int] NULL,
	[c2435] [int] NULL,
	[c2436] [int] NULL,
	[c2437] [int] NULL,
	[c2438] [int] NULL,
	[c2439] [int] NULL,
	[c2440] [int] NULL,
	[c2441] [int] NULL,
	[c2442] [int] NULL,
	[c2443] [int] NULL,
	[c2444] [int] NULL,
	[c2445] [int] NULL,
	[c2446] [int] NULL,
	[c2447] [int] NULL,
	[c2448] [int] NULL,
	[c2449] [int] NULL,
	[c2450] [int] NULL,
	[c2451] [int] NULL,
	[c2452] [int] NULL,
	[c2453] [int] NULL,
	[c2454] [int] NULL,
	[c2455] [int] NULL,
	[c2456] [int] NULL,
	[c2457] [int] NULL,
	[c2458] [int] NULL,
	[c2459] [int] NULL,
	[c2460] [int] NULL,
	[c2461] [int] NULL,
	[c2462] [int] NULL,
	[c2463] [int] NULL,
	[c2464] [int] NULL,
	[c2465] [int] NULL,
	[c2466] [int] NULL,
	[c2467] [int] NULL,
	[c2468] [int] NULL,
	[c2469] [int] NULL,
	[c2470] [int] NULL,
	[c2471] [int] NULL,
	[c2472] [int] NULL,
	[c2473] [int] NULL,
	[c2474] [int] NULL,
	[c2475] [int] NULL,
	[c2476] [int] NULL,
	[c2477] [int] NULL,
	[c2478] [int] NULL,
	[c2479] [int] NULL,
	[c2480] [int] NULL,
	[c2481] [int] NULL,
	[c2482] [int] NULL,
	[c2483] [int] NULL,
	[c2484] [int] NULL,
	[c2485] [int] NULL,
	[c2486] [int] NULL,
	[c2487] [int] NULL,
	[c2488] [int] NULL,
	[c2489] [int] NULL,
	[c2490] [int] NULL,
	[c2491] [int] NULL,
	[c2492] [int] NULL,
	[c2493] [int] NULL,
	[c2494] [int] NULL,
	[c2495] [int] NULL,
	[c2496] [int] NULL,
	[c2497] [int] NULL,
	[c2498] [int] NULL,
	[c2499] [int] NULL,
	[c2500] [int] NULL,
	[c2501] [int] NULL,
	[c2502] [int] NULL,
	[c2503] [int] NULL,
	[c2504] [int] NULL,
	[c2505] [int] NULL,
	[c2506] [int] NULL,
	[c2507] [int] NULL,
	[c2508] [int] NULL,
	[c2509] [int] NULL,
	[c2510] [int] NULL,
	[c2511] [int] NULL,
	[c2512] [int] NULL,
	[c2513] [int] NULL,
	[c2514] [int] NULL,
	[c2515] [int] NULL,
	[c2516] [int] NULL,
	[c2517] [int] NULL,
	[c2518] [int] NULL,
	[c2519] [int] NULL,
	[c2520] [int] NULL,
	[c2521] [int] NULL,
	[c2522] [int] NULL,
	[c2523] [int] NULL,
	[c2524] [int] NULL,
	[c2525] [int] NULL,
	[c2526] [int] NULL,
	[c2527] [int] NULL,
	[c2528] [int] NULL,
	[c2529] [int] NULL,
	[c2530] [int] NULL,
	[c2531] [int] NULL,
	[c2532] [int] NULL,
	[c2533] [int] NULL,
	[c2534] [int] NULL,
	[c2535] [int] NULL,
	[c2536] [int] NULL,
	[c2537] [int] NULL,
	[c2538] [int] NULL,
	[c2539] [int] NULL,
	[c2540] [int] NULL,
	[c2541] [int] NULL,
	[c2542] [int] NULL,
	[c2543] [int] NULL,
	[c2544] [int] NULL,
	[c2545] [int] NULL,
	[c2546] [int] NULL,
	[c2547] [int] NULL,
	[c2548] [int] NULL,
	[c2549] [int] NULL,
	[c2550] [int] NULL,
	[c2551] [int] NULL,
	[c2552] [int] NULL,
	[c2553] [int] NULL,
	[c2554] [int] NULL,
	[c2555] [int] NULL,
	[c2556] [int] NULL,
	[c2557] [int] NULL,
	[c2558] [int] NULL,
	[c2559] [int] NULL,
	[c2560] [int] NULL,
	[c2561] [int] NULL,
	[c2562] [int] NULL,
	[c2563] [int] NULL,
	[c2564] [int] NULL,
	[c2565] [int] NULL,
	[c2566] [int] NULL,
	[c2567] [int] NULL,
	[c2568] [int] NULL,
	[c2569] [int] NULL,
	[c2570] [int] NULL,
	[c2571] [int] NULL,
	[c2572] [int] NULL,
	[c2573] [int] NULL,
	[c2574] [int] NULL,
	[c2575] [int] NULL,
	[c2576] [int] NULL,
	[c2577] [int] NULL,
	[c2578] [int] NULL,
	[c2579] [int] NULL,
	[c2580] [int] NULL,
	[c2581] [int] NULL,
	[c2582] [int] NULL,
	[c2583] [int] NULL,
	[c2584] [int] NULL,
	[c2585] [int] NULL,
	[c2586] [int] NULL,
	[c2587] [int] NULL,
	[c2588] [int] NULL,
	[c2589] [int] NULL,
	[c2590] [int] NULL,
	[c2591] [int] NULL,
	[c2592] [int] NULL,
	[c2593] [int] NULL,
	[c2594] [int] NULL,
	[c2595] [int] NULL,
	[c2596] [int] NULL,
	[c2597] [int] NULL,
	[c2598] [int] NULL,
	[c2599] [int] NULL,
	[c2600] [int] NULL,
	[c2601] [int] NULL,
	[c2602] [int] NULL,
	[c2603] [int] NULL,
	[c2604] [int] NULL,
	[c2605] [int] NULL,
	[c2606] [int] NULL,
	[c2607] [int] NULL,
	[c2608] [int] NULL,
	[c2609] [int] NULL,
	[c2610] [int] NULL,
	[c2611] [int] NULL,
	[c2612] [int] NULL,
	[c2613] [int] NULL,
	[c2614] [int] NULL,
	[c2615] [int] NULL,
	[c2616] [int] NULL,
	[c2617] [int] NULL,
	[c2618] [int] NULL,
	[c2619] [int] NULL,
	[c2620] [int] NULL,
	[c2621] [int] NULL,
	[c2622] [int] NULL,
	[c2623] [int] NULL,
	[c2624] [int] NULL,
	[c2625] [int] NULL,
	[c2626] [int] NULL,
	[c2627] [int] NULL,
	[c2628] [int] NULL,
	[c2629] [int] NULL,
	[c2630] [int] NULL,
	[c2631] [int] NULL,
	[c2632] [int] NULL,
	[c2633] [int] NULL,
	[c2634] [int] NULL,
	[c2635] [int] NULL,
	[c2636] [int] NULL,
	[c2637] [int] NULL,
	[c2638] [int] NULL,
	[c2639] [int] NULL,
	[c2640] [int] NULL,
	[c2641] [int] NULL,
	[c2642] [int] NULL,
	[c2643] [int] NULL,
	[c2644] [int] NULL,
	[c2645] [int] NULL,
	[c2646] [int] NULL,
	[c2647] [int] NULL,
	[c2648] [int] NULL,
	[c2649] [int] NULL,
	[c2650] [int] NULL,
	[c2651] [int] NULL,
	[c2652] [int] NULL,
	[c2653] [int] NULL,
	[c2654] [int] NULL,
	[c2655] [int] NULL,
	[c2656] [int] NULL,
	[c2657] [int] NULL,
	[c2658] [int] NULL,
	[c2659] [int] NULL,
	[c2660] [int] NULL,
	[c2661] [int] NULL,
	[c2662] [int] NULL,
	[c2663] [int] NULL,
	[c2664] [int] NULL,
	[c2665] [int] NULL,
	[c2666] [int] NULL,
	[c2667] [int] NULL,
	[c2668] [int] NULL,
	[c2669] [int] NULL,
	[c2670] [int] NULL,
	[c2671] [int] NULL,
	[c2672] [int] NULL,
	[c2673] [int] NULL,
	[c2674] [int] NULL,
	[c2675] [int] NULL,
	[c2676] [int] NULL,
	[c2677] [int] NULL,
	[c2678] [int] NULL,
	[c2679] [int] NULL,
	[c2680] [int] NULL,
	[c2681] [int] NULL,
	[c2682] [int] NULL,
	[c2683] [int] NULL,
	[c2684] [int] NULL,
	[c2685] [int] NULL,
	[c2686] [int] NULL,
	[c2687] [int] NULL,
	[c2688] [int] NULL,
	[c2689] [int] NULL,
	[c2690] [int] NULL,
	[c2691] [int] NULL,
	[c2692] [int] NULL,
	[c2693] [int] NULL,
	[c2694] [int] NULL,
	[c2695] [int] NULL,
	[c2696] [int] NULL,
	[c2697] [int] NULL,
	[c2698] [int] NULL,
	[c2699] [int] NULL,
	[c2700] [int] NULL,
	[c2701] [int] NULL,
	[c2702] [int] NULL,
	[c2703] [int] NULL,
	[c2704] [int] NULL,
	[c2705] [int] NULL,
	[c2706] [int] NULL,
	[c2707] [int] NULL,
	[c2708] [int] NULL,
	[c2709] [int] NULL,
	[c2710] [int] NULL,
	[c2711] [int] NULL,
	[c2712] [int] NULL,
	[c2713] [int] NULL,
	[c2714] [int] NULL,
	[c2715] [int] NULL,
	[c2716] [int] NULL,
	[c2717] [int] NULL,
	[c2718] [int] NULL,
	[c2719] [int] NULL,
	[c2720] [int] NULL,
	[c2721] [int] NULL,
	[c2722] [int] NULL,
	[c2723] [int] NULL,
	[c2724] [int] NULL,
	[c2725] [int] NULL,
	[c2726] [int] NULL,
	[c2727] [int] NULL,
	[c2728] [int] NULL,
	[c2729] [int] NULL,
	[c2730] [int] NULL,
	[c2731] [int] NULL,
	[c2732] [int] NULL,
	[c2733] [int] NULL,
	[c2734] [int] NULL,
	[c2735] [int] NULL,
	[c2736] [int] NULL,
	[c2737] [int] NULL,
	[c2738] [int] NULL,
	[c2739] [int] NULL,
	[c2740] [int] NULL,
	[c2741] [int] NULL,
	[c2742] [int] NULL,
	[c2743] [int] NULL,
	[c2744] [int] NULL,
	[c2745] [int] NULL,
	[c2746] [int] NULL,
	[c2747] [int] NULL,
	[c2748] [int] NULL,
	[c2749] [int] NULL,
	[c2750] [int] NULL,
	[c2751] [int] NULL,
	[c2752] [int] NULL,
	[c2753] [int] NULL,
	[c2754] [int] NULL,
	[c2755] [int] NULL,
	[c2756] [int] NULL,
	[c2757] [int] NULL,
	[c2758] [int] NULL,
	[c2759] [int] NULL,
	[c2760] [int] NULL,
	[c2761] [int] NULL,
	[c2762] [int] NULL,
	[c2763] [int] NULL,
	[c2764] [int] NULL,
	[c2765] [int] NULL,
	[c2766] [int] NULL,
	[c2767] [int] NULL,
	[c2768] [int] NULL,
	[c2769] [int] NULL,
	[c2770] [int] NULL,
	[c2771] [int] NULL,
	[c2772] [int] NULL,
	[c2773] [int] NULL,
	[c2774] [int] NULL,
	[c2775] [int] NULL,
	[c2776] [int] NULL,
	[c2777] [int] NULL,
	[c2778] [int] NULL,
	[c2779] [int] NULL,
	[c2780] [int] NULL,
	[c2781] [int] NULL,
	[c2782] [int] NULL,
	[c2783] [int] NULL,
	[c2784] [int] NULL,
	[c2785] [int] NULL,
	[c2786] [int] NULL,
	[c2787] [int] NULL,
	[c2788] [int] NULL,
	[c2789] [int] NULL,
	[c2790] [int] NULL,
	[c2791] [int] NULL,
	[c2792] [int] NULL,
	[c2793] [int] NULL,
	[c2794] [int] NULL,
	[c2795] [int] NULL,
	[c2796] [int] NULL,
	[c2797] [int] NULL,
	[c2798] [int] NULL,
	[c2799] [int] NULL,
	[c2800] [int] NULL,
	[c2801] [int] NULL,
	[c2802] [int] NULL,
	[c2803] [int] NULL,
	[c2804] [int] NULL,
	[c2805] [int] NULL,
	[c2806] [int] NULL,
	[c2807] [int] NULL,
	[c2808] [int] NULL,
	[c2809] [int] NULL,
	[c2810] [int] NULL,
	[c2811] [int] NULL,
	[c2812] [int] NULL,
	[c2813] [int] NULL,
	[c2814] [int] NULL,
	[c2815] [int] NULL,
	[c2816] [int] NULL,
	[c2817] [int] NULL,
	[c2818] [int] NULL,
	[c2819] [int] NULL,
	[c2820] [int] NULL,
	[c2821] [int] NULL,
	[c2822] [int] NULL,
	[c2823] [int] NULL,
	[c2824] [int] NULL,
	[c2825] [int] NULL,
	[c2826] [int] NULL,
	[c2827] [int] NULL,
	[c2828] [int] NULL,
	[c2829] [int] NULL,
	[c2830] [int] NULL,
	[c2831] [int] NULL,
	[c2832] [int] NULL,
	[c2833] [int] NULL,
	[c2834] [int] NULL,
	[c2835] [int] NULL,
	[c2836] [int] NULL,
	[c2837] [int] NULL,
	[c2838] [int] NULL,
	[c2839] [int] NULL,
	[c2840] [int] NULL,
	[c2841] [int] NULL,
	[c2842] [int] NULL,
	[c2843] [int] NULL,
	[c2844] [int] NULL,
	[c2845] [int] NULL,
	[c2846] [int] NULL,
	[c2847] [int] NULL,
	[c2848] [int] NULL,
	[c2849] [int] NULL,
	[c2850] [int] NULL,
 CONSTRAINT [PK_Badges02] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[BadgeStatsAttributes]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[BadgeStatsAttributes](
	[Id] [int] NOT NULL,
	[Name] [varchar](128) NULL,
PRIMARY KEY CLUSTERED 
(
	[Id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[Base]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Base](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[UserId] [int] NULL,
	[SupergroupId] [int] NULL,
	[Data] [text] NULL,
	[ZipData] [varbinary](max) NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[BaseRaids]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BaseRaids](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[AttackerSG] [int] NULL,
	[DefenderSG] [int] NULL,
	[Time] [int] NULL,
	[Length] [int] NULL,
	[CompleteTime] [int] NULL,
	[ScheduledTime] [int] NULL,
	[AttackersWon] [int] NULL,
	[Instant] [int] NULL,
	[MaxParticipants] [int] NULL,
	[ForfeitChecked] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Boosts]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Boosts](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[PowerID] [int] NULL,
	[Idx] [int] NULL,
	[CategoryName] [int] NULL,
	[PowerSetName] [int] NULL,
	[BoostName] [int] NULL,
	[Level] [int] NULL,
	[NumCombines] [int] NULL,
	[Var0] [real] NULL,
	[Var1] [real] NULL,
	[Var2] [real] NULL,
	[Var3] [real] NULL,
	[SlottedPowerupsMask] [int] NULL,
	[PowerupSlotType0] [int] NULL,
	[PowerupSlotId0] [int] NULL,
	[PowerupSlotType1] [int] NULL,
	[PowerupSlotId1] [int] NULL,
	[PowerupSlotType2] [int] NULL,
	[PowerupSlotId2] [int] NULL,
	[PowerupSlotType3] [int] NULL,
	[PowerupSlotId3] [int] NULL,
	[PowerupSlotType4] [int] NULL,
	[PowerupSlotId4] [int] NULL,
	[PowerupSlotType5] [int] NULL,
	[PowerupSlotId5] [int] NULL,
	[PowerupSlotType6] [int] NULL,
	[PowerupSlotId6] [int] NULL,
	[PowerupSlotType7] [int] NULL,
	[PowerupSlotId7] [int] NULL,
	[PowerupSlotType8] [int] NULL,
	[PowerupSlotId8] [int] NULL,
	[PowerupSlotType9] [int] NULL,
	[PowerupSlotId9] [int] NULL,
 CONSTRAINT [PK_Boosts] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[CertificationHistory]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[CertificationHistory](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[claimed] [int] NULL,
	[deleted] [int] NULL,
	[Name] [varchar](max) NULL,
 CONSTRAINT [PK_CertificationHistory] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[ChatChannels]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ChatChannels](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ChannelName] [nvarchar](52) NULL,
	[ChannelOptions] [int] NULL,
	[Color1] [int] NULL,
	[Color2] [int] NULL,
 CONSTRAINT [PK_ChatChannels] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[ChatTabs]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[ChatTabs](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[TabName] [nvarchar](38) NULL,
	[SystemChannels] [int] NULL,
	[UserChannels] [int] NULL,
	[TabOptions] [int] NULL,
	[DefaultChannel] [int] NULL,
	[DefaultType] [int] NULL,
	[SystemChannelsBitField] [varchar](16) NULL,
 CONSTRAINT [PK_ChatTabs] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[ChatWindows]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ChatWindows](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[TabList] [int] NULL,
	[SelectedTab] [int] NULL,
	[SelectedTabBot] [int] NULL,
	[Divider] [real] NULL,
 CONSTRAINT [PK_ChatWindows] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[CombatMonitorStat]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CombatMonitorStat](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[iKey] [int] NULL,
	[iOrder] [int] NULL,
 CONSTRAINT [PK_CombatMonitorStat] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[CompletedOrders]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CompletedOrders](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[OrderId0] [int] NULL,
	[OrderId1] [int] NULL,
	[OrderId2] [int] NULL,
	[OrderId3] [int] NULL,
 CONSTRAINT [PK_CompletedOrders] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Contacts]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Contacts](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
	[TaskIssued] [varchar](64) NULL,
	[StoryArcIssued] [varchar](8) NULL,
	[DialogSeed] [int] NULL,
	[ContactIntroSeed] [int] NULL,
	[ContactPoints] [int] NULL,
	[ContactRelationship] [int] NULL,
	[ContactsIntroduced] [int] NULL,
	[SeenPlayer] [int] NULL,
	[NotifyPlayer] [int] NULL,
	[ItemsBought] [int] NULL,
	[RewardContact] [tinyint] NULL,
	[BrokerHandle] [int] NULL,
 CONSTRAINT [PK_Contacts] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[CostumeParts]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CostumeParts](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Name] [int] NULL,
	[Geom] [int] NULL,
	[Tex1] [int] NULL,
	[Tex2] [int] NULL,
	[DisplayName] [int] NULL,
	[Region] [int] NULL,
	[BodySet] [int] NULL,
	[Color1] [int] NULL,
	[Color2] [int] NULL,
	[CostumeNum] [int] NULL,
	[FxName] [int] NULL,
	[Color3] [int] NULL,
	[Color4] [int] NULL,
 CONSTRAINT [PK_CostumeParts] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[DefeatRecord]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DefeatRecord](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[VictorId] [int] NULL,
	[DefeatTime] [int] NULL,
 CONSTRAINT [PK_DefeatRecord] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[DefenderParticipants]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DefenderParticipants](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[DbId] [int] NULL,
 CONSTRAINT [PK_DefenderParticipants] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Email]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Email](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[Sender] [int] NULL,
	[Subject] [nvarchar](80) NULL,
	[Msg] [nvarchar](3900) NULL,
	[Sent] [datetime] NULL,
	[SenderAuth] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Ents]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Ents](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[TeamupsId] [int] NULL,
	[SupergroupsId] [int] NULL,
	[TaskforcesId] [int] NULL,
	[AuthId] [int] NULL,
	[AuthName] [varchar](64) NULL,
	[Name] [varchar](128) NULL,
	[StaticMapId] [smallint] NULL,
	[MapId] [smallint] NULL,
	[PosX] [real] NULL,
	[PosY] [real] NULL,
	[PosZ] [real] NULL,
	[OrientP] [real] NULL,
	[OrientY] [real] NULL,
	[OrientR] [real] NULL,
	[TotalTime] [int] NULL,
	[LoginCount] [int] NULL,
	[LastActive] [datetime] NULL,
	[AccessLevel] [tinyint] NULL,
	[ChatBanExpire] [datetime] NULL,
	[DbFlags] [int] NULL,
	[Locale] [tinyint] NULL,
	[GurneyMapId] [smallint] NULL,
	[TitleCommon] [nvarchar](64) NULL,
	[TitleOrigin] [nvarchar](64) NULL,
	[MouseSpeed] [real] NULL,
	[TurnSpeed] [real] NULL,
	[TopChatFilter] [int] NULL,
	[BotChatFilter] [int] NULL,
	[ChatSendChannel] [int] NULL,
	[KeyProfile] [nvarchar](64) NULL,
	[KeybindCount] [int] NULL,
	[FriendCount] [int] NULL,
	[Rank] [int] NULL,
	[TimePlayed] [int] NULL,
	[MemberSince] [datetime] NULL,
	[TaskForceMode] [tinyint] NULL,
	[BodyType] [tinyint] NULL,
	[BodyScale] [real] NULL,
	[BoneScale] [real] NULL,
	[ColorSkin] [int] NULL,
	[Motto] [nvarchar](256) NULL,
	[Description] [nvarchar](2048) NULL,
	[CurrentTray] [int] NULL,
	[CurrentAltTray] [int] NULL,
	[ChatDivider] [real] NULL,
	[SpawnTarget] [nvarchar](128) NULL,
	[Class] [int] NULL,
	[Origin] [int] NULL,
	[Level] [int] NULL,
	[ExperiencePoints] [int] NULL,
	[ExperienceDebt] [int] NULL,
	[InfluencePoints] [int] NULL,
	[HitPoints] [real] NULL,
	[Endurance] [real] NULL,
	[ChatFontSize] [tinyint] NULL,
	[UniqueTaskIssued] [varchar](32) NULL,
	[TitleSpecial] [nvarchar](256) NULL,
	[TitlesChosen] [int] NULL,
	[TitleSpecialExpires] [int] NULL,
	[AuthUserData] [varchar](32) NULL,
	[UiSettings] [int] NULL,
	[ShowSettings] [int] NULL,
	[NPCCostume] [smallint] NULL,
	[Banned] [tinyint] NULL,
	[NumCostumeSlots] [int] NULL,
	[SuperPrimary] [int] NULL,
	[SuperSecondary] [int] NULL,
	[CurrentCostume] [int] NULL,
	[SuperPrimary2] [int] NULL,
	[SuperSecondary2] [int] NULL,
	[SuperTertiary] [int] NULL,
	[SuperQuaternary] [int] NULL,
	[FxSpecial] [nvarchar](256) NULL,
	[FxSpecialExpires] [int] NULL,
	[CsrModified] [int] NULL,
	[DateCreated] [datetime] NULL,
	[Gender] [tinyint] NULL,
	[NameGender] [tinyint] NULL,
	[PlayerType] [tinyint] NULL,
	[Prestige] [int] NULL,
	[IsSlotLocked] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[Ents2]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Ents2](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[RespecTokens] [int] NULL,
	[PendingReward] [nvarchar](272) NULL,
	[PendingRewardVillian] [int] NULL,
	[PendingRewardLevel] [int] NULL,
	[TitleBadge] [int] NULL,
	[ChatSettings] [int] NULL,
	[PrimaryChatMinimized] [int] NULL,
	[MousePitch] [int] NULL,
	[UiSettings2] [int] NULL,
	[UserSendChannel] [tinyint] NULL,
	[FreeTailorSessions] [int] NULL,
	[MapOptions] [int] NULL,
	[Notoriety] [int] NULL,
	[ChatBubbleTextColor] [int] NULL,
	[ChatBubbleBackColor] [int] NULL,
	[TitleTheText] [nvarchar](20) NULL,
	[DividerSuperName] [int] NULL,
	[DividerSuperMap] [int] NULL,
	[DividerSuperTitle] [int] NULL,
	[DividerEmailFrom] [int] NULL,
	[DividerEmailSubject] [int] NULL,
	[DividerFriendName] [int] NULL,
	[DividerLfgName] [int] NULL,
	[DividerLfgMap] [int] NULL,
	[ChatBeta] [tinyint] NULL,
	[LfgFlags] [int] NULL,
	[Comment] [nvarchar](256) NULL,
	[TooltipDelay] [real] NULL,
	[UltraTailor] [int] NULL,
	[ArenaPaid] [int] NULL,
	[ArenaPaidAmount] [int] NULL,
	[ArenaPrizeAmount] [int] NULL,
	[Insight] [real] NULL,
	[CurrentAlt2Tray] [int] NULL,
	[MaxHitPoints] [real] NULL,
	[WisdomPoints] [int] NULL,
	[WisdomLevel] [int] NULL,
	[PvPSwitch] [tinyint] NULL,
	[Reputation] [real] NULL,
	[VillainGurneyMapId] [smallint] NULL,
	[SkillsUnlocked] [tinyint] NULL,
	[Rage] [real] NULL,
	[ExitMissionContext] [int] NULL,
	[ExitMissionSubHandle] [int] NULL,
	[ExitMissionCompoundPos] [int] NULL,
	[ExitMissionOwnerId] [int] NULL,
	[ExitMissionSuccess] [tinyint] NULL,
	[TeamCompleteOption] [tinyint] NULL,
	[TimeInSGMode] [int] NULL,
	[UpdateTeamTask] [tinyint] NULL,
	[BuffSettings] [int] NULL,
	[RecipeInvBonus] [int] NULL,
	[RecipeInvTotal] [int] NULL,
	[SalvageInvBonus] [int] NULL,
	[SalvageInvTotal] [int] NULL,
	[AuctionInvBonus] [int] NULL,
	[AuctionInvTotal] [int] NULL,
	[UiSettings3] [int] NULL,
	[StoredSalvageInvBonus] [int] NULL,
	[StoredSalvageInvTotal] [int] NULL,
	[AccSvrLock] [varchar](74) NULL,
	[TrayIndexes] [int] NULL,
	[HideField] [int] NULL,
	[originalPrimary] [varchar](256) NULL,
	[originalSecondary] [varchar](256) NULL,
	[MouseScrollSpeed] [real] NULL,
	[ExperienceRest] [int] NULL,
	[CurBuild] [int] NULL,
	[LevelBuild0] [int] NULL,
	[LevelBuild1] [int] NULL,
	[LevelBuild2] [int] NULL,
	[LevelBuild3] [int] NULL,
	[LevelBuild4] [int] NULL,
	[LevelBuild5] [int] NULL,
	[LevelBuild6] [int] NULL,
	[LevelBuild7] [int] NULL,
	[RaidsId] [int] NULL,
	[LevelingPactsId] [int] NULL,
	[PendingArchitectTickets] [int] NULL,
	[BuildChangeTime] [int] NULL,
	[BuildName0] [nvarchar](64) NULL,
	[BuildName1] [nvarchar](64) NULL,
	[BuildName2] [nvarchar](64) NULL,
	[BuildName3] [nvarchar](64) NULL,
	[BuildName4] [nvarchar](64) NULL,
	[BuildName5] [nvarchar](64) NULL,
	[BuildName6] [nvarchar](64) NULL,
	[BuildName7] [nvarchar](64) NULL,
	[ExitMissionPlayerCreated] [int] NULL,
	[LastDayJobsStart] [datetime] NULL,
	[ArchitectMissionsCompleted] [int] NULL,
	[PlayerSubType] [tinyint] NULL,
	[InfluenceType] [tinyint] NULL,
	[InfluenceEscrow] [int] NULL,
	[AutoAcceptAbove] [tinyint] NULL,
	[AutoAcceptBelow] [tinyint] NULL,
	[LevelAdjust] [int] NULL,
	[TeamSize] [int] NULL,
	[UpgradeAV] [int] NULL,
	[DowngradeBoss] [int] NULL,
	[PraetorianProgress] [tinyint] NULL,
	[SpecialMapReturnData] [nvarchar](256) NULL,
	[IncarnateTimer0] [int] NULL,
	[IncarnateTimer1] [int] NULL,
	[IncarnateTimer2] [int] NULL,
	[IncarnateTimer3] [int] NULL,
	[IncarnateTimer4] [int] NULL,
	[IncarnateTimer5] [int] NULL,
	[IncarnateTimer6] [int] NULL,
	[IncarnateTimer7] [int] NULL,
	[IncarnateTimer8] [int] NULL,
	[IncarnateTimer9] [int] NULL,
	[PopHelpLatest0] [int] NULL,
	[PopHelpLatest1] [int] NULL,
	[PopHelpLatest2] [int] NULL,
	[PopHelpLatest3] [int] NULL,
	[PopHelpLatest4] [int] NULL,
	[PopHelpStatus] [varchar](1024) NULL,
	[TitleColor1] [int] NULL,
	[TitleColor2] [int] NULL,
	[AuthUserDataEx] [varchar](256) NULL,
	[LeaguesId] [int] NULL,
	[SpecialReturnInProgress] [tinyint] NULL,
	[CurrentRazerTray] [int] NULL,
	[RequiresGoingRogueOrTrial] [int] NULL,
	[HomeDBID] [int] NULL,
	[HomeShard] [int] NULL,
	[RemoteDBID] [int] NULL,
	[VisitStartTime] [int] NULL,
	[HomeSGID] [int] NULL,
	[HomeLPID] [int] NULL,
	[ShardVisitorData] [nvarchar](128) NULL,
	[RemoteShard] [int] NULL,
	[DisplayAlignmentStatsToOthers] [int] NULL,
	[DesiredTeamNumber] [int] NULL,
	[LastAutoCommandRunTime] [int] NULL,
	[IsTeamLeader] [int] NULL,
	[LastTurnstileEventID] [int] NULL,
	[LastTurnstileMission] [int] NULL,
	[TurnstileTeamLock] [int] NULL,
	[PendingCertification0] [int] NULL,
	[PendingCertification1] [int] NULL,
	[PendingCertification2] [int] NULL,
	[PendingCertification3] [int] NULL,
	[HelperStatus] [int] NULL,
	[UiSettings4] [int] NULL,
	[MapOptionRevision] [int] NULL,
	[MapOptions2] [int] NULL,
	[SelectedContactOnZoneEnter] [int] NULL,
	[PendingCertificationGrant] [int] NULL,
	[TeamupTimer_ActivePlayer] [int] NULL,
	[ValidateCostume] [int] NULL,
	[NumCostumeStored] [int] NULL,
	[DoNotKick] [int] NULL,
	[LastTurnstileStartTime] [int] NULL,
	[HideOpenSalvageWarning] [tinyint] NULL,
	[Absorb] [real] NULL,
	[hideStorePiecesState] [tinyint] NULL,
	[cursorScale] [real] NULL,
	[NewFeaturesVersion] [int] NULL,
 CONSTRAINT [PK_Ents2] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[EventHistory]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[EventHistory](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[TimeStamp] [int] NULL,
	[EventName] [nvarchar](100) NULL,
	[AuthName] [nvarchar](64) NULL,
	[Name] [nvarchar](42) NULL,
	[DBID] [int] NULL,
	[Level] [int] NULL,
	[Class] [int] NULL,
	[PrimaryPowerset] [int] NULL,
	[SecondaryPowerset] [int] NULL,
	[BandTable] [nvarchar](100) NULL,
	[PercentileTable] [nvarchar](100) NULL,
	[OfferedTable] [int] NULL,
	[RewardChosen] [nvarchar](100) NULL,
	[StagesPlayed] [int] NULL,
	[StagesQualified] [int] NULL,
	[StagesAvgScore] [int] NULL,
	[StagesAvgPercentile] [int] NULL,
	[TimePaused] [int] NULL,
	[StagesPaused] [int] NULL,
	[StageScore1] [int] NULL,
	[StageThreshold1] [int] NULL,
	[StageRank1] [int] NULL,
	[StagePercentile1] [int] NULL,
	[StageSamples1] [int] NULL,
	[StageScore2] [int] NULL,
	[StageThreshold2] [int] NULL,
	[StageRank2] [int] NULL,
	[StagePercentile2] [int] NULL,
	[StageSamples2] [int] NULL,
	[StageScore3] [int] NULL,
	[StageThreshold3] [int] NULL,
	[StageRank3] [int] NULL,
	[StagePercentile3] [int] NULL,
	[StageSamples3] [int] NULL,
	[StageScore4] [int] NULL,
	[StageThreshold4] [int] NULL,
	[StageRank4] [int] NULL,
	[StagePercentile4] [int] NULL,
	[StageSamples4] [int] NULL,
	[StageScore5] [int] NULL,
	[StageThreshold5] [int] NULL,
	[StageRank5] [int] NULL,
	[StagePercentile5] [int] NULL,
	[StageSamples5] [int] NULL,
	[StageScore6] [int] NULL,
	[StageThreshold6] [int] NULL,
	[StageRank6] [int] NULL,
	[StagePercentile6] [int] NULL,
	[StageSamples6] [int] NULL,
	[StageScore7] [int] NULL,
	[StageThreshold7] [int] NULL,
	[StageRank7] [int] NULL,
	[StagePercentile7] [int] NULL,
	[StageSamples7] [int] NULL,
	[StageScore8] [int] NULL,
	[StageThreshold8] [int] NULL,
	[StageRank8] [int] NULL,
	[StagePercentile8] [int] NULL,
	[StageSamples8] [int] NULL,
	[StageScore9] [int] NULL,
	[StageThreshold9] [int] NULL,
	[StageRank9] [int] NULL,
	[StagePercentile9] [int] NULL,
	[StageSamples9] [int] NULL,
	[StageScore10] [int] NULL,
	[StageThreshold10] [int] NULL,
	[StageRank10] [int] NULL,
	[StagePercentile10] [int] NULL,
	[StageSamples10] [int] NULL,
	[StageScore11] [int] NULL,
	[StageThreshold11] [int] NULL,
	[StageRank11] [int] NULL,
	[StagePercentile11] [int] NULL,
	[StageSamples11] [int] NULL,
	[StageScore12] [int] NULL,
	[StageThreshold12] [int] NULL,
	[StageRank12] [int] NULL,
	[StagePercentile12] [int] NULL,
	[StageSamples12] [int] NULL,
	[StageScore13] [int] NULL,
	[StageThreshold13] [int] NULL,
	[StageRank13] [int] NULL,
	[StagePercentile13] [int] NULL,
	[StageSamples13] [int] NULL,
	[StageScore14] [int] NULL,
	[StageThreshold14] [int] NULL,
	[StageRank14] [int] NULL,
	[StagePercentile14] [int] NULL,
	[StageSamples14] [int] NULL,
	[StageScore15] [int] NULL,
	[StageThreshold15] [int] NULL,
	[StageRank15] [int] NULL,
	[StagePercentile15] [int] NULL,
	[StageSamples15] [int] NULL,
	[StageScore16] [int] NULL,
	[StageThreshold16] [int] NULL,
	[StageRank16] [int] NULL,
	[StagePercentile16] [int] NULL,
	[StageSamples16] [int] NULL,
	[StageScore17] [int] NULL,
	[StageThreshold17] [int] NULL,
	[StageRank17] [int] NULL,
	[StagePercentile17] [int] NULL,
	[StageSamples17] [int] NULL,
	[StageScore18] [int] NULL,
	[StageThreshold18] [int] NULL,
	[StageRank18] [int] NULL,
	[StagePercentile18] [int] NULL,
	[StageSamples18] [int] NULL,
	[StageScore19] [int] NULL,
	[StageThreshold19] [int] NULL,
	[StageRank19] [int] NULL,
	[StagePercentile19] [int] NULL,
	[StageSamples19] [int] NULL,
	[StageScore20] [int] NULL,
	[StageThreshold20] [int] NULL,
	[StageRank20] [int] NULL,
	[StagePercentile20] [int] NULL,
	[StageSamples20] [int] NULL,
	[StageScore21] [int] NULL,
	[StageThreshold21] [int] NULL,
	[StageRank21] [int] NULL,
	[StagePercentile21] [int] NULL,
	[StageSamples21] [int] NULL,
	[StageScore22] [int] NULL,
	[StageThreshold22] [int] NULL,
	[StageRank22] [int] NULL,
	[StagePercentile22] [int] NULL,
	[StageSamples22] [int] NULL,
	[StageScore23] [int] NULL,
	[StageThreshold23] [int] NULL,
	[StageRank23] [int] NULL,
	[StagePercentile23] [int] NULL,
	[StageSamples23] [int] NULL,
	[StageScore24] [int] NULL,
	[StageThreshold24] [int] NULL,
	[StageRank24] [int] NULL,
	[StagePercentile24] [int] NULL,
	[StageSamples24] [int] NULL,
	[StageScore25] [int] NULL,
	[StageThreshold25] [int] NULL,
	[StageRank25] [int] NULL,
	[StagePercentile25] [int] NULL,
	[StageSamples25] [int] NULL,
	[StageScore26] [int] NULL,
	[StageThreshold26] [int] NULL,
	[StageRank26] [int] NULL,
	[StagePercentile26] [int] NULL,
	[StageSamples26] [int] NULL,
	[StageScore27] [int] NULL,
	[StageThreshold27] [int] NULL,
	[StageRank27] [int] NULL,
	[StagePercentile27] [int] NULL,
	[StageSamples27] [int] NULL,
	[StageScore28] [int] NULL,
	[StageThreshold28] [int] NULL,
	[StageRank28] [int] NULL,
	[StagePercentile28] [int] NULL,
	[StageSamples28] [int] NULL,
	[StageScore29] [int] NULL,
	[StageThreshold29] [int] NULL,
	[StageRank29] [int] NULL,
	[StagePercentile29] [int] NULL,
	[StageSamples29] [int] NULL,
	[StageScore30] [int] NULL,
	[StageThreshold30] [int] NULL,
	[StageRank30] [int] NULL,
	[StagePercentile30] [int] NULL,
	[StageSamples30] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[FameStrings]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FameStrings](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[String] [nvarchar](256) NULL,
 CONSTRAINT [PK_FameStrings] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Friends]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Friends](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Id] [int] NULL,
	[Class] [int] NULL,
	[Origin] [int] NULL,
	[Description] [nvarchar](128) NULL,
 CONSTRAINT [PK_Friends] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[GmailClaims]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GmailClaims](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[MailId] [int] NULL,
	[GmailClaimState] [tinyint] NULL,
 CONSTRAINT [PK_GmailClaims] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[GmailPending]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GmailPending](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[MailId] [int] NULL,
	[XactID] [int] NULL,
	[GmailXactState] [int] NULL,
	[Influence] [int] NULL,
	[RequestTime] [int] NULL,
	[Subject] [nvarchar](320) NULL,
	[Attachment] [nvarchar](510) NULL,
	[Recipient] [nvarchar](64) NULL,
	[Inventory] [nvarchar](510) NULL,
	[BankedInfluence] [int] NULL,
	[Body] [nvarchar](max) NULL,
 CONSTRAINT [PK_GmailPending] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Ignore]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Ignore](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[authid] [int] NULL,
 CONSTRAINT [PK_Ignore] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Inspirations]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Inspirations](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Col] [int] NULL,
	[Row] [int] NULL,
	[CategoryName] [int] NULL,
	[PowerSetName] [int] NULL,
	[PowerName] [int] NULL,
 CONSTRAINT [PK_Inspirations] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[InvBaseDetail]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[InvBaseDetail](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[name] [int] NULL,
	[amount] [int] NULL,
 CONSTRAINT [PK_InvBaseDetail] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[InvRecipe0]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[InvRecipe0](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Enhancement_Test] [int] NULL,
	[Enhancement_Test_2] [int] NULL,
	[Base_Disrupter_Pylon_Arcane] [int] NULL,
	[base_ViewingPortal] [int] NULL,
	[base_MysticOverseer] [int] NULL,
	[base_AstralHaruspex] [int] NULL,
	[base_SeerDesk] [int] NULL,
	[base_MysticOrrery] [int] NULL,
	[base_InfernalAdvisor] [int] NULL,
	[base_MysticalBookshelf] [int] NULL,
	[base_ScryingPaintings] [int] NULL,
	[Base_SnowCrystal] [int] NULL,
	[Base_DampeningRay_Arcane] [int] NULL,
	[Base_EliteStonethrower] [int] NULL,
	[Base_EliteSnowCrystal] [int] NULL,
	[Base_EliteDampeningRay_Arcane] [int] NULL,
	[Base_EliteEnergyBeam_Arcane] [int] NULL,
	[Base_EliteIgniter_Arcane] [int] NULL,
	[Base_EliteSapper_Arcane] [int] NULL,
	[Base_EliteAnalyzer] [int] NULL,
	[Base_ImprovedStonethrower] [int] NULL,
	[Base_ImprovedSnowCrystal] [int] NULL,
	[Base_ImprovedDampeningRay_Arcane] [int] NULL,
	[Base_ImprovedSapper_Arcane] [int] NULL,
	[Base_ImprovedAnalyzer] [int] NULL,
	[Base_Sapper_Arcane] [int] NULL,
	[Base_TeslaCage_Arcane] [int] NULL,
	[Base_Turret_Repair_Arcane] [int] NULL,
	[Base_Turret_Energy_Reducer_Arcane] [int] NULL,
	[Base_DamageBooster_Arcane] [int] NULL,
	[Base_SeeingCrystal] [int] NULL,
	[Base_EnergyShield_Arcane] [int] NULL,
	[Base_EagleEye] [int] NULL,
	[Base_Igniter_Arcane] [int] NULL,
	[Base_ControlRune] [int] NULL,
	[Base_PainField_Arcane] [int] NULL,
	[Base_SlowField_Arcane] [int] NULL,
	[Base_StealthSuppression_Arcane] [int] NULL,
	[Base_WeaknessField_Arcane] [int] NULL,
	[Base_Battery4_Arcane] [int] NULL,
	[Base_Battery3_Arcane] [int] NULL,
	[Base_EnergyShieldL1_Arcane] [int] NULL,
	[Base_EnergyL3_Arcane] [int] NULL,
	[Base_EnergyL2_Arcane] [int] NULL,
	[Base_TreeofWonders_Arcane] [int] NULL,
	[Base_AnatomicalCharts] [int] NULL,
	[Base_RuneSpire] [int] NULL,
	[Base_BasicTelepad_Arcane] [int] NULL,
	[Base_Oracle_Arcane] [int] NULL,
	[Base_Oracle_Arcane_v] [int] NULL,
	[Base_Toolchest_Arcane] [int] NULL,
	[Base_Empowerment2_Arcane] [int] NULL,
	[Base_Empowerment3_Arcane] [int] NULL,
	[Base_Increase_Attack_Speed_Arcane] [int] NULL,
	[Base_Increase_Flight_Speed_Arcane] [int] NULL,
	[Base_Increase_Jump_Speed_Arcane] [int] NULL,
	[Base_Increase_Recovery_Arcane] [int] NULL,
	[Base_Increase_Run_Speed_Arcane] [int] NULL,
	[Base_Fire_Resistance_Arcane] [int] NULL,
	[Base_Knockback_Protection_Arcane] [int] NULL,
	[Base_Knockback_Increase_Arcane] [int] NULL,
	[Base_Smashing_Resistance_Arcane] [int] NULL,
	[Base_Lethal_Resistance_Arcane] [int] NULL,
	[Base_Cold_Resistance_Arcane] [int] NULL,
	[Base_Energy_Resistance_Arcane] [int] NULL,
	[Base_Negative_Energy_Resistance_Arcane] [int] NULL,
	[Base_Psionic_Resistance_Arcane] [int] NULL,
	[Base_Toxic_Resistance_Arcane] [int] NULL,
	[Base_Endurance_Drain_Resistance_Arcane] [int] NULL,
	[Base_Increase_Perception_Arcane] [int] NULL,
	[Base_Grant_Invisibility_Arcane] [int] NULL,
	[Base_Confusion_Resistance_Arcane] [int] NULL,
	[Base_Fear_Resistance_Arcane] [int] NULL,
	[Base_Disorient_Resistance_Arcane] [int] NULL,
	[Base_Hold_Resistance_Arcane] [int] NULL,
	[Base_Immobilize_Resistance_Arcane] [int] NULL,
	[Base_Sleep_Resistance_Arcane] [int] NULL,
	[Base_Slow_Resistance_Arcane] [int] NULL,
	[S_TechMaterial_5thColumn] [int] NULL,
	[S_TechPower_5thColumn] [int] NULL,
	[S_TechHardware_5thColumn] [int] NULL,
	[S_TechPrototype_5thColumn] [int] NULL,
	[S_TechSoftware_5thColumn] [int] NULL,
	[S_ExperimentalTech_5thColumn] [int] NULL,
	[S_TechMaterial_Arachnoids] [int] NULL,
	[S_TechPower_Arachnoids] [int] NULL,
	[S_TechHardware_Arachnoids] [int] NULL,
	[S_TechPrototype_Arachnoids] [int] NULL,
	[S_TechSoftware_Arachnoids] [int] NULL,
	[S_ExperimentalTech_Arachnoids] [int] NULL,
	[S_TechMaterial_Arachnos] [int] NULL,
	[S_TechPower_Arachnos] [int] NULL,
	[S_TechHardware_Arachnos] [int] NULL,
	[S_TechPrototype_Arachnos] [int] NULL,
	[S_TechSoftware_Arachnos] [int] NULL,
	[S_ExperimentalTech_Arachnos] [int] NULL,
	[S_TechMaterial_AxisAmerica] [int] NULL,
	[S_TechPower_AxisAmerica] [int] NULL,
	[S_TechHardware_AxisAmerica] [int] NULL,
	[S_TechPrototype_AxisAmerica] [int] NULL,
	[S_TechSoftware_AxisAmerica] [int] NULL,
	[S_ExperimentalTech_AxisAmerica] [int] NULL,
	[S_MysticElement_BanishedPantheon] [int] NULL,
	[S_MysticFoci_BanishedPantheon] [int] NULL,
	[S_ArcaneEssence_BanishedPantheon] [int] NULL,
	[S_ArcaneGlyph_BanishedPantheon] [int] NULL,
	[S_MagicalWard_BanishedPantheon] [int] NULL,
	[S_MagicalArtifact_BanishedPantheon] [int] NULL,
	[S_MysticElement_Cabal] [int] NULL,
	[S_MysticFoci_Cabal] [int] NULL,
	[S_ArcaneEssence_Cabal] [int] NULL,
	[S_ArcaneGlyph_Cabal] [int] NULL,
	[S_MysticElement_CapauDiableDemons] [int] NULL,
	[S_MysticFoci_CapauDiableDemons] [int] NULL,
	[S_MysticElement_Carnival] [int] NULL,
	[S_MysticFoci_Carnival] [int] NULL,
	[S_ArcaneEssence_Carnival] [int] NULL,
	[S_ArcaneGlyph_Carnival] [int] NULL,
	[S_MagicalWard_Carnival] [int] NULL,
	[S_MagicalArtifact_Carnival] [int] NULL,
	[S_MysticElement_CircleOfThorns] [int] NULL,
	[S_MysticFoci_CircleOfThorns] [int] NULL,
	[S_ArcaneEssence_CircleOfThorns] [int] NULL,
	[S_ArcaneGlyph_CircleOfThorns] [int] NULL,
	[S_MagicalWard_CircleOfThorns] [int] NULL,
	[S_MagicalArtifact_CircleOfThorns] [int] NULL,
	[S_TechMaterial_Clockwork] [int] NULL,
	[S_TechPower_Clockwork] [int] NULL,
	[S_MysticElement_Coralax] [int] NULL,
	[S_MysticFoci_Coralax] [int] NULL,
	[S_ArcaneEssence_Coralax] [int] NULL,
	[S_ArcaneGlyph_Coralax] [int] NULL,
	[S_TechMaterial_Council] [int] NULL,
	[S_TechPower_Council] [int] NULL,
	[S_TechHardware_Council] [int] NULL,
	[S_TechPrototype_Council] [int] NULL,
	[S_TechSoftware_Council] [int] NULL,
	[S_ExperimentalTech_Council] [int] NULL,
	[S_TechMaterial_CouncilEmpire] [int] NULL,
	[S_TechPower_CouncilEmpire] [int] NULL,
	[S_TechHardware_CouncilEmpire] [int] NULL,
	[S_TechPrototype_CouncilEmpire] [int] NULL,
	[S_TechSoftware_CouncilEmpire] [int] NULL,
	[S_ExperimentalTech_CouncilEmpire] [int] NULL,
	[S_TechMaterial_ConsortiumGuards] [int] NULL,
	[S_TechPower_ConsortiumGuards] [int] NULL,
	[S_TechHardware_ConsortiumGuards] [int] NULL,
	[S_TechPrototype_ConsortiumGuards] [int] NULL,
	[S_TechSoftware_ConsortiumGuards] [int] NULL,
	[S_ExperimentalTech_ConsortiumGuards] [int] NULL,
	[S_TechMaterial_Crey] [int] NULL,
	[S_TechPower_Crey] [int] NULL,
	[S_TechHardware_Crey] [int] NULL,
	[S_TechPrototype_Crey] [int] NULL,
	[S_TechSoftware_Crey] [int] NULL,
	[S_ExperimentalTech_Crey] [int] NULL,
	[S_MysticElement_CroatoaGhosts] [int] NULL,
	[S_MysticFoci_CroatoaGhosts] [int] NULL,
	[S_ArcaneEssence_CroatoaGhosts] [int] NULL,
	[S_ArcaneGlyph_CroatoaGhosts] [int] NULL,
	[S_TechMaterial_DevouringEarth] [int] NULL,
	[S_TechPower_DevouringEarth] [int] NULL,
	[S_TechHardware_DevouringEarth] [int] NULL,
	[S_TechPrototype_DevouringEarth] [int] NULL,
	[S_TechSoftware_DevouringEarth] [int] NULL,
	[S_ExperimentalTech_DevouringEarth] [int] NULL,
	[S_MysticElement_FirBolg] [int] NULL,
	[S_MysticFoci_FirBolg] [int] NULL,
	[S_ArcaneEssence_FirBolg] [int] NULL,
	[S_ArcaneGlyph_FirBolg] [int] NULL,
	[S_TechMaterial_Freakshow] [int] NULL,
	[S_TechPower_Freakshow] [int] NULL,
	[S_TechHardware_Freakshow] [int] NULL,
	[S_TechPrototype_Freakshow] [int] NULL,
	[S_TechSoftware_Freakshow] [int] NULL,
	[S_ExperimentalTech_Freakshow] [int] NULL,
	[S_TechMaterial_FreedomCorps] [int] NULL,
	[S_TechPower_FreedomCorps] [int] NULL,
	[S_TechHardware_FreedomCorps] [int] NULL,
	[S_TechPrototype_FreedomCorps] [int] NULL,
	[S_TechSoftware_FreedomCorps] [int] NULL,
	[S_ExperimentalTech_FreedomCorps] [int] NULL,
	[S_MysticElement_Ghosts] [int] NULL,
	[S_MysticFoci_Ghosts] [int] NULL,
	[S_ArcaneEssence_Ghosts] [int] NULL,
	[S_ArcaneGlyph_Ghosts] [int] NULL,
	[S_TechMaterial_GoldBricker] [int] NULL,
	[S_TechPower_GoldBricker] [int] NULL,
	[S_TechMaterial_GoodTurrets] [int] NULL,
	[S_TechPower_GoodTurrets] [int] NULL,
	[S_TechHardware_GoodTurrets] [int] NULL,
	[S_TechPrototype_GoodTurrets] [int] NULL,
	[S_TechSoftware_GoodTurrets] [int] NULL,
	[S_ExperimentalTech_GoodTurrets] [int] NULL,
	[S_MysticElement_Hellions] [int] NULL,
	[S_MysticFoci_Hellions] [int] NULL,
	[S_TechMaterial_Hydra] [int] NULL,
	[S_TechPower_Hydra] [int] NULL,
	[S_TechHardware_Hydra] [int] NULL,
	[S_TechPrototype_Hydra] [int] NULL,
	[S_TechSoftware_Hydra] [int] NULL,
	[S_ExperimentalTech_Hydra] [int] NULL,
	[S_MysticElement_Igneous] [int] NULL,
	[S_MysticFoci_Igneous] [int] NULL,
	[S_MysticElement_Infected] [int] NULL,
	[S_MysticFoci_Infected] [int] NULL,
	[S_TechMaterial_KnivesOfArtemis] [int] NULL,
	[S_TechPower_KnivesOfArtemis] [int] NULL,
	[S_TechHardware_KnivesOfArtemis] [int] NULL,
	[S_TechPrototype_KnivesOfArtemis] [int] NULL,
	[S_TechSoftware_KnivesOfArtemis] [int] NULL,
	[S_ExperimentalTech_KnivesOfArtemis] [int] NULL,
	[S_MysticElement_LegacyChain] [int] NULL,
	[S_MysticFoci_LegacyChain] [int] NULL,
	[S_ArcaneEssence_LegacyChain] [int] NULL,
	[S_ArcaneGlyph_LegacyChain] [int] NULL,
	[S_MagicalWard_LegacyChain] [int] NULL,
	[S_MagicalArtifact_LegacyChain] [int] NULL,
	[S_MysticElement_Luddites] [int] NULL,
	[S_MysticFoci_Luddites] [int] NULL,
	[S_TechMaterial_Malta] [int] NULL,
	[S_TechPower_Malta] [int] NULL,
	[S_TechHardware_Malta] [int] NULL,
	[S_TechPrototype_Malta] [int] NULL,
	[S_TechSoftware_Malta] [int] NULL,
	[S_ExperimentalTech_Malta] [int] NULL,
	[S_TechMaterial_Mooks] [int] NULL,
	[S_TechPower_Mooks] [int] NULL,
	[S_TechMaterial_Nemesis] [int] NULL,
	[S_TechPower_Nemesis] [int] NULL,
	[S_TechHardware_Nemesis] [int] NULL,
	[S_TechPrototype_Nemesis] [int] NULL,
	[S_TechSoftware_Nemesis] [int] NULL,
	[S_ExperimentalTech_Nemesis] [int] NULL,
	[S_TechMaterial_NemesisAutomatons] [int] NULL,
	[S_TechPower_NemesisAutomatons] [int] NULL,
	[S_TechHardware_NemesisAutomatons] [int] NULL,
	[S_TechPrototype_NemesisAutomatons] [int] NULL,
	[S_TechSoftware_NemesisAutomatons] [int] NULL,
	[S_ExperimentalTech_NemesisAutomatons] [int] NULL,
	[S_TechMaterial_Nictus] [int] NULL,
	[S_TechPower_Nictus] [int] NULL,
	[S_TechHardware_Nictus] [int] NULL,
	[S_TechPrototype_Nictus] [int] NULL,
	[S_TechSoftware_Nictus] [int] NULL,
	[S_ExperimentalTech_Nictus] [int] NULL,
	[S_MysticElement_Outcasts] [int] NULL,
	[S_MysticFoci_Outcasts] [int] NULL,
	[S_TechMaterial_ParagonProtectors] [int] NULL,
	[S_TechPower_ParagonProtectors] [int] NULL,
	[S_TechHardware_ParagonProtectors] [int] NULL,
	[S_TechPrototype_ParagonProtectors] [int] NULL,
	[S_TechSoftware_ParagonProtectors] [int] NULL,
	[S_ExperimentalTech_ParagonProtectors] [int] NULL,
	[S_TechMaterial_Prisoners] [int] NULL,
	[S_TechPower_Prisoners] [int] NULL,
	[S_TechHardware_Prisoners] [int] NULL,
	[S_TechPrototype_Prisoners] [int] NULL,
	[S_TechSoftware_Prisoners] [int] NULL,
	[S_ExperimentalTech_Prisoners] [int] NULL,
	[S_MysticElement_PsychicClockwork] [int] NULL,
	[S_MysticFoci_PsychicClockwork] [int] NULL,
	[S_ArcaneEssence_PsychicClockwork] [int] NULL,
	[S_ArcaneGlyph_PsychicClockwork] [int] NULL,
	[S_MagicalWard_PsychicClockwork] [int] NULL,
	[S_MagicalArtifact_PsychicClockwork] [int] NULL,
	[S_MysticElement_RedCaps] [int] NULL,
	[S_MysticFoci_RedCaps] [int] NULL,
	[S_ArcaneEssence_RedCaps] [int] NULL,
	[S_ArcaneGlyph_RedCaps] [int] NULL,
	[S_TechMaterial_Rikti] [int] NULL,
	[S_TechPower_Rikti] [int] NULL,
	[S_TechHardware_Rikti] [int] NULL,
	[S_TechPrototype_Rikti] [int] NULL,
	[S_TechSoftware_Rikti] [int] NULL,
	[S_ExperimentalTech_Rikti] [int] NULL,
	[S_TechMaterial_RogueArachnos] [int] NULL,
	[S_TechPower_RogueArachnos] [int] NULL,
	[S_TechHardware_RogueArachnos] [int] NULL,
	[S_TechPrototype_RogueArachnos] [int] NULL,
	[S_TechSoftware_RogueArachnos] [int] NULL,
	[S_ExperimentalTech_RogueArachnos] [int] NULL,
	[S_TechMaterial_RogueIslandPolice] [int] NULL,
	[S_TechPower_RogueIslandPolice] [int] NULL,
	[S_TechHardware_RogueIslandPolice] [int] NULL,
	[S_TechPrototype_RogueIslandPolice] [int] NULL,
	[S_TechSoftware_RogueIslandPolice] [int] NULL,
	[S_ExperimentalTech_RogueIslandPolice] [int] NULL,
	[S_TechMaterial_RogueRobots] [int] NULL,
	[S_TechPower_RogueRobots] [int] NULL,
	[S_TechHardware_RogueRobots] [int] NULL,
	[S_TechPrototype_RogueRobots] [int] NULL,
	[S_TechSoftware_RogueRobots] [int] NULL,
	[S_ExperimentalTech_RogueRobots] [int] NULL,
	[S_MysticElement_Scrapyarders] [int] NULL,
	[S_MysticFoci_Scrapyarders] [int] NULL,
	[S_ArcaneEssence_Scrapyarders] [int] NULL,
	[S_ArcaneGlyph_Scrapyarders] [int] NULL,
	[S_TechMaterial_SecurityGuards] [int] NULL,
	[S_TechPower_SecurityGuards] [int] NULL,
	[S_TechHardware_SecurityGuards] [int] NULL,
	[S_TechPrototype_SecurityGuards] [int] NULL,
	[S_TechSoftware_SecurityGuards] [int] NULL,
	[S_ExperimentalTech_SecurityGuards] [int] NULL,
	[S_MysticElement_Shivans] [int] NULL,
	[S_MysticFoci_Shivans] [int] NULL,
	[S_ArcaneEssence_Shivans] [int] NULL,
	[S_ArcaneGlyph_Shivans] [int] NULL,
	[S_TechMaterial_SlagGolems] [int] NULL,
	[S_TechPower_SlagGolems] [int] NULL,
	[S_TechHardware_SlagGolems] [int] NULL,
	[S_TechPrototype_SlagGolems] [int] NULL,
	[S_MysticElement_Skulls] [int] NULL,
	[S_MysticFoci_Skulls] [int] NULL,
	[S_TechMaterial_SkyRaiders] [int] NULL,
	[S_TechPower_SkyRaiders] [int] NULL,
	[S_TechHardware_SkyRaiders] [int] NULL,
	[S_TechPrototype_SkyRaiders] [int] NULL,
	[S_TechSoftware_SkyRaiders] [int] NULL,
	[S_ExperimentalTech_SkyRaiders] [int] NULL,
	[S_MysticElement_Snakes] [int] NULL,
	[S_MysticFoci_Snakes] [int] NULL,
	[S_TechMaterial_SpetsnazCommandos] [int] NULL,
	[S_TechPower_SpetsnazCommandos] [int] NULL,
	[S_MysticElement_SpectralPirates] [int] NULL,
	[S_MysticFoci_SpectralPirates] [int] NULL,
	[S_TechMaterial_TheFamily] [int] NULL,
	[S_TechPower_TheFamily] [int] NULL,
	[S_TechHardware_TheFamily] [int] NULL,
	[S_TechPrototype_TheFamily] [int] NULL,
	[S_TechSoftware_TheFamily] [int] NULL,
	[S_ExperimentalTech_TheFamily] [int] NULL,
	[S_TechMaterial_TheLost] [int] NULL,
	[S_TechPower_TheLost] [int] NULL,
	[S_TechHardware_TheLost] [int] NULL,
	[S_TechPrototype_TheLost] [int] NULL,
	[S_TechSoftware_TheLost] [int] NULL,
	[S_ExperimentalTech_TheLost] [int] NULL,
	[S_TechMaterial_Thugs] [int] NULL,
	[S_TechPower_Thugs] [int] NULL,
	[S_TechMaterial_Trolls] [int] NULL,
	[S_TechPower_Trolls] [int] NULL,
	[S_MysticElement_Tsoo] [int] NULL,
	[S_MysticFoci_Tsoo] [int] NULL,
	[S_ArcaneEssence_Tsoo] [int] NULL,
	[S_ArcaneGlyph_Tsoo] [int] NULL,
	[S_MagicalWard_Tsoo] [int] NULL,
	[S_MagicalArtifact_Tsoo] [int] NULL,
	[S_MysticElement_Tuatha] [int] NULL,
	[S_MysticFoci_Tuatha] [int] NULL,
	[S_ArcaneEssence_Tuatha] [int] NULL,
	[S_ArcaneGlyph_Tuatha] [int] NULL,
	[S_TechMaterial_Turrets] [int] NULL,
	[S_TechPower_Turrets] [int] NULL,
	[S_TechHardware_Turrets] [int] NULL,
	[S_TechPrototype_Turrets] [int] NULL,
	[S_TechSoftware_Turrets] [int] NULL,
	[S_ExperimentalTech_Turrets] [int] NULL,
	[S_TechMaterial_Vahzilok] [int] NULL,
	[S_TechPower_Vahzilok] [int] NULL,
	[S_TechHardware_Vahzilok] [int] NULL,
	[S_TechPrototype_Vahzilok] [int] NULL,
	[S_MysticElement_Wailers] [int] NULL,
	[S_MysticFoci_Wailers] [int] NULL,
	[S_ArcaneEssence_Wailers] [int] NULL,
	[S_ArcaneGlyph_Wailers] [int] NULL,
	[S_MysticElement_Warriors] [int] NULL,
	[S_MysticFoci_Warriors] [int] NULL,
	[S_ArcaneEssence_Warriors] [int] NULL,
	[S_ArcaneGlyph_Warriors] [int] NULL,
	[S_TechMaterial_Wyvern] [int] NULL,
	[S_TechPower_Wyvern] [int] NULL,
	[S_TechHardware_Wyvern] [int] NULL,
	[S_TechPrototype_Wyvern] [int] NULL,
	[Base_Disrupter_Pylon_Tech] [int] NULL,
	[base_EdgeCornerTerminal] [int] NULL,
	[base_EdgeHolodisplay] [int] NULL,
	[base_EdgeMegaMonitor] [int] NULL,
	[base_EdgeTerminal] [int] NULL,
	[base_AutonomousExpertSystem] [int] NULL,
	[base_EdgeDatabase] [int] NULL,
	[base_Holodisplay] [int] NULL,
	[base_MonitorBank] [int] NULL,
	[Base_ChillCannon] [int] NULL,
	[Base_DampeningRay_Tech] [int] NULL,
	[Base_EliteChainGun] [int] NULL,
	[Base_EliteChillCannon] [int] NULL,
	[Base_EliteDampeningRay_Tech] [int] NULL,
	[Base_EliteEnergyBeam_Tech] [int] NULL,
	[Base_EliteIgniter_Tech] [int] NULL,
	[Base_EliteSapper_Tech] [int] NULL,
	[Base_EliteScanner] [int] NULL,
	[Base_ImprovedChainGun] [int] NULL,
	[Base_ImprovedChillCannon] [int] NULL,
	[Base_ImprovedDampeningRay_Tech] [int] NULL,
	[Base_ImprovedSapper_Tech] [int] NULL,
	[Base_ImprovedScanner] [int] NULL,
	[Base_Sapper_Tech] [int] NULL,
	[Base_TeslaCage_Tech] [int] NULL,
	[Base_Turret_Repair_Tech] [int] NULL,
	[Base_Turret_Energy_Reducer_Tech] [int] NULL,
	[Base_DamageBooster_Tech] [int] NULL,
	[Base_TargetingModule] [int] NULL,
	[Base_EnergyShield_Tech] [int] NULL,
	[Base_SensorArray] [int] NULL,
	[Base_Igniter_Tech] [int] NULL,
	[Base_Relay] [int] NULL,
	[Base_PainField_Tech] [int] NULL,
	[Base_SlowField_Tech] [int] NULL,
	[Base_StealthSuppression_Tech] [int] NULL,
	[Base_WeaknessField_Tech] [int] NULL,
	[Base_Battery4_Tech] [int] NULL,
	[Base_Battery3_Tech] [int] NULL,
	[Base_EnergyShieldL1_Tech] [int] NULL,
	[Base_EnergyL3_Tech] [int] NULL,
	[Base_EnergyL2_Tech] [int] NULL,
	[Base_AutoDoc_Tech] [int] NULL,
	[Base_CombatLogs_Tech] [int] NULL,
	[Base_TPStabilizer_Tech] [int] NULL,
	[Base_BasicTelepad_Tech] [int] NULL,
	[Base_CrimeComputer_Tech] [int] NULL,
	[Base_CrimeComputer_Tech_v] [int] NULL,
	[Base_Toolchest_Tech] [int] NULL,
	[Base_Empowerment2_Tech] [int] NULL,
	[Base_Empowerment3_Tech] [int] NULL,
	[Base_Increase_Attack_Speed_Tech] [int] NULL,
	[Base_Increase_Flight_Speed_Tech] [int] NULL,
	[Base_Increase_Jump_Speed_Tech] [int] NULL,
	[Base_Increase_Recovery_Tech] [int] NULL,
	[Base_Increase_Run_Speed_Tech] [int] NULL,
	[Base_Fire_Resistance_Tech] [int] NULL,
	[Base_Knockback_Protection_Tech] [int] NULL,
	[Base_Knockback_Increase_Tech] [int] NULL,
	[Base_Smashing_Resistance_Tech] [int] NULL,
	[Base_Lethal_Resistance_Tech] [int] NULL,
	[Base_Cold_Resistance_Tech] [int] NULL,
	[Base_Energy_Resistance_Tech] [int] NULL,
	[Base_Negative_Energy_Resistance_Tech] [int] NULL,
	[Base_Psionic_Resistance_Tech] [int] NULL,
	[Base_Toxic_Resistance_Tech] [int] NULL,
	[Base_Endurance_Drain_Resistance_Tech] [int] NULL,
	[Base_Increase_Perception_Tech] [int] NULL,
	[Base_Grant_Invisibility_Tech] [int] NULL,
	[Base_Confusion_Resistance_Tech] [int] NULL,
	[Base_Fear_Resistance_Tech] [int] NULL,
	[Base_Disorient_Resistance_Tech] [int] NULL,
	[Base_Hold_Resistance_Tech] [int] NULL,
	[Base_Immobilize_Resistance_Tech] [int] NULL,
	[Base_Sleep_Resistance_Tech] [int] NULL,
	[Base_Slow_Resistance_Tech] [int] NULL,
 CONSTRAINT [PK_InvRecipe0] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[InvRecipeInvention]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[InvRecipeInvention](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[c0Type] [int] NULL,
	[c0Amount] [int] NULL,
	[c1Type] [int] NULL,
	[c1Amount] [int] NULL,
	[c2Type] [int] NULL,
	[c2Amount] [int] NULL,
	[c3Type] [int] NULL,
	[c3Amount] [int] NULL,
	[c4Type] [int] NULL,
	[c4Amount] [int] NULL,
	[c5Type] [int] NULL,
	[c5Amount] [int] NULL,
	[c6Type] [int] NULL,
	[c6Amount] [int] NULL,
	[c7Type] [int] NULL,
	[c7Amount] [int] NULL,
	[c8Type] [int] NULL,
	[c8Amount] [int] NULL,
	[c9Type] [int] NULL,
	[c9Amount] [int] NULL,
	[c10Type] [int] NULL,
	[c10Amount] [int] NULL,
	[c11Type] [int] NULL,
	[c11Amount] [int] NULL,
	[c12Type] [int] NULL,
	[c12Amount] [int] NULL,
	[c13Type] [int] NULL,
	[c13Amount] [int] NULL,
	[c14Type] [int] NULL,
	[c14Amount] [int] NULL,
	[c15Type] [int] NULL,
	[c15Amount] [int] NULL,
	[c16Type] [int] NULL,
	[c16Amount] [int] NULL,
	[c17Type] [int] NULL,
	[c17Amount] [int] NULL,
	[c18Type] [int] NULL,
	[c18Amount] [int] NULL,
	[c19Type] [int] NULL,
	[c19Amount] [int] NULL,
	[c20Type] [int] NULL,
	[c20Amount] [int] NULL,
	[c21Type] [int] NULL,
	[c21Amount] [int] NULL,
	[c22Type] [int] NULL,
	[c22Amount] [int] NULL,
	[c23Type] [int] NULL,
	[c23Amount] [int] NULL,
	[c24Type] [int] NULL,
	[c24Amount] [int] NULL,
	[c25Type] [int] NULL,
	[c25Amount] [int] NULL,
	[c26Type] [int] NULL,
	[c26Amount] [int] NULL,
	[c27Type] [int] NULL,
	[c27Amount] [int] NULL,
	[c28Type] [int] NULL,
	[c28Amount] [int] NULL,
	[c29Type] [int] NULL,
	[c29Amount] [int] NULL,
	[c30Type] [int] NULL,
	[c30Amount] [int] NULL,
	[c31Type] [int] NULL,
	[c31Amount] [int] NULL,
	[c32Type] [int] NULL,
	[c32Amount] [int] NULL,
	[c33Type] [int] NULL,
	[c33Amount] [int] NULL,
	[c34Type] [int] NULL,
	[c34Amount] [int] NULL,
	[c35Type] [int] NULL,
	[c35Amount] [int] NULL,
	[c36Type] [int] NULL,
	[c36Amount] [int] NULL,
	[c37Type] [int] NULL,
	[c37Amount] [int] NULL,
	[c38Type] [int] NULL,
	[c38Amount] [int] NULL,
	[c39Type] [int] NULL,
	[c39Amount] [int] NULL,
	[c40Type] [int] NULL,
	[c40Amount] [int] NULL,
	[c41Type] [int] NULL,
	[c41Amount] [int] NULL,
	[c42Type] [int] NULL,
	[c42Amount] [int] NULL,
	[c43Type] [int] NULL,
	[c43Amount] [int] NULL,
	[c44Type] [int] NULL,
	[c44Amount] [int] NULL,
	[c45Type] [int] NULL,
	[c45Amount] [int] NULL,
	[c46Type] [int] NULL,
	[c46Amount] [int] NULL,
	[c47Type] [int] NULL,
	[c47Amount] [int] NULL,
	[c48Type] [int] NULL,
	[c48Amount] [int] NULL,
	[c49Type] [int] NULL,
	[c49Amount] [int] NULL,
	[c50Type] [int] NULL,
	[c50Amount] [int] NULL,
	[c51Type] [int] NULL,
	[c51Amount] [int] NULL,
	[c52Type] [int] NULL,
	[c52Amount] [int] NULL,
	[c53Type] [int] NULL,
	[c53Amount] [int] NULL,
	[c54Type] [int] NULL,
	[c54Amount] [int] NULL,
	[c55Type] [int] NULL,
	[c55Amount] [int] NULL,
	[c56Type] [int] NULL,
	[c56Amount] [int] NULL,
	[c57Type] [int] NULL,
	[c57Amount] [int] NULL,
	[c58Type] [int] NULL,
	[c58Amount] [int] NULL,
	[c59Type] [int] NULL,
	[c59Amount] [int] NULL,
	[c60Type] [int] NULL,
	[c60Amount] [int] NULL,
	[c61Type] [int] NULL,
	[c61Amount] [int] NULL,
	[c62Type] [int] NULL,
	[c62Amount] [int] NULL,
	[c63Type] [int] NULL,
	[c63Amount] [int] NULL,
	[c64Type] [int] NULL,
	[c64Amount] [int] NULL,
	[c65Type] [int] NULL,
	[c65Amount] [int] NULL,
	[c66Type] [int] NULL,
	[c66Amount] [int] NULL,
	[c67Type] [int] NULL,
	[c67Amount] [int] NULL,
	[c68Type] [int] NULL,
	[c68Amount] [int] NULL,
	[c69Type] [int] NULL,
	[c69Amount] [int] NULL,
	[c70Type] [int] NULL,
	[c70Amount] [int] NULL,
	[c71Type] [int] NULL,
	[c71Amount] [int] NULL,
	[c72Type] [int] NULL,
	[c72Amount] [int] NULL,
	[c73Type] [int] NULL,
	[c73Amount] [int] NULL,
	[c74Type] [int] NULL,
	[c74Amount] [int] NULL,
	[c75Type] [int] NULL,
	[c75Amount] [int] NULL,
	[c76Type] [int] NULL,
	[c76Amount] [int] NULL,
	[c77Type] [int] NULL,
	[c77Amount] [int] NULL,
	[c78Type] [int] NULL,
	[c78Amount] [int] NULL,
	[c79Type] [int] NULL,
	[c79Amount] [int] NULL,
	[c80Type] [int] NULL,
	[c80Amount] [int] NULL,
	[c81Type] [int] NULL,
	[c81Amount] [int] NULL,
	[c82Type] [int] NULL,
	[c82Amount] [int] NULL,
	[c83Type] [int] NULL,
	[c83Amount] [int] NULL,
	[c84Type] [int] NULL,
	[c84Amount] [int] NULL,
	[c85Type] [int] NULL,
	[c85Amount] [int] NULL,
	[c86Type] [int] NULL,
	[c86Amount] [int] NULL,
	[c87Type] [int] NULL,
	[c87Amount] [int] NULL,
	[c88Type] [int] NULL,
	[c88Amount] [int] NULL,
	[c89Type] [int] NULL,
	[c89Amount] [int] NULL,
	[c90Type] [int] NULL,
	[c90Amount] [int] NULL,
	[c91Type] [int] NULL,
	[c91Amount] [int] NULL,
	[c92Type] [int] NULL,
	[c92Amount] [int] NULL,
	[c93Type] [int] NULL,
	[c93Amount] [int] NULL,
	[c94Type] [int] NULL,
	[c94Amount] [int] NULL,
	[c95Type] [int] NULL,
	[c95Amount] [int] NULL,
	[c96Type] [int] NULL,
	[c96Amount] [int] NULL,
	[c97Type] [int] NULL,
	[c97Amount] [int] NULL,
	[c98Type] [int] NULL,
	[c98Amount] [int] NULL,
	[c99Type] [int] NULL,
	[c99Amount] [int] NULL,
	[c100Type] [int] NULL,
	[c100Amount] [int] NULL,
	[c101Type] [int] NULL,
	[c101Amount] [int] NULL,
	[c102Type] [int] NULL,
	[c102Amount] [int] NULL,
	[c103Type] [int] NULL,
	[c103Amount] [int] NULL,
	[c104Type] [int] NULL,
	[c104Amount] [int] NULL,
	[c105Type] [int] NULL,
	[c105Amount] [int] NULL,
	[c106Type] [int] NULL,
	[c106Amount] [int] NULL,
	[c107Type] [int] NULL,
	[c107Amount] [int] NULL,
	[c108Type] [int] NULL,
	[c108Amount] [int] NULL,
	[c109Type] [int] NULL,
	[c109Amount] [int] NULL,
	[c110Type] [int] NULL,
	[c110Amount] [int] NULL,
	[c111Type] [int] NULL,
	[c111Amount] [int] NULL,
	[c112Type] [int] NULL,
	[c112Amount] [int] NULL,
	[c113Type] [int] NULL,
	[c113Amount] [int] NULL,
	[c114Type] [int] NULL,
	[c114Amount] [int] NULL,
	[c115Type] [int] NULL,
	[c115Amount] [int] NULL,
	[c116Type] [int] NULL,
	[c116Amount] [int] NULL,
	[c117Type] [int] NULL,
	[c117Amount] [int] NULL,
	[c118Type] [int] NULL,
	[c118Amount] [int] NULL,
	[c119Type] [int] NULL,
	[c119Amount] [int] NULL,
	[c120Type] [int] NULL,
	[c120Amount] [int] NULL,
	[c121Type] [int] NULL,
	[c121Amount] [int] NULL,
	[c122Type] [int] NULL,
	[c122Amount] [int] NULL,
	[c123Type] [int] NULL,
	[c123Amount] [int] NULL,
	[c124Type] [int] NULL,
	[c124Amount] [int] NULL,
	[c125Type] [int] NULL,
	[c125Amount] [int] NULL,
	[c126Type] [int] NULL,
	[c126Amount] [int] NULL,
	[c127Type] [int] NULL,
	[c127Amount] [int] NULL,
	[c128Type] [int] NULL,
	[c128Amount] [int] NULL,
	[c129Type] [int] NULL,
	[c129Amount] [int] NULL,
	[c130Type] [int] NULL,
	[c130Amount] [int] NULL,
	[c131Type] [int] NULL,
	[c131Amount] [int] NULL,
	[c132Type] [int] NULL,
	[c132Amount] [int] NULL,
	[c133Type] [int] NULL,
	[c133Amount] [int] NULL,
	[c134Type] [int] NULL,
	[c134Amount] [int] NULL,
	[c135Type] [int] NULL,
	[c135Amount] [int] NULL,
	[c136Type] [int] NULL,
	[c136Amount] [int] NULL,
	[c137Type] [int] NULL,
	[c137Amount] [int] NULL,
	[c138Type] [int] NULL,
	[c138Amount] [int] NULL,
	[c139Type] [int] NULL,
	[c139Amount] [int] NULL,
	[c140Type] [int] NULL,
	[c140Amount] [int] NULL,
	[c141Type] [int] NULL,
	[c141Amount] [int] NULL,
	[c142Type] [int] NULL,
	[c142Amount] [int] NULL,
	[c143Type] [int] NULL,
	[c143Amount] [int] NULL,
	[c144Type] [int] NULL,
	[c144Amount] [int] NULL,
	[c145Type] [int] NULL,
	[c145Amount] [int] NULL,
	[c146Type] [int] NULL,
	[c146Amount] [int] NULL,
	[c147Type] [int] NULL,
	[c147Amount] [int] NULL,
	[c148Type] [int] NULL,
	[c148Amount] [int] NULL,
	[c149Type] [int] NULL,
	[c149Amount] [int] NULL,
	[c150Type] [int] NULL,
	[c150Amount] [int] NULL,
	[c151Type] [int] NULL,
	[c151Amount] [int] NULL,
	[c152Type] [int] NULL,
	[c152Amount] [int] NULL,
	[c153Type] [int] NULL,
	[c153Amount] [int] NULL,
	[c154Type] [int] NULL,
	[c154Amount] [int] NULL,
	[c155Type] [int] NULL,
	[c155Amount] [int] NULL,
	[c156Type] [int] NULL,
	[c156Amount] [int] NULL,
	[c157Type] [int] NULL,
	[c157Amount] [int] NULL,
	[c158Type] [int] NULL,
	[c158Amount] [int] NULL,
	[c159Type] [int] NULL,
	[c159Amount] [int] NULL,
	[c160Type] [int] NULL,
	[c160Amount] [int] NULL,
	[c161Type] [int] NULL,
	[c161Amount] [int] NULL,
	[c162Type] [int] NULL,
	[c162Amount] [int] NULL,
	[c163Type] [int] NULL,
	[c163Amount] [int] NULL,
	[c164Type] [int] NULL,
	[c164Amount] [int] NULL,
	[c165Type] [int] NULL,
	[c165Amount] [int] NULL,
	[c166Type] [int] NULL,
	[c166Amount] [int] NULL,
	[c167Type] [int] NULL,
	[c167Amount] [int] NULL,
	[c168Type] [int] NULL,
	[c168Amount] [int] NULL,
	[c169Type] [int] NULL,
	[c169Amount] [int] NULL,
	[c170Type] [int] NULL,
	[c170Amount] [int] NULL,
	[c171Type] [int] NULL,
	[c171Amount] [int] NULL,
	[c172Type] [int] NULL,
	[c172Amount] [int] NULL,
	[c173Type] [int] NULL,
	[c173Amount] [int] NULL,
	[c174Type] [int] NULL,
	[c174Amount] [int] NULL,
	[c175Type] [int] NULL,
	[c175Amount] [int] NULL,
	[c176Type] [int] NULL,
	[c176Amount] [int] NULL,
	[c177Type] [int] NULL,
	[c177Amount] [int] NULL,
	[c178Type] [int] NULL,
	[c178Amount] [int] NULL,
	[c179Type] [int] NULL,
	[c179Amount] [int] NULL,
	[c180Type] [int] NULL,
	[c180Amount] [int] NULL,
	[c181Type] [int] NULL,
	[c181Amount] [int] NULL,
	[c182Type] [int] NULL,
	[c182Amount] [int] NULL,
	[c183Type] [int] NULL,
	[c183Amount] [int] NULL,
	[c184Type] [int] NULL,
	[c184Amount] [int] NULL,
	[c185Type] [int] NULL,
	[c185Amount] [int] NULL,
	[c186Type] [int] NULL,
	[c186Amount] [int] NULL,
	[c187Type] [int] NULL,
	[c187Amount] [int] NULL,
	[c188Type] [int] NULL,
	[c188Amount] [int] NULL,
	[c189Type] [int] NULL,
	[c189Amount] [int] NULL,
	[c190Type] [int] NULL,
	[c190Amount] [int] NULL,
	[c191Type] [int] NULL,
	[c191Amount] [int] NULL,
	[c192Type] [int] NULL,
	[c192Amount] [int] NULL,
	[c193Type] [int] NULL,
	[c193Amount] [int] NULL,
	[c194Type] [int] NULL,
	[c194Amount] [int] NULL,
	[c195Type] [int] NULL,
	[c195Amount] [int] NULL,
	[c196Type] [int] NULL,
	[c196Amount] [int] NULL,
	[c197Type] [int] NULL,
	[c197Amount] [int] NULL,
	[c198Type] [int] NULL,
	[c198Amount] [int] NULL,
	[c199Type] [int] NULL,
	[c199Amount] [int] NULL,
	[c200Type] [int] NULL,
	[c200Amount] [int] NULL,
	[c201Type] [int] NULL,
	[c201Amount] [int] NULL,
	[c202Type] [int] NULL,
	[c202Amount] [int] NULL,
	[c203Type] [int] NULL,
	[c203Amount] [int] NULL,
	[c204Type] [int] NULL,
	[c204Amount] [int] NULL,
	[c205Type] [int] NULL,
	[c205Amount] [int] NULL,
	[c206Type] [int] NULL,
	[c206Amount] [int] NULL,
	[c207Type] [int] NULL,
	[c207Amount] [int] NULL,
	[c208Type] [int] NULL,
	[c208Amount] [int] NULL,
	[c209Type] [int] NULL,
	[c209Amount] [int] NULL,
	[c210Type] [int] NULL,
	[c210Amount] [int] NULL,
	[c211Type] [int] NULL,
	[c211Amount] [int] NULL,
	[c212Type] [int] NULL,
	[c212Amount] [int] NULL,
	[c213Type] [int] NULL,
	[c213Amount] [int] NULL,
	[c214Type] [int] NULL,
	[c214Amount] [int] NULL,
	[c215Type] [int] NULL,
	[c215Amount] [int] NULL,
	[c216Type] [int] NULL,
	[c216Amount] [int] NULL,
	[c217Type] [int] NULL,
	[c217Amount] [int] NULL,
	[c218Type] [int] NULL,
	[c218Amount] [int] NULL,
	[c219Type] [int] NULL,
	[c219Amount] [int] NULL,
	[c220Type] [int] NULL,
	[c220Amount] [int] NULL,
	[c221Type] [int] NULL,
	[c221Amount] [int] NULL,
	[c222Type] [int] NULL,
	[c222Amount] [int] NULL,
	[c223Type] [int] NULL,
	[c223Amount] [int] NULL,
	[c224Type] [int] NULL,
	[c224Amount] [int] NULL,
	[c225Type] [int] NULL,
	[c225Amount] [int] NULL,
	[c226Type] [int] NULL,
	[c226Amount] [int] NULL,
	[c227Type] [int] NULL,
	[c227Amount] [int] NULL,
	[c228Type] [int] NULL,
	[c228Amount] [int] NULL,
	[c229Type] [int] NULL,
	[c229Amount] [int] NULL,
	[c230Type] [int] NULL,
	[c230Amount] [int] NULL,
	[c231Type] [int] NULL,
	[c231Amount] [int] NULL,
	[c232Type] [int] NULL,
	[c232Amount] [int] NULL,
	[c233Type] [int] NULL,
	[c233Amount] [int] NULL,
	[c234Type] [int] NULL,
	[c234Amount] [int] NULL,
	[c235Type] [int] NULL,
	[c235Amount] [int] NULL,
	[c236Type] [int] NULL,
	[c236Amount] [int] NULL,
	[c237Type] [int] NULL,
	[c237Amount] [int] NULL,
	[c238Type] [int] NULL,
	[c238Amount] [int] NULL,
	[c239Type] [int] NULL,
	[c239Amount] [int] NULL,
	[c240Type] [int] NULL,
	[c240Amount] [int] NULL,
	[c241Type] [int] NULL,
	[c241Amount] [int] NULL,
	[c242Type] [int] NULL,
	[c242Amount] [int] NULL,
	[c243Type] [int] NULL,
	[c243Amount] [int] NULL,
	[c244Type] [int] NULL,
	[c244Amount] [int] NULL,
	[c245Type] [int] NULL,
	[c245Amount] [int] NULL,
	[c246Type] [int] NULL,
	[c246Amount] [int] NULL,
	[c247Type] [int] NULL,
	[c247Amount] [int] NULL,
	[c248Type] [int] NULL,
	[c248Amount] [int] NULL,
	[c249Type] [int] NULL,
	[c249Amount] [int] NULL,
	[c250Type] [int] NULL,
	[c250Amount] [int] NULL,
	[c251Type] [int] NULL,
	[c251Amount] [int] NULL,
	[c252Type] [int] NULL,
	[c252Amount] [int] NULL,
	[c253Type] [int] NULL,
	[c253Amount] [int] NULL,
	[c254Type] [int] NULL,
	[c254Amount] [int] NULL,
	[c255Type] [int] NULL,
	[c255Amount] [int] NULL,
	[c256Type] [int] NULL,
	[c256Amount] [int] NULL,
	[c257Type] [int] NULL,
	[c257Amount] [int] NULL,
	[c258Type] [int] NULL,
	[c258Amount] [int] NULL,
	[c259Type] [int] NULL,
	[c259Amount] [int] NULL,
	[c260Type] [int] NULL,
	[c260Amount] [int] NULL,
	[c261Type] [int] NULL,
	[c261Amount] [int] NULL,
	[c262Type] [int] NULL,
	[c262Amount] [int] NULL,
	[c263Type] [int] NULL,
	[c263Amount] [int] NULL,
	[c264Type] [int] NULL,
	[c264Amount] [int] NULL,
	[c265Type] [int] NULL,
	[c265Amount] [int] NULL,
	[c266Type] [int] NULL,
	[c266Amount] [int] NULL,
	[c267Type] [int] NULL,
	[c267Amount] [int] NULL,
	[c268Type] [int] NULL,
	[c268Amount] [int] NULL,
	[c269Type] [int] NULL,
	[c269Amount] [int] NULL,
	[c270Type] [int] NULL,
	[c270Amount] [int] NULL,
	[c271Type] [int] NULL,
	[c271Amount] [int] NULL,
	[c272Type] [int] NULL,
	[c272Amount] [int] NULL,
	[c273Type] [int] NULL,
	[c273Amount] [int] NULL,
	[c274Type] [int] NULL,
	[c274Amount] [int] NULL,
	[c275Type] [int] NULL,
	[c275Amount] [int] NULL,
	[c276Type] [int] NULL,
	[c276Amount] [int] NULL,
	[c277Type] [int] NULL,
	[c277Amount] [int] NULL,
	[c278Type] [int] NULL,
	[c278Amount] [int] NULL,
	[c279Type] [int] NULL,
	[c279Amount] [int] NULL,
	[c280Type] [int] NULL,
	[c280Amount] [int] NULL,
	[c281Type] [int] NULL,
	[c281Amount] [int] NULL,
	[c282Type] [int] NULL,
	[c282Amount] [int] NULL,
	[c283Type] [int] NULL,
	[c283Amount] [int] NULL,
	[c284Type] [int] NULL,
	[c284Amount] [int] NULL,
	[c285Type] [int] NULL,
	[c285Amount] [int] NULL,
	[c286Type] [int] NULL,
	[c286Amount] [int] NULL,
	[c287Type] [int] NULL,
	[c287Amount] [int] NULL,
	[c288Type] [int] NULL,
	[c288Amount] [int] NULL,
	[c289Type] [int] NULL,
	[c289Amount] [int] NULL,
	[c290Type] [int] NULL,
	[c290Amount] [int] NULL,
	[c291Type] [int] NULL,
	[c291Amount] [int] NULL,
	[c292Type] [int] NULL,
	[c292Amount] [int] NULL,
	[c293Type] [int] NULL,
	[c293Amount] [int] NULL,
	[c294Type] [int] NULL,
	[c294Amount] [int] NULL,
	[c295Type] [int] NULL,
	[c295Amount] [int] NULL,
	[c296Type] [int] NULL,
	[c296Amount] [int] NULL,
	[c297Type] [int] NULL,
	[c297Amount] [int] NULL,
	[c298Type] [int] NULL,
	[c298Amount] [int] NULL,
	[c299Type] [int] NULL,
	[c299Amount] [int] NULL,
	[c300Type] [int] NULL,
	[c300Amount] [int] NULL,
	[c301Type] [int] NULL,
	[c301Amount] [int] NULL,
	[c302Type] [int] NULL,
	[c302Amount] [int] NULL,
	[c303Type] [int] NULL,
	[c303Amount] [int] NULL,
	[c304Type] [int] NULL,
	[c304Amount] [int] NULL,
	[c305Type] [int] NULL,
	[c305Amount] [int] NULL,
	[c306Type] [int] NULL,
	[c306Amount] [int] NULL,
	[c307Type] [int] NULL,
	[c307Amount] [int] NULL,
	[c308Type] [int] NULL,
	[c308Amount] [int] NULL,
	[c309Type] [int] NULL,
	[c309Amount] [int] NULL,
	[c310Type] [int] NULL,
	[c310Amount] [int] NULL,
	[c311Type] [int] NULL,
	[c311Amount] [int] NULL,
	[c312Type] [int] NULL,
	[c312Amount] [int] NULL,
	[c313Type] [int] NULL,
	[c313Amount] [int] NULL,
	[c314Type] [int] NULL,
	[c314Amount] [int] NULL,
	[c315Type] [int] NULL,
	[c315Amount] [int] NULL,
	[c316Type] [int] NULL,
	[c316Amount] [int] NULL,
	[c317Type] [int] NULL,
	[c317Amount] [int] NULL,
	[c318Type] [int] NULL,
	[c318Amount] [int] NULL,
	[c319Type] [int] NULL,
	[c319Amount] [int] NULL,
	[c320Type] [int] NULL,
	[c320Amount] [int] NULL,
	[c321Type] [int] NULL,
	[c321Amount] [int] NULL,
	[c322Type] [int] NULL,
	[c322Amount] [int] NULL,
	[c323Type] [int] NULL,
	[c323Amount] [int] NULL,
	[c324Type] [int] NULL,
	[c324Amount] [int] NULL,
	[c325Type] [int] NULL,
	[c325Amount] [int] NULL,
	[c326Type] [int] NULL,
	[c326Amount] [int] NULL,
	[c327Type] [int] NULL,
	[c327Amount] [int] NULL,
	[c328Type] [int] NULL,
	[c328Amount] [int] NULL,
	[c329Type] [int] NULL,
	[c329Amount] [int] NULL,
	[c330Type] [int] NULL,
	[c330Amount] [int] NULL,
	[c331Type] [int] NULL,
	[c331Amount] [int] NULL,
	[c332Type] [int] NULL,
	[c332Amount] [int] NULL,
	[c333Type] [int] NULL,
	[c333Amount] [int] NULL,
	[c334Type] [int] NULL,
	[c334Amount] [int] NULL,
	[c335Type] [int] NULL,
	[c335Amount] [int] NULL,
	[c336Type] [int] NULL,
	[c336Amount] [int] NULL,
	[c337Type] [int] NULL,
	[c337Amount] [int] NULL,
	[c338Type] [int] NULL,
	[c338Amount] [int] NULL,
	[c339Type] [int] NULL,
	[c339Amount] [int] NULL,
	[c340Type] [int] NULL,
	[c340Amount] [int] NULL,
	[c341Type] [int] NULL,
	[c341Amount] [int] NULL,
	[c342Type] [int] NULL,
	[c342Amount] [int] NULL,
	[c343Type] [int] NULL,
	[c343Amount] [int] NULL,
	[c344Type] [int] NULL,
	[c344Amount] [int] NULL,
	[c345Type] [int] NULL,
	[c345Amount] [int] NULL,
	[c346Type] [int] NULL,
	[c346Amount] [int] NULL,
	[c347Type] [int] NULL,
	[c347Amount] [int] NULL,
	[c348Type] [int] NULL,
	[c348Amount] [int] NULL,
	[c349Type] [int] NULL,
	[c349Amount] [int] NULL,
	[c350Type] [int] NULL,
	[c350Amount] [int] NULL,
	[c351Type] [int] NULL,
	[c351Amount] [int] NULL,
	[c352Type] [int] NULL,
	[c352Amount] [int] NULL,
	[c353Type] [int] NULL,
	[c353Amount] [int] NULL,
	[c354Type] [int] NULL,
	[c354Amount] [int] NULL,
	[c355Type] [int] NULL,
	[c355Amount] [int] NULL,
	[c356Type] [int] NULL,
	[c356Amount] [int] NULL,
	[c357Type] [int] NULL,
	[c357Amount] [int] NULL,
	[c358Type] [int] NULL,
	[c358Amount] [int] NULL,
	[c359Type] [int] NULL,
	[c359Amount] [int] NULL,
	[c360Type] [int] NULL,
	[c360Amount] [int] NULL,
	[c361Type] [int] NULL,
	[c361Amount] [int] NULL,
	[c362Type] [int] NULL,
	[c362Amount] [int] NULL,
	[c363Type] [int] NULL,
	[c363Amount] [int] NULL,
	[c364Type] [int] NULL,
	[c364Amount] [int] NULL,
	[c365Type] [int] NULL,
	[c365Amount] [int] NULL,
	[c366Type] [int] NULL,
	[c366Amount] [int] NULL,
	[c367Type] [int] NULL,
	[c367Amount] [int] NULL,
	[c368Type] [int] NULL,
	[c368Amount] [int] NULL,
	[c369Type] [int] NULL,
	[c369Amount] [int] NULL,
	[c370Type] [int] NULL,
	[c370Amount] [int] NULL,
	[c371Type] [int] NULL,
	[c371Amount] [int] NULL,
	[c372Type] [int] NULL,
	[c372Amount] [int] NULL,
	[c373Type] [int] NULL,
	[c373Amount] [int] NULL,
	[c374Type] [int] NULL,
	[c374Amount] [int] NULL,
	[c375Type] [int] NULL,
	[c375Amount] [int] NULL,
	[c376Type] [int] NULL,
	[c376Amount] [int] NULL,
	[c377Type] [int] NULL,
	[c377Amount] [int] NULL,
	[c378Type] [int] NULL,
	[c378Amount] [int] NULL,
	[c379Type] [int] NULL,
	[c379Amount] [int] NULL,
	[c380Type] [int] NULL,
	[c380Amount] [int] NULL,
	[c381Type] [int] NULL,
	[c381Amount] [int] NULL,
	[c382Type] [int] NULL,
	[c382Amount] [int] NULL,
	[c383Type] [int] NULL,
	[c383Amount] [int] NULL,
	[c384Type] [int] NULL,
	[c384Amount] [int] NULL,
	[c385Type] [int] NULL,
	[c385Amount] [int] NULL,
	[c386Type] [int] NULL,
	[c386Amount] [int] NULL,
	[c387Type] [int] NULL,
	[c387Amount] [int] NULL,
	[c388Type] [int] NULL,
	[c388Amount] [int] NULL,
	[c389Type] [int] NULL,
	[c389Amount] [int] NULL,
	[c390Type] [int] NULL,
	[c390Amount] [int] NULL,
	[c391Type] [int] NULL,
	[c391Amount] [int] NULL,
	[c392Type] [int] NULL,
	[c392Amount] [int] NULL,
	[c393Type] [int] NULL,
	[c393Amount] [int] NULL,
	[c394Type] [int] NULL,
	[c394Amount] [int] NULL,
	[c395Type] [int] NULL,
	[c395Amount] [int] NULL,
	[c396Type] [int] NULL,
	[c396Amount] [int] NULL,
	[c397Type] [int] NULL,
	[c397Amount] [int] NULL,
	[c398Type] [int] NULL,
	[c398Amount] [int] NULL,
	[c399Type] [int] NULL,
	[c399Amount] [int] NULL,
	[c400Type] [int] NULL,
	[c400Amount] [int] NULL,
	[c401Type] [int] NULL,
	[c401Amount] [int] NULL,
	[c402Type] [int] NULL,
	[c402Amount] [int] NULL,
	[c403Type] [int] NULL,
	[c403Amount] [int] NULL,
	[c404Type] [int] NULL,
	[c404Amount] [int] NULL,
	[c405Type] [int] NULL,
	[c405Amount] [int] NULL,
	[c406Type] [int] NULL,
	[c406Amount] [int] NULL,
	[c407Type] [int] NULL,
	[c407Amount] [int] NULL,
	[c408Type] [int] NULL,
	[c408Amount] [int] NULL,
	[c409Type] [int] NULL,
	[c409Amount] [int] NULL,
	[c410Type] [int] NULL,
	[c410Amount] [int] NULL,
	[c411Type] [int] NULL,
	[c411Amount] [int] NULL,
	[c412Type] [int] NULL,
	[c412Amount] [int] NULL,
	[c413Type] [int] NULL,
	[c413Amount] [int] NULL,
	[c414Type] [int] NULL,
	[c414Amount] [int] NULL,
	[c415Type] [int] NULL,
	[c415Amount] [int] NULL,
	[c416Type] [int] NULL,
	[c416Amount] [int] NULL,
	[c417Type] [int] NULL,
	[c417Amount] [int] NULL,
	[c418Type] [int] NULL,
	[c418Amount] [int] NULL,
	[c419Type] [int] NULL,
	[c419Amount] [int] NULL,
	[c420Type] [int] NULL,
	[c420Amount] [int] NULL,
	[c421Type] [int] NULL,
	[c421Amount] [int] NULL,
	[c422Type] [int] NULL,
	[c422Amount] [int] NULL,
	[c423Type] [int] NULL,
	[c423Amount] [int] NULL,
	[c424Type] [int] NULL,
	[c424Amount] [int] NULL,
	[c425Type] [int] NULL,
	[c425Amount] [int] NULL,
	[c426Type] [int] NULL,
	[c426Amount] [int] NULL,
	[c427Type] [int] NULL,
	[c427Amount] [int] NULL,
	[c428Type] [int] NULL,
	[c428Amount] [int] NULL,
	[c429Type] [int] NULL,
	[c429Amount] [int] NULL,
	[c430Type] [int] NULL,
	[c430Amount] [int] NULL,
	[c431Type] [int] NULL,
	[c431Amount] [int] NULL,
	[c432Type] [int] NULL,
	[c432Amount] [int] NULL,
	[c433Type] [int] NULL,
	[c433Amount] [int] NULL,
	[c434Type] [int] NULL,
	[c434Amount] [int] NULL,
	[c435Type] [int] NULL,
	[c435Amount] [int] NULL,
	[c436Type] [int] NULL,
	[c436Amount] [int] NULL,
	[c437Type] [int] NULL,
	[c437Amount] [int] NULL,
	[c438Type] [int] NULL,
	[c438Amount] [int] NULL,
	[c439Type] [int] NULL,
	[c439Amount] [int] NULL,
	[c440Type] [int] NULL,
	[c440Amount] [int] NULL,
	[c441Type] [int] NULL,
	[c441Amount] [int] NULL,
	[c442Type] [int] NULL,
	[c442Amount] [int] NULL,
	[c443Type] [int] NULL,
	[c443Amount] [int] NULL,
	[c444Type] [int] NULL,
	[c444Amount] [int] NULL,
	[c445Type] [int] NULL,
	[c445Amount] [int] NULL,
	[c446Type] [int] NULL,
	[c446Amount] [int] NULL,
	[c447Type] [int] NULL,
	[c447Amount] [int] NULL,
	[c448Type] [int] NULL,
	[c448Amount] [int] NULL,
	[c449Type] [int] NULL,
	[c449Amount] [int] NULL,
 CONSTRAINT [PK_InvRecipeInvention] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[InvSalvage0]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[InvSalvage0](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[S_MysticElement] [int] NULL,
	[S_MysticFoci] [int] NULL,
	[S_ArcaneEssence] [int] NULL,
	[S_ArcaneGlyph] [int] NULL,
	[S_MagicalWard] [int] NULL,
	[S_MagicalArtifact] [int] NULL,
	[S_TechMaterial] [int] NULL,
	[S_TechPower] [int] NULL,
	[S_TechHardware] [int] NULL,
	[S_TechPrototype] [int] NULL,
	[S_TechSoftware] [int] NULL,
	[S_ExperiementalTech] [int] NULL,
	[S_Carapace] [int] NULL,
	[S_AbberantTech] [int] NULL,
	[S_ArachnosGun] [int] NULL,
	[S_ArmorShard] [int] NULL,
	[S_BlasterTech] [int] NULL,
	[S_SpiderEye] [int] NULL,
	[S_Amulet] [int] NULL,
	[S_CrystalCodexFragments] [int] NULL,
	[S_Demonlogica] [int] NULL,
	[S_Potions] [int] NULL,
	[S_SpiritGuide] [int] NULL,
	[S_UrMetal] [int] NULL,
	[S_BrokenMask] [int] NULL,
	[S_Sigil] [int] NULL,
	[S_Fetish] [int] NULL,
	[S_VerminousVictuals] [int] NULL,
	[S_Spark] [int] NULL,
	[S_StaticCharge] [int] NULL,
	[S_Charms] [int] NULL,
	[S_Cloak] [int] NULL,
	[S_DaemonicChain] [int] NULL,
	[S_Dagger] [int] NULL,
	[S_Orichalcum] [int] NULL,
	[S_Ring] [int] NULL,
	[S_Spells] [int] NULL,
	[S_Talisman] [int] NULL,
	[S_ThornFragment] [int] NULL,
	[S_PsionicReceptacle] [int] NULL,
	[S_PsionizedMetal] [int] NULL,
	[S_BloodSample] [int] NULL,
	[S_CeramicCompound] [int] NULL,
	[S_ExoticAlloy] [int] NULL,
	[S_ExoticCompound] [int] NULL,
	[S_Lens] [int] NULL,
	[S_StableProtonium] [int] NULL,
	[S_CoralaxBlood] [int] NULL,
	[S_CoralShard] [int] NULL,
	[S_FishScale] [int] NULL,
	[S_BodyArmorFragment] [int] NULL,
	[S_BrokenCreyRifle] [int] NULL,
	[S_CommunicationDevice] [int] NULL,
	[S_CreyTech] [int] NULL,
	[S_GhostTrinket] [int] NULL,
	[S_PhantomTears] [int] NULL,
	[S_Scrap] [int] NULL,
	[S_EbonClaw] [int] NULL,
	[S_ScentofBrimstone] [int] NULL,
	[S_DevouringCulture] [int] NULL,
	[S_HamidonLichen] [int] NULL,
	[S_BlackMarketSuperGear] [int] NULL,
	[S_FirBolgStraw] [int] NULL,
	[S_EnergySource] [int] NULL,
	[S_FreakshowCybernetics] [int] NULL,
	[S_Pelt] [int] NULL,
	[S_BioSample] [int] NULL,
	[S_AeonTech] [int] NULL,
	[S_GoldPlatedSkin] [int] NULL,
	[S_LevDisk] [int] NULL,
	[S_Hex] [int] NULL,
	[S_Tatoo] [int] NULL,
	[S_GeneticSample] [int] NULL,
	[S_KheldianTech] [int] NULL,
	[S_DreadTramalian] [int] NULL,
	[S_FaLakAmulet] [int] NULL,
	[S_RylehsRain] [int] NULL,
	[S_SpielmansSignet] [int] NULL,
	[S_LudditePouch] [int] NULL,
	[S_SaintsMedallion] [int] NULL,
	[S_Notes] [int] NULL,
	[S_MutatedSample] [int] NULL,
	[S_NeoOrganics] [int] NULL,
	[S_Ectoplasm] [int] NULL,
	[S_EctoplasmicResidue] [int] NULL,
	[S_BabbageEngine] [int] NULL,
	[S_NemesisWeapon] [int] NULL,
	[S_SteamTechImplant] [int] NULL,
	[S_VacuumCircuits] [int] NULL,
	[S_NictusMemento] [int] NULL,
	[S_NictusTech] [int] NULL,
	[S_Coins] [int] NULL,
	[S_BrokenRiktiWeapon] [int] NULL,
	[S_CortexDevice] [int] NULL,
	[S_RiktiArmorFragment] [int] NULL,
	[S_RiktiCommunicationsDevice] [int] NULL,
	[S_RiktiControlUnit] [int] NULL,
	[S_RiktiPlasmaRifle] [int] NULL,
	[S_ToolsoftheTrade] [int] NULL,
	[S_ShivanEctoplasm] [int] NULL,
	[S_CrystalSkull] [int] NULL,
	[S_SkyRaiderAntiGravUnit] [int] NULL,
	[S_SkyRaiderDevice] [int] NULL,
	[S_SkyRaiderWeapon] [int] NULL,
	[S_SnakeFang] [int] NULL,
	[S_SnakeVenom] [int] NULL,
	[S_AstrologyBook] [int] NULL,
	[S_DaemonHeart] [int] NULL,
	[S_RiktiDataPad] [int] NULL,
	[S_RiktiPlasmaWeapon] [int] NULL,
	[S_Saber] [int] NULL,
	[S_BlackBox] [int] NULL,
	[S_BrokenCreyPistol] [int] NULL,
	[S_CyberneticCharger] [int] NULL,
	[S_DangerousChemicals] [int] NULL,
	[S_EChip] [int] NULL,
	[S_GraphiteComposite] [int] NULL,
	[S_MercuryCircuits] [int] NULL,
	[S_Nanites] [int] NULL,
	[S_PortalTech] [int] NULL,
	[S_PoweredArmorCircuitry] [int] NULL,
	[S_Titanium] [int] NULL,
	[S_InnovativeCode] [int] NULL,
	[S_AncestralWeapon] [int] NULL,
	[S_ArcanePowder] [int] NULL,
	[S_EnchantedWeapon] [int] NULL,
	[S_Etherium] [int] NULL,
	[S_TsooInk] [int] NULL,
	[S_Furs] [int] NULL,
	[S_CyberneticImplant] [int] NULL,
	[S_SyntheticOrgans] [int] NULL,
	[S_UnknownChemicals] [int] NULL,
	[S_Claw] [int] NULL,
	[S_AncientWeapon] [int] NULL,
	[S_Bracer] [int] NULL,
	[S_IceShard] [int] NULL,
	[S_Headset] [int] NULL,
	[S_TrickArrows] [int] NULL,
	[S_TissueSample] [int] NULL,
	[S_ArachnoidBlood] [int] NULL,
	[S_ArachnoidVenom] [int] NULL,
	[S_CIAFiles] [int] NULL,
	[S_HamidonSpore] [int] NULL,
	[S_Glittershrooms] [int] NULL,
	[S_PumpkinBomb] [int] NULL,
	[S_PumpkinShard] [int] NULL,
	[S_KheldianBloodSample] [int] NULL,
	[S_ElementalManual] [int] NULL,
	[S_Treatise] [int] NULL,
	[S_Hatchet] [int] NULL,
	[S_NemesisStaff] [int] NULL,
	[S_HellishTooth] [int] NULL,
	[S_NictusAmmo] [int] NULL,
	[S_NecklaceofTeeth] [int] NULL,
	[S_RedcapPouch] [int] NULL,
	[S_Shillelagh] [int] NULL,
	[S_DNAMutation] [int] NULL,
	[S_BoneFragment] [int] NULL,
	[S_MilitaryIntelligence] [int] NULL,
	[S_DataFiles] [int] NULL,
	[S_ParagonPoliceFiles] [int] NULL,
	[S_SuperadineExtract] [int] NULL,
	[S_Scrolls] [int] NULL,
	[S_SyntheticDrug] [int] NULL,
	[S_BookOfBlood] [int] NULL,
	[S_WitchsHat] [int] NULL,
	[S_SlagCulture] [int] NULL,
	[S_Magma] [int] NULL,
	[S_Pumice] [int] NULL,
	[S_MeatCleaver] [int] NULL,
	[S_ExperimentalTech] [int] NULL,
	[S_SpellsOfPower] [int] NULL,
	[S_UnstableRadPistol] [int] NULL,
	[S_WeaponOfMu] [int] NULL,
	[S_AlienTech] [int] NULL,
	[S_NanoFluid] [int] NULL,
	[S_SignatureSalvage] [int] NULL,
	[S_SignatureSalvageU] [int] NULL,
	[S_AccessBypass_MM] [int] NULL,
	[S_PrototypeElement_MM] [int] NULL,
	[S_HumanBloodSample] [int] NULL,
	[S_InanimateCarbonRod] [int] NULL,
	[S_ComputerVirus] [int] NULL,
	[S_SimpleChemical] [int] NULL,
	[S_Brass] [int] NULL,
	[S_Boresight] [int] NULL,
	[S_ImprovisedCybernetic] [int] NULL,
	[S_InertGas] [int] NULL,
	[S_ScientificTheory] [int] NULL,
	[S_CircuitBoard] [int] NULL,
	[S_StabilizedMutantGenome] [int] NULL,
	[S_Iron] [int] NULL,
	[S_KineticWeapon] [int] NULL,
	[S_TemporalAnalyzer] [int] NULL,
	[S_CeramicArmorPlate] [int] NULL,
	[S_HydraulicPiston] [int] NULL,
	[S_Silver] [int] NULL,
	[S_MathematicProof] [int] NULL,
	[S_MutantBloodSample] [int] NULL,
	[S_HeavyWater] [int] NULL,
	[S_DaemonProgram] [int] NULL,
	[S_ChemicalFormula] [int] NULL,
	[S_Polycarbon] [int] NULL,
	[S_Scope] [int] NULL,
	[S_CommercialCybernetic] [int] NULL,
	[S_CorrosiveGas] [int] NULL,
	[S_ScientificLaw] [int] NULL,
	[S_DataDrive] [int] NULL,
	[S_MutantDNAStrand] [int] NULL,
	[S_Steel] [int] NULL,
	[S_EnergyWeapon] [int] NULL,
	[S_TemporalTracer] [int] NULL,
	[S_TitaniumShard] [int] NULL,
	[S_PneumaticPiston] [int] NULL,
	[S_Gold] [int] NULL,
	[S_ChaosTheorem] [int] NULL,
	[S_AlienBloodSample] [int] NULL,
	[S_EnrichedPlutonium] [int] NULL,
	[S_SourceCode] [int] NULL,
	[S_ComplexChemicalFormula] [int] NULL,
	[S_PlasmaCapacitor] [int] NULL,
	[S_HeadsUpDisplay] [int] NULL,
	[S_MilitaryCybernetic] [int] NULL,
	[S_ReactiveGas] [int] NULL,
	[S_ConspiratorialEvidence] [int] NULL,
	[S_HolographicMemory] [int] NULL,
	[S_MutatingGenome] [int] NULL,
	[S_Impervium] [int] NULL,
	[S_PhotonicWeapon] [int] NULL,
	[S_ChronalSkip] [int] NULL,
	[S_RiktiAlloy] [int] NULL,
	[S_PositronicMatrix] [int] NULL,
	[S_Platinum] [int] NULL,
	[S_SyntheticIntelligenceUnit] [int] NULL,
	[S_AncientArtifact] [int] NULL,
	[S_LuckCharm] [int] NULL,
	[S_ClockworkWinder] [int] NULL,
	[S_SpellScroll] [int] NULL,
	[S_SpiritualEssence] [int] NULL,
	[S_RuneboundArmor] [int] NULL,
	[S_MasterworkWeapon] [int] NULL,
	[S_Rune] [int] NULL,
	[S_DemonicBloodSample] [int] NULL,
	[S_AlchemicalSilver] [int] NULL,
	[S_AncientBone] [int] NULL,
	[S_SpellInk] [int] NULL,
	[S_Never_MeltingIce] [int] NULL,
	[S_Fortune] [int] NULL,
	[S_Ruby] [int] NULL,
	[S_DemonicThreatReport] [int] NULL,
	[S_RegeneratingFlesh] [int] NULL,
	[S_SpiritThorn] [int] NULL,
	[S_UnearthedRelic] [int] NULL,
	[S_TemporalSands] [int] NULL,
	[S_ClockworkGear] [int] NULL,
	[S_VolumeoftheObsidianLibrum] [int] NULL,
	[S_PsionicEctoplasm] [int] NULL,
	[S_SoulboundArmor] [int] NULL,
	[S_EnscorcelledWeapon] [int] NULL,
	[S_Symbol] [int] NULL,
	[S_BloodoftheIncarnate] [int] NULL,
	[S_AlchemicalGold] [int] NULL,
	[S_CarnivalofShadowsMask] [int] NULL,
	[S_LivingTattoo] [int] NULL,
	[S_UnquenchableFlame] [int] NULL,
	[S_Destiny] [int] NULL,
	[S_Sapphire] [int] NULL,
	[S_PsionicThreatReport] [int] NULL,
	[S_BleedingStone] [int] NULL,
	[S_ThornTreeVine] [int] NULL,
	[S_LamentBox] [int] NULL,
	[S_StrandofFate] [int] NULL,
	[S_PsioniclyChargedBrass] [int] NULL,
	[S_PagefromtheMalleusMundi] [int] NULL,
	[S_PsionicManfestation] [int] NULL,
	[S_SymbioticArmor] [int] NULL,
	[S_DeificWeapon] [int] NULL,
	[S_EmpoweredSigil] [int] NULL,
	[S_BlackBloodoftheEarth] [int] NULL,
	[S_EnchantedImpervium] [int] NULL,
	[S_MuVestment] [int] NULL,
	[S_SoulTrappedGem] [int] NULL,
	[S_PangeanSoil] [int] NULL,
	[S_Prophecy] [int] NULL,
	[S_Diamond] [int] NULL,
	[S_MagicalConspiracy] [int] NULL,
	[S_EssenceoftheFuries] [int] NULL,
	[S_HamidonGoo] [int] NULL,
	[S_NevermeltingIce] [int] NULL,
	[S_StatesmanMask_H2006] [int] NULL,
	[S_LordRecluseMask_H2006] [int] NULL,
	[S_BackAlleyBrawlerGloves_H2006] [int] NULL,
	[S_HamidonCostume_H2006] [int] NULL,
	[S_VanguardMerit] [int] NULL,
	[S_CandyCane] [int] NULL,
	[S_Hero1BloodLostCure] [int] NULL,
	[S_AzuriasWandLostCure] [int] NULL,
	[S_LostSerumLostCure] [int] NULL,
	[S_CoTIncantationLostCure] [int] NULL,
	[S_GraveDirtLostCure] [int] NULL,
	[S_MeritReward] [int] NULL,
	[S_TailorDiscountCoupon] [int] NULL,
	[S_AuctionDiscountCoupon] [int] NULL,
	[S_InventionDiscountCoupon] [int] NULL,
	[S_BrainStormIdea] [int] NULL,
	[S_ArchitectTicket] [int] NULL,
	[S_AndroidCircuitry] [int] NULL,
	[S_AndroidBlaster] [int] NULL,
	[S_BrainLichen] [int] NULL,
	[S_PrimordialMoss] [int] NULL,
	[S_AndroidArmorPlate] [int] NULL,
	[S_PlagueSpores] [int] NULL,
	[S_UndamagedAndroidBrain] [int] NULL,
	[S_ProgenitorLichen] [int] NULL,
	[S_ImplosionGrenade] [int] NULL,
	[S_ExoticIsotope] [int] NULL,
	[S_IFFCodes] [int] NULL,
	[S_IFFCodeFragement] [int] NULL,
	[S_ResistanceManifesto] [int] NULL,
	[S_PassagesoftheManifesto] [int] NULL,
	[S_SuperconductiveAlloy] [int] NULL,
	[S_EssenceofThought] [int] NULL,
	[S_EssenceofPower] [int] NULL,
	[S_MeditationTechniques] [int] NULL,
	[S_SecretDocuments] [int] NULL,
	[S_GenomeSequences] [int] NULL,
	[S_ParapsychicEnergies] [int] NULL,
	[S_NanotechInterface] [int] NULL,
	[S_ArcaneCantrip] [int] NULL,
	[S_CatalyticAgents] [int] NULL,
	[S_BiopatternData] [int] NULL,
	[S_BiomorphicGoo] [int] NULL,
	[S_EnchantedSand] [int] NULL,
	[S_DetailedReports] [int] NULL,
	[S_GenomicAnalysis] [int] NULL,
	[S_SuperchargedCapacitor] [int] NULL,
	[S_UnstableDNA] [int] NULL,
	[S_RadiationShielding] [int] NULL,
	[S_TelluricDiode] [int] NULL,
	[S_EnergizedCrystal] [int] NULL,
	[S_CytoliticInfusion] [int] NULL,
	[S_WornSpellbook] [int] NULL,
	[S_MathematicalAnalysis] [int] NULL,
	[S_GluonCompound] [int] NULL,
	[S_MagneticInducers] [int] NULL,
	[S_DimensionalPocket] [int] NULL,
	[S_AdvancedTechBlueprint] [int] NULL,
	[S_AncientTexts] [int] NULL,
	[S_SecretTechnique] [int] NULL,
	[S_SemiConsciousEnergy] [int] NULL,
	[S_LivingRelicFragment] [int] NULL,
	[S_ThaumicResonator] [int] NULL,
	[S_MagnetizedFlesh] [int] NULL,
	[S_EnchantedPlutonium] [int] NULL,
	[S_SelfEvolvingAlloy] [int] NULL,
	[S_OmegaEssence] [int] NULL,
	[S_SuperconductiveAlloys] [int] NULL,
	[S_LivingRelicFragments] [int] NULL,
	[S_PartialGenomeMap] [int] NULL,
	[S_SecretDataPrintout] [int] NULL,
	[S_PortablePowerSource] [int] NULL,
	[S_PraetorianGodShard] [int] NULL,
	[S_BindingOfTheBrain] [int] NULL,
	[S_BindingOfTheHeart] [int] NULL,
	[S_BloodOfAPraetor] [int] NULL,
	[S_SiderealLocus] [int] NULL,
	[S_CrosslifeEmpathy] [int] NULL,
	[S_DimensionalKeystone] [int] NULL,
	[S_BloodOfTheEmperor] [int] NULL,
	[S_PangalaticLocus] [int] NULL,
	[S_MultiverseEmpathy] [int] NULL,
	[S_TransuniversalKeystone] [int] NULL,
	[S_PangalacticLocus] [int] NULL,
	[S_IncarnateShard] [int] NULL,
	[S_GraiMatter] [int] NULL,
	[S_EnhancedRibosomes] [int] NULL,
	[S_Hero1DNASample] [int] NULL,
	[S_AncientNictusFragment] [int] NULL,
	[S_EssenceoftheIncarnate] [int] NULL,
	[S_VanguardDNAMetamatrix] [int] NULL,
	[S_IncarnateInfusedNictus] [int] NULL,
	[S_DropoftheWell] [int] NULL,
	[S_InfiniteTessellation] [int] NULL,
	[S_PenumbraofRularuu] [int] NULL,
	[S_HeroMerit] [int] NULL,
	[S_VillainMerit] [int] NULL,
	[S_EndgameMerit01] [int] NULL,
	[S_EndgameMerit02] [int] NULL,
	[S_EndgameMerit03] [int] NULL,
	[S_EndgameMerit04] [int] NULL,
	[S_EndgameMerit05] [int] NULL,
	[S_NoticeOfTheWell] [int] NULL,
	[S_FavorOfTheWell] [int] NULL,
	[S_NanotechGrowthMedium] [int] NULL,
	[S_SuperconductiveMembrane] [int] NULL,
	[S_LivingRelic] [int] NULL,
	[S_ForbiddenTechnique] [int] NULL,
	[S_IncarnateThread] [int] NULL,
	[S_FortuneToken] [int] NULL,
	[S_EnhancementBooster] [int] NULL,
	[S_EnhancementUnslotter] [int] NULL,
	[S_EnhancementCatalyst] [int] NULL,
	[S_HVSuperPackSalvage] [int] NULL,
	[S_UHVSuperPackSalvage] [int] NULL,
	[S_TimerReset] [int] NULL,
	[S_EnhancementConverter] [int] NULL,
	[S_TimerReset_WST] [int] NULL,
	[S_TimerReset_AM] [int] NULL,
	[S_TimerReset_EMP] [int] NULL,
	[S_TimerReset_SSA] [int] NULL,
	[S_RVSuperPackSalvage] [int] NULL,
	[S_WinterPackSalvage] [int] NULL,
 CONSTRAINT [PK_InvSalvage0] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[InvStoredSalvage0]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[InvStoredSalvage0](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[S_MysticElement] [int] NULL,
	[S_MysticFoci] [int] NULL,
	[S_ArcaneEssence] [int] NULL,
	[S_ArcaneGlyph] [int] NULL,
	[S_MagicalWard] [int] NULL,
	[S_MagicalArtifact] [int] NULL,
	[S_TechMaterial] [int] NULL,
	[S_TechPower] [int] NULL,
	[S_TechHardware] [int] NULL,
	[S_TechPrototype] [int] NULL,
	[S_TechSoftware] [int] NULL,
	[S_Carapace] [int] NULL,
	[S_AbberantTech] [int] NULL,
	[S_ArachnosGun] [int] NULL,
	[S_ArmorShard] [int] NULL,
	[S_BlasterTech] [int] NULL,
	[S_SpiderEye] [int] NULL,
	[S_Amulet] [int] NULL,
	[S_CrystalCodexFragments] [int] NULL,
	[S_Demonlogica] [int] NULL,
	[S_Potions] [int] NULL,
	[S_SpiritGuide] [int] NULL,
	[S_UrMetal] [int] NULL,
	[S_BrokenMask] [int] NULL,
	[S_Sigil] [int] NULL,
	[S_Fetish] [int] NULL,
	[S_VerminousVictuals] [int] NULL,
	[S_Spark] [int] NULL,
	[S_StaticCharge] [int] NULL,
	[S_Charms] [int] NULL,
	[S_Cloak] [int] NULL,
	[S_DaemonicChain] [int] NULL,
	[S_Dagger] [int] NULL,
	[S_Orichalcum] [int] NULL,
	[S_Ring] [int] NULL,
	[S_Spells] [int] NULL,
	[S_Talisman] [int] NULL,
	[S_ThornFragment] [int] NULL,
	[S_PsionicReceptacle] [int] NULL,
	[S_PsionizedMetal] [int] NULL,
	[S_BloodSample] [int] NULL,
	[S_CeramicCompound] [int] NULL,
	[S_ExoticAlloy] [int] NULL,
	[S_ExoticCompound] [int] NULL,
	[S_Lens] [int] NULL,
	[S_StableProtonium] [int] NULL,
	[S_CoralaxBlood] [int] NULL,
	[S_CoralShard] [int] NULL,
	[S_FishScale] [int] NULL,
	[S_BodyArmorFragment] [int] NULL,
	[S_BrokenCreyRifle] [int] NULL,
	[S_CommunicationDevice] [int] NULL,
	[S_CreyTech] [int] NULL,
	[S_GhostTrinket] [int] NULL,
	[S_PhantomTears] [int] NULL,
	[S_Scrap] [int] NULL,
	[S_EbonClaw] [int] NULL,
	[S_ScentofBrimstone] [int] NULL,
	[S_DevouringCulture] [int] NULL,
	[S_HamidonLichen] [int] NULL,
	[S_BlackMarketSuperGear] [int] NULL,
	[S_FirBolgStraw] [int] NULL,
	[S_EnergySource] [int] NULL,
	[S_FreakshowCybernetics] [int] NULL,
	[S_Pelt] [int] NULL,
	[S_BioSample] [int] NULL,
	[S_AeonTech] [int] NULL,
	[S_GoldPlatedSkin] [int] NULL,
	[S_LevDisk] [int] NULL,
	[S_Hex] [int] NULL,
	[S_Tatoo] [int] NULL,
	[S_GeneticSample] [int] NULL,
	[S_KheldianTech] [int] NULL,
	[S_DreadTramalian] [int] NULL,
	[S_FaLakAmulet] [int] NULL,
	[S_RylehsRain] [int] NULL,
	[S_SpielmansSignet] [int] NULL,
	[S_LudditePouch] [int] NULL,
	[S_SaintsMedallion] [int] NULL,
	[S_Notes] [int] NULL,
	[S_MutatedSample] [int] NULL,
	[S_NeoOrganics] [int] NULL,
	[S_Ectoplasm] [int] NULL,
	[S_EctoplasmicResidue] [int] NULL,
	[S_BabbageEngine] [int] NULL,
	[S_NemesisWeapon] [int] NULL,
	[S_SteamTechImplant] [int] NULL,
	[S_VacuumCircuits] [int] NULL,
	[S_NictusMemento] [int] NULL,
	[S_NictusTech] [int] NULL,
	[S_Coins] [int] NULL,
	[S_BrokenRiktiWeapon] [int] NULL,
	[S_CortexDevice] [int] NULL,
	[S_RiktiArmorFragment] [int] NULL,
	[S_RiktiCommunicationsDevice] [int] NULL,
	[S_RiktiControlUnit] [int] NULL,
	[S_RiktiPlasmaRifle] [int] NULL,
	[S_ToolsoftheTrade] [int] NULL,
	[S_ShivanEctoplasm] [int] NULL,
	[S_CrystalSkull] [int] NULL,
	[S_SkyRaiderAntiGravUnit] [int] NULL,
	[S_SkyRaiderDevice] [int] NULL,
	[S_SkyRaiderWeapon] [int] NULL,
	[S_SnakeFang] [int] NULL,
	[S_SnakeVenom] [int] NULL,
	[S_AstrologyBook] [int] NULL,
	[S_DaemonHeart] [int] NULL,
	[S_RiktiDataPad] [int] NULL,
	[S_RiktiPlasmaWeapon] [int] NULL,
	[S_Saber] [int] NULL,
	[S_BlackBox] [int] NULL,
	[S_BrokenCreyPistol] [int] NULL,
	[S_CyberneticCharger] [int] NULL,
	[S_DangerousChemicals] [int] NULL,
	[S_EChip] [int] NULL,
	[S_GraphiteComposite] [int] NULL,
	[S_MercuryCircuits] [int] NULL,
	[S_Nanites] [int] NULL,
	[S_PortalTech] [int] NULL,
	[S_PoweredArmorCircuitry] [int] NULL,
	[S_Titanium] [int] NULL,
	[S_InnovativeCode] [int] NULL,
	[S_AncestralWeapon] [int] NULL,
	[S_ArcanePowder] [int] NULL,
	[S_EnchantedWeapon] [int] NULL,
	[S_Etherium] [int] NULL,
	[S_TsooInk] [int] NULL,
	[S_Furs] [int] NULL,
	[S_CyberneticImplant] [int] NULL,
	[S_SyntheticOrgans] [int] NULL,
	[S_Claw] [int] NULL,
	[S_AncientWeapon] [int] NULL,
	[S_Bracer] [int] NULL,
	[S_IceShard] [int] NULL,
	[S_Headset] [int] NULL,
	[S_TrickArrows] [int] NULL,
	[S_TissueSample] [int] NULL,
	[S_ArachnoidBlood] [int] NULL,
	[S_ArachnoidVenom] [int] NULL,
	[S_CIAFiles] [int] NULL,
	[S_HamidonSpore] [int] NULL,
	[S_Glittershrooms] [int] NULL,
	[S_PumpkinBomb] [int] NULL,
	[S_PumpkinShard] [int] NULL,
	[S_KheldianBloodSample] [int] NULL,
	[S_ElementalManual] [int] NULL,
	[S_Treatise] [int] NULL,
	[S_Hatchet] [int] NULL,
	[S_NemesisStaff] [int] NULL,
	[S_HellishTooth] [int] NULL,
	[S_NictusAmmo] [int] NULL,
	[S_NecklaceofTeeth] [int] NULL,
	[S_RedcapPouch] [int] NULL,
	[S_Shillelagh] [int] NULL,
	[S_DNAMutation] [int] NULL,
	[S_BoneFragment] [int] NULL,
	[S_MilitaryIntelligence] [int] NULL,
	[S_DataFiles] [int] NULL,
	[S_ParagonPoliceFiles] [int] NULL,
	[S_SuperadineExtract] [int] NULL,
	[S_Scrolls] [int] NULL,
	[S_SyntheticDrug] [int] NULL,
	[S_BookOfBlood] [int] NULL,
	[S_WitchsHat] [int] NULL,
	[S_SlagCulture] [int] NULL,
	[S_Magma] [int] NULL,
	[S_Pumice] [int] NULL,
	[S_MeatCleaver] [int] NULL,
	[S_ExperimentalTech] [int] NULL,
	[S_SpellsOfPower] [int] NULL,
	[S_UnstableRadPistol] [int] NULL,
	[S_WeaponOfMu] [int] NULL,
	[S_AlienTech] [int] NULL,
	[S_NanoFluid] [int] NULL,
	[S_AccessBypass_MM] [int] NULL,
	[S_PrototypeElement_MM] [int] NULL,
	[S_HumanBloodSample] [int] NULL,
	[S_InanimateCarbonRod] [int] NULL,
	[S_ComputerVirus] [int] NULL,
	[S_SimpleChemical] [int] NULL,
	[S_Brass] [int] NULL,
	[S_Boresight] [int] NULL,
	[S_ImprovisedCybernetic] [int] NULL,
	[S_InertGas] [int] NULL,
	[S_ScientificTheory] [int] NULL,
	[S_CircuitBoard] [int] NULL,
	[S_StabilizedMutantGenome] [int] NULL,
	[S_Iron] [int] NULL,
	[S_KineticWeapon] [int] NULL,
	[S_TemporalAnalyzer] [int] NULL,
	[S_CeramicArmorPlate] [int] NULL,
	[S_HydraulicPiston] [int] NULL,
	[S_Silver] [int] NULL,
	[S_MathematicProof] [int] NULL,
	[S_MutantBloodSample] [int] NULL,
	[S_HeavyWater] [int] NULL,
	[S_DaemonProgram] [int] NULL,
	[S_ChemicalFormula] [int] NULL,
	[S_Polycarbon] [int] NULL,
	[S_Scope] [int] NULL,
	[S_CommercialCybernetic] [int] NULL,
	[S_CorrosiveGas] [int] NULL,
	[S_ScientificLaw] [int] NULL,
	[S_DataDrive] [int] NULL,
	[S_MutantDNAStrand] [int] NULL,
	[S_Steel] [int] NULL,
	[S_EnergyWeapon] [int] NULL,
	[S_TemporalTracer] [int] NULL,
	[S_TitaniumShard] [int] NULL,
	[S_PneumaticPiston] [int] NULL,
	[S_Gold] [int] NULL,
	[S_ChaosTheorem] [int] NULL,
	[S_AlienBloodSample] [int] NULL,
	[S_EnrichedPlutonium] [int] NULL,
	[S_SourceCode] [int] NULL,
	[S_ComplexChemicalFormula] [int] NULL,
	[S_PlasmaCapacitor] [int] NULL,
	[S_HeadsUpDisplay] [int] NULL,
	[S_MilitaryCybernetic] [int] NULL,
	[S_ReactiveGas] [int] NULL,
	[S_ConspiratorialEvidence] [int] NULL,
	[S_HolographicMemory] [int] NULL,
	[S_MutatingGenome] [int] NULL,
	[S_Impervium] [int] NULL,
	[S_PhotonicWeapon] [int] NULL,
	[S_ChronalSkip] [int] NULL,
	[S_RiktiAlloy] [int] NULL,
	[S_PositronicMatrix] [int] NULL,
	[S_Platinum] [int] NULL,
	[S_SyntheticIntelligenceUnit] [int] NULL,
	[S_AncientArtifact] [int] NULL,
	[S_LuckCharm] [int] NULL,
	[S_ClockworkWinder] [int] NULL,
	[S_SpellScroll] [int] NULL,
	[S_SpiritualEssence] [int] NULL,
	[S_RuneboundArmor] [int] NULL,
	[S_MasterworkWeapon] [int] NULL,
	[S_Rune] [int] NULL,
	[S_DemonicBloodSample] [int] NULL,
	[S_AlchemicalSilver] [int] NULL,
	[S_AncientBone] [int] NULL,
	[S_SpellInk] [int] NULL,
	[S_Fortune] [int] NULL,
	[S_Ruby] [int] NULL,
	[S_DemonicThreatReport] [int] NULL,
	[S_RegeneratingFlesh] [int] NULL,
	[S_SpiritThorn] [int] NULL,
	[S_UnearthedRelic] [int] NULL,
	[S_TemporalSands] [int] NULL,
	[S_ClockworkGear] [int] NULL,
	[S_VolumeoftheObsidianLibrum] [int] NULL,
	[S_PsionicEctoplasm] [int] NULL,
	[S_SoulboundArmor] [int] NULL,
	[S_EnscorcelledWeapon] [int] NULL,
	[S_Symbol] [int] NULL,
	[S_BloodoftheIncarnate] [int] NULL,
	[S_AlchemicalGold] [int] NULL,
	[S_CarnivalofShadowsMask] [int] NULL,
	[S_LivingTattoo] [int] NULL,
	[S_UnquenchableFlame] [int] NULL,
	[S_Destiny] [int] NULL,
	[S_Sapphire] [int] NULL,
	[S_PsionicThreatReport] [int] NULL,
	[S_BleedingStone] [int] NULL,
	[S_ThornTreeVine] [int] NULL,
	[S_LamentBox] [int] NULL,
	[S_StrandofFate] [int] NULL,
	[S_PsioniclyChargedBrass] [int] NULL,
	[S_PagefromtheMalleusMundi] [int] NULL,
	[S_PsionicManfestation] [int] NULL,
	[S_SymbioticArmor] [int] NULL,
	[S_DeificWeapon] [int] NULL,
	[S_EmpoweredSigil] [int] NULL,
	[S_BlackBloodoftheEarth] [int] NULL,
	[S_EnchantedImpervium] [int] NULL,
	[S_MuVestment] [int] NULL,
	[S_SoulTrappedGem] [int] NULL,
	[S_PangeanSoil] [int] NULL,
	[S_Prophecy] [int] NULL,
	[S_Diamond] [int] NULL,
	[S_MagicalConspiracy] [int] NULL,
	[S_EssenceoftheFuries] [int] NULL,
	[S_HamidonGoo] [int] NULL,
	[S_NevermeltingIce] [int] NULL,
	[S_StatesmanMask_H2006] [int] NULL,
	[S_LordRecluseMask_H2006] [int] NULL,
	[S_BackAlleyBrawlerGloves_H2006] [int] NULL,
	[S_HamidonCostume_H2006] [int] NULL,
	[S_VanguardMerit] [int] NULL,
	[S_CandyCane] [int] NULL,
	[S_Hero1BloodLostCure] [int] NULL,
	[S_AzuriasWandLostCure] [int] NULL,
	[S_LostSerumLostCure] [int] NULL,
	[S_CoTIncantationLostCure] [int] NULL,
	[S_GraveDirtLostCure] [int] NULL,
	[S_MeritReward] [int] NULL,
	[S_TailorDiscountCoupon] [int] NULL,
	[S_AuctionDiscountCoupon] [int] NULL,
	[S_InventionDiscountCoupon] [int] NULL,
	[S_BrainStormIdea] [int] NULL,
	[S_ArchitectTicket] [int] NULL,
	[S_AndroidCircuitry] [int] NULL,
	[S_AndroidBlaster] [int] NULL,
	[S_BrainLichen] [int] NULL,
	[S_PrimordialMoss] [int] NULL,
	[S_AndroidArmorPlate] [int] NULL,
	[S_PlagueSpores] [int] NULL,
	[S_UndamagedAndroidBrain] [int] NULL,
	[S_ProgenitorLichen] [int] NULL,
	[S_ImplosionGrenade] [int] NULL,
	[S_ExoticIsotope] [int] NULL,
	[S_IFFCodes] [int] NULL,
	[S_IFFCodeFragement] [int] NULL,
	[S_ResistanceManifesto] [int] NULL,
	[S_PassagesoftheManifesto] [int] NULL,
	[S_SuperconductiveAlloy] [int] NULL,
	[S_EssenceofThought] [int] NULL,
	[S_EssenceofPower] [int] NULL,
	[S_MeditationTechniques] [int] NULL,
	[S_SecretDocuments] [int] NULL,
	[S_GenomeSequences] [int] NULL,
	[S_ParapsychicEnergies] [int] NULL,
	[S_NanotechInterface] [int] NULL,
	[S_ArcaneCantrip] [int] NULL,
	[S_CatalyticAgents] [int] NULL,
	[S_BiopatternData] [int] NULL,
	[S_BiomorphicGoo] [int] NULL,
	[S_EnchantedSand] [int] NULL,
	[S_DetailedReports] [int] NULL,
	[S_GenomicAnalysis] [int] NULL,
	[S_SuperchargedCapacitor] [int] NULL,
	[S_UnstableDNA] [int] NULL,
	[S_RadiationShielding] [int] NULL,
	[S_TelluricDiode] [int] NULL,
	[S_EnergizedCrystal] [int] NULL,
	[S_CytoliticInfusion] [int] NULL,
	[S_WornSpellbook] [int] NULL,
	[S_MathematicalAnalysis] [int] NULL,
	[S_GluonCompound] [int] NULL,
	[S_MagneticInducers] [int] NULL,
	[S_DimensionalPocket] [int] NULL,
	[S_AdvancedTechBlueprint] [int] NULL,
	[S_AncientTexts] [int] NULL,
	[S_SecretTechnique] [int] NULL,
	[S_SemiConsciousEnergy] [int] NULL,
	[S_LivingRelicFragment] [int] NULL,
	[S_ThaumicResonator] [int] NULL,
	[S_MagnetizedFlesh] [int] NULL,
	[S_EnchantedPlutonium] [int] NULL,
	[S_SelfEvolvingAlloy] [int] NULL,
	[S_OmegaEssence] [int] NULL,
	[S_SuperconductiveAlloys] [int] NULL,
	[S_LivingRelicFragments] [int] NULL,
	[S_PartialGenomeMap] [int] NULL,
	[S_SecretDataPrintout] [int] NULL,
	[S_PortablePowerSource] [int] NULL,
	[S_PraetorianGodShard] [int] NULL,
	[S_BindingOfTheBrain] [int] NULL,
	[S_BindingOfTheHeart] [int] NULL,
	[S_BloodOfAPraetor] [int] NULL,
	[S_SiderealLocus] [int] NULL,
	[S_CrosslifeEmpathy] [int] NULL,
	[S_DimensionalKeystone] [int] NULL,
	[S_BloodOfTheEmperor] [int] NULL,
	[S_PangalaticLocus] [int] NULL,
	[S_MultiverseEmpathy] [int] NULL,
	[S_TransuniversalKeystone] [int] NULL,
	[S_PangalacticLocus] [int] NULL,
	[S_IncarnateShard] [int] NULL,
	[S_GraiMatter] [int] NULL,
	[S_EnhancedRibosomes] [int] NULL,
	[S_Hero1DNASample] [int] NULL,
	[S_AncientNictusFragment] [int] NULL,
	[S_EssenceoftheIncarnate] [int] NULL,
	[S_VanguardDNAMetamatrix] [int] NULL,
	[S_IncarnateInfusedNictus] [int] NULL,
	[S_DropoftheWell] [int] NULL,
	[S_InfiniteTessellation] [int] NULL,
	[S_PenumbraofRularuu] [int] NULL,
	[S_HeroMerit] [int] NULL,
	[S_VillainMerit] [int] NULL,
	[S_EndgameMerit01] [int] NULL,
	[S_EndgameMerit02] [int] NULL,
	[S_EndgameMerit03] [int] NULL,
	[S_EndgameMerit04] [int] NULL,
	[S_EndgameMerit05] [int] NULL,
	[S_NoticeOfTheWell] [int] NULL,
	[S_FavorOfTheWell] [int] NULL,
	[S_NanotechGrowthMedium] [int] NULL,
	[S_SuperconductiveMembrane] [int] NULL,
	[S_LivingRelic] [int] NULL,
	[S_ForbiddenTechnique] [int] NULL,
	[S_IncarnateThread] [int] NULL,
	[S_FortuneToken] [int] NULL,
	[S_EnhancementBooster] [int] NULL,
	[S_EnhancementUnslotter] [int] NULL,
	[S_EnhancementCatalyst] [int] NULL,
	[S_HVSuperPackSalvage] [int] NULL,
	[S_UHVSuperPackSalvage] [int] NULL,
	[S_TimerReset] [int] NULL,
	[S_EnhancementConverter] [int] NULL,
	[S_TimerReset_WST] [int] NULL,
	[S_TimerReset_AM] [int] NULL,
	[S_TimerReset_EMP] [int] NULL,
	[S_TimerReset_SSA] [int] NULL,
	[S_RVSuperPackSalvage] [int] NULL,
	[S_WinterPackSalvage] [int] NULL,
 CONSTRAINT [PK_InvStoredSalvage0] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[ItemOfPowerGames]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemOfPowerGames](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[IOPGameID] [int] NULL,
	[IOPGameStartTime] [int] NULL,
	[IOPGameState] [int] NULL,
	[RaidLength] [int] NULL,
	[RaidSize] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[ItemsOfPower]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemsOfPower](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[ItemID] [int] NULL,
	[ItemName] [nvarchar](256) NULL,
	[ItemSuperGroupOwner] [int] NULL,
	[ItemCreationTime] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[KeyBinds]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[KeyBinds](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Command] [nvarchar](512) NULL,
	[KeyCode] [int] NULL,
	[Modifier] [int] NULL,
 CONSTRAINT [PK_KeyBinds] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Leagues]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Leagues](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[LeaderId] [int] NULL,
	[version] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[LevelingPacts]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[LevelingPacts](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[Count] [int] NULL,
	[Experience] [int] NULL,
	[Influence1] [int] NULL,
	[Influence2] [int] NULL,
	[timeLogged] [int] NULL,
	[version] [int] NULL,
	[Infamy1] [int] NULL,
	[Infamy2] [int] NULL,
	[timeLoggedLevel0] [int] NULL,
	[timeLoggedLevel1] [int] NULL,
	[timeLoggedLevel2] [int] NULL,
	[timeLoggedLevel3] [int] NULL,
	[timeLoggedLevel4] [int] NULL,
	[timeLoggedLevel5] [int] NULL,
	[timeLoggedLevel6] [int] NULL,
	[timeLoggedLevel7] [int] NULL,
	[timeLoggedLevel8] [int] NULL,
	[timeLoggedLevel9] [int] NULL,
	[timeLoggedLevel10] [int] NULL,
	[timeLoggedLevel11] [int] NULL,
	[timeLoggedLevel12] [int] NULL,
	[timeLoggedLevel13] [int] NULL,
	[timeLoggedLevel14] [int] NULL,
	[timeLoggedLevel15] [int] NULL,
	[timeLoggedLevel16] [int] NULL,
	[timeLoggedLevel17] [int] NULL,
	[timeLoggedLevel18] [int] NULL,
	[timeLoggedLevel19] [int] NULL,
	[timeLoggedLevel20] [int] NULL,
	[timeLoggedLevel21] [int] NULL,
	[timeLoggedLevel22] [int] NULL,
	[timeLoggedLevel23] [int] NULL,
	[timeLoggedLevel24] [int] NULL,
	[timeLoggedLevel25] [int] NULL,
	[timeLoggedLevel26] [int] NULL,
	[timeLoggedLevel27] [int] NULL,
	[timeLoggedLevel28] [int] NULL,
	[timeLoggedLevel29] [int] NULL,
	[timeLoggedLevel30] [int] NULL,
	[timeLoggedLevel31] [int] NULL,
	[timeLoggedLevel32] [int] NULL,
	[timeLoggedLevel33] [int] NULL,
	[timeLoggedLevel34] [int] NULL,
	[timeLoggedLevel35] [int] NULL,
	[timeLoggedLevel36] [int] NULL,
	[timeLoggedLevel37] [int] NULL,
	[timeLoggedLevel38] [int] NULL,
	[timeLoggedLevel39] [int] NULL,
	[timeLoggedLevel40] [int] NULL,
	[timeLoggedLevel41] [int] NULL,
	[timeLoggedLevel42] [int] NULL,
	[timeLoggedLevel43] [int] NULL,
	[timeLoggedLevel44] [int] NULL,
	[timeLoggedLevel45] [int] NULL,
	[timeLoggedLevel46] [int] NULL,
	[timeLoggedLevel47] [int] NULL,
	[timeLoggedLevel48] [int] NULL,
	[timeLoggedLevel49] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[MapDataTokens]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MapDataTokens](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Name] [nvarchar](128) NULL,
	[Value] [int] NULL,
 CONSTRAINT [PK_MapDataTokens] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[MapGroups]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MapGroups](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[Name] [nvarchar](128) NULL,
	[ActiveEvent] [nvarchar](128) NULL,
	[EventSignal] [nvarchar](128) NULL,
	[EventSignalTS] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[MapHistory]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MapHistory](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[MapType] [int] NULL,
	[MapID] [int] NULL,
	[PosX] [real] NULL,
	[PosY] [real] NULL,
	[PosZ] [real] NULL,
	[OrientP] [real] NULL,
	[OrientY] [real] NULL,
	[OrientR] [real] NULL,
 CONSTRAINT [PK_MapHistory] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[MARTYTracks]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[MARTYTracks](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[MARTYPerMinuteAccum] [int] NULL,
	[MARTYRingBuffer] [varchar](960) NULL,
	[MARTYCurrentMinute] [int] NULL,
	[MARTYRingBufferLoc] [int] NULL,
	[MARTY5MinSum] [int] NULL,
	[MARTY15MinSum] [int] NULL,
	[MARTY30MinSum] [int] NULL,
	[MARTY60MinSum] [int] NULL,
	[MARTY120MinSum] [int] NULL,
	[MARTYThrottled] [int] NULL,
 CONSTRAINT [PK_MARTYTracks] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[MinedData]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[MinedData](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Field] [varchar](510) NULL,
	[Ever] [int] NULL,
	[ThisWeek] [int] NULL,
	[LastWeek] [int] NULL,
 CONSTRAINT [PK_MinedData] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[MiningAccumulator]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[MiningAccumulator](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[Name] [varchar](510) NULL,
	[WeekStart] [datetime] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[NewspaperHistory]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NewspaperHistory](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[NTHIndexItem] [int] NULL,
	[NTHItem0] [int] NULL,
	[NTHItem1] [int] NULL,
	[NTHItem2] [int] NULL,
	[NTHItem3] [int] NULL,
	[NTHItem4] [int] NULL,
	[NTHItem5] [int] NULL,
	[NTHItem6] [int] NULL,
	[NTHItem7] [int] NULL,
	[NTHItem8] [int] NULL,
	[NTHItem9] [int] NULL,
	[NTHItem10] [int] NULL,
	[NTHItem11] [int] NULL,
	[NTHItem12] [int] NULL,
	[NTHItem13] [int] NULL,
	[NTHItem14] [int] NULL,
	[NTHItem15] [int] NULL,
	[NTHItem16] [int] NULL,
	[NTHItem17] [int] NULL,
	[NTHItem18] [int] NULL,
	[NTHItem19] [int] NULL,
	[NTHIndexPerson] [int] NULL,
	[NTHPerson0] [int] NULL,
	[NTHPerson1] [int] NULL,
	[NTHPerson2] [int] NULL,
	[NTHPerson3] [int] NULL,
	[NTHPerson4] [int] NULL,
	[NTHPerson5] [int] NULL,
	[NTHPerson6] [int] NULL,
	[NTHPerson7] [int] NULL,
	[NTHPerson8] [int] NULL,
	[NTHPerson9] [int] NULL,
	[NTHPerson10] [int] NULL,
	[NTHPerson11] [int] NULL,
	[NTHPerson12] [int] NULL,
	[NTHPerson13] [int] NULL,
	[NTHPerson14] [int] NULL,
	[NTHPerson15] [int] NULL,
	[NTHPerson16] [int] NULL,
	[NTHPerson17] [int] NULL,
	[NTHPerson18] [int] NULL,
	[NTHPerson19] [int] NULL,
 CONSTRAINT [PK_NewspaperHistory] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Offline]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Offline](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[AuthId] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Participants]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Participants](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[DbId] [int] NULL,
	[Archetype] [int] NULL,
	[Level] [int] NULL,
	[SgId] [int] NULL,
	[SgLeader] [int] NULL,
	[Side] [int] NULL,
	[Paid] [int] NULL,
	[Entryfee] [int] NULL,
	[Dropped] [int] NULL,
	[Seat0] [int] NULL,
	[Seat1] [int] NULL,
	[Seat2] [int] NULL,
	[Seat3] [int] NULL,
	[Seat4] [int] NULL,
	[Seat5] [int] NULL,
	[Seat6] [int] NULL,
	[Seat7] [int] NULL,
	[Seat8] [int] NULL,
	[Seat9] [int] NULL,
	[Seat10] [int] NULL,
	[RoundLastFloated] [int] NULL,
 CONSTRAINT [PK_Participants] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[PendingOrders]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PendingOrders](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[OrderId0] [int] NULL,
	[OrderId1] [int] NULL,
	[OrderId2] [int] NULL,
	[OrderId3] [int] NULL,
 CONSTRAINT [PK_PendingOrders] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Petitions]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Petitions](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[AuthName] [nvarchar](64) NULL,
	[Name] [nvarchar](128) NULL,
	[PosX] [real] NULL,
	[PosY] [real] NULL,
	[PosZ] [real] NULL,
	[MapName] [nvarchar](512) NULL,
	[Date] [datetime] NULL,
	[Summary] [nvarchar](256) NULL,
	[Msg] [nvarchar](2048) NULL,
	[Fetched] [tinyint] NULL,
	[Done] [tinyint] NULL,
	[Category] [tinyint] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[PetNames]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[PetNames](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[PowerName] [varchar](256) NULL,
	[PetNumber] [int] NULL,
	[PetName] [nvarchar](32) NULL,
	[EntityDef] [varchar](256) NULL,
 CONSTRAINT [PK_PetNames] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[PopHelpAttributes]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PopHelpAttributes](
	[Id] [int] NOT NULL,
	[Name] [varchar](128) NULL,
PRIMARY KEY CLUSTERED 
(
	[Id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[PowerCustomizations]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PowerCustomizations](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[PowerCatName] [int] NULL,
	[PowerSetName] [int] NULL,
	[PowerName] [int] NULL,
	[Color1] [int] NULL,
	[Color2] [int] NULL,
	[Token] [int] NULL,
	[SlotId] [int] NULL,
 CONSTRAINT [PK_PowerCustomizations] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Powers]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Powers](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[PowerID] [int] NULL,
	[CategoryName] [int] NULL,
	[PowerSetName] [int] NULL,
	[PowerName] [int] NULL,
	[PowerLevelBought] [int] NULL,
	[PowerNumBoostsBought] [int] NULL,
	[PowerSetLevelBought] [int] NULL,
	[NumCharges] [int] NULL,
	[UsageTime] [real] NULL,
	[CreationTime] [int] NULL,
	[RechargedAt] [int] NULL,
	[Active] [tinyint] NULL,
	[Var0] [real] NULL,
	[Var1] [real] NULL,
	[Var2] [real] NULL,
	[Var3] [real] NULL,
	[SlottedPowerupsMask] [int] NULL,
	[PowerupSlotType0] [int] NULL,
	[PowerupSlotId0] [int] NULL,
	[PowerupSlotType1] [int] NULL,
	[PowerupSlotId1] [int] NULL,
	[PowerupSlotType2] [int] NULL,
	[PowerupSlotId2] [int] NULL,
	[PowerupSlotType3] [int] NULL,
	[PowerupSlotId3] [int] NULL,
	[PowerupSlotType4] [int] NULL,
	[PowerupSlotId4] [int] NULL,
	[PowerupSlotType5] [int] NULL,
	[PowerupSlotId5] [int] NULL,
	[PowerupSlotType6] [int] NULL,
	[PowerupSlotId6] [int] NULL,
	[PowerupSlotType7] [int] NULL,
	[PowerupSlotId7] [int] NULL,
	[PowerupSlotType8] [int] NULL,
	[PowerupSlotId8] [int] NULL,
	[PowerupSlotType9] [int] NULL,
	[PowerupSlotId9] [int] NULL,
	[AvailableTime] [real] NULL,
	[BuildNum] [int] NULL,
	[Disabled] [tinyint] NULL,
	[InstanceBought] [int] NULL,
	[UniqueID] [int] NULL,
 CONSTRAINT [PK_Powers] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[QueuedRewardTables]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[QueuedRewardTables](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Name] [nvarchar](256) NULL,
	[Vgroup] [int] NULL,
	[Level] [int] NULL,
 CONSTRAINT [PK_QueuedRewardTables] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[RecentBadge]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RecentBadge](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Idx] [int] NULL,
	[TimeAwarded] [int] NULL,
 CONSTRAINT [PK_RecentBadge] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Recipes]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Recipes](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Recipe] [int] NULL,
	[Count] [int] NULL,
 CONSTRAINT [PK_Recipes] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Recipients]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Recipients](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Recipient] [int] NULL,
	[State] [tinyint] NULL,
 CONSTRAINT [PK_Recipients] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[RewardTokens]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RewardTokens](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[PieceName] [int] NULL,
	[RewardValue] [int] NULL,
	[RewardTime] [int] NULL,
 CONSTRAINT [PK_RewardTokens] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[RewardTokensActive]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RewardTokensActive](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[PieceName] [int] NULL,
	[RewardValue] [int] NULL,
	[RewardTime] [int] NULL,
 CONSTRAINT [PK_RewardTokensActive] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Seating]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Seating](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[MapId] [int] NULL,
	[Round] [int] NULL,
	[WinningSide] [int] NULL,
	[MatchTime] [int] NULL,
 CONSTRAINT [PK_Seating] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[SGRaidInfos]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SGRaidInfos](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[PlayerType] [int] NULL,
	[WindowHour] [int] NULL,
	[WindowDays] [int] NULL,
	[OpenMount] [int] NULL,
	[MaxRaidSize] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[SgrpBadgeStats]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SgrpBadgeStats](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[id0] [int] NULL,
	[id1] [int] NULL,
	[id2] [int] NULL,
	[id3] [int] NULL,
	[id4] [int] NULL,
	[id5] [int] NULL,
	[id6] [int] NULL,
	[id7] [int] NULL,
	[id8] [int] NULL,
	[id9] [int] NULL,
	[id10] [int] NULL,
	[id11] [int] NULL,
	[id12] [int] NULL,
	[id13] [int] NULL,
	[id14] [int] NULL,
	[id15] [int] NULL,
	[id16] [int] NULL,
	[id17] [int] NULL,
	[id18] [int] NULL,
	[id19] [int] NULL,
	[id20] [int] NULL,
	[id21] [int] NULL,
	[id22] [int] NULL,
	[id23] [int] NULL,
	[id24] [int] NULL,
	[id25] [int] NULL,
	[id26] [int] NULL,
	[id27] [int] NULL,
	[id28] [int] NULL,
	[id29] [int] NULL,
	[id30] [int] NULL,
	[id31] [int] NULL,
	[id32] [int] NULL,
	[id33] [int] NULL,
	[id34] [int] NULL,
	[id35] [int] NULL,
	[id36] [int] NULL,
	[id37] [int] NULL,
	[id38] [int] NULL,
	[id39] [int] NULL,
	[id40] [int] NULL,
	[id41] [int] NULL,
	[id42] [int] NULL,
	[id43] [int] NULL,
	[id44] [int] NULL,
	[id45] [int] NULL,
	[id46] [int] NULL,
	[id47] [int] NULL,
	[id48] [int] NULL,
	[id49] [int] NULL,
	[id50] [int] NULL,
	[id51] [int] NULL,
	[id52] [int] NULL,
	[id53] [int] NULL,
	[id54] [int] NULL,
	[id55] [int] NULL,
	[id56] [int] NULL,
	[id57] [int] NULL,
	[id58] [int] NULL,
	[id59] [int] NULL,
	[id60] [int] NULL,
	[id61] [int] NULL,
	[id62] [int] NULL,
	[id63] [int] NULL,
	[id64] [int] NULL,
	[id65] [int] NULL,
	[id66] [int] NULL,
	[id67] [int] NULL,
	[id68] [int] NULL,
	[id69] [int] NULL,
	[id70] [int] NULL,
	[id71] [int] NULL,
	[id72] [int] NULL,
	[id73] [int] NULL,
	[id74] [int] NULL,
	[id75] [int] NULL,
	[id76] [int] NULL,
	[id77] [int] NULL,
	[id78] [int] NULL,
	[id79] [int] NULL,
	[id80] [int] NULL,
	[id81] [int] NULL,
	[id82] [int] NULL,
	[id83] [int] NULL,
	[id84] [int] NULL,
	[id85] [int] NULL,
	[id86] [int] NULL,
	[id87] [int] NULL,
	[id88] [int] NULL,
	[id89] [int] NULL,
	[id90] [int] NULL,
	[id91] [int] NULL,
	[id92] [int] NULL,
	[id93] [int] NULL,
	[id94] [int] NULL,
	[id95] [int] NULL,
	[id96] [int] NULL,
	[id97] [int] NULL,
	[id98] [int] NULL,
	[id99] [int] NULL,
	[id100] [int] NULL,
	[id101] [int] NULL,
	[id102] [int] NULL,
	[id103] [int] NULL,
	[id104] [int] NULL,
	[id105] [int] NULL,
	[id106] [int] NULL,
	[id107] [int] NULL,
	[id108] [int] NULL,
	[id109] [int] NULL,
	[id110] [int] NULL,
	[id111] [int] NULL,
	[id112] [int] NULL,
	[id113] [int] NULL,
	[id114] [int] NULL,
	[id115] [int] NULL,
	[id116] [int] NULL,
	[id117] [int] NULL,
	[id118] [int] NULL,
	[id119] [int] NULL,
	[id120] [int] NULL,
	[id121] [int] NULL,
	[id122] [int] NULL,
	[id123] [int] NULL,
	[id124] [int] NULL,
	[id125] [int] NULL,
	[id126] [int] NULL,
	[id127] [int] NULL,
	[id128] [int] NULL,
	[id129] [int] NULL,
	[id130] [int] NULL,
	[id131] [int] NULL,
	[id132] [int] NULL,
	[id133] [int] NULL,
	[id134] [int] NULL,
	[id135] [int] NULL,
	[id136] [int] NULL,
	[id137] [int] NULL,
	[id138] [int] NULL,
	[id139] [int] NULL,
	[id140] [int] NULL,
	[id141] [int] NULL,
	[id142] [int] NULL,
	[id143] [int] NULL,
	[id144] [int] NULL,
	[id145] [int] NULL,
	[id146] [int] NULL,
	[id147] [int] NULL,
	[id148] [int] NULL,
	[id149] [int] NULL,
	[id150] [int] NULL,
	[id151] [int] NULL,
	[id152] [int] NULL,
	[id153] [int] NULL,
	[id154] [int] NULL,
	[id155] [int] NULL,
	[id156] [int] NULL,
	[id157] [int] NULL,
	[id158] [int] NULL,
	[id159] [int] NULL,
	[id160] [int] NULL,
	[id161] [int] NULL,
	[id162] [int] NULL,
	[id163] [int] NULL,
	[id164] [int] NULL,
	[id165] [int] NULL,
	[id166] [int] NULL,
	[id167] [int] NULL,
	[id168] [int] NULL,
	[id169] [int] NULL,
	[id170] [int] NULL,
	[id171] [int] NULL,
	[id172] [int] NULL,
	[id173] [int] NULL,
	[id174] [int] NULL,
	[id175] [int] NULL,
	[id176] [int] NULL,
	[id177] [int] NULL,
	[id178] [int] NULL,
	[id179] [int] NULL,
	[id180] [int] NULL,
	[id181] [int] NULL,
	[id182] [int] NULL,
	[id183] [int] NULL,
	[id184] [int] NULL,
	[id185] [int] NULL,
	[id186] [int] NULL,
	[id187] [int] NULL,
	[id188] [int] NULL,
	[id189] [int] NULL,
	[id190] [int] NULL,
	[id191] [int] NULL,
	[id192] [int] NULL,
	[id193] [int] NULL,
	[id194] [int] NULL,
	[id195] [int] NULL,
	[id196] [int] NULL,
	[id197] [int] NULL,
	[id198] [int] NULL,
	[id199] [int] NULL,
	[id200] [int] NULL,
	[id201] [int] NULL,
	[id202] [int] NULL,
	[id203] [int] NULL,
	[id204] [int] NULL,
	[id205] [int] NULL,
	[id206] [int] NULL,
	[id207] [int] NULL,
	[id208] [int] NULL,
	[id209] [int] NULL,
	[id210] [int] NULL,
	[id211] [int] NULL,
	[id212] [int] NULL,
	[id213] [int] NULL,
	[id214] [int] NULL,
	[id215] [int] NULL,
	[id216] [int] NULL,
	[id217] [int] NULL,
	[id218] [int] NULL,
	[id219] [int] NULL,
	[id220] [int] NULL,
	[id221] [int] NULL,
	[id222] [int] NULL,
	[id223] [int] NULL,
	[id224] [int] NULL,
	[id225] [int] NULL,
	[id226] [int] NULL,
	[id227] [int] NULL,
	[id228] [int] NULL,
	[id229] [int] NULL,
	[id230] [int] NULL,
	[id231] [int] NULL,
	[id232] [int] NULL,
	[id233] [int] NULL,
	[id234] [int] NULL,
	[id235] [int] NULL,
	[id236] [int] NULL,
	[id237] [int] NULL,
	[id238] [int] NULL,
	[id239] [int] NULL,
	[id240] [int] NULL,
	[id241] [int] NULL,
	[id242] [int] NULL,
	[id243] [int] NULL,
	[id244] [int] NULL,
	[id245] [int] NULL,
	[id246] [int] NULL,
	[id247] [int] NULL,
	[id248] [int] NULL,
	[id249] [int] NULL,
	[id250] [int] NULL,
	[id251] [int] NULL,
	[id252] [int] NULL,
	[id253] [int] NULL,
	[id254] [int] NULL,
	[id255] [int] NULL,
	[id256] [int] NULL,
	[id257] [int] NULL,
	[id258] [int] NULL,
	[id259] [int] NULL,
	[id260] [int] NULL,
	[id261] [int] NULL,
	[id262] [int] NULL,
	[id263] [int] NULL,
	[id264] [int] NULL,
	[id265] [int] NULL,
	[id266] [int] NULL,
	[id267] [int] NULL,
	[id268] [int] NULL,
	[id269] [int] NULL,
	[id270] [int] NULL,
	[id271] [int] NULL,
	[id272] [int] NULL,
	[id273] [int] NULL,
	[id274] [int] NULL,
	[id275] [int] NULL,
	[id276] [int] NULL,
	[id277] [int] NULL,
	[id278] [int] NULL,
	[id279] [int] NULL,
	[id280] [int] NULL,
	[id281] [int] NULL,
	[id282] [int] NULL,
	[id283] [int] NULL,
	[id284] [int] NULL,
	[id285] [int] NULL,
	[id286] [int] NULL,
	[id287] [int] NULL,
	[id288] [int] NULL,
	[id289] [int] NULL,
	[id290] [int] NULL,
	[id291] [int] NULL,
	[id292] [int] NULL,
	[id293] [int] NULL,
	[id294] [int] NULL,
	[id295] [int] NULL,
	[id296] [int] NULL,
	[id297] [int] NULL,
	[id298] [int] NULL,
	[id299] [int] NULL,
	[id300] [int] NULL,
	[id301] [int] NULL,
	[id302] [int] NULL,
	[id303] [int] NULL,
	[id304] [int] NULL,
	[id305] [int] NULL,
	[id306] [int] NULL,
	[id307] [int] NULL,
	[id308] [int] NULL,
	[id309] [int] NULL,
	[id310] [int] NULL,
	[id311] [int] NULL,
	[id312] [int] NULL,
	[id313] [int] NULL,
	[id314] [int] NULL,
	[id315] [int] NULL,
	[id316] [int] NULL,
	[id317] [int] NULL,
	[id318] [int] NULL,
	[id319] [int] NULL,
	[id320] [int] NULL,
	[id321] [int] NULL,
	[id322] [int] NULL,
	[id323] [int] NULL,
	[id324] [int] NULL,
	[id325] [int] NULL,
	[id326] [int] NULL,
	[id327] [int] NULL,
	[id328] [int] NULL,
	[id329] [int] NULL,
	[id330] [int] NULL,
	[id331] [int] NULL,
	[id332] [int] NULL,
	[id333] [int] NULL,
	[id334] [int] NULL,
	[id335] [int] NULL,
	[id336] [int] NULL,
	[id337] [int] NULL,
	[id338] [int] NULL,
	[id339] [int] NULL,
	[id340] [int] NULL,
	[id341] [int] NULL,
	[id342] [int] NULL,
	[id343] [int] NULL,
	[id344] [int] NULL,
	[id345] [int] NULL,
	[id346] [int] NULL,
	[id347] [int] NULL,
	[id348] [int] NULL,
	[id349] [int] NULL,
	[id350] [int] NULL,
	[id351] [int] NULL,
	[id352] [int] NULL,
	[id353] [int] NULL,
	[id354] [int] NULL,
	[id355] [int] NULL,
	[id356] [int] NULL,
	[id357] [int] NULL,
	[id358] [int] NULL,
	[id359] [int] NULL,
	[id360] [int] NULL,
	[id361] [int] NULL,
	[id362] [int] NULL,
	[id363] [int] NULL,
	[id364] [int] NULL,
	[id365] [int] NULL,
	[id366] [int] NULL,
	[id367] [int] NULL,
	[id368] [int] NULL,
	[id369] [int] NULL,
	[id370] [int] NULL,
	[id371] [int] NULL,
	[id372] [int] NULL,
	[id373] [int] NULL,
	[id374] [int] NULL,
	[id375] [int] NULL,
	[id376] [int] NULL,
	[id377] [int] NULL,
	[id378] [int] NULL,
	[id379] [int] NULL,
	[id380] [int] NULL,
	[id381] [int] NULL,
	[id382] [int] NULL,
	[id383] [int] NULL,
	[id384] [int] NULL,
	[id385] [int] NULL,
	[id386] [int] NULL,
	[id387] [int] NULL,
	[id388] [int] NULL,
	[id389] [int] NULL,
	[id390] [int] NULL,
	[id391] [int] NULL,
	[id392] [int] NULL,
	[id393] [int] NULL,
	[id394] [int] NULL,
	[id395] [int] NULL,
	[id396] [int] NULL,
	[id397] [int] NULL,
	[id398] [int] NULL,
	[id399] [int] NULL,
	[id400] [int] NULL,
	[id401] [int] NULL,
	[id402] [int] NULL,
	[id403] [int] NULL,
	[id404] [int] NULL,
	[id405] [int] NULL,
	[id406] [int] NULL,
	[id407] [int] NULL,
	[id408] [int] NULL,
	[id409] [int] NULL,
	[id410] [int] NULL,
	[id411] [int] NULL,
	[id412] [int] NULL,
	[id413] [int] NULL,
	[id414] [int] NULL,
	[id415] [int] NULL,
	[id416] [int] NULL,
	[id417] [int] NULL,
	[id418] [int] NULL,
	[id419] [int] NULL,
	[id420] [int] NULL,
	[id421] [int] NULL,
	[id422] [int] NULL,
	[id423] [int] NULL,
	[id424] [int] NULL,
	[id425] [int] NULL,
	[id426] [int] NULL,
	[id427] [int] NULL,
	[id428] [int] NULL,
	[id429] [int] NULL,
	[id430] [int] NULL,
	[id431] [int] NULL,
	[id432] [int] NULL,
	[id433] [int] NULL,
	[id434] [int] NULL,
	[id435] [int] NULL,
	[id436] [int] NULL,
	[id437] [int] NULL,
	[id438] [int] NULL,
	[id439] [int] NULL,
	[id440] [int] NULL,
	[id441] [int] NULL,
	[id442] [int] NULL,
	[id443] [int] NULL,
	[id444] [int] NULL,
	[id445] [int] NULL,
	[id446] [int] NULL,
	[id447] [int] NULL,
	[id448] [int] NULL,
	[id449] [int] NULL,
	[id450] [int] NULL,
	[id451] [int] NULL,
	[id452] [int] NULL,
	[id453] [int] NULL,
	[id454] [int] NULL,
	[id455] [int] NULL,
	[id456] [int] NULL,
	[id457] [int] NULL,
	[id458] [int] NULL,
	[id459] [int] NULL,
	[id460] [int] NULL,
	[id461] [int] NULL,
	[id462] [int] NULL,
	[id463] [int] NULL,
	[id464] [int] NULL,
	[id465] [int] NULL,
	[id466] [int] NULL,
	[id467] [int] NULL,
	[id468] [int] NULL,
	[id469] [int] NULL,
	[id470] [int] NULL,
	[id471] [int] NULL,
	[id472] [int] NULL,
	[id473] [int] NULL,
	[id474] [int] NULL,
	[id475] [int] NULL,
	[id476] [int] NULL,
	[id477] [int] NULL,
	[id478] [int] NULL,
	[id479] [int] NULL,
	[id480] [int] NULL,
	[id481] [int] NULL,
	[id482] [int] NULL,
 CONSTRAINT [PK_SgrpBadgeStats] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[SgrpCustomRanks]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[SgrpCustomRanks](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Name] [nvarchar](128) NULL,
	[Permissions] [varchar](8) NULL,
 CONSTRAINT [PK_SgrpCustomRanks] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[SgrpMembers]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SgrpMembers](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Dbid] [int] NULL,
	[Rank] [int] NULL,
 CONSTRAINT [PK_SgrpMembers] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[SgrpPraetBonusIDs]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SgrpPraetBonusIDs](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
 CONSTRAINT [PK_SgrpPraetBonusIDs] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[SgrpRewardTokens]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SgrpRewardTokens](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[PieceName] [int] NULL,
	[RewardValue] [int] NULL,
	[RewardTime] [int] NULL,
 CONSTRAINT [PK_SgrpRewardTokens] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[SGTask]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[SGTask](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
	[SubHandle] [int] NULL,
	[CompoundPos] [int] NULL,
	[Seed] [int] NULL,
	[State] [int] NULL,
	[Clues] [varchar](8) NULL,
	[ClueNeedsIntro] [int] NULL,
	[SpawnGiven] [int] NULL,
	[Level] [int] NULL,
	[Timeout] [int] NULL,
	[AssignedDbId] [int] NULL,
	[AssignedTime] [int] NULL,
	[MissionMapId] [int] NULL,
	[MissionDoorMapId] [int] NULL,
	[MissionDoorPosX] [real] NULL,
	[MissionDoorPosY] [real] NULL,
	[MissionDoorPosZ] [real] NULL,
	[CompleteObjectives] [varchar](120) NULL,
	[VillainType] [varchar](512) NULL,
	[VillainCount] [int] NULL,
	[VillainType2] [varchar](512) NULL,
	[VillainCount2] [int] NULL,
	[DeliveryTargetName] [varchar](512) NULL,
	[NextVisitLocation] [int] NULL,
	[SubtaskSuccess] [int] NULL,
	[Notoriety] [int] NULL,
	[SkillLevel] [int] NULL,
	[VillainGroup] [int] NULL,
	[MapSet] [int] NULL,
	[TeamCompleted] [tinyint] NULL,
	[LevelAdjust] [int] NULL,
	[TeamSize] [int] NULL,
	[UpgradeAV] [int] NULL,
	[DowngradeBoss] [int] NULL,
	[TimerType] [int] NULL,
	[FailOnTimeout] [int] NULL,
	[TimeZero] [int] NULL,
 CONSTRAINT [PK_SGTask] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[ShardAccounts]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[ShardAccounts](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[SlotCount] [int] NULL,
	[AuthName] [varchar](64) NULL,
	[WasVIP] [int] NULL,
	[LoyaltyStatus0] [int] NULL,
	[LoyaltyStatus1] [int] NULL,
	[LoyaltyStatus2] [int] NULL,
	[LoyaltyStatus3] [int] NULL,
	[LoyaltyPointsUnspent] [int] NULL,
	[LoyaltyPointsEarned] [int] NULL,
	[AccountStatusFlags] [int] NULL,
	[AccountInventorySize] [int] NULL,
	[ShowPremiumSlotLockNag] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[SouvenirClues]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SouvenirClues](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
 CONSTRAINT [PK_SouvenirClues] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[SpecialDetails]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SpecialDetails](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Detail] [int] NULL,
	[Creation] [datetime] NULL,
	[Flags] [int] NULL,
	[Expires] [datetime] NULL,
 CONSTRAINT [PK_SpecialDetails] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Stats]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Stats](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Name] [int] NULL,
	[General_Today] [int] NULL,
	[General_Yesterday] [int] NULL,
	[General_ThisMonth] [int] NULL,
	[General_LastMonth] [int] NULL,
	[Kills_Today] [int] NULL,
	[Kills_Yesterday] [int] NULL,
	[Kills_ThisMonth] [int] NULL,
	[Kills_LastMonth] [int] NULL,
	[Deaths_Today] [int] NULL,
	[Deaths_Yesterday] [int] NULL,
	[Deaths_ThisMonth] [int] NULL,
	[Deaths_LastMonth] [int] NULL,
	[Time_Today] [int] NULL,
	[Time_Yesterday] [int] NULL,
	[Time_ThisMonth] [int] NULL,
	[Time_LastMonth] [int] NULL,
	[XP_Today] [int] NULL,
	[XP_Yesterday] [int] NULL,
	[XP_ThisMonth] [int] NULL,
	[XP_LastMonth] [int] NULL,
	[Influence_Today] [int] NULL,
	[Influence_Yesterday] [int] NULL,
	[Influence_ThisMonth] [int] NULL,
	[Influence_LastMonth] [int] NULL,
	[Wisdom_Today] [int] NULL,
	[Wisdom_Yesterday] [int] NULL,
	[Wisdom_ThisMonth] [int] NULL,
	[Wisdom_LastMonth] [int] NULL,
	[Architect_XP_Today] [int] NULL,
	[Architect_XP_Yesterday] [int] NULL,
	[Architect_XP_ThisMonth] [int] NULL,
	[Architect_XP_LastMonth] [int] NULL,
	[Architect_Influence_Today] [int] NULL,
	[Architect_Influence_Yesterday] [int] NULL,
	[Architect_Influence_ThisMonth] [int] NULL,
	[Architect_Influence_LastMonth] [int] NULL,
 CONSTRAINT [PK_Stats] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[statserver_SupergroupStats]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[statserver_SupergroupStats](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[name] [nvarchar](128) NULL,
	[idSgrp] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[StoryArcs]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[StoryArcs](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
	[Contact] [int] NULL,
	[Episode] [int] NULL,
	[Seed] [int] NULL,
	[ClueNeedsIntro] [int] NULL,
	[Clues] [varchar](8) NULL,
	[TaskComplete] [varchar](8) NULL,
	[TaskIssued] [varchar](8) NULL,
	[PlayerCreatedID] [int] NULL,
 CONSTRAINT [PK_StoryArcs] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[SuperCostumeParts]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SuperCostumeParts](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Name] [int] NULL,
	[Geom] [int] NULL,
	[Tex1] [int] NULL,
	[Tex2] [int] NULL,
	[DisplayName] [int] NULL,
	[Region] [int] NULL,
	[BodySet] [int] NULL,
	[Color1] [int] NULL,
	[Color2] [int] NULL,
	[CostumeNum] [int] NULL,
	[FxName] [int] NULL,
	[Color3] [int] NULL,
	[Color4] [int] NULL,
 CONSTRAINT [PK_SuperCostumeParts] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[SuperGroupAllies]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SuperGroupAllies](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[AllyId] [int] NULL,
	[DisplayRank] [int] NULL,
	[DontTalkTo] [int] NULL,
	[AllyName] [nvarchar](128) NULL,
 CONSTRAINT [PK_SuperGroupAllies] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Supergroups]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Supergroups](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[LeaderId] [int] NULL,
	[Motto] [nvarchar](512) NULL,
	[Name] [nvarchar](128) NULL,
	[Message] [nvarchar](512) NULL,
	[LeaderTitle] [nvarchar](128) NULL,
	[CaptainTitle] [nvarchar](128) NULL,
	[MemberTitle] [nvarchar](128) NULL,
	[Emblem] [nvarchar](256) NULL,
	[PrimaryColor] [int] NULL,
	[SecondaryColor] [int] NULL,
	[AllianceChatTalkRank] [int] NULL,
	[Description] [nvarchar](1024) NULL,
	[Tithe] [int] NULL,
	[DemoteTimeout] [int] NULL,
	[Influence] [int] NULL,
	[Prestige] [int] NULL,
	[PrestigeBase] [int] NULL,
	[PrestigeAddedUpkeep] [int] NULL,
	[UpkeepRentDue] [int] NULL,
	[UpkeepSecsRentDueDate] [int] NULL,
	[BaseEntryPermission] [int] NULL,
	[SpacesForItemsOfPower] [int] NULL,
	[PlayerType] [int] NULL,
	[Flags] [int] NULL,
	[BadgesOwned] [varchar](264) NULL,
	[BonusCount] [int] NULL,
	[DateCreated] [datetime] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[TaskForceContacts]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[TaskForceContacts](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
	[TaskIssued] [varchar](64) NULL,
	[StoryArcIssued] [varchar](8) NULL,
	[DialogSeed] [int] NULL,
	[ContactIntroSeed] [int] NULL,
	[ContactPoints] [int] NULL,
	[ContactRelationship] [int] NULL,
	[ContactsIntroduced] [int] NULL,
	[SeenPlayer] [int] NULL,
	[NotifyPlayer] [int] NULL,
	[ItemsBought] [int] NULL,
	[RewardContact] [tinyint] NULL,
	[BrokerHandle] [int] NULL,
 CONSTRAINT [PK_TaskForceContacts] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[TaskForceParameters]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TaskForceParameters](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[param] [int] NULL,
 CONSTRAINT [PK_TaskForceParameters] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Taskforces]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Taskforces](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[LeaderId] [int] NULL,
	[Name] [nvarchar](128) NULL,
	[UniqueTaskIssued] [varchar](32) NULL,
	[Level] [int] NULL,
	[LevelAdjust] [int] NULL,
	[DeleteMe] [int] NULL,
	[ExemplarLevel] [int] NULL,
	[Parameters] [int] NULL,
	[MinTeamSize] [int] NULL,
	[ArchitectId] [int] NULL,
	[ArchitectTestMode] [int] NULL,
	[ArchitectAuthId] [int] NULL,
	[PlayerStoryArc] [varbinary](max) NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[TaskForceSouvenirClues]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TaskForceSouvenirClues](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
 CONSTRAINT [PK_TaskForceSouvenirClues] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[TaskForceStoryArcs]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[TaskForceStoryArcs](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
	[Contact] [int] NULL,
	[Episode] [int] NULL,
	[Seed] [int] NULL,
	[ClueNeedsIntro] [int] NULL,
	[Clues] [varchar](8) NULL,
	[TaskComplete] [varchar](8) NULL,
	[TaskIssued] [varchar](8) NULL,
	[PlayerCreatedID] [int] NULL,
 CONSTRAINT [PK_TaskForceStoryArcs] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[TaskForceTasks]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[TaskForceTasks](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
	[SubHandle] [int] NULL,
	[CompoundPos] [int] NULL,
	[Seed] [int] NULL,
	[State] [int] NULL,
	[Clues] [varchar](8) NULL,
	[ClueNeedsIntro] [int] NULL,
	[SpawnGiven] [int] NULL,
	[Level] [int] NULL,
	[Timeout] [int] NULL,
	[AssignedDbId] [int] NULL,
	[AssignedTime] [int] NULL,
	[MissionMapId] [int] NULL,
	[MissionDoorMapId] [int] NULL,
	[MissionDoorPosX] [real] NULL,
	[MissionDoorPosY] [real] NULL,
	[MissionDoorPosZ] [real] NULL,
	[CompleteObjectives] [varchar](120) NULL,
	[VillainType] [varchar](512) NULL,
	[VillainCount] [int] NULL,
	[VillainType2] [varchar](512) NULL,
	[VillainCount2] [int] NULL,
	[DeliveryTargetName] [varchar](512) NULL,
	[NextVisitLocation] [int] NULL,
	[SubtaskSuccess] [int] NULL,
	[Notoriety] [int] NULL,
	[SkillLevel] [int] NULL,
	[VillainGroup] [int] NULL,
	[MapSet] [int] NULL,
	[TeamCompleted] [tinyint] NULL,
	[SideObjectives] [int] NULL,
	[PlayerCreated] [int] NULL,
	[PlayerCreatedID] [int] NULL,
	[LevelAdjust] [int] NULL,
	[TeamSize] [int] NULL,
	[UpgradeAV] [int] NULL,
	[DowngradeBoss] [int] NULL,
	[MysteryInvestigation_VarType] [varchar](512) NULL,
	[MysteryInvestigation_VarValue] [varchar](512) NULL,
	[TimerType] [int] NULL,
	[FailOnTimeout] [int] NULL,
	[TimeZero] [int] NULL,
 CONSTRAINT [PK_TaskForceTasks] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[Tasks]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Tasks](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
	[SubHandle] [int] NULL,
	[CompoundPos] [int] NULL,
	[Seed] [int] NULL,
	[State] [int] NULL,
	[Clues] [varchar](8) NULL,
	[ClueNeedsIntro] [int] NULL,
	[SpawnGiven] [int] NULL,
	[Level] [int] NULL,
	[Timeout] [int] NULL,
	[AssignedDbId] [int] NULL,
	[AssignedTime] [int] NULL,
	[MissionMapId] [int] NULL,
	[MissionDoorMapId] [int] NULL,
	[MissionDoorPosX] [real] NULL,
	[MissionDoorPosY] [real] NULL,
	[MissionDoorPosZ] [real] NULL,
	[CompleteObjectives] [varchar](120) NULL,
	[VillainType] [varchar](512) NULL,
	[VillainCount] [int] NULL,
	[VillainType2] [varchar](512) NULL,
	[VillainCount2] [int] NULL,
	[DeliveryTargetName] [varchar](512) NULL,
	[NextVisitLocation] [int] NULL,
	[SubtaskSuccess] [int] NULL,
	[Notoriety] [int] NULL,
	[SkillLevel] [int] NULL,
	[VillainGroup] [int] NULL,
	[MapSet] [int] NULL,
	[TeamCompleted] [tinyint] NULL,
	[SideObjectives] [int] NULL,
	[PlayerCreated] [int] NULL,
	[PlayerCreatedID] [int] NULL,
	[LevelAdjust] [int] NULL,
	[TeamSize] [int] NULL,
	[UpgradeAV] [int] NULL,
	[DowngradeBoss] [int] NULL,
	[MysteryInvestigation_VarType] [varchar](512) NULL,
	[MysteryInvestigation_VarValue] [varchar](512) NULL,
	[TimerType] [int] NULL,
	[FailOnTimeout] [int] NULL,
	[TimeZero] [int] NULL,
 CONSTRAINT [PK_Tasks] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[TeamLeaderIds]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TeamLeaderIds](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[TeamLeadId] [int] NULL,
 CONSTRAINT [PK_TeamLeaderIds] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[TeamLockStatus]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TeamLockStatus](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[TeamLockStatus] [int] NULL,
 CONSTRAINT [PK_TeamLockStatus] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[TeamupRewardTokensActive]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TeamupRewardTokensActive](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[PieceName] [int] NULL,
	[RewardValue] [int] NULL,
	[RewardTime] [int] NULL,
 CONSTRAINT [PK_TeamupRewardTokensActive] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[Teamups]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Teamups](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[LeaderId] [int] NULL,
	[MissionMapId] [int] NULL,
	[PlayersOnMap] [int] NULL,
	[InstanceType] [int] NULL,
	[Contact] [int] NULL,
	[Status] [nvarchar](512) NULL,
	[SidkickCount] [int] NULL,
	[KeyClues] [int] NULL,
	[KheldianCount] [int] NULL,
	[LastAmbush] [int] NULL,
	[TeamLevel] [int] NULL,
	[TeamMentor] [int] NULL,
	[TeamSwapLock] [int] NULL,
	[ActivePlayerDbid] [int] NULL,
	[ActivePlayerRevision] [int] NULL,
	[ProbationalActivePlayerDbid] [int] NULL,
	[ProbationalActivePlayerDbidExpiration] [int] NULL,
	[MaximumPlayerCount] [int] NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[TeamupTask]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[TeamupTask](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[ID] [int] NULL,
	[SubHandle] [int] NULL,
	[CompoundPos] [int] NULL,
	[Seed] [int] NULL,
	[State] [int] NULL,
	[Clues] [varchar](8) NULL,
	[ClueNeedsIntro] [int] NULL,
	[SpawnGiven] [int] NULL,
	[Level] [int] NULL,
	[Timeout] [int] NULL,
	[AssignedDbId] [int] NULL,
	[AssignedTime] [int] NULL,
	[MissionMapId] [int] NULL,
	[MissionDoorMapId] [int] NULL,
	[MissionDoorPosX] [real] NULL,
	[MissionDoorPosY] [real] NULL,
	[MissionDoorPosZ] [real] NULL,
	[CompleteObjectives] [varchar](120) NULL,
	[VillainType] [varchar](512) NULL,
	[VillainCount] [int] NULL,
	[VillainType2] [varchar](512) NULL,
	[VillainCount2] [int] NULL,
	[DeliveryTargetName] [varchar](512) NULL,
	[NextVisitLocation] [int] NULL,
	[SubtaskSuccess] [int] NULL,
	[Notoriety] [int] NULL,
	[SkillLevel] [int] NULL,
	[VillainGroup] [int] NULL,
	[MapSet] [int] NULL,
	[TeamCompleted] [tinyint] NULL,
	[SideObjectives] [int] NULL,
	[PlayerCreated] [int] NULL,
	[PlayerCreatedID] [int] NULL,
	[LevelAdjust] [int] NULL,
	[TeamSize] [int] NULL,
	[UpgradeAV] [int] NULL,
	[DowngradeBoss] [int] NULL,
	[MysteryInvestigation_VarType] [varchar](512) NULL,
	[MysteryInvestigation_VarValue] [varchar](512) NULL,
	[TimerType] [int] NULL,
	[FailOnTimeout] [int] NULL,
	[TimeZero] [int] NULL,
 CONSTRAINT [PK_TeamupTask] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[TestDataBaseTypes]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[TestDataBaseTypes](
	[ContainerId] [int] IDENTITY(1,1) NOT NULL,
	[Active] [int] NULL,
	[test_byte] [tinyint] NULL,
	[test_short] [smallint] NULL,
	[test_int] [int] NULL,
	[test_float] [real] NULL,
	[test_attr] [int] NULL,
	[test_conref] [int] NULL,
	[test_date] [datetime] NULL,
	[test_utf8] [nvarchar](64) NULL,
	[test_ascii] [varchar](64) NULL,
	[test_estr_utf8] [nvarchar](8) NULL,
	[test_estr_ascii] [varchar](8) NULL,
	[test_utf8_cached] [nvarchar](64) NULL,
	[test_ascii_cached] [varchar](64) NULL,
	[test_bin] [varchar](64) NULL,
	[test_large_estr_binary] [varbinary](max) NULL,
	[test_large_estr_utf8] [nvarchar](max) NULL,
	[test_large_estr_ascii] [varchar](max) NULL,
PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[Tray]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Tray](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[Type] [int] NULL,
	[InspirationCol] [int] NULL,
	[InspirationRow] [int] NULL,
	[PowerName] [int] NULL,
	[PowerSetName] [int] NULL,
	[CategoryName] [int] NULL,
	[Command] [nvarchar](512) NULL,
	[Icon] [nvarchar](512) NULL,
	[Name] [nvarchar](64) NULL,
 CONSTRAINT [PK_Tray] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[VisitedMaps]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[VisitedMaps](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[MapId] [int] NULL,
	[Cells] [varchar](2048) NULL,
 CONSTRAINT [PK_VisitedMaps] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[Windows]    Script Date: 4/19/2019 11:58:28 PM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Windows](
	[ContainerId] [int] NOT NULL,
	[SubId] [int] NOT NULL,
	[xp] [int] NULL,
	[yp] [int] NULL,
	[wd] [int] NULL,
	[ht] [int] NULL,
	[dragFrame] [int] NULL,
	[mode] [int] NULL,
	[locked] [int] NULL,
	[Color] [int] NULL,
	[BackColor] [int] NULL,
	[Scale] [real] NULL,
	[Maximized] [int] NULL,
 CONSTRAINT [PK_Windows] PRIMARY KEY CLUSTERED 
(
	[ContainerId] ASC,
	[SubId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Index [SupergroupId_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [SupergroupId_ind] ON [dbo].[Base]
(
	[SupergroupId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [UserId_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [UserId_ind] ON [dbo].[Base]
(
	[UserId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [AuthId_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [AuthId_ind] ON [dbo].[Ents]
(
	[AuthId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
SET ANSI_PADDING ON

GO
/****** Object:  Index [AuthName_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [AuthName_ind] ON [dbo].[Ents]
(
	[AuthName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
SET ANSI_PADDING ON

GO
/****** Object:  Index [Name_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [Name_ind] ON [dbo].[Ents]
(
	[Name] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [SupergroupsId_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [SupergroupsId_ind] ON [dbo].[Ents]
(
	[SupergroupsId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [TaskforcesId_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [TaskforcesId_ind] ON [dbo].[Ents]
(
	[TaskforcesId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [TeamupsId_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [TeamupsId_ind] ON [dbo].[Ents]
(
	[TeamupsId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
SET ANSI_PADDING ON

GO
/****** Object:  Index [AccSvrLock_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [AccSvrLock_ind] ON [dbo].[Ents2]
(
	[AccSvrLock] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [LastDayJobsStart_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [LastDayJobsStart_ind] ON [dbo].[Ents2]
(
	[LastDayJobsStart] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [LeaguesId_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [LeaguesId_ind] ON [dbo].[Ents2]
(
	[LeaguesId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [LevelingPactsId_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [LevelingPactsId_ind] ON [dbo].[Ents2]
(
	[LevelingPactsId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [RaidsId_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [RaidsId_ind] ON [dbo].[Ents2]
(
	[RaidsId] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
SET ANSI_PADDING ON

GO
/****** Object:  Index [Field_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [Field_ind] ON [dbo].[MinedData]
(
	[Field] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
SET ANSI_PADDING ON

GO
/****** Object:  Index [Name_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [Name_ind] ON [dbo].[MiningAccumulator]
(
	[Name] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [Date_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [Date_ind] ON [dbo].[Petitions]
(
	[Date] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [Petitions_Fetched_date]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [Petitions_Fetched_date] ON [dbo].[Petitions]
(
	[Fetched] ASC,
	[Date] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [Recipient_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [Recipient_ind] ON [dbo].[Recipients]
(
	[Recipient] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
SET ANSI_PADDING ON

GO
/****** Object:  Index [AuthName_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [AuthName_ind] ON [dbo].[ShardAccounts]
(
	[AuthName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
SET ANSI_PADDING ON

GO
/****** Object:  Index [Name_ind]    Script Date: 4/19/2019 11:58:28 PM ******/
CREATE NONCLUSTERED INDEX [Name_ind] ON [dbo].[Supergroups]
(
	[Name] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[AccountInventory]  WITH CHECK ADD  CONSTRAINT [FK_AccountInventory_ContainerId_ShardAccounts] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[ShardAccounts] ([ContainerId])
GO
ALTER TABLE [dbo].[AccountInventory] CHECK CONSTRAINT [FK_AccountInventory_ContainerId_ShardAccounts]
GO
ALTER TABLE [dbo].[Appearance]  WITH CHECK ADD  CONSTRAINT [FK_Appearance_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Appearance] CHECK CONSTRAINT [FK_Appearance_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Appearance]  WITH CHECK ADD  CONSTRAINT [FK_Appearance_PrimaryPowerToken_Attributes] FOREIGN KEY([PrimaryPowerToken])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Appearance] CHECK CONSTRAINT [FK_Appearance_PrimaryPowerToken_Attributes]
GO
ALTER TABLE [dbo].[Appearance]  WITH CHECK ADD  CONSTRAINT [FK_Appearance_SecondaryPowerToken_Attributes] FOREIGN KEY([SecondaryPowerToken])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Appearance] CHECK CONSTRAINT [FK_Appearance_SecondaryPowerToken_Attributes]
GO
ALTER TABLE [dbo].[AttackerParticipants]  WITH CHECK ADD  CONSTRAINT [FK_AttackerParticipants_ContainerId_BaseRaids] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[BaseRaids] ([ContainerId])
GO
ALTER TABLE [dbo].[AttackerParticipants] CHECK CONSTRAINT [FK_AttackerParticipants_ContainerId_BaseRaids]
GO
ALTER TABLE [dbo].[AttribMods]  WITH CHECK ADD  CONSTRAINT [FK_AttribMods_CategoryName_Attributes] FOREIGN KEY([CategoryName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[AttribMods] CHECK CONSTRAINT [FK_AttribMods_CategoryName_Attributes]
GO
ALTER TABLE [dbo].[AttribMods]  WITH CHECK ADD  CONSTRAINT [FK_AttribMods_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[AttribMods] CHECK CONSTRAINT [FK_AttribMods_ContainerId_Ents]
GO
ALTER TABLE [dbo].[AttribMods]  WITH CHECK ADD  CONSTRAINT [FK_AttribMods_CustomFXToken_Attributes] FOREIGN KEY([CustomFXToken])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[AttribMods] CHECK CONSTRAINT [FK_AttribMods_CustomFXToken_Attributes]
GO
ALTER TABLE [dbo].[AttribMods]  WITH CHECK ADD  CONSTRAINT [FK_AttribMods_PowerName_Attributes] FOREIGN KEY([PowerName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[AttribMods] CHECK CONSTRAINT [FK_AttribMods_PowerName_Attributes]
GO
ALTER TABLE [dbo].[AttribMods]  WITH CHECK ADD  CONSTRAINT [FK_AttribMods_PowerSetName_Attributes] FOREIGN KEY([PowerSetName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[AttribMods] CHECK CONSTRAINT [FK_AttribMods_PowerSetName_Attributes]
GO
ALTER TABLE [dbo].[BadgeMonitor]  WITH CHECK ADD  CONSTRAINT [FK_BadgeMonitor_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[BadgeMonitor] CHECK CONSTRAINT [FK_BadgeMonitor_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Badges]  WITH CHECK ADD  CONSTRAINT [FK_Badges_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Badges] CHECK CONSTRAINT [FK_Badges_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Badges01]  WITH CHECK ADD  CONSTRAINT [FK_Badges01_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Badges01] CHECK CONSTRAINT [FK_Badges01_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Badges02]  WITH CHECK ADD  CONSTRAINT [FK_Badges02_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Badges02] CHECK CONSTRAINT [FK_Badges02_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Base]  WITH CHECK ADD  CONSTRAINT [FK_Base_SupergroupId_Supergroups] FOREIGN KEY([SupergroupId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[Base] CHECK CONSTRAINT [FK_Base_SupergroupId_Supergroups]
GO
ALTER TABLE [dbo].[Boosts]  WITH CHECK ADD  CONSTRAINT [FK_Boosts_BoostName_Attributes] FOREIGN KEY([BoostName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Boosts] CHECK CONSTRAINT [FK_Boosts_BoostName_Attributes]
GO
ALTER TABLE [dbo].[Boosts]  WITH CHECK ADD  CONSTRAINT [FK_Boosts_CategoryName_Attributes] FOREIGN KEY([CategoryName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Boosts] CHECK CONSTRAINT [FK_Boosts_CategoryName_Attributes]
GO
ALTER TABLE [dbo].[Boosts]  WITH CHECK ADD  CONSTRAINT [FK_Boosts_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Boosts] CHECK CONSTRAINT [FK_Boosts_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Boosts]  WITH CHECK ADD  CONSTRAINT [FK_Boosts_PowerSetName_Attributes] FOREIGN KEY([PowerSetName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Boosts] CHECK CONSTRAINT [FK_Boosts_PowerSetName_Attributes]
GO
ALTER TABLE [dbo].[CertificationHistory]  WITH CHECK ADD  CONSTRAINT [FK_CertificationHistory_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[CertificationHistory] CHECK CONSTRAINT [FK_CertificationHistory_ContainerId_Ents]
GO
ALTER TABLE [dbo].[ChatChannels]  WITH CHECK ADD  CONSTRAINT [FK_ChatChannels_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[ChatChannels] CHECK CONSTRAINT [FK_ChatChannels_ContainerId_Ents]
GO
ALTER TABLE [dbo].[ChatTabs]  WITH CHECK ADD  CONSTRAINT [FK_ChatTabs_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[ChatTabs] CHECK CONSTRAINT [FK_ChatTabs_ContainerId_Ents]
GO
ALTER TABLE [dbo].[ChatWindows]  WITH CHECK ADD  CONSTRAINT [FK_ChatWindows_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[ChatWindows] CHECK CONSTRAINT [FK_ChatWindows_ContainerId_Ents]
GO
ALTER TABLE [dbo].[CombatMonitorStat]  WITH CHECK ADD  CONSTRAINT [FK_CombatMonitorStat_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[CombatMonitorStat] CHECK CONSTRAINT [FK_CombatMonitorStat_ContainerId_Ents]
GO
ALTER TABLE [dbo].[CompletedOrders]  WITH CHECK ADD  CONSTRAINT [FK_CompletedOrders_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[CompletedOrders] CHECK CONSTRAINT [FK_CompletedOrders_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Contacts]  WITH CHECK ADD  CONSTRAINT [FK_Contacts_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Contacts] CHECK CONSTRAINT [FK_Contacts_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Contacts]  WITH CHECK ADD  CONSTRAINT [FK_Contacts_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Contacts] CHECK CONSTRAINT [FK_Contacts_ID_Attributes]
GO
ALTER TABLE [dbo].[CostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_CostumeParts_BodySet_Attributes] FOREIGN KEY([BodySet])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[CostumeParts] CHECK CONSTRAINT [FK_CostumeParts_BodySet_Attributes]
GO
ALTER TABLE [dbo].[CostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_CostumeParts_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[CostumeParts] CHECK CONSTRAINT [FK_CostumeParts_ContainerId_Ents]
GO
ALTER TABLE [dbo].[CostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_CostumeParts_DisplayName_Attributes] FOREIGN KEY([DisplayName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[CostumeParts] CHECK CONSTRAINT [FK_CostumeParts_DisplayName_Attributes]
GO
ALTER TABLE [dbo].[CostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_CostumeParts_FxName_Attributes] FOREIGN KEY([FxName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[CostumeParts] CHECK CONSTRAINT [FK_CostumeParts_FxName_Attributes]
GO
ALTER TABLE [dbo].[CostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_CostumeParts_Geom_Attributes] FOREIGN KEY([Geom])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[CostumeParts] CHECK CONSTRAINT [FK_CostumeParts_Geom_Attributes]
GO
ALTER TABLE [dbo].[CostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_CostumeParts_Name_Attributes] FOREIGN KEY([Name])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[CostumeParts] CHECK CONSTRAINT [FK_CostumeParts_Name_Attributes]
GO
ALTER TABLE [dbo].[CostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_CostumeParts_Region_Attributes] FOREIGN KEY([Region])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[CostumeParts] CHECK CONSTRAINT [FK_CostumeParts_Region_Attributes]
GO
ALTER TABLE [dbo].[CostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_CostumeParts_Tex1_Attributes] FOREIGN KEY([Tex1])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[CostumeParts] CHECK CONSTRAINT [FK_CostumeParts_Tex1_Attributes]
GO
ALTER TABLE [dbo].[CostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_CostumeParts_Tex2_Attributes] FOREIGN KEY([Tex2])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[CostumeParts] CHECK CONSTRAINT [FK_CostumeParts_Tex2_Attributes]
GO
ALTER TABLE [dbo].[DefeatRecord]  WITH CHECK ADD  CONSTRAINT [FK_DefeatRecord_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[DefeatRecord] CHECK CONSTRAINT [FK_DefeatRecord_ContainerId_Ents]
GO
ALTER TABLE [dbo].[DefenderParticipants]  WITH CHECK ADD  CONSTRAINT [FK_DefenderParticipants_ContainerId_BaseRaids] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[BaseRaids] ([ContainerId])
GO
ALTER TABLE [dbo].[DefenderParticipants] CHECK CONSTRAINT [FK_DefenderParticipants_ContainerId_BaseRaids]
GO
ALTER TABLE [dbo].[Ents]  WITH CHECK ADD  CONSTRAINT [FK_Ents_Class_Attributes] FOREIGN KEY([Class])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Ents] CHECK CONSTRAINT [FK_Ents_Class_Attributes]
GO
ALTER TABLE [dbo].[Ents]  WITH CHECK ADD  CONSTRAINT [FK_Ents_Origin_Attributes] FOREIGN KEY([Origin])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Ents] CHECK CONSTRAINT [FK_Ents_Origin_Attributes]
GO
ALTER TABLE [dbo].[Ents]  WITH CHECK ADD  CONSTRAINT [FK_Ents_SupergroupsId_Supergroups] FOREIGN KEY([SupergroupsId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[Ents] CHECK CONSTRAINT [FK_Ents_SupergroupsId_Supergroups]
GO
ALTER TABLE [dbo].[Ents]  WITH CHECK ADD  CONSTRAINT [FK_Ents_TaskforcesId_Taskforces] FOREIGN KEY([TaskforcesId])
REFERENCES [dbo].[Taskforces] ([ContainerId])
GO
ALTER TABLE [dbo].[Ents] CHECK CONSTRAINT [FK_Ents_TaskforcesId_Taskforces]
GO
ALTER TABLE [dbo].[Ents2]  WITH CHECK ADD  CONSTRAINT [FK_Ents2_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Ents2] CHECK CONSTRAINT [FK_Ents2_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Ents2]  WITH CHECK ADD  CONSTRAINT [FK_Ents2_LevelingPactsId_LevelingPacts] FOREIGN KEY([LevelingPactsId])
REFERENCES [dbo].[LevelingPacts] ([ContainerId])
GO
ALTER TABLE [dbo].[Ents2] CHECK CONSTRAINT [FK_Ents2_LevelingPactsId_LevelingPacts]
GO
ALTER TABLE [dbo].[EventHistory]  WITH CHECK ADD  CONSTRAINT [FK_EventHistory_Class_Attributes] FOREIGN KEY([Class])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[EventHistory] CHECK CONSTRAINT [FK_EventHistory_Class_Attributes]
GO
ALTER TABLE [dbo].[EventHistory]  WITH CHECK ADD  CONSTRAINT [FK_EventHistory_PrimaryPowerset_Attributes] FOREIGN KEY([PrimaryPowerset])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[EventHistory] CHECK CONSTRAINT [FK_EventHistory_PrimaryPowerset_Attributes]
GO
ALTER TABLE [dbo].[EventHistory]  WITH CHECK ADD  CONSTRAINT [FK_EventHistory_SecondaryPowerset_Attributes] FOREIGN KEY([SecondaryPowerset])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[EventHistory] CHECK CONSTRAINT [FK_EventHistory_SecondaryPowerset_Attributes]
GO
ALTER TABLE [dbo].[FameStrings]  WITH CHECK ADD  CONSTRAINT [FK_FameStrings_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[FameStrings] CHECK CONSTRAINT [FK_FameStrings_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Friends]  WITH CHECK ADD  CONSTRAINT [FK_Friends_Class_Attributes] FOREIGN KEY([Class])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Friends] CHECK CONSTRAINT [FK_Friends_Class_Attributes]
GO
ALTER TABLE [dbo].[Friends]  WITH CHECK ADD  CONSTRAINT [FK_Friends_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Friends] CHECK CONSTRAINT [FK_Friends_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Friends]  WITH CHECK ADD  CONSTRAINT [FK_Friends_Origin_Attributes] FOREIGN KEY([Origin])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Friends] CHECK CONSTRAINT [FK_Friends_Origin_Attributes]
GO
ALTER TABLE [dbo].[GmailClaims]  WITH CHECK ADD  CONSTRAINT [FK_GmailClaims_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[GmailClaims] CHECK CONSTRAINT [FK_GmailClaims_ContainerId_Ents]
GO
ALTER TABLE [dbo].[GmailPending]  WITH CHECK ADD  CONSTRAINT [FK_GmailPending_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[GmailPending] CHECK CONSTRAINT [FK_GmailPending_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Ignore]  WITH CHECK ADD  CONSTRAINT [FK_Ignore_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Ignore] CHECK CONSTRAINT [FK_Ignore_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Inspirations]  WITH CHECK ADD  CONSTRAINT [FK_Inspirations_CategoryName_Attributes] FOREIGN KEY([CategoryName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Inspirations] CHECK CONSTRAINT [FK_Inspirations_CategoryName_Attributes]
GO
ALTER TABLE [dbo].[Inspirations]  WITH CHECK ADD  CONSTRAINT [FK_Inspirations_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Inspirations] CHECK CONSTRAINT [FK_Inspirations_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Inspirations]  WITH CHECK ADD  CONSTRAINT [FK_Inspirations_PowerName_Attributes] FOREIGN KEY([PowerName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Inspirations] CHECK CONSTRAINT [FK_Inspirations_PowerName_Attributes]
GO
ALTER TABLE [dbo].[Inspirations]  WITH CHECK ADD  CONSTRAINT [FK_Inspirations_PowerSetName_Attributes] FOREIGN KEY([PowerSetName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Inspirations] CHECK CONSTRAINT [FK_Inspirations_PowerSetName_Attributes]
GO
ALTER TABLE [dbo].[InvBaseDetail]  WITH CHECK ADD  CONSTRAINT [FK_InvBaseDetail_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[InvBaseDetail] CHECK CONSTRAINT [FK_InvBaseDetail_ContainerId_Ents]
GO
ALTER TABLE [dbo].[InvBaseDetail]  WITH CHECK ADD  CONSTRAINT [FK_InvBaseDetail_name_Attributes] FOREIGN KEY([name])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvBaseDetail] CHECK CONSTRAINT [FK_InvBaseDetail_name_Attributes]
GO
ALTER TABLE [dbo].[InvRecipe0]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipe0_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[InvRecipe0] CHECK CONSTRAINT [FK_InvRecipe0_ContainerId_Ents]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c0Type_Attributes] FOREIGN KEY([c0Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c0Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c100Type_Attributes] FOREIGN KEY([c100Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c100Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c101Type_Attributes] FOREIGN KEY([c101Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c101Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c102Type_Attributes] FOREIGN KEY([c102Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c102Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c103Type_Attributes] FOREIGN KEY([c103Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c103Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c104Type_Attributes] FOREIGN KEY([c104Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c104Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c105Type_Attributes] FOREIGN KEY([c105Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c105Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c106Type_Attributes] FOREIGN KEY([c106Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c106Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c107Type_Attributes] FOREIGN KEY([c107Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c107Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c108Type_Attributes] FOREIGN KEY([c108Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c108Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c109Type_Attributes] FOREIGN KEY([c109Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c109Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c10Type_Attributes] FOREIGN KEY([c10Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c10Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c110Type_Attributes] FOREIGN KEY([c110Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c110Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c111Type_Attributes] FOREIGN KEY([c111Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c111Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c112Type_Attributes] FOREIGN KEY([c112Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c112Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c113Type_Attributes] FOREIGN KEY([c113Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c113Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c114Type_Attributes] FOREIGN KEY([c114Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c114Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c115Type_Attributes] FOREIGN KEY([c115Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c115Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c116Type_Attributes] FOREIGN KEY([c116Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c116Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c117Type_Attributes] FOREIGN KEY([c117Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c117Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c118Type_Attributes] FOREIGN KEY([c118Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c118Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c119Type_Attributes] FOREIGN KEY([c119Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c119Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c11Type_Attributes] FOREIGN KEY([c11Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c11Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c120Type_Attributes] FOREIGN KEY([c120Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c120Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c121Type_Attributes] FOREIGN KEY([c121Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c121Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c122Type_Attributes] FOREIGN KEY([c122Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c122Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c123Type_Attributes] FOREIGN KEY([c123Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c123Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c124Type_Attributes] FOREIGN KEY([c124Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c124Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c125Type_Attributes] FOREIGN KEY([c125Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c125Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c126Type_Attributes] FOREIGN KEY([c126Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c126Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c127Type_Attributes] FOREIGN KEY([c127Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c127Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c128Type_Attributes] FOREIGN KEY([c128Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c128Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c129Type_Attributes] FOREIGN KEY([c129Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c129Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c12Type_Attributes] FOREIGN KEY([c12Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c12Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c130Type_Attributes] FOREIGN KEY([c130Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c130Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c131Type_Attributes] FOREIGN KEY([c131Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c131Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c132Type_Attributes] FOREIGN KEY([c132Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c132Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c133Type_Attributes] FOREIGN KEY([c133Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c133Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c134Type_Attributes] FOREIGN KEY([c134Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c134Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c135Type_Attributes] FOREIGN KEY([c135Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c135Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c136Type_Attributes] FOREIGN KEY([c136Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c136Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c137Type_Attributes] FOREIGN KEY([c137Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c137Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c138Type_Attributes] FOREIGN KEY([c138Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c138Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c139Type_Attributes] FOREIGN KEY([c139Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c139Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c13Type_Attributes] FOREIGN KEY([c13Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c13Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c140Type_Attributes] FOREIGN KEY([c140Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c140Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c141Type_Attributes] FOREIGN KEY([c141Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c141Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c142Type_Attributes] FOREIGN KEY([c142Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c142Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c143Type_Attributes] FOREIGN KEY([c143Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c143Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c144Type_Attributes] FOREIGN KEY([c144Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c144Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c145Type_Attributes] FOREIGN KEY([c145Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c145Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c146Type_Attributes] FOREIGN KEY([c146Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c146Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c147Type_Attributes] FOREIGN KEY([c147Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c147Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c148Type_Attributes] FOREIGN KEY([c148Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c148Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c149Type_Attributes] FOREIGN KEY([c149Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c149Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c14Type_Attributes] FOREIGN KEY([c14Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c14Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c150Type_Attributes] FOREIGN KEY([c150Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c150Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c151Type_Attributes] FOREIGN KEY([c151Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c151Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c152Type_Attributes] FOREIGN KEY([c152Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c152Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c153Type_Attributes] FOREIGN KEY([c153Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c153Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c154Type_Attributes] FOREIGN KEY([c154Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c154Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c155Type_Attributes] FOREIGN KEY([c155Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c155Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c156Type_Attributes] FOREIGN KEY([c156Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c156Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c157Type_Attributes] FOREIGN KEY([c157Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c157Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c158Type_Attributes] FOREIGN KEY([c158Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c158Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c159Type_Attributes] FOREIGN KEY([c159Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c159Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c15Type_Attributes] FOREIGN KEY([c15Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c15Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c160Type_Attributes] FOREIGN KEY([c160Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c160Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c161Type_Attributes] FOREIGN KEY([c161Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c161Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c162Type_Attributes] FOREIGN KEY([c162Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c162Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c163Type_Attributes] FOREIGN KEY([c163Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c163Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c164Type_Attributes] FOREIGN KEY([c164Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c164Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c165Type_Attributes] FOREIGN KEY([c165Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c165Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c166Type_Attributes] FOREIGN KEY([c166Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c166Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c167Type_Attributes] FOREIGN KEY([c167Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c167Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c168Type_Attributes] FOREIGN KEY([c168Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c168Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c169Type_Attributes] FOREIGN KEY([c169Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c169Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c16Type_Attributes] FOREIGN KEY([c16Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c16Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c170Type_Attributes] FOREIGN KEY([c170Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c170Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c171Type_Attributes] FOREIGN KEY([c171Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c171Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c172Type_Attributes] FOREIGN KEY([c172Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c172Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c173Type_Attributes] FOREIGN KEY([c173Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c173Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c174Type_Attributes] FOREIGN KEY([c174Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c174Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c175Type_Attributes] FOREIGN KEY([c175Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c175Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c176Type_Attributes] FOREIGN KEY([c176Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c176Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c177Type_Attributes] FOREIGN KEY([c177Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c177Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c178Type_Attributes] FOREIGN KEY([c178Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c178Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c179Type_Attributes] FOREIGN KEY([c179Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c179Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c17Type_Attributes] FOREIGN KEY([c17Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c17Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c180Type_Attributes] FOREIGN KEY([c180Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c180Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c181Type_Attributes] FOREIGN KEY([c181Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c181Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c182Type_Attributes] FOREIGN KEY([c182Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c182Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c183Type_Attributes] FOREIGN KEY([c183Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c183Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c184Type_Attributes] FOREIGN KEY([c184Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c184Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c185Type_Attributes] FOREIGN KEY([c185Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c185Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c186Type_Attributes] FOREIGN KEY([c186Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c186Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c187Type_Attributes] FOREIGN KEY([c187Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c187Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c188Type_Attributes] FOREIGN KEY([c188Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c188Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c189Type_Attributes] FOREIGN KEY([c189Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c189Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c18Type_Attributes] FOREIGN KEY([c18Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c18Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c190Type_Attributes] FOREIGN KEY([c190Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c190Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c191Type_Attributes] FOREIGN KEY([c191Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c191Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c192Type_Attributes] FOREIGN KEY([c192Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c192Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c193Type_Attributes] FOREIGN KEY([c193Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c193Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c194Type_Attributes] FOREIGN KEY([c194Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c194Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c195Type_Attributes] FOREIGN KEY([c195Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c195Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c196Type_Attributes] FOREIGN KEY([c196Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c196Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c197Type_Attributes] FOREIGN KEY([c197Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c197Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c198Type_Attributes] FOREIGN KEY([c198Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c198Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c199Type_Attributes] FOREIGN KEY([c199Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c199Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c19Type_Attributes] FOREIGN KEY([c19Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c19Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c1Type_Attributes] FOREIGN KEY([c1Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c1Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c200Type_Attributes] FOREIGN KEY([c200Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c200Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c201Type_Attributes] FOREIGN KEY([c201Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c201Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c202Type_Attributes] FOREIGN KEY([c202Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c202Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c203Type_Attributes] FOREIGN KEY([c203Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c203Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c204Type_Attributes] FOREIGN KEY([c204Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c204Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c205Type_Attributes] FOREIGN KEY([c205Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c205Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c206Type_Attributes] FOREIGN KEY([c206Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c206Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c207Type_Attributes] FOREIGN KEY([c207Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c207Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c208Type_Attributes] FOREIGN KEY([c208Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c208Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c209Type_Attributes] FOREIGN KEY([c209Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c209Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c20Type_Attributes] FOREIGN KEY([c20Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c20Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c210Type_Attributes] FOREIGN KEY([c210Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c210Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c211Type_Attributes] FOREIGN KEY([c211Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c211Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c212Type_Attributes] FOREIGN KEY([c212Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c212Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c213Type_Attributes] FOREIGN KEY([c213Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c213Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c214Type_Attributes] FOREIGN KEY([c214Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c214Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c215Type_Attributes] FOREIGN KEY([c215Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c215Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c216Type_Attributes] FOREIGN KEY([c216Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c216Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c217Type_Attributes] FOREIGN KEY([c217Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c217Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c218Type_Attributes] FOREIGN KEY([c218Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c218Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c219Type_Attributes] FOREIGN KEY([c219Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c219Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c21Type_Attributes] FOREIGN KEY([c21Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c21Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c220Type_Attributes] FOREIGN KEY([c220Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c220Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c221Type_Attributes] FOREIGN KEY([c221Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c221Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c222Type_Attributes] FOREIGN KEY([c222Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c222Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c223Type_Attributes] FOREIGN KEY([c223Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c223Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c224Type_Attributes] FOREIGN KEY([c224Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c224Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c225Type_Attributes] FOREIGN KEY([c225Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c225Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c226Type_Attributes] FOREIGN KEY([c226Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c226Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c227Type_Attributes] FOREIGN KEY([c227Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c227Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c228Type_Attributes] FOREIGN KEY([c228Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c228Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c229Type_Attributes] FOREIGN KEY([c229Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c229Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c22Type_Attributes] FOREIGN KEY([c22Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c22Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c230Type_Attributes] FOREIGN KEY([c230Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c230Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c231Type_Attributes] FOREIGN KEY([c231Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c231Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c232Type_Attributes] FOREIGN KEY([c232Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c232Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c233Type_Attributes] FOREIGN KEY([c233Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c233Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c234Type_Attributes] FOREIGN KEY([c234Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c234Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c235Type_Attributes] FOREIGN KEY([c235Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c235Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c236Type_Attributes] FOREIGN KEY([c236Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c236Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c237Type_Attributes] FOREIGN KEY([c237Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c237Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c238Type_Attributes] FOREIGN KEY([c238Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c238Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c239Type_Attributes] FOREIGN KEY([c239Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c239Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c23Type_Attributes] FOREIGN KEY([c23Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c23Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c240Type_Attributes] FOREIGN KEY([c240Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c240Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c241Type_Attributes] FOREIGN KEY([c241Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c241Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c242Type_Attributes] FOREIGN KEY([c242Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c242Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c243Type_Attributes] FOREIGN KEY([c243Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c243Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c244Type_Attributes] FOREIGN KEY([c244Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c244Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c245Type_Attributes] FOREIGN KEY([c245Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c245Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c246Type_Attributes] FOREIGN KEY([c246Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c246Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c247Type_Attributes] FOREIGN KEY([c247Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c247Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c248Type_Attributes] FOREIGN KEY([c248Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c248Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c249Type_Attributes] FOREIGN KEY([c249Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c249Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c24Type_Attributes] FOREIGN KEY([c24Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c24Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c250Type_Attributes] FOREIGN KEY([c250Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c250Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c251Type_Attributes] FOREIGN KEY([c251Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c251Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c252Type_Attributes] FOREIGN KEY([c252Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c252Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c253Type_Attributes] FOREIGN KEY([c253Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c253Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c254Type_Attributes] FOREIGN KEY([c254Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c254Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c255Type_Attributes] FOREIGN KEY([c255Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c255Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c256Type_Attributes] FOREIGN KEY([c256Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c256Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c257Type_Attributes] FOREIGN KEY([c257Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c257Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c258Type_Attributes] FOREIGN KEY([c258Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c258Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c259Type_Attributes] FOREIGN KEY([c259Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c259Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c25Type_Attributes] FOREIGN KEY([c25Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c25Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c260Type_Attributes] FOREIGN KEY([c260Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c260Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c261Type_Attributes] FOREIGN KEY([c261Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c261Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c262Type_Attributes] FOREIGN KEY([c262Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c262Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c263Type_Attributes] FOREIGN KEY([c263Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c263Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c264Type_Attributes] FOREIGN KEY([c264Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c264Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c265Type_Attributes] FOREIGN KEY([c265Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c265Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c266Type_Attributes] FOREIGN KEY([c266Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c266Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c267Type_Attributes] FOREIGN KEY([c267Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c267Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c268Type_Attributes] FOREIGN KEY([c268Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c268Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c269Type_Attributes] FOREIGN KEY([c269Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c269Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c26Type_Attributes] FOREIGN KEY([c26Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c26Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c270Type_Attributes] FOREIGN KEY([c270Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c270Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c271Type_Attributes] FOREIGN KEY([c271Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c271Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c272Type_Attributes] FOREIGN KEY([c272Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c272Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c273Type_Attributes] FOREIGN KEY([c273Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c273Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c274Type_Attributes] FOREIGN KEY([c274Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c274Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c275Type_Attributes] FOREIGN KEY([c275Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c275Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c276Type_Attributes] FOREIGN KEY([c276Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c276Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c277Type_Attributes] FOREIGN KEY([c277Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c277Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c278Type_Attributes] FOREIGN KEY([c278Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c278Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c279Type_Attributes] FOREIGN KEY([c279Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c279Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c27Type_Attributes] FOREIGN KEY([c27Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c27Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c280Type_Attributes] FOREIGN KEY([c280Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c280Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c281Type_Attributes] FOREIGN KEY([c281Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c281Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c282Type_Attributes] FOREIGN KEY([c282Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c282Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c283Type_Attributes] FOREIGN KEY([c283Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c283Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c284Type_Attributes] FOREIGN KEY([c284Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c284Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c285Type_Attributes] FOREIGN KEY([c285Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c285Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c286Type_Attributes] FOREIGN KEY([c286Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c286Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c287Type_Attributes] FOREIGN KEY([c287Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c287Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c288Type_Attributes] FOREIGN KEY([c288Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c288Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c289Type_Attributes] FOREIGN KEY([c289Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c289Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c28Type_Attributes] FOREIGN KEY([c28Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c28Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c290Type_Attributes] FOREIGN KEY([c290Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c290Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c291Type_Attributes] FOREIGN KEY([c291Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c291Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c292Type_Attributes] FOREIGN KEY([c292Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c292Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c293Type_Attributes] FOREIGN KEY([c293Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c293Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c294Type_Attributes] FOREIGN KEY([c294Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c294Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c295Type_Attributes] FOREIGN KEY([c295Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c295Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c296Type_Attributes] FOREIGN KEY([c296Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c296Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c297Type_Attributes] FOREIGN KEY([c297Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c297Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c298Type_Attributes] FOREIGN KEY([c298Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c298Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c299Type_Attributes] FOREIGN KEY([c299Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c299Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c29Type_Attributes] FOREIGN KEY([c29Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c29Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c2Type_Attributes] FOREIGN KEY([c2Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c2Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c300Type_Attributes] FOREIGN KEY([c300Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c300Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c301Type_Attributes] FOREIGN KEY([c301Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c301Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c302Type_Attributes] FOREIGN KEY([c302Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c302Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c303Type_Attributes] FOREIGN KEY([c303Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c303Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c304Type_Attributes] FOREIGN KEY([c304Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c304Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c305Type_Attributes] FOREIGN KEY([c305Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c305Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c306Type_Attributes] FOREIGN KEY([c306Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c306Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c307Type_Attributes] FOREIGN KEY([c307Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c307Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c308Type_Attributes] FOREIGN KEY([c308Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c308Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c309Type_Attributes] FOREIGN KEY([c309Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c309Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c30Type_Attributes] FOREIGN KEY([c30Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c30Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c310Type_Attributes] FOREIGN KEY([c310Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c310Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c311Type_Attributes] FOREIGN KEY([c311Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c311Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c312Type_Attributes] FOREIGN KEY([c312Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c312Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c313Type_Attributes] FOREIGN KEY([c313Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c313Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c314Type_Attributes] FOREIGN KEY([c314Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c314Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c315Type_Attributes] FOREIGN KEY([c315Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c315Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c316Type_Attributes] FOREIGN KEY([c316Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c316Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c317Type_Attributes] FOREIGN KEY([c317Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c317Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c318Type_Attributes] FOREIGN KEY([c318Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c318Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c319Type_Attributes] FOREIGN KEY([c319Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c319Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c31Type_Attributes] FOREIGN KEY([c31Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c31Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c320Type_Attributes] FOREIGN KEY([c320Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c320Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c321Type_Attributes] FOREIGN KEY([c321Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c321Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c322Type_Attributes] FOREIGN KEY([c322Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c322Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c323Type_Attributes] FOREIGN KEY([c323Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c323Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c324Type_Attributes] FOREIGN KEY([c324Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c324Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c325Type_Attributes] FOREIGN KEY([c325Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c325Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c326Type_Attributes] FOREIGN KEY([c326Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c326Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c327Type_Attributes] FOREIGN KEY([c327Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c327Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c328Type_Attributes] FOREIGN KEY([c328Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c328Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c329Type_Attributes] FOREIGN KEY([c329Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c329Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c32Type_Attributes] FOREIGN KEY([c32Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c32Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c330Type_Attributes] FOREIGN KEY([c330Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c330Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c331Type_Attributes] FOREIGN KEY([c331Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c331Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c332Type_Attributes] FOREIGN KEY([c332Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c332Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c333Type_Attributes] FOREIGN KEY([c333Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c333Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c334Type_Attributes] FOREIGN KEY([c334Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c334Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c335Type_Attributes] FOREIGN KEY([c335Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c335Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c336Type_Attributes] FOREIGN KEY([c336Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c336Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c337Type_Attributes] FOREIGN KEY([c337Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c337Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c338Type_Attributes] FOREIGN KEY([c338Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c338Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c339Type_Attributes] FOREIGN KEY([c339Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c339Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c33Type_Attributes] FOREIGN KEY([c33Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c33Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c340Type_Attributes] FOREIGN KEY([c340Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c340Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c341Type_Attributes] FOREIGN KEY([c341Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c341Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c342Type_Attributes] FOREIGN KEY([c342Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c342Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c343Type_Attributes] FOREIGN KEY([c343Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c343Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c344Type_Attributes] FOREIGN KEY([c344Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c344Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c345Type_Attributes] FOREIGN KEY([c345Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c345Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c346Type_Attributes] FOREIGN KEY([c346Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c346Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c347Type_Attributes] FOREIGN KEY([c347Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c347Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c348Type_Attributes] FOREIGN KEY([c348Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c348Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c349Type_Attributes] FOREIGN KEY([c349Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c349Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c34Type_Attributes] FOREIGN KEY([c34Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c34Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c350Type_Attributes] FOREIGN KEY([c350Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c350Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c351Type_Attributes] FOREIGN KEY([c351Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c351Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c352Type_Attributes] FOREIGN KEY([c352Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c352Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c353Type_Attributes] FOREIGN KEY([c353Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c353Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c354Type_Attributes] FOREIGN KEY([c354Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c354Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c355Type_Attributes] FOREIGN KEY([c355Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c355Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c356Type_Attributes] FOREIGN KEY([c356Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c356Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c357Type_Attributes] FOREIGN KEY([c357Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c357Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c358Type_Attributes] FOREIGN KEY([c358Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c358Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c359Type_Attributes] FOREIGN KEY([c359Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c359Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c35Type_Attributes] FOREIGN KEY([c35Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c35Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c360Type_Attributes] FOREIGN KEY([c360Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c360Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c361Type_Attributes] FOREIGN KEY([c361Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c361Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c362Type_Attributes] FOREIGN KEY([c362Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c362Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c363Type_Attributes] FOREIGN KEY([c363Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c363Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c364Type_Attributes] FOREIGN KEY([c364Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c364Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c365Type_Attributes] FOREIGN KEY([c365Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c365Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c366Type_Attributes] FOREIGN KEY([c366Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c366Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c367Type_Attributes] FOREIGN KEY([c367Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c367Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c368Type_Attributes] FOREIGN KEY([c368Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c368Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c369Type_Attributes] FOREIGN KEY([c369Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c369Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c36Type_Attributes] FOREIGN KEY([c36Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c36Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c370Type_Attributes] FOREIGN KEY([c370Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c370Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c371Type_Attributes] FOREIGN KEY([c371Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c371Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c372Type_Attributes] FOREIGN KEY([c372Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c372Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c373Type_Attributes] FOREIGN KEY([c373Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c373Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c374Type_Attributes] FOREIGN KEY([c374Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c374Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c375Type_Attributes] FOREIGN KEY([c375Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c375Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c376Type_Attributes] FOREIGN KEY([c376Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c376Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c377Type_Attributes] FOREIGN KEY([c377Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c377Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c378Type_Attributes] FOREIGN KEY([c378Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c378Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c379Type_Attributes] FOREIGN KEY([c379Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c379Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c37Type_Attributes] FOREIGN KEY([c37Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c37Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c380Type_Attributes] FOREIGN KEY([c380Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c380Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c381Type_Attributes] FOREIGN KEY([c381Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c381Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c382Type_Attributes] FOREIGN KEY([c382Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c382Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c383Type_Attributes] FOREIGN KEY([c383Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c383Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c384Type_Attributes] FOREIGN KEY([c384Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c384Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c385Type_Attributes] FOREIGN KEY([c385Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c385Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c386Type_Attributes] FOREIGN KEY([c386Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c386Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c387Type_Attributes] FOREIGN KEY([c387Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c387Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c388Type_Attributes] FOREIGN KEY([c388Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c388Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c389Type_Attributes] FOREIGN KEY([c389Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c389Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c38Type_Attributes] FOREIGN KEY([c38Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c38Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c390Type_Attributes] FOREIGN KEY([c390Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c390Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c391Type_Attributes] FOREIGN KEY([c391Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c391Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c392Type_Attributes] FOREIGN KEY([c392Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c392Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c393Type_Attributes] FOREIGN KEY([c393Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c393Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c394Type_Attributes] FOREIGN KEY([c394Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c394Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c395Type_Attributes] FOREIGN KEY([c395Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c395Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c396Type_Attributes] FOREIGN KEY([c396Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c396Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c397Type_Attributes] FOREIGN KEY([c397Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c397Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c398Type_Attributes] FOREIGN KEY([c398Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c398Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c399Type_Attributes] FOREIGN KEY([c399Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c399Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c39Type_Attributes] FOREIGN KEY([c39Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c39Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c3Type_Attributes] FOREIGN KEY([c3Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c3Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c400Type_Attributes] FOREIGN KEY([c400Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c400Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c401Type_Attributes] FOREIGN KEY([c401Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c401Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c402Type_Attributes] FOREIGN KEY([c402Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c402Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c403Type_Attributes] FOREIGN KEY([c403Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c403Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c404Type_Attributes] FOREIGN KEY([c404Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c404Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c405Type_Attributes] FOREIGN KEY([c405Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c405Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c406Type_Attributes] FOREIGN KEY([c406Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c406Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c407Type_Attributes] FOREIGN KEY([c407Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c407Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c408Type_Attributes] FOREIGN KEY([c408Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c408Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c409Type_Attributes] FOREIGN KEY([c409Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c409Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c40Type_Attributes] FOREIGN KEY([c40Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c40Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c410Type_Attributes] FOREIGN KEY([c410Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c410Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c411Type_Attributes] FOREIGN KEY([c411Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c411Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c412Type_Attributes] FOREIGN KEY([c412Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c412Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c413Type_Attributes] FOREIGN KEY([c413Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c413Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c414Type_Attributes] FOREIGN KEY([c414Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c414Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c415Type_Attributes] FOREIGN KEY([c415Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c415Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c416Type_Attributes] FOREIGN KEY([c416Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c416Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c417Type_Attributes] FOREIGN KEY([c417Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c417Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c418Type_Attributes] FOREIGN KEY([c418Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c418Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c419Type_Attributes] FOREIGN KEY([c419Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c419Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c41Type_Attributes] FOREIGN KEY([c41Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c41Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c420Type_Attributes] FOREIGN KEY([c420Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c420Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c421Type_Attributes] FOREIGN KEY([c421Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c421Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c422Type_Attributes] FOREIGN KEY([c422Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c422Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c423Type_Attributes] FOREIGN KEY([c423Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c423Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c424Type_Attributes] FOREIGN KEY([c424Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c424Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c425Type_Attributes] FOREIGN KEY([c425Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c425Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c426Type_Attributes] FOREIGN KEY([c426Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c426Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c427Type_Attributes] FOREIGN KEY([c427Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c427Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c428Type_Attributes] FOREIGN KEY([c428Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c428Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c429Type_Attributes] FOREIGN KEY([c429Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c429Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c42Type_Attributes] FOREIGN KEY([c42Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c42Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c430Type_Attributes] FOREIGN KEY([c430Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c430Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c431Type_Attributes] FOREIGN KEY([c431Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c431Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c432Type_Attributes] FOREIGN KEY([c432Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c432Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c433Type_Attributes] FOREIGN KEY([c433Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c433Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c434Type_Attributes] FOREIGN KEY([c434Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c434Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c435Type_Attributes] FOREIGN KEY([c435Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c435Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c436Type_Attributes] FOREIGN KEY([c436Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c436Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c437Type_Attributes] FOREIGN KEY([c437Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c437Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c438Type_Attributes] FOREIGN KEY([c438Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c438Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c439Type_Attributes] FOREIGN KEY([c439Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c439Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c43Type_Attributes] FOREIGN KEY([c43Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c43Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c440Type_Attributes] FOREIGN KEY([c440Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c440Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c441Type_Attributes] FOREIGN KEY([c441Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c441Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c442Type_Attributes] FOREIGN KEY([c442Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c442Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c443Type_Attributes] FOREIGN KEY([c443Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c443Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c444Type_Attributes] FOREIGN KEY([c444Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c444Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c445Type_Attributes] FOREIGN KEY([c445Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c445Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c446Type_Attributes] FOREIGN KEY([c446Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c446Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c447Type_Attributes] FOREIGN KEY([c447Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c447Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c448Type_Attributes] FOREIGN KEY([c448Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c448Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c449Type_Attributes] FOREIGN KEY([c449Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c449Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c44Type_Attributes] FOREIGN KEY([c44Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c44Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c45Type_Attributes] FOREIGN KEY([c45Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c45Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c46Type_Attributes] FOREIGN KEY([c46Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c46Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c47Type_Attributes] FOREIGN KEY([c47Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c47Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c48Type_Attributes] FOREIGN KEY([c48Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c48Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c49Type_Attributes] FOREIGN KEY([c49Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c49Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c4Type_Attributes] FOREIGN KEY([c4Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c4Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c50Type_Attributes] FOREIGN KEY([c50Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c50Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c51Type_Attributes] FOREIGN KEY([c51Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c51Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c52Type_Attributes] FOREIGN KEY([c52Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c52Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c53Type_Attributes] FOREIGN KEY([c53Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c53Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c54Type_Attributes] FOREIGN KEY([c54Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c54Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c55Type_Attributes] FOREIGN KEY([c55Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c55Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c56Type_Attributes] FOREIGN KEY([c56Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c56Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c57Type_Attributes] FOREIGN KEY([c57Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c57Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c58Type_Attributes] FOREIGN KEY([c58Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c58Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c59Type_Attributes] FOREIGN KEY([c59Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c59Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c5Type_Attributes] FOREIGN KEY([c5Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c5Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c60Type_Attributes] FOREIGN KEY([c60Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c60Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c61Type_Attributes] FOREIGN KEY([c61Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c61Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c62Type_Attributes] FOREIGN KEY([c62Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c62Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c63Type_Attributes] FOREIGN KEY([c63Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c63Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c64Type_Attributes] FOREIGN KEY([c64Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c64Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c65Type_Attributes] FOREIGN KEY([c65Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c65Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c66Type_Attributes] FOREIGN KEY([c66Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c66Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c67Type_Attributes] FOREIGN KEY([c67Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c67Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c68Type_Attributes] FOREIGN KEY([c68Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c68Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c69Type_Attributes] FOREIGN KEY([c69Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c69Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c6Type_Attributes] FOREIGN KEY([c6Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c6Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c70Type_Attributes] FOREIGN KEY([c70Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c70Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c71Type_Attributes] FOREIGN KEY([c71Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c71Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c72Type_Attributes] FOREIGN KEY([c72Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c72Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c73Type_Attributes] FOREIGN KEY([c73Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c73Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c74Type_Attributes] FOREIGN KEY([c74Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c74Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c75Type_Attributes] FOREIGN KEY([c75Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c75Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c76Type_Attributes] FOREIGN KEY([c76Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c76Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c77Type_Attributes] FOREIGN KEY([c77Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c77Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c78Type_Attributes] FOREIGN KEY([c78Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c78Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c79Type_Attributes] FOREIGN KEY([c79Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c79Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c7Type_Attributes] FOREIGN KEY([c7Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c7Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c80Type_Attributes] FOREIGN KEY([c80Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c80Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c81Type_Attributes] FOREIGN KEY([c81Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c81Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c82Type_Attributes] FOREIGN KEY([c82Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c82Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c83Type_Attributes] FOREIGN KEY([c83Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c83Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c84Type_Attributes] FOREIGN KEY([c84Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c84Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c85Type_Attributes] FOREIGN KEY([c85Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c85Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c86Type_Attributes] FOREIGN KEY([c86Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c86Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c87Type_Attributes] FOREIGN KEY([c87Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c87Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c88Type_Attributes] FOREIGN KEY([c88Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c88Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c89Type_Attributes] FOREIGN KEY([c89Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c89Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c8Type_Attributes] FOREIGN KEY([c8Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c8Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c90Type_Attributes] FOREIGN KEY([c90Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c90Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c91Type_Attributes] FOREIGN KEY([c91Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c91Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c92Type_Attributes] FOREIGN KEY([c92Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c92Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c93Type_Attributes] FOREIGN KEY([c93Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c93Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c94Type_Attributes] FOREIGN KEY([c94Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c94Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c95Type_Attributes] FOREIGN KEY([c95Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c95Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c96Type_Attributes] FOREIGN KEY([c96Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c96Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c97Type_Attributes] FOREIGN KEY([c97Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c97Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c98Type_Attributes] FOREIGN KEY([c98Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c98Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c99Type_Attributes] FOREIGN KEY([c99Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c99Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_c9Type_Attributes] FOREIGN KEY([c9Type])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_c9Type_Attributes]
GO
ALTER TABLE [dbo].[InvRecipeInvention]  WITH CHECK ADD  CONSTRAINT [FK_InvRecipeInvention_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[InvRecipeInvention] CHECK CONSTRAINT [FK_InvRecipeInvention_ContainerId_Ents]
GO
ALTER TABLE [dbo].[InvSalvage0]  WITH CHECK ADD  CONSTRAINT [FK_InvSalvage0_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[InvSalvage0] CHECK CONSTRAINT [FK_InvSalvage0_ContainerId_Ents]
GO
ALTER TABLE [dbo].[InvStoredSalvage0]  WITH CHECK ADD  CONSTRAINT [FK_InvStoredSalvage0_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[InvStoredSalvage0] CHECK CONSTRAINT [FK_InvStoredSalvage0_ContainerId_Ents]
GO
ALTER TABLE [dbo].[KeyBinds]  WITH CHECK ADD  CONSTRAINT [FK_KeyBinds_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[KeyBinds] CHECK CONSTRAINT [FK_KeyBinds_ContainerId_Ents]
GO
ALTER TABLE [dbo].[MapDataTokens]  WITH CHECK ADD  CONSTRAINT [FK_MapDataTokens_ContainerId_MapGroups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[MapGroups] ([ContainerId])
GO
ALTER TABLE [dbo].[MapDataTokens] CHECK CONSTRAINT [FK_MapDataTokens_ContainerId_MapGroups]
GO
ALTER TABLE [dbo].[MapHistory]  WITH CHECK ADD  CONSTRAINT [FK_MapHistory_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[MapHistory] CHECK CONSTRAINT [FK_MapHistory_ContainerId_Ents]
GO
ALTER TABLE [dbo].[MARTYTracks]  WITH CHECK ADD  CONSTRAINT [FK_MARTYTracks_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[MARTYTracks] CHECK CONSTRAINT [FK_MARTYTracks_ContainerId_Ents]
GO
ALTER TABLE [dbo].[MinedData]  WITH CHECK ADD  CONSTRAINT [FK_MinedData_ContainerId_MiningAccumulator] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[MiningAccumulator] ([ContainerId])
GO
ALTER TABLE [dbo].[MinedData] CHECK CONSTRAINT [FK_MinedData_ContainerId_MiningAccumulator]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_ContainerId_Ents]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem0_Attributes] FOREIGN KEY([NTHItem0])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem0_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem1_Attributes] FOREIGN KEY([NTHItem1])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem1_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem10_Attributes] FOREIGN KEY([NTHItem10])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem10_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem11_Attributes] FOREIGN KEY([NTHItem11])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem11_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem12_Attributes] FOREIGN KEY([NTHItem12])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem12_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem13_Attributes] FOREIGN KEY([NTHItem13])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem13_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem14_Attributes] FOREIGN KEY([NTHItem14])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem14_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem15_Attributes] FOREIGN KEY([NTHItem15])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem15_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem16_Attributes] FOREIGN KEY([NTHItem16])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem16_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem17_Attributes] FOREIGN KEY([NTHItem17])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem17_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem18_Attributes] FOREIGN KEY([NTHItem18])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem18_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem19_Attributes] FOREIGN KEY([NTHItem19])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem19_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem2_Attributes] FOREIGN KEY([NTHItem2])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem2_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem3_Attributes] FOREIGN KEY([NTHItem3])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem3_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem4_Attributes] FOREIGN KEY([NTHItem4])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem4_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem5_Attributes] FOREIGN KEY([NTHItem5])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem5_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem6_Attributes] FOREIGN KEY([NTHItem6])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem6_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem7_Attributes] FOREIGN KEY([NTHItem7])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem7_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem8_Attributes] FOREIGN KEY([NTHItem8])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem8_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHItem9_Attributes] FOREIGN KEY([NTHItem9])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHItem9_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson0_Attributes] FOREIGN KEY([NTHPerson0])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson0_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson1_Attributes] FOREIGN KEY([NTHPerson1])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson1_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson10_Attributes] FOREIGN KEY([NTHPerson10])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson10_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson11_Attributes] FOREIGN KEY([NTHPerson11])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson11_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson12_Attributes] FOREIGN KEY([NTHPerson12])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson12_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson13_Attributes] FOREIGN KEY([NTHPerson13])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson13_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson14_Attributes] FOREIGN KEY([NTHPerson14])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson14_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson15_Attributes] FOREIGN KEY([NTHPerson15])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson15_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson16_Attributes] FOREIGN KEY([NTHPerson16])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson16_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson17_Attributes] FOREIGN KEY([NTHPerson17])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson17_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson18_Attributes] FOREIGN KEY([NTHPerson18])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson18_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson19_Attributes] FOREIGN KEY([NTHPerson19])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson19_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson2_Attributes] FOREIGN KEY([NTHPerson2])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson2_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson3_Attributes] FOREIGN KEY([NTHPerson3])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson3_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson4_Attributes] FOREIGN KEY([NTHPerson4])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson4_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson5_Attributes] FOREIGN KEY([NTHPerson5])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson5_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson6_Attributes] FOREIGN KEY([NTHPerson6])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson6_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson7_Attributes] FOREIGN KEY([NTHPerson7])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson7_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson8_Attributes] FOREIGN KEY([NTHPerson8])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson8_Attributes]
GO
ALTER TABLE [dbo].[NewspaperHistory]  WITH CHECK ADD  CONSTRAINT [FK_NewspaperHistory_NTHPerson9_Attributes] FOREIGN KEY([NTHPerson9])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[NewspaperHistory] CHECK CONSTRAINT [FK_NewspaperHistory_NTHPerson9_Attributes]
GO
ALTER TABLE [dbo].[Participants]  WITH CHECK ADD  CONSTRAINT [FK_Participants_Archetype_Attributes] FOREIGN KEY([Archetype])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Participants] CHECK CONSTRAINT [FK_Participants_Archetype_Attributes]
GO
ALTER TABLE [dbo].[Participants]  WITH CHECK ADD  CONSTRAINT [FK_Participants_ContainerId_ArenaEvents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[ArenaEvents] ([ContainerId])
GO
ALTER TABLE [dbo].[Participants] CHECK CONSTRAINT [FK_Participants_ContainerId_ArenaEvents]
GO
ALTER TABLE [dbo].[PendingOrders]  WITH CHECK ADD  CONSTRAINT [FK_PendingOrders_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[PendingOrders] CHECK CONSTRAINT [FK_PendingOrders_ContainerId_Ents]
GO
ALTER TABLE [dbo].[PetNames]  WITH CHECK ADD  CONSTRAINT [FK_PetNames_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[PetNames] CHECK CONSTRAINT [FK_PetNames_ContainerId_Ents]
GO
ALTER TABLE [dbo].[PowerCustomizations]  WITH CHECK ADD  CONSTRAINT [FK_PowerCustomizations_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[PowerCustomizations] CHECK CONSTRAINT [FK_PowerCustomizations_ContainerId_Ents]
GO
ALTER TABLE [dbo].[PowerCustomizations]  WITH CHECK ADD  CONSTRAINT [FK_PowerCustomizations_PowerCatName_Attributes] FOREIGN KEY([PowerCatName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[PowerCustomizations] CHECK CONSTRAINT [FK_PowerCustomizations_PowerCatName_Attributes]
GO
ALTER TABLE [dbo].[PowerCustomizations]  WITH CHECK ADD  CONSTRAINT [FK_PowerCustomizations_PowerName_Attributes] FOREIGN KEY([PowerName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[PowerCustomizations] CHECK CONSTRAINT [FK_PowerCustomizations_PowerName_Attributes]
GO
ALTER TABLE [dbo].[PowerCustomizations]  WITH CHECK ADD  CONSTRAINT [FK_PowerCustomizations_PowerSetName_Attributes] FOREIGN KEY([PowerSetName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[PowerCustomizations] CHECK CONSTRAINT [FK_PowerCustomizations_PowerSetName_Attributes]
GO
ALTER TABLE [dbo].[PowerCustomizations]  WITH CHECK ADD  CONSTRAINT [FK_PowerCustomizations_Token_Attributes] FOREIGN KEY([Token])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[PowerCustomizations] CHECK CONSTRAINT [FK_PowerCustomizations_Token_Attributes]
GO
ALTER TABLE [dbo].[Powers]  WITH CHECK ADD  CONSTRAINT [FK_Powers_CategoryName_Attributes] FOREIGN KEY([CategoryName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Powers] CHECK CONSTRAINT [FK_Powers_CategoryName_Attributes]
GO
ALTER TABLE [dbo].[Powers]  WITH CHECK ADD  CONSTRAINT [FK_Powers_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Powers] CHECK CONSTRAINT [FK_Powers_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Powers]  WITH CHECK ADD  CONSTRAINT [FK_Powers_PowerName_Attributes] FOREIGN KEY([PowerName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Powers] CHECK CONSTRAINT [FK_Powers_PowerName_Attributes]
GO
ALTER TABLE [dbo].[Powers]  WITH CHECK ADD  CONSTRAINT [FK_Powers_PowerSetName_Attributes] FOREIGN KEY([PowerSetName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Powers] CHECK CONSTRAINT [FK_Powers_PowerSetName_Attributes]
GO
ALTER TABLE [dbo].[QueuedRewardTables]  WITH CHECK ADD  CONSTRAINT [FK_QueuedRewardTables_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[QueuedRewardTables] CHECK CONSTRAINT [FK_QueuedRewardTables_ContainerId_Ents]
GO
ALTER TABLE [dbo].[RecentBadge]  WITH CHECK ADD  CONSTRAINT [FK_RecentBadge_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[RecentBadge] CHECK CONSTRAINT [FK_RecentBadge_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Recipes]  WITH CHECK ADD  CONSTRAINT [FK_Recipes_ContainerId_Supergroups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[Recipes] CHECK CONSTRAINT [FK_Recipes_ContainerId_Supergroups]
GO
ALTER TABLE [dbo].[Recipes]  WITH CHECK ADD  CONSTRAINT [FK_Recipes_Recipe_Attributes] FOREIGN KEY([Recipe])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Recipes] CHECK CONSTRAINT [FK_Recipes_Recipe_Attributes]
GO
ALTER TABLE [dbo].[Recipients]  WITH CHECK ADD  CONSTRAINT [FK_Recipients_ContainerId_Email] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Email] ([ContainerId])
GO
ALTER TABLE [dbo].[Recipients] CHECK CONSTRAINT [FK_Recipients_ContainerId_Email]
GO
ALTER TABLE [dbo].[RewardTokens]  WITH CHECK ADD  CONSTRAINT [FK_RewardTokens_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[RewardTokens] CHECK CONSTRAINT [FK_RewardTokens_ContainerId_Ents]
GO
ALTER TABLE [dbo].[RewardTokens]  WITH CHECK ADD  CONSTRAINT [FK_RewardTokens_PieceName_Attributes] FOREIGN KEY([PieceName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[RewardTokens] CHECK CONSTRAINT [FK_RewardTokens_PieceName_Attributes]
GO
ALTER TABLE [dbo].[RewardTokensActive]  WITH CHECK ADD  CONSTRAINT [FK_RewardTokensActive_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[RewardTokensActive] CHECK CONSTRAINT [FK_RewardTokensActive_ContainerId_Ents]
GO
ALTER TABLE [dbo].[RewardTokensActive]  WITH CHECK ADD  CONSTRAINT [FK_RewardTokensActive_PieceName_Attributes] FOREIGN KEY([PieceName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[RewardTokensActive] CHECK CONSTRAINT [FK_RewardTokensActive_PieceName_Attributes]
GO
ALTER TABLE [dbo].[Seating]  WITH CHECK ADD  CONSTRAINT [FK_Seating_ContainerId_ArenaEvents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[ArenaEvents] ([ContainerId])
GO
ALTER TABLE [dbo].[Seating] CHECK CONSTRAINT [FK_Seating_ContainerId_ArenaEvents]
GO
ALTER TABLE [dbo].[SgrpBadgeStats]  WITH CHECK ADD  CONSTRAINT [FK_SgrpBadgeStats_ContainerId_statserver_SupergroupStats] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[statserver_SupergroupStats] ([ContainerId])
GO
ALTER TABLE [dbo].[SgrpBadgeStats] CHECK CONSTRAINT [FK_SgrpBadgeStats_ContainerId_statserver_SupergroupStats]
GO
ALTER TABLE [dbo].[SgrpCustomRanks]  WITH CHECK ADD  CONSTRAINT [FK_SgrpCustomRanks_ContainerId_Supergroups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[SgrpCustomRanks] CHECK CONSTRAINT [FK_SgrpCustomRanks_ContainerId_Supergroups]
GO
ALTER TABLE [dbo].[SgrpMembers]  WITH CHECK ADD  CONSTRAINT [FK_SgrpMembers_ContainerId_Supergroups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[SgrpMembers] CHECK CONSTRAINT [FK_SgrpMembers_ContainerId_Supergroups]
GO
ALTER TABLE [dbo].[SgrpPraetBonusIDs]  WITH CHECK ADD  CONSTRAINT [FK_SgrpPraetBonusIDs_ContainerId_Supergroups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[SgrpPraetBonusIDs] CHECK CONSTRAINT [FK_SgrpPraetBonusIDs_ContainerId_Supergroups]
GO
ALTER TABLE [dbo].[SgrpRewardTokens]  WITH CHECK ADD  CONSTRAINT [FK_SgrpRewardTokens_ContainerId_Supergroups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[SgrpRewardTokens] CHECK CONSTRAINT [FK_SgrpRewardTokens_ContainerId_Supergroups]
GO
ALTER TABLE [dbo].[SgrpRewardTokens]  WITH CHECK ADD  CONSTRAINT [FK_SgrpRewardTokens_PieceName_Attributes] FOREIGN KEY([PieceName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SgrpRewardTokens] CHECK CONSTRAINT [FK_SgrpRewardTokens_PieceName_Attributes]
GO
ALTER TABLE [dbo].[SGTask]  WITH CHECK ADD  CONSTRAINT [FK_SGTask_ContainerId_Supergroups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[SGTask] CHECK CONSTRAINT [FK_SGTask_ContainerId_Supergroups]
GO
ALTER TABLE [dbo].[SGTask]  WITH CHECK ADD  CONSTRAINT [FK_SGTask_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SGTask] CHECK CONSTRAINT [FK_SGTask_ID_Attributes]
GO
ALTER TABLE [dbo].[SouvenirClues]  WITH CHECK ADD  CONSTRAINT [FK_SouvenirClues_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[SouvenirClues] CHECK CONSTRAINT [FK_SouvenirClues_ContainerId_Ents]
GO
ALTER TABLE [dbo].[SouvenirClues]  WITH CHECK ADD  CONSTRAINT [FK_SouvenirClues_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SouvenirClues] CHECK CONSTRAINT [FK_SouvenirClues_ID_Attributes]
GO
ALTER TABLE [dbo].[SpecialDetails]  WITH CHECK ADD  CONSTRAINT [FK_SpecialDetails_ContainerId_Supergroups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[SpecialDetails] CHECK CONSTRAINT [FK_SpecialDetails_ContainerId_Supergroups]
GO
ALTER TABLE [dbo].[SpecialDetails]  WITH CHECK ADD  CONSTRAINT [FK_SpecialDetails_Detail_Attributes] FOREIGN KEY([Detail])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SpecialDetails] CHECK CONSTRAINT [FK_SpecialDetails_Detail_Attributes]
GO
ALTER TABLE [dbo].[Stats]  WITH CHECK ADD  CONSTRAINT [FK_Stats_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Stats] CHECK CONSTRAINT [FK_Stats_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Stats]  WITH CHECK ADD  CONSTRAINT [FK_Stats_Name_Attributes] FOREIGN KEY([Name])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Stats] CHECK CONSTRAINT [FK_Stats_Name_Attributes]
GO
ALTER TABLE [dbo].[StoryArcs]  WITH CHECK ADD  CONSTRAINT [FK_StoryArcs_Contact_Attributes] FOREIGN KEY([Contact])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[StoryArcs] CHECK CONSTRAINT [FK_StoryArcs_Contact_Attributes]
GO
ALTER TABLE [dbo].[StoryArcs]  WITH CHECK ADD  CONSTRAINT [FK_StoryArcs_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[StoryArcs] CHECK CONSTRAINT [FK_StoryArcs_ContainerId_Ents]
GO
ALTER TABLE [dbo].[StoryArcs]  WITH CHECK ADD  CONSTRAINT [FK_StoryArcs_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[StoryArcs] CHECK CONSTRAINT [FK_StoryArcs_ID_Attributes]
GO
ALTER TABLE [dbo].[SuperCostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_SuperCostumeParts_BodySet_Attributes] FOREIGN KEY([BodySet])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SuperCostumeParts] CHECK CONSTRAINT [FK_SuperCostumeParts_BodySet_Attributes]
GO
ALTER TABLE [dbo].[SuperCostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_SuperCostumeParts_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[SuperCostumeParts] CHECK CONSTRAINT [FK_SuperCostumeParts_ContainerId_Ents]
GO
ALTER TABLE [dbo].[SuperCostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_SuperCostumeParts_DisplayName_Attributes] FOREIGN KEY([DisplayName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SuperCostumeParts] CHECK CONSTRAINT [FK_SuperCostumeParts_DisplayName_Attributes]
GO
ALTER TABLE [dbo].[SuperCostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_SuperCostumeParts_FxName_Attributes] FOREIGN KEY([FxName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SuperCostumeParts] CHECK CONSTRAINT [FK_SuperCostumeParts_FxName_Attributes]
GO
ALTER TABLE [dbo].[SuperCostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_SuperCostumeParts_Geom_Attributes] FOREIGN KEY([Geom])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SuperCostumeParts] CHECK CONSTRAINT [FK_SuperCostumeParts_Geom_Attributes]
GO
ALTER TABLE [dbo].[SuperCostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_SuperCostumeParts_Name_Attributes] FOREIGN KEY([Name])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SuperCostumeParts] CHECK CONSTRAINT [FK_SuperCostumeParts_Name_Attributes]
GO
ALTER TABLE [dbo].[SuperCostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_SuperCostumeParts_Region_Attributes] FOREIGN KEY([Region])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SuperCostumeParts] CHECK CONSTRAINT [FK_SuperCostumeParts_Region_Attributes]
GO
ALTER TABLE [dbo].[SuperCostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_SuperCostumeParts_Tex1_Attributes] FOREIGN KEY([Tex1])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SuperCostumeParts] CHECK CONSTRAINT [FK_SuperCostumeParts_Tex1_Attributes]
GO
ALTER TABLE [dbo].[SuperCostumeParts]  WITH CHECK ADD  CONSTRAINT [FK_SuperCostumeParts_Tex2_Attributes] FOREIGN KEY([Tex2])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[SuperCostumeParts] CHECK CONSTRAINT [FK_SuperCostumeParts_Tex2_Attributes]
GO
ALTER TABLE [dbo].[SuperGroupAllies]  WITH CHECK ADD  CONSTRAINT [FK_SuperGroupAllies_ContainerId_Supergroups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Supergroups] ([ContainerId])
GO
ALTER TABLE [dbo].[SuperGroupAllies] CHECK CONSTRAINT [FK_SuperGroupAllies_ContainerId_Supergroups]
GO
ALTER TABLE [dbo].[TaskForceContacts]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceContacts_ContainerId_Taskforces] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Taskforces] ([ContainerId])
GO
ALTER TABLE [dbo].[TaskForceContacts] CHECK CONSTRAINT [FK_TaskForceContacts_ContainerId_Taskforces]
GO
ALTER TABLE [dbo].[TaskForceContacts]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceContacts_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[TaskForceContacts] CHECK CONSTRAINT [FK_TaskForceContacts_ID_Attributes]
GO
ALTER TABLE [dbo].[TaskForceParameters]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceParameters_ContainerId_Taskforces] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Taskforces] ([ContainerId])
GO
ALTER TABLE [dbo].[TaskForceParameters] CHECK CONSTRAINT [FK_TaskForceParameters_ContainerId_Taskforces]
GO
ALTER TABLE [dbo].[TaskForceSouvenirClues]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceSouvenirClues_ContainerId_Taskforces] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Taskforces] ([ContainerId])
GO
ALTER TABLE [dbo].[TaskForceSouvenirClues] CHECK CONSTRAINT [FK_TaskForceSouvenirClues_ContainerId_Taskforces]
GO
ALTER TABLE [dbo].[TaskForceSouvenirClues]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceSouvenirClues_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[TaskForceSouvenirClues] CHECK CONSTRAINT [FK_TaskForceSouvenirClues_ID_Attributes]
GO
ALTER TABLE [dbo].[TaskForceStoryArcs]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceStoryArcs_Contact_Attributes] FOREIGN KEY([Contact])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[TaskForceStoryArcs] CHECK CONSTRAINT [FK_TaskForceStoryArcs_Contact_Attributes]
GO
ALTER TABLE [dbo].[TaskForceStoryArcs]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceStoryArcs_ContainerId_Taskforces] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Taskforces] ([ContainerId])
GO
ALTER TABLE [dbo].[TaskForceStoryArcs] CHECK CONSTRAINT [FK_TaskForceStoryArcs_ContainerId_Taskforces]
GO
ALTER TABLE [dbo].[TaskForceStoryArcs]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceStoryArcs_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[TaskForceStoryArcs] CHECK CONSTRAINT [FK_TaskForceStoryArcs_ID_Attributes]
GO
ALTER TABLE [dbo].[TaskForceTasks]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceTasks_ContainerId_Taskforces] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Taskforces] ([ContainerId])
GO
ALTER TABLE [dbo].[TaskForceTasks] CHECK CONSTRAINT [FK_TaskForceTasks_ContainerId_Taskforces]
GO
ALTER TABLE [dbo].[TaskForceTasks]  WITH CHECK ADD  CONSTRAINT [FK_TaskForceTasks_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[TaskForceTasks] CHECK CONSTRAINT [FK_TaskForceTasks_ID_Attributes]
GO
ALTER TABLE [dbo].[Tasks]  WITH CHECK ADD  CONSTRAINT [FK_Tasks_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Tasks] CHECK CONSTRAINT [FK_Tasks_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Tasks]  WITH CHECK ADD  CONSTRAINT [FK_Tasks_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Tasks] CHECK CONSTRAINT [FK_Tasks_ID_Attributes]
GO
ALTER TABLE [dbo].[TeamLeaderIds]  WITH CHECK ADD  CONSTRAINT [FK_TeamLeaderIds_ContainerId_Leagues] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Leagues] ([ContainerId])
GO
ALTER TABLE [dbo].[TeamLeaderIds] CHECK CONSTRAINT [FK_TeamLeaderIds_ContainerId_Leagues]
GO
ALTER TABLE [dbo].[TeamLockStatus]  WITH CHECK ADD  CONSTRAINT [FK_TeamLockStatus_ContainerId_Leagues] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Leagues] ([ContainerId])
GO
ALTER TABLE [dbo].[TeamLockStatus] CHECK CONSTRAINT [FK_TeamLockStatus_ContainerId_Leagues]
GO
ALTER TABLE [dbo].[TeamupRewardTokensActive]  WITH CHECK ADD  CONSTRAINT [FK_TeamupRewardTokensActive_ContainerId_Teamups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Teamups] ([ContainerId])
GO
ALTER TABLE [dbo].[TeamupRewardTokensActive] CHECK CONSTRAINT [FK_TeamupRewardTokensActive_ContainerId_Teamups]
GO
ALTER TABLE [dbo].[TeamupRewardTokensActive]  WITH CHECK ADD  CONSTRAINT [FK_TeamupRewardTokensActive_PieceName_Attributes] FOREIGN KEY([PieceName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[TeamupRewardTokensActive] CHECK CONSTRAINT [FK_TeamupRewardTokensActive_PieceName_Attributes]
GO
ALTER TABLE [dbo].[Teamups]  WITH CHECK ADD  CONSTRAINT [FK_Teamups_Contact_Attributes] FOREIGN KEY([Contact])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Teamups] CHECK CONSTRAINT [FK_Teamups_Contact_Attributes]
GO
ALTER TABLE [dbo].[TeamupTask]  WITH CHECK ADD  CONSTRAINT [FK_TeamupTask_ContainerId_Teamups] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Teamups] ([ContainerId])
GO
ALTER TABLE [dbo].[TeamupTask] CHECK CONSTRAINT [FK_TeamupTask_ContainerId_Teamups]
GO
ALTER TABLE [dbo].[TeamupTask]  WITH CHECK ADD  CONSTRAINT [FK_TeamupTask_ID_Attributes] FOREIGN KEY([ID])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[TeamupTask] CHECK CONSTRAINT [FK_TeamupTask_ID_Attributes]
GO
ALTER TABLE [dbo].[TestDataBaseTypes]  WITH CHECK ADD  CONSTRAINT [FK_TestDataBaseTypes_test_attr_Attributes] FOREIGN KEY([test_attr])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[TestDataBaseTypes] CHECK CONSTRAINT [FK_TestDataBaseTypes_test_attr_Attributes]
GO
ALTER TABLE [dbo].[Tray]  WITH CHECK ADD  CONSTRAINT [FK_Tray_CategoryName_Attributes] FOREIGN KEY([CategoryName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Tray] CHECK CONSTRAINT [FK_Tray_CategoryName_Attributes]
GO
ALTER TABLE [dbo].[Tray]  WITH CHECK ADD  CONSTRAINT [FK_Tray_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Tray] CHECK CONSTRAINT [FK_Tray_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Tray]  WITH CHECK ADD  CONSTRAINT [FK_Tray_PowerName_Attributes] FOREIGN KEY([PowerName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Tray] CHECK CONSTRAINT [FK_Tray_PowerName_Attributes]
GO
ALTER TABLE [dbo].[Tray]  WITH CHECK ADD  CONSTRAINT [FK_Tray_PowerSetName_Attributes] FOREIGN KEY([PowerSetName])
REFERENCES [dbo].[Attributes] ([Id])
GO
ALTER TABLE [dbo].[Tray] CHECK CONSTRAINT [FK_Tray_PowerSetName_Attributes]
GO
ALTER TABLE [dbo].[VisitedMaps]  WITH CHECK ADD  CONSTRAINT [FK_VisitedMaps_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[VisitedMaps] CHECK CONSTRAINT [FK_VisitedMaps_ContainerId_Ents]
GO
ALTER TABLE [dbo].[Windows]  WITH CHECK ADD  CONSTRAINT [FK_Windows_ContainerId_Ents] FOREIGN KEY([ContainerId])
REFERENCES [dbo].[Ents] ([ContainerId])
GO
ALTER TABLE [dbo].[Windows] CHECK CONSTRAINT [FK_Windows_ContainerId_Ents]
GO
USE [master]
GO
ALTER DATABASE [cohdb] SET  READ_WRITE 
GO
