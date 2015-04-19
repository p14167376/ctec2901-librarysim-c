#ifndef AVL_any_H
#define AVL_any_H

// Auto-generated header file for an avl of any
// values.  This avl has no exception handling but
// prints a message on stdout and calls exit(1).
// Author: drs.

typedef struct avl_any_implementation avl_any;
#include "any.h"
typedef int (*rel_func)(any a, any b);

avl_any * new_avl_any(rel_func lt);
int  avl_any_isempty(avl_any *t);
int  avl_any_isfull(avl_any *t);
int  avl_any_size(avl_any *t);
int  avl_any_depth(avl_any *t);
void avl_any_insert(avl_any *t, any x);
void avl_any_delete(avl_any *t, any x);
int  avl_any_contains(avl_any *t, any x);
void avl_any_preorder_print(avl_any *t, void (* item_print)(any item));
void avl_any_inorder_print(avl_any *t, void (* item_print)(any item));
void avl_any_postorder_print(avl_any *t, void (* item_print)(any item));
void avl_any_simple_print(avl_any *t, void (* item_print)(any item));

any  avl_any_find        (avl_any *t, any key);
void avl_any_inorder_map (avl_any *t,
	                      void (* item_process)(any item, any context),
	                      any context);

void avl_any_release(avl_any *t);
#endif
