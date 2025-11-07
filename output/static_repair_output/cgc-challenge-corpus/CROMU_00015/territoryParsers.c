#include <stdio.h>   // For printf, fgets, stdin
#include <stdlib.h>  // For atoi, atof, malloc, free, calloc
#include <string.h>  // For memset, strlen, strncpy, strcmp, strcspn
#include <ctype.h>   // For isalnum, isspace, isalpha, isdigit

// --- Forward Declarations for custom types and functions ---

// Assuming a 32-bit architecture for pointer sizes (4 bytes) based on original offsets and sizes.

#define MAX_NAME_LEN 20         // 0x14 - 1 for null terminator
#define MAX_FOUNDER_LEN 30      // 0x1e - 1 for null terminator
#define MAX_BORDERS 100
#define MAX_COUNTIES 10
#define BORDER_COORD_COUNT 4    // Lat Start, Long Start, Lat End, Long End

// Dummy County struct (just enough to make pointer arithmetic and usage plausible)
typedef struct {
    char name[MAX_NAME_LEN]; // Assuming County name is at the start of the County object
    // Add other county members as needed for full functionality
} County;

// Territory struct definition derived from offsets in the original code
typedef struct {
    char name[MAX_NAME_LEN];             // 0x00 - 0x13 (20 bytes total for name)
    char founder[MAX_FOUNDER_LEN];       // 0x14 - 0x31 (30 bytes total for founder)
    int population;                      // 0x34
    int established;                     // 0x38
    int border_count;                    // 0x3c
    double *borders[MAX_BORDERS];        // 0x40 - 0x1bc (100 * 4 bytes for double* on 32-bit system)
    int county_count;                    // 0x1d0
    County *counties[MAX_COUNTIES];      // 0x1d4 - 0x1fb (10 * 4 bytes for County* on 32-bit system)
} Territory; // Total size: 0x1fc (508 bytes)

// Dummy ParserContext struct to represent the parsing state
typedef struct {
    char *buffer;
    int current_offset;
    int buffer_len; // Length of the buffer string
} ParserContext;

// --- Dummy Implementations for external functions to ensure compilability ---

// Custom memory allocation function
void *allocate(size_t size, int param_2, void **out_ptr) {
    *out_ptr = calloc(1, size); // Allocate and zero-initialize
    return (*out_ptr == NULL) ? (void*)1 : (void*)0; // Return 0 on success, 1 on failure
}

// Custom memory deallocation function
void deallocate(void *ptr, size_t size) {
    if (ptr) {
        free(ptr);
    }
    // size parameter is ignored by standard free, but kept for signature match
}

// Custom input receiving function
int receive_until(char *buffer, int max_len, int unknown_param) {
    if (fgets(buffer, max_len + 1, stdin) != NULL) { // max_len is for content, +1 for null
        buffer[strcspn(buffer, "\n")] = 0; // Remove trailing newline character
        return 0; // Success
    }
    return 1; // Failure
}

// Forward declarations of functions in the snippet
void printTerritoryInfo(Territory *territory_ptr);
void initTerritory(Territory *territory_ptr);
void freeTerritory(Territory *territory_ptr);
Territory *territoryTopLevel(ParserContext *parser_ctx);
char *extractFounder(ParserContext *parser_ctx);
int extractEstablished(ParserContext *parser_ctx);

// Dummy initCounty function
void initCounty(County *county_ptr) {
    if (county_ptr) {
        memset(county_ptr, 0, sizeof(County));
    }
}

// Dummy countyMenu function (returns 1 to exit, 0 to delete county)
int countyMenu(County *county_ptr) {
    if (county_ptr) {
        printf("Entering menu for County: %s\n", county_ptr->name);
    }
    printf("1) Exit County Menu\n");
    printf("0) Delete County and Exit Menu\n");
    printf("Selection: ");
    char selection_buf[5];
    receive_until(selection_buf, 4, 0);
    int selection = atoi(selection_buf);
    return selection; // Return 0 for delete, 1 for exit (or other for continue)
}

// Dummy freeCounty function
void freeCounty(County *county_ptr) {
    if (county_ptr) {
        // Free any internal allocations of County here if needed
        deallocate(county_ptr, sizeof(County));
    }
}

