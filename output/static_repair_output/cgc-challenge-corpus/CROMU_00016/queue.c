#include <stdio.h>    // For printf, puts, getchar
#include <stdlib.h>   // For atoi, malloc, free
#include <string.h>   // For memset
#include <stdint.h>   // For uint8_t, uint32_t, uintptr_t
#include <stdbool.h>  // For bool
#include <math.h>     // For round

// Global variables, inferred types
uint8_t DAT_00018a2c = 0;   // Number of queues
bool DAT_00018a38 = false;  // Is a priority queue? (flag for queue 0)

// Forward declaration of Queue structure
typedef struct Queue Queue;

// DAT_00018a3c is an array of pointers to Queue structures
Queue **DAT_00018a3c = NULL;

// Placeholder for an external double constant. Actual value unknown.
// Assuming a default value that doesn't cause division by zero or extreme results.
double _DAT_00015528 = 1.0;

// Define the Queue structure based on memory offsets observed in the original code
struct Queue {
    uint32_t depth;         // offset 0x00 (local_1c)
    uint32_t unknown1;      // offset 0x04 (unused in provided code)
    uint32_t field_8;       // offset 0x08 (local_30[2])
    uint32_t field_c;       // offset 0x0c (local_30[3], overwritten by local_5c calculation)
    uint32_t _padding[4];   // offsets 0x10, 0x14, 0x18, 0x1c, 0x20 to reach 0x24
    uint8_t min_priority;   // offset 0x24 (char, local_14)
    uint8_t max_priority;   // offset 0x25 (char, local_18)
    uint8_t weight;         // offset 0x26 (byte, local_1d)
    uint8_t _padding2[1];   // offset 0x27
    void *ring_buffer_tail; // offset 0x28 (local_30[10])
    void *ring_buffer_read; // offset 0x2c (local_30[0xb])
    void *ring_buffer_head; // offset 0x30 (local_30[0xc])
}; // Total size 0x34 (52 bytes)

// Stubs for external functions
// readUntil: Reads input from stdin until size chars or delimiter is found.
// Returns number of chars read, or -1 on error (e.g., EOF).
// This is a simplified implementation assuming newline as delimiter and consuming it.
int readUntil(char *buf, int size, char delimiter) {
    if (size <= 0) return 0;
    int c;
    int i = 0;
    while (i < size - 1 && (c = getchar()) != EOF && c != delimiter) {
        buf[i++] = (char)c;
    }
    buf[i] = '\0';
    if (c == EOF) return -1;
    if (c != delimiter && i == size - 1) { // If buffer filled and delimiter not reached, consume rest of line
        while ((c = getchar()) != EOF && c != '\n');
    }
    return i;
}

// allocate: Allocates memory and stores the pointer in *ptr. Returns 0 on success, -1 on failure.
int allocate(size_t size, int flags, void **ptr) {
    *ptr = malloc(size);
    if (*ptr == NULL) {
        perror("Memory allocation failed");
        return -1;
    }
    return 0;
}

// deallocate: Frees memory. Returns 0 on success, -1 on failure (e.g., invalid ptr).
int deallocate(void *ptr, size_t size) { // 'size' parameter is often ignored by free, but kept for signature
    if (ptr != NULL) {
        free(ptr);
        return 0;
    }
    return -1;
}

// Function declarations
int InitRingBuffer(uint32_t depth, void **ring_buffer_head_ptr);
int DestroyRingBuffer(uint8_t queue_index);
int DestroyQueues(void);

