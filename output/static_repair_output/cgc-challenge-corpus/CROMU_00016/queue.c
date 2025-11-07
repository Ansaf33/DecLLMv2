#include <stdio.h>   // For printf, puts, fgets, stdin
#include <stdlib.h>  // For atoi, malloc, free
#include <string.h>  // For memset, strlen
#include <stdbool.h> // For bool type
#include <math.h>    // For round
#include <limits.h>  // For UCHAR_MAX

// --- Custom/External Function Definitions ---

// Placeholder for `readUntil`. Original takes buffer, max_len (excluding null terminator), and terminator char.
// Returns number of characters read (excluding terminator), or -1 on error.
static int readUntil(char *buffer, int max_len, char terminator) {
    if (fgets(buffer, max_len + 1, stdin) == NULL) {
        return -1; // Error or EOF
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == terminator) {
        buffer[len - 1] = '\0'; // Remove trailing newline
        len--;
    }
    return (int)len;
}

// Placeholder for `allocate`. Original returns 0 on success, non-zero on failure.
static int allocate(unsigned int size, int flags, void **out_ptr) {
    (void)flags; // flags parameter is unused in this simplified version
    *out_ptr = malloc(size);
    return (*out_ptr == NULL) ? 1 : 0; // 0 for success, 1 for failure
}

// Placeholder for `deallocate`. Original takes ptr and size (size often ignored by free).
static void deallocate(void *ptr, unsigned int size) {
    (void)size; // size parameter is unused in standard free
    free(ptr);
}

// `bzero` is deprecated; `memset` is the standard equivalent.
#define bzero(ptr, size) memset(ptr, 0, size)

// `ROUND` looks like a simple rounding function.
#define ROUND(x) round(x)

// --- Global Variables (Original names preserved as per request) ---
static unsigned char DAT_00018a2c; // Number of queues
static char DAT_00018a38;         // Is priority queue flag (0 or 1)
static void** DAT_00018a3c;       // Array of pointers to queue structures
static double _DAT_00015528 = 1.0; // Placeholder value; actual value unknown from snippet.
                                   // Assuming it's a divisor, 1.0 is a safe default to avoid division by zero.

// --- Structure Definitions (Inferred from memory access patterns) ---

// RingBufferSlot structure, 0x14 bytes (20 bytes)
typedef struct RingBufferSlot {
    char data[16];   // Placeholder for packet data or other fields
    void* next_slot; // Pointer to the next slot in the ring buffer (+0x10 offset)
} RingBufferSlot;

// Queue structure, 0x34 bytes (52 bytes)
typedef struct Queue {
    unsigned int depth;                 // +0x00 (local_30[0])
    unsigned int field_04;              // +0x04 (local_30[1], unused in snippet)
    void* current_pkt_ptr;              // +0x08 (local_30[2]) - set to head_ring_buffer_ptr
    unsigned int calculated_val;        // +0x0C (local_30[3])
    char field_10[0x24 - 0x10];         // +0x10 to +0x23 - padding/other fields
    char min_priority;                  // +0x24 (char at local_30 + 9*sizeof(uint))
    char max_priority;                  // +0x25
    unsigned char weight;               // +0x26
    char field_27[0x28 - 0x27];         // +0x27 - padding
    void* tail_ring_buffer_ptr;         // +0x28 (local_30[10])
    void* current_ring_buffer_ptr;      // +0x2C (local_30[0xb])
    void* head_ring_buffer_ptr;         // +0x30 (local_30[0xc]) - Passed to InitRingBuffer
} Queue;


