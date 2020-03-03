// This file contains per-map info.  Broadly, this is stuff designers
// care about, like level ranges, as opposed to maps.db that contains
// stuff that programmers care about.
//
// This contains the specifications on what mission doors, contacts,
// touch locations, etc. should exist in each map.  Both maps and tasks
// are checked against this info.

// Outbreak
Map City_00_01.txt
{
	MissionMinLevel 1
	MissionMaxLevel 10
	SpawnOverrideMinLevel 1
	SpawnOverrideMaxLevel 1

	MissionDoors TutorialMission
	PersistentNPCs Officer_Parks, Detective_Wright, Sergeant_Hicks, Doctor_Miller, Lt_MacReady, Paragon_SWAT, Officer_Flint, Professor_Hoffman
	VisitLocations TutorialConningPlaque4, TutorialConningPlaque2, TutorialConningPlaque1, TutorialPlaqueX, 
	EncounterLayouts Single, CenteredAround, Duo, Encounter, 
	VillainGroups Contaminated, 
}

// Atlas Park
Map City_01_01.txt
{
	// appropriate level ranges for this zone
	MissionMinLevel 1
	MissionMaxLevel 9
	SpawnOverrideMinLevel 1
	SpawnOverrideMaxLevel 9

	
	MissionDoors Office, AbandonedOffice, Warehouse, AbandonedWarehouse, Sewers, Caves, Tech
	PersistentNPCs FreedomCorp_City_01_01, Tony_Kord, Laurence_Mansfield, Jose_Brogan, FreedomCorp_City_01_01b, Susan_Davies, Security_Chief_01, MsLiberty, Antonio_Nash, FreedomCorp_City_01_03, Iris_Parker, SuperGroupRegistrar, City_Representative, Sarah_Juarez, Henry_Peter_Wong, Azuria, Rick_Davies, Sarah_Peters, Security_Chief_04, Jonathan_Smythe, FreedomCorp_City_01_01c, Charlie, NotorietyContact6
	VisitLocations Statue1, Patrol_Hard_5_City_01_01, Patrol_Hard_4_City_01_01, Patrol_Hard_3_City_01_01, Patrol_Hard_2_City_01_01, Patrol_Hard_1_City_01_01, Patrol_Easy_5_City_01_01, Patrol_Easy_4_City_01_01, Patrol_Easy_3_City_01_01, Patrol_Easy_2_City_01_01, Patrol_Easy_1_City_01_01,  
	//Plaques CitizenCrimeAct2, AxisAttack1, CitizenCrimeAct1, MightForRightAct1, PsychicFeedbackLoop1,
	EncounterLayouts AroundDoor, Single, Snatch, Ambush, AroundVandalism, Around, 
	VillainGroups Clockwork, Hellions, 
}

// Kings Row
Map City_01_02.txt
{
	MissionMinLevel 5
	SpawnOverrideMinLevel 5

	MissionDoors Office, AbandonedOffice, AbandonedWarehouse, Warehouse, Sewers, Caves, Tech
	PersistentNPCs Genevieve_Sanders, Juan_Jimenez, Vic_Johansson, Paula_Dempsey, Blue_Steel, Security_Chief_03, Samuel_Pierce, Ron_Hughes, Linda_Summers, NotorietyContact2
	VisitLocations Patrol_Easy_5_City_01_02, Patrol_Easy_1_City_01_02, Patrol_Easy_2_City_01_02, Patrol_Easy_3_City_01_02, Patrol_Easy_4_City_01_02, Patrol_Hard_5_City_01_02, Patrol_Hard_4_City_01_02, Patrol_Hard_3_City_01_02, Patrol_Hard_2_City_01_02, Patrol_Hard_1_City_01_02,  
	//Plaques AxisAttack3, AxisAttack4, PsychicFeedbackLoop3, PsychicFeedbackLoop4, PsychicFeedbackLoop5,
	EncounterLayouts Rumble, Snatch, Single, AroundVandalism, Ambush, Duo, Encounter, Around, 
	VillainGroups CircleOfThorns, Skulls, TheLost, Vahzilok, Clockwork, Trolls, 
}

// Galaxy City
Map City_01_03.txt
{
	MissionMinLevel 1
	MissionMaxLevel 9
	SpawnOverrideMinLevel 1
	SpawnOverrideMaxLevel 9

	MissionDoors Office, AbandonedWarehouse, Warehouse, Caves, Sewers, Tech
	PersistentNPCs FreedomCorp_City_01_03c, FreedomCorp_City_01_03b, FreedomCorp_City_01_03, Charlie_Sparks, Security_Chief_02, Kiros_Nandelu, Derek_Amberson, Clarence_Jackson, Back_Alley_Brawler, Maurice_Feldon, SuperGroupRegistrar, Paco_Sanchez, Kip_Cantorum, Rebecca_Brinell, Caitlin_Murray, Gregor_Richardson, Rachel_Torres, NotorietyContact1
	VisitLocations Patrol_Easy_1_City_01_03, Patrol_Easy_2_City_01_03, Patrol_Easy_3_City_01_03, Patrol_Easy_4_City_01_03, Patrol_Easy_5_City_01_03, Patrol_Hard_1_City_01_03, Patrol_Hard_2_City_01_03, Patrol_Hard_3_City_01_03, Patrol_Hard_4_City_01_03, Patrol_Hard_5_City_01_03, Statue2, 
	//Plaques AxisAttack2, CitizenCrimeAct3, MightForRightAct2, MightForRightAct3, PsychicFeedbackLoop2, 
	EncounterLayouts Snatch, Single, AroundVandalism, Duo, Encounter, Around, 
	VillainGroups Hellions, Vahzilok, Clockwork, 
}

