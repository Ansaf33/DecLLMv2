#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h> // For bool type

// Define the structs based on the original code's memory access patterns
#define MAX_PLANET_NAME_LEN 19 // +1 for null terminator = 20 bytes
#define MAX_COUNTRIES_PER_PLANET 30
#define MAX_COUNTRY_NAME_LEN 255 // Inferred from allocate(0x218) for Country_t

typedef struct Country {
    char name[MAX_COUNTRY_NAME_LEN + 1]; // Offset 0, size 256 (0x100)
    // Other fields (if any) would follow here. Total size 0x218.
} Country_t;

typedef struct Planet {
    char name[MAX_PLANET_NAME_LEN + 1]; // 0x0 - 0x13
    double period;                      // 0x14
    double orbit_speed;                 // 0x1c
    double aphelion;                    // 0x24
    double perihelion;                  // 0x2c
    double mean_radius;                 // 0x34
    double equatorial_radius;           // 0x3c
    double mass;                        // 0x44
    double gravity;                     // 0x4c
    int population;                     // 0x54
    int num_countries;                  // 0x58
    Country_t *countries[MAX_COUNTRIES_PER_PLANET]; // 0x5c to 0xd3. Total size 0xd4.
} Planet_t;

// Parsing context structure
typedef struct ParseContext {
    const char *input_buffer;
    size_t buffer_len;
    size_t current_idx;
} ParseContext_t;


// --- Dummy Function Declarations (for compilation) ---
// These functions are not part of the original snippet but are called by it.
// They are provided as minimal implementations to allow the snippet to compile.

// In a real scenario, this would read from stdin or a socket
void receive_until(char *buffer, size_t max_len, int terminator) {
    if (fgets(buffer, max_len + 1, stdin) != NULL) {
        // Remove trailing newline if present
        buffer[strcspn(buffer, "\n")] = 0;
    } else {
        buffer[0] = '\0'; // Ensure null-termination on error
    }
}

// Simple malloc/free for dummies. Real allocate/deallocate might involve memory pools.
// The original `allocate` had `param_3` as an `int*` to store the allocated address.
// This dummy uses `void**` for `out_ptr` to be more flexible.
void *allocate(size_t size, int flags, void **out_ptr) {
    void *mem = malloc(size);
    if (out_ptr) *out_ptr = mem; // Store allocated pointer if `out_ptr` is provided
    // flags unused for dummy
    return mem;
}

void deallocate(void *ptr, size_t size) {
    free(ptr);
    // size unused for dummy, but kept for signature matching
}

void initCountry(Country_t *country) {
    if (country) {
        memset(country->name, 0, sizeof(country->name));
        // Initialize other country fields if they existed
    }
}

// Returns 0 to delete country, 1 to exit menu (without delete)
int countryMenu(Country_t *country) {
    printf("Entering Country Menu for %s (dummy)\n", country ? country->name : "NULL");
    // In a real scenario, this would present a menu for country options.
    // Simulate exit without deletion for now.
    return 1;
}

void freeCountry(Country_t *country) {
    if (country) {
        printf("Freeing Country: %s (dummy)\n", country->name);
        deallocate(country, sizeof(Country_t));
    }
}

void printCountryInfo(Country_t *country) {
    if (country) {
        printf("\tCountry: %s (dummy info)\n", country->name);
    }
}

void skipWhiteSpace(ParseContext_t *ctx) {
    if (!ctx || !ctx->input_buffer) return;
    while (ctx->current_idx < ctx->buffer_len && isspace((unsigned char)ctx->input_buffer[ctx->current_idx])) {
        ctx->current_idx++;
    }
}

char *pullNextElementName(ParseContext_t *ctx) {
    if (!ctx || !ctx->input_buffer) return NULL;
    skipWhiteSpace(ctx);
    if (ctx->current_idx >= ctx->buffer_len || ctx->input_buffer[ctx->current_idx] != '{') {
        return NULL; // No more elements or not an opening brace
    }
    size_t initial_idx = ctx->current_idx; // Save for potential reset
    ctx->current_idx++; // Skip '{'
    skipWhiteSpace(ctx);
    if (ctx->current_idx >= ctx->buffer_len || ctx->input_buffer[ctx->current_idx] == '#') {
        ctx->current_idx = initial_idx; // It's a closing tag, not an element
        return NULL;
    }
    size_t start_name = ctx->current_idx;
    while (ctx->current_idx < ctx->buffer_len && isalnum((unsigned char)ctx->input_buffer[ctx->current_idx])) {
        ctx->current_idx++;
    }
    if (start_name == ctx->current_idx) { // No name extracted
        ctx->current_idx = initial_idx;
        return NULL;
    }
    size_t name_len = ctx->current_idx - start_name;
    char *name = (char *)allocate(name_len + 1, 0, NULL);
    if (name) {
        strncpy(name, ctx->input_buffer + start_name, name_len);
        name[name_len] = '\0';
    }
    // Rewind ctx->current_idx to just after '{' for subsequent parsing
    ctx->current_idx = initial_idx + 1; 
    return name; // Caller is responsible for deallocating
}