// Dummy printCountyInfo function
void printCountyInfo(County* county_ptr) {
    if (county_ptr) {
        printf("\t\t\tCounty: %s (Placeholder Info)\n", county_ptr->name);
    } else {
        printf("\t\t\tCounty: (Null)\n");
    }
}


// --- Dummy Parser Helper Functions ---
int skipWhiteSpace(ParserContext *ctx) {
    if (!ctx || !ctx->buffer) return -1;
    int original_offset = ctx->current_offset;
    while (ctx->current_offset < ctx->buffer_len && isspace(ctx->buffer[ctx->current_offset])) {
        ctx->current_offset++;
    }
    return original_offset; // Return original offset, or new offset based on common practice
}

int getIndex(ParserContext *ctx, int *out_index) {
    if (!ctx || !ctx->buffer || !out_index) return -1;
    *out_index = ctx->current_offset;
    return ctx->current_offset;
}

int atChar(ParserContext *ctx, char c) {
    if (!ctx || !ctx->buffer || ctx->current_offset >= ctx->buffer_len) return 0;
    return ctx->buffer[ctx->current_offset] == c;
}

int incChar(ParserContext *ctx) {
    if (!ctx || !ctx->buffer || ctx->current_offset >= ctx->buffer_len) return -1;
    ctx->current_offset++;
    return 0; // Success
}

int skipLength(ParserContext *ctx, int len) {
    if (!ctx || !ctx->buffer || ctx->current_offset + len > ctx->buffer_len) return -1;
    ctx->current_offset += len;
    return 0; // Success
}

int skipAlpha(ParserContext *ctx) {
    if (!ctx || !ctx->buffer) return -1;
    int start = ctx->current_offset;
    while (ctx->current_offset < ctx->buffer_len && isalpha(ctx->buffer[ctx->current_offset])) {
        ctx->current_offset++;
    }
    return ctx->current_offset; // Returns the end index
}

int skipInt(ParserContext *ctx) {
    if (!ctx || !ctx->buffer) return -1;
    int start = ctx->current_offset;
    while (ctx->current_offset < ctx->buffer_len && isdigit(ctx->buffer[ctx->current_offset])) {
        ctx->current_offset++;
    }
    return ctx->current_offset; // Returns the end index
}

int skipToNonAlphaNum(ParserContext *ctx) {
    if (!ctx || !ctx->buffer) return -1;
    int start = ctx->current_offset;
    while (ctx->current_offset < ctx->buffer_len && isalnum(ctx->buffer[ctx->current_offset])) {
        ctx->current_offset++;
    }
    return ctx->current_offset; // Returns the end index
}

int skipToNonAlphaNumSpace(ParserContext *ctx) {
    if (!ctx || !ctx->buffer) return -1;
    int start = ctx->current_offset;
    while (ctx->current_offset < ctx->buffer_len && (isalnum(ctx->buffer[ctx->current_offset]) || isspace(ctx->buffer[ctx->current_offset]))) {
        ctx->current_offset++;
    }
    return ctx->current_offset; // Returns the end index
}

char *copyData(ParserContext *ctx, int start, int end) {
    if (!ctx || !ctx->buffer || start < 0 || end > ctx->buffer_len || start >= end) return NULL;
    size_t len = end - start;
    char *data = (char *)malloc(len + 1);
    if (data) {
        strncpy(data, ctx->buffer + start, len);
        data[len] = '\0';
    }
    return data;
}

char *pullNextElementName(ParserContext *ctx) {
    // This is a simplified dummy. In a real parser, it would extract the next element ID.
    // For this example, it cycles through some common elements.
    static int call_count = 0;
    static const char *elements[] = {"Name", "Population", "Established", "Founder", "Border", "County"};
    static const int num_elements = sizeof(elements) / sizeof(elements[0]);

    skipWhiteSpace(ctx);
    if (ctx->current_offset >= ctx->buffer_len || ctx->buffer[ctx->current_offset] == '{') {
        // If we hit a new block or end of buffer, stop
        call_count = 0; // Reset for next territory if any
        return NULL;
    }

    if (call_count < num_elements) {
        return strdup(elements[call_count++]);
    }
    call_count = 0;
    return NULL;
}

