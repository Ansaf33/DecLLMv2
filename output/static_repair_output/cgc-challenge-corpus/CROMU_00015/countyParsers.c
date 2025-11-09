#include <stdio.h>
#include <stdlib.h>   // For atoi, atof, malloc, free
#include <string.h>   // For memset, strcmp, strncpy, strlen, strcspn
#include <ctype.h>    // For isalpha, isalnum, isspace
#include <stdbool.h>  // For bool type

// --- Type Definitions (assuming 4-byte int, 8-byte double/long long) ---
typedef int undefined4;
typedef double undefined8;
// long double is a standard C type.

// --- Forward Declarations / Stubs for External Functions ---
// These are placeholders. Real implementations would be linked externally.
// Replaced bzero with memset for modern C compatibility.
// Assuming these are custom functions from the original context.
void receive_until(char *buffer, int delimiter, int max_len) {
    // Placeholder: Reads up to max_len-1 characters or until newline, then null-terminates.
    if (fgets(buffer, max_len, stdin) != NULL) {
        // Remove trailing newline if present
        buffer[strcspn(buffer, "\n")] = 0;
    } else {
        buffer[0] = '\0'; // Ensure null-terminated on error
    }
}

int allocate(size_t size, int type, void **ptr) {
    *ptr = malloc(size);
    return (*ptr == NULL); // 0 on success, non-zero on failure
}

void deallocate(void *ptr, size_t size) {
    free(ptr);
}

// Structures for ParserState, City, Border, County
// Based on memory offsets and usage in the original code.

typedef struct ParserState {
    char *input_string; // Corresponds to *param_1 in original parsing functions
    int current_index;  // Corresponds to param_1[2] in original parsing functions
    // Add other parser state variables if necessary
} ParserState;

typedef struct City {
    char name[20]; // 0x14 bytes
    // ... other city data (total 0x80 bytes implied by allocate(0x80))
} City;

typedef struct Border {
    double lat_start;
    double long_start;
    double lat_end;
    double long_end;
} Border; // Total 4 * sizeof(double) = 32 bytes (0x20)

typedef struct County {
    char name[20];            // 0x00 - 0x13 (20 bytes)
    char seat[20];            // 0x14 - 0x27 (20 bytes)
    int population;           // 0x28 (4 bytes)
    double area;              // 0x2c (8 bytes)
    double density;           // 0x34 (8 bytes)
    int city_count;           // 0x3c (4 bytes)
    City *cities[20];         // 0x40 - 0x8C (20 * 4 bytes = 80 bytes for pointers)
    int border_count;         // 0x90 (4 bytes)
    Border *borders[20];      // 0x94 - 0xE0 (20 * 4 bytes = 80 bytes for pointers)
                              // Total size: 20+20+4+8+8+4+80+4+80 = 228 bytes (0xE4)
} County;

// County related function prototypes
void freeCounty(County *county_ptr);
County *initCounty(County *county_ptr);
void printCountyInfo(County *county_ptr);
int countyMenu(County *county_ptr);
County *countyTopLevel(ParserState *parser_state);

// City related function prototypes
void initCity(void *city_ptr) {
    if (city_ptr) {
        memset(city_ptr, 0, sizeof(City));
    }
}

int cityMenu(void *city_ptr) {
    printf("Entering City Menu for %s (stub)\n", ((City*)city_ptr)->name);
    // In a real program, this would involve user input and logic.
    return 1; // Exit city menu, don't delete city
}

void freeCity(void *city_ptr) {
    if (city_ptr) {
        deallocate(city_ptr, sizeof(City));
    }
}

void printCityInfo() {
    printf("\t\t\t\tCity Info (stub)\n");
}

// Parser related function prototypes
void skipWhiteSpace(ParserState *parser_state) {
    if (!parser_state || !parser_state->input_string) return;
    while (isspace((unsigned char)parser_state->input_string[parser_state->current_index])) {
        parser_state->current_index++;
    }
}

int atChar(ParserState *parser_state, char c) {
    if (!parser_state || !parser_state->input_string) return 0;
    return (parser_state->input_string[parser_state->current_index] == c);
}

void incChar(ParserState *parser_state) {
    if (parser_state && parser_state->input_string) {
        parser_state->current_index++;
    }
}