void getIndex(ParseContext_t *ctx, size_t *index) {
    if (ctx && index) {
        *index = ctx->current_idx;
    }
}

// Maps element name strings to integer "enums"
int elementNameToEnum(const char *name) {
    if (!name) return 0; // Default/Unknown
    if (strcmp(name, "Name") == 0) return 1;
    if (strcmp(name, "Period") == 0) return 2;
    if (strcmp(name, "OrbitSpeed") == 0) return 3;
    if (strcmp(name, "Aphelion") == 0) return 4;
    if (strcmp(name, "Perihelion") == 0) return 5;
    if (strcmp(name, "Radius") == 0) return 6; // Mean Radius
    if (strcmp(name, "ERadius") == 0) return 7; // Equatorial Radius
    if (strcmp(name, "Mass") == 0) return 8;
    if (strcmp(name, "Gravity") == 0) return 9;
    if (strcmp(name, "Population") == 0) return 10;
    if (strcmp(name, "Country") == 0) return 11;
    return 0; // Unknown element
}

bool atChar(ParseContext_t *ctx, char c) {
    if (!ctx || !ctx->input_buffer || ctx->current_idx >= ctx->buffer_len) return false;
    return ctx->input_buffer[ctx->current_idx] == c;
}

int incChar(ParseContext_t *ctx) {
    if (!ctx || !ctx->input_buffer || ctx->current_idx >= ctx->buffer_len) return -1;
    ctx->current_idx++;
    return ctx->current_idx;
}

int skipLength(ParseContext_t *ctx, size_t length) {
    if (!ctx || !ctx->input_buffer || ctx->current_idx + length > ctx->buffer_len) return -1;
    ctx->current_idx += length;
    return ctx->current_idx;
}

size_t skipToNonAlphaNum(ParseContext_t *ctx) {
    if (!ctx || !ctx->input_buffer) return (size_t)-1;
    size_t initial_idx = ctx->current_idx;
    while (ctx->current_idx < ctx->buffer_len && isalnum((unsigned char)ctx->input_buffer[ctx->current_idx])) {
        ctx->current_idx++;
    }
    return (initial_idx == ctx->current_idx) ? (size_t)-1 : ctx->current_idx; // Return new index or -1 if no movement
}

char *copyData(ParseContext_t *ctx, size_t start_idx, size_t end_idx) {
    if (!ctx || !ctx->input_buffer || start_idx > end_idx || end_idx > ctx->buffer_len) return NULL;
    size_t len = end_idx - start_idx;
    char *data = (char *)allocate(len + 1, 0, NULL);
    if (data) {
        strncpy(data, ctx->input_buffer + start_idx, len);
        data[len] = '\0';
    }
    return data;
}

size_t skipFloat(ParseContext_t *ctx) {
    if (!ctx || !ctx->input_buffer) return (size_t)-1;
    size_t initial_idx = ctx->current_idx;
    bool has_digit = false;
    // Handle optional sign
    if (ctx->current_idx < ctx->buffer_len && (ctx->input_buffer[ctx->current_idx] == '-' || ctx->input_buffer[ctx->current_idx] == '+')) {
        ctx->current_idx++;
    }
    // Handle integer part
    while (ctx->current_idx < ctx->buffer_len && isdigit((unsigned char)ctx->input_buffer[ctx->current_idx])) {
        ctx->current_idx++;
        has_digit = true;
    }
    // Handle fractional part
    if (ctx->current_idx < ctx->buffer_len && ctx->input_buffer[ctx->current_idx] == '.') {
        ctx->current_idx++;
        while (ctx->current_idx < ctx->buffer_len && isdigit((unsigned char)ctx->input_buffer[ctx->current_idx])) {
            ctx->current_idx++;
            has_digit = true;
        }
    }
    // Handle exponent part (e.g., 1.23e-4)
    if (ctx->current_idx < ctx->buffer_len && (ctx->input_buffer[ctx->current_idx] == 'e' || ctx->input_buffer[ctx->current_idx] == 'E')) {
        ctx->current_idx++;
        if (ctx->current_idx < ctx->buffer_len && (ctx->input_buffer[ctx->current_idx] == '-' || ctx->input_buffer[ctx->current_idx] == '+')) {
            ctx->current_idx++;
        }
        bool has_exp_digit = false;
        while (ctx->current_idx < ctx->buffer_len && isdigit((unsigned char)ctx->input_buffer[ctx->current_idx])) {
            ctx->current_idx++;
            has_exp_digit = true;
        }
        if (!has_exp_digit) return (size_t)-1; // Exponent sign without digits is invalid
    }
    return (has_digit || (initial_idx != ctx->current_idx && (ctx->input_buffer[initial_idx] == '.' || (ctx->input_buffer[initial_idx] == '-' && ctx->current_idx > initial_idx+1 && ctx->input_buffer[initial_idx+1] == '.')))) ? ctx->current_idx : (size_t)-1;
}