// Function: InitQueues
int InitQueues(void) {
    char input_buffer[10];
    int chars_read;
    uint8_t num_queues;        // local_d
    uint8_t current_queue_idx; // local_e
    int min_priority;          // local_14
    int max_priority;          // local_18
    uint32_t queue_depth;      // local_1c
    uint8_t queue_weight;      // local_1d
    uint8_t total_weight_sum = 0; // local_1e
    uint8_t min_queue_weight = 0xff; // local_1f (initialized to max possible byte value)
    int i;                     // local_24 for loops
    char priority_check_sum = '@'; // local_25 (ASCII for '@' is 64)

    // Get number of queues (1-8)
    while (true) {
        printf("How many queues (1-8)?: ");
        chars_read = readUntil(input_buffer, sizeof(input_buffer), '\n');
        if (chars_read == -1) {
            return -1; // Error
        }
        num_queues = (uint8_t)atoi(input_buffer);
        if (num_queues >= 1 && num_queues <= 8) {
            break;
        }
        printf("Invalid number of queues. Please enter a value between 1 and 8.\n");
    }

    DAT_00018a2c = num_queues;
    DAT_00018a38 = false; // Initially no priority queue

    // Allocate array of pointers for queues
    if (allocate(num_queues * sizeof(Queue *), 0, (void **)&DAT_00018a3c) != 0) {
        puts("Failed to allocate interface queue pointers");
        return -1;
    }
    memset(DAT_00018a3c, 0, num_queues * sizeof(Queue *));

    for (current_queue_idx = 0; current_queue_idx < num_queues; current_queue_idx++) {
        printf("Queue %u:\n", current_queue_idx);

        // Check if queue is a priority queue (only for queue 0)
        if (current_queue_idx == 0) {
            memset(input_buffer, 0, sizeof(input_buffer));
            while (true) {
                printf("  Is queue #%u a priority queue (y,n): ", current_queue_idx);
                chars_read = readUntil(input_buffer, 2, '\n'); // Read 'y' or 'n' + newline
                if (chars_read == -1) {
                    return -1;
                }
                if (input_buffer[0] == 'n' || input_buffer[0] == 'y') {
                    break;
                }
                printf("  Invalid input. Please enter 'y' or 'n'.\n");
            }
            if (input_buffer[0] == 'y') {
                DAT_00018a38 = true;
            }
        }

        // Get minimum priority for the queue
        min_priority = -1;
        while (min_priority < 0 || min_priority > 0x3f) { // 0x3f is 63
            printf("  What's the minimum priority packet to place in queue #%u (0-63): ", current_queue_idx);
            chars_read = readUntil(input_buffer, sizeof(input_buffer), '\n');
            if (chars_read == -1) {
                return -1;
            }
            if (chars_read > 0 && input_buffer[0] != '\0') { // Check if input was provided
                min_priority = atoi(input_buffer);
                if (min_priority < 0 || min_priority > 0x3f) {
                    printf("  Invalid priority value. Please enter a value between 0 and 63.\n");
                    min_priority = -1; // Reset for re-prompt
                    continue;
                }

                // Check for priority overlap with previous queues
                for (i = 0; i < current_queue_idx; i++) {
                    Queue *prev_queue = DAT_00018a3c[i];
                    if (prev_queue && min_priority >= prev_queue->min_priority && min_priority <= prev_queue->max_priority) {
                        printf("  That priority value is already assigned to queue %d\n", i);
                        min_priority = -1; // Reset to re-prompt
                        break; // Exit inner loop, re-prompt for min_priority
                    }
                }
            } else {
                min_priority = -1; // No input or empty, re-prompt
            }
        }

        // Get maximum priority for the queue
        max_priority = -1;
        while (max_priority < 0 || max_priority > 0x3f) {
            printf("  What's the maximum priority packet to place in queue #%u (0-63): ", current_queue_idx);
            chars_read = readUntil(input_buffer, sizeof(input_buffer), '\n');
            if (chars_read == -1) {
                return -1;
            }
            if (chars_read > 0 && input_buffer[0] != '\0') {
                max_priority = atoi(input_buffer);
                if (max_priority < 0 || max_priority > 0x3f) {
                    printf("  Invalid priority value. Please enter a value between 0 and 63.\n");
                    max_priority = -1;
                    continue;
                }

                if (max_priority < min_priority) {
                    puts("  Maximum priority must be greater or equal to minimum priority");
                    max_priority = -1; // Reset for re-prompt
                    continue;
                }

                // Check for priority overlap with previous queues
                for (i = 0; i < current_queue_idx; i++) {
                    Queue *prev_queue = DAT_00018a3c[i];
                    if (prev_queue && max_priority >= prev_queue->min_priority && max_priority <= prev_queue->max_priority) {
                        printf("  That priority value is already assigned to queue %d\n", i);
                        max_priority = -1; // Reset to re-prompt
                        break; // Exit inner loop, re-prompt for max_priority
                    }
                }
            } else {
                max_priority = -1; // No input or empty, re-prompt
            }
        }

        // Get queue depth
        queue_depth = 0;
        while (queue_depth == 0 || queue_depth > 0x400) { // 0x400 is 1024
            printf("  What is the depth of queue #%u (1 - 1024 packets): ", current_queue_idx);
            chars_read = readUntil(input_buffer, sizeof(input_buffer), '\n');
            if (chars_read == -1) {
                return -1;
            }
            queue_depth = (chars_read > 0 && input_buffer[0] != '\0') ? (uint32_t)atoi(input_buffer) : 0;
            if (queue_depth == 0 || queue_depth > 0x400) {
                printf("  Invalid queue depth. Please enter a value between 1 and 1024.\n");
            }
        }

        // Get queue weight
        queue_weight = 0;
        // If it's not a priority queue OR it's not the first queue (i.e., not the special case for queue 0)
        if (!DAT_00018a38 || current_queue_idx != 0) {
            while (queue_weight == 0 || queue_weight > 100) {
                printf("  What is the weight of queue #%u (1 - 100 percent): ", current_queue_idx);
                chars_read = readUntil(input_buffer, sizeof(input_buffer), '\n');
                if (chars_read == -1) {
                    return -1;
                }
                uint32_t temp_weight = (chars_read > 0 && input_buffer[0] != '\0') ? (uint32_t)atoi(input_buffer) : 0;
                queue_weight = (uint8_t)temp_weight;

                if (queue_weight == 0 || queue_weight > 100) {
                    printf("  Invalid queue weight. Please enter a value between 1 and 100.\n");
                    continue;
                }

                if ((uint32_t)queue_weight + total_weight_sum > 100) {
                    puts("  Total weight of all queues can not exceed 100%");
                    queue_weight = 0; // Reset for re-prompt
                } else {
                    if (current_queue_idx == num_queues - 1 && ((uint32_t)queue_weight + total_weight_sum != 100)) {
                        puts("  Total weight of all queues must total up to 100%");
                        queue_weight = 0; // Reset for re-prompt
                    }
                }
            }
            if (queue_weight < min_queue_weight) {
                min_queue_weight = queue_weight;
            }
            total_weight_sum += queue_weight;
        } else {
            // For the priority queue (queue 0 if DAT_00018a38 is true), weight is 0.
            queue_weight = 0;
        }

        // Allocate memory for the queue structure
        Queue *new_queue;
        if (allocate(sizeof(Queue), 0, (void **)&new_queue) != 0) {
            printf("  Failed to allocate queue #%u\n", current_queue_idx);
            return -1;
        }
        memset(new_queue, 0, sizeof(Queue));
        DAT_00018a3c[current_queue_idx] = new_queue;

        // Populate queue structure fields
        new_queue->depth = queue_depth;
        new_queue->min_priority = (uint8_t)min_priority;
        new_queue->max_priority = (uint8_t)max_priority;
        new_queue->weight = queue_weight;
        new_queue->field_8 = new_queue->field_c; // Original: local_30[2] = local_30[3];

        // Initialize ring buffer for the queue
        if (InitRingBuffer(queue_depth, &new_queue->ring_buffer_head) != 0) {
            puts("  Failed to allocate pkt ring buffer");
            return -1;
        }
        new_queue->ring_buffer_tail = new_queue->ring_buffer_head; // Original: local_30[10] = local_30[0xc];
        new_queue->ring_buffer_read = new_queue->ring_buffer_head; // Original: local_30[0xb] = local_30[0xc];
    }

    // After all queues are initialized, perform final checks and calculations
    // Validate if all priority values from 0 to 63 are accounted for.
    priority_check_sum = '@'; // ASCII for '@' is 64
    for (i = 0; i < num_queues; i++) {
        Queue *q = DAT_00018a3c[i];
        if (q) {
            // This calculation assumes non-overlapping, contiguous priority ranges
            // that collectively cover 0-63. Each range [min, max] contributes (min - max - 1)
            // to the sum. If all 64 priorities are covered exactly, the sum will be -64.
            // So 64 + (-64) = 0.
            priority_check_sum += (q->min_priority - q->max_priority) - 1;
        }
    }

    if (priority_check_sum != '\0') { // ASCII for '\0' is 0
        printf("Not all priority values from 0 to 63 are accounted for in the queue definitions.\n");
        return -1;
    }

    // Perform final queue configuration based on weights for non-priority queues.
    for (i = 0; i < num_queues; i++) {
        Queue *q = DAT_00018a3c[i];
        // Apply calculation if not a priority queue OR if it's not queue 0 (if it is a priority queue)
        if (q && (!DAT_00018a38 || i != 0)) {
            // Original: local_5c = (undefined4)(longlong)ROUND(((double)(uint)*(byte *)(iVar1 + 0x26) / _DAT_00015528) * (double)(150000 / (ulonglong)(longlong)(int)(uint)local_1f));
            // iVar1 here is the address of the queue struct (q).
            // *(byte *)(iVar1 + 0x26) is q->weight. local_1f is min_queue_weight.
            double weight_ratio = (double)q->weight / _DAT_00015528;
            double factor = 150000.0 / (double)min_queue_weight;
            q->field_c = (uint32_t)round(weight_ratio * factor);
        }
    }

    return 0;
}

