//
// The memory hierarchy uses a write back policy and a LRU block replacement approach.
//

#ifndef L1CACHE_H
#define L1CACHE_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../Cache.h"

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL1(uint32_t, uint8_t *, uint32_t);

typedef struct CacheLine {
	uint8_t Valid;	//bits de validade e tag
	uint8_t Dirty;
	uint32_t Tag;
} CacheLine;

typedef struct L1Cache {
	uint32_t init;
	CacheLine* lines;	//ponteiro para a primeira (must be lista de linhas)
} L1Cache;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);


#endif
