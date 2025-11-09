#include <stdio.h>    // For printf
#include <stdlib.h>   // For atoi, atof, NULL, size_t
#include <string.h>   // For memset, strcmp, strncpy, strlen
#include <ctype.h>    // For isalnum, isspace
#include <stdbool.h>  // For bool type

// Forward declarations for structures
typedef struct Territory Territory;
typedef struct ParserContext ParserContext;

// Assume these are external functions provided by the environment
// Placeholder prototypes for functions not provided in the snippet.
// These are necessary for the provided code to compile.
void receive_until(char *buffer, int max_len, int timeout);
int allocate(size_t size, int flags, void **ptr);
void deallocate(void *ptr, size_t size);
void initCounty(void *county_ptr);
int countyMenu(void *county_ptr); // Returns 0 on delete, 1 on exit
void freeCounty(void *county_ptr);
void printCountyInfo(void *county_ptr); // Assuming it takes a county pointer

// Parser related helper functions (prototypes based on usage in the snippet)
int skipWhiteSpace(ParserContext *ctx); // Returns index after skip
int getIndex(ParserContext *ctx, int *out_index); // Stores current index in out_index
int atChar(ParserContext *ctx, char c); // Returns 1 if char matches, 0 otherwise
int incChar(ParserContext *ctx); // Increments offset, returns new offset or -1 on error
int skipAlpha(ParserContext *ctx); // Returns index after alpha chars
char *copyData(ParserContext *ctx, int start_idx, int end_idx); // Returns char* or NULL
char *pullNextElementName(ParserContext *ctx);
int elementNameToEnum(const char *name); // Returns an enum value
char *extractName(ParserContext *ctx);
int extractPopulation(ParserContext *ctx);
double *extractBorder(ParserContext *ctx); // Returns double* or NULL
char *extractFounder(ParserContext *ctx);
void *countyTopLevel(ParserContext *ctx); // Returns County* or NULL
int skipLength(ParserContext *ctx, int length); // Skips length characters
int skipToNonAlphaNumSpace(ParserContext *ctx);
int skipInt(ParserContext *ctx);
int skipToNonAlphaNum(ParserContext *ctx);

// Inferred structure for Territory based on memory offsets and usage
struct Territory {
    char name[20];            // Offset 0x0
    char founder[30];         // Offset 0x14
    int population;           // Offset 0x34
    int established;          // Offset 0x38
    int num_borders;          // Offset 0x3c
    double *borders[100];     // Offset 0x40, array of pointers to 4 doubles (32 bytes each)
    int num_counties;         // Offset 0x1d0
    void *counties[10];       // Offset 0x1d4, array of pointers to County structs
}; // Total size 0x1fc (508 bytes)

// Inferred structure for ParserContext based on usage of param_1 in parsing functions
struct ParserContext {
    char *buffer;         // The input string being parsed
    int current_offset;   // Current position in the buffer
    // Other fields like line number, etc., might exist but are not used in the snippet
};

// Function: initTerritory
void initTerritory(Territory *t) {
  if (t != NULL) {
    memset(t->name, 0, sizeof(t->name));
    memset(t->founder, 0, sizeof(t->founder));
    t->population = -1; // Original code used 0xffffffff
    t->established = 0;
    t->num_counties = 0;
    t->num_borders = 0;
    memset(t->borders, 0, sizeof(t->borders));
    memset(t->counties, 0, sizeof(t->counties));
  }
}

// Function: freeTerritory
void freeTerritory(Territory *t) {
  if (t != NULL) {
    for (int i = 0; i < t->num_borders; i++) {
      if (t->borders[i] != NULL) {
        deallocate(t->borders[i], 0x20); // 4 doubles (8 bytes each) = 32 bytes
        t->borders[i] = NULL;
      }
    }
    // Iterate through the fixed-size array of county pointers
    for (int i = 0; i < 10; i++) {
      if (t->counties[i] != NULL) {
        freeCounty(t->counties[i]);
        t->counties[i] = NULL;
      }
    }
    deallocate(t, sizeof(Territory)); // Use sizeof(Territory) for clarity
  }
}

