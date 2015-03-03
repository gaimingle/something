#include <stdio.h>
#include <stdlib.h>
#include "skiplist.h"

int randLevel()
{
    int level = 0;
    while ((rand() & 0xFFFF) < (0.5 * 0xFFFF)) {
        level += 1;
    }
    return (level < MAX_LEVEL) ? level : MAX_LEVEL;
}

skiplist createSkiplist()
{
    int i;
    skiplist list;

    list = (skiplist) malloc(sizeof(struct skiplist));
    list->level = -1;
    list->header = NEW_NODE_OF_LEVEL(MAX_LEVEL);

    for (i = 0; i < MAX_NUMBER_OF_LEVEL; i++) {
        list->header->forward[i] = NULL;
    }

    return list;
}

void insert(skiplist list, int key)
{
    int i, level;
    node p;
    node update[MAX_NUMBER_OF_LEVEL];

    p = list->header;
    /* must from up to down */
    for (i = list->level; i >= 0; i--) {
        while (p->forward[i] && p->forward[i]->key < key)
            p = p->forward[i];
        update[i] = p;
    }

    /* duplicated key? */
    if (p->forward[0] && p->forward[0]->key == key) {
        fprintf(stderr, "key %d already exists\n", key);
        return;
    }

    /* generate random level */
    level = randLevel();
    if (level > list->level) {
        for (i = level; i > list->level; i--) {
            update[i] = list->header;
        }
        list->level = level;
    }

    p = NEW_NODE_OF_LEVEL(level);
    p->key = key;
    /* update pointer */
    for (i = 0; i <= level; i++) {
        p->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = p;
    }
}

void delete(skiplist list, int key)
{
    int i;
    node p;
    node update[MAX_NUMBER_OF_LEVEL];

    p = list->header;
    /* must from up to down */
    for (i = list->level; i >= 0; i--) {
        while (p->forward[i] && p->forward[i]->key < key)
            p = p->forward[i];
        update[i] = p;
    }

    /* key exists? */
    if (!p->forward[0] || p->forward[0]->key != key) {
        fprintf(stderr, "key %d doesn't exist\n", key);
        return;
    }

    /* update pointer */
    p = p->forward[0];
    for (i = 0; i <= list->level && update[i]->forward[i] == p; i++)
        update[i]->forward[i] = p->forward[i];
    free(p);

    /* update level */
    for (i = list->level; i >= 0 && !list->header->forward[i]; i--)
        list->level--;
}

void print(skiplist list, int level)
{
    node p;

    p = list->header;
    for (; p->forward[level] != NULL; p = p->forward[level])
        printf("%d -> ", p->forward[level]->key);

    printf("NULL\n");
}

int main() 
{
    skiplist list = createSkiplist();

    insert(list, 3);
    insert(list, 7);
    insert(list, 4);
    print(list, 0);
    insert(list, 4);
    insert(list, 6);
    print(list, 0);
    print(list, 0);
    delete(list, 100);
    delete(list, 3);
    print(list, 0);
    delete(list, 6);
    delete(list, 4);
    delete(list, 7);
    print(list, 0);
    return 0;
}
