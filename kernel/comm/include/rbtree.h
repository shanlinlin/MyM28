#ifndef rbtree__
#define rbtree__

typedef enum {
    RBTREE_COLOR_RED,
    RBTREE_COLOR_BLACK
} rbtree_e_color;

struct rbtree_s_node;
typedef int (*rbtree_p_compare)(struct rbtree_s_node const* node, struct rbtree_s_node const* tree);

typedef struct rbtree_s_root {
    struct rbtree_s_root* self;
    struct rbtree_s_node* node;
    rbtree_p_compare      compare;
} rbtree_s_root;

typedef struct rbtree_s_node {
    struct rbtree_s_node* parent;
    struct rbtree_s_node* left;
    struct rbtree_s_node* right;
    rbtree_e_color        color;
} rbtree_s_node;

void           rbtree_s_root_init(rbtree_s_root* root);
void           rbtree_s_root_set_compare(rbtree_s_root* root, rbtree_p_compare compare);
rbtree_s_node* rbtree_s_root_find_node(rbtree_s_root* root, rbtree_s_node* key);
rbtree_s_node* rbtree_s_root_add_node(rbtree_s_root* root, rbtree_s_node* node);
rbtree_s_node* rbtree_s_root_del_node(rbtree_s_root* root, rbtree_s_node* key);

void rbtree_s_node_init(rbtree_s_node* node);

#endif