// Function: printTerritoryInfo
void printTerritoryInfo(Territory *t) {
  if (t != NULL) {
    printf("\t\tTerritory: ");
    if (t->name[0] == '\0') {
      printf("Unknown\n");
    } else {
      printf("%s\n", t->name);
    }
    if (t->population > -1) { // Check if population was set (not 0xffffffff)
      printf("\t\t\tPopulation: %d\n", t->population);
    }
    printf("\t\t\tEstablished: %d\n", t->established);
    printf("\t\t\tFounder: ");
    if (t->founder[0] == '\0') {
      printf("Unknown\n");
    } else {
      printf("%s\n", t->founder);
    }
    for (int i = 0; i < t->num_borders; i++) {
      double *border = t->borders[i];
      if (border != NULL) {
        printf("\t\t\tBorder: %f %f %f %f\n", border[0], border[1], border[2], border[3]);
      }
    }
    for (int i = 0; i < 10; i++) { // Iterate through all possible county slots
      if (t->counties[i] != NULL) {
        printCountyInfo(t->counties[i]); // Pass the county pointer
      }
    }
  }
}

// Function: territoryMenu
int territoryMenu(Territory *t) {
  int selection;
  char input_buffer[30];
  double *new_border_coords = NULL;
  void *new_county_ptr = NULL;
  int i;
  int menu_result = 0; // 0 for delete, 1 for exit without delete

  if (t == NULL) {
    return 0;
  }

  while (true) { // Replaced goto LAB_0001b4d5 with an infinite loop
    printf("\nTerritory: %s\n", t->name);
    printf("1) Display Territory Info\n");
    printf("2) Set Founder\n");
    printf("3) Set Population\n");
    printf("4) Set Established\n");
    printf("5) Add Border\n");
    printf("6) Add County\n");
    printf("7) Select County\n");
    printf("8) Delete Territory and Exit Menu\n"); // Corrected text
    printf("9) Exit Menu\n");

    memset(input_buffer, 0, sizeof(input_buffer));
    printf("Selection: ");
    receive_until(input_buffer, 10, 3); // Max 9 digits + null for atoi
    selection = atoi(input_buffer);

    memset(input_buffer, 0, sizeof(input_buffer)); // Clear buffer for next input

    switch (selection) {
      case 1:
        printTerritoryInfo(t);
        break;
      case 2:
        printf("\n-> ");
        receive_until(input_buffer, 10, sizeof(t->founder) - 1);
        i = 0;
        while (i < sizeof(t->founder) - 1 && (isalnum((unsigned char)input_buffer[i]) || input_buffer[i] == ' ')) {
          t->founder[i] = input_buffer[i];
          i++;
        }
        t->founder[i] = '\0';
        break;
      case 3:
        printf("\n-> ");
        receive_until(input_buffer, 10, sizeof(t->founder) - 1); // Max 29 chars for population string
        t->population = atoi(input_buffer);
        break;
      case 4:
        printf("\n-> ");
        receive_until(input_buffer, 10, sizeof(t->founder) - 1); // Max 29 chars for established string
        t->established = atoi(input_buffer);
        break;
      case 5:
        if (t->num_borders < 100) {
          if (allocate(0x20, 0, (void**)&new_border_coords) == 0) { // Allocate for 4 doubles
            printf("Lat Start: ");
            receive_until(input_buffer, 10, 19);
            new_border_coords[0] = atof(input_buffer);

            printf("Long Start: ");
            receive_until(input_buffer, 10, 19);
            new_border_coords[1] = atof(input_buffer);

            printf("Lat End: ");
            receive_until(input_buffer, 10, 19);
            new_border_coords[2] = atof(input_buffer);

            printf("Long End: ");
            receive_until(input_buffer, 10, 19);
            new_border_coords[3] = atof(input_buffer);

            t->borders[t->num_borders++] = new_border_coords;
          } else {
            new_border_coords = NULL; // Allocation failed
          }
        } else {
          printf("!!Max borders reached\n");
        }
        break;
      case 6:
        i = 0;
        while (i < 10 && t->counties[i] != NULL) { // Find first empty slot
          i++;
        }
        if (i == 10) {
          printf("!!Max counties reached\n");
        } else {
          printf("\nNew County Name: ");
          receive_until(input_buffer, 10, 19); // Max 19 chars for county name
          if (allocate(0xe4, 0, &new_county_ptr) == 0) { // Allocate for a County struct
            initCounty(new_county_ptr);
            t->counties[i] = new_county_ptr;
            int j = 0;
            // Assuming county name is at offset 0 within the County struct
            while (j < 19 && isalnum((unsigned char)input_buffer[j])) {
              ((char*)new_county_ptr)[j] = input_buffer[j];
              j++;
            }
            ((char*)new_county_ptr)[j] = '\0';
            t->num_counties++;
          } else {
            new_county_ptr = NULL; // Allocation failed
          }
        }
        break;
      case 7:
        for (i = 0; i < 10; i++) {
          if (t->counties[i] != NULL) {
            printf("%d) %s\n", i + 1, (char*)t->counties[i]); // Assuming county name at offset 0
          }
        }
        printf("\n-> ");
        receive_until(input_buffer, 10, 19);
        selection = atoi(input_buffer); // Reusing 'selection' for county index

        if (selection < 1 || selection > 10) {
          printf("Invalid choice...\n");
        } else if (t->counties[selection - 1] == NULL) {
          printf("Invalid choice...\n");
        } else {
          if (countyMenu(t->counties[selection - 1]) == 0) { // If countyMenu returns 0 (delete)
            t->counties[selection - 1] = NULL;
            t->num_counties--;
          }
        }
        break;
      case 8:
        freeTerritory(t);
        menu_result = 0; // Indicate territory was deleted
        return menu_result; // Exit menu and function
      case 9:
        menu_result = 1; // Indicate exit without deletion
        return menu_result; // Exit menu and function
      default:
        printf("Invalid\n");
        break;
    }
  }
}

