#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>   // For round()
#include <stdbool.h> // For bool type
#include <stdint.h> // For uintptr_t

// Custom types mapping from decompiled code
typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned int undefined4; // Assuming 4-byte unsigned int

// --- Global Variables ---
// DAT_00018a2c: Number of queues
unsigned char g_num_queues = 0;
// DAT_00018a38: Flag for priority queue system (0 or 1)
char g_is_priority_queue_system = 0;
// DAT_00018a3c: Array of pointers to Queue structures
// This will be allocated dynamically based on g_num_queues
struct Queue **g_queue_pointers = NULL;
// _DAT_00015528: A double value used in calculations
double g_some_double_value = 1.0; // Default to 1.0 to prevent division by zero, actual value unknown.

// --- Structure Definitions ---

// RingBufferEntry: 20 bytes (0x14)
typedef struct RingBufferEntry {
    char data[16]; // Placeholder for actual packet data or metadata
    struct RingBufferEntry *next; // Pointer to the next entry in the ring (0x10 offset)
} RingBufferEntry;

// Queue structure: 52 bytes (0x34)
typedef struct Queue {
    uint depth;             // 0x00
    uint field_4;           // 0x04 (unknown purpose)
    uint field_8;           // 0x08 (gets value from calculated_value)
    uint calculated_value;  // 0x0c (stores the result of ROUND calculation)
    // ... 0x10 - 0x23 (padding or other fields)
    char min_priority;      // 0x24
    char max_priority;      // 0x25
    byte weight;            // 0x26
    // ... 0x27 (padding)
    RingBufferEntry *current_read_ptr;  // 0x28
    RingBufferEntry *current_write_ptr; // 0x2c
    RingBufferEntry *ring_buffer_head;  // 0x30 (points to the first entry of the first page)
} Queue;

// --- Custom Memory Allocation/Deallocation Wrappers ---
// Replaces 'allocate'
int custom_allocate(size_t size, int flags, void **out_ptr) {
    *out_ptr = malloc(size);
    if (*out_ptr == NULL) {
        return 1; // Failure
    }
    memset(*out_ptr, 0, size); // bzero equivalent
    return 0; // Success
}

// Replaces 'deallocate'
void custom_deallocate(void *ptr, size_t size) {
    free(ptr);
}

// --- Helper for reading input ---
// Replaces 'readUntil'
int read_input(char *buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return -1; // Error or EOF
    }
    // Remove trailing newline if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 0; // Success
}

// --- Function Prototypes ---
undefined4 InitRingBuffer(uint param_1, RingBufferEntry **param_2);
undefined4 DestroyRingBuffer(byte param_1);
undefined4 InitQueues(void);
undefined4 DestroyQueues(void);

