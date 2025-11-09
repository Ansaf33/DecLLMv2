#include <stdio.h>    // For printf
#include <stdlib.h>   // For atoi, atof
#include <string.h>   // For strlen, strcmp, strncpy, strcpy, memset
#include <ctype.h>    // For isalnum, isspace

// Define constants for City object memory layout
#define CITY_NAME_OFFSET          0x0
#define CITY_NAME_SIZE            20
#define CITY_MAYOR_OFFSET         0x14
#define CITY_MAYOR_SIZE           30
#define CITY_URL_OFFSET           0x32
#define CITY_URL_SIZE             30
#define CITY_POPULATION_OFFSET    0x50
#define CITY_POPULATION_SIZE      4 // sizeof(int)
#define CITY_NUM_BORDERS_OFFSET   0x54
#define CITY_NUM_BORDERS_SIZE     4 // sizeof(int)
#define CITY_BORDERS_ARRAY_OFFSET 0x58
#define CITY_MAX_BORDERS          10 // 0xb - 1 for 0-indexed array
#define CITY_OBJECT_ALLOC_SIZE    0x80 // 128 bytes, implies 32-bit pointers for borders[10] to fit
#define BORDER_DATA_SIZE          0x20 // 32 bytes (4 doubles)

// Parser context structure inferred from usage
// param_1 in cityTopLevel, skipOpen, etc. is treated as this struct pointer
typedef struct ParserContext {
    char *buffer;
    int buffer_len; // Or some other length/size, not explicitly used consistently for length
    int current_offset;
} ParserContext;

// External function declarations (assuming these are provided elsewhere)
// Return types and parameters inferred from usage in the provided snippet
extern int receive_until(char *buf, int max_len, int max_fill);
extern int allocate(size_t size, int unk_param, void **out_ptr);
extern void deallocate(void *ptr, size_t size);
extern int getIndex(ParserContext *ctx, int *out_index);
extern void* copyData(ParserContext *ctx, int start, int end);
extern int skipAlpha(ParserContext *ctx);
extern int elementNameToEnum(const char *name);
extern char* pullNextElementName(ParserContext *ctx);
extern char* extractName(ParserContext *ctx);
extern int extractPopulation(ParserContext *ctx);
extern double* extractBorder(ParserContext *ctx); // Returns double* to border data
extern char* extractMayor(ParserContext *ctx);
extern char* extractUrl(ParserContext *ctx);
extern void skipWhiteSpace(ParserContext *ctx);
extern int atChar(ParserContext *ctx, char c);
extern int incChar(ParserContext *ctx);
extern int skipToNonAlphaNumSpace(ParserContext *ctx);
extern void skipUrl(ParserContext *ctx);


// Function: freeCharPtr
void freeCharPtr(char **ptr_to_char_ptr) {
  if ((ptr_to_char_ptr != NULL) && (*ptr_to_char_ptr != NULL)) {
    size_t string_length = strlen(*ptr_to_char_ptr);
    deallocate(*ptr_to_char_ptr, string_length + 1);
    *ptr_to_char_ptr = NULL;
  }
}

// Function: freeCity
void freeCity(char *city_ptr) {
  if (city_ptr != NULL) {
    int num_borders = *(int *)(city_ptr + CITY_NUM_BORDERS_OFFSET);
    for (int i = 0; i < num_borders; i++) {
      double **border_ptr_array = (double **)(city_ptr + CITY_BORDERS_ARRAY_OFFSET);
      if (border_ptr_array[i] != NULL) {
        deallocate(border_ptr_array[i], BORDER_DATA_SIZE);
        border_ptr_array[i] = NULL; // Clear the pointer after deallocating
      }
    }
    deallocate(city_ptr, CITY_OBJECT_ALLOC_SIZE);
  }
}

