#include "L1Cache.h"

//global variables
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache L1;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {
	if (address >= DRAM_SIZE - WORD_SIZE + 1)
		exit(-1);

	// read/fetch a block from memory RAM
	if (mode == MODE_READ) {
		memcpy(data, &(DRAM[address]), BLOCK_SIZE);
		time += DRAM_READ_TIME;
	}
	// write a block in memory RAM
	else if (mode == MODE_WRITE) {
		memcpy(&(DRAM[address]), data, BLOCK_SIZE);
		time += DRAM_WRITE_TIME;
	}
}

/*********************** L1 cache *************************/

void initCache() {
	L1.init = 0;
	for (int i = 0; i < L1_NUM_LINES; i++) {
		L1.lines[i].Valid = 0;
		L1.lines[i].Dirty = 0;
		L1.lines[i].Tag = 0;
		for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE) {
			L1.lines[i].Data[j] = 0;
		}
	}
	L1.init = 1;
}

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

	uint32_t offset, index, tag, MemAddress;

	/* init cache */
	if (L1.init == 0) {
		initCache();
	}

	// Note:
	// Since our words are 4 bytes long and our cache lines are 16 words long,
	// each L1 cache line will be 16*4 = 64 bytes long.
	// As our L1 cache is byte-addressable, we will need log2(64) = 6 bits for our offset.
	// Furthermore, as our cache size is 256*BLOCK_SIZE, we can infer that our L1 cache has 256 lines.
	// As a result, we will need log2(256) = 8 bits to index our L1 cache.
	// As per the usual cache address composition, the tag will occupy the remaining bits.
	// Our addresses are always 32 bits long, so our tag will occupy the first 32-8-6 = 18 bits.

	tag = address >> 14; // extracting the tag from the address (removing 14 least significant bits)
	uint32_t index_mask = 0x3FC0; // = 0b00000000000000000011111111000000
	index = address && index_mask;
	uint32_t offset_mask = 0x3F; // = 0b00000000000000000000000000111111
	offset = address && offset_mask;


	MemAddress = (address >> 6) << 6; // address of the block in memory
									  // (last bits must be 0, no offset needed)
	// creating line pointer
	CacheLine *Line = &L1.lines[index];

	// accessing the L1 cache

	// CACHE MISS (invalid line or incorrect tag)
	if (!Line->Valid || Line->Tag != tag) {

		if ((Line->Valid) && (Line->Dirty)) {	// line has a dirty block - block's been modified (line not empty)
			accessDRAM(MemAddress, Line->Data, MODE_WRITE); // then write back old block (write-back policy!)
		}

		// Read new block from memory to cache
		accessDRAM(MemAddress, Line->Data, MODE_READ); // get new block from DRAM

		Line->Valid = 1;
		Line->Tag = tag;
		Line->Dirty = 0;
	}


	// CACHE HIT (correct index, validity and tag)

	if (mode == MODE_READ) {    // read data from cache line
		memcpy(data, &(Line->Data[offset]), WORD_SIZE);
		time += L1_READ_TIME;
	}
	else if (mode == MODE_WRITE) { // write data from cache line
		memcpy(&(Line->Data[offset]), data, WORD_SIZE);
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
