#include <stdio.h>   // For printf, stdin, getline, perror
#include <stdlib.h>  // For atoi, free, calloc
#include <string.h>  // For strlen
#include <stddef.h>  // For size_t
#include <sys/types.h> // For ssize_t

// Forward declarations for opaque types.
// The actual memory layout will be accessed via byte offsets.
typedef struct LogBook LogBook;
typedef struct Dive Dive;
typedef struct SubDive SubDive;

// Dummy definitions for LogBook, Dive, SubDive structures.
// These are minimal to allow compilation and type-checking for pointers.
// Their actual content and size are implicitly handled by the manual offset access.
struct SubDive {
    // Placeholder to allow sizeof(SubDive) and pointer type.
    // The actual 'next' pointer is accessed at offset +8.
    char _placeholder[sizeof(void*) * 2]; // Ensure enough space for at least one pointer and some data
};

struct Dive {
    // Placeholder to allow sizeof(Dive) and pointer type.
    // 'first_sub_dive' is accessed at offset +0x78, 'next_dive' at +0x7c.
    char _placeholder[0x7c + sizeof(void*)]; // Ensure enough space for the highest offset + pointer
};

struct LogBook {
    // Placeholder to allow sizeof(LogBook) and pointer type.
    // 'first_dive' is accessed at offset +0x9c.
    char _placeholder[0x9c + sizeof(void*)]; // Ensure enough space for the highest offset + pointer
};

// Dummy function to list dives.
// It accesses the linked list structure using direct memory offsets.
void list_dives(LogBook *logbook_struct_ptr) {
    printf("Listing dives (dummy function)...\n");
    // Access first_dive pointer at offset 0x9c from logbook_struct_ptr
    Dive *current_dive = *(Dive **)((char *)logbook_struct_ptr + 0x9c);
    if (current_dive == NULL) {
        printf("No dives to list.\n");
        return;
    }
    int count = 1;
    while (current_dive != NULL) {
        printf("Dive #%d\n", count++);
        // Access next_dive pointer at offset 0x7c from current_dive
        current_dive = *(Dive **)((char *)current_dive + 0x7c);
    }
}

// Function: remove_dives
// param_1 is treated as a pointer to the LogBook structure, but accessed via char* for offsets.
int remove_dives(void *param_1) {
  char *logbook_base_ptr = (char *)param_1; // Use char* for byte-level pointer arithmetic

  char *line = NULL; // Buffer for getline, will be allocated dynamically
  size_t len = 0;    // Current allocated size for 'line'
  ssize_t bytes_read; // Return value of getline

  int dive_num_to_delete = 0;
  int current_dive_num = 1;

  // Get the pointer to the first dive from the LogBook structure at offset 0x9c
  Dive *current_dive = *(Dive **)(logbook_base_ptr + 0x9c);

  if (current_dive == NULL) {
    printf("\n");
    printf("Dive Log is empty\n");
  } else {
    // Call list_dives (which expects a LogBook* type)
    list_dives((LogBook *)param_1);
    printf("\n");
    printf("Enter Dive # to delete or blank to abort: ");

    bytes_read = getline(&line, &len, stdin);

    if (bytes_read != -1) { // Check for read error or EOF
      // Remove trailing newline character if present
      if (bytes_read > 0 && line[bytes_read - 1] == '\n') {
          line[bytes_read - 1] = '\0';
      }
      
      if (strlen(line) == 0) { // Blank input means abort
          printf("Aborted.\n");
          if (line) free(line); // Free buffer allocated by getline
          return 0;
      }

      dive_num_to_delete = atoi(line); // Convert input string to integer
      if (line) free(line); // Free buffer after use

      // Re-initialize current_dive as list_dives might have iterated it or we need a fresh start
      current_dive = *(Dive **)(logbook_base_ptr + 0x9c);

      if (dive_num_to_delete == 1) { // Deleting the first dive
        if (current_dive == NULL) { // Should not happen if initial check passed, but for robustness
            printf("Error: Log is empty, cannot delete dive 1.\n");
            return 0;
        }
        
        Dive *dive_to_free = current_dive; // This is the node to be deleted
        
        // Update the LogBook's first_dive pointer to point to the second dive
        // *(Dive **)(param_1 + 0x9c) = current_dive->next_dive;
        *(Dive **)(logbook_base_ptr + 0x9c) = *(Dive **)((char *)current_dive + 0x7c);

        // Free all associated sub-dives
        SubDive *current_sub_dive = *(SubDive **)((char *)dive_to_free + 0x78);
        while (current_sub_dive != NULL) {
          SubDive *next_sub_dive = *(SubDive **)((char *)current_sub_dive + 8); // Get next sub-dive before freeing
          free(current_sub_dive);
          current_sub_dive = next_sub_dive;
        }
        free(dive_to_free); // Free the dive node itself
        printf("Dive #1 deleted successfully.\n");
      } else if (dive_num_to_delete > 1) { // Deleting a dive other than the first
        Dive *prev_dive = NULL; // Pointer to the dive preceding the one to be deleted
        current_dive_num = 1; // Reset counter for traversal

        // Traverse the list to find the dive to delete and its predecessor
        while (current_dive_num < dive_num_to_delete && current_dive != NULL) {
          prev_dive = current_dive; // Keep track of the previous dive
          current_dive = *(Dive **)((char *)current_dive + 0x7c); // Move to the next dive
          current_dive_num++;
        }

        // After the loop:
        // current_dive points to the dive to be deleted (or NULL if not found)
        // prev_dive points to the dive before current_dive (or NULL if current_dive was the first)

        if (current_dive_num == dive_num_to_delete && current_dive != NULL && prev_dive != NULL) {
          // Found the dive to delete, and it's not the first one.
          // Link the previous dive's 'next_dive' pointer to the current_dive's 'next_dive' pointer
          // prev_dive->next_dive = current_dive->next_dive;
          *(Dive **)((char *)prev_dive + 0x7c) = *(Dive **)((char *)current_dive + 0x7c);

          // Free all associated sub-dives of the deleted dive
          SubDive *current_sub_dive = *(SubDive **)((char *)current_dive + 0x78);
          while (current_sub_dive != NULL) {
            SubDive *next_sub_dive = *(SubDive **)((char *)current_sub_dive + 8);
            free(current_sub_dive);
            current_sub_dive = next_sub_dive;
          }
          free(current_dive); // Free the dive node itself
          printf("Dive #%d deleted successfully.\n", dive_num_to_delete);
        } else {
          // Dive not found or an invalid number was entered (e.g., beyond list bounds)
          printf("Invalid dive number entered or dive not found.\n");
        }
      } else { // dive_num_to_delete is 0 or negative
          printf("Invalid dive number entered.\n");
      }
    } else { // getline failed (e.g., EOF or error)
      if (line) free(line); // Free buffer if getline allocated something before error
      perror("Error reading input");
    }
  }
  return 0;
}