// Function: InitRingBuffer
undefined4 InitRingBuffer(uint depth, RingBufferEntry **ring_buffer_head_ptr) {
    const uint ENTRIES_PER_PAGE = 0xcc; // 204 entries per 0x1000 (4096) page
    const size_t PAGE_SIZE = 0x1000; // 4096 bytes
    const size_t ENTRY_SIZE = sizeof(RingBufferEntry); // 20 bytes

    uint num_pages = depth / ENTRIES_PER_PAGE;
    if (depth % ENTRIES_PER_PAGE != 0 || depth == 0) { // If depth is not a multiple, or depth is 0, add one page (unless depth is 0)
        if (depth > 0) num_pages++;
    }

    RingBufferEntry *first_entry_of_ring = NULL;
    RingBufferEntry *prev_page_last_entry = NULL; // Tracks the last entry of the previous page
    
    // List to keep track of allocated page pointers for cleanup
    RingBufferEntry **allocated_pages = NULL;
    if (num_pages > 0) {
        allocated_pages = (RingBufferEntry**)malloc(num_pages * sizeof(RingBufferEntry*));
        if (allocated_pages == NULL) {
            puts("Failed to allocate page tracking array");
            return 0xffffffff;
        }
    }

    for (uint current_page_idx = 0; current_page_idx < num_pages; ++current_page_idx) {
        void *page_ptr_void;
        if (custom_allocate(PAGE_SIZE, 0, &page_ptr_void) != 0) {
            puts("Failed to allocate pkt page");
            // Cleanup: free all pages allocated so far
            for (uint i = 0; i < current_page_idx; ++i) {
                custom_deallocate(allocated_pages[i], PAGE_SIZE);
            }
            free(allocated_pages);
            return 0xffffffff;
        }
        RingBufferEntry *current_page_first_entry = (RingBufferEntry *)page_ptr_void;
        allocated_pages[current_page_idx] = current_page_first_entry; // Store page pointer

        if (current_page_idx == 0) {
            first_entry_of_ring = current_page_first_entry; // This is the overall head
        }

        if (prev_page_last_entry != NULL) {
            prev_page_last_entry->next = current_page_first_entry; // Link previous page's last entry to current page's first
        }

        uint entries_on_current_page = ENTRIES_PER_PAGE;
        if (current_page_idx == num_pages - 1) { // Last page
            entries_on_current_page = depth - (current_page_idx * ENTRIES_PER_PAGE);
            if (entries_on_current_page == 0 && depth > 0) entries_on_current_page = ENTRIES_PER_PAGE; // Ensure at least 1 entry if depth > 0 and it's the only page
        }
        if (depth == 0) entries_on_current_page = 0;

        RingBufferEntry *current_entry_on_page = current_page_first_entry;
        for (uint i = 0; i < entries_on_current_page; ++i) {
            if (i < entries_on_current_page - 1) {
                current_entry_on_page->next = (RingBufferEntry *)((char *)current_entry_on_page + ENTRY_SIZE);
            } else {
                prev_page_last_entry = current_entry_on_page; // This is the last entry on the current page
            }
            current_entry_on_page = (RingBufferEntry *)((char *)current_entry_on_page + ENTRY_SIZE); // Advance pointer to next physical location
        }
    }

    if (num_pages > 0 && prev_page_last_entry != NULL) {
        prev_page_last_entry->next = first_entry_of_ring; // Complete the ring
    } else if (num_pages == 0) {
        first_entry_of_ring = NULL; // If depth is 0, no ring buffer is created. head should be NULL.
    } else if (depth == 1 && num_pages == 1 && first_entry_of_ring != NULL) { // Special case for single entry
        first_entry_of_ring->next = first_entry_of_ring;
    }

    *ring_buffer_head_ptr = first_entry_of_ring;
    free(allocated_pages); // Free the temporary tracking array
    return 0;
}

// Function: DestroyRingBuffer
undefined4 DestroyRingBuffer(byte queue_idx) {
    if (queue_idx >= g_num_queues) {
        return 0xffffffff;
    }

    Queue *q = g_queue_pointers[queue_idx];
    if (q == NULL || q->ring_buffer_head == NULL) {
        return 0; // Nothing to destroy or already destroyed
    }

    uint depth = q->depth;
    RingBufferEntry *first_page_head_entry = q->ring_buffer_head;
    RingBufferEntry *current_page_head_entry = first_page_head_entry;

    const uint ENTRIES_PER_PAGE = 0xcc; // 204
    const size_t PAGE_SIZE = 0x1000;
    uint num_pages = depth / ENTRIES_PER_PAGE;
    if (depth % ENTRIES_PER_PAGE != 0 || depth == 0) {
        if (depth > 0) num_pages++;
    }
    
    // If depth is 0, num_pages is 0, loop won't run.
    if (num_pages == 0) {
        q->ring_buffer_head = NULL;
        return 0;
    }

    for (uint i = 0; i < num_pages; ++i) {
        RingBufferEntry *next_page_head_entry = NULL;
        RingBufferEntry *temp_entry = current_page_head_entry;
        
        // Find the last entry on the current page.
        // The condition `0xfff < (addr1 ^ addr2)` checks if addresses span a 4KB boundary.
        // This is a heuristic to detect if `temp_entry` and `temp_entry->next` are on different pages.
        while (temp_entry != NULL && temp_entry->next != NULL &&
               !(((uintptr_t)temp_entry ^ (uintptr_t)temp_entry->next) > 0xfff)) {
            temp_entry = temp_entry->next;
            if (temp_entry == first_page_head_entry) break; // Avoid infinite loop if only one page and we wrapped
        }
        
        if (temp_entry != NULL) {
            next_page_head_entry = temp_entry->next; // This should be the first entry of the next page
        }

        custom_deallocate(current_page_head_entry, PAGE_SIZE); // Free the page
        
        current_page_head_entry = next_page_head_entry;
        if (current_page_head_entry == first_page_head_entry) {
            break; // We've wrapped around
        }
    }

    q->ring_buffer_head = NULL; // Clear the head pointer
    return 0;
}

