#include "L1Cache.h"

//global variables
uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache L1Cache;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {
//accessing RAM memory?
	if (address >= DRAM_SIZE - WORD_SIZE + 1)
		exit(-1);

	//read/fetch a block from memory RAM
	if (mode == MODE_READ) {
		memcpy(data, &(DRAM[address]), BLOCK_SIZE);
		time += DRAM_READ_TIME;
	}
	//write a block in memory RAM
	if (mode == MODE_WRITE) {
		memcpy(&(DRAM[address]), data, BLOCK_SIZE);
		time += DRAM_WRITE_TIME;
	}
}

/*********************** L1 cache *************************/

void initCache() { SimpleCache.init = 0; }

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

	uint32_t Index, Tag, MemAddress;
	uint8_t TempBlock[BLOCK_SIZE];

	/* init cache */
	if (L1Cache.init == 0) {
		L1Cache.line.Valid = 0;
		L1Cache.init = 1;
	}

	CacheLine *Line = &L1Cache.line;

	Tag = address >> 14; // Extract tag
	Index = ((address << 18) >> 14); // Extract index (8 bits)
	// Don't I need offset? Offset = ((address << 26) >> 26)

	MemAddress = (address >> 6) << 6; // address of the block in memory
									  // (last bits must be 0, no offset needed)

	/* access Cache*/

	//if line is not valid or tag is not equal to tag of address (cache miss) aka
	if (!Line->Valid || Line->Tag != Tag) {         // if block not present
		accessDRAM(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

		if ((Line->Valid) && (Line->Dirty)) {	// line has a dirty block - block's been modified (line not empty)
			MemAddress = Line->Tag << 3;        // get address of the block in memory
			accessDRAM(MemAddress, &(L1Cache[0]), MODE_WRITE); // then write back old block
		}
		//put desired block in cache
		memcpy(&(L1Cache[0]), TempBlock,
			   BLOCK_SIZE); // copy new block to cache line
		Line->Valid = 1;
		Line->Tag = Tag;
		Line->Dirty = 0;
	} // if miss, then replaced with the correct block


	/*if cache hit (correct index, validity and tag)*/

	if (mode == MODE_READ) {    // read data from cache line
		if (0 == (address % 8)) { // even word on block
			memcpy(data, &(L1Cache[0]), WORD_SIZE);
		} else { // odd word on block	//WHAT
			memcpy(data, &(L1Cache[WORD_SIZE]), WORD_SIZE);
		}
		time += L1_READ_TIME;
	}

	if (mode == MODE_WRITE) { // write data from cache line
		if (!(address % 8)) {   // even word on block
			memcpy(&(L1Cache[0]), data, WORD_SIZE);
		} else { // odd word on block
			memcpy(&(L1Cache[WORD_SIZE]), data, WORD_SIZE);
		}
		time += L1_WRITE_TIME;
		Line->Dirty = 1;
	}
}

void read(uint32_t address, uint8_t *data) {
	accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
	accessL1(address, data, MODE_WRITE);
}