int getIndex(ParserState *parser_state, int *index_ptr) {
    if (parser_state && index_ptr) {
        *index_ptr = parser_state->current_index;
        return 1;
    }
    return 0;
}

int skipAlpha(ParserState *parser_state) {
    if (!parser_state || !parser_state->input_string) return -1;
    int start_idx = parser_state->current_index;
    while (isalpha((unsigned char)parser_state->input_string[parser_state->current_index])) {
        parser_state->current_index++;
    }
    return (parser_state->current_index == start_idx) ? -1 : parser_state->current_index;
}

char *copyData(ParserState *parser_state, int start_idx, int end_idx) {
    if (!parser_state || !parser_state->input_string || start_idx < 0 || end_idx <= start_idx) return NULL;
    size_t length = end_idx - start_idx;
    char *data = (char *)malloc(length + 1);
    if (data) {
        strncpy(data, parser_state->input_string + start_idx, length);
        data[length] = '\0';
    }
    return data;
}

int checkId(ParserState *parser_state, char *expected_id, int check_hash);
long double extractDensity(ParserState *parser_state);
long double extractArea(ParserState *parser_state);
char *extractSeat(ParserState *parser_state);
char *pullNextElementName(ParserState *parser_state);
int elementNameToEnum(char *element_name);
char *extractName(ParserState *parser_state);
undefined4 extractPopulation(ParserState *parser_state);
undefined4 extractBorder(ParserState *parser_state);
undefined4 cityTopLevel(ParserState *parser_state); // Returns City* cast to undefined4
void skipLength(ParserState *parser_state, int length);
int skipFloat(ParserState *parser_state);

