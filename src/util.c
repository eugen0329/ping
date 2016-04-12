#include "util.h"

u_int16_t calc_in_cksum(u_int16_t * addr, int len)
{
    u_int32_t sum = 0;
    u_int16_t result;

    // Add 16-bit words to 32-bit accumulator(sum), then fold back all the carry
    // bits from the top 16-bit into the lower
    while (len > 1) {
        sum += *addr++;
        len -= 2;
    }

    // add an odd byte, if necessary
    if (len == 1)
        sum += *(u_int8_t *) addr;

    sum =  (sum >> 16) + (sum & 0xFFFF); // add hi 16-bits to low 16-bits
    sum += (sum >> 16);                  // add carry

    result = ~sum;                       // trunc to 16-bits
    return result;
}

void sub_tval(struct timeval *out, struct timeval *in)
{
    if ( (out->tv_usec -= in->tv_usec) < 0) {   /* out -= in */
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

