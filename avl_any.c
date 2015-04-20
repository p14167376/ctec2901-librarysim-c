//===========================================================================
// FILE: avl_any.c
//===========================================================================
// Implementation file for AVL Tree struct
// Original Author: David Smallwood
// Modifications:
//  - Added avl_any_find() function.
//  - Added avl_any_inorder_map() function.
//  - Use SAFE_MALLOC(), SAFE_MALLOC_EXISTING() & SAFE_FREE()
//  - Ensured max line length of 80 to suit assignment criteria :p
//---------------------------------------------------------------------------

//#define TRACE_ON

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "avl_any.h"
#include "trace.h"
#include "smalloc.h"

typedef struct node
{
    char height;
    any item;
    struct node * left;
    struct node * right;
} node_t;

struct avl_any_implementation
{
    struct node * root;
    int size;
    rel_func lt;
};

avl_any * new_avl_any(rel_func lt)
{
    SAFE_MALLOC(avl_any,t);
    t->root = NULL;
    t->size = 0;
    t->lt = lt;
    return t;
}

#define LT(A,B) t->lt(A,B)



#define ABS(X)    ((X<0) ? -(X) : X)
#define MAX(X,Y)  ((X>Y) ? X : Y)
#define HEIGHT(P) ((P == NULL) ? 0 : P->height)


int  avl_any_isempty(avl_any *t)
{
    assert(t!=NULL);
    return (t->size == 0);
}
    
int  avl_any_isfull(avl_any *t)
{
    return 0;
}
    
int  avl_any_size(avl_any *t)
{
    assert(t!=NULL);
    return t->size;
}
    
int  get_depth(struct node *p)
{
    int l,r;
    if (p==NULL)
       return 0;
    else {
       l = 1 + get_depth(p->left);
       r = 1 + get_depth(p->right);
       return (l>r) ? l : r;
    }
}

int avl_any_depth(avl_any *t)
{
    assert(t!=NULL);
    return get_depth(t->root);
}

struct node* rotate_left(struct node *p)
{
    struct node *q;
    assert(p!=NULL);
    assert(p->right!=NULL);
    q = p->right;
    p->right = q->left;
    q->left = p;
    p->height = 1 + MAX(HEIGHT(p->left),HEIGHT(p->right));
    q->height = 1 + MAX(HEIGHT(q->left),HEIGHT(q->right));
    return q;
}

struct node* rotate_right(struct node *p)
{
    struct node *q;
    assert(p!=NULL);
    assert(p->left!=NULL);
    q = p->left;
    p->left = q->right;
    q->right = p;
    p->height = 1 + MAX(HEIGHT(p->left),HEIGHT(p->right));
    q->height = 1 + MAX(HEIGHT(q->left),HEIGHT(q->right));
    return q;
}

struct node* ensure_balance(struct node *p)
{
    // trivially balanced - return now
    if (p==NULL)
        return p;

    p->height = 1 + MAX(HEIGHT(p->left),HEIGHT(p->right));

    // left-heavy  - rebalance needed
    if ( HEIGHT(p->left) - HEIGHT(p->right) > 1)
    {
        if (HEIGHT(p->left->left) > HEIGHT(p->left->right))
            p = rotate_right(p);
        else {
            p->left = rotate_left(p->left);
            p = rotate_right(p);
        }
    }
    // right-heavy - rebalance needed
    else if ( HEIGHT(p->right) - HEIGHT(p->left) > 1)
    {

        if (HEIGHT(p->right->right) > HEIGHT(p->right->left))
           p = rotate_left(p);
        else {
           p->right = rotate_right(p->right);
           p = rotate_left(p);
        }
    }
    else// balanced - no rebalance needed
        ;
        
    return p;
}


struct node * ins(avl_any *t, any x, struct node *p)
{
    if (p==NULL)
    {
        SAFE_MALLOC_EXISTING(node_t,p);
        p->left   = p->right = NULL;
        p->item   = x;
        (t->size)++;
    }
    // inserting into left sub-tree
    else if (LT(x,p->item)) {
        p->left = ins(t, x, p->left);
    }
    // inserting into right sub-tree
    else if (LT(p->item,x)) {
        p->right = ins(t, x, p->right);
    }

    return ensure_balance(p);
}

void avl_any_insert(avl_any *t, any x)
{
    assert(t!=NULL);
    t->root = ins(t, x, t->root);
}

struct node * findmin(struct node *p)
{
    if (p==NULL)
        printf("error: (avl_any) findmin: empty tree\n");
        