// Function: InitRingBuffer
int InitRingBuffer(uint32_t queue_depth, void **ring_buffer_head_ptr) {
    const uint32_t ENTRIES_PER_PAGE = 0xcc; // 204 entries
    const uint32_t ENTRY_SIZE = 0x14;       // 20 bytes per entry
    const uint32_t PAGE_SIZE = 0x1000;      // 4096 bytes per page

    uint32_t num_pages = queue_depth / ENTRIES_PER_PAGE + 1;
    void *current_page_ptr = NULL;
    void *last_entry_in_prev_page = NULL; // Tracks the last entry of the previous page
    uint32_t entries_linked_total = 0;    // Total entries linked across all pages

    *ring_buffer_head_ptr = NULL; // Initialize head pointer to NULL

    for (uint32_t page_idx = 0; page_idx < num_pages; page_idx++) {
        if (allocate(PAGE_SIZE, 0, &current_page_ptr) != 0) {
            puts("Failed to allocate pkt page");
            // Need to clean up previously allocated pages if allocation fails mid-way
            // For simplicity, returning -1 for now as per original code's behavior.
            return -1;
        }
        memset(current_page_ptr, 0, PAGE_SIZE);

        if (page_idx == 0) {
            *ring_buffer_head_ptr = current_page_ptr; // The first page's start is the ring buffer head
        }

        if (last_entry_in_prev_page != NULL) {
            // Link the last entry of the previous page to the first entry of the current page
            *(void **)((char *)last_entry_in_prev_page + 0x10) = current_page_ptr;
        }

        uint32_t entries_in_current_page;
        // Link entries within the current page
        for (entries_in_current_page = 0;
             (entries_in_current_page < ENTRIES_PER_PAGE - 1) && (entries_linked_total < queue_depth - 1);
             entries_in_current_page++) {
            void *current_entry_ptr = (char *)current_page_ptr + entries_in_current_page * ENTRY_SIZE;
            void *next_entry_ptr = (char *)current_page_ptr + (entries_in_current_page + 1) * ENTRY_SIZE;
            *(void **)((char *)current_entry_ptr + 0x10) = next_entry_ptr;
            entries_linked_total++;
        }
        // Update last_entry_in_prev_page to the last entry linked in the current page
        last_entry_in_prev_page = (char *)current_page_ptr + entries_in_current_page * ENTRY_SIZE;
    }

    // After all pages are allocated and entries linked,
    // link the last entry of the very last page back to the head to form a ring.
    if (last_entry_in_prev_page != NULL) {
        *(void **)((char *)last_entry_in_prev_page + 0x10) = *ring_buffer_head_ptr;
    }

    return 0;
}