// Function: territoryTopLevel
char *territoryTopLevel(ParserContext *ctx) {
    Territory *t = NULL;
    char *element_name = NULL;
    char *temp_str = NULL;
    int initial_ctx_offset = -1; // To restore context on error

    if (ctx == NULL) {
        return NULL;
    }

    if (allocate(sizeof(Territory), 0, (void**)&t) != 0) {
        return NULL; // Allocation failed
    }
    initTerritory(t);

    initial_ctx_offset = ctx->current_offset; // Save current offset for error reporting

    skipWhiteSpace(ctx);
    if (atChar(ctx, '{') == 0) {
        printf("!!Territory: Failed to locate opening brace.\n");
        goto error_cleanup;
    }
    incChar(ctx); // Skip '{'
    skipWhiteSpace(ctx);

    int start_idx = ctx->current_offset;
    int end_idx = skipAlpha(ctx);
    if (end_idx == -1 || end_idx == start_idx) {
        printf("!!Territory: Failed to locate the end of the element id.\n");
        goto error_cleanup;
    }

    temp_str = copyData(ctx, start_idx, end_idx);
    if (temp_str == NULL) {
        printf("!!Territory: Failed to copy element id.\n");
        goto error_cleanup;
    }

    if (strcmp(temp_str, "Territory") != 0) {
        printf("!!Territory: Invalid opening element id: %s\n", temp_str);
        deallocate(temp_str, strlen(temp_str) + 1);
        temp_str = NULL;
        goto error_cleanup;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) {
        printf("!!Territory: Failed to locate initial closing brace.\n");
        goto error_cleanup;
    }
    incChar(ctx); // Skip '}'

    // Parse elements within the territory
    while ((element_name = pullNextElementName(ctx)) != NULL) {
        int element_enum = elementNameToEnum(element_name);
        deallocate(element_name, strlen(element_name) + 1);
        element_name = NULL; // Clear for next iteration

        switch (element_enum) {
            case 1: // Name
                temp_str = extractName(ctx);
                if (temp_str == NULL) goto error_cleanup;
                strncpy(t->name, temp_str, sizeof(t->name) - 1);
                t->name[sizeof(t->name) - 1] = '\0';
                deallocate(temp_str, strlen(temp_str) + 1);
                temp_str = NULL;
                break;
            case 10: // Population
                t->population = extractPopulation(ctx);
                if (t->population < 0) goto error_cleanup;
                break;
            case 14: { // Border
                if (t->num_borders >= 100) {
                    printf("!!Max territory border count is %d\n", 100);
                    goto error_cleanup;
                }
                double *border_ptr = extractBorder(ctx);
                if (border_ptr == NULL) goto error_cleanup;
                t->borders[t->num_borders++] = border_ptr;
                break;
            }
            case 16: // Established
                t->established = extractEstablished(ctx);
                break;
            case 17: // Founder
                temp_str = extractFounder(ctx);
                if (temp_str == NULL) goto error_cleanup;
                strncpy(t->founder, temp_str, sizeof(t->founder) - 1);
                t->founder[sizeof(t->founder) - 1] = '\0';
                deallocate(temp_str, strlen(temp_str) + 1);
                temp_str = NULL;
                break;
            case 18: { // County
                if (t->num_counties >= 10) {
                    printf("!!Max territory county count is %d\n", 10);
                    goto error_cleanup;
                }
                void *county_ptr = countyTopLevel(ctx);
                if (county_ptr == NULL) goto error_cleanup;
                t->counties[t->num_counties++] = county_ptr;
                break;
            }
            default:
                printf("Invalid element '%s' for territory\n", element_name);
                goto error_cleanup;
        }
    }

    // Final closing tag check
    skipWhiteSpace(ctx);
    if (atChar(ctx, '{') == 0) {
        printf("!!Territory: Failed to locate final opening brace.\n");
        goto error_cleanup;
    }
    skipLength(ctx, 1); // Skip '{'
    skipWhiteSpace(ctx);
    if (atChar(ctx, '#') == 0) {
        printf("!!Territory: Failed to locate closing mark '#'.\n");
        goto error_cleanup;
    }
    skipLength(ctx, 1); // Skip '#'

    start_idx = ctx->current_offset;
    end_idx = skipAlpha(ctx);
    if (end_idx == -1 || end_idx == start_idx) {
        printf("!!Territory: Failed to locate the end of the closing element id.\n");
        goto error_cleanup;
    }

    temp_str = copyData(ctx, start_idx, end_idx);
    if (temp_str == NULL) {
        printf("!!Territory: Failed to copy closing element id.\n");
        goto error_cleanup;
    }

    if (strcmp(temp_str, "Territory") != 0) {
        printf("!!Territory: Invalid closing element id: %s\n", temp_str);
        deallocate(temp_str, strlen(temp_str) + 1);
        temp_str = NULL;
        goto error_cleanup;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) {
        printf("!!Territory: Failed to locate final closing brace.\n");
        goto error_cleanup;
    }
    incChar(ctx); // Skip '}'

    return (char*)t; // Success

error_cleanup:
    if (t != NULL) {
        freeTerritory(t); // This will deallocate t itself
        t = NULL;
    }
    if (element_name != NULL) {
        deallocate(element_name, strlen(element_name) + 1);
    }
    if (temp_str != NULL) {
        deallocate(temp_str, strlen(temp_str) + 1);
    }
    ctx->current_offset = initial_ctx_offset; // Restore parser context for error reporting
    printf("Error at: %s\n", ctx->buffer + ctx->current_offset);
    return NULL;
}