// Function: InitRingBuffer
int InitRingBuffer(unsigned int param_1, void **param_2) {
    const unsigned int SLOTS_PER_PAGE_CONST = 204; // 0xCC from original code
    unsigned int num_pages = param_1 / SLOTS_PER_PAGE_CONST + 1;
    void* last_slot_in_prev_page = NULL;
    unsigned int total_slots_linked = 0;

    for (unsigned int page_idx = 0; page_idx < num_pages; ++page_idx) {
        void* current_page_mem;
        if (allocate(0x1000, 0, &current_page_mem) != 0) {
            puts("Failed to allocate pkt page");
            return -1; // Return -1 for error
        }
        bzero(current_page_mem, 0x1000);

        if (page_idx == 0) {
            *param_2 = current_page_mem; // Set the head of the entire ring buffer
        }
        if (last_slot_in_prev_page != NULL) {
            // Link the last slot of the previous page to the first slot of the current page
            *(void**)((char*)last_slot_in_prev_page + 0x10) = current_page_mem;
        }

        void* current_slot_ptr = current_page_mem;
        // Link slots sequentially within the current page
        for (unsigned int slot_idx_in_page = 0;
             slot_idx_in_page < SLOTS_PER_PAGE_CONST - 1 && total_slots_linked < param_1 - 1;
             ++slot_idx_in_page) {
            *(void**)((char*)current_slot_ptr + 0x10) = (char*)current_slot_ptr + 0x14;
            current_slot_ptr = (char*)current_slot_ptr + 0x14;
            total_slots_linked++;
        }
        last_slot_in_prev_page = current_slot_ptr; // Store the last linked slot in this page
    }
    // Link the very last slot of the entire ring buffer back to its head
    if (last_slot_in_prev_page != NULL) {
        *(void**)((char*)last_slot_in_prev_page + 0x10) = *param_2;
    }
    return 0; // Return 0 for success
}

// Function: DestroyRingBuffer
int DestroyRingBuffer(unsigned char queue_index) {
    if (queue_index >= DAT_00018a2c) {
        return -1;
    }

    Queue* q_ptr = (Queue*)DAT_00018a3c[queue_index];
    unsigned int total_depth = q_ptr->depth;
    void* head_ring_buffer_ptr = q_ptr->head_ring_buffer_ptr;

    const unsigned int SLOTS_PER_PAGE_CONST = 204;
    unsigned int num_pages = total_depth / SLOTS_PER_PAGE_CONST + 1;

    void* current_page_base = head_ring_buffer_ptr;
    
    for (unsigned int page_idx = 0; page_idx < num_pages; ++page_idx) {
        void* page_to_deallocate = current_page_base;
        void* current_slot_in_chain = current_page_base;
        void* next_page_start = NULL;

        // Traverse the current page to find the start of the next page or the ring head
        while (true) {
            void* next_slot_addr = *(void**)((char*)current_slot_in_chain + 0x10);

            if (next_slot_addr == head_ring_buffer_ptr) { // Looped back to the head of the entire ring
                next_page_start = head_ring_buffer_ptr;
                break;
            }
            // Check if next_slot_addr is in a different memory page than current_slot_in_chain
            // A page boundary is crossed if the XOR difference is greater than 0xFFF (4095)
            // This indicates a jump from the end of one 0x1000 page to the start of another.
            if (((unsigned long)current_slot_in_chain ^ (unsigned long)next_slot_addr) > 0xFFF) {
                next_page_start = next_slot_addr; // This is the start of the next page
                break;
            }
            current_slot_in_chain = next_slot_addr; // Move to the next slot within the current page
        }
        current_page_base = next_page_start; // Set for the next iteration of the outer loop

        deallocate(page_to_deallocate, 0x1000);
    }
    return 0;
}