size_t skipInt(ParseContext_t *ctx) {
    if (!ctx || !ctx->input_buffer) return (size_t)-1;
    size_t initial_idx = ctx->current_idx;
    bool has_digit = false;
    // Handle optional sign
    if (ctx->current_idx < ctx->buffer_len && (ctx->input_buffer[ctx->current_idx] == '-' || ctx->input_buffer[ctx->current_idx] == '+')) {
        ctx->current_idx++;
    }
    while (ctx->current_idx < ctx->buffer_len && isdigit((unsigned char)ctx->input_buffer[ctx->current_idx])) {
        ctx->current_idx++;
        has_digit = true;
    }
    return has_digit ? ctx->current_idx : (size_t)-1;
}

size_t skipAlpha(ParseContext_t *ctx) {
    if (!ctx || !ctx->input_buffer) return (size_t)-1;
    size_t initial_idx = ctx->current_idx;
    while (ctx->current_idx < ctx->buffer_len && isalpha((unsigned char)ctx->input_buffer[ctx->current_idx])) {
        ctx->current_idx++;
    }
    return (initial_idx == ctx->current_idx) ? (size_t)-1 : ctx->current_idx;
}

// Helper function for extractX (double version)
double extractDoubleValue(ParseContext_t *ctx, const char *expected_name) {
    double value = -1.0;
    char *extracted_name = NULL;
    char *extracted_value_str = NULL;
    size_t start_idx_value = 0;
    size_t end_idx_value = 0;
    
    if (ctx == NULL) return -1.0;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) { printf("!!Failed to locate opening brace for %s\n", expected_name); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip opening brace for %s\n", expected_name); goto cleanup; }

    skipWhiteSpace(ctx);
    size_t start_name_idx = ctx->current_idx;
    if (skipToNonAlphaNum(ctx) == (size_t)-1) { printf("!!Failed to locate end of element ID for %s\n", expected_name); goto cleanup; }
    extracted_name = copyData(ctx, start_name_idx, ctx->current_idx);
    if (extracted_name == NULL) { printf("!!Failed to copy element ID for %s\n", expected_name); goto cleanup; }
    if (strcmp(extracted_name, expected_name) != 0) {
        printf("!!Element ID is not \"%s\", found \"%s\"\n", expected_name, extracted_name);
        goto cleanup;
    }
    deallocate(extracted_name, strlen(extracted_name) + 1);
    extracted_name = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate initial closing brace for %s\n", expected_name); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip initial closing brace for %s\n", expected_name); goto cleanup; }

    skipWhiteSpace(ctx);
    start_idx_value = ctx->current_idx;
    end_idx_value = skipFloat(ctx);
    if (end_idx_value == (size_t)-1) { printf("!!Failed to locate end of data for %s\n", expected_name); goto cleanup; }
    
    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) { printf("!!Failed to locate final opening brace for %s\n", expected_name); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip final opening brace for %s\n", expected_name); goto cleanup; }
    
    skipWhiteSpace(ctx);
    if (!atChar(ctx, '#')) { printf("!!Failed to locate closing mark for %s\n", expected_name); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip closing mark for %s\n", expected_name); goto cleanup; }

    start_name_idx = ctx->current_idx;
    if (skipToNonAlphaNum(ctx) == (size_t)-1) { printf("!!Failed to locate end of closing element ID for %s\n", expected_name); goto cleanup; }
    extracted_name = copyData(ctx, start_name_idx, ctx->current_idx);
    if (extracted_name == NULL) { printf("!!Failed to copy closing element ID for %s\n", expected_name); goto cleanup; }
    if (strcmp(extracted_name, expected_name) != 0) {
        printf("!!Invalid closing element ID: \"%s\", expected \"%s\"\n", extracted_name, expected_name);
        goto cleanup;
    }
    deallocate(extracted_name, strlen(extracted_name) + 1);
    extracted_name = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate final closing brace for %s\n", expected_name); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip final closing brace for %s\n", expected_name); goto cleanup; }

    extracted_value_str = copyData(ctx, start_idx_value, end_idx_value);
    if (extracted_value_str == NULL) { printf("!!Failed to copy data for %s\n", expected_name); goto cleanup; }
    value = atof(extracted_value_str);

cleanup:
    if (extracted_name != NULL) deallocate(extracted_name, strlen(extracted_name) + 1);
    if (extracted_value_str != NULL) deallocate(extracted_value_str, strlen(extracted_value_str) + 1);
    return value;
}

