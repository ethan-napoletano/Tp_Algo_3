#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

/*------------------------  on definie les types  -----------------------------*/

typedef enum { red, black } NodeColor;

struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    int key;
    NodeColor color;
};


struct _BSTreeIterator {
    const BinarySearchTree* collection;
    const BinarySearchTree* (*begin)(const BinarySearchTree*);
    const BinarySearchTree* current;
    const BinarySearchTree* (*next)(const BinarySearchTree*);
};

/*------------------------  ca cest la couleur -----------------------------*/
static NodeColor node_color(const BinarySearchTree* t) {
    if (!t) return black;
    return ((const struct _bstree*)t)->color;
}
static void set_color(BinarySearchTree* t, NodeColor c) {
    if (!t) return;
    ((struct _bstree*)t)->color = c;
}

/*------------------------  declaration interne  -----------------------------*/
BinarySearchTree* grandparent(BinarySearchTree* n);
BinarySearchTree* uncle(BinarySearchTree* n);
BinarySearchTree* fixredblack_insert(BinarySearchTree* x);
BinarySearchTree* fixredblack_insert_case1(BinarySearchTree* x);
BinarySearchTree* fixredblack_insert_case2_left(BinarySearchTree* x);
BinarySearchTree* fixredblack_insert_case2_right(BinarySearchTree* x);
BinarySearchTree* fixredblack_remove(BinarySearchTree* p, BinarySearchTree* x);
BinarySearchTree* fixredblack_remove_case1(BinarySearchTree* p, BinarySearchTree* x);
BinarySearchTree* fixredblack_remove_case2(BinarySearchTree* p, BinarySearchTree* x);


/*------------------------  BaseBSTree  -----------------------------*/

BinarySearchTree* bstree_create(void) {
    return NULL;
}

BinarySearchTree* bstree_cons(BinarySearchTree* left, BinarySearchTree* right, int key) {
    BinarySearchTree* t = malloc(sizeof(struct _bstree));
    t->parent = NULL;
    t->left = left;
    t->right = right;
    t->key = key;
    t->color = red; 
    if (left) left->parent = t;
    if (right) right->parent = t;
    return t;
}

void freenode(const BinarySearchTree* t, void* n) {
    (void)n;
    free((BinarySearchTree*)t);
}

void bstree_delete(ptrBinarySearchTree* t) {
    bstree_depth_postfix(*t, freenode, NULL);
    *t = NULL;
}

bool bstree_empty(const BinarySearchTree* t) {
    return t == NULL;
}

int bstree_key(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->key;
}

BinarySearchTree* bstree_left(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->left;
}

BinarySearchTree* bstree_right(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->right;
}

BinarySearchTree* bstree_parent(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->parent;
}

void bstree_add(ptrBinarySearchTree* t, int v) {
    BinarySearchTree* parent = NULL;
    BinarySearchTree** cur = t;
    while (*cur) {
        parent = *cur;
        if (v == (*cur)->key) return;
        cur = (v < (*cur)->key) ? &((*cur)->left) : &((*cur)->right);
    }

    *cur = bstree_cons(NULL, NULL, v);
    (*cur)->parent = parent;
    BinarySearchTree* stop = fixredblack_insert(*cur);
    BinarySearchTree* root = stop;

    while (root && root->parent) root = root->parent;
    if (root) *t = root;
    else if (*t == NULL) *t = stop;
    if (*t) set_color(*t, black);
}

const BinarySearchTree* bstree_search(const BinarySearchTree* t, int v) {
    while (t && v != t->key) {
        t = (v < t->key) ? t->left : t->right;
    }
    return t;

}

/*------------------------  Successeur / Predecesseur -----------------------------*/


static const BinarySearchTree* find_next(const BinarySearchTree* x,const BinarySearchTree* (*primary)(const BinarySearchTree*),const BinarySearchTree* (*secondary)(const BinarySearchTree*)) {
    assert(x != NULL);
    const BinarySearchTree* child = primary(x);
    if (child) {
        while (secondary(child)) child = secondary(child);
        return child;
    }
    const BinarySearchTree* p = x->parent;
    const BinarySearchTree* cur = x;
    while (p && cur == primary(p)) {
        cur = p;
        p = p->parent;
    }
    return p;
}

