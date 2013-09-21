#include "ds/rbtree.h"
#include <assert.h>

/*
 *  @author:   gaccob <weiye.zhang@qq.com>
 *  @date:     2010-12-16
 *  @function: rbtree header(object container, memcpy object)
 *  @performance:

 gprof results of    100w times insert + find + delete
 Each sample counts as 0.01 seconds.
   %   cumulative    self              self       total
  time     seconds   seconds      calls   s/call   s/call  name
  44.86      1.76      1.76    2000000     0.00     0.00  rbtree_find_node
  24.98      2.74      0.98    1000000     0.00     0.00  rbtree_insert
  17.08      3.42      0.67     76763792     0.00     0.00  cmp(void const*, void const*, void*)
   3.82      3.57      0.15     750940     0.00     0.00  rbtree_delete_adjust
   3.57      3.71      0.14     536020     0.00     0.00  rbtree_min
   1.53      3.77      0.06     999463     0.00     0.00  rbtree_delete_node
   1.27      3.82      0.05     999462     0.00     0.00  rbtree_insert_adjust
   1.27      3.87      0.05          1     0.05     3.90  perf_test(gz::rbtree**, int)
   0.76      3.90      0.03                               func_test(gz::rbtree**)
   0.51      3.92      0.02    1000000     0.00     0.00  rbtree_find
   0.25      3.93      0.01    1000000     0.00     0.00  rbtree_delete
   0.25      3.94      0.01                               frame_dummy
   0.00      3.94      0.00     536020     0.00     0.00  rbtree_next
   0.00      3.94      0.00     516754     0.00     0.00  rbtree_rotl
   0.00      3.94      0.00     488069     0.00     0.00  rbtree_rotr
   0.00      3.94      0.00          1     0.00     0.00  global constructors keyed to _Z4funcPKvPv
   0.00      3.94      0.00          1     0.00     0.00  __static_initialization_and_destruction_0(int, int)
   0.00      3.94      0.00          1     0.00     0.00  rbtree_create
   0.00      3.94      0.00          1     0.00     0.00  rbtree_destroy
  granularity: each sample hit covers 2 byte(s) for 0.25% of 3.94 seconds


 *  @ 1. rbtree is first a binary tree
 *  @    2. each is either red or black
 *  @ 3. root node is black
 *  @ 4. evert red node has only black children
 *  @ 5. from every node to its descendent leaf, there're same black nodes in the path
*/

#define RBTREE_BLACK    0
#define RBTREE_RED      1

typedef struct rbtree_node_t
{
    struct rbtree_node_t* parent;
    struct rbtree_node_t* left;
    struct rbtree_node_t* right;
    void* data;
    int color;
} rbtree_node_t;

typedef struct rbtree_t
{
    rbtree_node_t* root;
    rbtree_cmp cmp;
} rbtree_t;

struct rbtree_t* rbtree_init(rbtree_cmp cmp)
{
    struct rbtree_t* tree = (struct rbtree_t*)MALLOC(sizeof(struct rbtree_t));
    if (!tree) return NULL;
    tree->root = 0;
    tree->cmp = cmp;
    return tree;
}

struct rbtree_node_t* _rbtree_min_node(struct rbtree_node_t* node)
{
    struct rbtree_node_t* p;
    if (node) {
        p = node;
        while(p->left) {
            p = p->left;
        }
        return p;
    }
    return NULL;
}

struct rbtree_node_t* _rbtree_max_node(struct rbtree_node_t* node)
{
    struct rbtree_node_t* p;
    if (node) {
        p = node;
        while(p->right) {
            p = p->right;
        }
        return p;
    }
    return NULL;
}

struct rbtree_node_t* _rbtree_next_node(struct rbtree_node_t* node)
{
    struct rbtree_node_t *source, *dest;
    if (!node) return NULL;

    // min node of right child tree
    if (node->right) {
        return _rbtree_min_node(node->right);
    }
    // or first parent whose left child tree contains node
    source = node;
    dest = node->parent;
    while (dest && source == dest->right) {
        source = dest;
        dest = dest->parent;
    }
    return dest;
}