int extractPopulation(ParseContext_t *ctx) {
    int value = -1; // Default for unset population
    char *extracted_name = NULL;
    char *extracted_value_str = NULL;
    size_t start_idx_value = 0;
    size_t end_idx_value = 0;
    
    if (ctx == NULL) return -1;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) { printf("!!Failed to locate opening brace for Population\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip opening brace for Population\n"); goto cleanup; }

    skipWhiteSpace(ctx);
    size_t start_name_idx = ctx->current_idx;
    if (skipToNonAlphaNum(ctx) == (size_t)-1) { printf("!!Failed to locate end of element ID for Population\n"); goto cleanup; }
    extracted_name = copyData(ctx, start_name_idx, ctx->current_idx);
    if (extracted_name == NULL) { printf("!!Failed to copy element ID for Population\n"); goto cleanup; }
    if (strcmp(extracted_name, "Population") != 0) {
        printf("!!Element ID is not \"Population\", found \"%s\"\n", extracted_name);
        goto cleanup;
    }
    deallocate(extracted_name, strlen(extracted_name) + 1);
    extracted_name = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate initial closing brace for Population\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip initial closing brace for Population\n"); goto cleanup; }

    skipWhiteSpace(ctx);
    start_idx_value = ctx->current_idx;
    end_idx_value = skipInt(ctx); // Assuming a skipInt function exists
    if (end_idx_value == (size_t)-1) { printf("!!Failed to locate end of data for Population\n"); goto cleanup; }
    
    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) { printf("!!Failed to locate final opening brace for Population\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip final opening brace for Population\n"); goto cleanup; }
    
    skipWhiteSpace(ctx);
    if (!atChar(ctx, '#')) { printf("!!Failed to locate closing mark for Population\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip closing mark for Population\n"); goto cleanup; }

    start_name_idx = ctx->current_idx;
    if (skipToNonAlphaNum(ctx) == (size_t)-1) { printf("!!Failed to locate end of closing element ID for Population\n"); goto cleanup; }
    extracted_name = copyData(ctx, start_name_idx, ctx->current_idx);
    if (extracted_name == NULL) { printf("!!Failed to copy closing element ID for Population\n"); goto cleanup; }
    if (strcmp(extracted_name, "Population") != 0) {
        printf("!!Invalid closing element ID: \"%s\", expected \"Population\"\n", extracted_name);
        goto cleanup;
    }
    deallocate(extracted_name, strlen(extracted_name) + 1);
    extracted_name = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate final closing brace for Population\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip final closing brace for Population\n"); goto cleanup; }

    extracted_value_str = copyData(ctx, start_idx_value, end_idx_value);
    if (extracted_value_str == NULL) { printf("!!Failed to copy data for Population\n"); goto cleanup; }
    value = atoi(extracted_value_str);

cleanup:
    if (extracted_name != NULL) deallocate(extracted_name, strlen(extracted_name) + 1);
    if (extracted_value_str != NULL) deallocate(extracted_value_str, strlen(extracted_value_str) + 1);
    return value;
}

char *extractName(ParseContext_t *ctx) {
    char *name = NULL;
    char *extracted_tag = NULL;
    if (!ctx) return NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) { printf("!!Failed to locate opening brace for Name\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip opening brace for Name\n"); goto cleanup; }
    
    skipWhiteSpace(ctx);
    size_t start_name_idx = ctx->current_idx;
    if (skipToNonAlphaNum(ctx) == (size_t)-1) { printf("!!Failed to locate end of element ID for Name\n"); goto cleanup; }
    extracted_tag = copyData(ctx, start_name_idx, ctx->current_idx);
    if (!extracted_tag || strcmp(extracted_tag, "Name") != 0) {
        printf("!!Element ID is not \"Name\", found \"%s\"\n", extracted_tag ? extracted_tag : "NULL");
        goto cleanup;
    }
    deallocate(extracted_tag, strlen(extracted_tag) + 1);
    extracted_tag = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate initial closing brace for Name\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip initial closing brace for Name\n"); goto cleanup; }

    skipWhiteSpace(ctx);
    size_t start_value_idx = ctx->current_idx;
    // Name can contain spaces, so skip until next '{'
    while (ctx->current_idx < ctx->buffer_len && ctx->input_buffer[ctx->current_idx] != '{') {
        ctx->current_idx++;
    }
    size_t end_value_idx = ctx->current_idx;
    
    // Trim trailing whitespace from name
    while (end_value_idx > start_value_idx && isspace((unsigned char)ctx->input_buffer[end_value_idx - 1])) {
        end_value_idx--;
    }

    skipWhiteSpace(ctx); // Skip to the next '{'
    if (!atChar(ctx, '{')) { printf("!!Failed to locate final opening brace for Name\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip final opening brace for Name\n"); goto cleanup; }
    
    skipWhiteSpace(ctx);
    if (!atChar(ctx, '#')) { printf("!!Failed to locate closing mark for Name\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip closing mark for Name\n"); goto cleanup; }

    start_name_idx = ctx->current_idx;
    if (skipToNonAlphaNum(ctx) == (size_t)-1) { printf("!!Failed to locate end of closing element ID for Name\n"); goto cleanup; }
    extracted_tag = copyData(ctx, start_name_idx, ctx->current_idx);
    if (!extracted_tag || strcmp(extracted_tag, "Name") != 0) {
        printf("!!Invalid closing element ID: \"%s\", expected \"Name\"\n", extracted_tag ? extracted_tag : "NULL");
        goto cleanup;
    }
    deallocate(extracted_tag, strlen(extracted_tag) + 1);
    extracted_tag = NULL;

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) { printf("!!Failed to locate final closing brace for Name\n"); goto cleanup; }
    if (incChar(ctx) == -1) { printf("!!Failed to skip final closing brace for Name\n"); goto cleanup; }

    name = copyData(ctx, start_value_idx, end_value_idx);
    
