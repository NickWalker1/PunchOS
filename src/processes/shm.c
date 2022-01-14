#include "shm.h"
#include "process.h"


#include "../memory/heap.h"
#include "../memory/paging.h"

#include "../lib/list.h"


char BLOCKA[] ="BlockA";
char testa[] = "test string!";

list *shared_desc_list;

/* Initialises shared memory space */
void shm_init(){
    shared_desc_list=list_init_shared();
}


/* Returns the shared memory descriptor pointer if one has the same given name.
 * NULL otherwise */
shared_desc_t *shm_contains(char *name){
    list_elem *elem = shared_desc_list->head;
    while(elem!=NULL){
        if(strcmp(((shared_desc_t*)(elem->data))->name, name)==0) return elem->data;
        elem=elem->next;
    }
    return NULL;
}

shared_desc_t *shm_open(char *name, uint8_t flags){
    if(flags & O_CREATE){
        if(shm_contains(name)!=NULL) return NULL;

        void *paddr = get_next_free_phys_page(1,0);

        shared_desc_t *desc = shr_malloc(sizeof(shared_desc_t));
        
        strcpy(desc->name,name);
        desc->paddr=paddr;

        append_shared(shared_desc_list,desc);
        
        return desc;
    }

    if(flags & O_OPEN){
        return shm_contains(name);
    }
}


void *mmap(shared_desc_t *desc){
    void *vaddr = get_next_free_virt_page(1,0);

    map_page(desc->paddr,vaddr,F_ASSERT);

    return vaddr;
}


/* Not Implemented */
void shm_unlink(char *name){}


/* Test Function A */
void shm_A(){
    shm_init();
    shared_desc_t *desc = shm_open(BLOCKA,O_CREATE);

    if(!desc) PANIC("shm create failed");
    void *ptr= mmap(desc);

    write(ptr,testa,strlen(testa)+1);


    while(1);
}

/* Test Function B */
void shm_B(){
    proc_sleep(1,UNIT_SEC);

    shared_desc_t *desc = shm_open(BLOCKA,O_OPEN);
    void *dest = malloc(50);

    void *ptr = mmap(desc);

    read(dest,ptr,strlen(testa)+1);

    if(strcmp(ptr,testa)==0){
        println("WOOP WOOP!");
    }

    while(1);

}

/* Basic write function */
void *write(void *dest, void *src, size_t n){
    return memcpy(dest,src,n);
}

/* Basic read function */
void *read(void *dest, void *src, size_t n){
    return memcpy(dest, src, n);
}