// Steel Canyon
Map City_02_01.txt
{
	MissionMinLevel 10
	SpawnOverrideMinLevel 10

	MissionDoors Office, AbandonedOffice, Warehouse, AbandonedWarehouse, Caves, Sewers, Tech
	PersistentNPCs Kyle_Peck, Mutant_City_02_01, Tech_City_02_01c, Guy_Denson, Mannequin_Tech_Female, Science_City_02_01b, Shelly_Knowles, Natural_City_02_01, Virginia_Hoffman, Security_Chief_05, Fareed_Abdullah, Olivia_Chung, Wes_Schnabel, Mannequin_Mut_Male, IconEmployee_City_02_01c, Tech_City_02_01, Tech_City_02_01b, Positron, Mannequin_Nat_Female, Valkyrie, Tom_Bowden, FreedomCorp_City_02_02b, Mannequin_Sci_Male, IconEmployee_City_02_01b, Mannequin_Nat_Male, Willy_Starbuck, Wyatt_Anderson, Security_Chief_17, Science_City_02_01, Mannequin_Sci_Female, IconEmployee1, Hugo_Redding, Natural_City_02_01c, Magic_City_02_01c, IconEmployee_City_02_01a, Mutant_City_02_01c, Magic_City_02_01, Mutant_City_02_01b, Mannequin_Mut_Female, Mannequin_Tech_Male, Security_Chief_06, Magic_City_02_01b, Wilson_Zucco, Natural_City_02_01b, Colleen_Saramago, FreedomCorp_City_02_02c, Dr_Trevor_Seaborn, Carlos_Herrera, Alfonse_Rubel, Mannequin_Mage_Male, Science_City_02_01c, FreedomCorp_City_02_02, Athena_Currie, NotorietyContact7
	VisitLocations Patrol_Hard_2_City_02_01, Patrol_Easy_3_City_02_01, Patrol_Easy_1_City_02_01, Patrol_Easy_5_City_02_01, Patrol_Easy_4_City_02_01, Patrol_Easy_2_City_02_01, Patrol_Hard_5_City_02_01, Patrol_Hard_4_City_02_01, Patrol_Hard_1_City_02_01, Patrol_Hard_3_City_02_01,  
	//Plaques WarOnDrugs1, Superadine1, Superadine2, HeroCorps1,
	EncounterLayouts Around, Single, Snatch, Duo, Encounter, AroundVandalism, GroupedDirection, Rumble, 
	VillainGroups Tsoo, CircleofThorns, Clockwork, OutCasts, Vahzilok, Council,  
}

// Skyway City
Map City_02_02.txt
{
	MissionMinLevel 10
	SpawnOverrideMinLevel 10

	MissionDoors Office, AbandonedOffice, Caves, Warehouse, Caves, Tech, Sewers
	PersistentNPCs Mutant_City_02_01, Tech_City_02_01c, Mutant_City_02_01b, Lorenzo_DiCosta, Mark_Freeman, Mynx, Science_City_02_01b, Science_City_02_01c, Thao_Ku, Natural_City_02_01, Yancy_Rhymes, Security_Chief_07, Maggie_Greene, Tech_City_02_01, Karen_Parker, Tech_City_02_01b, Everett_Daniels, Haley_Philips, Yolanda_Baker, FreedomCorp_City_02_02b, Magic_City_02_01, Pavel_Garnier, Security_Chief_17, Science_City_02_01, Kong_Bao, Ann-Marie_Engles, Cho_Ge, Tristan_Caine, Corey_McCann, Natural_City_02_01c, Magic_City_02_01c, Warren_Trudeau, Synapse, Andre_Jimenez, Sgt_Suzanne_Bernhard, Vitaly_Cherenko, Jerry_Kazatsky, Jake_Montoya, Natural_City_02_01b, Kira_Lange, FreedomCorp_City_02_02c, Jill_Pastor, Mutant_City_02_01c, Hiro_Takashi, Magic_City_02_01b, Security_Chief_08, The_Can_Man, FreedomCorp_City_02_02, Carla_Brunelli, Juliana_Nehring, Sanjay_Chandra, NotorietyContact8
	VisitLocations Patrol_Hard_5_City_02_02, Patrol_Hard_4_City_02_02, Patrol_Hard_3_City_02_02, Patrol_Hard_2_City_02_02, Patrol_Hard_1_City_02_02, Patrol_Easy_5_City_02_02, Patrol_Easy_4_City_02_02, Patrol_Easy_3_City_02_02, Patrol_Easy_2_City_02_02, Patrol_Easy_1_City_02_02,  
	//Plaques WarOnDrugs2, WarOnDrugs3, Superadine3, HeroCorps2,
	EncounterLayouts AroundDoor, Single, Snatch, Duo, Encounter, AroundVandalism, Around, 
	VillainGroups Clockwork, TheLost, Trolls, 
}


Map City_02_03.txt
{
}

// Talos Island
Map City_03_01.txt
{
	MissionMinLevel 20
	SpawnOverrideMinLevel 20

	MissionDoors Office, AbandonedOffice, Warehouse,  Council,  Caves, Tech, Sewers, CircleOfThorns, TI_CargoShip
	PersistentNPCs Mutant_City_03_01c, Jim_Bell, Tech_City_03_01b, FreedomCorp_City_03_01b, Natural_City_03_02b, Science_City_03_02b, Manuel_Ruiz, Tech_City_03_01, John_Strobel, Joesef_Keller, Magic_City_03_01c, Eliza_Thorpe, Evan_DiTomasso, Luminary, Cain_Royce, Joshua_Stutz, Magic_City_03_01b, Science_City_03_02, Tyler_French, Claire_Childress, Tech_City_03_01c, Janelle_Irving, Lt_Col_Hugh_McDougal, Mutant_City_03_01, Mutant_City_03_01b, Jesse_Hobart, FreedomCorp_City_03_01, Piper_Irving, Vic_Garland, Alexander, Wendy_Klein, Bastion, Marvin_Weintraub, Annie_Coults, Oliver_Haak, Andrew_Fiore, Miriam_Bloechl, FreedomCorp_City_03_01c, Hinckley_Rasmussen, Security_Chief_10, Natural_City_03_02, Natural_City_03_02c, Betty_Abbot, Polly_Cooper, Robert_Koslowski, Science_City_03_02c, Security_Chief_09, Barry_Gosford, Andrea_Mitchell, Magic_City_03_01, NotorietyContact3
	VisitLocations Patrol_Hard_5_City_03_01, Patrol_Hard_4_City_03_01, Patrol_Hard_3_City_03_01, Patrol_Hard_2_City_03_01, Patrol_Hard_1_City_03_01, Patrol_Easy_5_City_03_01, Patrol_Easy_4_City_03_01, Patrol_Easy_3_City_03_01, Patrol_Easy_2_City_03_01, Patrol_Easy_1_City_03_01, 
	//Plaques SpecialCouncil2, FreedomPhalanx3, SpankyRabinowitz2, SpankyRabinowitz3, 
	EncounterLayouts Rumble, AroundStore, Single, Snatch, Duo, Encounter, AroundVandalism, Around, CargoLookout, CargoRumbles
	VillainGroups Warriors, CircleOfThorns, Tsoo, Freakshow, DevouringEarth, BanishedPantheon, 
}

