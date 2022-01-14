#pragma once

#include "../lib/typedefs.h"
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
    void *base;
    size_t write_hdr;
    size_t read_hdr;
    lock *mq_lock;
}mqd_t;



mqd_t *mq_open(char *name, mq_attr_t *attr, uint8_t flags);
size_t mq_close(mqd_t *mqdes);
size_t mq_send(mqd_t *mqdes, char *msg_pointer, size_t msg_size);