// Function: InitQueues
undefined4 InitQueues(void) {
    char input_buffer[10];
    int int_input_value;
    unsigned char current_queue_idx = 0;
    
    unsigned char total_weight_sum = 0;
    unsigned char min_weight_found = 0xff; // Initialized to max byte value

    // Get number of queues (1-8)
    while (true) {
        printf("How many queues (1-8)?: ");
        if (read_input(input_buffer, sizeof(input_buffer)) == -1) {
            return 0xffffffff;
        }
        int_input_value = atoi(input_buffer);
        if (int_input_value >= 1 && int_input_value <= 8) {
            g_num_queues = (unsigned char)int_input_value;
            break;
        }
        printf("Invalid input. Please enter a number between 1 and 8.\n");
    }

    g_is_priority_queue_system = 0; // DAT_00018a38 = '\0';

    // Allocate array for queue pointers
    if (custom_allocate(g_num_queues * sizeof(Queue *), 0, (void **)&g_queue_pointers) != 0) {
        puts("Failed to allocate interface queue pointers");
        return 0xffffffff;
    }
    // bzero is handled by custom_allocate memset

    // Loop for each queue
    while (current_queue_idx < g_num_queues) {
        printf("Queue %u:\n", (uint)current_queue_idx);

        // Check if it's a priority queue (only for queue 0)
        if (current_queue_idx == 0) {
            bool valid_input = false;
            while (!valid_input) {
                printf("  Is queue #%u a priority queue(y,n): ", (uint)current_queue_idx);
                if (read_input(input_buffer, sizeof(input_buffer)) == -1) {
                    return 0xffffffff;
                }
                if (input_buffer[0] == 'y' || input_buffer[0] == 'n') {
                    valid_input = true;
                } else {
                    printf("  Invalid input. Please enter 'y' or 'n'.\n");
                }
            }
            if (input_buffer[0] == 'y') {
                g_is_priority_queue_system = 1; // DAT_00018a38 = '\x01';
            }
        }

        int min_priority = -1;
        // Get minimum priority (0-63)
        while (min_priority < 0 || min_priority > 0x3f) {
            printf("  What's the minimum priority packet to place in queue #%u (0-63): ", (uint)current_queue_idx);
            if (read_input(input_buffer, sizeof(input_buffer)) == -1) {
                return 0xffffffff;
            }
            if (input_buffer[0] != '\0') {
                int_input_value = atoi(input_buffer);
                if (int_input_value >= 0 && int_input_value <= 0x3f) {
                    // Check for overlapping priority ranges with previous queues
                    bool overlap = false;
                    for (unsigned char prev_q_idx = 0; prev_q_idx < current_queue_idx; ++prev_q_idx) {
                        Queue *prev_q = g_queue_pointers[prev_q_idx];
                        if (int_input_value >= prev_q->min_priority && int_input_value <= prev_q->max_priority) {
                            printf("  That priority value is already assigned to queue %u\n", (uint)prev_q_idx);
                            overlap = true;
                            break;
                        }
                    }
                    if (!overlap) {
                        min_priority = int_input_value;
                    }
                } else {
                    printf("  Invalid priority. Must be between 0 and 63.\n");
                }
            }
        }

        int max_priority = -1;
        // Get maximum priority (0-63)
        while (max_priority < 0 || max_priority > 0x3f) {
            printf("  What's the maximum priority packet to place in queue #%u (0-63): ", (uint)current_queue_idx);
            if (read_input(input_buffer, sizeof(input_buffer)) == -1) {
                return 0xffffffff;
            }
            if (input_buffer[0] != '\0') {
                int_input_value = atoi(input_buffer);
                if (int_input_value < min_priority) {
                    puts("  Maximum priority must be greater or equal to minimum priority");
                } else if (int_input_value >= 0 && int_input_value <= 0x3f) {
                    // Check for overlapping priority ranges with previous queues
                    bool overlap = false;
                    for (unsigned char prev_q_idx = 0; prev_q_idx < current_queue_idx; ++prev_q_idx) {
                        Queue *prev_q = g_queue_pointers[prev_q_idx];
                        if (int_input_value >= prev_q->min_priority && int_input_value <= prev_q->max_priority) {
                            printf("  That priority value is already assigned to queue %u\n", (uint)prev_q_idx);
                            overlap = true;
                            break;
                        }
                    }
                    if (!overlap) {
                        max_priority = int_input_value;
                    }
                } else {
                    printf("  Invalid priority. Must be between 0 and 63.\n");
                }
            }
        }

        uint queue_depth = 0;
        // Get queue depth (1-1024)
        while (queue_depth == 0 || queue_depth > 0x400) {
            printf("  What is the depth of queue #%u (1 - 1024 packets): ", (uint)current_queue_idx);
            if (read_input(input_buffer, sizeof(input_buffer)) == -1) {
                return 0xffffffff;
            }
            int_input_value = atoi(input_buffer);
            if (int_input_value >= 1 && int_input_value <= 0x400) {
                queue_depth = (uint)int_input_value;
            } else {
                printf("  Invalid depth. Must be between 1 and 1024.\n");
            }
        }

        unsigned char queue_weight = 0;
        // Get queue weight (1-100)
        // Only if not a priority queue system OR not the first queue.
        if (g_is_priority_queue_system == 0 || current_queue_idx != 0) {
            while (queue_weight == 0 || queue_weight > 100) {
                printf("  What is the weight of queue #%u (1 - 100 percent): ", (uint)current_queue_idx);
                if (read_input(input_buffer, sizeof(input_buffer)) == -1) {
                    return 0xffffffff;
                }
                int_input_value = atoi(input_buffer);
                if (int_input_value >= 1 && int_input_value <= 100) {
                    if ((unsigned char)int_input_value + total_weight_sum > 100) {
                        puts("  Total weight of all queues can not exceed 100%");
                    } else {
                        queue_weight = (unsigned char)int_input_value;
                        // Check if it's the last queue and total weight is not 100%
                        if (current_queue_idx == g_num_queues - 1 && (queue_weight + total_weight_sum) != 100) {
                            puts("  Total weight of all queues must total up to 100%");
                            queue_weight = 0; // Reset
                        }
                    }
                } else {
                    printf("  Invalid weight. Must be between 1 and 100.\n");
                }
            }
            if (queue_weight < min_weight_found) {
                min_weight_found = queue_weight;
            }
            total_weight_sum += queue_weight;
        }

        // Allocate Queue structure
        Queue *new_queue;
        if (custom_allocate(sizeof(Queue), 0, (void **)&new_queue) != 0) {
            printf("  Failed to allocate queue #%u\n", (uint)current_queue_idx);
            return 0xffffffff;
        }
        g_queue_pointers[current_queue_idx] = new_queue;

        // Initialize Queue fields
        new_queue->depth = queue_depth;
        new_queue->min_priority = (char)min_priority;
        new_queue->max_priority = (char)max_priority;
        new_queue->weight = queue_weight;

        // Call InitRingBuffer
        if (InitRingBuffer(queue_depth, &new_queue->ring_buffer_head) != 0) {
            puts("  Failed to allocate pkt ring buffer");
            return 0xffffffff;
        }
        new_queue->current_read_ptr = new_queue->ring_buffer_head;
        new_queue->current_write_ptr = new_queue->ring_buffer_head;
        new_queue->field_8 = new_queue->calculated_value; // This is a direct translation of local_30[2] = local_30[3];

        current_queue_idx++;
    }

    // After all queues are initialized, perform final checks and calculations
    int priority_range_sum = 0;
    for (unsigned char i = 0; i < g_num_queues; ++i) {
        Queue *q = g_queue_pointers[i];
        priority_range_sum += (q->max_priority - q->min_priority + 1);
    }
    // Assuming priority range 0-63, total 64 unique priority values.
    if (priority_range_sum != 64) {
        printf("Not all priority values from 0 to 63 are accounted for in the queue definitions.\n");
        return 0xffffffff;
    }

    // Calculate final queue values (if not priority queue system or not the first queue)
    for (unsigned char i = 0; i < g_num_queues; ++i) {
        if (g_is_priority_queue_system == 0 || i != 0) {
            Queue *q = g_queue_pointers[i];
            // ROUND(((double)(uint)*(byte *)(iVar1 + 0x26) / _DAT_00015528) * (double)(150000 / (ulonglong)(longlong)(int)(uint)local_1f));
            q->calculated_value = (uint)round(((double)q->weight / g_some_double_value) *
                                                (150000.0 / (double)min_weight_found));
        }
    }

    return 0;
}

