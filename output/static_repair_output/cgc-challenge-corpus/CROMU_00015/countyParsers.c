#include <stdio.h>    // For printf
#include <stdlib.h>   // For atoi, atof, malloc, free
#include <string.h>   // For memset, strcmp, strlen, strncpy, strcspn
#include <ctype.h>    // For isalpha, isalnum, isdigit

// --- Forward Declarations for custom types and functions ---

// Assuming County structure based on memory offsets and usage
typedef struct County {
    char name[20];       // Offset 0x00
    char seat[20];       // Offset 0x14
    int population;      // Offset 0x28
    double area;         // Offset 0x2c
    double density;      // Offset 0x34
    int num_cities;      // Offset 0x3c
    void *cities[20];    // Offset 0x40 (array of pointers to City objects)
    int num_borders;     // Offset 0x90
    double *borders[20]; // Offset 0x94 (array of pointers to 4 doubles for border coordinates)
    // Total size 0xE4 (228 bytes)
} County;

// Assuming ParseState structure based on how parsing functions use int* param_1
// param_1[0] was *param_1 (char* data), param_1[2] was current_index
typedef struct ParseState {
    char *data;
    int current_index;
    // int length; // Not explicitly used but good practice
} ParseState;

// External custom functions (definitions assumed to be in another compilation unit)
void *allocate(size_t size, int unknown_param, void **out_ptr);
void deallocate(void *ptr, size_t size);
void receive_until(char *buffer, int delimiter, int max_len);
void initCity(int city_ptr); // int city_ptr is assumed to be City* cast to int
int cityMenu(int city_ptr);  // int city_ptr is assumed to be City* cast to int
void freeCity(int city_ptr); // int city_ptr is assumed to be City* cast to int
void printCityInfo();        // Assumed to print info for a specific city, but original code calls without argument
void printCountyInfo(County *county); // Type changed from char* to County*

// Parsing related functions
void skipWhiteSpace(ParseState *ps);
int atChar(ParseState *ps, char c);
void incChar(ParseState *ps);
void getIndex(ParseState *ps, int *index_out);
int skipAlpha(ParseState *ps);
char *copyData(ParseState *ps, int start_idx, int end_idx);
char *pullNextElementName(ParseState *ps);
int elementNameToEnum(char *element_name);
char *extractName(ParseState *ps);
int extractPopulation(ParseState *ps);
int extractBorder(ParseState *ps); // Returns an int (pointer to double array cast to int)
char *extractSeat(ParseState *ps);
double extractArea(ParseState *ps);
double extractDensity(ParseState *ps);
void skipLength(ParseState *ps, int length);
int skipFloat(ParseState *ps);
int checkId(ParseState *ps, char *id_name, int type); // type 0 for initial '{ID}', type 1 for closing '{#ID}'
int cityTopLevel(ParseState *ps); // Returns an int (pointer to City object cast to int)

