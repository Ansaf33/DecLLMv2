#include <stdio.h>   // For fprintf, stderr, printf
#include <stdlib.h>  // For calloc, free, exit, EXIT_FAILURE
#include <string.h>  // For memcpy
#include <stdint.h>  // For uint32_t

// --- Type Definitions ---
typedef unsigned int uint;
typedef uint32_t undefined4;
// `undefined` type is present in the original snippet but not used in the final logic.
// Keeping it for completeness as requested.
typedef unsigned char undefined; 

// --- Queue Structure Definition ---
// Based on the array accesses (param_1[0], param_1[1], param_1[2], param_1[3]),
// we deduce the structure of the queue.
typedef struct {
    uint capacity;     // Corresponds to param_1[0] or `*param_1`
    uint count;        // Corresponds to param_1[1]
    int peek_index;    // Corresponds to param_1[2]; -1 indicates not cached
    undefined4* data;  // Corresponds to param_1[3]; pointer to the dynamically allocated array
} Queue;

// --- Helper Functions ---

// xcalloc: A wrapper around calloc that exits the program on memory allocation failure.
void* xcalloc(size_t num, size_t size) {
    void* ptr = calloc(num, size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: xcalloc failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// _terminate: Terminates the program, typically used for unrecoverable errors.
void _terminate() {
    exit(EXIT_FAILURE);
}

// --- Queue Operations ---

// InitializeQueue: Sets up a new Queue instance with initial capacity and state.
void InitializeQueue(Queue *q) {
  if (q == NULL) {
    fprintf(stderr, "Error: InitializeQueue received a NULL queue pointer.\n");
    _terminate();
  }
  q->count = 0;
  q->capacity = 2; // Initial capacity
  q->data = (undefined4*)xcalloc(q->capacity, sizeof(undefined4)); // Allocate initial data array
  q->peek_index = -1; // No cached peek index initially
}

// FreeQueue: Releases all dynamically allocated memory associated with the Queue.
void FreeQueue(Queue *q) {
  if (q != NULL) {
    if (q->data != NULL) {
      free(q->data);
      q->data = NULL; // Prevent use-after-free
    }
    // The original code `free(param_1)` suggests the Queue structure itself
    // was dynamically allocated.
    free(q);
  }
}

// ShiftQueueElementsToFront: Compacts the queue by moving all non-zero elements
// to the beginning of the `data` array and filling the vacated slots with zeros.
// The return type `undefined4*` is unusual for a function that modifies in place;
// it's likely a decompiler artifact. Returning NULL is a common pattern for success.
undefined4* ShiftQueueElementsToFront(Queue *q) {
  if (q == NULL) {
    // Original returned `&_GLOBAL_OFFSET_TABLE_` which is not a valid C return for this context.
    // Returning NULL as a generic success/error indicator.
    return NULL;
  }

  uint first_empty_slot_idx;
  uint first_non_empty_element_idx;

  // This loop repeatedly finds the first empty slot and the first non-empty element
  // after it, then swaps them, until no more such pairs can be found.
  do {
    first_empty_slot_idx = q->capacity; // Sentinel: value greater than any valid index
    first_non_empty_element_idx = q->capacity;

    // Find the first '0' (empty slot) within the current logical `count` of elements.
    for (uint i = 0; i < q->count; ++i) {
      if (q->data[i] == 0) {
        first_empty_slot_idx = i;
        break;
      }
    }

    // If no empty slot was found, the queue is already compacted or empty of zeros.
    if (first_empty_slot_idx == q->capacity) {
      break; // Exit the do-while loop
    }

    // Find the first non-zero element *after* the `first_empty_slot_idx`,
    // also within the logical `count` of elements.
    for (uint i = first_empty_slot_idx + 1; i < q->count; ++i) {
      if (q->data[i] != 0) {
        first_non_empty_element_idx = i;
        break;
      }
    }

    // If no non-zero element was found after the first empty slot, no more compaction is possible.
    if (first_non_empty_element_idx == q->capacity) {
      break; // Exit the do-while loop
    }

    // Perform the swap: move the non-empty element to the empty slot.
    q->data[first_empty_slot_idx] = q->data[first_non_empty_element_idx];
    q->data[first_non_empty_element_idx] = 0; // Clear the old position

    q->peek_index = -1; // Invalidate cached peek index as queue structure changed

  } while (1); // The loop continues until a `break` condition is met.

  // The original function's return value is problematic. Returning NULL for success.
  return NULL;
}

// Enqueue: Adds a `value` to the end of the queue. Expands capacity if necessary.
void Enqueue(Queue *q, undefined4 value) {
  if (q == NULL) {
    fprintf(stderr, "Error: Enqueue received a NULL queue pointer.\n");
    _terminate();
  }

  // The original check `if (*(int *)(param_1[1] * 4 + param_1[3]) != 0)`
  // corresponds to `if (q->data[q->count] != 0)`.
  // This acts as an assertion that the slot where we are about to enqueue is empty.
  // For a typical queue, `q->count` points to the next available slot, which should be empty.
  if (q->count < q->capacity && q->data[q->count] != 0) {
    fprintf(stderr, "Error: Attempting to enqueue into an occupied slot at index %u. Queue state might be corrupt.\n", q->count);
    _terminate();
  }

  // If the queue is full (count equals capacity), expand its capacity.
  if (q->count == q->capacity) {
    uint new_capacity = q->capacity * 2;
    // Check for potential integer overflow or if new_capacity is not actually larger.
    if (new_capacity <= q->capacity) {
      fprintf(stderr, "Error: Queue capacity overflow or invalid size during expansion.\n");
      _terminate();
    }

    undefined4* new_data = (undefined4*)xcalloc(new_capacity, sizeof(undefined4));
    memcpy(new_data, q->data, q->count * sizeof(undefined4)); // Copy existing elements to new array
    free(q->data); // Free the old, smaller data array
    q->data = new_data; // Update the data pointer to the new, larger array
    q->capacity = new_capacity; // Update the queue's capacity
  }

  q->data[q->count] = value; // Place the new element at the current `count` index
  q->count++; // Increment the number of elements in the queue
  q->peek_index = -1; // Invalidate cached peek index
}

// Dequeue: Removes and returns the first non-zero element from the queue.
// Returns 0 if the queue is empty or an error occurs.
undefined4 Dequeue(Queue *q) {
  undefined4 dequeued_value = 0;
  q->peek_index = -1; // Invalidate cached peek index

  if (q == NULL) {
    return 0; // Error: NULL queue pointer
  }

  int found_idx = -1; // Use -1 as a sentinel to indicate no element was found
  // Iterate through the current elements to find the first non-zero one.
  for (uint i = 0; i < q->count; ++i) {
    if (q->data[i] != 0) {
      dequeued_value = q->data[i]; // Store the value to be returned
      q->data[i] = 0; // Mark the slot as empty
      found_idx = i; // Record the index where the element was found
      break; // Exit loop after finding and processing the first element
    }
  }

  // The original condition `if ((local_c & 0xff) == 0xff)` is ambiguous for `uint local_c`.
  // Interpreting it as a check for whether an element was found.
  // If `found_idx` is still -1, it means no non-zero element was found in the queue.
  if (found_idx == -1) {
    ShiftQueueElementsToFront(q); // Compact the queue, cleaning up any zeros at the front.
  }

  return dequeued_value; // Returns the dequeued value, or 0 if nothing was dequeued.
}

// Peek: Returns the first non-zero element from the queue without removing it.
// Returns 0 if the queue is empty or an error occurs.
undefined4 Peek(Queue *q) {
  if (q == NULL) {
    return 0; // Error: NULL queue pointer
  }

  // If `peek_index` is cached and points to a valid, non-zero element within bounds, return it.
  if (q->peek_index != -1 && (uint)q->peek_index < q->count && q->data[q->peek_index] != 0) {
    return q->data[q->peek_index];
  } else {
    // Otherwise, search for the first non-zero element.
    for (uint i = 0; i < q->count; ++i) {
      if (q->data[i] != 0) {
        q->peek_index = i; // Cache the index of the found element
        return q->data[i];
      }
    }
    // If no non-zero element is found after searching.
    q->peek_index = -1; // Reset peek index as nothing was found.
    return 0; // Return 0 to indicate no element to peek.
  }
}

// --- Main function for demonstration and testing ---
int main() {
    // Dynamically allocate the Queue structure itself, as implied by FreeQueue.
    Queue *my_queue = (Queue*)xcalloc(1, sizeof(Queue));
    InitializeQueue(my_queue);

    printf("Queue initialized. Capacity: %u, Count: %u\n", my_queue->capacity, my_queue->count);

    Enqueue(my_queue, 10);
    Enqueue(my_queue, 20);
    Enqueue(my_queue, 30);
    printf("Enqueued 10, 20, 30. Current Capacity: %u, Current Count: %u\n", my_queue->capacity, my_queue->count);

    printf("Peek: %u\n", Peek(my_queue)); // Expected: 10

    printf("Dequeue: %u\n", Dequeue(my_queue)); // Expected: 10
    printf("Peek after dequeue: %u\n", Peek(my_queue)); // Expected: 20

    Enqueue(my_queue, 40);
    printf("Enqueued 40. Current Capacity: %u, Current Count: %u\n", my_queue->capacity, my_queue->count);

    printf("Dequeue: %u\n", Dequeue(my_queue)); // Expected: 20
    printf("Dequeue: %u\n", Dequeue(my_queue)); // Expected: 30
    printf("Dequeue: %u\n", Dequeue(my_queue)); // Expected: 40

    printf("Dequeue (empty): %u\n", Dequeue(my_queue)); // Expected: 0 (queue is now logically empty)
    printf("Dequeue (empty, after compact): %u\n", Dequeue(my_queue)); // Expected: 0

    Enqueue(my_queue, 50);
    Enqueue(my_queue, 60);
    printf("Enqueued 50, 60. Current Capacity: %u, Current Count: %u\n", my_queue->capacity, my_queue->count);

    printf("Peek: %u\n", Peek(my_queue)); // Expected: 50

    FreeQueue(my_queue); // Free all allocated memory
    printf("Queue freed.\n");

    return 0;
}