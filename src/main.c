#include <stdio.h>
#include <stdlib.h>

#define GOOGLE_DNS "8.8.8.8"

int main(int argc, char *argv[])
{
    ping(GOOGLE_DNS);
    return 0;
}