cleanup:
    if (extracted_tag) deallocate(extracted_tag, strlen(extracted_tag) + 1);
    return name;
}

Country_t *countryTopLevel(ParseContext_t *ctx) {
    printf("Entering countryTopLevel (dummy)\n");
    // Simulate parsing a country block
    // For simplicity, just consume some input and return a dummy country
    skipWhiteSpace(ctx);
    char *tag = "{Country}";
    size_t tag_len = strlen(tag);
    if (ctx->buffer_len - ctx->current_idx < tag_len ||
        strncmp(ctx->input_buffer + ctx->current_idx, tag, tag_len) != 0) {
        printf("Missing {Country} tag (dummy)\n");
        return NULL;
    }
    ctx->current_idx += tag_len; // Skip "{Country}"
    skipWhiteSpace(ctx);

    // Consume until a closing tag for Country
    size_t initial_country_idx = ctx->current_idx; // Save context for error reporting
    while(ctx->current_idx < ctx->buffer_len) {
        if (ctx->input_buffer[ctx->current_idx] == '{' && 
            ctx->current_idx + 1 < ctx->buffer_len && ctx->input_buffer[ctx->current_idx+1] == '#') {
            // Found potential closing tag
            size_t temp_idx = ctx->current_idx + 2;
            size_t start_closing_name_idx = temp_idx;
            while (temp_idx < ctx->buffer_len && isalpha((unsigned char)ctx->input_buffer[temp_idx])) {
                temp_idx++;
            }
            char *closing_name = copyData(ctx, start_closing_name_idx, temp_idx);
            if (closing_name && strcmp(closing_name, "Country") == 0) {
                deallocate(closing_name, strlen(closing_name) + 1);
                // Found {#Country}
                ctx->current_idx = temp_idx; // Move past name
                skipWhiteSpace(ctx);
                if (atChar(ctx, '}')) {
                    incChar(ctx); // Skip '}'
                    Country_t *new_country = (Country_t*)allocate(sizeof(Country_t), 0, NULL);
                    if (new_country) {
                        initCountry(new_country);
                        // Dummy name for testing
                        strncpy(new_country->name, "DummyCountry", MAX_COUNTRY_NAME_LEN);
                        new_country->name[MAX_COUNTRY_NAME_LEN] = '\0';
                    }
                    return new_country;
                }
            }
            if (closing_name) deallocate(closing_name, strlen(closing_name) + 1);
        }
        ctx->current_idx++;
    }
    printf("Failed to find closing tag for Country (dummy) at index %zu\n", initial_country_idx);
    return NULL;
}


// --- Original Functions (Fixed) ---

