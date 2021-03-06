if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_CreatureSpell_Creature]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[CreatureSpell] DROP CONSTRAINT FK_CreatureSpell_Creature
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_SpawnerCreature_Creature]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[SpawnerCreature] DROP CONSTRAINT FK_SpawnerCreature_Creature
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_LootPackageItem_Item]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[LootPackageItem] DROP CONSTRAINT FK_LootPackageItem_Item
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_SpawnerCreature_Item]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[SpawnerCreature] DROP CONSTRAINT FK_SpawnerCreature_Item
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_Weapon_Item]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[Weapon] DROP CONSTRAINT FK_Weapon_Item
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_LootPackageItem_LootPackage]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[LootPackageItem] DROP CONSTRAINT FK_LootPackageItem_LootPackage
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_SpawnerCreature_LootPackage]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[SpawnerCreature] DROP CONSTRAINT FK_SpawnerCreature_LootPackage
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_SpawnerCreature_Spawner]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[SpawnerCreature] DROP CONSTRAINT FK_SpawnerCreature_Spawner
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_ZoneSpawner_Spawner]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[ZoneSpawner] DROP CONSTRAINT FK_ZoneSpawner_Spawner
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_CreatureSpell_Spell]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[CreatureSpell] DROP CONSTRAINT FK_CreatureSpell_Spell
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[FK_ZoneSpawner_Zone]') and OBJECTPROPERTY(id, N'IsForeignKey') = 1)
ALTER TABLE [dbo].[ZoneSpawner] DROP CONSTRAINT FK_ZoneSpawner_Zone
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[sp_CheckForBadSpawners]') and OBJECTPROPERTY(id, N'IsProcedure') = 1)
drop procedure [dbo].[sp_CheckForBadSpawners]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[CreatureSpell]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[CreatureSpell]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[LootPackageItem]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[LootPackageItem]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[SpawnerCreature]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[SpawnerCreature]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Weapon]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[Weapon]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[ZoneSpawner]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[ZoneSpawner]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Creature]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[Creature]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Item]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[Item]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[LootPackage]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[LootPackage]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Spawner]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[Spawner]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Spell]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[Spell]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Zone]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[Zone]
GO