// Function: initCity
char *initCity(char *city_ptr) {
  if (city_ptr == NULL) {
    return NULL;
  }

  // Clear name field
  memset(city_ptr + CITY_NAME_OFFSET, 0, CITY_NAME_SIZE);
  // Clear mayor field
  memset(city_ptr + CITY_MAYOR_OFFSET, 0, CITY_MAYOR_SIZE);
  // Clear url field
  memset(city_ptr + CITY_URL_OFFSET, 0, CITY_URL_SIZE);

  // Set population to -1 and num_borders to 0
  *(int *)(city_ptr + CITY_POPULATION_OFFSET) = -1;
  *(int *)(city_ptr + CITY_NUM_BORDERS_OFFSET) = 0;

  // Clear the array of border pointers
  memset(city_ptr + CITY_BORDERS_ARRAY_OFFSET, 0, CITY_MAX_BORDERS * sizeof(double*));

  return city_ptr;
}

// Function: printCityInfo
void printCityInfo(char *city_ptr) {
  if (city_ptr == NULL) {
    return;
  }

  printf("\t\t\t\tCity: ");
  if (*(char *)(city_ptr + CITY_NAME_OFFSET) == '\0') {
    printf("Unknown\n");
  } else {
    printf("%s\n", city_ptr + CITY_NAME_OFFSET);
  }

  if (*(int *)(city_ptr + CITY_POPULATION_OFFSET) > -1) {
    printf("\t\t\t\t\tPopulation: %d\n", *(int *)(city_ptr + CITY_POPULATION_OFFSET));
  }

  if (*(char *)(city_ptr + CITY_MAYOR_OFFSET) != '\0') {
    printf("\t\t\t\t\tMayor: %s\n", city_ptr + CITY_MAYOR_OFFSET);
  }

  if (*(char *)(city_ptr + CITY_URL_OFFSET) != '\0') {
    printf("\t\t\t\t\tUrl: %s\n", city_ptr + CITY_URL_OFFSET);
  }

  int num_borders = *(int *)(city_ptr + CITY_NUM_BORDERS_OFFSET);
  for (int i = 0; i < num_borders; i++) {
    double **border_ptr_array = (double **)(city_ptr + CITY_BORDERS_ARRAY_OFFSET);
    double *border_data = border_ptr_array[i];
    if (border_data != NULL) {
      printf("\t\t\t\t\tBorder: %f %f %f %f\n", border_data[0], border_data[1], border_data[2], border_data[3]);
    }
  }
}

// Function: skipOpen
int skipOpen(ParserContext *ctx, int check_hash) {
  if (ctx == NULL) {
    return 0;
  }
  skipWhiteSpace(ctx);
  if (atChar(ctx, '{') == 0) {
    return 0;
  }
  if (incChar(ctx) == -1) {
    return 0;
  }
  skipWhiteSpace(ctx);
  if (check_hash != 0) {
    if (atChar(ctx, '#') == 0) {
      return 0;
    }
    if (incChar(ctx) == -1) {
      return 0;
    }
  }
  return 1;
}

