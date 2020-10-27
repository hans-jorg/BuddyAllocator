/**
 *  @file   buddy.c
 *
 *  @note   Memory allocator using buddy allocator with bit vectors
 *
 *
 *  Level   |    Indices
 *  --------|---------------------
 *     0    |    0
 *     1    |    1-2
 *     2    |    3-4 * 5-6
 *     3    |    7-8 * 9-10 * 11-12 * 13-14
 *     4    |   15-16 * 17-18 * 19-20 * 21-22 * 23-24 * 25-26 * 27-28 * 29-30
 *
 *  @note
 *    All blocks at a level n can be found between in the range
 *       \f$ 2^n-1 \f$ to \f$ 2^{n+1}-2 \f$
 *
 *  @note
 *    To find the ancestor of a node k, subtract 1 and divide by 2, i.e.
 *       \f[
 *           antecessor(k) = \frac{k-1}{2}
 *       \f]
 *
 *  @note
 *    To find the successor of a node k, calculate \f$ 2*k+1 \f$ and  \f$ 2*k + 2 \f$
 *
 *  @note
 *    All right leaves have even indices and all left leaves are odd.
 *
 *  @note
 *    The allocation is governed by two bits: used and split. The used bit set indicates that
 *    this block is full allocated. The split bit indicate that it has been split and allocation
 *    is done further below.
 *
 *    When a block is used and its buddy too, the parent block used bit must be set.
 *
 *    When a block is set free and its buddy remains used, the parent block used bit must
 *      be cleared.
 *
 *    When a block is set free and its buddy is already free, the parent block split bit must
 *      be cleared.
 *
 *    By observing the two bits, one can determine its status.
 *
 */

#if defined(TEST) && !defined(DEBUG)
#define DEBUG
#endif

#include <stdint.h>
#ifdef DEBUG
#include <stdio.h>
#include <string.h>
#endif


#include "bitvector.h"


/**
 *  @brief  Size definition
*/
///@{
#define TOTALSIZE   16384
#define MINSIZE     1024
#define BASE        0x1000000

#define MAPSIZE        (TOTALSIZE/MINSIZE)

const int msize = MAPSIZE;              ///< const can not be used for array dimensions
///@}

/**
 *
 */
///@{
BV_DECLARE(used,MAPSIZE*2);                  ///< used/free map
BV_DECLARE(split,MAPSIZE*2);                 ///< already split
///@}

/**
 *  @brief  Structure used to navigate the allocation tree
 */

typedef struct {
    int         level;
    int         index;
    int         size;
    uint32_t    addr;
} nodeinfo;


/**
 *  @brief  buddy_init
 */
void
buddy_init(void) {

#ifdef DEBUG
    printf("Initialization for %d blocks\n",MAPSIZE);
#endif

    bv_clearall(used,MAPSIZE*2);
    bv_clearall(split,MAPSIZE*2);
}

/**
 *  @brief  buddy_alloc
 */
unsigned
buddy_alloc(unsigned size) {
int level;
int s;
int k;
int l;
uint32_t a;

nodeinfo stack[MAPSIZE];
int sp;
nodeinfo node;

#ifdef DEBUG
    printf("Allocating an area for %d bytes\n",size);
#endif
    // Too big?
    if( size > TOTALSIZE )
        return 0;

    // Already full
    if( bv_test(used,0) )
        return 0;

    sp = 0;
    stack[sp].level = 0;
    stack[sp].index = 0;
    stack[sp].size = TOTALSIZE;
    stack[sp].addr = 0;
    sp++;

    while( sp > 0 ) {
        sp--;
        node = stack[sp];
        k = node.index;
        s = node.size;
        a = node.addr;
        l = node.level;
#ifdef DEBUG
        printf("Trying block #%1d at level %d with size %d and address %X\n",
                k,l,s,a);
#endif

        // test if block already used
        if( bv_test(used,k) )
            continue;
        // test if need full block
        if( (size > s/2) || (s == MINSIZE) ) {
            // if already split, try another block
            if( bv_test(split,k) == 0 ) {
                // reserve it
                bv_set(used,k);
#ifdef DEBUG
                printf("Allocated block #%1d at %X\n",k,a);
#endif
                return BASE+a;
            }
        }
        s /= 2;
        if( size > s )
            continue;

        // Mark as split
        bv_set(split,k);
        // Try left and right leaves.
        l++;
        //Left must be on top of stack
        stack[sp].index = 2*k+2;
        stack[sp].addr  = a+s;
        stack[sp].size  = s;
        stack[sp].level = l;
        sp++;
        stack[sp].index = 2*k+1;
        stack[sp].addr  = a;
        stack[sp].size  = s;
        stack[sp].level = l;
        sp++;
    }
    return 0;
}

static inline int isodd(int n) { return n&1; }
static inline int iseven(int n) { return (n&1)^1; }

/**
 *  @brief  buddy_free
 */
