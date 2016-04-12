#ifndef UTIL_H_4BU2BU9L
#define UTIL_H_4BU2BU9L

#include <sys/time.h>
#include <sys/types.h>

u_int16_t calc_in_cksum(u_int16_t * addr, int len);
void sub_tval(struct timeval *out, struct timeval *in);

#endif /* end of include guard: UTIL_H_4BU2BU9L */
