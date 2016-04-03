#include "ping.h"

int ping(const char* ipv4)
{
    socket_t soc;

    if ((soc = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("ping: socket");
        exit(ERR_CREATE_SOCKET);
    }

    if(close(soc) < 0) {
        perror("ping: close");
        exit(ERR_CREATE_SOCKET);
    }

    return 0;
}
