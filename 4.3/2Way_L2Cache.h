
#ifndef _2WAY_L2CACHE_H
#define _2WAY_L2CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../Cache.h"

void resetTime();

uint32_t getTime();

#define L2_ASSOCIATIVITY 2
#define L1_NUM_LINES (L1_SIZE / BLOCK_SIZE)
#define L2_NUM_SETS (L2_SIZE / BLOCK_SIZE)/L2_ASSOCIATIVITY

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL1(uint32_t, uint8_t *, uint32_t);
void accessL2(uint32_t, uint8_t *, uint32_t);

typedef struct CacheLine {
	uint8_t Valid;	//bits de validade e tag
	uint8_t Dirty;
	uint32_t Tag;
	uint8_t Data[BLOCK_SIZE];
} CacheLine;

typedef struct CacheSet {
	CacheLine lines[L2_ASSOCIATIVITY];
} CacheSet;

typedef struct L1Cache {
	uint32_t init;
	CacheLine lines[L1_NUM_LINES];	// Num of Lines = L1_SIZE / BLOCK_SIZE
} L1Cache;

typedef struct L2Cache {
	uint32_t init;
	CacheSet sets[L2_NUM_SETS];	// Num of Lines = (L1_SIZE / BLOCK_SIZE) / L2_ASSOCIATIVITY
} L2Cache;


/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);



#endif
