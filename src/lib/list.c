#include "list.h"


#include "../paging/heap.h"
#include "../lib/debug.h"



/* creates a new empty list */
list* list_init(){
    list* new_list =(list*) malloc(sizeof(list));
    new_list->size=0;
    new_list->head=new_list->tail=NULL;
    return new_list;
}

/* Returns pointer to a new list with one element given */
list* list_init_with(void* data){
    list* new_list = (list*) malloc(sizeof(list));
    new_list->size=1;
    new_list->tail=new_list->head=(list_elem*) malloc(sizeof(list_elem));
    
    new_list->head->data=data;
    new_list->size=1;
    return new_list;
}

void* pop(list* l){
    if(is_empty(l)) return NULL;
    list_elem* elem = l->head;
    l->head=elem->next;
    if(l->head)
        l->head->prev=0;
    void* data=elem->data;
    l->size--;
    
    free(elem);

    return data;
}

bool push(list* l, void* data){
    list_elem* elem = (list_elem*) malloc(sizeof(list_elem));
    if(!elem) return false;
    elem->data=data;
    elem->next=l->head;
    l->head->prev=elem;
    l->head=elem;
    l->size++;

    return true;
}


/* Appends data to list. */
bool append(list* l, void* data){
    list_elem* elem = (list_elem*) malloc(sizeof(list_elem));
    if(!elem) return false;
    elem->data=data;
    elem->next=NULL;
    if(is_empty(l)) {
        l->head=elem;
        l->tail=elem;
        elem->prev=NULL;
        elem->next=NULL;
        l->size++;
        return true;
    }
    elem->prev=l->tail;
    l->tail=elem;
    elem->prev->next=elem;
    l->size++;
    return true;
}

/* Returns true if the list l is empty */
bool is_empty(list* l){
    return l->size==0;
}

/* Removes the element with the given data from the list.
 * Returns true on if the data was present and removed.*/
bool remove(list* l, void* data){
    list_elem* elem=l->head;
    helper_variable=1;
    while(elem->data!=data && elem->next!=NULL){
        elem=elem->next;
    }
    if(elem->data!=data) return false;
 
    if(elem==l->head) {
        l->head=elem->next;
        if(l->head)
            l->head->prev=NULL;
 
    }else{
        elem->prev->next=elem->next;
    }
    if(elem==l->tail) {
        l->tail=elem->prev;
    }else{
        elem->next->prev=elem->prev;
    }
    
    free(elem);
    l->size--;
    return true;
}

void list_dump(list* l){
    list_elem* elem=l->head;
    println("[");

    if(elem!=NULL){
        print(itoa((int)elem->data,str,BASE_HEX));
        elem=elem->next;
    }

    while(elem!=NULL){
        print(",");
        print(itoa((int)elem->data,str,BASE_HEX));
        elem=elem->next;
    }
    print("]");
}

void* list_get(list* l, uint32_t idx){
    if(idx>=l->size) PANIC("LIST OUT OF BOUNDS EXCEPTION");
    list_elem* elem= l->head;
    for(uint32_t i=0;i<idx;i++) elem=elem->next;
    
    return elem->data;
}

int get_size(list *l){
    return l->size;
}
