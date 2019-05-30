/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef STATAGG_H__
#define STATAGG_H__

C_DECLARATIONS_BEGIN

void stat_ReadTable(void);
	// Call once at startup to set up initial state of stats

void stat_Update(void);
	// Call in a loop, every once and a while it will update all the stats
	// and broadcast the current tables to all servers.

void stat_UpdateStatsForEnt(int dbid, char *pchEnt);
	// Update all the stats which appear in the given text ent definition.

C_DECLARATIONS_END

#endif /* #ifndef STATAGG_H__ */

/* End of File */

