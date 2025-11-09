#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h> // For 'true' in while(true)
#include <stdint.h>  // For standard integer types like uint32_t if needed, but not strictly required for this problem.

// --- Placeholder Type Definitions ---

// Placeholder for a string object type returned by initString
// The original code uses `if (*local_18 == 0)` and `deallocate(local_18,0xc);`.
// `0xc` (12 bytes) is a common size for a pointer + length/ref_count on a 64-bit system.
typedef struct String_t {
    int ref_count; // Or length, or some other integer field
    char *data;    // Pointer to the actual string data
} String_t;

// Placeholder for a Planet structure
// Allocation size 0xd4 (212 bytes).
// The original code copies the name directly into the start of the allocated memory.
#define PLANET_NAME_MAX_LEN 29 // From receive_until(local_36,10,0x1d) for name input
typedef struct Planet {
    char name[PLANET_NAME_MAX_LEN + 1]; // +1 for null terminator
    // Add other fields here to make up the 0xd4 (212 bytes) total size
    // For now, use padding to match the size.
    char padding[212 - (PLANET_NAME_MAX_LEN + 1)]; // 212 - 30 = 182 bytes
} Planet;

// --- Global Variable Declaration ---

// Global array of pointers to Planets
Planet* solarSystem[10];

// --- Placeholder Function Implementations (to make code compile) ---

// Custom allocator/deallocator
// These are likely wrappers around malloc/free, possibly with some tracking or security features.
// Returns 0 on success, non-zero on failure.
int allocate(size_t size, int flags, void **out_ptr) {
    *out_ptr = malloc(size);
    if (*out_ptr == NULL) {
        fprintf(stderr, "Allocation failed for size %zu\n", size);
        return 1; // Indicate failure
    }
    memset(*out_ptr, 0, size); // Initialize allocated memory to zero
    return 0; // Indicate success
}

// Custom deallocator. 'size' might be used by a custom allocator.
void deallocate(void *ptr, size_t size) {
    if (ptr) {
        // In a real scenario, 'size' might be used by a custom allocator
        // for memory pool management or debugging. For simple malloc/free, it's ignored.
        free(ptr);
    }
}

// Mimic bzero behavior for receive_until's buffer and reads from stdin.
// `terminator_char` is usually '\n' (ASCII 10).
void receive_until(char *buffer, int terminator_char, int max_len) {
    // Clear the buffer first as per bzero usage in original snippet
    memset(buffer, 0, max_len + 1); // +1 for null terminator

    if (fgets(buffer, max_len + 1, stdin) == NULL) {
        // Handle error or EOF
        buffer[0] = '\0'; // Ensure buffer is empty string
        return;
    }
    // Remove trailing newline if present, or other terminator char
    char *newline_pos = strchr(buffer, terminator_char);
    if (newline_pos != NULL) {
        *newline_pos = '\0';
    }
}

// Assumed to take a char* (from pml_buffer) and create a String_t object.
String_t* initString(void *data_buffer) {
    String_t *str_obj = (String_t*)malloc(sizeof(String_t)); // Assuming sizeof(String_t) is 12 bytes or similar
    if (str_obj) {
        str_obj->ref_count = 1; // Initialize reference count or length
        str_obj->data = strdup((char*)data_buffer); // Duplicate the string data
        if (!str_obj->data) {
            free(str_obj);
            return NULL;
        }
    }
    return str_obj;
}

// Frees the String_t object and its internal data.
void freeString(String_t *str_obj) {
    if (str_obj) {
        free(str_obj->data); // Free the duplicated string data
        free(str_obj);       // Free the String_t object itself
    }
}

// This function likely parses PML (Planet Markup Language) and creates a Planet object.
Planet* planetTopLevel(String_t *pml_string_obj) {
    printf("Parsing PML: %s\n", pml_string_obj->data);
    Planet *p = NULL;
    // Allocate a new Planet object
    if (allocate(sizeof(Planet), 0, (void**)&p) != 0) {
        return NULL; // Allocation failed
    }
    // For demonstration, just set a dummy name
    strncpy(p->name, "PML_Planet", PLANET_NAME_MAX_LEN);
    p->name[PLANET_NAME_MAX_LEN] = '\0';
    return p;
}

// This function initializes a newly allocated Planet object (other than its name).
void initPlanet(Planet *planet) {
    // The original code copies the name *after* this call.
    // So, this likely sets up other default fields or just ensures zeroing.
    // Our allocate already zeroes, so this might be redundant or for specific fields.
    if (planet) {
        // Example: planet->mass = 0; planet->radius = 0; etc.
    }
}

// Prints information about a planet.
void printPlanetInfo(Planet *planet) {
    if (planet) {
        printf("  Planet: %s\n", planet->name);
        // Print other info if the Planet struct had more fields
    } else {
        printf("  Planet: (Null)\n");
    }
}

// Presents a menu for a selected planet. Returns 0 if the planet should be removed, 1 otherwise.
int planetMenu(Planet *planet) {
    printf("--- Planet Menu for %s ---\n", planet->name);
    printf("1) Do something\n");
    printf("2) Delete Planet\n");
    printf("3) Back\n");
    printf("Selection: ");
    char menu_input[10];
    receive_until(menu_input, '\n', 9); // Max 9 chars for input
    int choice = atoi(menu_input);
    if (choice == 2) {
        printf("Deleting planet %s.\n", planet->name);
        deallocate(planet, sizeof(Planet)); // Deallocate the planet itself
        return 0; // Signal to remove from solarSystem array
    }
    return 1; // Keep the planet
}