// Function: DestroyRingBuffer
int DestroyRingBuffer(uint8_t queue_index) {
    const uint32_t ENTRIES_PER_PAGE = 0xcc;
    const uint32_t PAGE_SIZE = 0x1000;

    if (queue_index >= DAT_00018a2c) {
        return -1; // Invalid queue index
    }

    Queue *q = DAT_00018a3c[queue_index];
    if (q == NULL || q->ring_buffer_head == NULL) {
        return 0; // Nothing to destroy or already destroyed
    }

    void *current_page_start = q->ring_buffer_head;
    uint32_t pages_to_free = q->depth / ENTRIES_PER_PAGE + 1;
    uint32_t pages_freed = 0;

    while (pages_freed < pages_to_free) {
        void *page_to_deallocate = current_page_start;
        void *walker = current_page_start;

        // Traverse the linked list of entries to find the start of the next page.
        // The loop advances 'walker' until it finds the last entry of the current page.
        // The last entry of a page points to the first entry of the next page,
        // or back to the head if it's the last page in the ring.
        while (true) {
            void *next_entry_ptr = *(void **)((char *)walker + 0x10);

            // Condition 1: If the next entry is the head of the ring buffer,
            // we've traversed the entire ring and 'walker' is the last entry before wrapping.
            if (next_entry_ptr == q->ring_buffer_head) {
                current_page_start = q->ring_buffer_head; // Will cause loop to terminate if only one page or all pages processed
                break;
            }

            // Condition 2: If the current entry ('walker') and the next entry ('next_entry_ptr')
            // are in different 0x1000 memory blocks, it indicates a page boundary.
            // 'walker' is the last entry of the current page, and 'next_entry_ptr' is the start of the next page.
            if (((uintptr_t)next_entry_ptr ^ (uintptr_t)walker) > 0xfff) {
                current_page_start = next_entry_ptr; // Update for the next iteration
                break;
            }

            // If neither condition met, continue traversing within the current page.
            walker = next_entry_ptr;
        }

        deallocate(page_to_deallocate, PAGE_SIZE);
        pages_freed++;
    }

    // Clear queue ring buffer pointers after all pages are freed
    q->ring_buffer_head = NULL;
    q->ring_buffer_tail = NULL;
    q->ring_buffer_read = NULL;

    return 0;
}

