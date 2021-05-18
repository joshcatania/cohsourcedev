#ifndef _HTTP_H
#define _HTTP_H

typedef struct ServerAPIConfig ServerAPIConfig;

void startHttp(ServerAPIConfig* config);
void stopHttp(ServerAPIConfig* config);

#endif
