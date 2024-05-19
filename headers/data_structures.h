#ifndef __DATA_STRUCTURES_H__
#define __DATA_STRUCTURES_H__

#include <stdlib.h>

/**
 * @file data_structures.h
 * @brief Dynamic array data structure and related functions.
 */

#define INITIAL_CAPACITY 10

/**
 * @brief Structure representing a dynamic array.
 */
typedef struct {
    void *data;       /**< Pointer to the data stored in the array */
    size_t data_size; /**< Size of each element in the array */
    size_t size;      /**< Number of elements currently in the array */
    size_t capacity;  /**< Maximum number of elements the array can hold before resizing */
} Array;

/**
 * @brief Creates a new dynamic array.
 * 
 * @param data_size Size of each element in the array.
 * @return Array* Pointer to the newly created array, or NULL if memory allocation fails.
 */
Array *create_array(size_t data_size);

/**
 * @brief Retrieves an element from the array at a specified index.
 * 
 * @param array Pointer to the array.
 * @param i Index of the element to retrieve.
 * @return void* Pointer to the element, or NULL if the index is out of bounds.
 */
void *get_from_array(Array *array, size_t i);

/**
 * @brief Sets an element in the array at a specified index.
 * 
 * @param array Pointer to the array.
 * @param element Pointer to the element to set.
 * @param index Index where the element should be set.
 * @return int 0 on success, -1 if the index is out of bounds or memory allocation fails.
 */
int set_in_array(Array *array, void *element, size_t index);

/**
 * @brief Appends an element to the end of the array.
 * 
 * @param array Pointer to the array.
 * @param value Pointer to the element to append.
 * @return int 0 on success, -1 if memory allocation fails.
 */
int append_to_array(Array *array, void *value);

/**
 * @brief Removes an element from the array at a specified index.
 * 
 * @param array Pointer to the array.
 * @param index Index of the element to remove.
 * @return int 0 on success, -1 if the index is out of bounds.
 */
int remove_from_array_at(Array *array, size_t index);

/**
 * @brief Frees the memory allocated for the array.
 * 
 * @param array Pointer to the array to free.
 */
void free_array(Array *array);

#endif /* __DATA_STRUCTURES_H__ */