// Independence Port
Map City_03_02.txt
{
	MissionMinLevel 22
	SpawnOverrideMinLevel 22

	MissionDoors AbandonedOffice, AbandonedWarehouse, Warehouse,  Council,  Sewers, Tech, IP_CargoShip
	PersistentNPCs Justin_Greene, Mutant_City_03_01c, Mannequin_Tech_Female, IconEmployee_City_03_02d, Lorenzo_Tate, Tech_City_03_01b, FreedomCorp_City_03_01b, Natural_City_03_02b, Science_City_03_02b, Magic_City_03_01c, Collin_Larson, Tech_City_03_01, Magic_City_03_01, Mannequin_Mut_Male, Mannequin_Nat_Female, Georgia_Fields, Christine_Lansdale, Melanie_Peebles, Mannequin_Sci_Male, Mannequin_Nat_Male, Dr_Cheng, Science_City_03_02, Sister_Psyche, Mannequin_Sci_Female, Magic_City_03_01b, IconEmployee_City_03_02e, Security_Chief_12, Mutant_City_03_01, Mutant_City_03_01b, Jake_Kim, Aurora_Borealis, IconEmployee_City_03_02f, FreedomCorp_City_03_01, Tech_City_03_01c, Dennis_Ewell, Mannequin_Mut_Female, Laurie_Pennington, Security_Chief_11, FreedomCorp_City_03_01c, Laura_Brunetti, Natural_City_03_02, Wilma_Peterson, Wanda_Travis, Amanda_Loomis, Karl_Bolger, Natural_City_03_02c, Oswald_Cuthbert, Kevin_Cordell, Wilson_Eziquerra, Science_City_03_02c, Justine_Kelly, Ashwin_Lannister, Rondel_Jackson, Mannequin_Mage_Male, Mannequin_Tech_Male, Kirsten_Woods, IconEmployee2, Jane_Hallaway, NotorietyContact9
	VisitLocations Patrol_Easy_1_City_03_02, Patrol_Easy_2_City_03_02, Patrol_Easy_3_City_03_02, Patrol_Easy_4_City_03_02, Patrol_Easy_5_City_03_02, Patrol_Hard_1_City_03_02, Patrol_Hard_2_City_03_02, Patrol_Hard_3_City_03_02, Patrol_Hard_4_City_03_02, Patrol_Hard_5_City_03_02, 
	//Plaques SpecialCouncil1, FreedomPhalanx1, FreedomPhalanx2, SpankyRabinowitz1, 
	EncounterLayouts Around, Single, AroundVandalism, Duo, Encounter, GroupedDirection, Rumble, RumbleCatwalk, CargoLookout, CargoRumbles
	VillainGroups Tsoo, Council,  TheFamily, DevouringEarth, 
}

// Croatoa
Map City_03_03.txt
{
	EntryMinLevel 25
	MissionMinLevel 25
	SpawnOverrideMinLevel 28

	MissionDoors Cro_Fairy1, Cro_Fairy2, Cro_Fairy3, Cro_Fairy4, Cro_Fairy5, Cro_Fairy6, Cro_Fairy7, Cro_Fairy8, Cro_Fairy9, Cro_Salamanca_Warehouse, Cro_Salamanca_Office, Cro_Salamanca_Office, Cro_Salamanca_Sewers, Cro_BT_Caves, Cro_Lake_Caves, Cro_SR_Caves, Cro_NC_Caves, Cro_MW_Caves, Cro_SR_Tree 
	PersistentNPCs Gordon_Bower, Skipper_LeGrange, Kelly_Nemmers, Buck_Salinger, Katie_Hannon, War_Witch
	//VisitLocations Patrol_Easy_1_City_03_02, Patrol_Easy_2_City_03_02, Patrol_Easy_3_City_03_02, Patrol_Easy_4_City_03_02, Patrol_Easy_5_City_03_02, Patrol_Hard_1_City_03_02, Patrol_Hard_2_City_03_02, Patrol_Hard_3_City_03_02, Patrol_Hard_4_City_03_02, Patrol_Hard_5_City_03_02, 
	//Plaques SpecialCouncil1, FreedomPhalanx1, FreedomPhalanx2, SpankyRabinowitz1, 
	EncounterLayouts Event_Tuatha, Event_Fey, Event_Cabal, Event_FirBolg, FeySquabble, CabalCaptured, FeyTargetPractice, FeyAmbush, FeyBattleLine, JackPatrol, FeyLoiter, FeyGuarding, FeyScaffold, FeyConference, FeyLoiter, CabalLoiter, Fey_CabalRngRumble, Fey_CabalRumble, FeyGuarding, FeyCeremony, TotemPull, FeyTrap, FeyTrees, FirTrees, WalkingTrees, TuathaCeremony, ForestClearing, InvasionForce, InvasionPrep1, InvasionPrep2, TuathaLoiter, TuathaTrees, WispLoiterDuo, WispPatrol, WispTrees, JackSleeping, JackPatrol, FirLoiter, 3WayFight, TomeReading, EochaiGroup, EochaiTree, FirBarnAttack, FirCabalStandOff, Coerce, FirBolgPatrol, FirPortal, FieldAmbush, FromAbove, Scarecrows, Vandalize, EochaiWorship, HaystackRave, NightIncursion, FarmerDigHole, FarmerPickAxe, PumpkinWheelbarrel, Repair, Weeding, WheelbarrelRocks, CabalLoiter, TuathaLoiter, TuathaHayStack, TuathaFields, FirHayStack, GhostsHaunt, GhostsLoiter, CabalHarass, Loiter, GhostHaunt, Loiter, SanityCheck, TotemPull, Trees, GhostHaunt, Emergence, Rumble, TotemPull, NPC, Loiter, Converse, Blocked, Blocking, CarHopping, Phone, Rummage, FromBuilding, Rampage, GhostPerch, FirCabalTalks, CabalDoorAmbush, CabalDeepDefense, CabalLoiter, CabalNightLights, CabalShoreDefense, CabalRedCapHostage, CabalFarming, CabalSkyPatrol, CabalGiantTree
	VillainGroups FirBolg, Tuatha, Cabal, RedCaps, CroatoaGhosts
}

