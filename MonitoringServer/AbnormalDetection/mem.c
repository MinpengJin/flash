#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define buf 8*1024*1024

int main()
{
    int i;
    for(i=0;i<40000;i++)
    {
        malloc(buf);
    }
    sleep(40);
    
    return 0;
}