CREATE TABLE [dbo].[Creature] (
	[CreatureId] [int] NOT NULL ,
	[CreatureDesc] [varchar] (50) NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[Item] (
	[ItemId] [int] NOT NULL ,
	[ItemDesc] [varchar] (50) NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[LootPackage] (
	[LootPackageId] [int] NOT NULL ,
	[LootPackageDesc] [varchar] (50) NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[Spawner] (
	[SpawnerId] [int] NOT NULL ,
	[SpawnerDesc] [varchar] (50) NOT NULL ,
	[SpawnRadius] [real] NOT NULL ,
	[MinPopulationCount] [smallint] NOT NULL ,
	[ActiveFlag] [bit] NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[Spell] (
	[SpellId] [int] NOT NULL ,
	[SpellDesc] [varchar] (50) NOT NULL ,
	[MinDamageAmount] [smallint] NOT NULL ,
	[MaxDamageAmount] [smallint] NOT NULL ,
	[MaxEffectiveDistance] [real] NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[Zone] (
	[ZoneId] [int] NOT NULL ,
	[ZoneDesc] [varchar] (50) NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[CreatureSpell] (
	[CreatureId] [int] NOT NULL ,
	[SpellId] [int] NOT NULL ,
	[DamageMultiplier] [real] NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[LootPackageItem] (
	[LootPackageId] [int] NOT NULL ,
	[ItemId] [int] NOT NULL ,
	[MinItemAmount] [smallint] NOT NULL ,
	[MaxItemAmount] [smallint] NOT NULL ,
	[DropProbability] [smallint] NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[SpawnerCreature] (
	[SpawnerId] [int] NOT NULL ,
	[CreatureId] [int] NOT NULL ,
	[MinSpawnCount] [smallint] NOT NULL ,
	[MaxSpawnCount] [smallint] NOT NULL ,
	[GroupedFlag] [bit] NOT NULL ,
	[MinScaleFactor] [real] NOT NULL ,
	[MaxScaleFactor] [real] NOT NULL ,
	[MinHitPoints] [smallint] NOT NULL ,
	[MaxHitPoints] [smallint] NOT NULL ,
	[MinManaPoints] [smallint] NOT NULL ,
	[MaxManaPoints] [smallint] NOT NULL ,
	[LootPackageId] [int] NOT NULL ,
	[EquippedItemId] [int] NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[Weapon] (
	[ItemId] [int] NOT NULL ,
	[DamageRange] [real] NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[ZoneSpawner] (
	[ZoneId] [int] NOT NULL ,
	[SpawnerId] [int] NOT NULL ,
	[LocationX] [real] NOT NULL ,
	[LocationY] [real] NOT NULL ,
	[LocationZ] [real] NOT NULL 
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[Creature] WITH NOCHECK ADD 
	CONSTRAINT [PK_Creature] PRIMARY KEY  CLUSTERED 
	(
		[CreatureId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[Item] WITH NOCHECK ADD 
	CONSTRAINT [PK_Item] PRIMARY KEY  CLUSTERED 
	(
		[ItemId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[LootPackage] WITH NOCHECK ADD 
	CONSTRAINT [PK_LootPackage] PRIMARY KEY  CLUSTERED 
	(
		[LootPackageId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[Spawner] WITH NOCHECK ADD 
	CONSTRAINT [PK_Spawner] PRIMARY KEY  CLUSTERED 
	(
		[SpawnerId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[Spell] WITH NOCHECK ADD 
	CONSTRAINT [PK_Spell] PRIMARY KEY  CLUSTERED 
	(
		[SpellId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[Zone] WITH NOCHECK ADD 
	CONSTRAINT [PK_Zone] PRIMARY KEY  CLUSTERED 
	(
		[ZoneId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[CreatureSpell] WITH NOCHECK ADD 
	CONSTRAINT [PK_CreatureSpell] PRIMARY KEY  CLUSTERED 
	(
		[CreatureId],
		[SpellId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[LootPackageItem] WITH NOCHECK ADD 
	CONSTRAINT [PK_LootPackageItem] PRIMARY KEY  CLUSTERED 
	(
		[LootPackageId],
		[ItemId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[SpawnerCreature] WITH NOCHECK ADD 
	CONSTRAINT [PK_SpawnerCreature] PRIMARY KEY  CLUSTERED 
	(
		[SpawnerId],
		[CreatureId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[Weapon] WITH NOCHECK ADD 
	CONSTRAINT [PK_Weapon] PRIMARY KEY  CLUSTERED 
	(
		[ItemId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[ZoneSpawner] WITH NOCHECK ADD 
	CONSTRAINT [PK_ZoneSpawner] PRIMARY KEY  CLUSTERED 
	(
		[ZoneId],
		[SpawnerId]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[CreatureSpell] ADD 
	CONSTRAINT [FK_CreatureSpell_Creature] FOREIGN KEY 
	(
		[CreatureId]
	) REFERENCES [dbo].[Creature] (
		[CreatureId]
	),
	CONSTRAINT [FK_CreatureSpell_Spell] FOREIGN KEY 
	(
		[SpellId]
	) REFERENCES [dbo].[Spell] (
		[SpellId]
	)
GO

ALTER TABLE [dbo].[LootPackageItem] ADD 
	CONSTRAINT [FK_LootPackageItem_Item] FOREIGN KEY 
	(
		[ItemId]
	) REFERENCES [dbo].[Item] (
		[ItemId]
	),
	CONSTRAINT [FK_LootPackageItem_LootPackage] FOREIGN KEY 
	(
		[LootPackageId]
	) REFERENCES [dbo].[LootPackage] (
		[LootPackageId]
	)
GO

ALTER TABLE [dbo].[SpawnerCreature] ADD 
	CONSTRAINT [FK_SpawnerCreature_Creature] FOREIGN KEY 
	(
		[CreatureId]
	) REFERENCES [dbo].[Creature] (
		[CreatureId]
	),
	CONSTRAINT [FK_SpawnerCreature_Item] FOREIGN KEY 
	(
		[EquippedItemId]
	) REFERENCES [dbo].[Item] (
		[ItemId]
	),
	CONSTRAINT [FK_SpawnerCreature_LootPackage] FOREIGN KEY 
	(
		[LootPackageId]
	) REFERENCES [dbo].[LootPackage] (
		[LootPackageId]
	),
	CONSTRAINT [FK_SpawnerCreature_Spawner] FOREIGN KEY 
	(
		[SpawnerId]
	) REFERENCES [dbo].[Spawner] (
		[SpawnerId]
	)
GO

ALTER TABLE [dbo].[Weapon] ADD 
	CONSTRAINT [FK_Weapon_Item] FOREIGN KEY 
	(
		[ItemId]
	) REFERENCES [dbo].[Item] (
		[ItemId]
	)
GO

ALTER TABLE [dbo].[ZoneSpawner] ADD 
	CONSTRAINT [FK_ZoneSpawner_Spawner] FOREIGN KEY 
	(
		[SpawnerId]
	) REFERENCES [dbo].[Spawner] (
		[SpawnerId]
	),
	CONSTRAINT [FK_ZoneSpawner_Zone] FOREIGN KEY 
	(
		[ZoneId]
	) REFERENCES [dbo].[Zone] (
		[ZoneId]
	)
GO

SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS OFF 
GO

CREATE PROCEDURE sp_CheckForBadSpawners
AS
  SELECT SpawnerDesc, SpawnRadius, LocationX, LocationY, LocationZ
  FROM Spawner A, ZoneSpawner B
  WHERE A.SpawnerId = B.SpawnerId 
  AND ZoneId = 5
  AND SpawnRadius > 30
GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

