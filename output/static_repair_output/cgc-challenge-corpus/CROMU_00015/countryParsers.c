#include <stdio.h>    // For printf, stdin, fgets
#include <stdlib.h>   // For atoi, atof, malloc, free
#include <string.h>   // For memset, strlen, strncpy, strcmp, strcspn
#include <ctype.h>    // For isalpha, isalnum, isspace
#include <math.h>     // For atof (though it's in stdlib.h, it's often associated with math operations)

// Dummy declarations for external functions and types
// In a real scenario, these would be defined elsewhere or in included headers.

// Enum for element types, inferred from usage in countryTopLevel
typedef enum {
    ELEMENT_UNKNOWN = -1,
    ELEMENT_COUNTRY_NAME = 1,
    ELEMENT_POPULATION = 10,
    ELEMENT_CAPITOL = 12,
    ELEMENT_LANGUAGE = 13,
    ELEMENT_BORDER = 14,
    ELEMENT_TERRITORY = 15
} ElementType;

// Structure for parsing context, inferred from usage in countryTopLevel, extractLanguage, extractCapitol
typedef struct ParseContext {
    char* buffer;        // The string being parsed
    int current_index;   // Current position in the buffer
    // Add other fields as needed for parsing context
} ParseContext;

// Dummy memory management wrappers
// Returns 0 on success, non-zero on failure
int allocate(size_t size, int type_unused, void** ptr_out) {
    *ptr_out = malloc(size);
    if (*ptr_out == NULL) {
        fprintf(stderr, "Memory allocation failed for size %zu\n", size);
        return 1; // Failure
    }
    memset(*ptr_out, 0, size); // Initialize allocated memory to zero
    return 0; // Success
}

// Dummy deallocation wrapper
void deallocate(void* ptr, size_t size_unused) {
    free(ptr);
}

// Dummy external functions (replace with actual implementations if available)
// These are simplified for compilation purposes.
void receive_until(char* buffer, int max_len, int type_unused) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        // Handle error or EOF, e.g., clear buffer
        buffer[0] = '\0';
    }
    // Remove trailing newline if present
    buffer[strcspn(buffer, "\n")] = '\0';
}

