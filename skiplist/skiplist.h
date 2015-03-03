#ifndef SKIPLIST_H
#define SKIPLIST_H

typedef struct node {
    int key;
    struct node *forward[1];
} *node;

typedef struct skiplist {
    int level;
    node header;
} *skiplist;

#define MAX_NUMBER_OF_LEVEL 16
#define MAX_LEVEL (MAX_NUMBER_OF_LEVEL - 1)

#define NEW_NODE_OF_LEVEL(level) (node) malloc(sizeof(struct node) + level * sizeof(node))

#endif