// Founder's Falls
Map City_04_01.txt
{
	MissionMinLevel 31
	SpawnOverrideMinLevel 31

	MissionDoors Office, Caves, sewers, Tech, CircleOfThorns, Council
	PersistentNPCs Security_Chief_15, Tina_Chung, Maxwell_Christopher, Peter_Stemitz, Infernal, Phillipa_Meraux, Cadao_Kestrel, Jenny_Firkins, Mannequin_Tech_Female, Anton_Sampson, Numina, Jose_Escalante, Janet_Kellum, Angus_McQueen, Penny_Preston, Mark_IV, Mannequin_Tech_Male, Mannequin_Mut_Male, IconEmployee3, IconEmployee_City_04_01g, IconEmployee_City_04_01i, Ginger_Yates, IconEmployee_City_04_01h, Mannequin_Nat_Female, Mannequin_Sci_Male, Mannequin_Nat_Male, Mannequin_Mut_Female, Indigo, Security_Chief_16, Agent_Six, Mannequin_Mage_Male, Mannequin_Sci_Female, Maren_MacGregor, Madeleine_Casey, James_Harlan, NotorietyContact10
	VisitLocations Patrol_Easy_1_City_04_01, Patrol_Easy_2_City_04_01, Patrol_Easy_4_City_04_01, Patrol_Hard_1_City_04_01, Patrol_Easy_3_City_04_01, Patrol_Hard_2_City_04_01, Patrol_Easy_5_City_04_01, Patrol_Hard_3_City_04_01, Patrol_Hard_4_City_04_01, Patrol_Hard_5_City_04_01, 
	//Plaques RiktiWar1, DimensionalBarrior1,
	EncounterLayouts Rumble, Around, Catwalk, Single, AroundVandalism, Duo, Encounter, Phalanx, GroupedDirection, 
	VillainGroups CircleOfThorns, Crey, Rikti, DevouringEarth, Council,  
}

// Brickstown
Map City_04_02.txt
{
	MissionMinLevel 30
	SpawnOverrideMinLevel 30

	MissionDoors Office, AbandonedOffice, AbandonedWarehouse, Warehouse, Sewers,  Council,  Caves, Tech, Council
	PersistentNPCs Merisel_Valenzuela, Neal_Kendrick, Security_Chief_14, Holsten_Armitage, Swan, Colleen_Nelson, Gordon_Stacy, Steven_Sheridan, Lou_Pasterelli, Security_Chief_13, Serafina, Manticore, Allison_King, NotorietyContact4
	VisitLocations Patrol_Hard_5_City_04_02, Patrol_Hard_4_City_04_02, Patrol_Hard_3_City_04_02, Patrol_Hard_2_City_04_02, Patrol_Hard_1_City_04_02, Patrol_Easy_5_City_04_02, Patrol_Easy_4_City_04_02, Patrol_Easy_3_City_04_02, Patrol_Easy_2_City_04_02, Patrol_Easy_1_City_04_02, 
	//Plaques DimensionalBarrier2, DimensionalBarrier3, 
	EncounterLayouts Rumble, Snatch, Single, AroundVandalism, Duo, Encounter, Phalanx, GroupedDirection, Around, 
	VillainGroups Crey, Prisoners, Freakshow, Council,  
}

// Peregrine Island
Map City_05_01.txt
{
	MissionMinLevel 40
	SpawnOverrideMinLevel 40

	MissionDoors Office, Tech, Portal, Warehouse, CircleOfThorns,  Council,  Caves, PI_CargoShip
	PersistentNPCs Harvey_Maylor, Ghost_Falcon, Maria_Jenkins, Tina_Macintyre, Unai_Kemen, Crimson, NotorietyContact5
	VisitLocations Patrol_Easy_1_City_05_01, Patrol_Easy_2_City_05_01, Patrol_Easy_3_City_05_01, Patrol_Easy_4_City_05_01, Patrol_Easy_5_City_05_01, Patrol_Hard_1_City_05_01, Patrol_Hard_2_City_05_01, Patrol_Hard_3_City_05_01, Patrol_Hard_4_City_05_01, Patrol_Hard_5_City_05_01, 
	//Plaques NemesisSeizesControl1, 
	EncounterLayouts CircledAroundEX, Catwalk, Assault, Single, PersuadeDemo, Ambush, Encounter, Phalanx, Around, CargoLookout, CargoRumbles
	VillainGroups Malta, CircleOfThorns, Carnival, Nemesis, Rikti, DevouringEarth, 
}

// Perez Park
Map Hazard_01_01.txt
{
	EntryMinLevel 7
	MissionMinLevel 9
	SpawnOverrideMinLevel 9

	MissionDoors Office, AbandonedOffice, Caves, Tech, Sewers
	VisitLocations Patrol_Hard_5_Hazard_01_01, Patrol_Hard_4_Hazard_01_01, Patrol_Hard_3_Hazard_01_01, Patrol_Hard_2_Hazard_01_01, Patrol_Hard_1_Hazard_01_01, Patrol_Easy_5_Hazard_01_01, Patrol_Easy_4_Hazard_01_01, Patrol_Easy_3_Hazard_01_01, Patrol_Easy_2_Hazard_01_01, Patrol_Easy_1_Hazard_01_01, 
	//Plaques AxisAttack5, CitizenCrimeAct4, CitizenCrimeAct5, MightForRightAct4, 
	EncounterLayouts Rumble, Ambush, Encounter, Around, Single, 
	VillainGroups CircleofThorns, Clockwork, Hydra, Skulls, Vahzilok, Hellions, TheLost, 
}

