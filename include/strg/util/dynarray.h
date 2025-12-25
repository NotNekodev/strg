#ifndef STRG_DYNARRAY_H
#define STRG_DYNARRAY_H

#define DYNARRAY_START_SIZE 8
#define DYNARRAY_GROWTH_FACTOR 2 // so it grows by 2 every time it needs to grow

#define DYNARRAY_FOREACH(array, index, elem)                  \
void *elem; \
for (int index = 0;                                       \
index < (array)->size &&                             \
((elem) = (array)->items[index], 1);                 \
++index)


struct dynarray {
    void **items;
    int capacity;
    int size;
};

struct dynarray *dynarray_create(void);
void dynarray_destroy(struct dynarray *array);
int dynarray_add(struct dynarray *array, void *item);
int dynarray_add_at_index(struct dynarray *array, void *item, int index);
void *dynarray_get(struct dynarray *array, int index);
int dynarray_size(struct dynarray *array);
void *dynarray_remove(struct dynarray *array, int index);

#endif //STRG_DYNARRAY_H