static const BinarySearchTree* c_bstree_left(const BinarySearchTree* t) {
    return bstree_left((BinarySearchTree*)t);
}

static const BinarySearchTree* c_bstree_right(const BinarySearchTree* t) {
    return bstree_right((BinarySearchTree*)t);
}


const BinarySearchTree* bstree_successor(const BinarySearchTree* x) {
    return find_next(x, c_bstree_right, c_bstree_left);
}

const BinarySearchTree* bstree_predecessor(const BinarySearchTree* x) {
    return find_next(x, c_bstree_left, c_bstree_right);
}


/*------------------------  Rotations comme en vue en td-----------------------------*/

void leftrotate(BinarySearchTree *x) {
    BinarySearchTree* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) {
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

void rightrotate(BinarySearchTree *y) {
    BinarySearchTree* x = y->left;
    y->left = x->right;
    if (x->right) x->right->parent = y;
    x->parent = y->parent;
    if (!y->parent) {
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;
}


void testrotateleft(BinarySearchTree *t) { leftrotate(t); }
void testrotateright(BinarySearchTree *t) { rightrotate(t); }

/*------------------------  Swap nodes  -----------------------------*/

void bstree_swap_nodes(ptrBinarySearchTree* tree, ptrBinarySearchTree from, ptrBinarySearchTree to) {
    assert(!bstree_empty(*tree) && !bstree_empty(from) && !bstree_empty(to));

    if (from->parent) {
        if (from->parent->left == from) from->parent->left = to;
        else from->parent->right = to;
    } else {
        *tree = to;
    }

    if (to->parent) {
        if (to->parent->left == to) to->parent->left = from;
        else to->parent->right = from;
    } else {
        *tree = from;
    }

    BinarySearchTree* tmp_parent = from->parent;
    from->parent = to->parent;
    to->parent = tmp_parent;

    BinarySearchTree* tmp_left = from->left;
    from->left = to->left;
    to->left = tmp_left;
    if (from->left) from->left->parent = from;
    if (to->left) to->left->parent = to;

    BinarySearchTree* tmp_right = from->right;
    from->right = to->right;
    to->right = tmp_right;
    if (from->right) from->right->parent = from;
    if (to->right) to->right->parent = to;
}
/*-------------------------------------------------------------------*/

void bstree_remove_node(ptrBinarySearchTree* t, ptrBinarySearchTree current) {
    assert(!bstree_empty(*t) && !bstree_empty(current));
    ptrBinarySearchTree substitute;

    if (current->left == NULL && current->right == NULL) {
        substitute = NULL;
    } else if (current->left == NULL) {
        substitute = current->right;
    } else if (current->right == NULL) {
        substitute = current->left;
    } else {
        ptrBinarySearchTree succ = (BinarySearchTree*)bstree_successor(current);
        current->key = succ->key;
        current = succ;
        substitute = current->right;
    }

    NodeColor curcol = node_color(current);
    NodeColor subcol = node_color(substitute);

    if (substitute) substitute->parent = current->parent;

    if (!current->parent) {
        *t = substitute;
    } else if (current == current->parent->left) {
        current->parent->left = substitute;
    } else {
        current->parent->right = substitute;
    }
    if (curcol == black) {
        if (substitute == NULL || subcol == black) {
            BinarySearchTree* root_after = fixredblack_remove(current->parent, substitute);
            if (root_after) {
                while (root_after->parent) root_after = root_after->parent;
                *t = root_after;
            }
        } else {
            set_color(substitute, black);
        }
    }

    free(current);
}

void bstree_remove(ptrBinarySearchTree* t, int v) {
    BinarySearchTree* current = *t;
    while (current) {
        if (v < current->key) current = current->left;
        else if (v > current->key) current = current->right;
        else {
            bstree_remove_node(t, current);
            return;
        }
    }
}

/*------------------------  Traversals / Visitors  -----------------------------*/

void bstree_depth_prefix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (!t) return;
    f(t, environment);
    bstree_depth_prefix(t->left, f, environment);
    bstree_depth_prefix(t->right, f, environment);
}

void bstree_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (!t) return;
    bstree_depth_infix(t->left, f, environment);
    f(t, environment);
    bstree_depth_infix(t->right, f, environment);
}