// Function: InitQueues
int InitQueues(void) {
    char input_buffer[10];
    unsigned char num_queues;
    
    // Prompt for number of queues (1-8)
    do {
        printf("How many queues (1-8)?: ");
        if (readUntil(input_buffer, sizeof(input_buffer) - 1, '\n') == -1) {
            return -1;
        }
        num_queues = (unsigned char)atoi(input_buffer);
    } while (num_queues == 0 || num_queues > 8);
    DAT_00018a2c = num_queues;

    DAT_00018a38 = 0; // Initialize priority queue flag to false

    // Allocate array for queue pointers
    if (allocate((unsigned int)num_queues * sizeof(Queue*), 0, &DAT_00018a3c) != 0) {
        puts("Failed to allocate interface queue pointers");
        return -1;
    }
    bzero(DAT_00018a3c, (unsigned int)num_queues * sizeof(Queue*));

    unsigned char total_weight_sum = 0;   // Accumulates total weight, was local_1e
    unsigned char min_weight_so_far = UCHAR_MAX; // Tracks minimum weight, was local_1f (initialized to 0xff)

    // Loop for each queue to initialize its properties
    for (unsigned char q_idx = 0; q_idx < num_queues; ++q_idx) {
        printf("Queue %u:\n", q_idx);

        // Ask if queue #0 is a priority queue
        if (q_idx == 0) {
            bzero(input_buffer, sizeof(input_buffer));
            do {
                printf("  Is queue #%u a priority queue(y,n): ", q_idx);
                if (readUntil(input_buffer, 1, '\n') == -1) { // Read only 1 char for 'y'/'n'
                    return -1;
                }
            } while (input_buffer[0] != 'y' && input_buffer[0] != 'n');
            if (input_buffer[0] == 'y') {
                DAT_00018a38 = 1;
            }
        }

        // Prompt for minimum priority
        int min_priority; // was local_14
        bool min_priority_ok;
        do {
            min_priority_ok = true;
            printf("  What's the minimum priority packet to place in queue #%u (0-63): ", q_idx);
            if (readUntil(input_buffer, sizeof(input_buffer) - 1, '\n') == -1) {
                return -1;
            }
            if (input_buffer[0] != '\0') {
                min_priority = atoi(input_buffer);
                if (min_priority < 0 || min_priority > 63) {
                    min_priority_ok = false;
                } else {
                    // Check for overlap with previously defined queues
                    for (unsigned char prev_q_idx = 0; prev_q_idx < q_idx; ++prev_q_idx) {
                        Queue* prev_q = (Queue*)DAT_00018a3c[prev_q_idx];
                        if ((prev_q->min_priority <= min_priority) && (min_priority <= prev_q->max_priority)) {
                            printf("  That priority value is already assigned to queue %u\n", prev_q_idx);
                            min_priority_ok = false;
                            break;
                        }
                    }
                }
            } else { // No input provided
                min_priority_ok = false;
            }
        } while (!min_priority_ok);

        // Prompt for maximum priority
        int max_priority; // was local_18
        bool max_priority_ok;
        do {
            max_priority_ok = true;
            printf("  What's the maximum priority packet to place in queue #%u (0-63): ", q_idx);
            if (readUntil(input_buffer, sizeof(input_buffer) - 1, '\n') == -1) {
                return -1;
            }
            if (input_buffer[0] != '\0') {
                max_priority = atoi(input_buffer);
                if (max_priority < 0 || max_priority > 63) {
                    max_priority_ok = false;
                } else if (max_priority < min_priority) {
                    puts("  Maximum priority must be greater or equal to minimum priority");
                    max_priority_ok = false;
                } else {
                    // Check for overlap with previously defined queues
                    for (unsigned char prev_q_idx = 0; prev_q_idx < q_idx; ++prev_q_idx) {
                        Queue* prev_q = (Queue*)DAT_00018a3c[prev_q_idx];
                        if ((prev_q->min_priority <= max_priority) && (max_priority <= prev_q->max_priority)) {
                            printf("  That priority value is already assigned to queue %u\n", prev_q_idx);
                            max_priority_ok = false;
                            break;
                        }
                    }
                }
            } else { // No input provided
                max_priority_ok = false;
            }
        } while (!max_priority_ok);

        // Prompt for queue depth
        unsigned int queue_depth; // was local_1c
        do {
            printf("  What is the depth of queue #%u (1 - 1024 packets): ", q_idx);
            if (readUntil(input_buffer, sizeof(input_buffer) - 1, '\n') == -1) {
                return -1;
            }
            queue_depth = (unsigned int)atoi(input_buffer);
        } while (queue_depth == 0 || queue_depth > 0x400); // 0x400 = 1024

        // Prompt for queue weight (if not a priority queue or if not the first queue)
        unsigned char queue_weight = 0; // was local_1d, initialized to 0
        if (!DAT_00018a38 || q_idx != 0) { // Condition: if NOT priority queue OR NOT the first queue
            do {
                printf("  What is the weight of queue #%u (1 - 100 percent): ", q_idx);
                if (readUntil(input_buffer, sizeof(input_buffer) - 1, '\n') == -1) {
                    return -1;
                }
                queue_weight = (unsigned char)atoi(input_buffer);

                if (queue_weight == 0 || queue_weight > 100) {
                    // Invalid range, loop again
                } else if ((unsigned int)queue_weight + total_weight_sum > 100) {
                    puts("  Total weight of all queues can not exceed 100%");
                    queue_weight = 0; // Force re-prompt
                } else if (q_idx == num_queues - 1 && (unsigned int)queue_weight + total_weight_sum != 100) {
                    // If this is the last queue, total weight must be 100%
                    puts("  Total weight of all queues must total up to 100%");
                    queue_weight = 0; // Force re-prompt
                }
            } while (queue_weight == 0 || queue_weight > 100);

            if (queue_weight < min_weight_so_far) {
                min_weight_so_far = queue_weight;
            }
            total_weight_sum += queue_weight;
        }

        // Allocate memory for the queue structure
        Queue* current_queue_ptr;
        if (allocate(sizeof(Queue), 0, (void**)&DAT_00018a3c[q_idx]) != 0) {
            printf("  Failed to allocate queue #%u\n", q_idx);
            return -1;
        }
        current_queue_ptr = (Queue*)DAT_00018a3c[q_idx];
        bzero(current_queue_ptr, sizeof(Queue));

        // Populate queue structure fields
        current_queue_ptr->depth = queue_depth;
        current_queue_ptr->min_priority = (char)min_priority;
        current_queue_ptr->max_priority = (char)max_priority;
        current_queue_ptr->weight = queue_weight;

        // Initialize ring buffer for packets
        if (InitRingBuffer(queue_depth, &current_queue_ptr->head_ring_buffer_ptr) != 0) {
            puts("  Failed to allocate pkt ring buffer");
            return -1;
        }
        current_queue_ptr->current_pkt_ptr = current_queue_ptr->head_ring_buffer_ptr; // local_30[2] = local_30[3]
        current_queue_ptr->tail_ring_buffer_ptr = current_queue_ptr->head_ring_buffer_ptr;
        current_queue_ptr->current_ring_buffer_ptr = current_queue_ptr->head_ring_buffer_ptr;

        // Calculate and set 'calculated_val' (local_5c)
        // This calculation is skipped for the first priority queue, similar to weight.
        if (!DAT_00018a38 || q_idx != 0) {
            // Ensure min_weight_so_far is not zero to prevent division by zero.
            // Since weights are prompted for 1-100, min_weight_so_far will be >= 1 if any weights were set.
            // If all queues are priority queues and weights are skipped, min_weight_so_far remains UCHAR_MAX (255).
            // In that case, the division will result in a small number.
            // If current_queue_ptr->weight is 0 (for priority queues where weight is skipped), calculated_val will be 0.
            current_queue_ptr->calculated_val = (unsigned int)ROUND(
                ((double)current_queue_ptr->weight / _DAT_00015528) *
                (double)(150000.0 / (double)min_weight_so_far)
            );
        } else {
            current_queue_ptr->calculated_val = 0; // Explicitly set to 0 if calculation is skipped
        }
    } // End of for loop for each queue

    // Post-loop validation: Check if all priority values (0-63) are accounted for
    char checksum_val = '@'; // ASCII 64, was local_25
    for (unsigned char i = 0; i < num_queues; ++i) {
        Queue* q = (Queue*)DAT_00018a3c[i];
        checksum_val += (q->min_priority - q->max_priority) - 1;
    }
    if (checksum_val != '\0') {
        printf("Not all priority values from %d to %d are accounted for in the queue definitions.\n", 0, 63);
        return -1;
    }

    return 0; // Success
}

// Function: DestroyQueues
int DestroyQueues(void) {
    for (unsigned char q_idx = 0; q_idx < DAT_00018a2c; ++q_idx) {
        if (DAT_00018a3c[q_idx] != NULL) { // Check if queue pointer is valid
            // Destroy the ring buffer associated with the queue
            if (DestroyRingBuffer(q_idx) != 0) {
                // Handle error if DestroyRingBuffer fails (e.g., log, but continue to free queue struct)
                fprintf(stderr, "Warning: Failed to destroy ring buffer for queue %u\n", q_idx);
            }
            // Deallocate the queue structure itself
            deallocate(DAT_00018a3c[q_idx], sizeof(Queue));
            DAT_00018a3c[q_idx] = NULL; // Clear pointer after freeing
        }
    }
    // Deallocate the array of queue pointers
    if (DAT_00018a3c != NULL) {
        deallocate(DAT_00018a3c, (unsigned int)DAT_00018a2c * sizeof(Queue*));
        DAT_00018a3c = NULL; // Clear pointer after freeing
    }
    return 0;
}