    else if (p->left == NULL)
        return p;
    
    else
        return findmin(p->left);
}

struct node * del(avl_any *t, any x, struct node *p)
{
    struct node * n;
    
    // do nothing - data not in tree
    if (p == NULL)
        ;
        
    else if (LT(x,p->item)) 
        p->left = del(t, x, p->left);
        
    else if (LT(p->item,x))
        p->right = del(t, x, p->right);
    
    // found and has two children
    // get smallest in right tree
    // overwrite the item to be deleted
    // del the duplicate item in right tree
    else if (p->left!=NULL && p->right!=NULL)
    {
        n = findmin(p->right);
        p->item = n->item;
        p->right = del(t, p->item, p->right);
    }

    // found and has at most one child    
    else {
        n = p;
        if(p->left == NULL)
            // promote the right tree
            p = p->right;
            
        else if (p->right == NULL)
            // promote the left tree
            p = p->left;

        (t->size)--;  // reduce the count
        SAFE_FREE(n); // reclaim memory
    }
    
    return ensure_balance(p);
}

void avl_any_delete(avl_any *t, any x)
{
    assert(t!=NULL);
    t->root = del(t, x, t->root);
}

int isin(avl_any *t, any x, struct node *p)
{
    if (p==NULL)
        return 0;
        
    else if (LT(x,p->item))
        return isin(t, x, p->left);
        
    else if (LT(p->item,x))
        return isin(t, x, p->right);
        
    else 
        return 1;
}

int avl_any_contains(avl_any *t, any x)
{
    assert(t!=NULL);
    return isin(t,x,t->root);
}

void preorder(struct node *p, void (* item_print)(any item))
{
    if (p!=NULL) {
        item_print(p->item);
        preorder(p->left,item_print);
        preorder(p->right,item_print);
    }
}
void inorder(struct node *p, void (* item_print)(any item))
{
    if (p!=NULL) {
        inorder(p->left,item_print);
        item_print(p->item);
        inorder(p->right,item_print);
    }
}
void postorder(struct node *p, void (* item_print)(any item))
{
    if (p!=NULL) {
        postorder(p->left,item_print);
        postorder(p->right,item_print);
        item_print(p->item);
    }
}

void avl_any_preorder_print(avl_any *t, void (* item_print)(any item))
{
    assert(t!=NULL);
    preorder(t->root, item_print);
}

void avl_any_inorder_print(avl_any *t, void (* item_print)(any item))
{
    assert(t!=NULL);
    inorder(t->root, item_print);
}

void avl_any_postorder_print(avl_any *t, void (* item_print)(any item))
{
    assert(t!=NULL);
    postorder(t->root, item_print);
}

void simple_print(struct node *p, int ind, void (* item_print)(any item))
{
    int i;
    
    if (p!=NULL) {
        simple_print(p->right,ind+4,item_print);
        for(i=0;i<ind;i++) putchar(' ');
        item_print(p->item);
        printf(" (h=%i).... %p", ((int)(p->height)), p);
        putchar('\n');
        simple_print(p->left,ind+4,item_print);
    }
    else {
       for(i=0;i<ind;i++) putchar(' ');
       printf("%p\n", p);
    }
}

void avl_any_simple_print(avl_any *t, void (* item_print)(any item))
{
    assert(t!=NULL);
    simple_print(t->root,0,item_print);
}

any findin (avl_any *t, any x, struct node *p)
{
    if (p==NULL)
        return 0;

    else if (LT(x,p->item))
        return findin(t, x, p->left);

    else if (LT(p->item,x))
        return findin(t, x, p->right);

    else
        return p->item;
}

any avl_any_find (avl_any *t, any x)
{
    assert(t!=NULL);
    return findin (t,x,t->root);
}

void inorder_map(struct node *p, void (* item_process)(any item, any context), any context)
{
    // do not assert p, will be null for leaf nodes
    assert(item_process != NULL);
    // do not assert context - NULL may be valid

    if (p != NULL)
    {
        inorder_map(p->left, item_process, context);
        item_process (p->item, context);
        inorder_map(p->right, item_process, context);
    }
}

void avl_any_inorder_map(avl_any *t, void (* item_process)(any item, any context), any context)
{
    assert(t!=NULL);
    assert(item_process != NULL);
    // do not assert context - NULL may be valid

    inorder_map(t->root, item_process, context);
}

void avl_any_release(avl_any *t)
{
    assert(t != NULL);
    SAFE_FREE(t);
}