// --- Dummy Implementations for compilation (will be removed in final output) ---
// In a real scenario, these would be linked from another compilation unit.
#ifdef DUMMY_FUNCTIONS_ENABLED
void *allocate(size_t size, int unknown_param, void **out_ptr) {
    *out_ptr = malloc(size);
    return (*out_ptr == NULL) ? (void*)1 : (void*)0; // Mimic return 0 on success, non-zero on failure
}
void deallocate(void *ptr, size_t size) { free(ptr); }
void receive_until(char *buffer, int delimiter, int max_len) {
    if (fgets(buffer, max_len, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline if present
    } else {
        buffer[0] = 0; // Empty string on error
    }
}
void initCity(int city_ptr) { /* dummy */ }
int cityMenu(int city_ptr) { printf("City Menu (dummy) for city %d\n", city_ptr); return 0; /* dummy: delete city */ }
void freeCity(int city_ptr) { /* dummy */ }
void printCityInfo() { printf("\t\t\t\tCity Info (dummy)\n"); }

// Parsing dummies (basic, might not match original logic exactly without full context)
void skipWhiteSpace(ParseState *ps) {
    while (ps->data[ps->current_index] && (ps->data[ps->current_index] == ' ' || ps->data[ps->current_index] == '\t' || ps->data[ps->current_index] == '\n')) {
        ps->current_index++;
    }
}
int atChar(ParseState *ps, char c) {
    return ps->data[ps->current_index] == c;
}
void incChar(ParseState *ps) {
    if (ps->data[ps->current_index]) ps->current_index++;
}
void getIndex(ParseState *ps, int *index_out) {
    *index_out = ps->current_index;
}
int skipAlpha(ParseState *ps) {
    int start = ps->current_index;
    while (ps->data[ps->current_index] && isalpha(ps->data[ps->current_index])) {
        ps->current_index++;
    }
    return (ps->current_index == start) ? -1 : ps->current_index;
}
char *copyData(ParseState *ps, int start_idx, int end_idx) {
    if (start_idx < 0 || end_idx <= start_idx) return NULL;
    int len = end_idx - start_idx;
    char *buf = malloc(len + 1);
    if (!buf) return NULL;
    strncpy(buf, ps->data + start_idx, len);
    buf[len] = '\0';
    return buf;
}
char *pullNextElementName(ParseState *ps) { return NULL; /* dummy */ }
enum ElementType {
    ELEMENT_UNKNOWN = 0, ELEMENT_NAME = 1, ELEMENT_POPULATION = 10, ELEMENT_BORDER = 14,
    ELEMENT_SEAT = 19, ELEMENT_AREA = 20, ELEMENT_DENSITY = 21, ELEMENT_CITY = 22
};
int elementNameToEnum(char *element_name) { return ELEMENT_UNKNOWN; /* dummy */ }
char *extractName(ParseState *ps) { return NULL; /* dummy */ }
int extractPopulation(ParseState *ps) { return -1; /* dummy */ }
int extractBorder(ParseState *ps) { return 0; /* dummy */ }
void skipLength(ParseState *ps, int length) { ps->current_index += length; }
int skipFloat(ParseState *ps) { return -1; /* dummy */ }
int cityTopLevel(ParseState *ps) { return 0; /* dummy */ }
#endif
// --- End of Dummy Implementations ---


// Function: countyMenu
int countyMenu(County *county) {
    char input_buffer[30];
    int choice;

    if (county == NULL) {
        return 0;
    }

    int result = 3;
    int continue_menu = 1;

    do {
        printf("County: %s\n", county->name);
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

        memset(input_buffer, 0, sizeof(input_buffer));
        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
        choice = atoi(input_buffer);

        switch (choice) {
            default:
                printf("Invalid choice\n");
                break;
            case 1:
                printCountyInfo(county);
                break;
            case 2: // Set Seat
                printf("\n-> ");
                receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                int k = 0;
                while (k < sizeof(county->seat) - 1 && isalpha((unsigned char)input_buffer[k])) {
                    county->seat[k] = input_buffer[k];
                    k++;
                }
                county->seat[k] = '\0';
                break;
            case 3: // Set Population
                printf("\n-> ");
                receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                county->population = atoi(input_buffer);
                break;
            case 4: // Set Area
                printf("\n-> ");
                receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                county->area = atof(input_buffer);
                break;
            case 5: // Set Density
                printf("\n-> ");
                receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                county->density = atof(input_buffer);
                break;
            case 6: { // Add Border
                if (county->num_borders < 20) {
                    double *new_border_coords = NULL;
                    if (allocate(4 * sizeof(double), 0, (void**)&new_border_coords) == 0) {
                        printf("Lat Start: ");
                        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                        new_border_coords[0] = atof(input_buffer);

                        printf("Long Start: ");
                        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                        new_border_coords[1] = atof(input_buffer);

                        printf("Lat End: ");
                        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                        new_border_coords[2] = atof(input_buffer);

                        printf("Long End: ");
                        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                        new_border_coords[3] = atof(input_buffer);

                        county->borders[county->num_borders++] = new_border_coords;
                    } else {
                        printf("!!Failed to allocate memory for border.\n");
                    }
                } else {
                    printf("!!Max borders reached\n");
                }
                break;
            }
            case 7: { // Add City
                int city_idx = 0;
                while (city_idx < 20 && county->cities[city_idx] != NULL) {
                    city_idx++;
                }

                if (city_idx == 20) {
                    printf("!!Max cities reached\n");
                } else {
                    int new_city_ptr = 0;
                    if (allocate(0x80, 0, (void**)&new_city_ptr) == 0) {
                        initCity(new_city_ptr);
                        county->cities[city_idx] = (void*)(long)new_city_ptr; // Store as void*
                        printf("New City Name: ");
                        receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);
                        
                        int j = 0;
                        while (j < 0x80 - 1 && isalnum((unsigned char)input_buffer[j])) {
                            *((char *)(long)new_city_ptr + j) = input_buffer[j];
                            j++;
                        }
                        *((char *)(long)new_city_ptr + j) = '\0';
                        county->num_cities++;
                    } else {
                        printf("!!Failed to allocate memory for city.\n");
                    }
                }
                break;
            }
            case 8: { // Select City
                printf("Cities:\n");
                for (int i = 0; i < 20; i++) {
                    if (county->cities[i] != NULL) {
                        printf("%d) %s\n", i + 1, (char*)county->cities[i]);
                    }
                }
                printf("\n-> ");
                memset(input_buffer, 0, sizeof(input_buffer));
                receive_until(input_buffer, '\n', 4); // Max 4 chars + null for selection
                int selected_city_index = atoi(input_buffer);

                if (selected_city_index < 1 || selected_city_index > 20 || county->cities[selected_city_index - 1] == NULL) {
                    printf("Invalid choice\n");
                } else {
                    if (cityMenu((int)(long)county->cities[selected_city_index - 1]) == 0) {
                        freeCity((int)(long)county->cities[selected_city_index - 1]);
                        county->cities[selected_city_index - 1] = NULL;
                        county->num_cities--;
                    }
                }
                break;
            }
            case 9: // Delete County and Exit Menu
                freeCounty(county);
                result = 0;
                continue_menu = 0;
                break;
            case 10: // Exit Menu
                result = 1;
                continue_menu = 0;
                break;
        }
    } while (continue_menu);

    return result;
}

