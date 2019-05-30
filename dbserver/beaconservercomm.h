
#ifndef BEACONSERVERCOMM_H

C_DECLARATIONS_BEGIN

U32		beaconServerCount(void);
U32		beaconServerLongestWaitSeconds(void);

U32		beaconClientCount(void);

void	beaconCommInit(void);

void	beaconCommKillAtIP(U32 ip);

C_DECLARATIONS_END

#endif