// Main function for testing the remove_dives functionality
int main() {
    // Allocate a LogBook structure.
    // The size needs to accommodate the highest offset (0x9c) plus the size of a pointer.
    LogBook *my_logbook = (LogBook *)calloc(1, 0x9c + sizeof(void*));
    if (!my_logbook) {
        fprintf(stderr, "Failed to allocate LogBook\n");
        return 1;
    }

    // Determine allocation sizes for Dive and SubDive nodes.
    // Each node needs to be large enough to hold its highest offset field plus the pointer itself.
    size_t dive_node_size = 0x7c + sizeof(void*);
    size_t sub_dive_node_size = 8 + sizeof(void*);

    // Manually create dummy dives and sub-dives for testing
    // Dive 1
    Dive *dive1 = (Dive *)calloc(1, dive_node_size);
    if (!dive1) return 1;
    // SubDive for Dive 1
    SubDive *sub1_1 = (SubDive *)calloc(1, sub_dive_node_size);
    if (!sub1_1) return 1;
    *(SubDive **)((char *)dive1 + 0x78) = sub1_1; // dive1->first_sub_dive = sub1_1;

    // Dive 2
    Dive *dive2 = (Dive *)calloc(1, dive_node_size);
    if (!dive2) return 1;
    *(Dive **)((char *)dive1 + 0x7c) = dive2; // dive1->next_dive = dive2;
    // SubDives for Dive 2
    SubDive *sub2_1 = (SubDive *)calloc(1, sub_dive_node_size);
    if (!sub2_1) return 1;
    *(SubDive **)((char *)dive2 + 0x78) = sub2_1; // dive2->first_sub_dive = sub2_1;
    SubDive *sub2_2 = (SubDive *)calloc(1, sub_dive_node_size);
    if (!sub2_2) return 1;
    *(SubDive **)((char *)sub2_1 + 8) = sub2_2; // sub2_1->next = sub2_2;

    // Dive 3
    Dive *dive3 = (Dive *)calloc(1, dive_node_size);
    if (!dive3) return 1;
    *(Dive **)((char *)dive2 + 0x7c) = dive3; // dive2->next_dive = dive3;

    // Set the first dive of the logbook
    *(Dive **)((char *)my_logbook + 0x9c) = dive1; // my_logbook->first_dive = dive1;

    printf("Initial state:\n");
    list_dives(my_logbook);
    printf("\n");

    // Test Case 1: Remove Dive 2 (user enters '2')
    printf("--- Attempting to remove Dive #2 ---\n");
    remove_dives(my_logbook); 
    printf("\nState after first removal attempt:\n");
    list_dives(my_logbook);
    printf("\n");

    // Test Case 2: Remove Dive 1 (user enters '1'). Original Dive 1 is gone, now Dive 3 is the new Dive 1.
    printf("--- Attempting to remove Dive #1 (which is now original Dive 3) ---\n");
    remove_dives(my_logbook); 
    printf("\nState after second removal attempt:\n");
    list_dives(my_logbook);
    printf("\n");

    // Test Case 3: Remove Dive 1 again (user enters '1'). The log should now be empty.
    printf("--- Attempting to remove Dive #1 again (log should become empty) ---\n");
    remove_dives(my_logbook); 
    printf("\nState after third removal attempt:\n");
    list_dives(my_logbook);
    printf("\n");

    // Test Case 4: Attempt to remove from an empty log (user enters '1')
    printf("--- Attempting to remove from an empty log ---\n");
    remove_dives(my_logbook); 
    printf("\n");

    // Clean up any remaining dynamically allocated memory (should be none if removals were successful)
    Dive *current_cleanup = *(Dive **)((char *)my_logbook + 0x9c);
    while (current_cleanup != NULL) {
        Dive *next_cleanup = *(Dive **)((char *)current_cleanup + 0x7c);
        SubDive *current_sub_cleanup = *(SubDive **)((char *)current_cleanup + 0x78);
        while (current_sub_cleanup != NULL) {
            SubDive *next_sub_cleanup = *(SubDive **)((char *)current_sub_cleanup + 8);
            free(current_sub_cleanup);
            current_sub_cleanup = next_sub_cleanup;
        }
        free(current_cleanup);
        current_cleanup = next_cleanup;
    }
    free(my_logbook); // Free the LogBook structure itself

    return 0;
}