// The Hollows
Map Hazard_01_02.txt
{
	EntryMinLevel 5
	MissionMinLevel 5
	SpawnOverrideMinLevel 5

	MissionDoors AbandonedOffice, Caves, CircleOfThorns, Office
	VisitLocations Patrol_Hard_5_Hazard_01_02, Patrol_Hard_4_Hazard_01_02, Patrol_Hard_3_Hazard_01_02, Patrol_Hard_2_Hazard_01_02, Patrol_Hard_1_Hazard_01_02, Patrol_Easy_5_Hazard_01_02, Patrol_Easy_4_Hazard_01_02, Patrol_Easy_3_Hazard_01_02, Patrol_Easy_2_Hazard_01_02, Patrol_Easy_1_Hazard_01_02, 
	EncounterLayouts Around, Rumble, DestroyEX, Initiation, CenteredAround, Research, Ambush, FromBelow
	VillainGroups CircleofThorns, Trolls, Outcasts, Igneous, 
}

// The Tunnels
Map Hazard_01_03.txt
{
	EntryMinLevel 12
	MissionMinLevel 12
	SpawnOverrideMinLevel 12

	MissionDoors Cavern,
	EncounterLayouts Around, Ambush, Phalanx,
	VillainGroups CircleofThorns, Trolls, Igneous, 
}

// Boomtown
Map Hazard_02_01.txt
{
	EntryMinLevel 10
	MissionMinLevel 13
	SpawnOverrideMinLevel 13

	MissionDoors AbandonedOffice, Sewers
	VisitLocations Patrol_Easy_5_Hazard_02_01, Patrol_Easy_2_Hazard_02_01, Patrol_Easy_4_Hazard_02_01, Patrol_Hard_3_Hazard_02_01, Patrol_Hard_4_Hazard_02_01, Patrol_Hard_5_Hazard_02_01, Patrol_Hard_1_Hazard_02_01, Patrol_Hard_2_Hazard_02_01, 
	//Plaques WarOnDrugs4, Superadine4, HeroCorps5, HeroCorps3, HeroCorps4, 
	EncounterLayouts Rumble, Single, Phalanx, Around, AroundSpawn, 
	VillainGroups Clockwork, Outcasts, TheLost, Vahzilok, Council,  Trolls, 
}

// Dark Astoria
Map Hazard_03_01.txt
{
	EntryMinLevel 21
	MissionMinLevel 23
	SpawnOverrideMinLevel 23
	
	MissionDoors AbandonedOffice, AbandonedWarehouse, Caves, BPSA5StoryArcDoor, Sewers, CircleOfThorns, Council
	VisitLocations Patrol_Easy_1_Hazard_03_01, Patrol_Easy_4_Hazard_03_01, Patrol_Easy_3_Hazard_03_01, Patrol_Easy_2_Hazard_03_01, Patrol_Easy_5_Hazard_03_01, Patrol_Hard_2_Hazard_03_01, Patrol_Hard_3_Hazard_03_01, Patrol_Hard_4_Hazard_03_01, Patrol_Hard_5_Hazard_03_01, Patrol_Hard_1_Hazard_03_01, 
	//Plaques SpecialCouncil3, FreedomPhalanx4, SpankyRabinowitz4, SpankyRabinowitz5, 
	EncounterLayouts Rumble, ZombieBurst1, ZombieBurst4, ZombieBurst3, ZombieBurst2, Encounter, Phalanx, Around, Single, 
	VillainGroups Tsoo, BanishedPantheon, CircleofThorns, 
}

// Striga Isle
Map Hazard_03_02.txt
{
	EntryMinLevel 20
	MissionMinLevel 20
	SpawnOverrideMinLevel 23
	
	MissionDoors SI_cargoW, SI_cargoC, SI_cargoF, SI_building, SI_warehouse, SI_maw, SI_maw1, SI_mausoleum, SI_vampcave, SI_cave, SI_airbase, SI_lighthouse, SI_base, SI_basebunker, SI_smallrobot1, SI_smallrobot2, SI_giantrobot1, SI_giantrobot2, SI_giantrobot3, SI_radar1, SI_radar2, SI_radar3, SI_radar4
	EncounterLayouts Rumble, Encounter, GraveDig, BigAround, TurkeyShoot, BridgeAssault, StrigaPatrol, GuardShip, CrateMove, Dogfight, Destroy, Duo, ThroatRumble, Snake, Ambush, Phalanx, Around, PlaceBomb, BlockShip, StrigaAirPatrol, StrigaAirPatrolBird, Trio, BrawlShipRec, BrawlShipBox,  
	VillainGroups TheFamily, Warriors, Council, BanishedPantheon, Turrets, SkyRaiders, DevouringEarth
	PersistentNPCs Ravenstorm, Moonfire, Stephanie_Peebles, Long_Jack, Tobias_Hansen, Lars_Hansen, Ernesto_Hess
}

// Crey's Folly
Map Hazard_04_01.txt
{
	EntryMinLevel 30
	MissionMinLevel 32
	SpawnOverrideMinLevel 32

	MissionDoors AbandonedOffice, AbandonedWarehouse, Warehouse, Sewers, CreyLab
	VisitLocations Patrol_Easy_1_Hazard_04_01, Patrol_Easy_2_Hazard_04_01, Patrol_Easy_3_Hazard_04_01, Patrol_Easy_4_Hazard_04_01, Patrol_Easy_5_Hazard_04_01, Patrol_Hard_1_Hazard_04_01, Patrol_Hard_2_Hazard_04_01, Patrol_Hard_3_Hazard_04_01, Patrol_Hard_4_Hazard_04_01, Patrol_Hard_5_Hazard_04_01, 
	//Plaques RiktiWar2, RiktiWar3, 
	EncounterLayouts Rumble, Trio, Catwalk, Single, Snatch, Ambush, Duo, Encounter, RumbleRanged, Around, Destroy, 
	VillainGroups Crey, Rikti, Freakshow, Nemesis, DevouringEarth, 
}

// Firebase Zulu - Shadow Shard
Map Hazard_06_01.txt
{
	EntryMinLevel 40
	MissionMinLevel 40
	SpawnOverrideMinLevel 40

	MissionDoors Cave_Foxtrot, Cave_XRay, Cave_Tango, Cave_Echo, Tech_Charlie, Tech_Victor, Tech_XRay, Tech_Tango_1, Tech_Sierra, Tech_Alpha, Tech_Foxtrot, Tech_Zulu, Tech_Echo, Tech_Tango_2, SS_Caves, SS_Tech, 
	VisitLocations Hazard_06_01_Device, MonumentEcho1, MonumentSierra1, MonumentSierra2, MonumentSierra3, MonumentSierra4, MonumentTango1, MonumentTango2, MonumentTango3, MonumentTango4, MonumentTango5,
	PersistentNPCs General_Hammond, Dr_Quaterfield, Dr_Boyd, Lt_Volkov, Sgt_Goddard,
	EncounterLayouts Rumble, Trio, Encounter, Around, BigAround, Phalanx, Capture, skirmish, Outpost, 
	VillainGroups Crey, Nemesis, Rularuu, Reflections, 
}

