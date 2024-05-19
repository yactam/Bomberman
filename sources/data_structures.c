#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/data_structures.h"
#include "../headers/debug.h"

Array *create_array(size_t data_size) {
    Array *array = malloc(sizeof(Array));
    if (array == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }
    array->data_size = data_size;
    array->data = malloc(INITIAL_CAPACITY * data_size);
    if (array->data == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }
    array->size = 0;
    array->capacity = INITIAL_CAPACITY;
    return array;
}

void *get_from_array(Array *array, size_t i) {
    if(i > array->size) {
        return NULL; 
    }

    return (char *)array->data + (i * array->data_size);
}

int set_in_array(Array *array, void *element, size_t i) {
    if (i >= array->capacity) {
        size_t new_capacity = i + 1;
        void *new_data = realloc(array->data, new_capacity * array->data_size);
        if (new_data == NULL) {
            perror("Memory reallocation failed");
            return -1;
        }
        array->data = new_data;
        array->capacity = new_capacity;
    }

    memcpy((char *)array->data + (i * array->data_size), element, array->data_size);
    if (i >= array->size)
        array->size = i + 1;

    return 0;
}

int append_to_array(Array *array, void *value) {
    if (array->size >= array->capacity) {
        array->capacity *= 2;
        void *new_data = realloc(array->data, array->capacity * array->data_size);
        if (new_data == NULL) {
            perror("Memory reallocation failed");
            return -1;
        }
        array->data = new_data;
    }
    memcpy((char *)array->data + (array->size * array->data_size), value, array->data_size);
    array->size++;
    return 0;
}

int remove_from_array_at(Array *array, size_t index) {
    if (index >= array->size) {
        fprintf(stderr, "Index out of bounds\n");
        return -1;
    }
    memmove((char *)array->data + (index * array->data_size),
            (char *)array->data + ((index + 1) * array->data_size),
            (array->size - index - 1) * array->data_size);
    array->size--;
    return 0;
}

void free_array(Array *array) {
    free(array->data);
    free(array);
}
