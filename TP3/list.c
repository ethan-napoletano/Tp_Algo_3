/*-----------------------------------------------------------------*/
/*
 Licence Informatique - Structures de données
 Mathias Paulin (Mathias.Paulin@irit.fr)
 
 Implantation du TAD List vu en cours.
 */
/*-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"

typedef struct s_LinkedElement {
	int value;
	struct s_LinkedElement* previous;
	struct s_LinkedElement* next;
} LinkedElement;

typedef struct s_SubList{
    LinkedElement* head;
    LinkedElement* tail;
} SubList;


/* Use of a sentinel for implementing the list :
 The sentinel is a LinkedElement* whose next pointer refer always to the head of the list and previous pointer to the tail of the list
 */
struct s_List {
	LinkedElement* sentinel;
	int size;
};


/*-----------------------------------------------------------------*/

List* list_create(void) {
    List* l = malloc(sizeof(List));
    assert(l != NULL); 

    l->sentinel = malloc(sizeof(LinkedElement));
    assert(l->sentinel != NULL);
	
    l->sentinel->next = l->sentinel;
    l->sentinel->previous = l->sentinel;
    l->size = 0;
    return l;
}


/*-----------------------------------------------------------------*/
List* list_push_back(List* l, int v) {
    LinkedElement* new_elem = malloc(sizeof(LinkedElement));
    new_elem->value = v;

    LinkedElement* sentinel = l->sentinel;


    new_elem->previous = sentinel->previous;   
    new_elem->next = sentinel;                 
    sentinel->previous->next = new_elem;       
    sentinel->previous = new_elem;            

    l->size++;

    return l;
}


/*-----------------------------------------------------------------*/

void list_delete(ptrList* l) {
    List* list = *l;

    LinkedElement* ElemCourant = list->sentinel->next;
    while (ElemCourant != list->sentinel) {
        LinkedElement* ProchainDel = ElemCourant;
        ElemCourant = ElemCourant->next;
        free(ProchainDel);
    }

    free(list->sentinel);
    free(list);

    *l = NULL;
}


/*-----------------------------------------------------------------*/

List* list_push_front(List* l, int v) {
    LinkedElement* e = malloc(sizeof(LinkedElement));
    e->value = v;

    LinkedElement* sentinel = l->sentinel;

    e->next = sentinel->next;  
    e->previous = sentinel;       
    sentinel->next->previous = e;   
    sentinel->next = e;             

    l->size++;

    return l;
}


/*-----------------------------------------------------------------*/

int list_front(const List* l) {
	assert(!list_is_empty(l));
	return l->sentinel->next->value;
}

/*-----------------------------------------------------------------*/

int list_back(const List* l) {
	assert(!list_is_empty(l));
	return l->sentinel->previous->value;
}

/*-----------------------------------------------------------------*/

List* list_pop_front(List* l) {
	assert(!list_is_empty(l));
	LinkedElement* e = l->sentinel->next;
	 l->sentinel->next = e->next;
	 l->sentinel->next->previous = l->sentinel;
	 l->size--;
	 free(e);
	return l;
}

/*-----------------------------------------------------------------*/

List* list_pop_back(List* l) {
    assert(!list_is_empty(l));
    LinkedElement* e = l->sentinel->previous;   
    e->previous->next = l->sentinel;           
    l->sentinel->previous = e->previous;       
    l->size--;
    free(e);
    return l;
}


/*-----------------------------------------------------------------*/

List* list_insert_at(List* l, int p, int v) {
    assert(0 <= p && p <= l->size);//précondition pour etre sur que il soit dedans
	
    LinkedElement* new_elem = malloc(sizeof(LinkedElement));
    new_elem->value = v;
	LinkedElement* ElemCourant = l->sentinel->next; 
	
    for (int i = 0; i < p; i++) {
        ElemCourant = ElemCourant->next;
    }

    new_elem->next = ElemCourant;
    new_elem->previous = ElemCourant->previous;
    ElemCourant->previous->next = new_elem;
    ElemCourant->previous = new_elem;

    l->size++;

    return l;
}


/*-----------------------------------------------------------------*/

