#include "hash_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct pair_list *find_at(const struct hash_map *hash_map,
                                 size_t hash_value, const char *key)
{
    size_t index = hash_value % hash_map->size;
    struct pair_list *pair = hash_map->data[index];

    while (pair && strcmp(pair->key, key) != 0)
        pair = pair->next;

    return pair;
}

struct hash_map *hash_map_init(size_t size)
{
    struct hash_map *new_hash_map = malloc(sizeof(struct hash_map));
    if (!new_hash_map)
        return NULL;
    struct pair_list **pair_list = calloc(size, sizeof(struct pair_list *));

    if (!pair_list)
    {
        free(new_hash_map);
        return NULL;
    }

    new_hash_map->size = size;
    new_hash_map->data = pair_list;
    return new_hash_map;
}

bool hash_map_insert(struct hash_map *hash_map, const char *key, char *value,
                     bool *updated)
{
    if (!hash_map || hash_map->size == 0 || !key)
        return false;
    size_t hash_value = hash(key);
    size_t index = hash_value % hash_map->size;

    // Try to find existing key
    struct pair_list *existing = find_at(hash_map, hash_value, key);
    if (existing)
    {
        if (updated)
            *updated = true;
        existing->value = value;
        return true;
    }

    struct pair_list *new_pair_list = malloc(sizeof(struct pair_list));
    if (!new_pair_list)
        return false;
    new_pair_list->key = key;
    new_pair_list->value = value;
    new_pair_list->next = hash_map->data[index];

    hash_map->data[index] = new_pair_list;
    if (updated)
        *updated = false;

    return true;
}

void hash_map_free(struct hash_map *hash_map)
{
    if (hash_map == NULL)
        return;
    for (size_t i = 0; i < hash_map->size; i++)
    {
        struct pair_list *current = hash_map->data[i];
        while (current != NULL)
        {
            struct pair_list *to_free = current;
            current = current->next;
            free(to_free->value);
            free(to_free);
        }
    }
    free(hash_map->data);
    free(hash_map);
}

void hash_map_dump(struct hash_map *hash_map)
{
    if (!hash_map)
        return;

    for (size_t i = 0; i < hash_map->size; i++)
    {
        struct pair_list *first = hash_map->data[i];
        if (!first)
            continue;

        // Print first without comma
        printf("%s: %s", first->key, first->value);

        // Then the rest with preceding comma
        for (struct pair_list *current = first->next; current;
             current = current->next)
        {
            printf(", %s: %s", current->key, current->value);
        }
        putchar('\n');
    }
}

const char *hash_map_get(const struct hash_map *hash_map, const char *key)
{
    if (hash_map == NULL || key == NULL)
    {
        return NULL;
    }
    size_t hash_value = hash(key);
    size_t index = hash_value % hash_map->size;
    struct pair_list *current = hash_map->data[index];
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

bool hash_map_remove(struct hash_map *hash_map, const char *key)
{
    if (hash_map == NULL || key == NULL)
    {
        return false;
    }
    size_t hash_value = hash(key);
    size_t index = hash_value % hash_map->size;
    struct pair_list *current = hash_map->data[index];
    struct pair_list *prev = NULL;
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            if (prev == NULL)
            {
                hash_map->data[index] = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            free(current);
            return true;
        }

        prev = current;
        current = current->next;
    }
    return false;
}
