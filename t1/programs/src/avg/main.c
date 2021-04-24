#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>


int main(int argc, char *argv[])
{
    int sum;
    float avg;
    sum = 0;
    for (int counter = 1; argv[counter] != NULL; ++counter)     {
        sum += atoi(argv[counter]);
    }
    avg = sum/(float)(argc - 1);
    return avg;
}