// Function: freeCounty
void freeCounty(County *county) {
    if (county == NULL) {
        return;
    }

    for (int i = 0; i < county->num_borders; i++) {
        if (county->borders[i] != NULL) {
            deallocate(county->borders[i], 4 * sizeof(double)); // 0x20
            county->borders[i] = NULL;
        }
    }

    for (int i = 0; i < 20; i++) {
        if (county->cities[i] != NULL) {
            freeCity((int)(long)county->cities[i]);
            county->cities[i] = NULL;
        }
    }
    deallocate(county, 0xe4);
}

// Function: initCounty
County *initCounty(County *county) {
    if (county == NULL) {
        return NULL;
    }

    // Initialize name and seat to all zeros.
    memset(county->name, 0, sizeof(county->name));
    memset(county->seat, 0, sizeof(county->seat));

    // Initialize cities and borders pointers to NULL
    for (int i = 0; i < 20; i++) {
        county->cities[i] = NULL;
        county->borders[i] = NULL;
    }

    // Initialize specific fields
    county->population = -1;
    county->area = -1.0;
    county->density = -1.0;
    county->num_cities = 0;
    county->num_borders = 0;

    return county;
}

// Function: printCountyInfo
void printCountyInfo(County *county) {
    if (county == NULL) {
        return;
    }

    printf("\t\t\tCounty: ");
    if (county->name[0] == '\0') {
        printf("Unknown\n");
    } else {
        printf("%s\n", county->name);
    }

    printf("\t\t\t\tSeat: ");
    if (county->seat[0] == '\0') {
        printf("Unknown\n");
    } else {
        printf("%s\n", county->seat);
    }

    if (county->population >= 0) {
        printf("\t\t\t\tPopulation: %d\n", county->population);
    }

    if (county->area >= 0.0) {
        printf("\t\t\t\tArea: %f\n", county->area);
    }

    if (county->density >= 0.0) {
        printf("\t\t\t\tDensity: %f\n", county->density);
    }

    for (int i = 0; i < county->num_borders; i++) {
        if (county->borders[i] != NULL) {
            printf("\t\t\t\tBorder: %f %f %f %f\n",
                   county->borders[i][0],
                   county->borders[i][1],
                   county->borders[i][2],
                   county->borders[i][3]);
        }
    }

    for (int i = 0; i < 20; i++) {
        if (county->cities[i] != NULL) {
            printCityInfo(); // Assuming printCityInfo handles printing for the current city
        }
    }
}