// Function: DestroyQueues
int DestroyQueues(void) {
    for (uint8_t i = 0; i < DAT_00018a2c; i++) {
        if (DestroyRingBuffer(i) != 0) {
            // If DestroyRingBuffer fails, return error immediately as per original logic.
            return -1;
        }
        if (DAT_00018a3c[i] != NULL) {
            deallocate(DAT_00018a3c[i], sizeof(Queue));
            DAT_00018a3c[i] = NULL; // Clear pointer after freeing
        }
    }

    if (DAT_00018a3c != NULL) {
        deallocate(DAT_00018a3c, (size_t)DAT_00018a2c * sizeof(Queue *));
        DAT_00018a3c = NULL; // Clear array pointer
    }

    DAT_00018a2c = 0;   // Reset number of queues
    DAT_00018a38 = false; // Reset priority queue flag

    return 0;
}

// Main function placeholder for compilation and basic demonstration
int main() {
    printf("Initializing queues...\n");
    if (InitQueues() == 0) {
        printf("Queues initialized successfully.\n");
        // In a real application, you would perform operations with the queues here.
        // For demonstration, we just destroy them immediately.
        printf("Destroying queues...\n");
        if (DestroyQueues() == 0) {
            printf("Queues destroyed successfully.\n");
        } else {
            printf("Failed to destroy queues.\n");
        }
    } else {
        printf("Failed to initialize queues.\n");
    }
    return 0;
}