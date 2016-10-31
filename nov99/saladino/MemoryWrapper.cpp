#include <stdlib.h>
#include <memory.h>

// Defines
#define HASH_TABLE_SIZE 499

#define UNINITIALIZED 0xBEEFBEEF
#define FREED 0xDEDEDEDE

#define BLOCK_PREFIX_ALLOC 0xABCDABCD
#define BLOCK_PREFIX_FREE 0xBADABADA
#define BLOCK_SUFFIX 0xFEEDFEED

// Enumerated Types
enum BlockErrors {
	BE_NONE,
	BE_BLOCK_POINTER_NULL,
	BE_CORRUPT_HEADER,
	BE_CORRUPT_TRAILER,
	BE_ALREADY_FREED,
	BE_WRITTEN_TO_AFTER_FREED,
	BE_BLOCK_DOES_NOT_EXIST,
	BE_NUM_BLOCK_ERRORS
};	

// Structures
struct BlockHeader {
	BlockHeader *next;
	char *file;
	int line;
	int size;
	unsigned int tag;
};

struct BlockTrailer {
	unsigned int tag;
};

// Classes
class MemoryManager {
public:
	void *Allocate(unsigned int size,char *file,int line);
	void Deallocate(void *pointer);
	void ValidatePointer(void *pointer);
	void ValidateAllBlocks();
	int FindMemoryLeaks();
};

// Varaibles
static BlockHeader *Block_hash_table[HASH_TABLE_SIZE];

// Internal functions
static inline int Hash(void *pointer)
{
	return ((unsigned int)pointer>>3)%HASH_TABLE_SIZE;
}

static BlockErrors VerifyBlock(char *pointer)
{
	if(!pointer) {
		return BE_BLOCK_POINTER_NULL;
	}
	
	BlockHeader *header=(BlockHeader *)(pointer-sizeof(BlockHeader));
	
	if(header->tag==BLOCK_PREFIX_FREE) {
		return BE_ALREADY_FREED;
	}

	if(header->tag!=BLOCK_PREFIX_ALLOC) {
		return BE_CORRUPT_HEADER;
	}
		
	BlockTrailer *trailer=(BlockTrailer *)(pointer+header->size);
	if(trailer->tag!=BLOCK_SUFFIX) {
		return BE_CORRUPT_TRAILER;
	}

	return BE_NONE;
}

// External functions
void *MemoryManager::Allocate(unsigned int size,char *file,int line)
{
	// System level allocation
	char *pointer=(char *)malloc(size+sizeof(BlockHeader)+sizeof(BlockTrailer));
	if(!pointer) {
		// System level allocation failed
		return NULL;
	}

	// Clear memory block
	memset(pointer+sizeof(BlockHeader),UNINITIALIZED,size);

	// Fill in header
	BlockHeader *header=(BlockHeader *)pointer;
	header->file=file;
	header->line=line;
	header->size=size;
	header->tag=BLOCK_PREFIX_ALLOC;

	// Fill in trailer
	BlockTrailer *trailer=(BlockTrailer *)(pointer+size+sizeof(BlockHeader));
	trailer->tag=BLOCK_SUFFIX;

	// Put block into hash table
	int hash=Hash(header);
	header->next=Block_hash_table[hash];
	Block_hash_table[hash]=header;

	return pointer+sizeof(BlockHeader);
}

void MemoryManager::Deallocate(void *pointer)
{
	BlockErrors error=VerifyBlock((char*)pointer);
	if(error==BE_BLOCK_POINTER_NULL) {
		// Message that we got sent a NULL pointer
		return;
	} else if(error!=BE_NONE) {
		// Message that this block has an error
		return;
	}

	BlockHeader *header=(BlockHeader*)((char*)pointer-sizeof(BlockHeader));

	// Locate block in hash table
	int hash=Hash(header);
	BlockHeader *cur_header=Block_hash_table[hash];
	BlockHeader *prev_header=NULL;
	while(cur_header) {
		if(cur_header==header) {
			break;
		}
		prev_header=cur_header;
		cur_header=cur_header->next;	
	}

	if(!cur_header) {
		// Message that this block does not exist
		return;
	}

	// Mark as freed
	memset(pointer,header->size,FREED);
  header->tag=BLOCK_PREFIX_FREE;

	// System level deallocation
	free((char*)pointer-sizeof(BlockHeader));
}

//--------------------------------------------------

void MemoryManager::ValidatePointer(void *pointer)
{
	BlockErrors error=VerifyBlock((char *)pointer);
	if(error==BE_NONE) {
		BlockHeader *header=(BlockHeader *)((char *)pointer-sizeof(BlockHeader));

		// Make sure that this block exists in our hash table
		int hash_index=Hash(header);
		BlockHeader *cur_header=Block_hash_table[hash_index];
		while(cur_header) {
			if(cur_header==header) {
				break;
			}
			cur_header=cur_header->next;
		}

		if(cur_header==NULL) {
			// Message that this block does not exist
		}
	}

	// Message that this block has an error
}

//--------------------------------------------------

void MemoryManager::ValidateAllBlocks()
{
	// Walk through all allocated blocks
	for(int i=0; i<HASH_TABLE_SIZE; i++) {
		BlockHeader *cur_header=Block_hash_table[i];
		while(cur_header) {
			BlockErrors error=VerifyBlock((char *)cur_header+sizeof(BlockHeader));
			if(error!=BE_NONE) {
				// Message that bad things have occured
			}
			cur_header=cur_header->next;
		}
	}
}

//--------------------------------------------------

int MemoryManager::FindMemoryLeaks()
{
	int bytes_leaked=0;
	int leak_count=0;

	// Walk through all allocated blocks
	for(int i=0; i<HASH_TABLE_SIZE; i++) {
		BlockHeader *cur_header=Block_hash_table[i];
		while(cur_header) {
			if(cur_header->tag==BLOCK_PREFIX_ALLOC) {
				// Message the memory leak's file and line number

				bytes_leaked+=cur_header->size;
				leak_count++;
			}
			cur_header=cur_header->next;
		}
	}

	if(leak_count==0) {
		// Message no memory leaks
		// Congradulations
	}

	return leak_count;
}
