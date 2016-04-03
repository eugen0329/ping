#include <stdio.h>
#include <stdlib.h>

#define GOOGLE_DNS "8.8.8.8"

int main(int argc, char *argv[])
{
    return ping(GOOGLE_DNS);
}
