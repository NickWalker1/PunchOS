#pragma once

#include "../lib/typedefs.h"

#define O_CREAT    1<<0
#define O_OPEN      1<<1
#define O_NONBLOCK     1<<2


//--------------Message Passing------------------

#include "../sync/sync.h"

typedef struct mq_attr{
    size_t mq_flags; /* Flag variable */
    size_t mq_maxmsg; /* Max # of messages on queue */
    size_t mq_msgsize; /* Max msg size in bytes */
    size_t mq_curmsgs; /* # of messages currently in queue */
}mq_attr_t;

/* Message queue descriptor struct */
typedef struct mqd{
    char name[12];
    mq_attr_t *attr;
    void *base; /* Base vaddr of the mq block */
    int write_idx; /* Index of write header */
    int read_idx; /* Index of read header */
    lock mq_lock; /* Synchronisation lock */
}mqd_t;


void mq_init();
mqd_t *mq_open(char *name, mq_attr_t *attr, uint8_t flags);
size_t mq_close(mqd_t *mqdes);
size_t mq_send(mqd_t *mqdes, char *msg_pointer, size_t msg_size);
size_t mq_receive(mqd_t *mqdes, char *buffer , size_t buff_len);
