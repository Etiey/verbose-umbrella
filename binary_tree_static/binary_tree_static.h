#ifndef BINARY_TREE_STATIC_H
#define BINARY_TREE_STATIC_H

#include <stddef.h>

#define CAPACITY 128

struct value
{
    int val;
};

struct binary_tree
{
    size_t max_index;
    struct value *data[CAPACITY];
};

size_t size(const struct binary_tree *tree);
int height(const struct binary_tree *tree);
void dfs_print_prefix(const struct binary_tree *tree);
void bfs_print(const struct binary_tree *tree);
int is_complete(const struct binary_tree *tree);
int is_perfect(const struct binary_tree *tree);

#endif /* ! BINARY_TREE_STATIC_H */
