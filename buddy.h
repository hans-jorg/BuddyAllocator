#ifndef BUDDY_H
#define BUDDY_H
/**
 *  @file   buddy.h
 *
 *  @author Hans
 *  @date   27/10/2020
 */


#include "bitvector.h"

/**
 * Quantity of regions
 */

void  buddy_init_ex(int r);
void *buddy_alloc_ex(int r, unsigned size);
void  buddy_free_ex(int r, void *addr);

static inline void buddy_init(void) { buddy_init_ex(0); }
static inline void *buddy_alloc(unsigned size) { buddy_alloc_ex(0,size); }
static inline void  buddy_free(void *addr) {buddy_free_ex(0,addr); }



#ifdef DEBUG
void buddy_printmap_ex(int r);
void buddy_printaddresses_ex(int r);
static void inline buddy_printmap(void) { buddy_printmap_ex(0); }
static void inline buddy_printaddresses(void) { buddy_printaddresses_ex(0); }
#endif

#endif

