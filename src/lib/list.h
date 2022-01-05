#pragma once

#include "typedefs.h"

typedef struct list list;
typedef struct list_elem list_elem;

extern uint32_t helper_variable;

struct list{
    uint32_t size;
    list_elem* head;
    list_elem* tail;
};

struct list_elem{
    void* data; /* Pointer to whatever is in it */
    list_elem* next;
    list_elem* prev;
};

list* list_init();
list* list_init_with(void *data);

bool append(list* l, void *data);
void append_elem(list *l, list_elem *elem);
bool remove(list* l, void *data);
list_elem *remove_elem(list *l, void *data);
bool push(list* l, void* data);
void push_elem(list *l, list_elem *elem);
void* pop(list* l);
list_elem *pop_elem(list *l);
int get_size(list* l);
bool is_empty(list* l);
void list_dump(list* l);
void* list_get(list* l, uint32_t idx);