// Function: planetMenu
int planetMenu(Planet_t *planet) {
    int choice = 0;
    char input_buffer[30];
    int menu_status = 3; // 3 means continue, 0 means delete and exit, 1 means exit

    if (planet == NULL) {
        return 0; // Invalid planet pointer
    }

    do {
        memset(input_buffer, 0, sizeof(input_buffer));
        printf("\nPlanet: %s\n", planet->name);
        printf("1) Display planet information\n");
        printf("2) Set Period\n");
        printf("3) Set Orbit Speed\n");
        printf("4) Set Aphelion\n");
        printf("5) Set Perihelion\n");
        printf("6) Set Mean Radius\n");
        printf("7) Set Equatorial Radius\n");
        printf("8) Set Mass\n");
        printf("9) Set Gravity\n");
        printf("10) Set population\n");
        printf("11) Add Country\n");
        printf("12) Select country\n");
        printf("13) Delete Planet and exit menu\n");
        printf("14) Exit menu\n");
        printf("Selection: ");
        receive_until(input_buffer, sizeof(input_buffer) - 1, '\n'); // Read up to newline
        choice = atoi(input_buffer);

        switch (choice) {
            case 1:
                printPlanetInfo(planet);
                break;
            case 2:
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                planet->period = atof(input_buffer);
                break;
            case 3:
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                planet->orbit_speed = atof(input_buffer);
                break;
            case 4:
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                planet->aphelion = atof(input_buffer);
                break;
            case 5:
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                planet->perihelion = atof(input_buffer);
                break;
            case 6:
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                planet->mean_radius = atof(input_buffer);
                break;
            case 7:
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                planet->equatorial_radius = atof(input_buffer);
                break;
            case 8:
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                planet->mass = atof(input_buffer);
                break;
            case 9:
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                planet->gravity = atof(input_buffer);
                break;
            case 10:
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                planet->population = atoi(input_buffer);
                break;
            case 11: { // Add Country
                if (planet->num_countries < MAX_COUNTRIES_PER_PLANET) {
                    printf("\nNew Name: ");
                    receive_until(input_buffer, sizeof(input_buffer) - 1, '\n'); 
                    int country_slot_idx = -1;
                    for (int j = 0; j < MAX_COUNTRIES_PER_PLANET; ++j) {
                        if (planet->countries[j] == NULL) {
                            country_slot_idx = j;
                            break;
                        }
                    }

                    if (country_slot_idx == -1) {
                        printf("!!No country slots\n");
                    } else {
                        Country_t *new_country = (Country_t*)allocate(sizeof(Country_t), 0, NULL);
                        if (new_country != NULL) {
                            initCountry(new_country);
                            planet->countries[country_slot_idx] = new_country;
                            
                            int name_idx = 0;
                            // Copy alphanumeric characters up to MAX_COUNTRY_NAME_LEN
                            while (name_idx < MAX_COUNTRY_NAME_LEN && isalnum((unsigned char)input_buffer[name_idx])) {
                                new_country->name[name_idx] = input_buffer[name_idx];
                                name_idx++;
                            }
                            new_country->name[name_idx] = '\0'; // Null-terminate
                            planet->num_countries++;
                        } else {
                            printf("!!Failed to allocate structure\n");
                        }
                    }
                } else {
                    printf("Too many countries\n");
                }
                break;
            }
            case 12: { // Select country
                printf("\nCountries:\n");
                for (int j = 0; j < MAX_COUNTRIES_PER_PLANET; ++j) {
                    if (planet->countries[j] != NULL) {
                        printf("%d) %s\n", j + 1, planet->countries[j]->name);
                    }
                }
                memset(input_buffer, 0, sizeof(input_buffer));
                printf("\n-> ");
                receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
                int country_selection = atoi(input_buffer);

                if ((country_selection < 1) || (MAX_COUNTRIES_PER_PLANET < country_selection)) {
                    printf("Invalid choice...\n");
                } else if (planet->countries[country_selection - 1] == NULL) {
                    printf("Invalid choice...\n");
                } else {
                    int country_menu_result = countryMenu(planet->countries[country_selection - 1]);
                    if (country_menu_result == 0) { // If countryMenu returns 0, delete country
                        freeCountry(planet->countries[country_selection - 1]);
                        planet->countries[country_selection - 1] = NULL;
                        planet->num_countries--;
                    }
                }
                break;
            }
            case 13:
                freePlanet(planet);
                menu_status = 0; // Delete planet and exit
                break;
            case 14:
                menu_status = 1; // Exit menu
                break;
            default:
                printf("Invalid...\n");
                // menu_status remains 3, loop continues
        }
    } while (menu_status == 3); // Loop until an exit condition is met

    return menu_status;
}

// Function: freePlanet
void freePlanet(Planet_t *planet) {
    if (planet != NULL) {
        for (int i = 0; i < MAX_COUNTRIES_PER_PLANET; ++i) {
            if (planet->countries[i] != NULL) {
                freeCountry(planet->countries[i]);
                planet->countries[i] = NULL;
            }
        }
        deallocate(planet, sizeof(Planet_t)); // Using sizeof(Planet_t) for deallocation
    }
}

