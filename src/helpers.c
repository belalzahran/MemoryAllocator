#include "helpers.h"
#include "debug.h"

/* Helper function definitions go here */


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





void* prepareNewPage(ics_free_header **freelist_head)
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
    prologue->block_size |= 0x1;
    // printf("Set!\n");

    // printf("Initialized!\n");


    // printf("***HEADER***\n\n");
    // printf("Setting header set at location: %p\n", ((void*)prologue)+8);
    ics_free_header* newPageHeader = (ics_free_header*)(((void *)prologue) + 8);
    // printf("Initializing next and prev to NULL...\n");
    newPageHeader->next = NULL;
    newPageHeader->prev = NULL;
    // printf("Initialized header values...\n");
    isAligned(newPageHeader,8);
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
    isAligned(newPageFooter,8);


    // printf("***EPILOGUE***\n\n");
    // printf("Setting epilogue at location: %p\n",(ics_get_brk() - 8));
    ics_header * epilogue = (ics_header*) (ics_get_brk() - 8);
    epilogue->hid = HEADER_MAGIC;
    epilogue->block_size = 0;
    epilogue->requested_size = 0;
    // printf("Setting allocated bit of the Eplilogue to 1...");
    epilogue->block_size |= 0x1;
    // printf("Set!\n");

    // printf("\n\n\nTHE END OF THE HEAP ADDRESS(getbrk):        %p\n", ics_get_brk());
    // printf("THE END OF THE HEAP ADDRESS(header + 4088): %p\n\n\n\n", (void*)newPageHeader + 4088);

    // printf("Done preparing the newly added page.\n");
    // printf("Now setting head of the free linked list to point to newPageHeader...");
    *freelist_head = newPageHeader;
    // printf("Set!\n\n");

    return newPageHeader;

}