int elementNameToEnum(const char *name) {
    if (strcmp(name, "Name") == 0) return 1;
    if (strcmp(name, "Population") == 0) return 10;
    if (strcmp(name, "Border") == 0) return 14;
    if (strcmp(name, "Established") == 0) return 16;
    if (strcmp(name, "Founder") == 0) return 17;
    if (strcmp(name, "County") == 0) return 18;
    return 0; // Default/Invalid
}

char *extractName(ParserContext *ctx) {
    // Dummy implementation: returns a fixed name
    skipWhiteSpace(ctx);
    return strdup("{Name}TestTerritory{#Name}"); // Placeholder for parsing
}

int extractPopulation(ParserContext *ctx) {
    // Dummy implementation: returns a fixed population
    skipWhiteSpace(ctx);
    return 10000; // Placeholder for parsing
}

double *extractBorder(ParserContext *ctx) {
    // Dummy implementation: returns a fixed border coordinate set
    skipWhiteSpace(ctx);
    double *border_data = (double*)calloc(BORDER_COORD_COUNT, sizeof(double));
    if (border_data) {
        border_data[0] = 10.1; border_data[1] = 20.2;
        border_data[2] = 30.3; border_data[3] = 40.4;
    }
    return border_data;
}

County *countyTopLevel(ParserContext *ctx) {
    // Dummy implementation: allocates a new County and sets a dummy name
    skipWhiteSpace(ctx);
    County *new_county = NULL;
    if (allocate(sizeof(County), 0, (void**)&new_county) == 0) {
        initCounty(new_county);
        strncpy(new_county->name, "TestCounty", sizeof(new_county->name) - 1);
        new_county->name[sizeof(new_county->name) - 1] = '\0';
    }
    return new_county;
}

// --- Fixed and Refactored Functions ---