// Cascade Archipelago - Shadow Shard
Map Hazard_06_02.txt
{
	EntryMinLevel 40
	MissionMinLevel 40
	SpawnOverrideMinLevel 40

	MissionDoors Cave_AirShoals, Tech_Waterfall_1, Cave_Chanting_1, Cave_Chanting_2, Cave_Marooned, Tech_Waterfall_2, Cave_OneWay, Cave_Waterfall_3, Cave_Chanting_3, Cave_Tyrant_1, Tech_Marooned, Cave_Tyrant_2, SS_Caves, SS_Tech,
	PersistentNPCs Lt_Col_Flynn, Sara_Moore, Dr_Huxley,
	EncounterLayouts GuardSpot, BigAround, Trio, Battle, Around,
	VillainGroups Nemesis, Rularuu, Reflections, CircleofThorns,
}

// The Chantry - Shadow Shard
Map Hazard_06_03.txt
{
	EntryMinLevel 40
	MissionMinLevel 40
	SpawnOverrideMinLevel 44

	MissionDoors Cave_Guilt, Cave_Pain, Cave_Sorrow, SS_Caves, SS_Chantry, 
	VisitLocations MonumentPain, MonumentDenial, MonumentGuilt, MonumentShame, MonumentSorrow, MonumentRegret,
	PersistentNPCs Justin_Augustine, Faathim_The_Kind,
	EncounterLayouts BigAround, Trio, Around, Destroy, 
	VillainGroups Rularuu, Reflections, CircleofThorns,
}

// Abandoned Sewer Network
Map Trial_01_01.txt
{
	EntryMinLevel 36
	MissionMinLevel 38
	SpawnOverrideMinLevel 38

	MissionDoors SewerTrialRoom
	VisitLocations NemesisSeizesControl3, 
	PersistentNPCs Computer
	EncounterLayouts Ambush, DeathFromAbove, Direction, Around, InWater, 
	VillainGroups CircleOfThorns, Rikti, 
}

// Sewer Network
Map Trial_01_02.txt
{
	EntryMinLevel 1
	MissionMinLevel 1
	SpawnOverrideMinLevel 1

	EncounterLayouts Ambush, DeathFromAbove, Direction, Around, 
	VillainGroups TheLost, Hellions, Vahzilok, 
}

// Faultline
Map Trial_02_01.txt
{
	EntryMinLevel 14
	MissionMinLevel 16
	SpawnOverrideMinLevel 16
	
	//Plaques WarOnDrugs5, Superadine5, HeroCorps6, HeroCorps7, 
	EncounterLayouts AroundDoor, Single, Around, 
	VillainGroups CircleOfThorns, Clockwork, Vahzilok, 
}

// Terra Volta
Map Trial_03_01.txt
{
	EntryMinLevel 20
	MissionMinLevel 20
	SpawnOverrideMinLevel 25
	
	PersistentNPCs JasonS
	MissionDoors TV_Reactor
	//Plaques SpecialCouncil4, SpecialCouncil5, FreedomPhalanx5, SpankyRabinowitz6, 
	EncounterLayouts RumbleRanged, Catwalk, AroundDoor, Single, AroundVandalism, RumbleRangedCatwalk, CenteredAround, Phalanx, GroupedDirection, Rumble, RumbleCatwalk, 
	VillainGroups SkyRaiders, TheLost, Freakshow, DevouringEarth, 
}

// Eden
Map Trial_04_01.txt
{
	EntryMinLevel 33
	MissionMinLevel 35
	SpawnOverrideMinLevel 38
		
	//Plaques RiktiWar4, RiktiWar5, 
	MissionDoors EdenCave, EdenBuilding, EdenTrialRoom
	EncounterLayouts Rumble, Ambush, Phalanx, GroupedDirection, Around, Single, 
	VillainGroups Crey, Nemesis, DevouringEarth, 
}

// The Hive
Map Trial_04_02.txt
{
	EntryMinLevel 45
	MissionMinLevel 45
	SpawnOverrideMinLevel 46
	EncounterLayouts Ambush, GroupedDirection, Around, Hamidon, 
}

// Rikti Crash Site
Map Trial_05_01.txt
{
	EntryMinLevel 40
	MissionMinLevel 40
	SpawnOverrideMinLevel 40
	
	PersistentNPCs Richard_Flagg
	VisitLocations TV_Patrol_01, TV_Patrol_02, TV_Patrol_03, 
	//Plaques NemesisSeizesControl2, 
	MissionDoors Trial_AbandonedOffice
	EncounterLayouts Around_Spread_Out, Outpost, Single, Ambush, Attack, Encounter, skirmish, guard, capture, 
	VillainGroups Crey, Rikti, 
}

// The Storm Palace - Shadow Shard
Map Trial_06_01.txt
{
	EntryMinLevel 44
	MissionMinLevel 40
	SpawnOverrideMinLevel 46 
	
	MissionDoors SS_Caves, SS_StormPalace, Cave_Anger, Cave_Malice, Cave_Torment, Cave_Hatred, Cave_Fury, Cave_Destruction,
	EncounterLayouts Ambush, BigAround, BigGroupedDir, Snake, 
	VisitLocations MonumentMadness, MonumentAnger, MonumentMalice, MonumentTorment, MonumentHatred, MonumentFury, MonumentDestruction,
	VillainGroups Rularuu, StormElementals,
}

Map Vinceland.txt
{
	// appropriate level ranges for this zone
	MissionMinLevel 1
	MissionMaxLevel 50
	SpawnOverrideMinLevel 1
	SpawnOverrideMaxLevel 50

	// stuff that exists on this map
	MissionDoors VinceDoor
	EncounterLayouts AroundVandalism,
	VillainGroups Hellions,
}


//////////////////////////////////// CITY OF VILLAINS ////////////////////////////////////////


