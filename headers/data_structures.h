#ifndef __DATA_STRUCTURES_H__
#define __DATA_STRUCTURES_H__

#define INITIAL_CAPACITY 10

typedef struct {
    void *data;
    size_t data_size;
    size_t size;
    size_t capacity;
} Array;

Array *create_array(size_t data_size);
void *get_from_array(Array *array, size_t i);
int set_in_array(Array *array, void *element, size_t index);
int append_to_array(Array *array, void *value);
int remove_from_array_at(Array *array, size_t index);
void free_array(Array *array);

#endif