// Function: extractFounder
char *extractFounder(ParserContext *ctx) {
    char *founder_name = NULL;
    char *element_id = NULL;
    int start_idx, end_idx;

    if (ctx == NULL) return NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '{') == 0) { printf("!!Failed to locate opening brace\n"); return NULL; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip opening brace\n"); return NULL; }

    skipWhiteSpace(ctx);
    start_idx = ctx->current_offset;
    end_idx = skipAlpha(ctx);
    if (end_idx == -1 || end_idx == start_idx) { printf("!!Failed to locate the end of the element id\n"); return NULL; }

    element_id = copyData(ctx, start_idx, end_idx);
    if (element_id == NULL) { printf("!!Copy from %d to %d failed\n", start_idx, end_idx); return NULL; }

    if (strcmp(element_id, "Founder") != 0) {
        printf("!!Element id is not \"Founder\"\n");
        deallocate(element_id, strlen(element_id) + 1);
        return NULL;
    }
    deallocate(element_id, strlen(element_id) + 1);
    element_id = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) { printf("!!Failed to locate initial closing brace\n"); return NULL; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip initial closing brace\n"); return NULL; }

    start_idx = skipWhiteSpace(ctx);
    end_idx = skipToNonAlphaNumSpace(ctx);
    if (start_idx == end_idx) { return NULL; } // No data

    // Trim trailing whitespace from the data
    while (end_idx > start_idx && isspace((unsigned char)ctx->buffer[end_idx - 1])) {
        end_idx--;
    }

    founder_name = copyData(ctx, start_idx, end_idx);
    if (founder_name == NULL) { return NULL; }

    skipWhiteSpace(ctx);
    if (atChar(ctx, '{') == 0) { printf("!!Failed to locate the final opening brace\n"); deallocate(founder_name, strlen(founder_name) + 1); return NULL; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip the final opening brace\n"); deallocate(founder_name, strlen(founder_name) + 1); return NULL; }

    skipWhiteSpace(ctx);
    if (atChar(ctx, '#') == 0) { printf("!!Failed to locate the closing mark\n"); deallocate(founder_name, strlen(founder_name) + 1); return NULL; }
    if (skipLength(ctx, 1) == -1) { printf("!!Failed to skip closing mark\n"); deallocate(founder_name, strlen(founder_name) + 1); return NULL; }

    start_idx = ctx->current_offset;
    end_idx = skipAlpha(ctx);
    if (end_idx == -1 || end_idx == start_idx) { printf("!!Failed to locate the end of the closing element id\n"); deallocate(founder_name, strlen(founder_name) + 1); return NULL; }

    element_id = copyData(ctx, start_idx, end_idx);
    if (element_id == NULL) { printf("!!Copy closing element id failed\n"); deallocate(founder_name, strlen(founder_name) + 1); return NULL; }

    if (strcmp(element_id, "Founder") != 0) {
        printf("!!Closing element id is not \"Founder\"\n");
        deallocate(element_id, strlen(element_id) + 1);
        deallocate(founder_name, strlen(founder_name) + 1);
        return NULL;
    }
    deallocate(element_id, strlen(element_id) + 1);

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) { printf("!!Failed to locate final closing brace\n"); deallocate(founder_name, strlen(founder_name) + 1); return NULL; }
    skipLength(ctx, 1); // Skip '}'

    return founder_name;
}

