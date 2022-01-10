#include "shm.h"

list *shared_block_list;

void shm_init(){
    list_init_shared(shared_block_list);
}

/* Returns a block from shared_block_list if it has the given name */
shared_block_t *shm_block_contains(char *name){
    list_elem *b = shared_block_list->head;
    while(b!=NULL){
        if(strcmp(((shared_block_t*)b->data)->name,name)==0) return b->data;
        b=b->next;
    }
    return NULL;
}


/* Returns a pointer that can be used to access the shared block */
void *mmap(shared_block_t *block){
    void *vaddr=get_next_free_virt_page(1,0);

    map_page(block->base,vaddr,0);
    return vaddr;
}


/* Returns phyiscal address of new newly allocated space NOT MAPPED */
shared_block_t *shm_open(char *name, uint8_t flags){
    if(strlen(name)>11) return NULL;

    /* Create */
    if(flags & O_CREATE){
        if(shm_block_contains(name)) return NULL;

        void *paddr= get_next_free_phys_page(1,0);

        shared_block_t *block = shr_malloc(sizeof(shared_block_t));

        block->base=paddr;
        strcpy(block->name,name);
        block->rw_hdr_off=0;
        append_shared(shared_block_list, block);

        return block;
    }

    if(flags&O_OPEN){
       return shm_block_contains(name); 
    }
}


/* Write n bytes from data to ptr */
int write(void *ptr, void *data , size_t n);


/* Read n bytes from src to dest */
int read(void *src, void *dest, size_t n);