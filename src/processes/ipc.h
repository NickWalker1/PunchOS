#pragma once

#include "../lib/typedefs.h"

#define O_CREAT    1<<0
#define O_OPEN      1<<1
#define O_NONBLOCK     1<<2


typedef struct shared_desc{
    char name[12];
    void *paddr;
} shared_desc_t;

void shm_init();
shared_desc_t *shm_contains(char *name);
shared_desc_t *shm_open(char *name, uint8_t flags);
void *mmap(shared_desc_t *desc);
void shm_unlink(char *name); /* Not implemented */


void *write(void *dest ,void *src,size_t n);
void *read(void *dest,void *src,size_t n);

void shm_A();
void shm_B();


//--------------Message Passing------------------

#include "../sync/sync.h"

typedef struct mq_attr{
    size_t mq_flags;
    size_t mq_maxmsg; /* Max # of messages on queue */
    size_t mq_msgsize; /* Max msg size in bytes */
    size_t mq_curmsgs; /* # of messages currently in queue */
}mq_attr_t;

/* Message queue descriptor struct */
typedef struct mqd{
    char name[12];
    mq_attr_t *attr;
    void *base; /* Base vaddr of the mq block */
    size_t write_hdr; /* offset of write header */
    size_t read_hdr; /* Offset of read header */
    lock mq_lock; /* Lock */
}mqd_t;


void mq_init();
mqd_t *mq_open(char *name, mq_attr_t *attr, uint8_t flags);
size_t mq_close(mqd_t *mqdes);
size_t mq_send(mqd_t *mqdes, char *msg_pointer, size_t msg_size);
size_t mq_recieve(mqd_t *mqdes, char *msg_pointer, size_t msg_len);

void mq_A();
void mq_B();