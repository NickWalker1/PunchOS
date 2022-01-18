#include "list.h"


#include "../memory/heap.h"
#include "../lib/debug.h"


list *list_init_shared(){
    list *new_list=(list*) shr_malloc(sizeof(list));
    if(!new_list)
        return NULL;

    return list_setup(new_list,true);
}



/* creates a new empty list in process' local heap*/
list* list_init(){
    list *new_list=(list*) malloc(sizeof(list));
    if(!new_list) return NULL;
    return list_setup(new_list,false);
}

list *list_setup(list *l, bool is_shared){
    l->is_shared=is_shared;
    l->size=0;
    l->head=l->tail=NULL;
    return l;

}


void *pop_shared(list *l){
    ASSERT(l->is_shared,"List shared type mismatch on pop_shared");
    list_elem *elem=pop_elem(l);
    if(elem){
        void *data=elem->data;
        shr_free(elem);
        return data;
    }
    return NULL;
}


/* Pops an element from the list,frees it and returns the data pointer */
void* pop(list* l){
    ASSERT(!l->is_shared,"List shared type mismatch on pop");
    list_elem *elem = pop_elem(l);
    if(elem){
        void *data=elem->data;
        free(elem);
        return data;
    }
    
    return NULL;
    
}

/* Pops the first element from the list and returns it.
 * Returns NULL if the list is empty */
list_elem *pop_elem(list *l){
    if(is_empty(l)) {
        KERN_WARN("Attempted pop from empty");
        return NULL;
    }
    list_elem* elem = l->head;
    l->head=elem->next;
    l->size--;
    if(l->head)
        l->head->prev=0;

    return elem;
}


/* Allocates list element and pushes it to the front of the list */
bool push(list *l, void *data){
    ASSERT(!l->is_shared,"List shared type mismatch on push");
    list_elem* elem = (list_elem*) malloc(sizeof(list_elem));
    if(!elem) return false;
    elem->data=data;
    elem->next=l->head;
    push_elem(l,elem);

    return true;
}

bool push_shared(list *l, void *data){
    ASSERT(l->is_shared, "List shared type mismatch on push_shared");
    list_elem* elem = (list_elem*) shr_malloc(sizeof(list_elem));
    if(!elem) return false;
    elem->data=data;
    elem->next=l->head;
    push_elem(l,elem);

    return true;
}


/* Pushes a list element to the front of the list */
void push_elem(list *l, list_elem *elem){
    l->head->prev=elem;
    l->head=elem;
    l->size++;
}


/* Appends data to list. */
bool append(list *l, void *data){
    ASSERT(!l->is_shared,"List shared type mismatch on append");
    list_elem* elem = (list_elem*) malloc(sizeof(list_elem));
    if(!elem) return false;
    elem->data=data;
    elem->next=NULL;
    append_elem(l,elem);
    return true;
}

bool append_shared(list *l, void *data){
    ASSERT(l->is_shared,"List shared tpye mismatch on append_shared");
    list_elem* elem = (list_elem*) shr_malloc(sizeof(list_elem));
    if(!elem) return false;
    elem->data=data;
    elem->next=NULL;
    append_elem(l,elem);
    return true;
}


/* Appends a list element to the end of the list */
void append_elem(list *l, list_elem *elem){
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


/* Returns true if the list l is empty */
bool is_empty(list* l){
    return l->size==0;
}


/* Removes the first element with the given data from the list and frees it.
 * Returns true on success */
bool remove(list *l, void *data){
    ASSERT(!l->is_shared,"List shared type mismatch on remove");
    list_elem *elem = remove_elem(l,data);
    if(elem){
        free(elem);
        return true;
    }
    return false;
}

bool remove_shared(list *l, void *data){
    ASSERT(l->is_shared,"List shared type mismatch on remove_shared");
    list_elem *elem = remove_elem(l,data);
    if(elem){
        // println("removing");print(itoa(elem,str,BASE_HEX));
        shr_free(elem);
        return true;
    }
    return false;
}

/* Removes the first element from the list with the given data,
 * and returns that element */
list_elem *remove_elem(list *l, void *data){
    if(is_empty(l)){
        KERN_WARN("Attempted remove from empty");
        return NULL;
    } 
    
    list_elem* elem=l->head;

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
    l->size--;
    return elem;
}


/* Prints elements of given list */
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


/* Gets data of list element at given index */ //TODO update to return list_elem
void* list_get(list* l, uint32_t idx){
    if(idx>=l->size) PANIC("LIST OUT OF BOUNDS EXCEPTION");
    list_elem* elem= l->head;
    for(uint32_t i=0;i<idx;i++) elem=elem->next;
    
    return elem->data;
}

int get_size(list *l){
    return l->size;
}

list_elem *list_contains(list *l, void *data){
    list_elem *elem=l->head;
    while(elem!=NULL){
        if(elem->data==data) return elem;
        elem=elem->next;
    }
    return NULL;
}
