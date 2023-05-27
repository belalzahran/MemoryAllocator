#include "helpers.h"
#include "debug.h"

/* Helper function definitions go here */


size_t get_freelist_length(ics_free_header *freelist_head) 
{
    size_t length = 0;
    ics_free_header *current_node = freelist_head;
    
    while (current_node != NULL) {
        length++;
        current_node = current_node->next;
    }
    
    return length;
}

bool is_allocated(void *block) {

    uint64_t block_size;

    // Check if the block is a header or a footer
    if (((ics_header*)block)->hid == HEADER_MAGIC) {
        block_size = ((ics_header*)block)->block_size;
    } else if (((ics_footer*)block)->fid == FOOTER_MAGIC) {
        block_size = ((ics_footer*)block)->block_size;
    } else {
        printf("This block is neither a header nor a footer\n");
        return false; // or handle the error in a way that suits your program
    }

    // Check the least significant bit of the block_size
    if (block_size & 0x1) {
        printf("This block is marked as allocated\n");
        return true;
    } else {
        printf("This block is not marked as allocated\n");
        return false;
    }
}

void toggle_allocated_bit(void *block_header_or_footer, char* say, int x) 
{
    uint64_t *block_size_ptr = (uint64_t *)block_header_or_footer;

    // Check if the last bit of block_size is 1 or 0
    uint64_t last_bit = *block_size_ptr & 0x1;
    
    if (x == 0) {
        // If last bit is 1, set it to 0
        //printf("%s: allocated bit set to 0\n", say);
        *block_size_ptr &= ~0x1;
    } else {
        // If last bit is 0, set it to 1
        //printf("%s: allocated bit set to 1\n", say);
        *block_size_ptr |= 0x1;
    }
}

bool isAligned(void* ptr, int alignmentNum)
{
    if ((unsigned long)(ptr) % alignmentNum != 0)
    { 
        //printf("Your address is NOT aligned with %d!\n",alignmentNum);
        return false;
    }
    else{
        //printf("Your address is aligned with %d!\n",alignmentNum);
        return true;

    }

}


size_t getAlignedPayloadSize(size_t size)
{
    if (size < 16)
    {
        return 16;
    }
    else if (size == 16 || (size % 16 == 0))
    {
        return size;
    }
    else
    {
        int dataLeftOver = size % 16;
        size_t padding = 16 - dataLeftOver;
        return size + padding;
    }

}

void setEpilogue()
{
    ics_header * epilogue = (ics_header*) (ics_get_brk() - 8);
    epilogue->hid = HEADER_MAGIC;
    epilogue->block_size = 0;
    epilogue->requested_size = 0;
    toggle_allocated_bit((void*)epilogue,"epilogue", 1);
    // printf("we finsihed the eiplogueee!!!!!~~~\n");
}

void setEpilogueFooter(void* header)
{
    // printf("printing the size of header to see why it is wrong: %d\n",((ics_free_header*)header)->header.block_size);

    ics_footer * newPageFooter = (ics_footer*) (ics_get_brk() - 16);
    newPageFooter->block_size = ((ics_free_header*)header)->header.block_size;
    newPageFooter->fid = FOOTER_MAGIC;
    newPageFooter->requested_size = 0;
}

void updateHeader()
{
    ics_free_header* header = freelist_head;
    header->header.block_size = ics_get_brk() - 8 - ((void*)header);
}



void* prepareNewPage(ics_free_header **freelist_head, ics_free_header **freelist_next)
{
    
    ics_footer * prologue = (ics_footer*) ics_inc_brk();
    prologueAddress = (void*)prologue;
    prologue->fid = FOOTER_MAGIC;
    prologue->block_size = 0;
    prologue->requested_size = 0;
    toggle_allocated_bit((void*)prologue,"prologue", 1);
    ics_free_header* newPageHeader = (ics_free_header*)(((void *)prologue) + 8);
    newPageHeader->next = NULL;
    newPageHeader->prev = NULL;
    newPageHeader->header.hid = HEADER_MAGIC;
    newPageHeader->header.requested_size = 0;
    newPageHeader->header.block_size = ics_get_brk() - 8 - ((void*)newPageHeader); 

    setEpilogueFooter((void*)newPageHeader);
    setEpilogue();

    insertIntoFreeList(freelist_head, freelist_next, newPageHeader);
    return newPageHeader;

}



void removeFromFreeList(ics_free_header **freelist_head, ics_free_header **freelist_next, ics_free_header *block_to_remove) {

    ics_free_header *current_block = *freelist_head;
    
    do 
    {
        // Check if current_block is the block we want to remove
        if (current_block == block_to_remove) 
        {
            if (current_block == *freelist_head) 
            {
                *freelist_head = current_block->next;
            }
            if (current_block->prev != NULL)
            {
                current_block->prev->next = current_block->next;
            }
            if (current_block->next != NULL) 
            {
                current_block->next->prev = current_block->prev;
            }
            current_block->next = NULL;
            current_block->prev = NULL;

            
            ics_footer* current_footer = (ics_footer*)((void*)current_block + current_block->header.block_size - sizeof(ics_footer));
            toggle_allocated_bit(current_footer,"footer of block removed from free list", 1);
            toggle_allocated_bit(&(current_block->header), "header of block removed from free list", 1);
            

            if(get_freelist_length(*freelist_head) == 0)
            {
                *freelist_next = NULL;
            }
            
            return;
        }
        current_block = current_block->next;

    } while (current_block != *freelist_head);
}

