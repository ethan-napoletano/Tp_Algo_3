// skiplist.c
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "skiplist.h"
#include "rng.h"

typedef struct s_LinkedElement {
    int value;
    struct s_LinkedElement** next;
    struct s_LinkedElement** previous;
} LinkedElement;

struct s_SkipList {
    int nblevels;
    int size;
    RNG rng;
    LinkedElement* sentinelle;
};

static LinkedElement* sentinelle_create(int nblevels) {
    LinkedElement* s = (LinkedElement*)malloc(sizeof(LinkedElement));
    s->value = 0;
    s->next = (LinkedElement**)malloc(nblevels * sizeof(LinkedElement*));
    s->previous = (LinkedElement**)malloc(nblevels * sizeof(LinkedElement*));
    for (int i = 0; i < nblevels; ++i) {
        s->next[i] = s;
        s->previous[i] = s;
    }
    return s;
}

SkipList* skiplist_create(int nblevels) {
    assert(nblevels > 0);
    SkipList* list = (SkipList*)malloc(sizeof(SkipList));
    list->nblevels = nblevels;
    list->size = 0;
    list->rng = rng_initialize(0, nblevels);
    list->sentinelle = sentinelle_create(nblevels);
    return list;
}

void skiplist_delete(SkipList** d) {
    assert(d != NULL && *d != NULL);
    SkipList* sl = *d;
    LinkedElement* cur = sl->sentinelle->next[0];

    while (cur != sl->sentinelle) {
        LinkedElement* nxt = cur->next[0];
        free(cur->next);
        free(cur->previous);
        free(cur);
        cur = nxt;
    }

    free(sl->sentinelle->next);
    free(sl->sentinelle->previous);
    free(sl->sentinelle);

    free(sl);
    *d = NULL;
}

unsigned int skiplist_size(const SkipList* d) {
    assert(d != NULL);
    return d->size;
}


int skiplist_at(const SkipList* d, unsigned int i) {
    assert(d != NULL);
    assert(i < skiplist_size(d));
    LinkedElement* cur = d->sentinelle->next[0];
    for (unsigned int j = 0; j < i; ++j) {
        cur = cur->next[0];
    }
    return cur->value;
}

void skiplist_map(const SkipList* d, ScanOperator f, void *user_data) {
    assert(d != NULL);
    assert(f != NULL);

    char mode = 0;
    if (user_data != NULL) mode = *(char*)user_data;

    if (mode == 'd') {
        LinkedElement* cur = d->sentinelle->previous[0];
        while (cur != d->sentinelle) {
            f(cur->value, user_data);
            cur = cur->previous[0];
        }
    } else {
        LinkedElement* cur = d->sentinelle->next[0];
        while (cur != d->sentinelle) {
            f(cur->value, user_data);
            cur = cur->next[0];
        }
    }
}


SkipList* skiplist_insert(SkipList* d, int value) {
    assert(d != NULL);
    LinkedElement* update[d->nblevels];
    LinkedElement* current = d->sentinelle;
    for (int level = d->nblevels - 1; level >= 0; --level) {
        while (current->next[level] != d->sentinelle && current->next[level]->value < value) {
            current = current->next[level];
        }
        update[level] = current;
    }
    current = current->next[0];
    if (current == d->sentinelle || current->value != value) {
        int new_level = rng_get_value(&(d->rng));
        if (new_level < 0) new_level = 0;
        if (new_level > d->nblevels - 1) new_level = d->nblevels - 1;

        LinkedElement* node = (LinkedElement*)malloc(sizeof(LinkedElement));
        if (!node) return d;
        node->value = value;
        node->next = (LinkedElement**)malloc(d->nblevels * sizeof(LinkedElement*));
        node->previous = (LinkedElement**)malloc(d->nblevels * sizeof(LinkedElement*));
        if (!node->next || !node->previous) {
            free(node->next);
            free(node->previous);
            free(node);
            return d;
        }
        for (int i = 0; i < d->nblevels; ++i) {
            node->next[i] = d->sentinelle;
            node->previous[i] = d->sentinelle;
        }
        for (int i = 0; i <= new_level; ++i) {
            node->next[i] = update[i]->next[i];
            node->previous[i] = update[i];

            /* link neighbors: update predecessor and successor */
            update[i]->next[i]->previous[i] = node;
            update[i]->next[i] = node;
        }

        d->size++;
    }

    return d;
}

bool skiplist_search(const SkipList* d, int value, unsigned int* nb_operations) {
	assert (d != NULL);

	bool found = false;
	*nb_operations = 0;
	LinkedElement* current = d->sentinelle;

	for(int i = d->nblevels - 1; i >= 0 && !found; i--) {
		if (current->next[i] !=  d->sentinelle && current->next[i]->value == value) {
			found = true;
		}

		while (current->next[i] !=  d->sentinelle && current->next[i]->value < value) {
			current = current->next[i];
			(*nb_operations)++;
		}
	}
	current = current->next[0];
	(*nb_operations)++;
	if (current !=  d->sentinelle && current->value == value) {
		found = true;
		return found;
	}
	return found;
}


/* ----------------------- iterator  ----------------------- */
struct s_SkipListIterator {
    SkipList* list;
    LinkedElement* current;
    int forwardorbackward; 
};

SkipListIterator* skiplist_iterator_create(SkipList* list, IteratorDirection w) {
    assert(list != NULL);
    SkipListIterator* it = (SkipListIterator*)malloc(sizeof(SkipListIterator));
    it->list = list;
    it->forwardorbackward = (w == FORWARD_ITERATOR) ? 1 : -1;
    it->current = (it->forwardorbackward == 1) ? list->sentinelle->next[0] : list->sentinelle->previous[0];
    return it;
}

void skiplist_iterator_delete(SkipListIterator** l) {
    if (l == NULL || *l == NULL) return;
    free(*l);
    *l = NULL;
}

SkipListIterator* skiplist_iterator_begin(SkipListIterator* l) {
    assert(l != NULL && l->list != NULL);
    l->current = (l->forwardorbackward == 1) ? l->list->sentinelle->next[0] : l->list->sentinelle->previous[0];
    return l;
}

bool skiplist_iterator_end(SkipListIterator* l) {
    assert(l != NULL && l->list != NULL);
    return (l->current == l->list->sentinelle);
}

SkipListIterator* skiplist_iterator_next(SkipListIterator* l) {
    assert(l != NULL && l->list != NULL);
    if (l->current == l->list->sentinelle) return l; /* already at end */
    if (l->forwardorbackward == 1)
        l->current = l->current->next[0];
    else
        l->current = l->current->previous[0];
    return l;
}

int skiplist_iterator_value(SkipListIterator* l) {
    assert(l != NULL && l->list != NULL);
    return l->current->value;
}

SkipList* skiplist_remove(SkipList* d, int value) {
    assert(d != NULL);

    LinkedElement* update[d->nblevels];
    LinkedElement* current = d->sentinelle;

    for (int level = d->nblevels - 1; level >= 0; --level) {
        while (current->next[level] != d->sentinelle && current->next[level]->value < value) {
            current = current->next[level];
        }
        update[level] = current;
    }

    current = current->next[0];

    if (current == d->sentinelle || current->value != value) {
        return d;
    }

    for (int level = 0; level < d->nblevels; ++level) {
        if (update[level]->next[level] == current) {
            update[level]->next[level] = current->next[level];
            current->next[level]->previous[level] = update[level];
        }
    }

    free(current->next);
    free(current->previous);
    free(current);

    d->size--;
    return d;
}
