#include <stdio.h> // Required for printf

// Function: dive_statistics
// This function calculates and prints statistics for logged dives.
// It assumes 'param_1' points to a structure where:
// - The head of a linked list of dives is stored at offset 0x9c, as a 32-bit integer (representing an address).
// Each dive node in the linked list (pointed to by these 32-bit integer addresses) has:
// - The 'max_depth' value at offset 0x34, as a 32-bit integer.
// - The 'dive_length' value at offset 0x3c, as a 32-bit integer.
// - The 32-bit integer (address) of the 'next_dive' in the list at offset 0x7c.
// This design is typical of decompiled code from 32-bit systems where pointers are treated as integers.
int dive_statistics(void *param_1) {
    int total_dives = 0;             // Corresponds to original local_10 (initially total count, then reused for avg)
    int total_max_depth_sum = 0;     // Corresponds to original local_14
    int max_depth_entry_count = 0;   // Corresponds to original local_18
    int total_dive_length_sum = 0;   // Corresponds to original local_1c
    int dive_length_entry_count = 0; // Corresponds to original local_20

    printf("\n");

    // Retrieve the address of the first dive from param_1 + 0x9c.
    // This value is treated as a 32-bit integer representing a memory address.
    int current_dive_address = *(int *)((char *)param_1 + 0x9c); // Corresponds to original local_24

    if (current_dive_address == 0) { // If the first dive address is 0 (NULL)
        printf("No dives are logged\n");
        return -1; // Original returned 0xffffffff, which is -1 for a signed int
    } else {
        // Loop through the linked list of dives.
        // current_dive_address holds the memory address of the current dive node.
        for (; current_dive_address != 0; current_dive_address = *(int *)((char *)current_dive_address + 0x7c)) {
            total_dives++;

            // Access max_depth at offset 0x34 from the current dive's address.
            int max_depth = *(int *)((char *)current_dive_address + 0x34);
            if (max_depth != 0) {
                total_max_depth_sum += max_depth;
                max_depth_entry_count++;
            }

            // Access dive_length at offset 0x3c from the current dive's address.
            int dive_length = *(int *)((char *)current_dive_address + 0x3c);
            if (dive_length != 0) {
                total_dive_length_sum += dive_length;
                dive_length_entry_count++;
            }
        }

        // Print calculated statistics.
        printf("Dives logged: %d\n", total_dives);

        // Calculate and print Average Max Depth.
        if (max_depth_entry_count < 1) {
            printf("Average Max Depth: 0\n");
        } else {
            // Reuse total_dives for the average, mirroring the original code's variable reuse.
            total_dives = total_max_depth_sum / max_depth_entry_count;
            printf("Average Max Depth: %d\n", total_dives);
        }

        // Calculate and print Average Dive Length.
        if (dive_length_entry_count < 1) {
            printf("Average Dive Length: 0\n");
        } else {
            printf("Average Dive Length: %d\n", total_dive_length_sum / dive_length_entry_count);
        }
        return 0; // Original returned 0
    }
}