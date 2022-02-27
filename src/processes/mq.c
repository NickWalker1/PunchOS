#include "mq.h"
#include "process.h"


#include "../memory/heap.h"
#include "../memory/paging.h"

#include "../lib/list.h"


char BLOCKA[] ="BlockA";
char testa[] = "test string!";

list *shared_desc_list;


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


/* Initialise default values for message queue system */
void mq_init(){
    mq_list=list_init_shared();

    default_mq_attr=shr_malloc(sizeof(mq_attr_t));

    default_mq_attr->mq_flags=O_NONBLOCK;
    default_mq_attr->mq_maxmsg=16;
    default_mq_attr->mq_msgsize=256;
    default_mq_attr->mq_curmsgs=0;
}


/* Returns pointer to mqd_t struct allocated in shared space.
 * Will create the struct if given O_CREAT and a unique name.
 *    Can provide custom attributes using the given_attr arugment.
 *    On NULL defaults will be provided.
 * Will return an existing struct with O_OPEN and a valid name. */
mqd_t *mq_open(char *name, mq_attr_t *given_attr,uint8_t flags){
    if(strlen(name)>12) return NULL;

    if(flags & O_CREAT){
        if(mq_list_contains(name)!=NULL) return NULL;

        mq_attr_t *attr = shr_malloc(sizeof(mq_attr_t));
        if(!given_attr){
            memcpy(attr,default_mq_attr,sizeof(mq_attr_t));
        }
        else{
            memcpy(attr, given_attr,sizeof(mq_attr_t));
        }

        if(attr->mq_maxmsg*attr->mq_msgsize>PGSIZE){
            shr_free(attr);
            return NULL;
        } 

        mqd_t *mqdes = shr_malloc(sizeof(mqd_t));
        strcpy(mqdes->name,name);
        mqdes->attr=attr;
        mqdes->read_idx=0;
        mqdes->write_idx=0;
        mqdes->base=palloc_kern(1,F_ASSERT);
        // lock_init(&mqdes->mq_lock);

        append_shared(mq_list, mqdes);
        return mqdes;
    }

    if(flags & O_OPEN){
        return mq_list_contains(name);
    }

    return NULL;

}


/* Will free all memory associated with mqdes and remove it from the list of message queues. */
size_t mq_close(UNUSED mqd_t *mqdes){
    /* To be implemented */

    return 0;
}


/* Send the contents of msg_pointer to the message queue mqdes. */
size_t mq_send(mqd_t *mqdes, char *msg_pointer, size_t msg_size){
    mq_attr_t *attr = mqdes->attr;
    
    if(msg_size>attr->mq_msgsize) return 0;

    


    if(attr->mq_curmsgs==attr->mq_maxmsg){

	
        return 0;

    }

    // Write 
    
    mqdes->write_idx=mqdes->write_idx%attr->mq_maxmsg;
    // Calculate write header virtual address
    void *write_addr=mqdes->base + (mqdes->write_idx * attr->mq_msgsize);

    //Copy the memory
    memcpy(write_addr,msg_pointer,msg_size);

    attr->mq_curmsgs++;
    mqdes->write_idx++;


    return msg_size;
}


/* Given a message queue descripter and a buffer of size msg_len.
 * If there is a message in the queue, the oldest one will be written to the buffer if the buffer is big enough. */
size_t mq_receive(mqd_t *mqdes, char *buffer, size_t buff_len){
    ASSERT(mqdes!=NULL,"NULL mqdes in receive");
    mq_attr_t *attr = mqdes->attr;

    //Check return buffer is big enough
    if(buff_len < attr->mq_msgsize){
        KERN_WARN("MQ receive buffer too small");
        return 0;
    } 


    if(attr->mq_curmsgs==0) {
        return 0;
    }

    // Read from location of read_idx 
    void *read_addr= mqdes->base + mqdes->read_idx * attr->mq_msgsize;
    
    memcpy(buffer,read_addr,buff_len);

    mqdes->read_idx++;
    attr->mq_curmsgs--;


    return 0;
}
































/*      ANSWERS BELOW!!!        */





/* MQ recieve:
mq_attr_t *attr = mqdes->attr;

    //Check return buffer is big enough
    if(buff_len < attr->mq_msgsize){
        KERN_WARN("MQ receive buffer too small");
        return 0;
    } 

    lock_acquire(&mqdes->mq_lock);

    if(attr->mq_curmsgs==0) {
        lock_release(&mqdes->mq_lock);
        return 0;
    }

    // Read from location of read_idx 
    void *read_addr= mqdes->base + mqdes->read_idx * attr->mq_msgsize;
    
    memcpy(buffer,read_addr,buff_len);

    mqdes->read_idx++;
    attr->mq_curmsgs--;

    lock_release(&mqdes->mq_lock);

    return buff_len;
    */

/* MQ open: 
mqd_t *mq_open(char *name, mq_attr_t *given_attr, uint8_t flags){
    if(strlen(name)>12) return NULL;

    if(flags & O_CREAT){
        if(mq_list_contains(name)!=NULL) return NULL;

        mq_attr_t *attr = shr_malloc(sizeof(mq_attr_t));
        if(!given_attr){
            memcpy(attr,default_mq_attr,sizeof(mq_attr_t));
        }
        else{
            memcpy(attr, given_attr,sizeof(mq_attr_t));
        }

        if(attr->mq_maxmsg*attr->mq_msgsize>PGSIZE){
            shr_free(attr);
            return NULL;
        } 

        mqd_t *mqdes = shr_malloc(sizeof(mqd_t));

        // Fill out struct details 
        strcpy(mqdes->name,name);
        mqdes->attr=attr;
        mqdes->read_idx=0;
        mqdes->write_idx=0;
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
*/

/* ---------- MQ send  ---------
ASSERT(mqdes!=NULL,"NULL mqdes in send");
    mq_attr_t *attr = mqdes->attr;
    
    if(msg_size>attr->mq_msgsize) return 0;

    
    // Acquire lock 
    lock_acquire(&mqdes->mq_lock);


    if(attr->mq_curmsgs==attr->mq_maxmsg){

        lock_release(&mqdes->mq_lock);
	
        return 0;

    }

    // Write 
    
    mqdes->write_idx=mqdes->write_idx%attr->mq_maxmsg;
    // Calculate write header virtual address
    void *write_addr=mqdes->base + (mqdes->write_idx * attr->mq_msgsize);

    //Copy the memory
    memcpy(write_addr,msg_pointer,msg_size);

    attr->mq_curmsgs++;
    mqdes->write_idx++;

    // Release lock 
    lock_release(&mqdes->mq_lock);

    return msg_size;


*/


/*
mqd_t *mq_open(char *name, mq_attr_t *given_attr, uint8_t flags){
    if(strlen(name)>12) return NULL;

    if(flags & O_CREAT){
        if(mq_list_contains(name)!=NULL) return NULL;

        mq_attr_t *attr = shr_malloc(sizeof(mq_attr_t));
        if(!given_attr){
            memcpy(attr,default_mq_attr,sizeof(mq_attr_t));
        }
        else{
            memcpy(attr, given_attr,sizeof(mq_attr_t));
        }

        if(attr->mq_maxmsg*attr->mq_msgsize>PGSIZE){
            shr_free(attr);
            return NULL;
        } 

        mqd_t *mqdes = shr_malloc(sizeof(mqd_t));
        strcpy(mqdes->name,name);
        mqdes->attr=attr;
        mqdes->read_idx=0;
        mqdes->write_idx=0;
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
*/