// Function: territoryMenu
int territoryMenu(Territory *territory_ptr) {
  double *border_coords_ptr = NULL;
  int selected_option = 0;
  char input_buffer[MAX_FOUNDER_LEN + 1]; // Max length for founder name + null
  int loop_status = 3; // 0 for delete+exit, 1 for exit, 3 for continue

  if (territory_ptr == NULL) {
    return 0;
  }

  do {
    printf("\nTerritory: %s\n", territory_ptr->name);
    printf("1) Display Territory Info\n");
    printf("2) Set Founder\n");
    printf("3) Set Population\n");
    printf("4) Set Established\n");
    printf("5) Add Border\n");
    printf("6) Add County\n");
    printf("7) Select County\n");
    printf("8) Delete Territory and Exit Menu\n");
    printf("9) Exit Menu\n");

    memset(input_buffer, 0, sizeof(input_buffer));
    printf("Selection: ");
    receive_until(input_buffer, sizeof(input_buffer) - 1, 3);
    selected_option = atoi(input_buffer);

    switch (selected_option) {
    case 1:
      printTerritoryInfo(territory_ptr);
      break;
    case 2: {
      printf("\n-> ");
      memset(input_buffer, 0, sizeof(input_buffer));
      // Max founder length is MAX_FOUNDER_LEN - 1 for input
      receive_until(input_buffer, MAX_FOUNDER_LEN - 1, 0x1d);
      int i = 0;
      while (i < MAX_FOUNDER_LEN - 1 && (isalnum((int)input_buffer[i]) || input_buffer[i] == ' ')) {
        territory_ptr->founder[i] = input_buffer[i];
        i++;
      }
      territory_ptr->founder[i] = '\0';
      break;
    }
    case 3: {
      printf("\n-> ");
      memset(input_buffer, 0, sizeof(input_buffer));
      receive_until(input_buffer, sizeof(input_buffer) - 1, 0x1d);
      territory_ptr->population = atoi(input_buffer);
      break;
    }
    case 4: {
      printf("\n-> ");
      memset(input_buffer, 0, sizeof(input_buffer));
      receive_until(input_buffer, sizeof(input_buffer) - 1, 0x1d);
      territory_ptr->established = atoi(input_buffer);
      break;
    }
    case 5: {
      if (territory_ptr->border_count < MAX_BORDERS) {
        if (allocate(sizeof(double) * BORDER_COORD_COUNT, 0, (void**)&border_coords_ptr) == 0) { // Success
          printf("Lat Start: ");
          memset(input_buffer, 0, sizeof(input_buffer));
          receive_until(input_buffer, 0x13, 0x13); // Max 19 chars + null
          border_coords_ptr[0] = atof(input_buffer);

          printf("Long Start: ");
          memset(input_buffer, 0, sizeof(input_buffer));
          receive_until(input_buffer, 0x13, 0x13);
          border_coords_ptr[1] = atof(input_buffer);

          printf("Lat End: ");
          memset(input_buffer, 0, sizeof(input_buffer));
          receive_until(input_buffer, 0x13, 0x13);
          border_coords_ptr[2] = atof(input_buffer);

          printf("Long End: ");
          memset(input_buffer, 0, sizeof(input_buffer));
          receive_until(input_buffer, 0x13, 0x13);
          border_coords_ptr[3] = atof(input_buffer);

          territory_ptr->borders[territory_ptr->border_count] = border_coords_ptr;
          territory_ptr->border_count++;
        } else {
          border_coords_ptr = NULL; // Allocation failed
        }
      } else {
        printf("!!Max borders reached\n");
      }
      break;
    }
    case 6: {
      int county_idx = 0;
      while (county_idx < MAX_COUNTIES && territory_ptr->counties[county_idx] != NULL) {
        county_idx++;
      }

      if (county_idx == MAX_COUNTIES) {
        printf("!!Max counties reached\n");
      } else {
        printf("\nNew County Name: ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, MAX_NAME_LEN - 1, 0x13); // Max 19 chars + null

        County *new_county = NULL;
        if (allocate(sizeof(County), 0, (void**)&new_county) == 0) { // Success
          initCounty(new_county);
          territory_ptr->counties[county_idx] = new_county;
          int i = 0;
          while (i < MAX_NAME_LEN - 1 && isalnum((int)input_buffer[i])) {
            new_county->name[i] = input_buffer[i];
            i++;
          }
          new_county->name[i] = '\0';
          territory_ptr->county_count++;
        } else {
          new_county = NULL; // Allocation failed
        }
      }
      break;
    }
    case 7: {
      int i;
      for (i = 0; i < MAX_COUNTIES; i++) {
        if (territory_ptr->counties[i] != NULL) {
          printf("%d) %s\n", i + 1, territory_ptr->counties[i]->name);
        }
      }
      printf("\n-> ");
      memset(input_buffer, 0, sizeof(input_buffer));
      receive_until(input_buffer, 0x13, 0x13);
      int selected_county_idx = atoi(input_buffer) - 1; // Convert 1-based to 0-based index

      if (selected_county_idx < 0 || selected_county_idx >= MAX_COUNTIES) {
        printf("Invalid choice...\n");
      } else if (territory_ptr->counties[selected_county_idx] == NULL) {
        printf("Invalid choice...\n");
      } else {
        if (countyMenu(territory_ptr->counties[selected_county_idx]) == 0) { // If countyMenu returns 0 (delete county)
          freeCounty(territory_ptr->counties[selected_county_idx]);
          territory_ptr->counties[selected_county_idx] = NULL;
          territory_ptr->county_count--;
        }
      }
      break;
    }
    case 8:
      freeTerritory(territory_ptr);
      loop_status = 0; // Exit and free territory
      break;
    case 9:
      loop_status = 1; // Exit menu
      break;
    default:
      printf("Invalid\n");
      break;
    }
  } while (loop_status == 3); // Continue as long as not 8 or 9

  return loop_status;
}

// Function: printTerritoryInfo
void printTerritoryInfo(Territory *territory_ptr) {
  if (territory_ptr != NULL) {
    printf("\t\tTerritory: ");
    if (territory_ptr->name[0] == '\0') {
      printf("Unknown\n");
    } else {
      printf("%s\n", territory_ptr->name);
    }
    if (territory_ptr->population > -1) { // -1 indicates unknown
      printf("\t\t\tPopulation: %d\n", territory_ptr->population);
    }
    printf("\t\t\tEstablished: %d\n", territory_ptr->established);
    printf("\t\t\tFounder: ");
    if (territory_ptr->founder[0] == '\0') {
      printf("Unknown\n");
    } else {
      printf("%s\n", territory_ptr->founder);
    }

    for (int i = 0; i < territory_ptr->border_count; i++) {
      double *border_coords = territory_ptr->borders[i];
      if (border_coords != NULL) {
        printf("\t\t\tBorder: %f %f %f %f\n", border_coords[0], border_coords[1], border_coords[2], border_coords[3]);
      }
    }

    for (int i = 0; i < MAX_COUNTIES; i++) { // Iterate all possible county slots
      if (territory_ptr->counties[i] != NULL) {
        printCountyInfo(territory_ptr->counties[i]); // Call the (dummy) printCountyInfo
      }
    }
  }
  return;
}

// Function: freeTerritory
void freeTerritory(Territory *territory_ptr) {
  if (territory_ptr != NULL) {
    for (int i = 0; i < territory_ptr->border_count; i++) {
      if (territory_ptr->borders[i] != NULL) {
        deallocate(territory_ptr->borders[i], sizeof(double) * BORDER_COORD_COUNT);
        territory_ptr->borders[i] = NULL;
      }
    }
    // Iterate through all possible county slots, not just up to county_count,
    // as county_count might not reflect non-contiguous deletions.
    for (int i = 0; i < MAX_COUNTIES; i++) {
      if (territory_ptr->counties[i] != NULL) {
        freeCounty(territory_ptr->counties[i]); // This will deallocate the County object itself
        territory_ptr->counties[i] = NULL;
      }
    }
    deallocate(territory_ptr, sizeof(Territory));
  }
  return;
}

// Function: initTerritory
void initTerritory(Territory *territory_ptr) {
  if (territory_ptr != NULL) {
    memset(territory_ptr->name, 0, sizeof(territory_ptr->name));
    memset(territory_ptr->founder, 0, sizeof(territory_ptr->founder));
    territory_ptr->population = -1; // Corresponds to 0xffffffff
    territory_ptr->established = 0;
    territory_ptr->county_count = 0;
    territory_ptr->border_count = 0;
    memset(territory_ptr->borders, 0, sizeof(territory_ptr->borders));
    memset(territory_ptr->counties, 0, sizeof(territory_ptr->counties));
  }
  return;
}

// Function: territoryTopLevel
Territory *territoryTopLevel(ParserContext *parser_ctx) {
  Territory *new_territory = NULL;
  char *element_name = NULL;
  char *temp_data_str = NULL;
  int success = 0; // 0 for failure, 1 for success

  if (parser_ctx == NULL) {
    return NULL;
  }

  if (allocate(sizeof(Territory), 0, (void**)&new_territory) != 0) {
    printf("!!Failed to allocate memory for territory\n");
    return NULL;
  }
  initTerritory(new_territory);

  // Parse opening '{Territory}'
  if (skipWhiteSpace(parser_ctx) == -1 || !atChar(parser_ctx, '{') || incChar(parser_ctx) == -1) {
      printf("!!Territory: Failed to locate or skip opening brace\n");
  } else {
    skipWhiteSpace(parser_ctx);
    int start_idx, end_idx;
    getIndex(parser_ctx, &start_idx);
    end_idx = skipAlpha(parser_ctx);

    if (end_idx == -1 || end_idx == start_idx) {
        printf("!!Territory: Failed to locate the element id\n");
    } else {
      element_name = copyData(parser_ctx, start_idx, end_idx);
      if (element_name == NULL) {
          printf("!!Territory: Failed to copy element id\n");
      } else {
        if (strcmp(element_name, "Territory") != 0) {
            printf("!!Territory: Invalid opening element id: %s\n", element_name);
        } else {
          deallocate(element_name, strlen(element_name) + 1);
          element_name = NULL; // Clear, as it's no longer needed

          skipWhiteSpace(parser_ctx);
          if (!atChar(parser_ctx, '}') || incChar(parser_ctx) == -1) {
              printf("!!Territory: Failed to locate or skip initial closing brace\n");
          } else {
            // Main loop for parsing elements within the Territory block
            int element_parse_success = 1;
            while (element_parse_success && (element_name = pullNextElementName(parser_ctx)) != NULL) {
                int element_type = elementNameToEnum(element_name);
                deallocate(element_name, strlen(element_name) + 1);
                element_name = NULL;

                switch (element_type) {
                case 1: // Name
                    temp_data_str = extractName(parser_ctx);
                    if (temp_data_str == NULL) {
                        element_parse_success = 0;
                        printf("!!Territory: Failed to extract Name\n");
                        break;
                    }
                    strncpy(new_territory->name, temp_data_str, sizeof(new_territory->name) - 1);
                    new_territory->name[sizeof(new_territory->name) - 1] = '\0';
                    deallocate(temp_data_str, strlen(temp_data_str) + 1);
                    temp_data_str = NULL;
                    break;
                case 10: // Population
                    new_territory->population = extractPopulation(parser_ctx);
                    if (new_territory->population < 0) {
                        element_parse_success = 0;
                        printf("!!Territory: Invalid population value\n");
                        break;
                    }
                    break;
                case 14: { // Border
                    if (new_territory->border_count >= MAX_BORDERS) {
                        printf("!!Max territory border count is %d\n", MAX_BORDERS);
                        element_parse_success = 0;
                        break;
                    }
                    double *border_data = extractBorder(parser_ctx);
                    if (border_data == NULL) {
                        element_parse_success = 0;
                        printf("!!Territory: Failed to extract Border\n");
                        break;
                    }
                    new_territory->borders[new_territory->border_count] = border_data;
                    new_territory->border_count++;
                    break;
                }
                case 16: // Established
                    new_territory->established = extractEstablished(parser_ctx);
                    // extractEstablished returns 0 on error, which is a valid value for established.
                    // More robust error handling might be needed if 0 is not allowed.
                    break;
                case 17: // Founder
                    temp_data_str = extractFounder(parser_ctx);
                    if (temp_data_str == NULL) {
                        element_parse_success = 0;
                        printf("!!Territory: Failed to extract Founder\n");
                        break;
                    }
                    strncpy(new_territory->founder, temp_data_str, sizeof(new_territory->founder) - 1);
                    new_territory->founder[sizeof(new_territory->founder) - 1] = '\0';
                    deallocate(temp_data_str, strlen(temp_data_str) + 1);
                    temp_data_str = NULL;
                    break;
                case 18: { // County
                    if (new_territory->county_count >= MAX_COUNTIES) {
                        printf("!!Max territory county count is %d\n", MAX_COUNTIES);
                        element_parse_success = 0;
                        break;
                    }
                    County *new_county = countyTopLevel(parser_ctx);
                    if (new_county == NULL) {
                        element_parse_success = 0;
                        printf("!!Territory: Failed to extract County\n");
                        break;
                    }
                    new_territory->counties[new_territory->county_count] = new_county;
                    new_territory->county_count++;
                    break;
                }
                default:
                    printf("Invalid element for territory: %s\n", element_name ? element_name : "UNKNOWN");
                    element_parse_success = 0;
                    break;
                }
            } // End of element parsing loop

            if (element_parse_success) {
                // After parsing all elements, check for closing structure '{#Territory}'
                skipWhiteSpace(parser_ctx);
                if (!atChar(parser_ctx, '{') || skipLength(parser_ctx, 1) == -1) {
                    printf("!!Territory: Failed to locate or skip final opening brace\n");
                } else {
                    skipWhiteSpace(parser_ctx);
                    if (!atChar(parser_ctx, '#') || skipLength(parser_ctx, 1) == -1) {
                        printf("!!Territory: Failed to locate or skip closing mark '#'\n");
                    } else {
                        getIndex(parser_ctx, &start_idx);
                        end_idx = skipAlpha(parser_ctx);

                        if (end_idx == -1 || end_idx == start_idx) {
                            printf("!!Territory: Failed to locate the end of the closing element id\n");
                        } else {
                            element_name = copyData(parser_ctx, start_idx, end_idx);
                            if (element_name == NULL) {
                                printf("!!Territory: Failed to copy closing element id\n");
                            } else {
                                if (strcmp(element_name, "Territory") != 0) {
                                    printf("!!Territory: Invalid closing element id: %s\n", element_name);
                                } else {
                                    deallocate(element_name, strlen(element_name) + 1);
                                    element_name = NULL;

                                    skipWhiteSpace(parser_ctx);
                                    if (atChar(parser_ctx, '}') && incChar(parser_ctx) != -1) {
                                        success = 1; // All checks passed
                                    } else {
                                        printf("!!Territory: Failed to locate final closing brace\n");
                                    }
                                }
                            }
                        }
                    }
                }
            }
          }
        }
      }
    }
  }

  if (element_name != NULL) {
      deallocate(element_name, strlen(element_name) + 1);
  }
  if (temp_data_str != NULL) {
      deallocate(temp_data_str, strlen(temp_data_str) + 1);
  }

  if (!success && new_territory != NULL) {
    freeTerritory(new_territory);
    new_territory = NULL;
    printf("Error at: %s\n", parser_ctx->buffer + parser_ctx->current_offset);
  }

  return new_territory;
}

// Function: extractFounder
char *extractFounder(ParserContext *parser_ctx) {
  char *founder_name = NULL;
  char *element_id = NULL;
  int success = 0;

  if (parser_ctx == NULL) {
    return NULL;
  }

  int start_idx, end_idx;
  int content_start_idx, content_end_idx;

  if (skipWhiteSpace(parser_ctx) == -1 || !atChar(parser_ctx, '{') || skipLength(parser_ctx, 1) == -1) {
    printf("!!Failed to locate or skip opening brace\n");
  } else {
    skipWhiteSpace(parser_ctx);
    getIndex(parser_ctx, &start_idx);
    end_idx = skipAlpha(parser_ctx);

    if (end_idx == -1 || end_idx == start_idx) {
      printf("!!Failed to locate the end of the element id\n");
    } else {
      element_id = copyData(parser_ctx, start_idx, end_idx);
      if (element_id == NULL) {
        printf("!!Copy element id failed\n");
      } else {
        if (strcmp(element_id, "Founder") != 0) {
          printf("!!Element id is not \"Founder\"\n");
        } else {
          deallocate(element_id, strlen(element_id) + 1);
          element_id = NULL;

          skipWhiteSpace(parser_ctx);
          if (!atChar(parser_ctx, '}') || skipLength(parser_ctx, 1) == -1) {
            printf("!!Failed to locate or skip initial closing brace\n");
          } else {
            content_start_idx = skipWhiteSpace(parser_ctx);
            content_end_idx = skipToNonAlphaNumSpace(parser_ctx);

            if (content_start_idx != content_end_idx) {
              while (content_end_idx > content_start_idx && isspace((int)parser_ctx->buffer[content_end_idx - 1])) {
                content_end_idx--;
              }
              founder_name = copyData(parser_ctx, content_start_idx, content_end_idx);

              if (founder_name == NULL) {
                printf("!!Failed to copy founder data\n");
              } else {
                skipWhiteSpace(parser_ctx);
                if (!atChar(parser_ctx, '{') || incChar(parser_ctx) == -1) {
                  printf("!!Failed to locate or skip the final opening brace\n");
                } else {
                  skipWhiteSpace(parser_ctx);
                  if (!atChar(parser_ctx, '#') || skipLength(parser_ctx, 1) == -1) {
                    printf("!!Failed to locate or skip the closing mark\n");
                  } else {
                    getIndex(parser_ctx, &start_idx);
                    end_idx = skipAlpha(parser_ctx);

                    if (end_idx == -1 || end_idx == start_idx) {
                      printf("!!Failed to locate the end of the closing element id\n");
                    } else {
                      element_id = copyData(parser_ctx, start_idx, end_idx);
                      if (element_id == NULL) {
                        printf("!!Failed to copy closing element id\n");
                      } else {
                        if (strcmp(element_id, "Founder") != 0) {
                          printf("!!Invalid closing element id for Founder\n");
                        } else {
                          deallocate(element_id, strlen(element_id) + 1);
                          element_id = NULL;

                          skipWhiteSpace(parser_ctx);
                          if (atChar(parser_ctx, '}') && skipLength(parser_ctx, 1) != -1) {
                            success = 1; // All checks passed
                          } else {
                            printf("!!Failed to locate final closing brace\n");
                          }
                        }
                      }
                    }
                  }
                }
              }
            } else {
              printf("!!No founder data found (or empty string)\n");
            }
          }
        }
      }
    }
  }

  if (element_id != NULL) {
      deallocate(element_id, strlen(element_id) + 1);
  }

  if (!success && founder_name != NULL) {
    deallocate(founder_name, strlen(founder_name) + 1);
    founder_name = NULL;
  }

  return founder_name;
}

// Function: extractEstablished
int extractEstablished(ParserContext *parser_ctx) {
  int established_value = 0;
  char *element_id = NULL;
  char *data_str = NULL;
  int success = 0;

  if (parser_ctx == NULL) {
    return 0;
  }

  int start_idx, end_idx;
  int data_start_idx, data_end_idx;

  if (skipWhiteSpace(parser_ctx) == -1 || !atChar(parser_ctx, '{') || skipLength(parser_ctx, 1) == -1) {
    printf("!!Failed to locate or skip opening brace\n");
  } else {
    skipWhiteSpace(parser_ctx);
    getIndex(parser_ctx, &start_idx);
    end_idx = skipAlpha(parser_ctx);

    if (end_idx == -1 || end_idx == start_idx) {
      printf("!!Failed to locate the end of the element id\n");
    } else {
      element_id = copyData(parser_ctx, start_idx, end_idx);
      if (element_id == NULL) {
        printf("!!Copy element id failed\n");
      } else {
        if (strcmp(element_id, "Established") != 0) {
          printf("!!Element id is not \"Established\"\n");
        } else {
          deallocate(element_id, strlen(element_id) + 1);
          element_id = NULL;

          skipWhiteSpace(parser_ctx);
          if (!atChar(parser_ctx, '}') || skipLength(parser_ctx, 1) == -1) {
            printf("!!Failed to locate or skip initial closing brace\n");
          } else {
            skipWhiteSpace(parser_ctx);
            getIndex(parser_ctx, &data_start_idx);
            data_end_idx = skipInt(parser_ctx);

            if (data_end_idx == -1 || data_end_idx == data_start_idx) {
              printf("!!Failed to locate the end of the established data\n");
            } else {
              skipWhiteSpace(parser_ctx);
              if (!atChar(parser_ctx, '{') || incChar(parser_ctx) == -1) {
                printf("!!Failed to locate or skip the final opening brace\n");
              } else {
                skipWhiteSpace(parser_ctx);
                if (!atChar(parser_ctx, '#') || skipLength(parser_ctx, 1) == -1) {
                  printf("!!Failed to locate or skip the closing mark\n");
                } else {
                  getIndex(parser_ctx, &start_idx);
                  end_idx = skipToNonAlphaNum(parser_ctx);

                  if (end_idx == -1 || end_idx == start_idx) {
                    printf("!!Failed to locate the end of the closing element id\n");
                  } else {
                    element_id = copyData(parser_ctx, start_idx, end_idx);
                    if (element_id == NULL) {
                      printf("!!Failed to copy closing element id\n");
                    } else {
                      if (strcmp(element_id, "Established") != 0) {
                        printf("!!Invalid closing element id for Established\n");
                      } else {
                        deallocate(element_id, strlen(element_id) + 1);
                        element_id = NULL;

                        skipWhiteSpace(parser_ctx);
                        if (atChar(parser_ctx, '}') && skipLength(parser_ctx, 1) != -1) {
                          data_str = copyData(parser_ctx, data_start_idx, data_end_idx);
                          if (data_str != NULL) {
                            established_value = atoi(data_str);
                            deallocate(data_str, strlen(data_str) + 1);
                            data_str = NULL;
                            success = 1; // All checks passed
                          } else {
                            printf("!!Failed to copy established data\n");
                          }
                        } else {
                          printf("!!Failed to locate final closing brace\n");
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  if (element_id != NULL) {
      deallocate(element_id, strlen(element_id) + 1);
  }
  if (data_str != NULL) {
      deallocate(data_str, strlen(data_str) + 1);
  }

  return established_value; // Returns 0 on failure
}