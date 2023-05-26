#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>
#include <stddef.h>
#include "icsmm.h"



/* Helper function declarations go here */


void toggle_allocated_bit(void *block_header_or_footer);
bool isAligned(void* ptr, int alignmentNum);
size_t getAlignedPayloadSize(size_t size);
void* prepareNewPage(ics_free_header **freelist_head, ics_free_header **freelist_next);
void removeFromFreeList(ics_free_header **freelist_head, ics_free_header *block_to_remove);
void* getNextFit(size_t size, ics_free_header **freelist_head, ics_free_header **freelist_next);
void insertIntoFreeList(ics_free_header **freelist_head, ics_free_header *block);
void splitAndPrepFreeBlock(size_t size, ics_free_header* bigFreeHeader);




#endif