// Function: printPlanetInfo
void printPlanetInfo(Planet_t *planet) {
    if (planet != NULL) {
        if (planet->name[0] == '\0') {
            printf("Name: Unknown\n");
        } else {
            printf("Name: %s\n", planet->name); // Fixed format specifier
        }
        if (planet->population > -1) { // Check for valid population (assuming -1 means unset)
            printf("\tPopulation: %d\n", planet->population); // Fixed format specifier
        }
        if (planet->period >= 0.0) { // Check for valid period (assuming -1.0 means unset)
            printf("\tPeriod: %f\n", planet->period); // Fixed format specifier
        }
        if (planet->orbit_speed >= 0.0) {
            printf("\tOrbit Speed: %f\n", planet->orbit_speed);
        }
        if (planet->aphelion >= 0.0) {
            printf("\tAphelion: %f\n", planet->aphelion);
        }
        if (planet->perihelion >= 0.0) {
            printf("\tPerihelion: %f\n", planet->perihelion);
        }
        if (planet->mean_radius >= 0.0) {
            printf("\tRadius: %f\n", planet->mean_radius);
        }
        if (planet->equatorial_radius >= 0.0) {
            printf("\tERadius: %f\n", planet->equatorial_radius);
        }
        if (planet->mass >= 0.0) {
            printf("\tMass: %f\n", planet->mass);
        }
        if (planet->gravity >= 0.0) {
            printf("\tGravity: %f\n", planet->gravity);
        }
        for (int i = 0; i < MAX_COUNTRIES_PER_PLANET; ++i) {
            if (planet->countries[i] != NULL) {
                printCountryInfo(planet->countries[i]);
            }
        }
    }
}

// Function: initPlanet
void initPlanet(Planet_t *planet) {
    if (planet != NULL) {
        memset(planet->name, 0, sizeof(planet->name)); // bzero -> memset
        planet->period = -1.0;
        planet->orbit_speed = -1.0;
        planet->aphelion = -1.0;
        planet->perihelion = -1.0;
        planet->mean_radius = -1.0;
        planet->equatorial_radius = -1.0;
        planet->mass = -1.0;
        planet->gravity = -1.0;
        planet->population = -1;
        planet->num_countries = 0;
        for (int i = 0; i < MAX_COUNTRIES_PER_PLANET; ++i) {
            planet->countries[i] = NULL;
        }
    }
}