// Function: extractEstablished
int extractEstablished(ParserContext *ctx) {
    char *established_str = NULL;
    char *element_id = NULL;
    int established_val = 0;
    int start_idx, end_idx;

    if (ctx == NULL) return 0;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '{') == 0) { printf("!!Failed to locate opening brace\n"); return 0; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip opening brace\n"); return 0; }

    skipWhiteSpace(ctx);
    start_idx = ctx->current_offset;
    end_idx = skipAlpha(ctx);
    if (end_idx == -1 || end_idx == start_idx) { printf("!!Failed to locate the end of the element id\n"); return 0; }

    element_id = copyData(ctx, start_idx, end_idx);
    if (element_id == NULL) { printf("!!Copy from %d to %d failed\n", start_idx, end_idx); return 0; }

    if (strcmp(element_id, "Established") != 0) {
        printf("!!Element id is not \"Established\"\n");
        deallocate(element_id, strlen(element_id) + 1);
        return 0;
    }
    deallocate(element_id, strlen(element_id) + 1);
    element_id = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) { printf("!!Failed to locate initial closing brace\n"); return 0; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip initial closing brace\n"); return 0; }

    skipWhiteSpace(ctx);
    start_idx = ctx->current_offset;
    end_idx = skipInt(ctx);
    if (end_idx == -1 || end_idx == start_idx) { printf("!!Failed to locate the end of the established data\n"); return 0; }

    skipWhiteSpace(ctx);
    if (atChar(ctx, '{') == 0) { printf("!!Failed to locate the final opening brace\n"); return 0; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip the final opening brace\n"); return 0; }

    skipWhiteSpace(ctx);
    if (atChar(ctx, '#') == 0) { printf("!!Failed to locate the closing mark\n"); return 0; }
    if (skipLength(ctx, 1) == -1) { printf("!!Failed to skip closing mark\n"); return 0; }

    int closing_id_start = ctx->current_offset;
    int closing_id_end = skipToNonAlphaNum(ctx);
    if (closing_id_end == -1 || closing_id_end == closing_id_start) { printf("!!Failed to locate the end of the closing element id\n"); return 0; }

    element_id = copyData(ctx, closing_id_start, closing_id_end);
    if (element_id == NULL) { printf("!!Copy closing element id failed\n"); return 0; }

    if (strcmp(element_id, "Established") != 0) {
        printf("!!Closing element id is not \"Established\"\n");
        deallocate(element_id, strlen(element_id) + 1);
        return 0;
    }
    deallocate(element_id, strlen(element_id) + 1);
    element_id = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) { printf("!!Failed to locate final closing brace\n"); return 0; }
    skipLength(ctx, 1); // Skip '}'

    // Now extract the actual integer value
    established_str = copyData(ctx, start_idx, end_idx);
    if (established_str == NULL) { printf("!!Failed to copy established data\n"); return 0; }

    established_val = atoi(established_str);
    deallocate(established_str, strlen(established_str) + 1);

    return established_val;
}