void bstree_depth_postfix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (!t) return;
    bstree_depth_postfix(t->left, f, environment);
    bstree_depth_postfix(t->right, f, environment);
    f(t, environment);
}

void bstree_iterative_breadth(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    Queue* q = create_queue();
    queue_push(q, (void*)t);
    while (!queue_empty(q)) {
        const BinarySearchTree* node = queue_top(q);
        queue_pop(q);
        if (!node) continue;
        f(node, environment);
        if (node->left) queue_push(q, (void*)node->left);
        if (node->right) queue_push(q, (void*)node->right);
    }
    delete_queue(&q);
}
void bstree_iterative_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* env) {
    const BinarySearchTree* cur = t;
    while (cur->left) cur = cur->left;
    while (cur) {
        f(cur, env);
        if (cur->right) {
            cur = cur->right;
            while (cur->left) cur = cur->left;
        } else {
            const BinarySearchTree* p = cur->parent;
            while (p && cur == p->right) {
                cur = p;
                p = p->parent;
            }
            cur = p;
        }
    }
}

/*------------------------------------------------------------------------*/

const BinarySearchTree* goto_min(const BinarySearchTree* e) {
    while (e->left) e = e->left;
    return e;
}

const BinarySearchTree* goto_max(const BinarySearchTree* e) {
    while (e->right) e = e->right;
    return e;
}

BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
    BSTreeIterator* it = malloc(sizeof *it);
    it->collection = collection;
    it->begin = (direction == forward) ? goto_min : goto_max;
    it->next = (direction == forward) ? bstree_successor : bstree_predecessor;
    it->current = NULL;
    return it;
}

void bstree_iterator_delete(ptrBSTreeIterator* i) {
    if (!i || !*i) return;
    free(*i);
    *i = NULL;
}

BSTreeIterator* bstree_iterator_begin(BSTreeIterator* i) {
    i->current = i->begin(i->collection);
    return i;
}

bool bstree_iterator_end(const BSTreeIterator* i) {
    return i->current == NULL;
}

BSTreeIterator* bstree_iterator_next(BSTreeIterator* i) {
    if (!i->current) { 
        i->current = NULL; 
        return i; 
    }
    i->current = i->next(i->current);
    return i;
}

const BinarySearchTree* bstree_iterator_value(const BSTreeIterator* i) {
    return i->current;
}

/*-------------------------------------  DOT   ------------------------------------------------*/

void bstree_node_to_dot(const BinarySearchTree* t, void* stream) {
    if (!t || !stream) return;
    FILE* out = (FILE*)stream;
    const char* fill = (node_color(t) == red) ? "red" : "grey";
    fprintf(out, "    \"%p\" [label=\"%d\", style=filled, fillcolor=%s];\n", (void*)t, t->key, fill);
    if (t->left) {
        fprintf(out, "    \"%p\" -> \"%p\";\n", (void*)t, (void*)t->left);
    } else {
    }
    if (t->right) {
        fprintf(out, "    \"%p\" -> \"%p\";\n", (void*)t, (void*)t->right);
    } else {
    }
}

/*------------------------ grandparent et oncle -----------------------------*/

BinarySearchTree* grandparent(BinarySearchTree* n) {
    if (!n || !n->parent) return NULL;
    return n->parent->parent;
}

BinarySearchTree* uncle(BinarySearchTree* n) {
    BinarySearchTree* gp = grandparent(n);
    if (!gp) return NULL;
    if (n->parent == gp->left) return gp->right;
    return gp->left;
}

/*------------------------  fix after insertion (standard algorithm) -----------------------------*/

