#include <stdio.h>   // For printf, fgets, stdin
#include <stdlib.h>  // For atoi, atof, malloc, free
#include <string.h>  // For memset, strlen, strcspn, strncpy, strcmp
#include <ctype.h>   // For isalpha, isalnum, isspace
#include <stdbool.h> // For bool type

// --- Mock/Placeholder definitions for external functions and types ---
// These are necessary for the code to compile. Their actual implementations are assumed to exist elsewhere.

// Forward declaration for Territory, as it's used in Country
typedef struct Territory Territory;

// Assuming a structure for Country based on offsets from the original code
typedef struct Country {
    char name[20];           // 0x0 - 0x13 (0x14 bytes)
    char capitol[20];        // 0x14 - 0x27 (0x14 bytes)
    int population;          // 0x28
    int language_count;      // 0x2c
    char *languages[10];     // 0x30 (10 * sizeof(char*))
    int border_count;        // 0x58
    double *borders[100];    // 0x5c (100 * sizeof(double*))
    int territory_count;     // 0x1ec
    Territory *territories[10]; // 0x1f0 (10 * sizeof(Territory*))
    // Total size 0x218 (536 bytes)
} Country;

// Simplified Territory structure (actual size 0x1fc from allocate in countryMenu)
struct Territory {
    char name[20]; // Example, based on usage in countryMenu
    // ... other fields for territory, if any, to reach size 0x1fc
};

// Mock functions for input/output
void receive_until(char *buffer, int delimiter, int max_len) {
    if (fgets(buffer, max_len + 1, stdin) != NULL) {
        // Remove newline character if present
        buffer[strcspn(buffer, "\n")] = 0;
    }
}

void receive(int fd, char *buffer, int max_len, int *bytes_read) {
    // fd parameter is ignored in this mock as we use stdin
    if (fgets(buffer, max_len + 1, stdin) != NULL) {
        // Remove newline character if present
        buffer[strcspn(buffer, "\n")] = 0;
        if (bytes_read) *bytes_read = strlen(buffer);
    } else {
        if (bytes_read) *bytes_read = 0;
    }
}

// Mock memory allocation functions
// Returns 0 on success, non-zero on failure (mimics original 'allocate' behavior)
int allocate(size_t size, int flags, void **ptr_out) {
    *ptr_out = malloc(size);
    return (*ptr_out == NULL);
}

void deallocate(void *ptr, size_t size) {
    free(ptr);
}

// Mock parsing state structure (param_1 in parsing functions)
typedef struct ParserState {
    char *buffer;
    int current_index;
    int prev_index; // Used by getIndex in original code
} ParserState;

void skipWhiteSpace(ParserState *state) {
    while (state->buffer[state->current_index] != '\0' && isspace((unsigned char)state->buffer[state->current_index])) {
        state->current_index++;
    }
}

int getIndex(ParserState *state, int *index_out) {
    if (index_out) *index_out = state->current_index;
    state->prev_index = state->current_index; // Store for potential use by other functions
    return state->current_index;
}

int atChar(ParserState *state, char c) {
    return state->buffer[state->current_index] == c;
}

// Returns new current_index on success, -1 on failure
int skipLength(ParserState *state, int length) {
    if (state->current_index + length <= strlen(state->buffer)) {
        state->current_index += length;
        return state->current_index;
    }
    return -1;
}

// Returns new current_index after skipping alpha chars
int skipAlpha(ParserState *state) {
    int start_index = state->current_index;
    while (state->buffer[state->current_index] != '\0' && isalpha((unsigned char)state->buffer[state->current_index])) {
        state->current_index++;
    }
    return state->current_index;
}

// Returns new current_index after skipping alphanumeric chars
int skipToNonAlphaNum(ParserState *state) {
    int start_index = state->current_index;
    while (state->buffer[state->current_index] != '\0' && isalnum((unsigned char)state->buffer[state->current_index])) {
        state->current_index++;
    }
    return state->current_index;
}

char *copyData(ParserState *state, int start, int end) {
    if (start < 0 || end < start || (size_t)end > strlen(state->buffer)) return NULL;
    size_t length = end - start;
    char *data = malloc(length + 1);
    if (data) {
        strncpy(data, state->buffer + start, length);
        data[length] = '\0';
    }
    return data;
}

// Increments current_index by 1, returns new index or -1 on error
int incChar(ParserState *state) {
    if (state->buffer[state->current_index] != '\0') {
        state->current_index++;
        return state->current_index;
    }
    return -1;
}

