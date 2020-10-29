/**
 *  @file  testbuddy.c
 */

#include <stdio.h>
#include <string.h>

#include "buddy.h"

/**
 *  @brief  test program
 */
int
main (int argc, char *argv[])
{
char * a;
char *f1,*f2,*f3,*f4;
unsigned size;

    printf("\nAddresses\n");
    buddy_printaddresses();

    printf("\nMap\n");
    buddy_init();
    buddy_printmap();

    size = 30000;
    printf("Allocating %u\n",size);
    a = buddy_alloc(size);
    printf("a=%p\n",a);
    buddy_printmap();

    size = 15000;
    printf("Allocating %u\n",size);
    a = buddy_alloc(size);
    printf("a=%p\n",a);
    buddy_printmap();

#if 1
    printf("Freeing block #0\n");
    buddy_free(a);
    buddy_printmap();
#else
    printf("\nResetting...\n");
    buddy_init();
#endif

    size = 4000;
    printf("Allocating %u\n",size);
    a = buddy_alloc(size);
    printf("a=%p\n",a);
    buddy_printmap();
    f1 = a;

    size = 1000;
    printf("Allocating %u\n",size);
    a = buddy_alloc(size);
    printf("a=%p\n",a);
    buddy_printmap();
    f3 = a;

    size = 1000;
    printf("Allocating %u\n",size);
    a = buddy_alloc(size);
    printf("a=%p\n",a);
    buddy_printmap();
    f4 = a;

    size = 1000;
    printf("Allocating %u\n",size);
    a = buddy_alloc(size);
    printf("a=%p\n",a);
    buddy_printmap();
    f2 = a;

    size = 4000;
    printf("Allocating %u\n",size);
    a = buddy_alloc(size);
    printf("a=%p\n",a);
    buddy_printmap();

    printf("\nFreeing...\n");

    buddy_free(a);
    buddy_printmap();

    buddy_free(f1);
    buddy_printmap();

    buddy_free(f2);
    buddy_printmap();

    buddy_free(f3);
    buddy_printmap();

    buddy_free(f4);
    buddy_printmap();

    return 0;
}

