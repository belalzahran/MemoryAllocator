#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>
#include <stddef.h>
#include "icsmm.h"



/* Helper function declarations go here */


bool isAligned(void* ptr, int alignmentNum);
size_t getAlignedPayloadSize(size_t size);
void* prepareNewPage(ics_free_header **freelist_head);


#endif
