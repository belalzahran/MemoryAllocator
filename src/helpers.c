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
        printf("%s: allocated bit set to 0\n", say);
        *block_size_ptr &= ~0x1;
    } else {
        // If last bit is 0, set it to 1
        printf("%s: allocated bit set to 1\n", say);
        *block_size_ptr |= 0x1;
    }
}

bool isAligned(void* ptr, int alignmentNum)
{
    if ((unsigned long)(ptr) % alignmentNum != 0)
    { 
        printf("Your address is NOT aligned with %d!\n",alignmentNum);
        return false;
    }
    else{
        printf("Your address is aligned with %d!\n",alignmentNum);
        return true;

    }

}


size_t getAlignedPayloadSize(size_t size)
{
    if (size < 16)
    {
        return 16;
    }
    else if (size == 16)
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


void* prepareNewPage(ics_free_header **freelist_head, ics_free_header **freelist_next)
{
    // printf("brk location: %p\n",ics_get_brk());

    //create prologue and set allocated bit
    // printf("***PROLOGUE***\n\n");
    ics_footer * prologue = (ics_footer*) ics_inc_brk();
    // printf("Heap Beginning/Prologue set at: %p\n",(void*)prologue);
    // printf("Initializing prologue values...");
    prologue->fid = FOOTER_MAGIC;
    prologue->block_size = 0;
    prologue->requested_size = 0;
    // printf("Setting allocated bit of the Eplilogue to 1...");
    toggle_allocated_bit((void*)prologue,"prologue", 1);
    // printf("Set!\n");

    // printf("Initialized!\n");


    // printf("***HEADER***\n\n");
    // printf("Setting header set at location: %p\n", ((void*)prologue)+8);
    ics_free_header* newPageHeader = (ics_free_header*)(((void *)prologue) + 8);
    // printf("Initializing next and prev to NULL...\n");
    newPageHeader->next = NULL;
    newPageHeader->prev = NULL;
    // printf("Initialized header values...\n");
    // isAligned(newPageHeader,8);
    newPageHeader->header.hid = HEADER_MAGIC;
    newPageHeader->header.requested_size = 0;
    newPageHeader->header.block_size = ics_get_brk() - 8 - ((void*)newPageHeader) ; 
    // printf("Initialized!\n\n");
    


    // printf("***FOOTER***\n\n");
    // printf("Setting footer at location (adding up): %p\n", (((void *)newPageHeader) + (newPageHeader->header.block_size) - 8));
    // printf("Setting footer at location (getbrk): %p\n", ics_get_brk() - 16);
    ics_footer * newPageFooter = (ics_footer*) (ics_get_brk() - 16);
    newPageFooter->block_size = newPageHeader->header.block_size;
    newPageFooter->fid = FOOTER_MAGIC;
    newPageFooter->requested_size = 0;
    // isAligned(newPageFooter,8);


    // printf("***EPILOGUE***\n\n");
    // printf("Setting epilogue at location: %p\n",(ics_get_brk() - 8));
    ics_header * epilogue = (ics_header*) (ics_get_brk() - 8);
    epilogue->hid = HEADER_MAGIC;
    epilogue->block_size = 0;
    epilogue->requested_size = 0;
    // printf("Setting allocated bit of the Eplilogue to 1...");
    toggle_allocated_bit((void*)epilogue,"epilogue", 1);
    // printf("Set!\n");

    // printf("\n\n\nTHE END OF THE HEAP ADDRESS(getbrk):        %p\n", ics_get_brk());
    // printf("THE END OF THE HEAP ADDRESS(header + 4088): %p\n\n\n\n", (void*)newPageHeader + 4088);

    // printf("Done preparing the newly added page.\n");
    // printf("Now setting head of the free linked list to point to newPageHeader...");

    insertIntoFreeList(freelist_head, freelist_next, newPageHeader);


    // *freelist_head = newPageHeader;
    // *freelist_next = newPageHeader;
    // printf("Set!\n\n");

    return newPageHeader;

}



void removeFromFreeList(ics_free_header **freelist_head, ics_free_header *block_to_remove) {
    // Check if the freelist is empty or block to remove is NULL
    if (*freelist_head == NULL || block_to_remove == NULL) {
        return;
    }
    // if(get_freelist_length(*freelist_head) == 1)
    // {
    //     *freelist_head = NULL;
    //     *freelist_next = NULL;
    //     return;
    // }

    ics_free_header *current_block = *freelist_head;
    
    do {
        // Check if current_block is the block we want to remove
        if (current_block == block_to_remove) {
            // If the block to remove is at the head of the list
            if (current_block == *freelist_head) {
                *freelist_head = current_block->next;
            }

            // If there is a previous block, link it to the next block
            if (current_block->prev != NULL) {
                current_block->prev->next = current_block->next;
            }

            // If there is a next block, link it to the previous block
            if (current_block->next != NULL) {
                current_block->next->prev = current_block->prev;
            }

            // Set next and prev pointers of the removed block to NULL
            current_block->next = NULL;
            current_block->prev = NULL;

            // set the removed block to allocated once taking it from the free linked list


            toggle_allocated_bit(&(current_block->header), "header of block removed from free list", 1);
            ics_footer* current_footer = (ics_footer*)((void*)current_block + current_block->header.block_size - sizeof(ics_footer));
            toggle_allocated_bit(current_footer,"footer of block removed from free list", 1);
            

            return;
        }

        current_block = current_block->next;


    } while (current_block != *freelist_head);
}

// start searching at next
void* getNextFit(size_t size, ics_free_header **freelist_head, ics_free_header **freelist_next)
{
    size_t alignedSize = getAlignedPayloadSize(size);
    size_t potentialBlockSize = alignedSize + 16;

    if (*freelist_next != NULL) 
    {
        ics_free_header *searchStartLocation = *freelist_next;

        do
        {
            // check the block size and see if it fits
            //ics_freelist_print();
            if ((*freelist_next)->header.block_size > potentialBlockSize)
            {
                ics_free_header *selectedBlock = *freelist_next;
                *freelist_next = (*freelist_next)->next;
                removeFromFreeList(freelist_head, selectedBlock);
                //ics_freelist_print();

                // Check if the free list is empty
                if (*freelist_head == NULL) 
                {
                    *freelist_next = NULL;  // Make sure freelist_next doesn't point to an allocated block
                }
                return (void*)(selectedBlock);
            }
            // Access the next and previous blocks in the free list

            *freelist_next = (*freelist_next)->next;

        }while((*freelist_next != searchStartLocation) && (*freelist_next != NULL));


         if(get_freelist_length(*freelist_head) == 0)
        {
            *freelist_next = NULL;
        }

    }
    else
    {
        printf("*freelist_next is NULL\n\n\n");
        return (void*)0x34234;
    }
    return NULL;

        // if freelist next reaches block where it began searching, you must ask for more room
}

// inserting in address order
// loop should never reach nulll (loop back)
 // make sure that last node in free list points back to head
void insertIntoFreeList(ics_free_header **freelist_head, ics_free_header **freelist_next, ics_free_header *block) {

    ics_footer* current_footer = (ics_footer*)((char*)block + block->header.block_size - sizeof(ics_footer));
    toggle_allocated_bit((void*)block, "header of block inserted into free list", 0);
    toggle_allocated_bit(current_footer, "footer of block inserted into free list", 0);

    // Set next and prev pointers of the block to NULL initially
    block->next = NULL;
    block->prev = NULL;

    if (*freelist_head == NULL) {
        // Set the freelist_head to the new block
        *freelist_head = block;
        *freelist_next = block;
    } else {
        // If the free list is not empty
        ics_free_header *current = *freelist_head;

        // Check if the block should be inserted before the head
        if (block < *freelist_head) {
            block->next = *freelist_head;
            (*freelist_head)->prev = block;
            *freelist_head = block;
        } else {
            // Iterate through the rest of the list
            while (current->next != NULL && current->next < block) {
                current = current->next;
            }
            // Insert the block after the current block
            block->next = current->next;
            block->prev = current;
            if (current->next != NULL) {
                current->next->prev = block;
            }
            current->next = block;
        }
    }
}







void splitAndPrepFreeBlock(size_t size, ics_header* bigFreeHeader)
{

      // get the size we are going to split w

    size_t potentialBlockSize = getAlignedPayloadSize(size) + 16;
    ics_header* updatedHeader = bigFreeHeader;

    updatedHeader->block_size = potentialBlockSize;
    updatedHeader->requested_size = size;

    ics_footer* newFooter = (ics_footer*) (((void*)updatedHeader) + (updatedHeader->block_size) - sizeof(ics_footer));
    newFooter->block_size = updatedHeader->block_size;
    newFooter->fid = FOOTER_MAGIC;
    newFooter->requested_size = size;

    // must togge allocated bit again because block size was reset
    toggle_allocated_bit(updatedHeader,"updated header of new block allocated", 1);
    toggle_allocated_bit(newFooter,"new footer of newly allocated block", 1);

    // ics_header_print((void*)updatedHeader);

    // is_allocated((void*)updatedHeader);
    // is_allocated((void*)newFooter);
    


    // // *block_size_ptr &= ~0x1;
    // // } else {
    // //     // If last bit is 0, set it to 1
    // //     printf("%s: allocated bit set to 1\n", say);
    // //     *block_size_ptr |= 0x1;


    // ics_freelist_print();



  

    // update the header of block to be allocated


    // add a footer for the block to be allocated

    // add a new ics free header to the remaining free space of the heap
    // insert back into the free list and set the allocated bits back to free

}