BinarySearchTree* fixredblack_insert(BinarySearchTree* x) {
    if (!x) return NULL;
    if (x->parent == NULL) {
        set_color(x, black);
        return x;
    }
    if (node_color(x->parent) == black) return x;
    return fixredblack_insert_case1(x);
}

BinarySearchTree* fixredblack_insert_case1(BinarySearchTree* x) {
    BinarySearchTree* p = x->parent;
    BinarySearchTree* u = uncle(x);
    BinarySearchTree* gp = grandparent(x);

    if (!p) return x;
    if (u && node_color(u) == red) {
        set_color(p, black);
        set_color(u, black);
        if (gp) set_color(gp, red);
        return fixredblack_insert(gp);
    } else {
        if (!gp) return x;
        if (p == gp->left) return fixredblack_insert_case2_left(x);
        else return fixredblack_insert_case2_right(x);
    }
}

BinarySearchTree* fixredblack_insert_case2_left(BinarySearchTree* x) {
    BinarySearchTree* p = x->parent;
    BinarySearchTree* gp = grandparent(x);
    if (!p || !gp) return x;

    if (x == p->right) {
        leftrotate(p);
        x = p;
        p = x->parent;
    }
    rightrotate(gp);
    set_color(p, black);
    set_color(gp, red);
    return p;
}

BinarySearchTree* fixredblack_insert_case2_right(BinarySearchTree* x) {
    BinarySearchTree* p = x->parent;
    BinarySearchTree* gp = grandparent(x);
    if (!p || !gp) return x;

    if (x == p->left) {
        rightrotate(p);
        x = p;
        p = x->parent;
    }
    leftrotate(gp);
    set_color(p, black);
    set_color(gp, red);
    return p;
}


 
BinarySearchTree* fixredblack_remove(BinarySearchTree* p, BinarySearchTree* x) {
    if (p == NULL) {
        if (x) set_color(x, black);
        return x;
    }
    int x_is_left = (p->left == x);
    BinarySearchTree* s = x_is_left ? p->right : p->left;
    if (s && node_color(s) == red) {
        if (x_is_left) {
            leftrotate(p);
        } else {
            rightrotate(p);
        }
        set_color(s, black);
        set_color(p, red);
        s = x_is_left ? p->right : p->left;
    }
    NodeColor s_left_col = s && s->left ? node_color(s->left) : black;
    NodeColor s_right_col = s && s->right ? node_color(s->right) : black;
    if ((!s || (s_left_col == black && s_right_col == black))) {
        if (s) set_color(s, red);
        if (x) set_color(x, black);
        if (node_color(p) == red) {
            set_color(p, black);
            return p; 
        } else {
            return fixredblack_remove(p->parent, p);
        }
    }
    if (x_is_left) {
        if (s && node_color(s->right) == red) {
            leftrotate(p);
            set_color(s, node_color(p));
            set_color(p, black);
            if (s->right) set_color(s->right, black);
            return s;
        } else if (s && node_color(s->left) == red) {
            rightrotate(s);
            s = p->right;
            leftrotate(p);
            set_color(s, node_color(p));
            set_color(p, black);
            if (s->right) set_color(s->right, black);
            return s;
        }
    } else {
        if (s && node_color(s->left) == red) {
            rightrotate(p);
            set_color(s, node_color(p));
            set_color(p, black);
            if (s->left) set_color(s->left, black);
            return s;
        } else if (s && node_color(s->right) == red) {
            leftrotate(s);
            s = p->left;
            rightrotate(p);
            set_color(s, node_color(p));
            set_color(p, black);
            if (s->left) set_color(s->left, black);
            return s;
        }
    }
    BinarySearchTree* root = p;
    while (root->parent) root = root->parent;
    if (root) set_color(root, black);
    return root;
}


BinarySearchTree* fixredblack_remove_case1(BinarySearchTree* p, BinarySearchTree* x) {
    return fixredblack_remove(p, x);
}
BinarySearchTree* fixredblack_remove_case2(BinarySearchTree* p, BinarySearchTree* x) {
    return fixredblack_remove(p, x);
}

