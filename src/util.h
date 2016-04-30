#ifndef UTIL_H_4BU2BU9L
#define UTIL_H_4BU2BU9L

#include <sys/time.h>
#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define DONT_SAVE_OLD_VAL NULL
#define EMPTY_FLAGS 0
// See `man gettimeofday` NOTES
#define OBSOLETE_TIMEZONE_ARGUMENT NULL

u_int16_t calc_in_cksum(u_int16_t * addr, int len);
void sub_tval(struct timeval *out, struct timeval *in);
void self_ip(struct sockaddr_in* addr);

#endif /* end of include guard: UTIL_H_4BU2BU9L */