// Function: cityMenu
int cityMenu(char *city_ptr) {
  double *border_data_ptr = NULL;
  char input_buffer[CITY_MAYOR_SIZE]; // Max size needed for input selection, mayor, url, etc.
  int selection;
  int result_code = 0; // Default return on exit/error
  int keep_menu_open = 1;

  if (city_ptr == NULL) {
    return 0;
  }

  do {
    printf("\nCity: %s\n", city_ptr + CITY_NAME_OFFSET);
    printf("1) Display City Info\n");
    printf("2) Set Mayor\n");
    printf("3) Set Url\n");
    printf("4) Set Population\n");
    printf("5) Add Border\n");
    printf("6) Delete City and Exit Menu\n");
    printf("7) Exit Menu\n");
    printf("Selection: ");

    memset(input_buffer, 0, sizeof(input_buffer));
    // Max_len for receive_until is 10 in original, let's keep it for compatibility or assume it's max_chars_to_read
    receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
    selection = atoi(input_buffer);

    switch (selection) {
      default:
        printf("Invalid\n");
        break;
      case 1:
        printCityInfo(city_ptr);
        break;
      case 2: {
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, 10, CITY_MAYOR_SIZE - 1);
        int i = 0;
        char *mayor_dest = city_ptr + CITY_MAYOR_OFFSET;
        while (i < CITY_MAYOR_SIZE - 1 && (isalnum((int)input_buffer[i]) || input_buffer[i] == ' ')) {
          mayor_dest[i] = input_buffer[i];
          i++;
        }
        mayor_dest[i] = '\0'; // Null-terminate
        break;
      }
      case 3: {
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, 10, CITY_URL_SIZE - 1);
        int i = 0;
        char *url_dest = city_ptr + CITY_URL_OFFSET;
        while (i < CITY_URL_SIZE - 1 && (isalnum((int)input_buffer[i]) || input_buffer[i] == ':' || input_buffer[i] == '.' || input_buffer[i] == '/')) {
          url_dest[i] = input_buffer[i];
          i++;
        }
        url_dest[i] = '\0'; // Null-terminate
        break;
      }
      case 4:
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
        *(int *)(city_ptr + CITY_POPULATION_OFFSET) = atoi(input_buffer);
        break;
      case 5:
        if (*(int *)(city_ptr + CITY_NUM_BORDERS_OFFSET) < CITY_MAX_BORDERS) {
          border_data_ptr = NULL;
          if (allocate(BORDER_DATA_SIZE, 0, (void **)&border_data_ptr) == 0) {
            printf("Lat Start: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive_until(input_buffer, 10, 0x13); // Max len 19
            border_data_ptr[0] = atof(input_buffer);

            printf("Long Start: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive_until(input_buffer, 10, 0x13);
            border_data_ptr[1] = atof(input_buffer);

            printf("Lat End: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive_until(input_buffer, 10, 0x13);
            border_data_ptr[2] = atof(input_buffer);

            printf("Long End: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive_until(input_buffer, 10, 0x13);
            border_data_ptr[3] = atof(input_buffer);

            double **border_ptr_array = (double **)(city_ptr + CITY_BORDERS_ARRAY_OFFSET);
            border_ptr_array[*(int *)(city_ptr + CITY_NUM_BORDERS_OFFSET)] = border_data_ptr;
            *(int *)(city_ptr + CITY_NUM_BORDERS_OFFSET) += 1;
          } else {
            border_data_ptr = NULL; // Allocation failed
          }
        } else {
          printf("Max Borders\n");
        }
        break;
      case 6:
        freeCity(city_ptr);
        result_code = 0;
        keep_menu_open = 0;
        break;
      case 7:
        result_code = 1;
        keep_menu_open = 0;
        break;
    }
  } while (keep_menu_open);

  return result_code;
}

// Function: cityTopLevel
char * cityTopLevel(ParserContext *ctx) {
  char *city_ptr = NULL;
  char *element_name = NULL;
  int start_index = 0;
  int end_index = 0;
  int error_offset = 0;
  int element_enum_val;
  int border_index;
  char *tag_name = NULL;
  int parse_successful = 0;
  int inner_loop_active = 1; // Flag to control the inner while loop

  do { // Use do-while(0) to simulate a single exit point without goto
    if (ctx == NULL || skipOpen(ctx, 0) == 0) {
      break;
    }

    getIndex(ctx, &start_index);
    end_index = skipAlpha(ctx);
    if (end_index == -1 || end_index == start_index) {
      break;
    }

    tag_name = (char *)copyData(ctx, start_index, end_index);
    if (tag_name == NULL) {
      break;
    }

    if (strcmp(tag_name, "City") != 0) {
      freeCharPtr(&tag_name); tag_name = NULL;
      break;
    }
    freeCharPtr(&tag_name); tag_name = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) {
      break;
    }
    incChar(ctx);
    skipWhiteSpace(ctx);

    error_offset = ctx->current_offset;
    if (allocate(CITY_OBJECT_ALLOC_SIZE, 0, (void **)&city_ptr) != 0) {
      city_ptr = NULL;
      break;
    }
    initCity(city_ptr);

    element_name = pullNextElementName(ctx);
    while (element_name != NULL && inner_loop_active) {
      element_enum_val = elementNameToEnum(element_name);
      freeCharPtr(&element_name); element_name = NULL;

      switch (element_enum_val) {
        default:
          printf("!!Invalid element ID for City\n");
          inner_loop_active = 0;
          break;
        case 1: { // Name
          char *name_str = extractName(ctx);
          if (name_str == NULL) { inner_loop_active = 0; break; }
          memset(city_ptr + CITY_NAME_OFFSET, 0, CITY_NAME_SIZE);
          strncpy(city_ptr + CITY_NAME_OFFSET, name_str, CITY_NAME_SIZE - 1);
          freeCharPtr(&name_str);
          break;
        }
        case 10: { // Population
          int population_val = extractPopulation(ctx);
          *(int *)(city_ptr + CITY_POPULATION_OFFSET) = population_val;
          if (*(int *)(city_ptr + CITY_POPULATION_OFFSET) < 0) { inner_loop_active = 0; break; }
          break;
        }
        case 0xe: { // Border
          border_index = *(int *)(city_ptr + CITY_NUM_BORDERS_OFFSET);
          if (border_index >= CITY_MAX_BORDERS) { inner_loop_active = 0; break; }
          double *border_data = extractBorder(ctx);
          if (border_data == NULL) { inner_loop_active = 0; break; }
          double **border_ptr_array = (double **)(city_ptr + CITY_BORDERS_ARRAY_OFFSET);
          border_ptr_array[border_index] = border_data;
          *(int *)(city_ptr + CITY_NUM_BORDERS_OFFSET) += 1;
          break;
        }
        case 0x17: { // Mayor
          char *mayor_str = extractMayor(ctx);
          if (mayor_str == NULL) { inner_loop_active = 0; break; }
          memset(city_ptr + CITY_MAYOR_OFFSET, 0, CITY_MAYOR_SIZE);
          strcpy(city_ptr + CITY_MAYOR_OFFSET, mayor_str);
          freeCharPtr(&mayor_str);
          break;
        }
        case 0x18: { // Url
          char *url_str = extractUrl(ctx);
          if (url_str == NULL) { inner_loop_active = 0; break; }
          memset(city_ptr + CITY_URL_OFFSET, 0, CITY_URL_SIZE);
          strncpy(city_ptr + CITY_URL_OFFSET, url_str, CITY_URL_SIZE - 1);
          freeCharPtr(&url_str);
          break;
        }
      }
      if (inner_loop_active) { // Only pull next element if no error occurred in switch
        error_offset = ctx->current_offset;
        element_name = pullNextElementName(ctx);
      }
    }

    if (!inner_loop_active) { // If inner loop broke due to error
        break;
    }
    
    // After loop, expect closing tag
    if (city_ptr == NULL) { // Defensive check, should not happen if inner_loop_active is true
        break;
    }

    if (skipOpen(ctx, 1) == 0) { // Expect # and { for closing tag
      break;
    }

    getIndex(ctx, &start_index);
    end_index = skipAlpha(ctx);
    if (end_index == -1 || end_index == start_index) {
      break;
    }

    tag_name = (char *)copyData(ctx, start_index, end_index);
    if (tag_name == NULL) {
      break;
    }

    if (strcmp(tag_name, "City") != 0) {
      freeCharPtr(&tag_name); tag_name = NULL;
      break;
    }
    freeCharPtr(&tag_name); tag_name = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) {
      break;
    }
    incChar(ctx);
    parse_successful = 1; // Mark successful parsing

  } while (0); // Execute the block once

  // Cleanup section
  if (element_name != NULL) { freeCharPtr(&element_name); }
  if (tag_name != NULL) { freeCharPtr(&tag_name); }

  if (!parse_successful) {
    if (city_ptr != NULL) {
      freeCity(city_ptr);
      city_ptr = NULL;
    }
    printf("!!Error at: %s\n", ctx->buffer + error_offset);
    ctx->current_offset = error_offset;
  }
  return city_ptr;
}

