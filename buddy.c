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
 *         2^n - 1 to  2^{n+1}-2
 *
 *  @note
 *    To find the ancestor of a node k, subtract 1 and divide by 2, i.e.
 *             antecessor(k) = {k-1} over {2}
 *
 *  @note
 *    To find the successor of a node k, calculate 2*k+1  and   2*k + 2
 *
 *  @note
 *    All right leaves have even indices and all left leaves are odd.
 *
 *  @note
 *    The allocation is governed by two bits: region[r].used and region[r].split. The region[r].used bit set indicates that
 *    this block is full allocated. The region[r].split bit indicate that it has been region[r].split and allocation
 *    is done further below.
 *
 *    When a block is region[r].used and its buddy too, the parent block region[r].used bit must be set.
 *
 *    When a block is set free and its buddy remains region[r].used, the parent block region[r].used bit must
 *      be cleared.
 *
 *    When a block is set free and its buddy is already free, the parent block region[r].split bit must
 *      be cleared.
 *
 *    By observing the two bits, one can determine its status.
 *
 */

#include <stdint.h>
#ifdef DEBUG
#include <stdio.h>
#include <string.h>
#endif


#include "bitvector.h"
#include "buddy.h"

/**
 *  @brief  Definition of default region (0)
*/
///@{
#define TOTALSIZE       8388608                         ///< Size of region = 8 MB
#define MINSIZE         262144                          ///< Minimal block size = 1 KB
#define MAPSIZE         (TOTALSIZE/MINSIZE)             ///< Number of blocks
#define TREESIZE        (MAPSIZE*2-1)                   ///< Number of elements in the tree
#define BASE            (0xC000000)
///@}

/**
 *  @brief  Number of regions
 */
#define BUDDY_REGIONS_N 4

/**
 *  @brief  Structure to hold information about a region
 */
typedef struct {
    uint32_t    totalsize;              //<
    uint32_t    minsize;                //<
    uint32_t    mapsize;                //<
    uint32_t    treesize;               //<
    BV_TYPE     *used;                   //<
    BV_TYPE     *split;                  //<
    void        *base;                  //<
    char        *map;                   //<
} BUDDY_REGION;


/**
 * Bit vectors region[r].used for the default region
 */
///@{
BV_DECLARE(used,TREESIZE);                       ///< region[r].used/free map
BV_DECLARE(split,TREESIZE);                      ///< already region[r].split
///@}

///@{
char map[MAPSIZE];
///@}
/**
 *  @brief  Pools info
    uint32_t    minsize;
    uint32_t    totalsize;
    uint32_t    mapsize;
    uint32_t    treesize;
    BV_TYPE     used;
    BV_TYPE     split;
    void        *base;
    char        *map;
 */

BUDDY_REGION region[BUDDY_REGIONS_N] = {
    /* 0 */  { TOTALSIZE, MINSIZE, MAPSIZE, TREESIZE, used, split, (void *) BASE , map },
    /* 1 */  { 0 },
    /* 2 */  { 0 },
    /* 3 */  { 0 }
};

/**
 *  @brief  Structure region[r].used to navigate the allocation tree
 */

typedef struct {
    int         level;      ///< level of node
    int         index;      ///< index of node
    int         size;       ///< size of block
    uint32_t    addr;       ///< address of block
} nodeinfo;


/**
 *  @brief  buddy_init
 */
void
buddy_config(int r, uint32_t totalsize, uint32_t minsize, void *base,
                    BV_TYPE *used, BV_TYPE *split, char *map) {

    if( r >= BUDDY_REGIONS_N )
        return;
#define TOTALSIZE       8388608                         ///< Size of region = 8 MB
#define MINSIZE         262144                          ///< Minimal block size = 1 KB
#define MAPSIZE         (TOTALSIZE/MINSIZE)             ///< Number of blocks
#define TREESIZE        (MAPSIZE*2-1)                   ///< Number of elements in the tree
#define BASE            (0xC000000)

    region[r].totalsize = totalsize;
    region[r].minsize   = minsize;
    region[r].mapsize   = totalsize/minsize;
    region[r].treesize  = region[r].mapsize*2-1;

    region[r].used      = used;
    region[r].split     = split;

    region[r].base      = base;
    region[r].map       = map;
}


/**
 *  @brief  buddy_init
 */
void
buddy_init_ex(int r) {

    if( r >= BUDDY_REGIONS_N )
        return;
    bv_clearall(region[r].used,region[r].treesize);
    bv_clearall(region[r].split,region[r].treesize);
}