struct rbtree_node_t* _rbtree_prev_node(struct rbtree_node_t* node)
{
    struct rbtree_node_t *source, *dest;
    if (!node) return NULL;

    // max node of left child tree
    if (node->left) {
        return _rbtree_max_node(node->left);
    }
    // or first parent whose right child tree contains node
    source = node;
    dest = node->parent;
    while (dest && source == dest->left) {
        source = dest;
        dest = dest->parent;
    }
    return dest;
}


/* left rotate, node's right child not null
*
*         |                         |
*          x                         y
*        /   \         -->          / \
*       a     y                    x   c
*            /  \                 / \
*           b    c               a   b
*
*   rotate x
*/
void _rbtree_rotl(struct rbtree_t* tree, struct rbtree_node_t* node)
{
    struct rbtree_node_t *x, *y;
    assert(tree && node);

    x = node;
    y = node->right;

    // node b
    x->right = y->left;
    if (y->left) {
        y->left->parent = x;
    }
    // x & y change
    y->parent = x->parent;
    if (x == tree->root) {
        y->parent = 0;
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

/*
* right rotate, node's left child not null
*                |                  |
*                y                  x
*               /  \     -->      /   \
*              x    c            a     y
*             / \                     /  \
*            a   b                   b    c
* rotate y
*/
void _rbtree_rotr(struct rbtree_t* tree, struct rbtree_node_t* node)
{
    struct rbtree_node_t *x, *y;
    assert(tree && node);

    x = node->left;
    y = node;

    // node b
    y->left = x->right;
    if (x->right) {
        x->right->parent = y;
    }

    // x & y change
    x->parent = y->parent;
    if (y == tree->root) {
        x->parent = 0;
        tree->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    x->right = y;
    y->parent = x;
}


int _rbtree_insert_adjust(struct rbtree_t* tree, struct rbtree_node_t* node)
{
    struct rbtree_node_t *y, *z;
    if (!node || !tree) {
        return -1;
    }

    z = node;
    // z's parent is red, then z's parent is not root => z's parent has parent
    while (z != tree->root && RBTREE_RED == z->parent->color) {
        if (z->parent == z->parent->parent->left) {
            // 1. right uncle exist & uncle is red
            y = z->parent->parent->right;
            if (y && RBTREE_RED == y->color) {
                // uncle & fater is black, grandpa is red
                y->color = RBTREE_BLACK;
                z->parent->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;

                // node <- grandpa
                z = z->parent->parent;
            }
            // uncle not exist or uncle is black
            else {
                // 2. transform to 3.
                if (z == z->parent->right) {
                    z = z->parent;
                    _rbtree_rotl(tree, z);
                }

                // 3. uncle is black & node is left child
                z->parent->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                _rbtree_rotr(tree, z->parent->parent);
            }
        } else {
            // same as above, shift left<->right
            y = z->parent->parent->left;

            // 1. left uncle exist & uncle is red
            if (y && RBTREE_RED == y->color) {
                y->color = RBTREE_BLACK;
                z->parent->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                z = z->parent->parent;
            }
            // uncle not exist or uncle is black
            else {
                // 2. transform to 3.
                if (z == z->parent->left) {
                    z = z->parent;
                    _rbtree_rotr(tree, z);
                }

                // 3. uncle is black & node is left child
                z->parent->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                _rbtree_rotl(tree, z->parent->parent);
            }
        }
    }

    tree->root->color = RBTREE_BLACK;
    return 0;
}


int rbtree_insert(struct rbtree_t* tree, void* data)
{
    struct rbtree_node_t* parent, *dest, *node;
    if (!tree || !data) return -1;

    // root node
    if (!tree->root) {
        tree->root = (struct rbtree_node_t*)MALLOC(sizeof(struct rbtree_node_t));
        if (!tree->root) return -1;
        tree->root->parent = 0;
        tree->root->left = 0;
        tree->root->right = 0;
        tree->root->data = data;
        tree->root->color = RBTREE_BLACK;
        return 0;
    }

    // find dest node's parent, loop down
    parent = tree->root;
    dest = NULL;
    while (parent) {
        dest = parent;
        if (0 == tree->cmp(data, parent->data)) {
            return -1;
        }
        if (tree->cmp(data, parent->data) < 0) {
            parent = parent->left;
        } else {
            parent = parent->right;
        }
    }

    // alloc new node
    node = (struct rbtree_node_t*)MALLOC(sizeof(struct rbtree_node_t));
    if (!node) return -1;
    node->data = data;
    node->parent = dest;
    node->color = RBTREE_RED;
    node->left = 0;
    node->right = 0;

    // no circumstance allows same key in a tree
    if (0 == tree->cmp(data, dest->data)) {
        assert(0);
    }
    if (tree->cmp(data, dest->data) < 0) {
        dest->left = node;
    } else {
        dest->right = node;
    }
    return _rbtree_insert_adjust(tree, node);
}

struct rbtree_node_t* _rbtree_find_node(struct rbtree_t* tree,
                                        struct rbtree_node_t* node,
                                        void* data)
{
    int ret;
    if (!tree || !node || !data) return NULL;

    ret = tree->cmp(data, node->data);
    if (0 == ret) {
        return node;
    } else if (ret < 0) {
        return _rbtree_find_node(tree, node->left, data);
    } else {
        return _rbtree_find_node(tree, node->right, data);
    }
}

void* rbtree_find(struct rbtree_t* tree, void* data)
{
    struct rbtree_node_t* node;
    if (!tree || !data || !tree->root) return NULL;

    node = _rbtree_find_node(tree, tree->root, data);
    if (node) {
        return node->data;
    }
    return NULL;
}

void _rbtree_delete_adjust(struct rbtree_t* tree, struct rbtree_node_t* node, struct rbtree_node_t* parent)
{
    struct rbtree_node_t *x, *w;
    assert(tree);

    x = node;

    // x is deleted & x is BLACK, rebel "from every node to its descendent leaf
    // there're same black nodes in the path"
    while (x != tree->root && (!x || RBTREE_BLACK == x->color)) {
        // x is left child, find right brother
        if (x == parent->left && parent->right) {
            w = parent->right;

            // 1. if brother red, set black, set father red, then do left rotate
            if (RBTREE_RED == w->color) {
                w->color = RBTREE_BLACK;
                parent->color = RBTREE_RED;
                _rbtree_rotl(tree, parent);

                // transform to 2/3/4
                w = parent->right;
            }

            // 2. if brother node black, set red
            if ((!w->left || RBTREE_BLACK == w->left->color)
                && (!w->right || RBTREE_BLACK == w->right->color)) {
                w->color = RBTREE_RED;
                x = parent;
                parent = parent->parent;
            } else {
                // 3. brother's right child black
                if (NULL == w->right || RBTREE_BLACK == w->right->color) {
                    w->left->color = RBTREE_BLACK;
                    w->color = RBTREE_RED;
                    _rbtree_rotr(tree, w);
                    w = parent->right;
                }

                // 4. brother black, right chld red
                w->color = parent->color;
                parent->color = RBTREE_BLACK;
                w->right->color = RBTREE_BLACK;
                _rbtree_rotl(tree, parent);
                x = tree->root;
            }
        } else if (x == parent->right && parent->left) {
            w = parent->left;

            // 1. if brother red, set black, set father red, then do left rotate
            if (RBTREE_RED == w->color) {
                w->color = RBTREE_BLACK;
                parent->color = RBTREE_RED;
                _rbtree_rotr(tree, parent);

                // transform to 2/3/4
                w = parent->left;
            }

            // 2. if brother node black, set red
            if ((!w->left || RBTREE_BLACK == w->left->color)
                && (!w->right || RBTREE_BLACK == w->right->color)) {
                w->color = RBTREE_RED;
                x = parent;
                parent = parent->parent;
            } else {
                // 3. brother's right child black
                if (NULL == w->left || RBTREE_BLACK == w->left->color) {
                    w->right->color = RBTREE_BLACK;
                    w->color = RBTREE_RED;
                    _rbtree_rotl(tree, w);
                    w = parent->left;
                }

                // 4. brother black, right chld red
                w->color = parent->color;
                parent->color = RBTREE_BLACK;
                w->left->color = RBTREE_BLACK;
                _rbtree_rotr(tree, parent);
                x = tree->root;
            }
        }
    }

    if (x) {
        x->color = RBTREE_BLACK;
    }
}

void _rbtree_delete_node(struct rbtree_t* tree, struct rbtree_node_t* node)
{
    struct rbtree_node_t *p, *ch, *next;
    if (!tree || !node) return;

    #if 0
    1. dest node no child (* means dest node)
                  #
                /   \
               #     *
             /   \
            #     #
    #endif
    if (!node->left && !node->right)
    {
        // dest node is root
        if (tree->root == node) {
            FREE(tree->root);
            tree->root = 0;
        }

        // release node & reset p's child
        else {
            p = node->parent;
            if (p->right == node) {
                p->right = 0;
            } else {
                p->left = 0;
            }
            if (RBTREE_BLACK == node->color) {
                _rbtree_delete_adjust(tree, NULL, p);
            }
            FREE(node);
        }
    }

    #if 0
    2. dest node has 2 children, swap node & node's successor and then delete node's successor
                  #
                /   \
               #     *
              /     /  \
             #     #    c
                  /
                 a
                  \
                   b
        a->*,  link c & b
    #endif
    else if (node->left && node->right) {
        next = _rbtree_next_node(node);
        assert(next);
        node->data = next->data;

        // release next (next may have right children)
        p = next->parent;
        ch = next->right;
        if (next == p->left && ch) {
            p->left = ch;
            ch->parent = p;
        } else if (next == p->left && !ch) {
            p->left = 0;
        } else if (next == p->right && ch) {
            p->right = ch;
            ch->parent = p;
        } else if (next == p->right && !ch) {
            p->right = 0;
        }

        // adjust node is replaced deletd node
        if (RBTREE_BLACK == next->color) {
            _rbtree_delete_adjust(tree, ch, p);
        }
        FREE(next);
    }

    #if 0
    3. dest node has one child, then build new relation between n->parent & n->child
                   a
                 /   \
               #      *
            /    \      \
           #     #       b
       link a & b
    #endif
    else {
        p = node->parent;
        ch = 0;
        if (node->left) {
            ch = node->left;
        } else {
            ch = node->right;
        }
        // dest node is root, replace
        if (node == tree->root) {
            node->parent = 0;
            node->color = RBTREE_BLACK;
            node->left = ch->left;
            node->right = ch->right;
            node->data = ch->data;
            if (ch->left) {
                ch->left->parent = node;
            }
            if (ch->right) {
                ch->right->parent = node;
            }
            // adjust b & a
            FREE(ch);
        }

        // release dest node, and link p & ch
        else {
            ch->parent = p;
            if (node == p->left) {
                p->left = ch;
            } else {
                p->right= ch;
            }
            // adjust node is the only child of deleted black node
            if (RBTREE_BLACK == node->color) {
                _rbtree_delete_adjust(tree, ch, p);
            }
            FREE(node);
        }
    }
}

void* rbtree_delete(struct rbtree_t* tree, void* data)
{
    struct rbtree_node_t* node;
    void* res;
    if (!tree || !data || !tree->root) {
        return NULL;
    }
    node = _rbtree_find_node(tree, tree->root, data);
    if (!node) {
        return NULL;
    }
    res = node->data;
    _rbtree_delete_node(tree, node);
    return res;
}

void _rbtree_loop_node(struct rbtree_node_t* node, rbtree_func func)
{
    if (node) {
        if (node->left) {
            _rbtree_loop_node(node->left, func);
        }
        func(node->data);
        if (node->right) {
            _rbtree_loop_node(node->right, func);
        }
    }
}

void rbtree_loop(struct rbtree_t* tree, rbtree_func func)
{
    if (!tree || !tree->root) {
        return;
    }
    _rbtree_loop_node(tree->root, func);
}

void _rbtree_release_node(struct rbtree_node_t* node)
{
    if (node) {
        if (node->left) {
            _rbtree_release_node(node->left);
        }
        if (node->right) {
            _rbtree_release_node(node->right);
        }
        FREE(node);
        node = 0;
    }
}

void rbtree_release(struct rbtree_t* tree)
{
    if (tree) {
        if (tree->root) {
            _rbtree_release_node(tree->root);
        }
        FREE(tree);
    }
}

