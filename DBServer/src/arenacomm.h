#ifndef ARENACOMM_H
#define ARENACOMM_H

C_DECLARATIONS_BEGIN

void arenaCommInit(void);
int arenaServerCount(void);
void handleRequestArena(Packet* pak,NetLink* link);
int arenaServerSecondsSinceUpdate(void);

C_DECLARATIONS_END

#endif // ARENACOMM_H