void buddy_free(uint32_t addr) {
uint32_t disp = addr - BASE;
int b,d,k,p;

#ifdef DEBUG
    printf("Freeing block at %08X\n",addr);
#endif
    d = disp/MINSIZE;

    k = MAPSIZE+d-1;
#ifdef DEBUG
    printf("Found at low level block #%1d\n",k);
#endif
    // Free if it is not
    bv_clear(used,k);
    bv_clear(split,k);
    // Find block to be freed
    while( k > 0 ) {
            k /= 2;
#ifdef DEBUG
            printf("Searching up block #%1d\n",k);
#endif
        if( bv_test(used,k) ) {
#ifdef DEBUG
            printf("Freeing block #%1d\n",k);
#endif
            bv_clear(used,k);
            bv_clear(split,k);
            break;
        }
    }
    // Adjust parents
    while( k > 0 ) {
        // find buddy

        if( isodd(k) )
            b = k+1;
        else
            b = k-1;
#ifdef DEBUG1
        printf("Found buddy at block #%-1d\n",b);
#endif
        if( (bv_test(used,k)==0)&&(bv_test(used,b)==0)&&(bv_test(split,k)==0)&&(bv_test(split,b))) {
            p = k/2;
            bv_clear(split,p);
        }
        k /= 2;
    }
}



#ifdef DEBUG

/**
 *  @brief  fillmap
 */
void
fillmap(char *m,int start, int end, char c) {
int i;

    for(i=start;i<end;i++) {
        if( c == '-' || m[i] == '-' )
            m[i] = c;
        else
            m[i] = '*';
    }

}


/**
 *  @brief  buildmap
 */
void
buildmap(char *m) {
int level;
int s;
int k;
int l;
uint32_t a;
nodeinfo stack[MAPSIZE];
int sp;
nodeinfo node;

    fillmap(m,0,MAPSIZE,'-');

    sp = 0;
    stack[sp].level = 0;
    stack[sp].index = 0;
    stack[sp].size = TOTALSIZE/MINSIZE;
    stack[sp].addr = 0;
    sp++;

    while(sp>0) {
        sp--;
        node = stack[sp];
        k = node.index;
        s = node.size;
        a = node.addr;
        l = node.level;
#ifdef DEBUG1
        printf("Mapping block #%1d at level %d with size %d and address 0x%X",
                k,l,s,a);
#endif
        // test if block already used
        if( bv_test(used,k) ) {
#ifdef DEBUG1
            printf("--> USED");
#endif
            fillmap(m,a,a+s,'U');
        }
#ifdef DEBUG1
        putchar('\n');
#endif

        if( s == 1 )
            continue;

        s /= 2;
        // Try left and right leaves.
        l++;
        //Left must be on top of stack
        stack[sp].index = 2*k+2;
        stack[sp].addr  = a+s;
        stack[sp].size  = s;
        stack[sp].level = l;
        sp++;
        stack[sp].index = 2*k+1;
        stack[sp].addr  = a;
        stack[sp].size  = s;
        stack[sp].level = l;
        sp++;

    }

    m[MAPSIZE] = '\0';
}


/**
 *  @brief  print allocation map
 */
void buddy_printmap(void) {
char map[MAPSIZE+1];
    buildmap(map);
    printf("|%s|\n",map);
}



void buddy_printaddresses(void) {
int level;
int k;
int lim;
uint32_t addr;
uint32_t size;
int delta;

    level = 0;
    size = TOTALSIZE;
    lim = 0;
    addr = 0;
    delta = 1;
    for(k=0;k<MAPSIZE*2-1;k++) {
        printf("level = %-2d node = %-3d address = %08X  size=%08X\n",level,k,addr,size);
        if( k == lim ) {
            level++;
            delta *= 2;
            lim += delta;
            addr = 0;
            size /= 2;
            putchar('\n');
        } else {
            addr += size;
        }
    }
}

#endif

#ifdef TEST

/**
 *  @brief  test program
 */
int
main (int argc, char *argv[])
{
unsigned a;
unsigned f1,f2,f3,f4;


    printf("Total size = %d (%X)\n",TOTALSIZE,TOTALSIZE);
    printf("Minimal size = %d (%X)\n",MINSIZE,MINSIZE);

    printf("\nAddresses\n");
    buddy_printaddresses();

    printf("\nMap\n");
    buddy_init();
    buddy_printmap();

    a = buddy_alloc(33000);
    printf("a=%08X\n",a);
    buddy_printmap();

    a = buddy_alloc(15000);
    printf("a=%08X\n",a);
    buddy_printmap();

#if 1
    buddy_free(a);
    buddy_printmap();
#else
    printf("\nResetting...\n");
    buddy_init();
#endif

    a = buddy_alloc(4000);
    printf("a=%08X\n",a);
    buddy_printmap();
    f1 = a;

    a = buddy_alloc(1000);
    printf("a=%08X\n",a);
    buddy_printmap();
    f3 = a;

    a = buddy_alloc(1000);
    printf("a=%08X\n",a);
    buddy_printmap();
    f4 = a;

    a = buddy_alloc(1000);
    printf("a=%08X\n",a);
    buddy_printmap();
    f2 = a;

    a = buddy_alloc(4000);
    printf("a=%08X\n",a);
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

#endif

