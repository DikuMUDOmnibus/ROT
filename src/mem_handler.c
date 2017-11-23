
#include <sys/types.h>

#if defined(WIN32)
	#include <time.h>
#else
	#include <sys/time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "mem_handler.h"

/****/
//   Variables section.
/****/
RECYCLE_DATA *recycle_list;
CHUNK_DATA *chunk_list;

/****/
//   Functions section
/****/
// This is a raw function to load more memory into the main list if needed.
// It will double the amount loaded after each call. Memory that is allocated
// will sweep from two lists. 1) A freed list of old memory that has been latent.
// 2) All chunk lists to see if a suitable size can be found. Memory will not be wasted.
void load_mem(void) {
    static unsigned int amount = 100000; // We have a chunk with 1 byte to begin.

    CHUNK_DATA *pMl;

    pMl = malloc(sizeof(CHUNK_DATA));

    pMl->ptr = calloc(1, amount);
    pMl->amount = amount;
    pMl->used = 0;
    pMl->natural = TRUE;

    pMl->next = chunk_list;
    chunk_list = pMl;

    amount *= 2;
}

void make_chunk(void *mem, int size){
    CHUNK_DATA *pMl;

    pMl = malloc(sizeof(CHUNK_DATA));

    pMl->ptr = mem;
    pMl->amount = size;

    pMl->natural = FALSE;
    pMl->used = 0;
    pMl->next = chunk_list;
    chunk_list = pMl;

}

// This is a function to get a specific amount of memory from the main list.
void *alloc_mem(int iAmount) {
    CHUNK_DATA *pMl, *last = 0, *pMnext = 0;
    RECYCLE_DATA *pRl;

    // Search our recycle lists first.
    for(pRl = recycle_list;pRl;pRl = pRl->next) {
        if (pRl->size == iAmount) {
            // We found it. Now we have to get one of the nodes and free the other.
            void *hold;
            MEMORY_DATA *pMeml;

            if (pRl->list == 0)
                continue;

            hold = pRl->list->ptr;
            pMeml = pRl->list;
            pRl->list = pRl->list->next;
            free(pMeml);
            return hold;
        }
    }

    // search our entire chunk list
    for(pMl = chunk_list;pMl;pMl = pMnext) {
        pMnext = pMl->next;
        if(!pMl->natural && pMl->amount - pMl->used < 16) {
            if(last)
                last->next = pMl->next;
            else
                chunk_list = pMl->next;

            pMl->next = 0;
            free(pMl);
            continue;
        }

        last = pMl;

        if(pMl->amount - pMl->used < iAmount)
            // We do not have enough memory in this chunk. Continue to the next.
            continue;

        // Okay. We have enough memory in this chunk. Let's return the address.
        pMl->used += iAmount;
        return (void*)( ((char*)pMl->ptr) + pMl->used - iAmount);
    }

    // Nothing found. We need to allocate a new one. I'm going to deal with this recursively.
    load_mem();

    // Tail chaining..? Who cares anyways. ^_-
    return alloc_mem(iAmount);
}

void free_mem(void *ptr, int size) {
    RECYCLE_DATA *pRl;

    if (size==0)
        return;

    if (size>= 300) {
        make_chunk(ptr, size);
        return;
    }
    // search our entire recycle list for the correct size.
    for(pRl = recycle_list;pRl;pRl = pRl->next) {
        if (pRl->size == size) { // found
            MEMORY_DATA *pMl;

            pMl = malloc(sizeof(MEMORY_DATA));
            pMl->ptr = ptr;
            memset(ptr, 0, size);
            pMl->next = pRl->list;
            pRl->list = pMl;
            return;
        }
    }

    // We found nothing... We need to make a new node and add our memory to it.
    pRl = alloc_mem(sizeof(RECYCLE_DATA));
    pRl->next = recycle_list;
    recycle_list = pRl;

    pRl->size = size;
    pRl->list = 0;

    // recursion. Edit it if you want.
    free_mem(ptr, size);
}

// A command function for running a few diagnostics.
void do_memtest(CHAR_DATA *ch, char *argument) {
    char arg1[MSL], arg2[MSL];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!arg1[0]) {
        send_to_char("memtest <option>\r\nload debug\r\n", ch);
        return;
    }

    if (!strcmp(arg1, "load")) {
        if (!arg2[0]) {
            load_mem();
            send_to_char("Forcing a new chunk to load.\r\n", ch);
        }
        else {
            if (!is_number(arg2)) {
                send_to_char("That isn't a number!\r\n", ch);
                return;
            }

            make_chunk(malloc(atoi(arg2)), atoi(arg2));
        }
    }
    else if (!strcmp(arg1, "debug")) {
        if (!arg2[0]) {
            send_to_char("memtest debug <option>\r\nrecycle chunks\r\n", ch);
            return;
        }
        if (!strcmp(arg2, "chunks")) {
            CHUNK_DATA *pCl;
            for(pCl = chunk_list;pCl;pCl = pCl->next)
                printf_to_char(ch, "Chunk dump: Amount: %d Used: %d\r\n", pCl->amount, pCl->used);
        }
        else if (!strcmp(arg2, "recycle")) {
            RECYCLE_DATA *pRl;

            for(pRl = recycle_list;pRl;pRl = pRl->next) {
                MEMORY_DATA *pM;
                int i = 1;
                printf_to_char(ch, "Recycle dump: Size: %d\r\n", pRl->size);
                for(pM = pRl->list;pM;pM = pM->next, ++i)
                    if (i <= 5)
                        printf_to_char(ch, "         %2d : Ptr: %p\r\n", i, pM->ptr);

                if (i >= 6)
                    printf_to_char(ch, "%d not displayed.\r\n", i - 5);
            }
        }
        else
            send_to_char("memtest debug <option>\r\nrecycle chunks\r\n", ch);
    }
    else
        send_to_char("memtest <option>\r\nload debug\r\n", ch);
}
