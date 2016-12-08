#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main()
{
    int c;
    FILE * fp;

    fp = fopen("/proc/psinfo", "r");
    assert(fp != NULL);

    while ((c = fgetc(fp)) != EOF)
    {
        putchar(c);
    }

    fclose(fp);
    return 0;
}
