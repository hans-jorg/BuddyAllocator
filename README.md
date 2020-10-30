vectormaps
=============================

## Introduction

This is a text book implementation of a Buddy Allocator using bitmaps. It was first described in [1] and later in [2].

The memory is divided in a number of blocks equal to a power of 2.
As an example, consider a 16 MByte area divided as 16 blocks of 1 MByte each.
The allocator builds a binary tree and allocates contiguous blocks to attend a demand. The demand is rounded to a power of 2: 1024, 2048, 4096, 8192 and 16384.

At the lowest level, there are the nodes with information about the 1024 KBytes blocks. One level above, about the 2048. At the next level, 4096 and so on. At the top level, there is the node with information about the whole memory area

## Using bitmap

Instead of using linked lists, this implementation associates two bit to each node in the binary tree. One bit, called *used*, is set when the correspoding area is allocated. THe other bit, *split*, is set when a block is divided and one subordinated block is allocated.

In this code (*buddy.c*), these bits are stored in two bit vectors: *used* and *split*.

used | split | Description
-----|-------|-------------
  0  |   0   |  Block free at this level and below
  0  |   1   |  Block has been split and at least a block in a level below is been used.
  1  |   0   |  Block used and the size is determined by this level
  1  |   1   |  Should not occur 

The dimension of these bit vectores is 2*N, where N is the number of block at the lowest level.  The depth of the tree is the log_2 N.

The allocation process is O(log_2 N) and does not need to access the free area (avoiding problems in systems with virtual memory).

Just for illustration, the whole information about allocation in the above example is containded in two 32 bit integers.

The routines have the following API:

* buddy_init()
  Initializes the allocation tree

* buddy_alloc(uint32_t size)
  Returns the pointer to a block if one free block can be found. Otherwise, 
  returns NULL

* buddy_free(void *p)
  Returns the pointed block to the free list



## Bit vector manipulation code

In *bitvector.h* there are the routines used to manipulate the bit vectores. They are store as an array of 32 bits unsigned integers.

To easy the declaration of a bit vector, one can use the BV_DECLARE macro. The parameters are the name of bit vector (used to name the array) and its size. The elements of the array are BV_TYPE, defined as uint32_t (but this can be easily changed).

There is a set of inline (mostly) routines to manipulate the bit vectors.

    int bv_index(int bit);
    int bv_bit(int bit);
    BV_TYPE bv_mask(int bit) {
    void bv_set(bv_type v, int bit);
    void bv_clear(bv_type v, int bit);
    BV_TYPE bv_test(bv_type v, int bit);
    void bv_setall(bv_type v, int size);
    void bv_clearall(bv_type v, int size);
    void bv_toggleall(bv_type v, int size);

There is the alternative to the inline routines: use macros. Thery are enable by defining the preprocessor symbol BV_ENABLEMACROS during compilation. They are equivalent to the inline routines with the same name.

    BV_INDEX(BIT)
    BV_BIT(BIT)
    BV_MASK(BIT)
    BV_SET(X,BIT)
    BV_CLEAR(X,BIT)
    BV_TEST(X,BIT)


## References

[1]	Kenneth C. Knowlton. A Fast storage allocator. Communications of the ACM 8(10):623–625, Oct 1965.

[2] Donald E. Knuth. Fundamental Algorithms, Volume 1, The Art of Computer Pro-
gramming, Second Edition . Addison-Wesley, 1973.
