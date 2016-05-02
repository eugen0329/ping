#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset() */
#include "ping.h"

int main(int argc, char *argv[])
{
    char opt;

    ping_opts_t opts;
    memset(&opts, 0, sizeof(ping_opts_t));

    while((opt = getopt(argc, argv, "t:I:")) != -1) {
        switch (opt) {
            case 't':
                opts.ttl = atoi(optarg);
                break;
            case 'I':
                opts.source_addr = optarg;
                break;
            default:
                return usage();
        }
    }
    if(optind >= argc) {
        return usage();
    }
    opts.dest_addr = &(*argv[optind]);
    printf("opts.dest_addr %s\n", argv[optind]);

    return ping(&opts);
}

int usage()
{
    puts("Usage:\n\tping [-t ttl] [-I interface] destination");
    return -1;
}