// Function: planetTopLevel
Planet_t *planetTopLevel(ParseContext_t *ctx) {
    char *element_name = NULL;
    Planet_t *new_planet = NULL;
    size_t initial_ctx_idx; 
    
    if (ctx == NULL || ctx->input_buffer == NULL) {
        return NULL;
    }

    skipWhiteSpace(ctx);
    initial_ctx_idx = ctx->current_idx; // Save current index for error reset

    // Check for "{Planet}" tag
    const char *planet_tag = "{Planet}";
    size_t tag_len = strlen(planet_tag);
    if (ctx->buffer_len - ctx->current_idx < tag_len ||
        strncmp(ctx->input_buffer + ctx->current_idx, planet_tag, tag_len) != 0) {
        return NULL; // Not a Planet block
    }
    ctx->current_idx += tag_len; // Skip "{Planet}"
    
    skipWhiteSpace(ctx);
    
    new_planet = (Planet_t*)allocate(sizeof(Planet_t), 0, NULL);
    if (new_planet == NULL) {
        printf("!!Failed to allocate Planet structure\n");
        return NULL;
    }
    initPlanet(new_planet); // Initialize the new planet

    element_name = pullNextElementName(ctx); // pullNextElementName returns allocated char*
    while (element_name != NULL) {
        int element_enum = elementNameToEnum(element_name);
        deallocate(element_name, strlen(element_name) + 1); // Free element_name after use
        element_name = NULL; // Clear for next iteration

        switch (element_enum) {
            case 1: // Name
                element_name = extractName(ctx); // extractName returns allocated char*
                if (element_name != NULL) {
                    strncpy(new_planet->name, element_name, MAX_PLANET_NAME_LEN);
                    new_planet->name[MAX_PLANET_NAME_LEN] = '\0'; // Ensure null termination
                    deallocate(element_name, strlen(element_name) + 1);
                    element_name = NULL;
                } else {
                    goto parse_error;
                }
                break;
            case 2: // Period
                new_planet->period = extractDoubleValue(ctx, "Period");
                if (new_planet->period < 0.0) goto parse_error;
                break;
            case 3: // OrbitSpeed
                new_planet->orbit_speed = extractDoubleValue(ctx, "OrbitSpeed");
                if (new_planet->orbit_speed < 0.0) goto parse_error;
                break;
            case 4: // Aphelion
                new_planet->aphelion = extractDoubleValue(ctx, "Aphelion");
                if (new_planet->aphelion < 0.0) goto parse_error;
                break;
            case 5: // Perihelion
                new_planet->perihelion = extractDoubleValue(ctx, "Perihelion");
                if (new_planet->perihelion < 0.0) goto parse_error;
                break;
            case 6: // Radius (Mean Radius)
                new_planet->mean_radius = extractDoubleValue(ctx, "Radius");
                if (new_planet->mean_radius < 0.0) goto parse_error;
                break;
            case 7: // ERadius (Equatorial Radius)
                new_planet->equatorial_radius = extractDoubleValue(ctx, "ERadius");
                if (new_planet->equatorial_radius < 0.0) goto parse_error;
                break;
            case 8: // Mass
                new_planet->mass = extractDoubleValue(ctx, "Mass");
                if (new_planet->mass < 0.0) goto parse_error;
                break;
            case 9: // Gravity
                new_planet->gravity = extractDoubleValue(ctx, "Gravity");
                if (new_planet->gravity < 0.0) goto parse_error;
                break;
            case 10: // Population
                new_planet->population = extractPopulation(ctx);
                if (new_planet->population < -1) goto parse_error; // Assuming -1 means not set/invalid
                break;
            case 11: { // Country
                if (new_planet->num_countries < MAX_COUNTRIES_PER_PLANET) {
                    Country_t *country = countryTopLevel(ctx); // countryTopLevel returns allocated Country_t*
                    if (country != NULL) {
                        // Find first empty slot
                        int slot_idx = -1;
                        for(int i = 0; i < MAX_COUNTRIES_PER_PLANET; ++i) {
                            if(new_planet->countries[i] == NULL) {
                                slot_idx = i;
                                break;
                            }
                        }
                        if (slot_idx != -1) {
                           new_planet->countries[slot_idx] = country;
                           new_planet->num_countries++;
                        } else { // Should not happen if num_countries < MAX_COUNTRIES_PER_PLANET
                           printf("!!No available country slots despite check\n");
                           freeCountry(country);
                           goto parse_error;
                        }
                    } else {
                        printf("!!Failed to parse country block\n");
                        goto parse_error;
                    }
                } else {
                    printf("!!Only %d countries allowed\n", MAX_COUNTRIES_PER_PLANET);
                    goto parse_error; // Too many countries is an error for parsing
                }
                break;
            }
            default:
                printf("Not allowed under Planet: %s\n", element_name ? element_name : "NULL");
                goto parse_error;
        }
        
        element_name = pullNextElementName(ctx); // Get next element
    }

    // After parsing all elements, check for closing tag
    skipWhiteSpace(ctx);
    if (!atChar(ctx, '{')) {
        printf("!!Closing value failed for Planet\n");
        goto parse_error;
    }
    if (incChar(ctx) == -1) goto parse_error; // Skip '{'
    skipWhiteSpace(ctx);
    if (!atChar(ctx, '#')) {
        printf("!!Malformed closing element (missing #)\n");
        goto parse_error;
    }
    if (incChar(ctx) == -1) goto parse_error; // Skip '#'

    size_t start_name_idx = ctx->current_idx;
    if (skipAlpha(ctx) == (size_t)-1) { // Advance past element name
        printf("!!Malformed closing element name\n");
        goto parse_error;
    }
    char *closing_tag_name = copyData(ctx, start_name_idx, ctx->current_idx);
    if (closing_tag_name == NULL || strcmp(closing_tag_name, "Planet") != 0) {
        printf("!!Closing element malformed or mismatch: %s\n", closing_tag_name ? closing_tag_name : "NULL");
        if (closing_tag_name) deallocate(closing_tag_name, strlen(closing_tag_name) + 1);
        goto parse_error;
    }
    deallocate(closing_tag_name, strlen(closing_tag_name) + 1);

    skipWhiteSpace(ctx);
    if (!atChar(ctx, '}')) {
        printf("!!Failed to find final closing brace\n");
        goto parse_error;
    }
    if (incChar(ctx) == -1) goto parse_error; // Skip '}'

    return new_planet; // Success

parse_error:
    printf("!!Error at: %s\n", ctx->input_buffer + ctx->current_idx);
    if (new_planet != NULL) {
        freePlanet(new_planet);
    }
    if (element_name != NULL) {
        deallocate(element_name, strlen(element_name) + 1);
    }
    ctx->current_idx = initial_ctx_idx; // Reset context index on error (optional, but good practice for nested parsing)
    return NULL;
}

// Function: extractPeriod
double extractPeriod(ParseContext_t *ctx) {
    return extractDoubleValue(ctx, "Period");
}

// Function: extractOrbitSpeed
double extractOrbitSpeed(ParseContext_t *ctx) {
    return extractDoubleValue(ctx, "OrbitSpeed");
}

// Function: extractAphelion
double extractAphelion(ParseContext_t *ctx) {
    return extractDoubleValue(ctx, "Aphelion");
}

// Function: extractPerihelion
double extractPerihelion(ParseContext_t *ctx) {
    return extractDoubleValue(ctx, "Perihelion");
}

// Function: extractRadius
double extractRadius(ParseContext_t *ctx) {
    return extractDoubleValue(ctx, "Radius");
}

// Function: extractERadius
double extractERadius(ParseContext_t *ctx) {
    return extractDoubleValue(ctx, "ERadius");
}

// Function: extractMass
double extractMass(ParseContext_t *ctx) {
    return extractDoubleValue(ctx, "Mass");
}

// Function: extractGravity
double extractGravity(ParseContext_t *ctx) {
    return extractDoubleValue(ctx, "Gravity");
}