// start searching at next
void* loopAndCheckList(size_t size,  ics_free_header *freelist_head, ics_free_header *freelist_next)
{
    size_t alignedSize = getAlignedPayloadSize(size);
    size_t mallocRequestTotalBlockSize = alignedSize + 16;

    ics_free_header *selectedBlock;
    ics_free_header *searchStartLocation = freelist_next;
    
    do
    {
        if (freelist_next->header.block_size > mallocRequestTotalBlockSize)
        {
            return (void*)freelist_next;
        }

        // Iterate to the next block
        freelist_next = freelist_next->next;

        // If we reach the end of the list, loop back to the head
        if( freelist_next == NULL)
        {
            freelist_next = freelist_head;
        }

    }while((freelist_next != searchStartLocation) && (freelist_next != NULL));

    // No block large enough was found
    return NULL;
}


void insertIntoFreeList(ics_free_header **freelist_head, ics_free_header **freelist_next, ics_free_header *block) 
{
    toggle_allocated_bit((void*)block, "header of block inserted into free list", 0);
    ics_footer* current_footer = (ics_footer*)((char*)block + block->header.block_size - sizeof(ics_footer));
    current_footer->block_size = block->header.block_size;
    toggle_allocated_bit(current_footer, "footer of block inserted into free list", 0);

    // printf("Printing the block size of the footer that we insert: %d\n",current_footer->block_size);

    // Set next and prev pointers of the block to NULL initially
    block->next = NULL;
    block->prev = NULL;

    if (*freelist_head == NULL) 
    {
        // Set the freelist_head to the new block
        *freelist_head = block;
        *freelist_next = block;
    } 
    else 
    {
        // If the free list is not empty
        ics_free_header *current = *freelist_head;

        // Check if the block should be inserted before the head
        if (block < *freelist_head) 
        {
            block->next = *freelist_head;
            (*freelist_head)->prev = block;
            *freelist_head = block;
        } 
        else 
        {
            // Iterate through the rest of the list
            while (current->next != NULL && current->next < block) 
            {
                current = current->next;
            }
            // Insert the block after the current block
            block->next = current->next;
            block->prev = current;
            if (current->next != NULL) 
            {
                current->next->prev = block;
            }
            current->next = block;
        }
    }
}



void* splitAndPrepFreeBlock(size_t size, ics_header* bigFreeHeader)
{
    // compute sizes for later use
    size_t prevBlockSize = bigFreeHeader->block_size;
    size_t mallocRequestTotalBlockSize = getAlignedPayloadSize(size) + 16;
    size_t newFreeBlockSize = prevBlockSize - mallocRequestTotalBlockSize;

    // rename and fill header for malloc space
    ics_header* updatedHeader = bigFreeHeader;
    updatedHeader->block_size = mallocRequestTotalBlockSize;
    updatedHeader->requested_size = size;
    

    // create and fill new footer for malloc space
    ics_footer* newFooter = (ics_footer*) (((void*)updatedHeader) + (updatedHeader->block_size) - sizeof(ics_footer));
    newFooter->block_size = updatedHeader->block_size;
    newFooter->fid = FOOTER_MAGIC;
    newFooter->requested_size = size;
    toggle_allocated_bit(newFooter,"new footer of newly allocated block", 1);
    toggle_allocated_bit(updatedHeader,"updated header of new block allocated", 1);

    // create and fill new header for rest of free malloc space
    ics_free_header* newBigFreeHeader = (ics_free_header*) ((void*)newFooter + 8);
    newBigFreeHeader->header.block_size = prevBlockSize - mallocRequestTotalBlockSize;
    newBigFreeHeader->header.hid = HEADER_MAGIC;
    newBigFreeHeader->header.requested_size = 0;
    newBigFreeHeader->next = NULL;
    newBigFreeHeader->prev = NULL;
    

    // update footer for the rest of free malloc space
    ics_footer* newBigFreeFooter = (ics_footer*) (((void*)newBigFreeHeader) + newBigFreeHeader->header.block_size - 8);
    newBigFreeFooter->block_size = prevBlockSize - mallocRequestTotalBlockSize;
    toggle_allocated_bit(newBigFreeFooter,"new big free footer", 0);
    toggle_allocated_bit(newBigFreeHeader,"new big free header", 0);

    // insert the free space back into free list
    insertIntoFreeList(&freelist_head, &freelist_next, newBigFreeHeader);

    // create a void pointer to return pointer to malloc space
    void *returnValue = &(((ics_free_header*) updatedHeader)->next);


   return returnValue;

}