List* list_remove_at(List* l, int p) {
    assert(0 <= p && p < l->size); 
    LinkedElement* ElemCourant = l->sentinel->next; 
	
    for (int i = 0; i < p; i++) {
        ElemCourant = ElemCourant->next;
    }
    ElemCourant->previous->next = ElemCourant->next;
    ElemCourant->next->previous = ElemCourant->previous;

    free(ElemCourant);
    l->size--;

    return l;
}


/*-----------------------------------------------------------------*/

int list_at(const List* l, int p) {
    assert(0 <= p && p < l->size); 
    LinkedElement* current = l->sentinel->next; 
    for (int i = 0; i < p; i++) {
        current = current->next;
    }
    return current->value;
}


/*-----------------------------------------------------------------*/

bool list_is_empty(const List* l) {
    return l->size == 0;
}


/*-----------------------------------------------------------------*/

int list_size(const List* l) {
    return l->size;
}

/*-----------------------------------------------------------------*/

List* list_map(List* l, ListFunctor f, void* environment) {
	LinkedElement* current = l->sentinel->next;  
	while (current != l->sentinel) {
        current->value = f(current->value, environment);
		current = current->next;
    }

    return l; 
}


/*-----------------------------------------------------------------*/



SubList list_split(SubList l) {
    SubList list;
    if (l.head == NULL || l.head->next == NULL) {
        list.head = NULL;
        list.tail = NULL;
        return list;
    }

    LinkedElement* moitie = l.head;
    LinkedElement* fin = l.head->next;

    while (fin != NULL && fin->next != NULL) {
        moitie = moitie->next;
        fin = fin->next->next;
    }

    list.head = moitie->next;
    list.tail = l.tail;

    moitie->next = NULL;
    if (list.head)
        list.head->previous = NULL;

    return list;
}


/*-----------------------------------------------------------------*/

SubList list_merge(SubList leftlist, SubList rightlist, OrderFunctor f) {
    SubList new_sublist;
    LinkedElement* Gauche = leftlist.head;
    LinkedElement* Droite = rightlist.head;
    LinkedElement* head = NULL;
    LinkedElement* tail = NULL;

    while (Gauche != NULL && Droite != NULL) {
        LinkedElement* Elem;
        if (f(Gauche->value, Droite->value)) {
            Elem = Gauche;
            Gauche = Gauche->next;
        } else {
            Elem = Droite;
            Droite = Droite->next;
        }

        Elem->previous = tail;
        
        if (tail != NULL)
            tail->next = Elem;
        else
            head = Elem;

        tail = Elem;
    }
    LinkedElement* reste = NULL;

    if (Gauche != NULL)
        reste = Gauche;
    else if (Droite != NULL)
        reste = Droite;

    while (reste != NULL) {
        reste->previous = tail;
        if (tail != NULL){
            tail->next = reste;
        }
        else{
            head = reste;
        }
        tail = reste;
        reste = reste->next;
    }

   
    if (tail != NULL)
        tail->next = NULL;   

    new_sublist.head = head;
    new_sublist.tail = tail;

    return new_sublist;
}

/*-----------------------------------------------------------------*/

SubList list_mergesort(SubList l, OrderFunctor f) {
    if (l.head == NULL || l.head->next == NULL)
        return l;

    SubList right = list_split(l);

    SubList leftlist = { l.head, NULL };
    SubList rightlist = { right.head, right.tail };

    leftlist = list_mergesort(leftlist, f);
    rightlist = list_mergesort(rightlist, f);

    return list_merge(leftlist, rightlist, f);
}

/*-----------------------------------------------------------------*/


List* list_sort(List* l, OrderFunctor f) {
    SubList sub;
    sub.head = l->sentinel->next;
    sub.tail = l->sentinel->previous;

    sub.head->previous = NULL;
    sub.tail->next = NULL;

    SubList subtrie = list_mergesort(sub, f);


    subtrie.head->previous = l->sentinel;
    subtrie.tail->next = l->sentinel;
    l->sentinel->next = subtrie.head;
    l->sentinel->previous = subtrie.tail;

    return l;
}