void receive(int fd_unused, char* buffer, int max_len, int* bytes_read) {
    // Dummy implementation: read from stdin
    if (fgets(buffer, max_len, stdin) == NULL) {
        *bytes_read = -1; // Indicate error
        buffer[0] = '\0';
    } else {
        *bytes_read = strlen(buffer);
        // Remove trailing newline if present
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

// Skips whitespace characters and returns the new current_index
int skipWhiteSpace(ParseContext* ctx) {
    while (ctx->buffer[ctx->current_index] != '\0' && isspace((unsigned char)ctx->buffer[ctx->current_index])) {
        ctx->current_index++;
    }
    return ctx->current_index;
}

// Gets the current index and optionally stores it in a pointer
int getIndex(ParseContext* ctx, int* index) {
    if (index) {
        *index = ctx->current_index;
    }
    return ctx->current_index;
}

// Checks if the character at current_index matches 'c'
int atChar(ParseContext* ctx, char c) {
    return (ctx->buffer[ctx->current_index] == c);
}

// Skips 'length' characters forward. Returns 0 on success, -1 on failure (e.g., out of bounds).
int skipLength(ParseContext* ctx, int length) {
    // Basic bounds check, actual implementation would be more robust
    if (ctx->current_index + length > strlen(ctx->buffer)) {
        return -1;
    }
    ctx->current_index += length;
    return 0;
}

// Skips alphabetic characters and returns the new current_index
int skipAlpha(ParseContext* ctx) {
    while (ctx->buffer[ctx->current_index] != '\0' && isalpha((unsigned char)ctx->buffer[ctx->current_index])) {
        ctx->current_index++;
    }
    return ctx->current_index;
}

// Skips alphanumeric characters and returns the new current_index
int skipToNonAlphaNum(ParseContext* ctx) {
    while (ctx->buffer[ctx->current_index] != '\0' && isalnum((unsigned char)ctx->buffer[ctx->current_index])) {
        ctx->current_index++;
    }
    return ctx->current_index;
}

// Copies data from start_idx to end_idx (exclusive) from the buffer
void* copyData(ParseContext* ctx, int start_idx, int end_idx) {
    int len = end_idx - start_idx;
    if (len <= 0) return NULL;
    char* data = (char*)malloc(len + 1);
    if (data) {
        strncpy(data, ctx->buffer + start_idx, len);
        data[len] = '\0';
    }
    return data;
}

// Increments current_index by one. Returns 0 on success, -1 on failure.
int incChar(ParseContext* ctx) {
    if (ctx->buffer[ctx->current_index] == '\0') {
        return -1; // Cannot increment past null terminator
    }
    ctx->current_index++;
    return 0;
}

// Dummy territory functions
void initTerritory(void* territory_ptr) {
    printf("Initializing territory at %p\n", territory_ptr);
    // In a real implementation, this would initialize a Territory struct
}
int territoryMenu(void* territory_ptr) {
    printf("Territory menu entered for %p\n", territory_ptr);
    // Dummy: returns 1 (exit menu) or 0 (deleted)
    // For simplicity, always exit without deleting.
    return 1;
}
void printTerritoryInfo(void* territory_ptr) {
    printf("\t\tTerritory Info for %p (assuming name at start: %s)\n", territory_ptr, (char*)territory_ptr); // Dummy
}
void freeTerritory(void* territory_ptr) {
    printf("Freeing territory at %p\n", territory_ptr);
    deallocate(territory_ptr, 0); // Size 0 is dummy for deallocate
}
void* territoryTopLevel(ParseContext* ctx) {
    printf("Parsing territory from context at index %d\n", ctx->current_index);
    // Dummy: always allocate a dummy territory
    void* territory_ptr = NULL;
    if (allocate(0x1fc, 0, &territory_ptr) == 0) {
        // Assume territory name is just "DummyTerritory" for now
        strncpy((char*)territory_ptr, "DummyTerritory", 0x1fc - 1);
    }
    return territory_ptr;
}

// Dummy country parsing helper functions
char* extractName(ParseContext* ctx) {
    printf("Extracting country name from context at index %d\n", ctx->current_index);
    // Dummy: simulate extracting "TestCountryName"
    char* name = (char*)malloc(20);
    if (name) strncpy(name, "TestCountryName", 19);
    skipLength(ctx, 10); // Simulate parsing some data
    return name;
}
int extractPopulation(ParseContext* ctx) {
    printf("Extracting population from context at index %d\n", ctx->current_index);
    // Dummy: simulate extracting 1000000
    skipLength(ctx, 5); // Simulate parsing some data
    return 1000000;
}
double* extractBorder(ParseContext* ctx) {
    printf("Extracting border from context at index %d\n", ctx->current_index);
    // Dummy: simulate extracting 4 doubles
    double* border_coords = NULL;
    if (allocate(4 * sizeof(double), 0, (void**)&border_coords) == 0) {
        border_coords[0] = 1.0; border_coords[1] = 2.0;
        border_coords[2] = 3.0; border_coords[3] = 4.0;
    }
    skipLength(ctx, 20); // Simulate parsing some data
    return border_coords;
}
char* extractLanguage(ParseContext* ctx); // Forward declare
char* extractCapitol(ParseContext* ctx);  // Forward declare

// Function to convert element name string to enum, inferred from usage
ElementType elementNameToEnum(char* name) {
    if (name == NULL) return ELEMENT_UNKNOWN;
    if (strcmp(name, "Name") == 0) return ELEMENT_COUNTRY_NAME;
    if (strcmp(name, "Population") == 0) return ELEMENT_POPULATION;
    if (strcmp(name, "Capitol") == 0) return ELEMENT_CAPITOL;
    if (strcmp(name, "Language") == 0) return ELEMENT_LANGUAGE;
    if (strcmp(name, "Border") == 0) return ELEMENT_BORDER;
    if (strcmp(name, "Territory") == 0) return ELEMENT_TERRITORY;
    return ELEMENT_UNKNOWN;
}

char* pullNextElementName(ParseContext* ctx) {
    printf("Pulling next element name from context at index %d\n", ctx->current_index);
    // Dummy: For countryTopLevel, this would parse the next element like "{Name}..."
    // Simulate finding "Name" once, then "Population", then "Capitol", etc.
    static int call_count = 0;
    char* name = NULL;
    switch (call_count++) {
        case 0: name = (char*)copyData(ctx, 0, 5); strcpy(name, "Name"); break;
        case 1: name = (char*)copyData(ctx, 0, 9); strcpy(name, "Population"); break;
        case 2: name = (char*)copyData(ctx, 0, 7); strcpy(name, "Capitol"); break;
        case 3: name = (char*)copyData(ctx, 0, 8); strcpy(name, "Language"); break;
        case 4: name = (char*)copyData(ctx, 0, 6); strcpy(name, "Border"); break;
        case 5: name = (char*)copyData(ctx, 0, 9); strcpy(name, "Territory"); break;
        default: return NULL; // No more elements
    }
    // Simulate consuming some input for the element name
    ctx->current_index += (strlen(name) + 2); // e.g. {Name}
    return name;
}


// Structure definition for Country, inferred from memory offsets
#define MAX_COUNTRY_NAME_LEN 20
#define MAX_CAPITOL_NAME_LEN 20
#define MAX_LANGUAGES 10
#define MAX_BORDERS 100
#define MAX_TERRITORIES 10

typedef struct Country {
    char name[MAX_COUNTRY_NAME_LEN];          // Offset 0x0
    char capitol[MAX_CAPITOL_NAME_LEN];    // Offset 0x14
    int population;                         // Offset 0x28
    int num_languages;                      // Offset 0x2c
    char* languages[MAX_LANGUAGES];         // Offset 0x30 (array of char pointers)
    int num_borders;                        // Offset 0x58
    double* borders[MAX_BORDERS];           // Offset 0x5c (array of double pointers, each pointing to 4 doubles)
    int num_territories;                    // Offset 0x1ec
    void* territories[MAX_TERRITORIES];     // Offset 0x1f0 (array of void pointers to Territory structs)
} Country; // Total size 0x218 (536 bytes)

// Function: initCountry
void initCountry(Country* country) {
  if (country != NULL) {
    memset(country->name, 0, sizeof(country->name));
    memset(country->capitol, 0, sizeof(country->capitol));
    country->population = -1; // Default to unknown/unset
    country->num_languages = 0;
    country->num_borders = 0;
    country->num_territories = 0;
    // Clear language pointers, border pointers, and territory pointers
    memset(country->languages, 0, sizeof(country->languages));
    memset(country->borders, 0, sizeof(country->borders));
    memset(country->territories, 0, sizeof(country->territories));
  }
}

// Function: freeCountry
void freeCountry(Country* country) {
  if (country != NULL) {
    for (int i = 0; i < country->num_borders; i++) {
      if (country->borders[i] != NULL) {
        deallocate(country->borders[i], 4 * sizeof(double)); // 4 doubles per border
        country->borders[i] = NULL;
      }
    }
    for (int i = 0; i < country->num_languages; i++) {
      if (country->languages[i] != NULL) {
        deallocate(country->languages[i], strlen(country->languages[i]) + 1);
        country->languages[i] = NULL;
      }
    }
    for (int i = 0; i < MAX_TERRITORIES; i++) { // Iterate up to MAX_TERRITORIES, not num_territories, to ensure all slots are checked
      if (country->territories[i] != NULL) {
        freeTerritory(country->territories[i]);
        country->territories[i] = NULL;
      }
    }
    deallocate(country, sizeof(Country));
  }
}

// Function: printCountryInfo
void printCountryInfo(Country* country) {
  if (country != NULL) {
    printf("\tCountry: ");
    if (country->name[0] == '\0') {
      printf("Unknown\n");
    } else {
      printf("%s\n", country->name);
    }
    printf("\t\tCapitol: ");
    if (country->capitol[0] == '\0') {
      printf("Unknown\n");
    } else {
      printf("%s\n", country->capitol);
    }
    if (country->population >= 0) { // -1 indicates unset
      printf("\t\tPopulation: %d\n", country->population);
    }
    for (int i = 0; i < country->num_languages; i++) {
      if (country->languages[i] != NULL) {
        printf("\t\tLanguage: %s\n", country->languages[i]);
      }
    }
    for (int i = 0; i < country->num_borders; i++) {
      double* border_coords = country->borders[i];
      if (border_coords != NULL) {
        printf("\t\tBorder: %f %f %f %f\n", border_coords[0], border_coords[1], border_coords[2], border_coords[3]);
      }
    }
    for (int i = 0; i < MAX_TERRITORIES; i++) { // Iterate up to MAX_TERRITORIES
      if (country->territories[i] != NULL) {
        printTerritoryInfo(country->territories[i]);
      }
    }
  }
}

// Function: countryMenu
int countryMenu(Country* country) {
  char input_buffer[40]; // Sufficient for 0x28 (40) and 0x13 (19)
  int selection = 0;
  int char_idx_or_bytes_read = 0; // Reused for various char index operations or bytes_read

  if (country == NULL) {
    return 0; // Error or invalid country pointer
  }

  do {
    printf("\nCountry: %s\n", country->name);
    printf("1) Display CountryInfo\n");
    printf("2) Set Capitol\n");
    printf("3) Set Population\n");
    printf("4) Add Language\n");
    printf("5) Add Border\n");
    printf("6) Add Territory\n");
    printf("7) Select Territory\n");
    printf("8) Delete Country and Exit Menu\n");
    printf("9) Exit menu\n");
    
    memset(input_buffer, 0, sizeof(input_buffer));
    printf("Selection: ");
    receive_until(input_buffer, sizeof(input_buffer) - 1, 3);
    selection = atoi(input_buffer);

    if (selection < 1 || selection > 9) {
      printf("Invalid...\n");
      continue;
    }

    switch (selection) {
      case 1: // Display CountryInfo
        printCountryInfo(country);
        break;
      case 2: { // Set Capitol
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, sizeof(input_buffer) - 1, 0x27); // 0x27 is 39, so max 39 chars + null
        
        char_idx_or_bytes_read = 0;
        while (char_idx_or_bytes_read < MAX_CAPITOL_NAME_LEN - 1 && isalpha((unsigned char)input_buffer[char_idx_or_bytes_read])) {
          country->capitol[char_idx_or_bytes_read] = input_buffer[char_idx_or_bytes_read];
          char_idx_or_bytes_read++;
        }
        country->capitol[char_idx_or_bytes_read] = '\0';
        break;
      }
      case 3: { // Set Population
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, sizeof(input_buffer) - 1, 0x13); // 0x13 is 19
        country->population = atoi(input_buffer);
        break;
      }
      case 4: { // Add Language
        if (country->num_languages < MAX_LANGUAGES) {
          printf("\n-> ");
          memset(input_buffer, 0, sizeof(input_buffer));
          receive(0, input_buffer, sizeof(input_buffer) - 1, &char_idx_or_bytes_read); // 0x13 is 19

          size_t len = strlen(input_buffer);
          char* new_lang_ptr = NULL;
          if (allocate(len + 1, 0, (void**)&new_lang_ptr) == 0) { // 0 on success
            char_idx_or_bytes_read = 0;
            while (char_idx_or_bytes_read < len && isalpha((unsigned char)input_buffer[char_idx_or_bytes_read])) {
              new_lang_ptr[char_idx_or_bytes_read] = input_buffer[char_idx_or_bytes_read];
              char_idx_or_bytes_read++;
            }
            new_lang_ptr[char_idx_or_bytes_read] = '\0';
            country->languages[country->num_languages] = new_lang_ptr;
            country->num_languages++;
          } else {
            printf("!!Failed to allocate memory for language\n");
          }
        } else {
          printf("!!Max languages reached\n");
        }
        break;
      }
      case 5: { // Add Border
        if (country->num_borders < MAX_BORDERS) {
          double* new_border_coords = NULL;
          if (allocate(4 * sizeof(double), 0, (void**)&new_border_coords) == 0) { // 0x20 bytes for 4 doubles
            printf("Lat Start: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive(0, input_buffer, sizeof(input_buffer) - 1, &char_idx_or_bytes_read);
            new_border_coords[0] = atof(input_buffer);

            printf("Long Start: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive(0, input_buffer, sizeof(input_buffer) - 1, &char_idx_or_bytes_read);
            new_border_coords[1] = atof(input_buffer);

            printf("Lat End: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive(0, input_buffer, sizeof(input_buffer) - 1, &char_idx_or_bytes_read);
            new_border_coords[2] = atof(input_buffer);

            printf("Long End: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive(0, input_buffer, sizeof(input_buffer) - 1, &char_idx_or_bytes_read);
            new_border_coords[3] = atof(input_buffer);

            country->borders[country->num_borders] = new_border_coords;
            country->num_borders++;
          } else {
            printf("!!Failed to allocate memory for border\n");
          }
        } else {
          printf("!!Max borders reached\n");
        }
        break;
      }
      case 6: { // Add Territory
        if (country->num_territories < MAX_TERRITORIES) {
          int territory_idx = -1;
          // Find the first available slot
          for (int i = 0; i < MAX_TERRITORIES; ++i) {
            if (country->territories[i] == NULL) {
              territory_idx = i;
              break;
            }
          }

          if (territory_idx == -1) { // Should not happen if num_territories < MAX_TERRITORIES
            printf("!!Max Territories reached\n");
          } else {
            void* new_territory_ptr = NULL;
            // 0x1fc is 508 bytes, for a territory struct
            if (allocate(0x1fc, 0, &new_territory_ptr) == 0) {
              initTerritory(new_territory_ptr);
              printf("\nNew Territory: ");
              memset(input_buffer, 0, sizeof(input_buffer));
              receive_until(input_buffer, sizeof(input_buffer) - 1, 0x13);
              
              char_idx_or_bytes_read = 0;
              // Assuming territory struct starts with a name field
              char* territory_name_ptr = (char*)new_territory_ptr; // Simplified assumption
              while (isalnum((unsigned char)input_buffer[char_idx_or_bytes_read])) {
                territory_name_ptr[char_idx_or_bytes_read] = input_buffer[char_idx_or_bytes_read];
                char_idx_or_bytes_read++;
              }
              territory_name_ptr[char_idx_or_bytes_read] = '\0';
              
              country->territories[territory_idx] = new_territory_ptr;
              country->num_territories++;
            } else {
              printf("!!Failed to allocate structure\n");
              country->territories[territory_idx] = NULL; // Ensure it's null on failure
            }
          }
        } else {
          printf("!!Max Territories reached\n");
        }
        break;
      }
      case 7: { // Select Territory
        printf("\nTerritories:\n");
        int count_displayed = 0;
        for (int i = 0; i < MAX_TERRITORIES; i++) {
          if (country->territories[i] != NULL) {
            // Assuming territory struct starts with a name field for display
            printf("%d) %s\n", i + 1, (char*)country->territories[i]);
            count_displayed++;
          }
        }
        if (count_displayed == 0) {
            printf("No territories to display.\n");
            break;
        }

        memset(input_buffer, 0, sizeof(input_buffer));
        printf("\n-> ");
        receive(0, input_buffer, 3, &char_idx_or_bytes_read); // 3 chars + null, e.g., "10"
        int territory_selection = atoi(input_buffer);

        if (territory_selection < 1 || territory_selection > MAX_TERRITORIES || country->territories[territory_selection - 1] == NULL) {
          printf("Invalid choice...\n");
        } else {
          // territoryMenu returns 0 if territory was deleted, 1 on exit
          if (territoryMenu(country->territories[territory_selection - 1]) == 0) {
            country->territories[territory_selection - 1] = NULL; // Slot is now free
            country->num_territories--;
          }
        }
        break;
      }
      case 8: // Delete Country and Exit Menu
        freeCountry(country);
        return 0; // Indicate deletion and exit
    }
  } while (selection != 9);

  return 1; // Indicate successful exit (not deleted)
}

// Function: countryTopLevel
Country* countryTopLevel(ParseContext* parse_ctx) {
  Country* new_country = NULL;
  char* element_name = NULL;
  char* temp_str_data = NULL;
  int initial_context_idx = parse_ctx->current_index; // To revert on error
  int current_idx, end_idx;
  int cmp_result;
  
  if (allocate(sizeof(Country), 0, (void**)&new_country) != 0) { // Allocation failed
    return NULL;
  }

  initCountry(new_country);
  
  int parsing_success = 0; // 0 for failure, 1 for success

  do { // This loop replaces the goto for error handling
    skipWhiteSpace(parse_ctx);
    getIndex(parse_ctx, &current_idx);
    
    // Expect '{'
    if (!atChar(parse_ctx, '{')) {
        printf("!!Country: Expected opening brace\n");
        break; // Exit do-while on failure
    }
    skipLength(parse_ctx, 1);

    skipWhiteSpace(parse_ctx);
    current_idx = parse_ctx->current_index;
    end_idx = skipAlpha(parse_ctx);

    if (end_idx == -1 || current_idx == end_idx) {
        printf("!!Country: Failed to find element name\n");
        break;
    }
    temp_str_data = (char*)copyData(parse_ctx, current_idx, end_idx);
    if (temp_str_data == NULL) {
        printf("!!Country: Failed to copy element name\n");
        break;
    }

    cmp_result = strcmp(temp_str_data, "Country");
    deallocate(temp_str_data, strlen(temp_str_data) + 1); // Free temp_str_data immediately
    temp_str_data = NULL;

    if (cmp_result != 0) {
      printf("!!Country: Invalid opening element id\n");
      break;
    }

    skipWhiteSpace(parse_ctx);
    // Expect '}' after "Country" element id
    if (!atChar(parse_ctx, '}')) {
        printf("!!Country: Expected closing brace after 'Country' id\n");
        break;
    }
    incChar(parse_ctx);

    // Loop through country elements
    element_name = pullNextElementName(parse_ctx);
    while (element_name != NULL) {
      ElementType element_type = elementNameToEnum(element_name);
      deallocate(element_name, strlen(element_name) + 1); // Free element_name after use
      element_name = NULL; // Clear for next iteration

      parsing_success = 1; // Assume success for this element, set to 0 on failure
      switch (element_type) {
        case ELEMENT_COUNTRY_NAME:
          temp_str_data = extractName(parse_ctx);
          if (temp_str_data == NULL) { parsing_success = 0; break; }
          memset(new_country->name, 0, sizeof(new_country->name));
          strncpy(new_country->name, temp_str_data, sizeof(new_country->name) - 1);
          deallocate(temp_str_data, strlen(temp_str_data) + 1);
          temp_str_data = NULL;
          break;
        case ELEMENT_POPULATION:
          new_country->population = extractPopulation(parse_ctx);
          if (new_country->population < 0) { parsing_success = 0; break; }
          break;
        case ELEMENT_CAPITOL:
          temp_str_data = extractCapitol(parse_ctx);
          if (temp_str_data == NULL) { parsing_success = 0; break; }
          memset(new_country->capitol, 0, sizeof(new_country->capitol));
          strncpy(new_country->capitol, temp_str_data, sizeof(new_country->capitol) - 1);
          deallocate(temp_str_data, strlen(temp_str_data) + 1);
          temp_str_data = NULL;
          break;
        case ELEMENT_LANGUAGE:
          if (new_country->num_languages >= MAX_LANGUAGES) {
            printf("!!Max country language count is %d\n", MAX_LANGUAGES);
            parsing_success = 0; break;
          }
          new_country->languages[new_country->num_languages] = extractLanguage(parse_ctx);
          if (new_country->languages[new_country->num_languages] == NULL) { parsing_success = 0; break; }
          new_country->num_languages++;
          break;
        case ELEMENT_BORDER:
          if (new_country->num_borders >= MAX_BORDERS) {
            printf("!!Max country border count is %d\n", MAX_BORDERS);
            parsing_success = 0; break;
          }
          new_country->borders[new_country->num_borders] = extractBorder(parse_ctx);
          if (new_country->borders[new_country->num_borders] == NULL) { parsing_success = 0; break; }
          new_country->num_borders++;
          break;
        case ELEMENT_TERRITORY:
          if (new_country->num_territories >= MAX_TERRITORIES) {
            printf("!!Max territories is %d\n", MAX_TERRITORIES);
            parsing_success = 0; break;
          }
          new_country->territories[new_country->num_territories] = territoryTopLevel(parse_ctx);
          if (new_country->territories[new_country->num_territories] == NULL) { parsing_success = 0; break; }
          new_country->num_territories++;
          break;
        default:
          printf("Invalid element for country\n");
          parsing_success = 0; break;
      }
      if (!parsing_success) { // If any element parsing failed
          break; // Exit while (element_name != NULL) loop
      }
      element_name = pullNextElementName(parse_ctx);
    }

    if (!parsing_success) { // If break was from parsing_success = 0
        if (element_name != NULL) {
            deallocate(element_name, strlen(element_name) + 1); // Free remaining element_name
        }
        break; // Exit do-while if parsing failed in the loop
    }

    // After parsing all elements, expect closing #Country}
    skipWhiteSpace(parse_ctx);
    if (!atChar(parse_ctx, '{')) { printf("!!Country: Expected final opening brace\n"); break; }
    skipLength(parse_ctx, 1);
    skipWhiteSpace(parse_ctx);
    if (!atChar(parse_ctx, '#')) { printf("!!Country: Expected closing mark '#'\n"); break; }
    skipLength(parse_ctx, 1);

    current_idx = parse_ctx->current_index;
    end_idx = skipAlpha(parse_ctx);
    if (end_idx == -1 || current_idx == end_idx) {
        printf("!!Country: Failed to find closing element name\n");
        break;
    }
    temp_str_data = (char*)copyData(parse_ctx, current_idx, end_idx);
    if (temp_str_data == NULL) {
        printf("!!Country: Failed to copy closing element name\n");
        break;
    }

    cmp_result = strcmp(temp_str_data, "Country");
    deallocate(temp_str_data, strlen(temp_str_data) + 1);
    temp_str_data = NULL;

    if (cmp_result != 0) {
      printf("!!Country: Invalid closing element id\n");
      break;
    }

    skipWhiteSpace(parse_ctx);
    if (!atChar(parse_ctx, '}')) {
        printf("!!Country: Expected final closing brace\n");
        break;
    }
    incChar(parse_ctx);
    parsing_success = 1; // All checks passed
  } while (0); // Execute once, use break for early exit on error

  if (!parsing_success) {
    if (new_country != NULL) {
      freeCountry(new_country);
      new_country = NULL;
    }
    parse_ctx->current_index = initial_context_idx; // Revert context on error
    printf("Error at: %s\n", parse_ctx->buffer + parse_ctx->current_index);
  }

  return new_country;
}

// Function: extractLanguage
char* extractLanguage(ParseContext* ctx) {
  char* extracted_language = NULL;
  char* temp_element_id = NULL;
  int initial_context_idx = ctx->current_index;
  int current_idx, end_idx;
  int success = 0; // 0 for failure, 1 for success

  do {
    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) { printf("!!Failed to locate opening brace\n"); break; }
    if (skipLength(ctx, 1) == -1) { printf("!!Failed to skip opening brace\n"); break; }

    skipWhiteSpace(ctx);
    current_idx = ctx->current_index;
    end_idx = skipToNonAlphaNum(ctx);
    if (end_idx == -1) { printf("!!Failed to locate the end of the element id\n"); break; }

    temp_element_id = (char*)copyData(ctx, current_idx, end_idx);
    if (temp_element_id == NULL) { printf("!!Copy from %d to %d failed\n", current_idx, end_idx); break; }
    
    if (strcmp(temp_element_id, "Language") != 0) {
      printf("!!Element id is not \"Language\"\n");
      break;
    }
    deallocate(temp_element_id, strlen(temp_element_id) + 1); temp_element_id = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate initial closing brace\n"); break; }
    if (skipLength(ctx, 1) == -1) { printf("!!Failed to skip initial closing brace\n"); break; }

    current_idx = skipWhiteSpace(ctx);
    end_idx = skipAlpha(ctx);
    if (current_idx == end_idx) { printf("!!Failed to find language data\n"); break; }

    extracted_language = (char*)copyData(ctx, current_idx, end_idx);
    if (extracted_language == NULL) { printf("!!Failed to copy language data\n"); break; }

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) { printf("!!Failed to locate the final opening brace\n"); break; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip the final opening brace\n"); break; }

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '#')) { printf("!!Failed to locate the closing mark\n"); break; }
    current_idx = skipLength(ctx, 1);
    if (current_idx == -1) { printf("!!Failed to skip closing mark\n"); break; }

    end_idx = skipToNonAlphaNum(ctx);
    if (end_idx == -1) { printf("!!Failed to locate the end of the closing element id\n"); break; }

    temp_element_id = (char*)copyData(ctx, current_idx, end_idx);
    if (strcmp(temp_element_id, "Language") != 0) {
      printf("!!Invalid closing element id: %s\n", temp_element_id);
      break;
    }
    deallocate(temp_element_id, strlen(temp_element_id) + 1); temp_element_id = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate final closing brace\n"); break; }
    skipLength(ctx, 1);
    success = 1;
  } while(0);

  if (!success) {
    if (extracted_language != NULL) {
      deallocate(extracted_language, strlen(extracted_language) + 1);
      extracted_language = NULL;
    }
    if (temp_element_id != NULL) { // In case it was allocated but failed validation
      deallocate(temp_element_id, strlen(temp_element_id) + 1);
    }
    ctx->current_index = initial_context_idx; // Revert context on error
  }
  return extracted_language;
}

// Function: extractCapitol
char* extractCapitol(ParseContext* ctx) {
  char* extracted_capitol = NULL;
  char* temp_element_id = NULL;
  int initial_context_idx = ctx->current_index;
  int current_idx, end_idx;
  int success = 0; // 0 for failure, 1 for success

  do {
    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) { printf("!!Failed to locate opening brace\n"); break; }
    if (skipLength(ctx, 1) == -1) { printf("!!Failed to skip opening brace\n"); break; }

    current_idx = skipWhiteSpace(ctx);
    end_idx = skipToNonAlphaNum(ctx);
    if (end_idx == -1) { printf("!!Failed to locate the end of the element id\n"); break; }

    temp_element_id = (char*)copyData(ctx, current_idx, end_idx);
    if (temp_element_id == NULL) { printf("!!Failed to copy element id\n"); break; }

    if (strcmp(temp_element_id, "Capitol") != 0) {
      printf("!!Element id is not \"Capitol\"\n");
      break;
    }
    deallocate(temp_element_id, strlen(temp_element_id) + 1); temp_element_id = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate initial closing brace\n"); break; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip initial closing brace\n"); break; }

    getIndex(ctx, &current_idx);
    end_idx = skipAlpha(ctx);
    if (current_idx == end_idx) { printf("!!Failed to find capitol data\n"); break; }

    extracted_capitol = (char*)copyData(ctx, current_idx, end_idx);
    if (extracted_capitol == NULL) { printf("!!Failed to copy capitol data\n"); break; }

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) { printf("!!Failed to locate the final opening brace\n"); break; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip the final opening brace\n"); break; }

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '#')) { printf("!!Failed to locate the closing mark\n"); break; }
    current_idx = skipLength(ctx, 1);
    if (current_idx == -1) { printf("!!Failed to skip closing mark\n"); break; }

    end_idx = skipToNonAlphaNum(ctx);
    if (end_idx == -1) { printf("!!Failed to locate the end of the closing element id\n"); break; }

    temp_element_id = (char*)copyData(ctx, current_idx, end_idx);
    if (strcmp(temp_element_id, "Capitol") != 0) {
      printf("!!Invalid closing element id: %s\n", temp_element_id);
      break;
    }
    deallocate(temp_element_id, strlen(temp_element_id) + 1); temp_element_id = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate final closing brace\n"); break; }
    skipLength(ctx, 1);
    success = 1;
  } while(0);

  if (!success) {
    if (extracted_capitol != NULL) {
      deallocate(extracted_capitol, strlen(extracted_capitol) + 1);
      extracted_capitol = NULL;
    }
    if (temp_element_id != NULL) { // In case it was allocated but failed validation
      deallocate(temp_element_id, strlen(temp_element_id) + 1);
    }
    ctx->current_index = initial_context_idx; // Revert context on error
  }
  return extracted_capitol;
}