#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if(argc != 2) {
        fprintf(stderr, "USAGE:\n\t%s destination\n", argv[0]);
        return 0;
    }
    return ping(argv[1]);
}
