#ifndef PING_H_WEJQGZT5
#define PING_H_WEJQGZT5

#include <stdio.h>
#include <sys/time.h>    /* struct itimerval,  setitimer()*/
#include <stdbool.h>
#include <errno.h>
#include <float.h>      /* DBL_MAX */
#include <stdlib.h>      /* exit() */
#include <strings.h>     /* bzero() */
#include <sys/socket.h>  /* socket() */
#include <netinet/in.h>  /* IPPROTO_ICMP, sockaddr_in() */
#include <netinet/ip_icmp.h> /* ICMP_ECHOREPLY */
#include <arpa/inet.h>
#include <netdb.h>       /* struct hostent, gethostbyname() */
#include <signal.h>      /* struct sigaction, sigaction() */

#include "util.h"
#include "errors.h"

#ifndef SOC_RCV_MAX_BUF_SIZE
#  define SOC_RCV_MAX_BUF_SIZE 60 * 1024
#endif

#define ICMP_HEADER_LEN 8

typedef int socket_t;
typedef struct sigaction sigaction_t;
typedef struct sockaddr_in sockaddr_in_t;
typedef struct hostent hostent_t;
typedef struct itimerval itimerval_t;
typedef struct timeval timeval_t;

static pid_t pid;
static socket_t sd;

static int repeat_packets_receiving;

static size_t nreceived;
static size_t ntransmitted;

static double rtt_min = DBL_MAX;
static double rtt_max = 0;
static double rtt_sum = 0;
static sockaddr_in_t addr_from, addr_to;

static size_t icmp_data_len = 56;
static uint8_t ttl = 64;

char send_buf[SOC_RCV_MAX_BUF_SIZE];


int ping(const char* hostname);

static void init_signal_handlers();
static void init_interval_timer();

// SIG HANDLER
static void catcher(int signum);

static void finish();
static void pinger();
static int output(char* recv_buf, int msglen, struct timeval* tval);

#endif /* end of include guard: PING_H_WEJQGZT5 */