/**
 *  @brief  buddy_alloc
 */
void *
buddy_alloc_ex(int r, unsigned size) {
int level;
int s;
int k;
int l;
uint32_t a;

nodeinfo stack[region[r].mapsize];
int sp;
nodeinfo node;

    if( r >= BUDDY_REGIONS_N )
        return 0;

    // Too big?
    if( size > region[r].totalsize )
        return 0;

    // Already full
    if( bv_test(region[r].used,0) )
        return 0;

    sp = 0;
    stack[sp].level = 0;
    stack[sp].index = 0;
    stack[sp].size = region[r].totalsize;
    stack[sp].addr = 0;
    sp++;

    while( sp > 0 ) {
        sp--;
        node = stack[sp];
        k = node.index;
        s = node.size;
        a = node.addr;
        l = node.level;

        // test if block already region[r].used
        if( bv_test(region[r].used,k) )
            continue;
        // test if need full block
        if( (size > s/2) || (s == region[r].minsize) ) {
            // if already region[r].split, try another block
            if( bv_test(region[r].split,k) == 0 ) {
                // reserve it
                bv_set(region[r].used,k);
                return (void *) ((char *) region[r].base+a);
            }
        }
        s /= 2;
        if( size > s )
            continue;

        // Mark as region[r].split
        bv_set(region[r].split,k);
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
void buddy_free_ex(int r, void *addr) {
uint32_t disp = (char *) addr - (char *)region[r].base;       // 4 GB limit
int b,d,k,p;

    if( r >= BUDDY_REGIONS_N )
        return;

    d = disp/region[r].minsize;

    k = region[r].mapsize+d-1;
    // Free if it is not
    bv_clear(region[r].used,k);
    bv_clear(region[r].split,k);
    // Find block to be freed
    while( k > 0 ) {
            k /= 2;
        if( bv_test(region[r].used,k) ) {
            bv_clear(region[r].used,k);
            bv_clear(region[r].split,k);
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
        if( (bv_test(region[r].used,k)==0)&&(bv_test(region[r].used,b)==0)&&(bv_test(region[r].split,k)==0)&&(bv_test(region[r].split,b))) {
            p = k/2;
            bv_clear(region[r].split,p);
        }
        k /= 2;
    }
}



#ifdef DEBUG

/**
 *  @brief  fillmap
 */
void
fillmap_ex(int r, int start, int end, char c) {
int i;

int b,d,k,p;

    if( r >= BUDDY_REGIONS_N )
        return;

    if( !region[r].map )
        return;

    for(i=start;i<end;i++) {
        if( c == '-' || region[r].map[i] == '-' )
            region[r].map[i] = c;
        else
            region[r].map[i] = '*';
    }

}


/**
 *  @brief  buildmap
 */
void
buildmap_ex(int r) {
int level;
int s;
int k;
int l;
uint32_t a;
nodeinfo stack[region[r].mapsize];                  // C99 Only
int sp;
nodeinfo node;

    if( r >= BUDDY_REGIONS_N )
        return;

    if( !region[r].map )
        return;

    fillmap_ex(r,0,region[r].mapsize,'-');

    sp = 0;
    stack[sp].level = 0;
    stack[sp].index = 0;
    stack[sp].size = region[r].totalsize/region[r].minsize;
    stack[sp].addr = 0;
    sp++;

    while(sp>0) {
        sp--;
        node = stack[sp];
        k = node.index;
        s = node.size;
        a = node.addr;
        l = node.level;
        // test if block already region[r].used
        if( bv_test(region[r].used,k) ) {
            fillmap_ex(r,a,a+s,'U');
        }

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

    region[r].map[region[r].mapsize] = '\0';
}


/**
 *  @brief  print allocation map
 */
void
buddy_printmap_ex(int r) {
//char map[region[r].mapsize+1];          // Only C99

    if( r >= BUDDY_REGIONS_N )
        return;
    buildmap_ex(r);
    printf("|%s|\n",region[r].map);
}



void buddy_printaddresses_ex(int r) {
int level;
int k;
int lim;
uint32_t addr;
uint32_t size;
int delta;

    if( r >= BUDDY_REGIONS_N )
        return;

    level = 0;
    size = region[r].totalsize;
    lim = 0;
    addr = 0;
    delta = 1;
    for(k=0;k<region[r].treesize;k++) {
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

