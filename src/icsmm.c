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

void* prologueAddress = NULL;

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

void *ics_malloc(size_t size)
{ 

    if(size > 20448)
    {
        errno = ENOMEM;
        return NULL;
    }

    bool mallocBlockFound = false;
    ics_free_header* freeBlockHeader;

    printf("\nReceived Malloc Request...\n\n");
    
    if (freelist_head == NULL)
    {
        printf("1st IF BLk)Inside first if\n");
        prepareNewPage(&freelist_head, &freelist_next);
        printf("1st IF BLk)created new page\n");
        // printf("printing the header 1\n");
        // ics_header_print((void*)&(freelist_head->header));
        while(!mallocBlockFound)
        {
            printf("1st IF BLk)Inside while loop\n");
            // ics_freelist_print();
            freeBlockHeader = (ics_free_header*)loopAndCheckList(size, freelist_head, freelist_next);
            
            if(freeBlockHeader != NULL)
            {
                printf("1st IF BLk)BLOCK FOR MALLOC FOUND, PRINTING BLOCK..\n");
                // ics_header_print((void*)freeBlockHeader);
                mallocBlockFound = true;

                removeFromFreeList(&freelist_head, &freelist_next, freeBlockHeader);

                // printf("printing the header 3\n");
                // ics_header_print((void*)freeBlockHeader);
                continue;
            }

            printf("1st IF BLk)we DID NOT FIND a big enough block\n");

            if (ics_inc_brk() == (void*)-1)
            {
                errno = ENOMEM;
                return NULL;
            }
            
            // printf("(1st IF BLk)we will prepare new page now...\n");
            updateHeader();
            // printf("1st IF BLk)finished running update header\n");
            setEpilogue();
            // printf("1st IF BLk)finished running set epilogue\n");
            //ics_freelist_print();
            //ics_header_print((void*)freeBlockHeader);
            //ics_header_print(&(freelist_head->header));
            setEpilogueFooter(&(freelist_head->header));
            // ics_header_print(&(freelist_head->header));
            // printf("1st IF BLk)finished running set epilogue footer\n");
            printf("1st IF BLk)WE ARE GOING TO LOOP BACK AFTER ADDING MORE ROOM\n");

            
        }
        // printf("printing the header 4\n");
        // ics_header_print((void*)freeBlockHeader);
        void* retVal = splitAndPrepFreeBlock(size,(ics_header*)freeBlockHeader);
        return retVal;
        
    }
    else
    {
        printf("Inside else\n");
        while(!mallocBlockFound)
        {
            // printf("printing the header\n");
            // ics_header_print((void*)freeBlockHeader);
            // ics_freelist_print();
            freeBlockHeader = (ics_free_header*)loopAndCheckList(size, freelist_head, freelist_next);

            if(freeBlockHeader != NULL)
            {
                printf("FOUND THE BLOCK WE WILL USE TO RETURN MALLOC SPACE\n");
                // ics_header_print((void*)freeBlockHeader);
                // printf("we found a big enough block\n");
                mallocBlockFound = true;
                // printf("no we remove the block from the free list\n");
                // ics_freelist_print();
                removeFromFreeList(&freelist_head, &freelist_next, freeBlockHeader);
                // ics_freelist_print();
                // printf("block removed from the free list\n");
                continue;
            }

            printf("we DID NOT FIND a big enough block\n");
            if (ics_inc_brk() == (void*)-1)
            {
                errno = ENOMEM;
                return NULL;
            }
            printf("we will prepare new page now...\n");
            updateHeader();
            printf("finished running update header\n");
            setEpilogue();
            printf("finished running set epilogue\n");
            setEpilogueFooter(&(freelist_head->header));
            printf("finished running set epilogue footer\n");
            printf("WE ARE GOING TO LOOP BACK AFTER ADDING MORE ROOM\n");
            
        }

        // printf("no we will split and prep the block to be returned\n");
        void* retVal = splitAndPrepFreeBlock(size,(ics_header*)freeBlockHeader);
        // ics_header_print(retVal);
        return retVal;

    }
        // ics_free_header* freeBlockHeader = (ics_free_header*)getNextFit(size, &freelist_head, &freelist_next);
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