// Element enum for countryTopLevel parsing
typedef enum {
    ELEMENT_UNKNOWN = 0,
    ELEMENT_NAME = 1,
    ELEMENT_POPULATION = 10,
    ELEMENT_CAPITOL = 12,
    ELEMENT_LANGUAGE = 13,
    ELEMENT_BORDER = 14,
    ELEMENT_TERRITORY = 15
} ElementType;

ElementType elementNameToEnum(const char *name) {
    if (strcmp(name, "Name") == 0) return ELEMENT_NAME;
    if (strcmp(name, "Population") == 0) return ELEMENT_POPULATION;
    if (strcmp(name, "Capitol") == 0) return ELEMENT_CAPITOL;
    if (strcmp(name, "Language") == 0) return ELEMENT_LANGUAGE;
    if (strcmp(name, "Border") == 0) return ELEMENT_BORDER;
    if (strcmp(name, "Territory") == 0) return ELEMENT_TERRITORY;
    return ELEMENT_UNKNOWN;
}

char *pullNextElementName(ParserState *state) {
    skipWhiteSpace(state);
    if (!atChar(state, '{')) return NULL;
    if (skipLength(state, 1) == -1) return NULL;
    skipWhiteSpace(state);
    int start = state->current_index;
    int end = skipToNonAlphaNum(state);
    if (end == -1 || start == end) {
        // Rewind state for cleaner error handling if no element name found
        state->current_index = start - 1; // Back to '{'
        return NULL;
    }
    char *name = copyData(state, start, end);
    skipWhiteSpace(state);
    if (!atChar(state, '}')) {
        deallocate(name, strlen(name) + 1);
        state->current_index = start - 1; // Back to '{'
        return NULL;
    }
    if (skipLength(state, 1) == -1) {
        deallocate(name, strlen(name) + 1);
        state->current_index = start - 1; // Back to '{'
        return NULL;
    }
    return name;
}

// Forward declarations for actual functions
void initTerritory(Territory *t);
int territoryMenu(Territory *t);
void freeTerritory(Territory *t);
void printTerritoryInfo();
char *extractLanguage(ParserState *state);
char *extractCapitol(ParserState *state);
Country *countryTopLevel(ParserState *state);
void printCountryInfo(Country *country_ptr);
void freeCountry(Country *country_ptr);
void initCountry(Country *country_ptr);


