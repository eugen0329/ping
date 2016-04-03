#ifndef PING_H_WEJQGZT5
#define PING_H_WEJQGZT5

#include <stdio.h>
#include <stdlib.h>      /* exit() */
#include <sys/socket.h>  /* socket() */
#include <netinet/in.h>  /* IPPROTO_ICMP */
#include "errors.h"

typedef int socket_t;

int ping(const char* ipv4);

#endif /* end of include guard: PING_H_WEJQGZT5 */
