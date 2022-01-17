#include "ipc.h"
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
    if(flags & O_CREAT){
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

    return NULL;
}


void *mmap(shared_desc_t *desc){
    void *vaddr = get_next_free_virt_page(1,0);

    map_page(desc->paddr,vaddr,F_ASSERT);

    return vaddr;
}



/* Test Function A */
void shm_A(){
    shm_init();
    shared_desc_t *desc = shm_open(BLOCKA,O_CREAT);

    if(!desc) PANIC("shm create failed");
    void *ptr= mmap(desc);

    write(ptr,testa,strlen(testa)+1);


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

}

/* Basic write function */
void *write(void *dest, void *src, size_t n){
    return memcpy(dest,src,n);
}

/* Basic read function */
void *read(void *dest, void *src, size_t n){
    return memcpy(dest, src, n);
}


/*-----------------------------------------------------------*/
/*------------ IPC Message Passing functionality ------------*/
/*-----------------------------------------------------------*/


mq_attr_t *default_mq_attr;


list *mq_list; 

mqd_t *mq_list_contains(char *name){
    list_elem *elem = mq_list->head;

    while(elem!=NULL){
        if(strcmp(((mqd_t*)elem->data)->name,name)==0) return elem->data;
        elem=elem->next;
    }
    return NULL;
}


void mq_init(){
    mq_list=list_init_shared();

    default_mq_attr=shr_malloc(sizeof(mq_attr_t));

    default_mq_attr->mq_flags=O_NONBLOCK;
    default_mq_attr->mq_maxmsg=16;
    default_mq_attr->mq_msgsize=256;
    default_mq_attr->mq_curmsgs=0;
}


mqd_t *mq_open(char *name, mq_attr_t *attr, uint8_t flags){
    if(strlen(name)>12) return NULL;

    if(flags & O_CREAT){
        if(mq_list_contains(name)!=NULL) return NULL;

        if(!attr){
            attr = shr_malloc(sizeof(mq_attr_t));
            memcpy(attr,default_mq_attr,sizeof(mq_attr_t));
        }

        mqd_t *mqdes = shr_malloc(sizeof(mqd_t));
        strcpy(mqdes->name,name);
        mqdes->attr=attr;
        mqdes->read_hdr=0;
        mqdes->write_hdr=0;
        mqdes->base=palloc_kern(1,F_ASSERT);
        
        lock_init(&mqdes->mq_lock);

        append_shared(mq_list, mqdes);

        return mqdes;
    }

    if(flags & O_OPEN){
        return mq_list_contains(name);
    }

    return NULL;

}


size_t mq_close(mqd_t *mqdes){

}

size_t mq_send(mqd_t *mqdes, char *msg_pointer, size_t msg_size){
    ASSERT(mqdes!=NULL,"NULL mqdes in send");
    mq_attr_t *attr = mqdes->attr;
    
    if(msg_size>attr->mq_msgsize) return 0;

    
    /* Acquire lock */
    lock_acquire(&mqdes->mq_lock);


    if(attr->mq_curmsgs==attr->mq_maxmsg){
        //if not non blocking return 0

        return 0;
        // if blocking block
    }

    /* Write memory */
    
    mqdes->write_hdr=mqdes->write_hdr%attr->mq_maxmsg;
    // Calculate write header virtual address
    void *write_addr=mqdes->base + (mqdes->write_hdr * attr->mq_msgsize);

    //Copy the memory
    memcpy(write_addr,msg_pointer,msg_size);

    attr->mq_curmsgs++;
    mqdes->write_hdr++;

    /* Release lock */
    lock_release(&mqdes->mq_lock);

    return msg_size;
    
}

size_t mq_recieve(mqd_t *mqdes, char *msg_pointer, size_t msg_len){
    mq_attr_t *attr = mqdes->attr;

    //Check return buffer is big enough
    if(msg_len < attr->mq_msgsize) return 0;

    lock_acquire(&mqdes->mq_lock);

    if(attr->mq_curmsgs==0) return 0;
    /* Read from location of read_hdr */
    void *read_addr= mqdes->base + mqdes->read_hdr * attr->mq_msgsize;
    
    memcpy(msg_pointer,read_addr,msg_len);

    mqdes->read_hdr++;
    attr->mq_curmsgs--;

    lock_release(&mqdes->mq_lock);

    return msg_len;
}

char *WAA ="WAAAA";

void mq_A(){
    mq_init();
    
    mqd_t *mqdes = mq_open("MQ1",NULL,O_CREAT);

    int status = mq_send(mqdes,WAA,strlen(WAA));
    mq_send(mqdes,"WOO",4);
    mq_send(mqdes,"YEET",5);

    println(itoa(status, str, BASE_DEC));
    println(WAA);
}

void mq_B(){
    proc_sleep(1,UNIT_SEC);

    mqd_t *mqdes = mq_open("MQ1",NULL,O_OPEN);

    char *ret = malloc(2046);
    
    while(mq_recieve(mqdes,ret,2046)){
        println(ret);
    }
}