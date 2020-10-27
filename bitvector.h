#ifndef BITVECTOR_H
#define BITVECTOR_H
/**
 *  @file   bitvector.h
 *
 * @author  Hans
 * @date    21/10/2020
 */


#include <stdint.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/**
 *  @brief  These symbols define the data type used to store the bit vector,
 *          its size and how to get the index part (which element) and bit
 *          part (which bit).
 *
 *  @note   When changing theses symbols, the format specifier in bv_dump
 *          must be adjusted.
 */
///@{
#define BV_TYPE     uint32_t
#define BV_BITS     (32)
#define BV_ONE      (1U)
#define BV_SHIFT    5
#define BV_BITMASK  0x1F
///@}

/**
 *  @brief  data type for parameters
 */
typedef BV_TYPE *bv_type;

/*
 *  @brief  Size of vector in BV_TYPE
 */

#define BV_SIZE(N) (((N)+BV_BITS-1)/BV_BITS)
/**
 *  @brief  Macros used in bit manipulation
 */
///@{
#ifdef BV_ENABLEMACROS

#define BV_INDEX(BIT)       ((BIT)>>BV_SHIFT)
#define BV_BIT(BIT)         ((BIT)&BV_BITMASK)
#define BV_MASK(BIT)        (BV_ONE<<BV_BIT(BIT))

#define BV_SET(X,BIT)       X[BV_INDEX(BIT)] |= (BV_MASK(BIT))

#define BV_CLEAR(X,BIT)     X[BV_INDEX(BIT)] &= ~(BV_MASK(BIT))

#define BV_TEST(X,BIT)     (X[BV_INDEX(BIT])]&(BV_MASK(BIT)))

#endif
///@}

static inline int
bv_index(int bit) {
    return bit>>BV_SHIFT;
}

static inline int
bv_bit(int bit) {
    return bit&BV_BIT(bit);
}

static inline BV_TYPE
bv_mask(int bit) {
    return BV_ONE<<bv_bit(bit);
}


static inline void
bv_set(bv_type v, int bit) {
    v[bv_index(bit)] |= bv_mask(bit);
}

static inline void
bv_clear(bv_type v, int bit) {
    v[bv_index(bit)] &= ~bv_mask(bit);
}

static inline BV_TYPE
bv_test(bv_type v, int bit) {
int i = bv_index(bit);
    return v[i] & bv_mask(bit);
}

static inline void
bv_setall(bv_type v, int size) {
int i;
    for(i=0;i<BV_SIZE(size);i++) {
        v[i] = (unsigned) -1;
    }
}

static inline void
bv_clearall(bv_type v, int size) {
int i;
    for(i=0;i<BV_SIZE(size);i++) {
        v[i] = 0;
    }
}

static inline void
bv_toggleall(bv_type v, int size) {
int i;
    for(i=0;i<BV_SIZE(size);i++) {
        v[i] |= (unsigned) -1;
    }
}

#ifdef DEBUG
#ifdef BV_ENABLEMACROS
#define BV_DUMP(X,SIZE)  bv_dump((X),(SIZE))
#endif

void bv_dump( bv_type x, int size) {
int i;

    for(i=0;i<BV_SIZE(size);i++) {
        printf("%03d: %08X\n",i,(unsigned) x[i]);
    }
}

#endif

/**
 *  @brief  Macro to create a bit vector area
 */

#define BV_DECLARE(X,SIZE) \
        BV_TYPE X[BV_SIZE(SIZE)]


#endif
