#include <stdlib.h>

#include <strg/util/dynarray.h>

struct dynarray *dynarray_create() {
    struct dynarray *array = malloc(sizeof(struct dynarray));
    if (!array) {
        return NULL;
    }
    array->capacity = DYNARRAY_START_SIZE;
    array->size = 0;
    array->items = malloc(sizeof(void *) * array->capacity);
    if (!array->items) {
        free(array);
        return NULL;
    }
    return array;
}

void dynarray_destroy(struct dynarray *array) {
    // free all elements
    for (int i = 0; i < array->size; i++) {
        free(array->items[i]); // this ONLY works, because WE ONLY IN THIS PROJECT allocate all items with malloc :3
    }
    free(array->items);
    free(array);
}

int dynarray_add(struct dynarray *array, void *item) {
    if (array->size >= array->capacity) {
        int new_capacity = array->capacity * DYNARRAY_GROWTH_FACTOR;
        void **new_items = realloc(array->items, sizeof(void *) * new_capacity);
        if (!new_items) {
            return -1;
        }
        array->items = new_items;
        array->capacity = new_capacity;
    }
    array->items[array->size] = item;
    array->size++;
    return 0;
}

int dynarray_add_at_index(struct dynarray *array, void *item, int index) {
    if (index < 0 || index > array->size) {
        return -1;
    }
    if (array->size >= array->capacity) {
        int new_capacity = array->capacity * DYNARRAY_GROWTH_FACTOR;
        void **new_items = realloc(array->items, sizeof(void *) * new_capacity);
        if (!new_items) {
            return -1;
        }
        array->items = new_items;
        array->capacity = new_capacity;
    }
    for (int i = array->size; i > index; i--) {
        array->items[i] = array->items[i - 1];
    }
    array->items[index] = item;
    array->size++;
    return 0;
}

void *dynarray_remove(struct dynarray *array, int index) {
    if (index < 0 || index >= array->size) {
        return NULL;
    }

    void *removed_item = array->items[index];

    free(removed_item); // again, this ONLY works, because WE ONLY IN THIS PROJECT allocate all items with malloc :3

    for (int i = index; i < array->size - 1; i++) {
        array->items[i] = array->items[i + 1];
    }

    array->size--;
    return removed_item;
}

void *dynarray_get(struct dynarray *array, int index) {
    if (index < 0 || index >= array->size) {
        return NULL;
    }
    return array->items[index];
}

int dynarray_size(struct dynarray *array) {
    return array->size;
}