// Function: countyTopLevel
County *countyTopLevel(ParseState *ps) {
    County *new_county = NULL;
    int current_parse_pos_on_error = 0;
    char *element_name = NULL;
    int element_enum;
    size_t name_len;

    if (ps == NULL) {
        return NULL;
    }

    skipWhiteSpace(ps);
    getIndex(ps, &current_parse_pos_on_error); // Store current index for error recovery

    if (!atChar(ps, '{')) {
        return NULL;
    }
    incChar(ps);
    skipWhiteSpace(ps);

    // Check for "County" identifier
    int start_idx, end_idx;
    getIndex(ps, &start_idx);
    end_idx = skipAlpha(ps);
    if (end_idx == -1 || start_idx == end_idx) {
        ps->current_index = current_parse_pos_on_error;
        return NULL;
    }

    char *id_str = copyData(ps, start_idx, end_idx);
    if (id_str == NULL || strcmp(id_str, "County") != 0) {
        if (id_str) deallocate(id_str, strlen(id_str) + 1);
        ps->current_index = current_parse_pos_on_error;
        return NULL;
    }
    deallocate(id_str, strlen(id_str) + 1);

    skipWhiteSpace(ps);
    if (!atChar(ps, '}')) {
        ps->current_index = current_parse_pos_on_error;
        return NULL;
    }
    incChar(ps);
    skipWhiteSpace(ps);

    if (allocate(sizeof(County), 0, (void**)&new_county) != 0) {
        return NULL; // Allocation failed
    }
    initCounty(new_county);

    int parse_success = 1;
    element_name = pullNextElementName(ps);

    while (element_name != NULL) {
        element_enum = elementNameToEnum(element_name);
        name_len = strlen(element_name);
        deallocate(element_name, name_len + 1);
        element_name = NULL;

        switch (element_enum) {
            case 1: // ELEMENT_NAME
                char *county_name = extractName(ps);
                if (county_name == NULL) { parse_success = 0; break; }
                memset(new_county->name, 0, sizeof(new_county->name));
                strncpy(new_county->name, county_name, sizeof(new_county->name) - 1);
                deallocate(county_name, strlen(county_name) + 1);
                break;
            case 10: // ELEMENT_POPULATION
                new_county->population = extractPopulation(ps);
                if (new_county->population < 0) { parse_success = 0; break; }
                break;
            case 14: { // ELEMENT_BORDER
                if (new_county->num_borders >= 20) { printf("!!Max borders reached\n"); parse_success = 0; break; }
                double *border_coords = (double*)(long)extractBorder(ps);
                if (border_coords == NULL) { parse_success = 0; break; }
                new_county->borders[new_county->num_borders++] = border_coords;
                break;
            }
            case 19: // ELEMENT_SEAT
                char *county_seat = extractSeat(ps);
                if (county_seat == NULL) { parse_success = 0; break; }
                memset(new_county->seat, 0, sizeof(new_county->seat));
                strncpy(new_county->seat, county_seat, sizeof(new_county->seat) - 1);
                deallocate(county_seat, strlen(county_seat) + 1);
                break;
            case 20: // ELEMENT_AREA
                new_county->area = extractArea(ps);
                if (new_county->area < 0.0) { parse_success = 0; break; }
                break;
            case 21: // ELEMENT_DENSITY
                new_county->density = extractDensity(ps);
                if (new_county->density < 0.0) { parse_success = 0; break; }
                break;
            case 22: { // ELEMENT_CITY
                if (new_county->num_cities >= 20) { printf("!!Max cities reached\n"); parse_success = 0; break; }
                int city_ptr = cityTopLevel(ps);
                if (city_ptr == 0) { parse_success = 0; break; }
                new_county->cities[new_county->num_cities++] = (void*)(long)city_ptr;
                break;
            }
            default:
                printf("!!Element not allowed\n");
                parse_success = 0;
                break;
        }

        if (!parse_success) {
            break;
        }
        getIndex(ps, &current_parse_pos_on_error);
        element_name = pullNextElementName(ps);
    }

    if (parse_success) {
        skipWhiteSpace(ps);
        if (atChar(ps, '{')) {
            skipLength(ps, 1);
            skipWhiteSpace(ps);
            if (atChar(ps, '#')) {
                skipLength(ps, 1);
                getIndex(ps, &start_idx);
                end_idx = skipAlpha(ps);
                if (start_idx != end_idx && end_idx != -1) {
                    id_str = copyData(ps, start_idx, end_idx);
                    if (id_str != NULL && strcmp(id_str, "County") == 0) {
                        deallocate(id_str, strlen(id_str) + 1);
                        skipWhiteSpace(ps);
                        if (atChar(ps, '}')) {
                            incChar(ps);
                            return new_county; // Success
                        }
                    } else {
                        if (id_str) deallocate(id_str, strlen(id_str) + 1);
                    }
                }
            }
        }
    }

    // Error path: clean up and report
    if (new_county != NULL) {
        freeCounty(new_county);
        new_county = NULL;
    }
    printf("!!Error at: %s\n", ps->data + current_parse_pos_on_error);
    ps->current_index = current_parse_pos_on_error;
    return NULL;
}