// Function: DestroyQueues
undefined4 DestroyQueues(void) {
    for (unsigned char i = 0; i < g_num_queues; ++i) {
        if (g_queue_pointers[i] != NULL) {
            if (DestroyRingBuffer(i) != 0) {
                // Log error but continue to free other resources
                fprintf(stderr, "Error destroying ring buffer for queue %u. Continuing cleanup.\n", (uint)i);
                // The original code would return immediately on error here.
                // For robustness, we continue to free other resources.
            }
            custom_deallocate(g_queue_pointers[i], sizeof(Queue));
            g_queue_pointers[i] = NULL;
        }
    }
    if (g_queue_pointers != NULL) {
        custom_deallocate(g_queue_pointers, g_num_queues * sizeof(Queue *));
        g_queue_pointers = NULL;
    }
    g_num_queues = 0;
    g_is_priority_queue_system = 0;
    return 0;
}

// --- Main function for testing ---
int main() {
    // Initialize global double value (if it has a specific meaning/default)
    // For now, setting it to a non-zero value.
    g_some_double_value = 1.0; 

    printf("Initializing Queues...\n");
    if (InitQueues() == 0) {
        printf("Queues initialized successfully.\n");
        // Example: print some queue details
        for (unsigned char i = 0; i < g_num_queues; ++i) {
            Queue *q = g_queue_pointers[i];
            if (q) {
                printf("  Queue %u: Depth=%u, MinPrio=%d, MaxPrio=%d, Weight=%u, CalcVal=%u\n",
                       (uint)i, q->depth, q->min_priority, q->max_priority, q->weight, q->calculated_value);
            }
        }
    } else {
        printf("Failed to initialize queues.\n");
    }

    printf("Destroying Queues...\n");
    if (DestroyQueues() == 0) {
        printf("Queues destroyed successfully.\n");
    } else {
        printf("Failed to destroy queues.\n");
    }

    return 0;
}