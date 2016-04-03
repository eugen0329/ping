#ifndef PING_H_WEJQGZT5
#define PING_H_WEJQGZT5

#include <stdio.h>
#include <sys/time.h>    /* struct itimerval,  setitimer()*/
#include <stdbool.h>
#include <stdlib.h>      /* exit() */
#include <strings.h>     /* bzero() */
#include <sys/socket.h>  /* socket() */
#include <netinet/in.h>  /* IPPROTO_ICMP, sockaddr_in() */
#include <netdb.h>       /* struct hostent, gethostbyname() */
#include <signal.h>      /* struct sigaction, sigaction() */

#include "errors.h"

#define SOC_RCV_MAX_BUF_SIZE 60 * 1024
#define DONT_SAVE_OLD_VAL NULL

typedef int socket_t;
typedef struct sigaction sigaction_t;
typedef struct sockaddr_in sockaddr_in_t;
typedef struct hostent hostent_t;
typedef struct itimerval itimerval_t;

int ping(const char* hostname);

static void catcher(int signum);
static void init_signal_handlers();
static void init_interval_timer()

#endif /* end of include guard: PING_H_WEJQGZT5 */
