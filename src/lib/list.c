#include "list.h"


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

void push(list* l, void* data){
    list_elem* elem = (list_elem*) malloc(sizeof(list_elem));
    elem->data=data;
    elem->next=l->head;
    l->head->prev=elem;
    l->head=elem;
    l->size++;
}

void append(list* l, void* data){
    list_elem* elem = (list_elem*) malloc(sizeof(list_elem));
    elem->data=data;
    elem->next=NULL;
    if(is_empty(l)) {
        l->head=elem;
        l->tail=elem;
        elem->prev=NULL;
        elem->next=NULL;
        l->size++;
        return;
    }
    elem->prev=l->tail;
    l->tail=elem;
    elem->prev->next=elem;
    l->size++;
}

bool is_empty(list* l){
    return l->size==0;
}

bool remove(list* l, void* data){
    list_elem* elem=l->head;
    while(elem->data!=data && elem->next!=NULL){
        elem=elem->next;
    }
    if(elem->data==data){
        if(elem==l->head) {
            l->head=elem->next;
            l->head->prev=NULL;
        }else{
            elem->prev->next=elem->next;
        }

        if(elem==l->tail) {
            l->tail=elem->prev;
        }else{
            elem->next->prev=elem->prev;
        }
        // free(elem);
        l->size--;
        return true;
    }
    return false;

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
