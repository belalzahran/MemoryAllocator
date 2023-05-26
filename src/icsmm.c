#include "icsmm.h"
#include "debug.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * The allocator MUST store the head of its free list in this variable. 
 * Doing so will make it accessible via the extern keyword.
 * This will allow ics_freelist_print to access the value from a different file.
 */
ics_free_header *freelist_head = NULL;

/*
 * The allocator MUST use this pointer to refer to the position in the free list to 
 * starting searching from. 
 */
ics_free_header *freelist_next = NULL;

/*
 * This is your implementation of malloc. It acquires uninitialized memory from  
 * ics_inc_brk() that is 16-byte aligned, as needed.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If successful, the pointer to a valid region of memory of at least the
 * requested size is returned. Otherwise, NULL is returned and errno is set to 
 * ENOMEM - representing failure to allocate space for the request.
 * 
 * If size is 0, then NULL is returned and errno is set to EINVAL - representing
 * an invalid request.
 */
// newPageHeader->header.block_size |= 0x1;
        // newPageFooter->block_size |= 0x1;


        // conditions to perform
        // - malloc call is bigger then 5 pages
        // - malloc call needs to call ics_inc for more room
        // - malloc finds enough space and allocates block
        // - malloc call finishes last page and recives a fail from calling ics_inc

void *ics_malloc(size_t size) { 

    printf("\nReceived Malloc Request...\n\n");


    

    //// WAS HERE



    // if (freelist_head == NULL)
    // {

        // prepare for first malloc call
        printf("Adding a page to the heap...\n");
        prepareNewPage(&freelist_head, &freelist_next);
        printf("page added!\n");
        //ics_header_print(&(freelist_head->header));
        //ics_freelist_print();
        printf("The page is set up and we are preparing to allocate the block...\n");
        ics_free_header* blockToSplit = (ics_free_header*)getNextFit(size, &freelist_head, &freelist_next);
        // ics_freelist_print();
        // ics_header_print(blockToSplit);
        // printf("block to split:");
        // is_allocated((void*)blockToSplit);
        splitAndPrepFreeBlock(size, (ics_header*)blockToSplit);





        




        

        


    // }
    // else
    // {
    //     printf("finding the free block we want to use...");
    //     void* freeBlockHeader = getNextFit(size, &freelist_head, &freelist_next);
    // }
    

    // else if ()
    // {
    //     // request one page at a time and coalesce with free block at the end of previosul ym

// printf("Calculating total block size using payload size for header BLOCK_SIZE...\n");
//         size_t payloadSizeWithPadding = getAlignedPayloadSize(size);
//         newPageHeader->header.requested_size = size;

    // }
    // else if()
    // {

        
        // if no more space availablae from heap no more pages return NULL and ENOMEM
    // }


    // return (void*) newPageHeader;
    return (void*) 0x13342353;
}

/*
 * Marks a dynamically allocated block as no longer in use and coalesces with 
 * adjacent free blocks (as specified by Homework Document). 
 * Adds the block to the appropriate bucket according to the block placement policy.
 *
 * @param ptr Address of dynamically allocated memory returned by the function
 * ics_malloc.
 * 
 * @return 0 upon success, -1 if error and set errno accordingly.
 * 
 * If the address of the memory being freed is not valid, this function sets errno
 * to EINVAL. To determine if a ptr is not valid, (i) the header and footer are in
 * the managed  heap space, (ii) check the hid field of the ptr's header for
 * special value (iii) check the fid field of the ptr's footer for special value,
 * (iv) check that the block_size in the ptr's header and footer are equal, (v) 
 * the allocated bit is set in both ptr's header and footer, and (vi) the 
 * requested_size is identical in the header and footer.
 */
int ics_free(void *ptr) { 
    return -99999;
}

/*
 * Resizes the dynamically allocated memory, pointed to by ptr, to at least size 
 * bytes. See Homework Document for specific description.
 *
 * @param ptr Address of the previously allocated memory region.
 * @param size The minimum size to resize the allocated memory to.
 * @return If successful, the pointer to the block of allocated memory is
 * returned. Else, NULL is returned and errno is set appropriately.
 *
 * If there is no memory available ics_malloc will set errno to ENOMEM. 
 *
 * If ics_realloc is called with an invalid pointer, set errno to EINVAL. See ics_free
 * for more details.
 *
 * If ics_realloc is called with a valid pointer and a size of 0, the allocated     
 * block is free'd and return NULL.
 */
void *ics_realloc(void *ptr, size_t size) {
    return (void*) 0xFFFFFFFF;
}