// Function: countyMenu
int countyMenu(County *county_ptr) {
  int choice;
  char input_buffer[30];
  int result = 3; // Default return value for "continue"

  if (county_ptr == NULL) {
    return 0; // Invalid county pointer
  }

  do {
    printf("County: %s\n", county_ptr->name);
    printf("1) Display County Info\n");
    printf("2) Set Seat\n");
    printf("3) Set Population\n");
    printf("4) Set Area\n");
    printf("5) Set Density\n");
    printf("6) Add Border\n");
    printf("7) Add City\n");
    printf("8) Select City\n");
    printf("9) Delete County and Exit Menu\n");
    printf("10) Exit Menu\n");
    printf("Selection: ");

    memset(input_buffer, 0, sizeof(input_buffer)); // Clear buffer
    receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
    choice = atoi(input_buffer);

    switch (choice) {
      default:
        printf("Invalid choice\n");
        break;
      case 1:
        printCountyInfo(county_ptr);
        break;
      case 2: {
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, '\n', sizeof(county_ptr->seat) - 1); // Max 19 chars + null
        int i = 0;
        while (i < sizeof(county_ptr->seat) - 1 && isalpha((unsigned char)input_buffer[i])) {
          county_ptr->seat[i] = input_buffer[i];
          i++;
        }
        county_ptr->seat[i] = '\0';
        break;
      }
      case 3: {
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
        county_ptr->population = atoi(input_buffer);
        break;
      }
      case 4: {
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
        county_ptr->area = atof(input_buffer);
        break;
      }
      case 5: {
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
        county_ptr->density = atof(input_buffer);
        break;
      }
      case 6: {
        if (county_ptr->border_count < 20) {
          Border *new_border = NULL;
          if (allocate(sizeof(Border), 0, (void **)&new_border) == 0) {
            printf("Lat Start: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
            new_border->lat_start = atof(input_buffer);

            printf("Long Start: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
            new_border->long_start = atof(input_buffer);

            printf("Lat End: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
            new_border->lat_end = atof(input_buffer);

            printf("Long End: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
            new_border->long_end = atof(input_buffer);

            county_ptr->borders[county_ptr->border_count++] = new_border;
          } else {
            printf("!!Failed to allocate border\n");
          }
        } else {
          printf("!!Max borders reached\n");
        }
        break;
      }
      case 7: {
        int city_idx = -1;
        for (int i = 0; i < 20; i++) {
          if (county_ptr->cities[i] == NULL) {
            city_idx = i;
            break;
          }
        }

        if (city_idx == -1) {
          printf("!!Max cities reached\n");
        } else {
          City *new_city = NULL;
          if (allocate(sizeof(City), 0, (void **)&new_city) == 0) {
            initCity(new_city);
            county_ptr->cities[city_idx] = new_city;
            printf("New City Name: ");
            memset(input_buffer, 0, sizeof(input_buffer));
            receive_until(input_buffer, '\n', sizeof(new_city->name) - 1);
            int i = 0;
            while (i < sizeof(new_city->name) - 1 && isalnum((unsigned char)input_buffer[i])) {
              new_city->name[i] = input_buffer[i];
              i++;
            }
            new_city->name[i] = '\0';
            county_ptr->city_count++;
          } else {
            printf("!!Failed to allocate city\n");
          }
        }
        break;
      }
      case 8: {
        printf("Cities:\n");
        bool cities_found = false;
        for (int i = 0; i < 20; i++) {
          if (county_ptr->cities[i] != NULL) {
            printf("%d) %s\n", i + 1, county_ptr->cities[i]->name);
            cities_found = true;
          }
        }
        if (!cities_found) {
            printf("No cities to select.\n");
            break;
        }
        printf("\n-> ");
        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
        int selected_city_idx = atoi(input_buffer);

        if (selected_city_idx < 1 || selected_city_idx > 20 || county_ptr->cities[selected_city_idx - 1] == NULL) {
          printf("Invalid choice\n");
        } else {
          if (cityMenu(county_ptr->cities[selected_city_idx - 1]) == 0) {
            county_ptr->cities[selected_city_idx - 1] = NULL;
            county_ptr->city_count--;
          }
        }
        break;
      }
      case 9:
        freeCounty(county_ptr);
        result = 0; // Indicate county deleted and exit menu
        break;
      case 10:
        result = 1; // Indicate exit menu
        break;
    }
  } while (choice != 9 && choice != 10);

  return result;
}

// Function: freeCounty
void freeCounty(County *county_ptr) {
  if (county_ptr != NULL) {
    for (int i = 0; i < county_ptr->border_count; i++) {
      if (county_ptr->borders[i] != NULL) {
        deallocate(county_ptr->borders[i], sizeof(Border));
        county_ptr->borders[i] = NULL;
      }
    }
    for (int i = 0; i < 20; i++) {
      if (county_ptr->cities[i] != NULL) {
        freeCity(county_ptr->cities[i]);
        county_ptr->cities[i] = NULL;
      }
    }
    deallocate(county_ptr, sizeof(County));
  }
}

// Function: initCounty
County *initCounty(County *county_ptr) {
  if (county_ptr != NULL) {
    memset(county_ptr->name, 0, sizeof(county_ptr->name));
    memset(county_ptr->seat, 0, sizeof(county_ptr->seat));

    county_ptr->population = -1;
    county_ptr->area = -1.0;
    county_ptr->density = -1.0;

    county_ptr->city_count = 0;
    for (int i = 0; i < 20; i++) {
      county_ptr->cities[i] = NULL;
    }

    county_ptr->border_count = 0;
    for (int i = 0; i < 20; i++) {
      county_ptr->borders[i] = NULL;
    }
  }
  return county_ptr;
}

// Function: printCountyInfo
void printCountyInfo(County *county_ptr) {
  if (county_ptr != NULL) {
    printf("\t\t\tCounty: ");
    if (county_ptr->name[0] == '\0') {
      printf("Unknown\n");
    } else {
      printf("%s\n", county_ptr->name);
    }
    printf("\t\t\t\tSeat: ");
    if (county_ptr->seat[0] == '\0') {
      printf("Unknown\n");
    } else {
      printf("%s\n", county_ptr->seat);
    }
    if (county_ptr->population >= 0) {
      printf("\t\t\t\tPopulation: %d\n", county_ptr->population);
    }
    if (county_ptr->area >= 0.0) {
      printf("\t\t\t\tArea: %f\n", county_ptr->area);
    }
    if (county_ptr->density >= 0.0) {
      printf("\t\t\t\tDensity: %f\n", county_ptr->density);
    }
    for (int i = 0; i < county_ptr->border_count; i++) {
      if (county_ptr->borders[i] != NULL) {
        printf("\t\t\t\tBorder: %f %f %f %f\n",
               county_ptr->borders[i]->lat_start,
               county_ptr->borders[i]->long_start,
               county_ptr->borders[i]->lat_end,
               county_ptr->borders[i]->long_end);
      }
    }
    for (int i = 0; i < 20; i++) {
      if (county_ptr->cities[i] != NULL) {
        printCityInfo(); // Original code calls printCityInfo() without arguments
      }
    }
  }
}

// Function: countyTopLevel
County *countyTopLevel(ParserState *parser_state) {
  County *new_county = NULL;
  char *element_name = NULL;
  int current_pos_on_error; // Stores parser_state->current_index for error reporting
  int start_idx, end_idx;
  bool parsing_success = false;

  if (parser_state == NULL) {
    return NULL;
  }

  skipWhiteSpace(parser_state);
  current_pos_on_error = parser_state->current_index; // Save current index

  if (atChar(parser_state, '{')) {
    incChar(parser_state);
    skipWhiteSpace(parser_state);
    getIndex(parser_state, &start_idx);
    end_idx = skipAlpha(parser_state);

    if (end_idx != -1 && start_idx != end_idx) {
      char *tagName = copyData(parser_state, start_idx, end_idx);
      if (tagName != NULL) {
        if (strcmp(tagName, "County") == 0) {
          deallocate(tagName, strlen(tagName) + 1);
          skipWhiteSpace(parser_state);
          if (atChar(parser_state, '}')) {
            incChar(parser_state);
            if (allocate(sizeof(County), 0, (void **)&new_county) == 0 && new_county != NULL) {
              initCounty(new_county);
              parsing_success = true; // Assume success initially for inner loop
              element_name = pullNextElementName(parser_state);
              while (element_name != NULL && parsing_success) {
                int enum_val = elementNameToEnum(element_name);
                deallocate(element_name, strlen(element_name) + 1);
                element_name = NULL;

                switch (enum_val) {
                  default:
                    printf("!!Element not allowed\n");
                    parsing_success = false;
                    break;
                  case 1: { // Name
                    char *name_str = extractName(parser_state);
                    if (name_str == NULL) { parsing_success = false; break; }
                    strncpy(new_county->name, name_str, sizeof(new_county->name) - 1);
                    new_county->name[sizeof(new_county->name) - 1] = '\0';
                    deallocate(name_str, strlen(name_str) + 1);
                    break;
                  }
                  case 10: { // Population
                    new_county->population = extractPopulation(parser_state);
                    if (new_county->population < 0) { parsing_success = false; break; }
                    break;
                  }
                  case 14: { // Border
                    if (new_county->border_count >= 20) { parsing_success = false; break; }
                    Border *new_border_ptr = (Border *)extractBorder(parser_state);
                    if (new_border_ptr == NULL) { parsing_success = false; break; }
                    new_county->borders[new_county->border_count++] = new_border_ptr;
                    break;
                  }
                  case 19: { // Seat
                    char *seat_str = extractSeat(parser_state);
                    if (seat_str == NULL) { parsing_success = false; break; }
                    strncpy(new_county->seat, seat_str, sizeof(new_county->seat) - 1);
                    new_county->seat[sizeof(new_county->seat) - 1] = '\0';
                    deallocate(seat_str, strlen(seat_str) + 1);
                    break;
                  }
                  case 20: { // Area
                    new_county->area = extractArea(parser_state);
                    if (new_county->area < 0.0) { parsing_success = false; break; }
                    break;
                  }
                  case 21: { // Density
                    new_county->density = extractDensity(parser_state);
                    if (new_county->density < 0.0) { parsing_success = false; break; }
                    break;
                  }
                  case 22: { // City
                    if (new_county->city_count >= 20) { parsing_success = false; break; }
                    City *new_city_ptr = (City *)cityTopLevel(parser_state);
                    if (new_city_ptr == NULL) { parsing_success = false; break; }
                    new_county->cities[new_county->city_count++] = new_city_ptr;
                    break;
                  }
                }
                if (parsing_success) {
                    getIndex(parser_state, &current_pos_on_error);
                    element_name = pullNextElementName(parser_state);
                } else {
                    if (element_name) deallocate(element_name, strlen(element_name) + 1);
                    element_name = NULL; // Stop loop
                }
              }

              if (parsing_success) {
                skipWhiteSpace(parser_state);
                if (atChar(parser_state, '{')) {
                  skipLength(parser_state, 1);
                  skipWhiteSpace(parser_state);
                  if (atChar(parser_state, '#')) {
                    skipLength(parser_state, 1);
                    getIndex(parser_state, &start_idx);
                    end_idx = skipAlpha(parser_state);
                    if (start_idx != end_idx && end_idx != -1) {
                      char *closing_tag = copyData(parser_state, start_idx, end_idx);
                      if (closing_tag != NULL) {
                        if (strcmp(closing_tag, "County") == 0) {
                          deallocate(closing_tag, strlen(closing_tag) + 1);
                          skipWhiteSpace(parser_state);
                          if (atChar(parser_state, '}')) {
                            incChar(parser_state);
                            return new_county; // Success
                          }
                        } else {
                          deallocate(closing_tag, strlen(closing_tag) + 1);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        } else {
          deallocate(tagName, strlen(tagName) + 1);
        }
      }
    }
  }

  if (new_county != NULL) {
    freeCounty(new_county);
    new_county = NULL;
  }
  printf("!!Error at: %s\n", parser_state->input_string + current_pos_on_error);
  parser_state->current_index = current_pos_on_error;
  return NULL;
}

// Function: checkId
int checkId(ParserState *parser_state, char *expected_id, int check_hash) {
  if (parser_state == NULL) return 0;

  int original_index = parser_state->current_index;
  skipWhiteSpace(parser_state);

  if (!atChar(parser_state, '{')) { parser_state->current_index = original_index; return 0; }
  incChar(parser_state);

  if (check_hash) {
    skipWhiteSpace(parser_state);
    if (!atChar(parser_state, '#')) { parser_state->current_index = original_index; return 0; }
    incChar(parser_state);
  }

  int start_idx = parser_state->current_index;
  skipWhiteSpace(parser_state);
  start_idx = parser_state->current_index; // Update start_idx after skipping whitespace
  int end_idx = skipAlpha(parser_state);

  if (end_idx == -1 || start_idx == end_idx) { parser_state->current_index = original_index; return 0; }

  char *extracted_id = copyData(parser_state, start_idx, end_idx);
  if (extracted_id == NULL) { parser_state->current_index = original_index; return 0; }

  bool match = (strcmp(extracted_id, expected_id) == 0);
  deallocate(extracted_id, strlen(extracted_id) + 1);

  if (!match) { parser_state->current_index = original_index; return 0; }

  skipWhiteSpace(parser_state);
  if (parser_state->input_string[parser_state->current_index] == '}') {
    incChar(parser_state);
    return 1;
  }

  parser_state->current_index = original_index;
  return 0;
}

// Function: extractDensity
long double extractDensity(ParserState *parser_state) {
  long double density = -1.0L;
  int original_idx;
  getIndex(parser_state, &original_idx);

  if (checkId(parser_state, "Density", 0)) {
    int start_idx = parser_state->current_index;
    skipWhiteSpace(parser_state);
    start_idx = parser_state->current_index; // Update start_idx after skipping whitespace
    int end_idx = skipFloat(parser_state);

    if (end_idx != -1 && start_idx != end_idx) {
      char *num_str = copyData(parser_state, start_idx, end_idx);
      if (num_str != NULL) {
        density = atof(num_str);
        deallocate(num_str, strlen(num_str) + 1);

        if (checkId(parser_state, "Density", 1)) {
          return density;
        }
      }
    }
  }

  parser_state->current_index = original_idx;
  return -1.0L;
}

// Function: extractArea
long double extractArea(ParserState *parser_state) {
  long double area = -1.0L;
  int original_idx;
  getIndex(parser_state, &original_idx);

  if (checkId(parser_state, "Area", 0)) { // Check for {Area}
    int start_idx = parser_state->current_index;
    skipWhiteSpace(parser_state);
    start_idx = parser_state->current_index; // Update start_idx
    int end_idx = skipFloat(parser_state);

    if (end_idx != -1 && start_idx != end_idx) {
      char *num_str = copyData(parser_state, start_idx, end_idx);
      if (num_str != NULL) {
        area = atof(num_str);
        deallocate(num_str, strlen(num_str) + 1);

        if (checkId(parser_state, "Area", 1)) { // Check for {#Area}
          return area;
        }
      }
    }
  }

  parser_state->current_index = original_idx;
  return -1.0L;
}

// Function: extractSeat
char *extractSeat(ParserState *parser_state) {
  char *seat_name = NULL;
  int original_idx;
  getIndex(parser_state, &original_idx);

  if (checkId(parser_state, "Seat", 0)) { // Check for {Seat}
    int start_idx = parser_state->current_index;
    skipWhiteSpace(parser_state);
    start_idx = parser_state->current_index; // Update start_idx
    int end_idx = skipAlpha(parser_state);

    if (end_idx != -1 && start_idx != end_idx) {
      seat_name = copyData(parser_state, start_idx, end_idx);
      if (seat_name != NULL) {
        if (checkId(parser_state, "Seat", 1)) { // Check for {#Seat}
          return seat_name;
        }
        deallocate(seat_name, strlen(seat_name) + 1);
        seat_name = NULL;
      }
    }
  }

  parser_state->current_index = original_idx;
  return NULL;
}

// Parser related Stubs (continued)
char *pullNextElementName(ParserState *parser_state) {
    skipWhiteSpace(parser_state);
    int original_idx = parser_state->current_index;
    if (atChar(parser_state, '{')) {
        incChar(parser_state);
        skipWhiteSpace(parser_state);
        int start_idx, end_idx;
        getIndex(parser_state, &start_idx);
        end_idx = skipAlpha(parser_state);
        if (end_idx != -1 && start_idx != end_idx) {
             char *tag_name = copyData(parser_state, start_idx, end_idx);
             skipWhiteSpace(parser_state);
             if (atChar(parser_state, '}')) {
                 incChar(parser_state);
                 return tag_name;
             }
             deallocate(tag_name, strlen(tag_name) + 1);
        }
        parser_state->current_index = original_idx; // Rollback if not a valid tag
    }
    return NULL;
}

int elementNameToEnum(char *element_name) {
    if (strcmp(element_name, "Name") == 0) return 1;
    if (strcmp(element_name, "Population") == 0) return 10;
    if (strcmp(element_name, "Border") == 0) return 14;
    if (strcmp(element_name, "Seat") == 0) return 19;
    if (strcmp(element_name, "Area") == 0) return 20;
    if (strcmp(element_name, "Density") == 0) return 21;
    if (strcmp(element_name, "City") == 0) return 22;
    return 0; // Default/unknown
}

char *extractName(ParserState *parser_state) {
    char *name_str = NULL;
    int original_idx;
    getIndex(parser_state, &original_idx);

    if (checkId(parser_state, "Name", 0)) {
        int start_idx = parser_state->current_index;
        skipWhiteSpace(parser_state);
        start_idx = parser_state->current_index;
        int end_idx = skipAlpha(parser_state);
        if (end_idx != -1 && start_idx != end_idx) {
            name_str = copyData(parser_state, start_idx, end_idx);
            if (name_str != NULL) {
                if (checkId(parser_state, "Name", 1)) {
                    return name_str;
                }
                deallocate(name_str, strlen(name_str) + 1);
            }
        }
    }
    parser_state->current_index = original_idx;
    return NULL;
}

undefined4 extractPopulation(ParserState *parser_state) {
    int population = -1;
    int original_idx;
    getIndex(parser_state, &original_idx);

    if (checkId(parser_state, "Population", 0)) {
        int start_idx = parser_state->current_index;
        skipWhiteSpace(parser_state);
        start_idx = parser_state->current_index;
        int end_idx = skipFloat(parser_state);
        if (end_idx != -1 && start_idx != end_idx) {
            char *num_str = copyData(parser_state, start_idx, end_idx);
            if (num_str != NULL) {
                population = atoi(num_str);
                deallocate(num_str, strlen(num_str) + 1);
                if (checkId(parser_state, "Population", 1)) {
                    return population;
                }
            }
        }
    }
    parser_state->current_index = original_idx;
    return -1;
}

undefined4 extractBorder(ParserState *parser_state) {
    Border *new_border = NULL;
    int original_idx;
    getIndex(parser_state, &original_idx);

    if (checkId(parser_state, "Border", 0)) {
        if (allocate(sizeof(Border), 0, (void**)&new_border) == 0) {
            char buffer[30];
            int start_idx, end_idx;

            // Lat Start
            skipWhiteSpace(parser_state); start_idx = parser_state->current_index;
            end_idx = skipFloat(parser_state);
            if (end_idx == -1 || start_idx == end_idx) goto fail_extract_border;
            strncpy(buffer, parser_state->input_string + start_idx, end_idx - start_idx); buffer[end_idx - start_idx] = '\0';
            new_border->lat_start = atof(buffer);

            // Long Start
            skipWhiteSpace(parser_state); start_idx = parser_state->current_index;
            end_idx = skipFloat(parser_state);
            if (end_idx == -1 || start_idx == end_idx) goto fail_extract_border;
            strncpy(buffer, parser_state->input_string + start_idx, end_idx - start_idx); buffer[end_idx - start_idx] = '\0';
            new_border->long_start = atof(buffer);

            // Lat End
            skipWhiteSpace(parser_state); start_idx = parser_state->current_index;
            end_idx = skipFloat(parser_state);
            if (end_idx == -1 || start_idx == end_idx) goto fail_extract_border;
            strncpy(buffer, parser_state->input_string + start_idx, end_idx - start_idx); buffer[end_idx - start_idx] = '\0';
            new_border->lat_end = atof(buffer);

            // Long End
            skipWhiteSpace(parser_state); start_idx = parser_state->current_index;
            end_idx = skipFloat(parser_state);
            if (end_idx == -1 || start_idx == end_idx) goto fail_extract_border;
            strncpy(buffer, parser_state->input_string + start_idx, end_idx - start_idx); buffer[end_idx - start_idx] = '\0';
            new_border->long_end = atof(buffer);

            if (checkId(parser_state, "Border", 1)) {
                return (undefined4)new_border;
            }
        }
    }

fail_extract_border: // Using goto here for local cleanup, as it's common in error handling.
    if (new_border) deallocate(new_border, sizeof(Border));
    parser_state->current_index = original_idx;
    return 0; // Return NULL (0) on failure
}

undefined4 cityTopLevel(ParserState *parser_state) {
    City *new_city = NULL;
    int original_idx;
    getIndex(parser_state, &original_idx);

    if (checkId(parser_state, "City", 0)) {
        if (allocate(sizeof(City), 0, (void**)&new_city) == 0) {
            initCity(new_city);
            char *city_name_str = extractName(parser_state);
            if (city_name_str) {
                strncpy(new_city->name, city_name_str, sizeof(new_city->name) - 1);
                new_city->name[sizeof(new_city->name) - 1] = '\0';
                deallocate(city_name_str, strlen(city_name_str) + 1);

                if (checkId(parser_state, "City", 1)) {
                    return (undefined4)new_city;
                }
            }
        }
    }

    if (new_city) deallocate(new_city, sizeof(City));
    parser_state->current_index = original_idx;
    return 0;
}

void skipLength(ParserState *parser_state, int length) {
    if (parser_state && parser_state->input_string) {
        parser_state->current_index += length;
    }
}

int skipFloat(ParserState *parser_state) {
    if (!parser_state || !parser_state->input_string) return -1;
    int start_idx = parser_state->current_index;
    bool has_digit = false;
    
    // Check for optional sign
    if (parser_state->input_string[parser_state->current_index] == '-' ||
        parser_state->input_string[parser_state->current_index] == '+') {
        parser_state->current_index++;
    }

    while (isdigit((unsigned char)parser_state->input_string[parser_state->current_index])) {
        parser_state->current_index++;
        has_digit = true;
    }
    if (parser_state->input_string[parser_state->current_index] == '.') {
        parser_state->current_index++;
        while (isdigit((unsigned char)parser_state->input_string[parser_state->current_index])) {
            parser_state->current_index++;
            has_digit = true;
        }
    }
    return (has_digit) ? parser_state->current_index : -1;
}