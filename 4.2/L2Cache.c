#include "L2Cache.h"


//global variables
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache L1;
L2Cache L2;

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
	if (mode == MODE_WRITE) {
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

	L2.init = 0;
	for (int i = 0; i < L2_NUM_LINES; i++) {
		L2.lines[i].Valid = 0;
		L2.lines[i].Dirty = 0;
		L2.lines[i].Tag = 0;
		for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE) {
			L2.lines[i].Data[j] = 0;
		}
	}
	L2.init = 1;
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
			accessL2(address, Line->Data, MODE_WRITE);	// then write back old block to L2 (write-back policy!)
		}

		// Read new block from memory to cache
		//accessDRAM(MemAddress, Line->Data, MODE_READ); // get new block from DRAM
		accessL2(address, Line->Data, MODE_READ); // get new block from L2
		//FIXME
		Line->Valid = 1;
		Line->Tag = tag;
		Line->Dirty = 0;
	}


	// CACHE HIT (correct index, validity and tag)

	if (mode == MODE_READ) {    // read data from cache line
		memcpy(data, &(Line->Data[offset]), WORD_SIZE);
		time += L1_READ_TIME;
	}

	if (mode == MODE_WRITE) { // write data from cache line
		memcpy(&(Line->Data[offset]), data, WORD_SIZE);
		time += L1_WRITE_TIME;
		Line->Dirty = 1;
	}
}

void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

	uint32_t offset, index, tag, MemAddress;

	/* init cache */
	if (L2.init == 0) {
		initCache();
	}

	// Note:
	// Since our words are 4 bytes long and our cache lines are 16 words long,
	// each L2 cache line will be 16*4 = 64 bytes long.
	// As our L2 cache is byte-addressable, we will need log2(64) = 6 bits for our offset.
	// Furthermore, as our cache size is 512*BLOCK_SIZE, we can infer that our L2 cache has 512 lines.
	// As a result, we will need log2(512) = 9 bits to index our L2 cache.
	// As per the usual cache address composition, the tag will occupy the remaining bits.
	// Our addresses are always 32 bits long, so our tag will occupy the first 32-9-6 = 17 bits.

	tag = address >> 15; // extracting the tag from the address (removing 15 least significant bits)
	uint32_t index_mask = 0x7FC0; // = 0b00000000000000000111111111000000
	index = address && index_mask;
	uint32_t offset_mask = 0x3F; // = 0b00000000000000000000000000111111
	offset = address && offset_mask;


	MemAddress = (address >> 6) << 6; // address of the block in memory
	// (last bits must be 0, no offset needed)
	// creating line pointer
	CacheLine *L2_Line = &L2.lines[index];

	// accessing the L2 cache

	// CACHE MISS (invalid line or incorrect tag)
	if (!L2_Line->Valid || L2_Line->Tag != tag) {

		if ((L2_Line->Valid) && (L2_Line->Dirty)) {	// line has a dirty block - block's been modified (line not empty)
			accessDRAM(MemAddress, L2_Line->Data, MODE_WRITE); // then write back old block (write-back policy!)
		}
		// Read new block from memory to cache
		accessDRAM(MemAddress, L2_Line->Data, MODE_READ); // get new block from DRAM to L2
		//FIXME gotta fetch to L1 as well:
		//how

		L2_Line->Valid = 1;
		L2_Line->Tag = tag;
		L2_Line->Dirty = 0;
	}


	// L2 CACHE HIT (correct index, validity and tag)

	if (mode == MODE_READ) {    // fetch block from l2 to L1
		//data = L1.line.Data here
		memcpy(data, &(L2_Line->Data), WORD_SIZE);
		time += L2_READ_TIME;
	}

	if (mode == MODE_WRITE) { // write data from cache line
		//writing data in L2.line.data
		memcpy(&(L2_Line->Data[offset]), data, WORD_SIZE);
		//FIXME how to copy block to L1
		time += L2_WRITE_TIME;
		L2_Line->Dirty = 1;
	}
}

void read(uint32_t address, uint8_t *data) {
	accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
	accessL1(address, data, MODE_WRITE);
}
