#ifndef BEACONSERVERCOMM_H
#define BEACONSERVERCOMM_H

#ifdef __cplusplus
extern "C" {
#endif

U32        beaconServerCount(void);
U32        beaconServerLongestWaitSeconds(void);

U32        beaconClientCount(void);

void    beaconCommInit(void);

void    beaconCommKillAtIP(U32 ip);

#ifdef __cplusplus
}
#endif

#endif