// Function: extractMayor
char * extractMayor(ParserContext *ctx) {
  char *result_mayor_name = NULL;
  char *closing_tag_name = NULL;
  char *opening_tag_name = NULL;
  int start_index;
  int end_index;

  do { // do-while(0) for single exit point
    if (ctx == NULL || skipOpen(ctx, 0) == 0) {
      break;
    }

    start_index = ctx->current_offset;
    end_index = skipAlpha(ctx);
    if (end_index == -1 || start_index == end_index) {
      break;
    }

    opening_tag_name = (char *)copyData(ctx, start_index, end_index);
    if (opening_tag_name == NULL) {
      break;
    }

    if (strcmp(opening_tag_name, "Mayor") != 0) {
      break;
    }
    freeCharPtr(&opening_tag_name); opening_tag_name = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) {
      break;
    }
    incChar(ctx);
    skipWhiteSpace(ctx);

    start_index = ctx->current_offset;
    end_index = skipToNonAlphaNumSpace(ctx);
    if (end_index < 0 || start_index == end_index) {
      break;
    }

    // Trim trailing whitespace
    while (end_index > start_index && isspace((int)*(ctx->buffer + end_index - 1))) {
      end_index--;
    }
    
    result_mayor_name = (char *)copyData(ctx, start_index, end_index);
    if (result_mayor_name == NULL) {
      break;
    }

    skipWhiteSpace(ctx);
    if (skipOpen(ctx, 1) == 0) { // Expect # and { for closing tag
      break;
    }

    start_index = ctx->current_offset;
    end_index = skipAlpha(ctx);
    if (end_index == -1 || start_index == end_index) {
      break;
    }

    closing_tag_name = (char *)copyData(ctx, start_index, end_index);
    if (closing_tag_name == NULL) {
      break;
    }

    if (strcmp(closing_tag_name, "Mayor") != 0) {
      break;
    }
    freeCharPtr(&closing_tag_name); closing_tag_name = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) {
      break;
    }
    incChar(ctx);
    return result_mayor_name; // Success exit
  } while (0);

  // Cleanup on error path
  if (opening_tag_name != NULL) { freeCharPtr(&opening_tag_name); }
  if (closing_tag_name != NULL) { freeCharPtr(&closing_tag_name); }
  if (result_mayor_name != NULL) { freeCharPtr(&result_mayor_name); }
  return NULL;
}