// Breakout
Map V_City_00_01.txt
{
	MissionMinLevel 1
	MissionMaxLevel 1
	SpawnOverrideMinLevel 1
	SpawnOverrideMaxLevel 1
	
	MissionDoors VTutotial_Bunker Door
	EncounterLayouts Single, Trio, CenteredAround 
	VisitLocations Tutorial1, Tutorial2 
	VillainGroups Zig
	//Plaques Tutorial1
}

// Mercy Island
Map V_City_01_01.txt
{
	MissionMinLevel 1
	SpawnOverrideMinLevel 1
	
	PersistentNPCs Mongoose, Kalinda, Arbiter_Diaz, Notoriety_Contact_Mercy, Seer_Marino,
	MissionDoors MI_Darwin_SHole, MI_Darwin_AbandonedOffice, MI_Darwin_Warehouse, MI_Darwin_Arachnos, MI_Darwin_Manhole, MI_Mercy_SHole, MI_Mercy_Office, MI_Mercy_Warehouse, MI_Mercy_Arachnos, MI_Mercy_Manhole, MI_Mercy_Bank, MI_FTCerberus_SHole, MI_FTCerberus_Arachnos, MI_FTCerberus_Caves, MI_Larry_Casino, MI_FTCerberus_GhostWidow, MI_Darwin_TugBoat, MI_Arachnos_Clocktower, MI_Snakehole_Easy, MI_Darwin_AbandonedOffice, MI_Mercy_Warehouse 
	EncounterLayouts BarFightOut,  
	VisitLocations 
	VillainGroups Hellions, Skulls, Arachnos, Snakes, Infected, 
	//Plaques CoTHistory1, Mercy1, Mercy2, Mercy3, Mercy4, Mercy5, Mercy6
}

// Port Oakes
Map V_City_01_02.txt
{
	MissionMinLevel 5
	SpawnOverrideMinLevel 5
	
	PersistentNPCs Angelo_Vendetti, Billie_Heck, Drea_the_Hook, Mikey_the_Ear, Mr_Bocor, The_Radio, Veluta_Lunata,
	MissionDoors Office, AbandonedOffice, Caves, CargoShip, Warehouse, AbandonedWarehouse, Sewers, PO_AD_Office, PO_AD_AbandonedOffice, PO_Dock_AbandonedWarehouse, PO_Dock_AbandonedOffice, PO_Dock_Office, PO_CargoShip, PO_OS_Office, PO_M_Office, PO_FH_Caves, CircleofThorns, PO_FH_Office, PO_VM_Office, PO_Bank_01, PO_Bank_02, PO_AD_Caves, PO_Dock_Caves, PO_AD_Sewers, PO_OS_AbandonedWarehouse, PO_OS_Warehouse, PO_Dock_Sewers, PO_OS_Caves, PO_FH_Sewers, PO_M_Sewers, PO_Truck_Easy, PO_Truck_Hard, PO_Casino_01, PO_Casino_02, PO_CargoShip_Sunk, PO_TugBoat, PO_Dock_SnakeHole, PO_Dock_ManHole,  
	EncounterLayouts Duo, Bookie, Encounter, BackOff, AlleyShootout, Snatch, BarFightOut, Trio, Around, Hustle, StrigaPatrol, Destroy, Barrel, GuardShip, CrateMove, OilBlaze, Caught, Phalanx, GraveDig, 
	VisitLocations 
	VillainGroups Hellions, Arachnos, Council, CircleofThorns, SpectralPirates, TheLost, Coralax
	//Plaques CoTHistory2, PirateHistory4, PortOakes1, PortOakes2, PortOakes3, PortOakes4
}

// Cap Au Diable
Map V_City_02_01.txt
{
	MissionMinLevel 8
	SpawnOverrideMinLevel 8
	PersistentNPCs Dmitri_Krylov, Dr_Shelly_Percey, Golden_Roller, Marshal_Brass, Operative_Wellman, Peter_Themari, Virgil_Tarikoss, Willy_Wheeler,
	MissionDoors Office, AbandonedOffice, Caves, CargoShip, Warehouse, AbandonedWarehouse, Sewers, CD_HV_Caves, CD_HV_Arachnos, CD_HV_Sewers, CD_HV_CargoShipA, CD_HV_CargoShipB, CD_HV_Office, CD_CT_Sewers, CD_CT_Warehouse, CD_CT_AbandonedWarehouse, CD_AC_Office, CD_AC_Arachnos, CD_AC_Caves, CD_NH_Office, CD_NH_Sewers, CD_NH_WSPDR, CD_VH_Caves, CD_MD_Caves, CD_MD_Warehouse, Tarikoss_Portal
	EncounterLayouts 
	VisitLocations 
	VillainGroups Vahzilok, Clockwork, Arachnos, Luddites, GoldBrickers, CircleofThorns
	//Plaques DrAeonHistory1, DrAeonHistory2, DrAeonHistory3, CoTHistory3, CapauDiable1, CapauDiable2, CapauDiable3, CapauDiable4, CapauDiable5
	ZoneScripts scriptdefs/V_CapAuDiable_Gremlins.scriptdef 
}

// Sharkhead Isle
Map V_City_03_01.txt
{
	MissionMinLevel 20
	SpawnOverrideMinLevel 20
	
	PersistentNPCs Archmage_Tarixus, Captain_Petrovich, Crash_Cage, Doc_Buzzsaw, Lorenz_Ansaldo, Lt_Chalmers, Operative_Kirkland, Operative_Vargas, Vince_Dubrowski, Diviner_Maros, Henri_Dumont, Arbiter_Friesen, 
	MissionDoors Office, AbandonedOffice, Caves, CargoShip, Warehouse, AbandonedWarehouse, Sewers, SH_Port_Warehouse, SH_Port_AbandonedWarehouse, SH_Port_Office, SH_Port_AbandonedOffice, SH_Port_CargoShip_Family, SH_Port_CargoShip, SH_Crush_Warehouse, SH_Crush_AbandonedWarehouse, SH_Crush_Caves, SH_Crush_Sewers, SH_Hellforge_Caves, SH_Hellforge_Warehouse, SH_Hellforge_AbandonedWarehouse, SH_Pit_SkyRaiderBase, SH_Pit_Caves, SH_Pit_Council, SH_Pit_Cage_Warehouse, SH_Arachnos, SH_Villa_Office, SH_Villa_AbandonedOffice, SH_Villa_Caves, SH_Villa_SkyRaiderBase, SH_Potters_Mausoleum, SH_Potters_CoT, SH_Potters_Caves, SH_Potters_Caves_Easy, 
	EncounterLayouts 
	VisitLocations 
	VillainGroups Arachnos, CircleofThorns, Council, Freakshow, SkyRaiders, ConsortiumGuards, Scrapyarders, SlagGolems, TheFamily,
	//Plaques CoTHistory4, RecluseHistory2, Sharkhead1, Sharkhead2, Sharkhead3, Sharkhead4
}