// Function: checkId
int checkId(ParseState *ps, char *id_name, int type) {
    if (ps == NULL) {
        return 0;
    }
    int initial_index = ps->current_index;

    skipWhiteSpace(ps);
    if (!atChar(ps, '{')) {
        ps->current_index = initial_index;
        return 0;
    }
    incChar(ps);

    if (type == 1) { // For closing tag like {#ID}
        skipWhiteSpace(ps);
        if (!atChar(ps, '#')) {
            ps->current_index = initial_index;
            return 0;
        }
        incChar(ps);
    }

    int start_idx, end_idx;
    skipWhiteSpace(ps);
    getIndex(ps, &start_idx);
    end_idx = skipAlpha(ps);

    if (end_idx == -1 || start_idx == end_idx) {
        ps->current_index = initial_index;
        return 0;
    }

    char *id_str = copyData(ps, start_idx, end_idx);
    if (id_str == NULL) {
        ps->current_index = initial_index;
        return 0;
    }

    int match = (strcmp(id_str, id_name) == 0);
    deallocate(id_str, strlen(id_str) + 1);

    if (match) {
        skipWhiteSpace(ps);
        if (atChar(ps, '}')) {
            incChar(ps);
            return 1; // Success
        }
    }

    ps->current_index = initial_index; // Rollback on failure
    return 0;
}

// Function: extractDensity
double extractDensity(ParseState *ps) {
    double density_val = -1.0;
    char *num_str = NULL;
    int initial_index = 0;

    if (ps == NULL) {
        return -1.0;
    }

    getIndex(ps, &initial_index);

    if (!checkId(ps, "Density", 0)) {
        ps->current_index = initial_index;
        return -1.0;
    }

    int start_idx, end_idx;
    skipWhiteSpace(ps);
    start_idx = ps->current_index;
    end_idx = skipFloat(ps);

    if (end_idx == -1 || start_idx == end_idx) {
        ps->current_index = initial_index;
        return -1.0;
    }

    num_str = copyData(ps, start_idx, end_idx);
    if (num_str == NULL) {
        ps->current_index = initial_index;
        return -1.0;
    }

    density_val = atof(num_str);
    deallocate(num_str, (size_t)(end_idx - start_idx) + 1);
    num_str = NULL;

    if (!checkId(ps, "Density", 1)) {
        ps->current_index = initial_index;
        return -1.0;
    }

    return density_val;
}

// Function: extractArea
double extractArea(ParseState *ps) {
    double area_val = -1.0;
    char *num_str = NULL;
    int initial_index = 0;

    if (ps == NULL) {
        return -1.0;
    }

    getIndex(ps, &initial_index);

    if (!checkId(ps, "Area", 0)) {
        ps->current_index = initial_index;
        return -1.0;
    }

    int start_idx, end_idx;
    skipWhiteSpace(ps);
    start_idx = ps->current_index;
    end_idx = skipFloat(ps);

    if (end_idx == -1 || start_idx == end_idx) {
        ps->current_index = initial_index;
        return -1.0;
    }

    num_str = copyData(ps, start_idx, end_idx);
    if (num_str == NULL) {
        ps->current_index = initial_index;
        return -1.0;
    }

    area_val = atof(num_str);
    deallocate(num_str, (size_t)(end_idx - start_idx) + 1);
    num_str = NULL;

    if (!checkId(ps, "Area", 1)) {
        ps->current_index = initial_index;
        return -1.0;
    }

    return area_val;
}

// Function: extractSeat
char *extractSeat(ParseState *ps) {
    char *seat_name = NULL;
    int initial_index = 0;

    if (ps == NULL) {
        return NULL;
    }

    getIndex(ps, &initial_index);

    if (!checkId(ps, "Seat", 0)) {
        ps->current_index = initial_index;
        return NULL;
    }

    int start_idx, end_idx;
    skipWhiteSpace(ps);
    start_idx = ps->current_index;
    end_idx = skipAlpha(ps);

    if (end_idx == -1 || start_idx == end_idx) {
        ps->current_index = initial_index;
        return NULL;
    }

    seat_name = copyData(ps, start_idx, end_idx);
    if (seat_name == NULL) {
        ps->current_index = initial_index;
        return NULL;
    }

    if (!checkId(ps, "Seat", 1)) {
        deallocate(seat_name, strlen(seat_name) + 1);
        ps->current_index = initial_index;
        return NULL;
    }

    return seat_name;
}