// Function: extractUrl
char * extractUrl(ParserContext *ctx) {
  char *result_url = NULL;
  char *closing_tag_name = NULL;
  char *opening_tag_name = NULL;
  int start_index;

  do { // do-while(0) for single exit point
    if (ctx == NULL || skipOpen(ctx, 0) == 0) {
      break;
    }

    getIndex(ctx, &start_index);
    skipAlpha(ctx); // Updates ctx->current_offset
    opening_tag_name = (char *)copyData(ctx, start_index, ctx->current_offset);
    if (opening_tag_name == NULL) {
      break;
    }

    if (strcmp(opening_tag_name, "Url") != 0) {
      break;
    }
    freeCharPtr(&opening_tag_name); opening_tag_name = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) {
      break;
    }
    incChar(ctx);
    skipWhiteSpace(ctx);

    getIndex(ctx, &start_index);
    skipUrl(ctx); // Updates ctx->current_offset
    result_url = (char *)copyData(ctx, start_index, ctx->current_offset);
    if (result_url == NULL) {
      break;
    }

    skipWhiteSpace(ctx);
    if (skipOpen(ctx, 1) == 0) { // Expect # and { for closing tag
      break;
    }

    getIndex(ctx, &start_index);
    skipAlpha(ctx); // Updates ctx->current_offset
    closing_tag_name = (char *)copyData(ctx, start_index, ctx->current_offset);
    if (closing_tag_name == NULL) {
      break;
    }

    if (strcmp(closing_tag_name, "Url") != 0) {
      break;
    }
    freeCharPtr(&closing_tag_name); closing_tag_name = NULL;

    skipWhiteSpace(ctx);
    if (atChar(ctx, '}') == 0) {
      break;
    }
    incChar(ctx);
    return result_url; // Success exit
  } while (0);

  // Cleanup on error path
  if (opening_tag_name != NULL) { freeCharPtr(&opening_tag_name); }
  if (closing_tag_name != NULL) { freeCharPtr(&closing_tag_name); }
  if (result_url != NULL) { freeCharPtr(&result_url); }
  return NULL;
}