// Nerva Archipelago
Map V_City_03_02.txt
{
	MissionMinLevel 25
	SpawnOverrideMinLevel 25
	
	MissionDoors Office, AbandonedOffice, Caves, CargoShip, Warehouse, AbandonedWarehouse, Sewers, NA_CC_Crey, NA_CC_Longbow, NA_CC_Office, NA_CC_Sewers, NA_CC_Warehouse, NA_A_Longbow, NA_A_Sewers, NA_P_Caves, NA_P_Portal, NA_TI_CircleOfThorns 
	EncounterLayouts Trio, Encounter, Phalanx, Duo, StrigaPatrol, BarFightOut, Destroy, BackOff, GraveDig, GuardShip, BridgeAssault, DogFight
	VisitLocations 
	VillainGroups Coralax, FreedomCorps, TheFamily, TheLost, LegacyChain, Wyvern, Arachnos, Council, Crey, CircleofThorns, DevouringEarth
	//Plaques PirateHistory2, Nerva1, Nerva2, Nerva3, Nerva4, Nerva5
	ZoneScripts scriptdefs/V_Nerva_Demons.scriptdef
}

// St. Martial
Map V_City_04_01.txt
{
	MissionMinLevel 30
	SpawnOverrideMinLevel 30
	
	PersistentNPCs Arbiter_Hayes, Notoriety_Contact_StMartial, Hard_Luck, Hardcase, Basse_Croupier, Jezebel_Jones, Vivacious_Verandi, Johnny_Sonata, Slot_Machine,
	MissionDoors Office, AbandonedOffice, Caves, CargoShip, Warehouse, AbandonedWarehouse, Sewers, SM_TS_Warehouse, SM_TS_AbandonedWarehouse, SM_TS_AbandonedOffice, SM_TS_Sewers, SM_TS_Caves, SM_TS_Smooth_Caves, SM_TS_Arachnos, SM_R_Office, SM_R_AbandonedOffice, SM_R_Sewer, SM_R_ManHole, SM_R_Caves, SM_R_Lobby, SM_R_Tent, SM_F_Office, SM_F_AbandonedOffice, SM_F_Sewer, SM_F_ManHole, SM_B_Office, SM_B_Sewer, SM_B_Warehouse, 
	//EncounterLayouts Encounter, Duo, Bookie, GuardShip, Lookout, Phalanx, Abduction, SrigaPatrol, Barrel, Around, OilBlaze, Ambush, Caught, ChaseGuy, BarBrawlOut, BreakOut, CrateMove, PurseSnatch, RangedRumble, PlaceBomb, AroundDoor, AroundVandalism,  
	VisitLocations 
	VillainGroups Arachnos, Carnival, CircleofThorns, DevouringEarth, TheFamily, Freakshow, Tsoo, Wailers,
	//Plaques RecluseHistory1, StMartial1, StMartial2, StMartial3, StMartial4
}

// Grandville
Map V_City_05_01.txt
{
	MissionMinLevel 40
	SpawnOverrideMinLevel 40
	
	MissionDoors 
	EncounterLayouts 
	VisitLocations 
	VillainGroups 
	//Plaques RecluseHistory4, RecluseHistory5, Grandville1, Grandville2, Grandville3, Grandville4
}


// Bloody Bay
Map V_PvP_02_01.txt
{
	EntryMinLevel 15
	MissionMinLevel 15
	SpawnOverrideMinLevel 17
		 
	MissionDoors 
	EncounterLayouts  StrigaPatrol, Encounter, TowerTurret, GuardShip, Phalanx, Destroy, GraveDig, BigAround, Bookie, Barrel, Duo 
	VisitLocations 
	VillainGroups  Shivan, CircleofThorns, Freakshow, Arachnos, FreedomCorps
	//Plaques PirateHistory1, BloodyBay1, BloodyBay2, BloodyBay3, BloodyBay4
	ZoneScripts scriptdefs/V_BloodyBayPvP.scriptdef
}

// Siren's Call
Map V_PvP_03_01.txt
{
	EntryMinLevel 20
	MissionMinLevel 20
	SpawnOverrideMinLevel 22
	
	MissionDoors 
	EncounterLayouts Trio, Encounter, Phalanx, Lineup, DogFight, StrigaPatrol, Around, Duo, GuardShip, BarFightOut, Caught, BlockShip, Barrel, BridgeAssault, BackOff, Apprehend, 
	VisitLocations 
	VillainGroups Warriors, Arachnos, FreedomCorps, SkyRaiders, Clockwork, Tsoo
	//Plaques PirateHistory3, SirensCall1, SirensCall2, SirensCall3, SirensCall4
	ZoneScripts scriptdefs/V_SirensCallHunterGame.scriptdef
	ZoneScripts scriptdefs/V_SirensCallPvP.scriptdef
}

// Warburg
Map V_PvP_04_01.txt
{
	EntryMinLevel 30
	MissionMinLevel 30
	SpawnOverrideMinLevel 32
	
	MissionDoors 
	EncounterLayouts 
	VisitLocations 
	VillainGroups 
	//Plaques RecluseHistory3, Warburg1, Warburg2, Warburg3
	ZoneScripts scriptdefs/V_WarburgFootballGame.scriptdef
	ZoneScripts scriptdefs/V_WarburgPvP.scriptdef
}

// Recluse's Victory
Map V_PvP_05_01.txt
{
	EntryMinLevel 41
	MissionMinLevel 41
	SpawnOverrideMinLevel 43
	
	MissionDoors 
	EncounterLayouts 
	VisitLocations 
	VillainGroups 
	//Plaques RecluseVictory1, RecluseVictory2, RecluseVictory3, RecluseVictory3, RecluseVictory4
}