// --- Main Function ---

int main(void) {
    int result; // Replaces iVar1
    void *pml_buffer = NULL; // Replaces local_40
    Planet *new_planet_ptr = NULL; // Replaces local_3c
    char input_buffer[30]; // Replaces local_36
    String_t *pml_string_obj = NULL; // Replaces local_18
    int selection = 0; // Replaces local_14

    // Initialize the global solarSystem array to all NULL pointers
    memset(solarSystem, 0, sizeof(solarSystem));

    while (true) { // Replaces do...while(true)
        printf("\nPlanet Markup Language Main\n");
        printf("1) Print Planets\n");
        printf("2) Add PML\n");
        printf("3) Add Planet\n");
        printf("4) Select Planet\n");
        printf("5) Exit\n");
        printf("Selection: ");

        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, '\n', 4); // Read up to 4 chars for selection
        selection = atoi(input_buffer);

        switch (selection) {
            default:
                printf("Invalid...\n");
                break;
            case 1:
                for (selection = 0; selection < 10; selection++) {
                    if (solarSystem[selection] != NULL) {
                        printPlanetInfo(solarSystem[selection]);
                    }
                }
                break;
            case 2:
                // Attempt to allocate 0x1000 bytes for PML input
                result = allocate(0x1000, 0, &pml_buffer);
                if (result == 0) { // Allocation successful
                    printf("PML: ");
                    // Read PML input into the allocated buffer
                    receive_until((char*)pml_buffer, '\n', 0xfff); // Read up to 0xfff chars

                    pml_string_obj = initString(pml_buffer);
                    deallocate(pml_buffer, 0x1000); // Free the temporary PML input buffer

                    if (pml_string_obj != NULL) {
                        // The original code `if (*local_18 == 0)` implies checking an int field.
                        // Assuming String_t's first member `ref_count` serves this purpose.
                        if (pml_string_obj->ref_count == 0) {
                            freeString(pml_string_obj); // Deallocate the String_t object
                        } else {
                            selection = 0; // Reuse 'selection' as loop counter
                            while (selection < 10) {
                                if (solarSystem[selection] == NULL) {
                                    Planet *new_p = planetTopLevel(pml_string_obj);
                                    solarSystem[selection] = new_p;
                                    if (solarSystem[selection] == NULL) break; // If planetTopLevel failed
                                    selection++;
                                } else {
                                    selection++; // Move to next slot if current one is occupied
                                }
                            }
                            freeString(pml_string_obj); // Free the String_t object after use
                        }
                    }
                } else { // Allocation failed
                    pml_buffer = NULL; // Ensure pointer is NULL
                }
                break;
            case 3:
                printf("\n-> ");
                memset(input_buffer, 0, sizeof(input_buffer));
                receive_until(input_buffer, '\n', PLANET_NAME_MAX_LEN); // Read planet name

                // Find the first empty slot in solarSystem
                for (selection = 0; (selection < 10 && (solarSystem[selection] != NULL)); selection++);

                if (selection != 10) { // If an empty slot was found
                    // Allocate memory for a new Planet object
                    result = allocate(sizeof(Planet), 0, (void**)&new_planet_ptr);
                    if (result == 0) { // Allocation successful
                        initPlanet(new_planet_ptr); // Initialize other planet fields
                        solarSystem[selection] = new_planet_ptr; // Assign to solarSystem slot

                        int name_idx = 0; // Use a separate index for name copying
                        // Copy alphanumeric characters from input_buffer to planet's name
                        while (isalnum((unsigned char)input_buffer[name_idx])) {
                            new_planet_ptr->name[name_idx] = input_buffer[name_idx];
                            name_idx++;
                            // Prevent buffer overflow in planet name
                            if (name_idx >= PLANET_NAME_MAX_LEN) break;
                        }
                        new_planet_ptr->name[name_idx] = '\0'; // Null-terminate the name
                    } else { // Allocation failed
                        new_planet_ptr = NULL; // Ensure pointer is NULL
                    }
                } else {
                    printf("Solar system is full! Cannot add more planets.\n");
                }
                break;
            case 4:
                // Print list of existing planets with numbers
                for (selection = 0; selection < 10; selection++) {
                    if (solarSystem[selection] != NULL) {
                        Planet *current_planet = solarSystem[selection];
                        printf("%d) %s\n", selection + 1, current_planet->name);
                    }
                }

                memset(input_buffer, 0, sizeof(input_buffer));
                printf("\n-> ");
                receive_until(input_buffer, '\n', 4); // Read selection for planet menu
                selection = atoi(input_buffer);

                if ((selection < 1) || (10 < selection)) {
                    printf("Invalid selection.\n");
                } else if (solarSystem[selection - 1] == NULL) {
                    printf("Invalid selection (no planet at that slot).\n");
                } else {
                    // Call planetMenu for the selected planet
                    result = planetMenu(solarSystem[selection - 1]);
                    if (result == 0) { // If planetMenu indicates deletion
                        solarSystem[selection - 1] = NULL; // Remove planet from solarSystem
                    }
                }
                break;
            case 5:
                printf("Exitting..\n");
                // In a real application, you might deallocate all planets here.
                for (selection = 0; selection < 10; selection++) {
                    if (solarSystem[selection] != NULL) {
                        deallocate(solarSystem[selection], sizeof(Planet));
                        solarSystem[selection] = NULL;
                    }
                }
                return 0; // Exit program
        }
    }
}