// Mock extractName (simplified)
char *extractName(ParserState *state) {
    char *name_val = NULL;
    char *closing_tag = NULL;
    bool success = false;
    int original_idx = state->current_index;

    do {
        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip initial closing brace for Name\n"); break; }
        skipWhiteSpace(state);
        int start = state->current_index;
        int end = skipAlpha(state);
        if (start == end) { printf("!!Failed to find name data\n"); break; }
        name_val = copyData(state, start, end);
        if (!name_val) { printf("!!Failed to copy name data\n"); break; }
        skipWhiteSpace(state);
        if (!atChar(state, '{') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip final opening brace for Name\n"); break; }
        skipWhiteSpace(state);
        if (!atChar(state, '#') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip closing mark for Name\n"); break; }
        start = state->current_index;
        end = skipToNonAlphaNum(state);
        if (start == end) { printf("!!Failed to identify closing tag for Name\n"); break; }
        closing_tag = copyData(state, start, end);
        if (!closing_tag || strcmp(closing_tag, "Name") != 0) { printf("!!Invalid closing tag for Name\n"); break; }
        deallocate(closing_tag, strlen(closing_tag)+1); closing_tag = NULL;
        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip final closing brace for Name\n"); break; }
        success = true;
    } while(false);

    if (!success) {
        if (name_val) deallocate(name_val, strlen(name_val)+1);
        if (closing_tag) deallocate(closing_tag, strlen(closing_tag)+1);
        state->current_index = original_idx;
        return NULL;
    }
    return name_val;
}

// Mock extractPopulation (simplified)
int extractPopulation(ParserState *state) {
    int population = -1;
    char *pop_str = NULL;
    char *closing_tag = NULL;
    bool success = false;
    int original_idx = state->current_index;

    do {
        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip initial closing brace for Population\n"); break; }
        skipWhiteSpace(state);
        int start = state->current_index;
        while (state->buffer[state->current_index] != '\0' && isdigit((unsigned char)state->buffer[state->current_index])) {
            state->current_index++;
        }
        int end = state->current_index;
        if (start == end) { printf("!!Failed to find population data\n"); break; }
        pop_str = copyData(state, start, end);
        if (!pop_str) { printf("!!Failed to copy population data\n"); break; }
        population = atoi(pop_str);
        deallocate(pop_str, strlen(pop_str)+1); pop_str = NULL;
        skipWhiteSpace(state);
        if (!atChar(state, '{') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip final opening brace for Population\n"); break; }
        skipWhiteSpace(state);
        if (!atChar(state, '#') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip closing mark for Population\n"); break; }
        start = state->current_index;
        end = skipToNonAlphaNum(state);
        if (start == end) { printf("!!Failed to identify closing tag for Population\n"); break; }
        closing_tag = copyData(state, start, end);
        if (!closing_tag || strcmp(closing_tag, "Population") != 0) { printf("!!Invalid closing tag for Population\n"); break; }
        deallocate(closing_tag, strlen(closing_tag)+1); closing_tag = NULL;
        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip final closing brace for Population\n"); break; }
        success = true;
    } while(false);

    if (!success) {
        if (pop_str) deallocate(pop_str, strlen(pop_str)+1);
        if (closing_tag) deallocate(closing_tag, strlen(closing_tag)+1);
        state->current_index = original_idx;
        return -1;
    }
    return population;
}

void initTerritory(Territory *t) {
    if (t) {
        memset(t, 0, sizeof(Territory));
    }
}

int territoryMenu(Territory *t) {
    if (!t) return 1; // Indicate exit if null territory

    printf("Territory menu for %s (mock)\n", t->name);
    printf("1) Delete Territory\n");
    printf("2) Exit Territory Menu\n");
    char input[10];
    receive_until(input, 10, sizeof(input) - 1);
    int choice = atoi(input);
    if (choice == 1) {
        return 0; // Indicate deletion
    }
    return 1; // Indicate not deleted (or exit)
}

void freeTerritory(Territory *t) {
    if (t) {
        deallocate(t, sizeof(Territory));
    }
}

void printTerritoryInfo() {
    printf("\t\tTerritory: (mock info)\n");
}

double *extractBorder(ParserState *state) {
    double *border_coords = NULL;
    char *closing_tag = NULL;
    bool success = false;
    int original_idx = state->current_index;

    do {
        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip initial closing brace for Border\n"); break; }

        if (allocate(sizeof(double) * 4, 0, (void**)&border_coords) != 0) {
            printf("!!Failed to allocate memory for border coordinates\n");
            break;
        }

        // Read 4 doubles
        for (int i = 0; i < 4; ++i) {
            skipWhiteSpace(state);
            int start = state->current_index;
            while (state->buffer[state->current_index] != '\0' && (isdigit((unsigned char)state->buffer[state->current_index]) || state->buffer[state->current_index] == '.' || state->buffer[state->current_index] == '-')) {
                state->current_index++;
            }
            int end = state->current_index;
            if (start == end) { printf("!!Failed to find border coordinate %d\n", i); break; }
            char *num_str = copyData(state, start, end);
            if (!num_str) { printf("!!Failed to copy border coordinate %d string\n", i); break; }
            border_coords[i] = atof(num_str);
            deallocate(num_str, strlen(num_str) + 1);
        }
        if (border_coords == NULL) break; // If an error occurred in the loop

        skipWhiteSpace(state);
        if (!atChar(state, '{') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip final opening brace for Border\n"); break; }
        skipWhiteSpace(state);
        if (!atChar(state, '#') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip closing mark for Border\n"); break; }
        int start = state->current_index;
        int end = skipToNonAlphaNum(state);
        if (start == end) { printf("!!Failed to identify closing tag for Border\n"); break; }
        closing_tag = copyData(state, start, end);
        if (!closing_tag || strcmp(closing_tag, "Border") != 0) { printf("!!Invalid closing tag for Border\n"); break; }
        deallocate(closing_tag, strlen(closing_tag)+1); closing_tag = NULL;
        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip final closing brace for Border\n"); break; }
        success = true;
    } while(false);

    if (!success) {
        if (border_coords) deallocate(border_coords, sizeof(double) * 4);
        if (closing_tag) deallocate(closing_tag, strlen(closing_tag)+1);
        state->current_index = original_idx;
        return NULL;
    }
    return border_coords;
}

Territory *territoryTopLevel(ParserState *state) {
    Territory *new_territory = NULL;
    char *territory_name = NULL;
    char *closing_tag = NULL;
    bool success = false;
    int original_idx = state->current_index;

    do {
        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip initial closing brace for Territory\n"); break; }
        skipWhiteSpace(state);
        int start = state->current_index;
        int end = skipAlpha(state);
        if (start == end) { printf("!!Failed to find territory name data\n"); break; }
        territory_name = copyData(state, start, end);
        if (!territory_name) { printf("!!Failed to copy territory name data\n"); break; }

        if (allocate(sizeof(Territory), 0, (void**)&new_territory) != 0) {
            printf("!!Failed to allocate memory for new territory\n");
            break;
        }
        initTerritory(new_territory);
        strncpy(new_territory->name, territory_name, sizeof(new_territory->name) - 1);
        new_territory->name[sizeof(new_territory->name) - 1] = '\0';
        deallocate(territory_name, strlen(territory_name)+1); territory_name = NULL;

        skipWhiteSpace(state);
        if (!atChar(state, '{') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip final opening brace for Territory\n"); break; }
        skipWhiteSpace(state);
        if (!atChar(state, '#') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip closing mark for Territory\n"); break; }
        start = state->current_index;
        end = skipToNonAlphaNum(state);
        if (start == end) { printf("!!Failed to identify closing tag for Territory\n"); break; }
        closing_tag = copyData(state, start, end);
        if (!closing_tag || strcmp(closing_tag, "Territory") != 0) { printf("!!Invalid closing tag for Territory\n"); break; }
        deallocate(closing_tag, strlen(closing_tag)+1); closing_tag = NULL;
        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) { printf("!!Failed to locate or skip final closing brace for Territory\n"); break; }
        success = true;
    } while(false);

    if (!success) {
        if (new_territory) deallocate(new_territory, sizeof(Territory));
        if (territory_name) deallocate(territory_name, strlen(territory_name)+1);
        if (closing_tag) deallocate(closing_tag, strlen(closing_tag)+1);
        state->current_index = original_idx;
        return NULL;
    }
    return new_territory;
}

// --- End of Mock/Placeholder definitions ---


// Function: countryMenu
int countryMenu(Country *country_ptr) {
    char input_buffer[40];
    int selection;
    int char_idx; // Used for various character index operations, like string copying/bytes read

    if (!country_ptr) {
        return 0; // Return 0 (failure/exit) if country is null
    }

    while (true) { // Main menu loop
        printf("\nCountry: %s\n", country_ptr->name);
        printf("1) Display CountryInfo\n");
        printf("2) Set Capitol\n");
        printf("3) Set Population\n");
        printf("4) Add Language\n");
        printf("5) Add border\n");
        printf("6) Add Territory\n");
        printf("7) Select Territory\n");
        printf("8) Delete Country and Exit Menu\n");
        printf("9) Exit menu\n");

        memset(input_buffer, 0, sizeof(input_buffer));
        printf("Selection: ");
        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
        selection = atoi(input_buffer);

        if (selection < 1 || selection > 9) {
            printf("Invalid...\n");
            continue; // Re-display menu
        }

        switch (selection) {
            case 1:
                printCountryInfo(country_ptr);
                break;
            case 2: {
                printf("\n-> ");
                memset(input_buffer, 0, sizeof(input_buffer));
                receive_until(input_buffer, '\n', sizeof(country_ptr->capitol) - 1);
                // Copy only alpha characters to capitol
                char_idx = 0;
                while (input_buffer[char_idx] != '\0' && isalpha((unsigned char)input_buffer[char_idx]) && char_idx < sizeof(country_ptr->capitol) - 1) {
                    country_ptr->capitol[char_idx] = input_buffer[char_idx];
                    char_idx++;
                }
                country_ptr->capitol[char_idx] = '\0'; // Null-terminate
                break;
            }
            case 3: {
                printf("\n-> ");
                memset(input_buffer, 0, sizeof(input_buffer));
                receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                country_ptr->population = atoi(input_buffer);
                break;
            }
            case 4: {
                if (country_ptr->language_count < 10) {
                    printf("\n-> ");
                    memset(input_buffer, 0, sizeof(input_buffer));
                    receive(0, input_buffer, 19, &char_idx); // 0x13 (19) max length from original

                    size_t lang_len = strlen(input_buffer);
                    char *new_language = NULL;
                    if (allocate(lang_len + 1, 0, (void **)&new_language) == 0) { // Success
                        memset(new_language, 0, lang_len + 1);
                        char_idx = 0;
                        while (input_buffer[char_idx] != '\0' && isalpha((unsigned char)input_buffer[char_idx]) && char_idx < lang_len) {
                            new_language[char_idx] = input_buffer[char_idx];
                            char_idx++;
                        }
                        new_language[char_idx] = '\0'; // Null-terminate
                        country_ptr->languages[country_ptr->language_count] = new_language;
                        country_ptr->language_count++;
                    } else {
                        printf("!!Failed to allocate memory for language\n");
                    }
                } else {
                    printf("!!Max languages reached\n");
                }
                break;
            }
            case 5: {
                if (country_ptr->border_count < 100) {
                    double *new_border_coords = NULL;
                    // Allocate 4 doubles for Lat Start, Long Start, Lat End, Long End
                    if (allocate(sizeof(double) * 4, 0, (void **)&new_border_coords) == 0) { // Success
                        printf("Lat Start: ");
                        memset(input_buffer, 0, sizeof(input_buffer));
                        receive(0, input_buffer, 19, &char_idx);
                        new_border_coords[0] = atof(input_buffer);

                        printf("Long Start: ");
                        memset(input_buffer, 0, sizeof(input_buffer));
                        receive(0, input_buffer, 19, &char_idx);
                        new_border_coords[1] = atof(input_buffer);

                        printf("Lat End: ");
                        memset(input_buffer, 0, sizeof(input_buffer));
                        receive(0, input_buffer, 19, &char_idx);
                        new_border_coords[2] = atof(input_buffer);

                        printf("Long End: ");
                        memset(input_buffer, 0, sizeof(input_buffer));
                        receive(0, input_buffer, 19, &char_idx);
                        new_border_coords[3] = atof(input_buffer);

                        country_ptr->borders[country_ptr->border_count] = new_border_coords;
                        country_ptr->border_count++;
                    } else {
                        printf("!!Failed to allocate memory for border\n");
                    }
                } else {
                    printf("!!Max borders reached\n");
                }
                break;
            }
            case 6: {
                if (country_ptr->territory_count < 10) {
                    int territory_idx = 0;
                    // Find the first empty slot for a new territory
                    while (territory_idx < 10 && country_ptr->territories[territory_idx] != NULL) {
                        territory_idx++;
                    }

                    if (territory_idx == 10) {
                        printf("!!Max Territories reached (no empty slots)\n");
                    } else {
                        Territory *new_territory = NULL;
                        // 0x1fc is the size of Territory struct, based on allocation in original code
                        if (allocate(0x1fc, 0, (void **)&new_territory) == 0) { // Success
                            initTerritory(new_territory);
                            printf("\nNew Territory: ");
                            memset(input_buffer, 0, sizeof(input_buffer));
                            receive_until(input_buffer, '\n', sizeof(input_buffer) - 1); // Max 19 chars for name based on original

                            char_idx = 0;
                            while (input_buffer[char_idx] != '\0' && isalnum((unsigned char)input_buffer[char_idx]) && char_idx < sizeof(new_territory->name) - 1) {
                                new_territory->name[char_idx] = input_buffer[char_idx];
                                char_idx++;
                            }
                            new_territory->name[char_idx] = '\0'; // Null-terminate

                            country_ptr->territories[territory_idx] = new_territory;
                            country_ptr->territory_count++;
                        } else {
                            printf("!!Failed to allocate structure\n");
                            country_ptr->territories[territory_idx] = NULL; // Ensure slot is null on failure
                        }
                    }
                } else {
                    printf("!!Max Territories reached\n");
                }
                break;
            }
            case 7: {
                printf("\nTerritories:\n");
                for (int i = 0; i < 10; i++) {
                    if (country_ptr->territories[i] != NULL) {
                        printf("%d) %s\n", i + 1, country_ptr->territories[i]->name);
                    }
                }
                memset(input_buffer, 0, sizeof(input_buffer));
                printf("\n-> ");
                receive(0, input_buffer, 3, &char_idx); // Max 3 chars for selection (1-10)
                int territory_selection = atoi(input_buffer);

                if (territory_selection < 1 || territory_selection > 10 || country_ptr->territories[territory_selection - 1] == NULL) {
                    printf("Invalid choice...\n");
                } else {
                    // territoryMenu returns 0 if territory was deleted
                    if (territoryMenu(country_ptr->territories[territory_selection - 1]) == 0) {
                        freeTerritory(country_ptr->territories[territory_selection - 1]); // Free the memory
                        country_ptr->territories[territory_selection - 1] = NULL; // Clear pointer in array
                        country_ptr->territory_count--;
                    }
                }
                break;
            }
            case 8:
                freeCountry(country_ptr);
                return 0; // Exit menu and indicate country was deleted
            case 9:
                return 1; // Exit menu
        }
    }
}

// Function: printCountryInfo
void printCountryInfo(Country *country_ptr) {
    if (!country_ptr) {
        return;
    }

    printf("\tCountry: ");
    if (country_ptr->name[0] == '\0') {
        printf("Unknown\n");
    } else {
        printf("%s\n", country_ptr->name);
    }

    printf("\t\tCapitol: ");
    if (country_ptr->capitol[0] == '\0') {
        printf("Unknown\n");
    } else {
        printf("%s\n", country_ptr->capitol);
    }

    if (country_ptr->population >= 0) { // Original used -1 < population, meaning >= 0
        printf("\t\tPopulation: %d\n", country_ptr->population);
    }

    for (int i = 0; i < country_ptr->language_count; i++) {
        if (country_ptr->languages[i] != NULL) {
            printf("\t\tLanguage: %s\n", country_ptr->languages[i]);
        }
    }

    for (int i = 0; i < country_ptr->border_count; i++) {
        double *border_coords = country_ptr->borders[i];
        if (border_coords != NULL) {
            printf("\t\tBorder: %f %f %f %f\n", border_coords[0], border_coords[1], border_coords[2], border_coords[3]);
        }
    }

    for (int i = 0; i < 10; i++) { // Iterate through all potential territory slots
        if (country_ptr->territories[i] != NULL) {
            printTerritoryInfo(); // This mock doesn't take args, original also didn't.
        }
    }
}

// Function: freeCountry
void freeCountry(Country *country_ptr) {
    if (!country_ptr) {
        return;
    }

    // Free borders
    for (int i = 0; i < country_ptr->border_count; i++) {
        if (country_ptr->borders[i] != NULL) {
            deallocate(country_ptr->borders[i], sizeof(double) * 4); // Each border is 4 doubles
            country_ptr->borders[i] = NULL;
        }
    }

    // Free languages
    for (int i = 0; i < country_ptr->language_count; i++) {
        if (country_ptr->languages[i] != NULL) {
            deallocate(country_ptr->languages[i], strlen(country_ptr->languages[i]) + 1);
            country_ptr->languages[i] = NULL;
        }
    }

    // Free territories
    for (int i = 0; i < 10; i++) { // Iterate through all potential territory slots
        if (country_ptr->territories[i] != NULL) {
            freeTerritory(country_ptr->territories[i]); // Calls mock freeTerritory
            country_ptr->territories[i] = NULL;
        }
    }

    // Finally, free the country structure itself
    deallocate(country_ptr, sizeof(Country));
}

// Function: initCountry
void initCountry(Country *country_ptr) {
    if (!country_ptr) {
        return;
    }
    // Initialize all members to a known state
    memset(country_ptr->name, 0, sizeof(country_ptr->name));
    memset(country_ptr->capitol, 0, sizeof(country_ptr->capitol));

    country_ptr->population = -1; // Matches 0xffffffff from original
    country_ptr->language_count = 0;
    country_ptr->border_count = 0;
    country_ptr->territory_count = 0;

    memset(country_ptr->languages, 0, sizeof(country_ptr->languages));
    memset(country_ptr->borders, 0, sizeof(country_ptr->borders));
    memset(country_ptr->territories, 0, sizeof(country_ptr->territories));
}

// Function: countryTopLevel
Country *countryTopLevel(ParserState *state) {
    Country *new_country = NULL;
    char *element_id_str = NULL;
    char *extracted_data = NULL;
    int original_state_index = state->current_index; // To revert parser state on error
    bool success = false; // Flag to track overall success

    if (!state) {
        return NULL;
    }

    if (allocate(sizeof(Country), 0, (void **)&new_country) != 0) { // Allocation failed
        return NULL;
    }
    initCountry(new_country);

    do { // Using a do-while(false) loop to simulate a single-exit point without goto
        skipWhiteSpace(state);
        getIndex(state, NULL); // Store initial index

        if (!atChar(state, '{') || skipLength(state, 1) == -1) {
            printf("!!Country: Failed to locate or skip opening brace\n");
            break; // Exit do-while
        }

        skipWhiteSpace(state);
        int start_idx = state->current_index;
        int end_idx = skipToNonAlphaNum(state);

        if (end_idx == -1 || start_idx == end_idx) {
            printf("!!Country: Failed to identify element ID\n");
            break;
        }

        element_id_str = copyData(state, start_idx, end_idx);
        if (!element_id_str) {
            printf("!!Country: Failed to copy element ID\n");
            break;
        }

        if (strcmp(element_id_str, "Country") != 0) {
            printf("!!Country: Invalid opening element id: %s\n", element_id_str);
            break;
        }
        deallocate(element_id_str, strlen(element_id_str) + 1);
        element_id_str = NULL;

        skipWhiteSpace(state);
        if (!atChar(state, '}') || incChar(state) == -1) {
            printf("!!Country: Failed to locate or skip initial closing brace\n");
            break;
        }

        // Process sub-elements
        while ((extracted_data = pullNextElementName(state)) != NULL) {
            ElementType element_type = elementNameToEnum(extracted_data);
            deallocate(extracted_data, strlen(extracted_data) + 1);
            extracted_data = NULL; // Reset extracted_data for next iteration or error check

            switch (element_type) {
                case ELEMENT_NAME:
                    extracted_data = extractName(state);
                    if (!extracted_data) { success = false; break; }
                    strncpy(new_country->name, extracted_data, sizeof(new_country->name) - 1);
                    new_country->name[sizeof(new_country->name) - 1] = '\0';
                    deallocate(extracted_data, strlen(extracted_data) + 1);
                    extracted_data = NULL;
                    break;
                case ELEMENT_POPULATION:
                    new_country->population = extractPopulation(state);
                    if (new_country->population < 0) { success = false; break; }
                    break;
                case ELEMENT_CAPITOL:
                    extracted_data = extractCapitol(state);
                    if (!extracted_data) { success = false; break; }
                    strncpy(new_country->capitol, extracted_data, sizeof(new_country->capitol) - 1);
                    new_country->capitol[sizeof(new_country->capitol) - 1] = '\0';
                    deallocate(extracted_data, strlen(extracted_data) + 1);
                    extracted_data = NULL;
                    break;
                case ELEMENT_LANGUAGE:
                    if (new_country->language_count >= 10) {
                        printf("!!Max country language count is %d\n", 10);
                        success = false; break;
                    }
                    char *new_language = extractLanguage(state);
                    if (!new_language) { success = false; break; }
                    new_country->languages[new_country->language_count++] = new_language;
                    break;
                case ELEMENT_BORDER:
                    if (new_country->border_count >= 100) {
                        printf("!!Max country border count is %d\n", 100);
                        success = false; break;
                    }
                    double *border_coords = extractBorder(state);
                    if (!border_coords) { success = false; break; }
                    new_country->borders[new_country->border_count++] = border_coords;
                    break;
                case ELEMENT_TERRITORY:
                    if (new_country->territory_count >= 10) {
                        printf("!!Max territories is %d\n", 10);
                        success = false; break;
                    }
                    Territory *new_territory = territoryTopLevel(state);
                    if (!new_territory) { success = false; break; }
                    new_country->territories[new_country->territory_count++] = new_territory;
                    break;
                default:
                    printf("Invalid element for country: %s\n", extracted_data ? extracted_data : "Unknown");
                    success = false; break;
            }
            if (!success) break; // If an error occurred in switch, exit parsing loop
        }
        if (!success && extracted_data == NULL) break; // If loop exited due to error in switch, or pullNextElementName failed

        // Check closing tag
        skipWhiteSpace(state);
        if (!atChar(state, '{') || skipLength(state, 1) == -1) {
            printf("!!Country: Failed to locate or skip final opening brace\n");
            break;
        }
        skipWhiteSpace(state);
        if (!atChar(state, '#') || skipLength(state, 1) == -1) {
            printf("!!Country: Failed to locate or skip closing mark\n");
            break;
        }

        start_idx = state->current_index;
        end_idx = skipAlpha(state);
        if (end_idx == -1 || start_idx == end_idx) {
            printf("!!Country: Failed to identify closing element ID\n");
            break;
        }
        element_id_str = copyData(state, start_idx, end_idx);
        if (!element_id_str) {
            printf("!!Country: Failed to copy closing element ID\n");
            break;
        }

        if (strcmp(element_id_str, "Country") != 0) {
            printf("!!Country: Invalid closing element id: %s\n", element_id_str);
            break;
        }
        deallocate(element_id_str, strlen(element_id_str) + 1);
        element_id_str = NULL;

        skipWhiteSpace(state);
        if (!atChar(state, '}') || incChar(state) == -1) {
            printf("!!Country: Failed to locate or skip final closing brace\n");
            break;
        }

        success = true; // All checks passed
    } while (false); // Execute loop once

    if (!success) {
        if (new_country) {
            freeCountry(new_country);
            new_country = NULL;
        }
        if (element_id_str) {
            deallocate(element_id_str, strlen(element_id_str) + 1);
        }
        if (extracted_data) {
            deallocate(extracted_data, strlen(extracted_data) + 1);
        }
        state->current_index = original_state_index; // Revert parser state
        printf("Error at: %s\n", state->buffer + state->current_index);
    }
    return new_country;
}

// Function: extractLanguage
char *extractLanguage(ParserState *state) {
    char *language_data = NULL;
    char *element_id = NULL;
    int start_idx, end_idx;
    bool success = false;

    if (!state) return NULL;

    int original_idx = state->current_index; // For error recovery

    do { // Single-pass loop to replace goto
        skipWhiteSpace(state);
        if (!atChar(state, '{') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip opening brace for Language\n");
            break;
        }

        skipWhiteSpace(state);
        start_idx = state->current_index;
        end_idx = skipToNonAlphaNum(state);
        if (end_idx == -1 || start_idx == end_idx) {
            printf("!!Failed to locate the end of the element id for Language\n");
            break;
        }
        element_id = copyData(state, start_idx, end_idx);
        if (!element_id) {
            printf("!!Failed to copy element id for Language\n");
            break;
        }

        if (strcmp(element_id, "Language") != 0) {
            printf("!!Element id is not \"Language\": %s\n", element_id);
            break;
        }
        deallocate(element_id, strlen(element_id) + 1);
        element_id = NULL;

        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip initial closing brace for Language\n");
            break;
        }

        skipWhiteSpace(state);
        start_idx = state->current_index;
        end_idx = skipAlpha(state);
        if (start_idx == end_idx) {
            printf("!!Failed to find language data\n");
            break;
        }
        language_data = copyData(state, start_idx, end_idx);
        if (!language_data) {
            printf("!!Failed to copy language data\n");
            break;
        }

        skipWhiteSpace(state);
        if (!atChar(state, '{') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip the final opening brace for Language\n");
            break;
        }

        skipWhiteSpace(state);
        if (!atChar(state, '#') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip the closing mark for Language\n");
            break;
        }

        start_idx = state->current_index;
        end_idx = skipToNonAlphaNum(state);
        if (end_idx == -1 || start_idx == end_idx) {
            printf("!!Failed to locate the end of the closing element id for Language\n");
            break;
        }
        element_id = copyData(state, start_idx, end_idx);
        if (!element_id) {
            printf("!!Failed to copy closing element id for Language\n");
            break;
        }

        if (strcmp(element_id, "Language") != 0) {
            printf("!!Invalid closing element id: %s\n", element_id);
            break;
        }
        deallocate(element_id, strlen(element_id) + 1);
        element_id = NULL;

        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip final closing brace for Language\n");
            break;
        }
        success = true;
    } while (false);

    if (!success) {
        if (element_id) deallocate(element_id, strlen(element_id) + 1);
        if (language_data) deallocate(language_data, strlen(language_data) + 1);
        state->current_index = original_idx; // Revert parser state
        return NULL;
    }
    return language_data;
}

// Function: extractCapitol
char *extractCapitol(ParserState *state) {
    char *capitol_data = NULL;
    char *element_id = NULL;
    int start_idx, end_idx;
    bool success = false;

    if (!state) return NULL;

    int original_idx = state->current_index; // For error recovery

    do { // Single-pass loop to replace goto
        skipWhiteSpace(state);
        if (!atChar(state, '{') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip opening brace for Capitol\n");
            break;
        }

        skipWhiteSpace(state);
        start_idx = state->current_index;
        end_idx = skipToNonAlphaNum(state);
        if (end_idx == -1 || start_idx == end_idx) {
            printf("!!Failed to locate the end of the element id for Capitol\n");
            break;
        }
        element_id = copyData(state, start_idx, end_idx);
        if (!element_id) {
            printf("!!Failed to copy element id for Capitol\n");
            break;
        }

        if (strcmp(element_id, "Capitol") != 0) {
            printf("!!Element id is not \"Capitol\": %s\n", element_id);
            break;
        }
        deallocate(element_id, strlen(element_id) + 1);
        element_id = NULL;

        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip initial closing brace for Capitol\n");
            break;
        }

        skipWhiteSpace(state);
        start_idx = state->current_index;
        end_idx = skipAlpha(state);
        if (start_idx == end_idx) {
            printf("!!Failed to find capitol data\n");
            break;
        }
        capitol_data = copyData(state, start_idx, end_idx);
        if (!capitol_data) {
            printf("!!Failed to copy capitol data\n");
            break;
        }

        skipWhiteSpace(state);
        if (!atChar(state, '{') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip the final opening brace for Capitol\n");
            break;
        }

        skipWhiteSpace(state);
        if (!atChar(state, '#') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip the closing mark for Capitol\n");
            break;
        }

        start_idx = state->current_index;
        end_idx = skipToNonAlphaNum(state);
        if (end_idx == -1 || start_idx == end_idx) {
            printf("!!Failed to locate the end of the closing element id for Capitol\n");
            break;
        }
        element_id = copyData(state, start_idx, end_idx);
        if (!element_id) {
            printf("!!Failed to copy closing element id for Capitol\n");
            break;
        }

        if (strcmp(element_id, "Capitol") != 0) {
            printf("!!Invalid closing element id: %s\n", element_id);
            break;
        }
        deallocate(element_id, strlen(element_id) + 1);
        element_id = NULL;

        skipWhiteSpace(state);
        if (!atChar(state, '}') || skipLength(state, 1) == -1) {
            printf("!!Failed to locate or skip final closing brace for Capitol\n");
            break;
        }
        success = true;
    } while (false);

    if (!success) {
        if (element_id) deallocate(element_id, strlen(element_id) + 1);
        if (capitol_data) deallocate(capitol_data, strlen(capitol_data) + 1);
        state->current_index = original_idx; // Revert parser state
        return NULL;
    }
    return capitol_data;
}