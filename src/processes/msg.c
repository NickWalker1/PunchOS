#include "msg.h"
#include "shm.h"

/* IPC message passing functionality */

#include "../memory/heap.h"

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
            mq_attr_t *attr = shr_malloc(sizeof(mq_attr_t));
            memcpy(attr,default_mq_attr,sizeof(mq_attr_t));
        }

        mqd_t *mqdes = shr_malloc(sizeof(mqd_t));
        strcpy(mqdes->name,name);
        mqdes->attr=attr;
        mqdes->read_hdr=0;
        mqdes->write_hdr=0;
        mqdes->base=palloc_kern(1,F_ASSERT);

        append_shared(mq_list, mqdes);

        return mqdes;
    }

    if(flags & O_OPEN){
        return mq_list_contains(name);
    }

    return NULL;

}

size_t mq_send(mqd_t *mqdes, char *msg_pointer, size_t msg_size){
    ASSERT(mqdes!=NULL,"NULL mqdes in send");
    mq_attr_t *attr = mqdes->attr;
    if(attr->mq_curmsgs==attr->mq_maxmsg){
        //if not non blocking return 0
        // if blocking block
    }
}