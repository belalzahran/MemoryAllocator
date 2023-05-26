#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>
#include <stddef.h>
#include "icsmm.h"



/* Helper function declarations go here */



bool isAligned(void* ptr, int alignmentNum);


size_t getAlignedPayloadSize(size_t size);


void* prepareNewPage(ics_free_header **freelist_head, ics_free_header **freelist_next);



void removeFromFreeList(ics_free_header **freelist_head, ics_free_header *block_to_remove);

// start searching at next
void* getNextFit(size_t size, ics_free_header **freelist_head, ics_free_header **freelist_next);



// inserting in address order
// loop should never reach nulll (loop back)
 // make sure that last node in free list points back to head
void insertIntoFreeList(ics_free_header **freelist_head, ics_free_header *block);





#endif
