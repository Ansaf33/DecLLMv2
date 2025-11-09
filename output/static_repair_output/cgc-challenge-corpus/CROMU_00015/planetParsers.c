#include <stdio.h>
#include <stdlib.h> // For atoi, atof
#include <string.h> // For memset, strlen, strncpy, strcmp
#include <ctype.h>  // For isalnum
#include <stdbool.h> // For bool

// --- Assumed custom type definitions from decompiler ---
// Based on usage, these are mapped to standard C types.
typedef int UNDEFINED4;
typedef double UNDEFINED8;
typedef long double LONGDOUBLE;
typedef unsigned int UINT;

// --- Assumed parser state struct ---
// The original code uses an `int *` for parser state, accessing elements like `param_1[0]`, `param_1[1]`, `param_1[2]`.
// This struct provides a more type-safe and readable representation.
typedef struct ParserState {
    char *buffer;        // param_1[0]: Base address of the input buffer
    int length;          // param_1[1]: Length of the input buffer
    int current_index;   // param_1[2]: Current parsing index/offset in the buffer
} ParserState;

// --- Forward declarations for custom functions ---
// These functions are not provided in the snippet but are called by it.
// Their signatures are inferred from context.
void *allocate(size_t size, int param2, void **out_ptr);
void deallocate(void *ptr, size_t size);
void receive_until(char *buffer, int delimiter, int max_len);
int initCountry(void *country_ptr); // Assume it returns 0 on success
void printCountryInfo(void *country_ptr);
void freeCountry(void *country_ptr);
int countryMenu(void *country_ptr); // Assume it returns 0 if country is deleted, 1 otherwise

void skipWhiteSpace(ParserState *parser);
char *pullNextElementName(ParserState *parser);
void getIndex(ParserState *parser, int *index_ptr);
int elementNameToEnum(char *name); // Returns an int representing the element type
char *extractName(ParserState *parser);
UNDEFINED4 extractPopulation(ParserState *parser); // Returns int population
int atChar(ParserState *parser, char c); // Returns 1 if char matches, 0 otherwise
int incChar(ParserState *parser); // Increments index, returns new index or -1 on error
int skipLength(ParserState *parser, int length); // Skips length chars, returns new index or -1 on error
int skipToNonAlphaNum(ParserState *parser); // Returns index of non-alphanum char or -1
char *copyData(ParserState *parser, int start_idx, int end_idx); // Copies data from buffer, returns new string or NULL
int skipFloat(ParserState *parser); // Skips float, returns end index or -1
int skipAlpha(ParserState *parser); // Skips alpha chars, returns end index or -1
void *countryTopLevel(ParserState *parser); // Assumed to return a Country*

// --- Struct definitions ---
#define MAX_COUNTRIES 30
#define PLANET_NAME_MAX_LEN 19 // 19 chars + null terminator = 20 bytes

// Placeholder for Country struct, based on usage in planetMenu and planetTopLevel.
// allocate(0x218,...) suggests its size is 0x218 bytes.
// *(char *)(local_14 + local_38) = local_32[local_14]; suggests name is at offset 0.
typedef struct Country {
    char name[64]; // Example size, actual size would be 0x218
    // ... other country fields
} Country;

// Inferred Planet struct based on offsets and data types.
// Total size: 0xd4 bytes, matching allocate(0xd4, ...)
typedef struct Planet {
    char name[PLANET_NAME_MAX_LEN + 1]; // 0x00 - 0x13 (20 bytes)
    double period;                      // 0x14 - 0x1b (8 bytes)
    double orbit_speed;                 // 0x1c - 0x23 (8 bytes)
    double aphelion;                    // 0x24 - 0x2b (8 bytes)
    double perihelion;                  // 0x2c - 0x33 (8 bytes)
    double mean_radius;                 // 0x34 - 0x3b (8 bytes)
    double equatorial_radius;           // 0x3c - 0x43 (8 bytes)
    double mass;                        // 0x44 - 0x4b (8 bytes)
    double gravity;                     // 0x4c - 0x53 (8 bytes)
    int population;                     // 0x54 - 0x57 (4 bytes)
    int country_count;                  // 0x58 - 0x5b (4 bytes)
    Country *countries[MAX_COUNTRIES];  // 0x5c - 0xd3 (30 * 4 = 120 bytes)
} Planet;

// Enum for element names used in planetTopLevel's switch statement
enum PlanetElement {
    ELEMENT_UNKNOWN = 0,
    ELEMENT_NAME = 1,
    ELEMENT_PERIOD = 2,
    ELEMENT_ORBIT_SPEED = 3,
    ELEMENT_APHELION = 4,
    ELEMENT_PERIHELION = 5,
    ELEMENT_MEAN_RADIUS = 6,
    ELEMENT_EQUATORIAL_RADIUS = 7,
    ELEMENT_MASS = 8,
    ELEMENT_GRAVITY = 9,
    ELEMENT_POPULATION = 10,
    ELEMENT_COUNTRY = 11
};

// --- Function Prototypes for the supplied snippet ---
// These are declared here to ensure they are available for use in the fixed code.
void freePlanet(Planet *planet);
void printPlanetInfo(Planet *planet);
void initPlanet(Planet *planet);
Planet *planetTopLevel(ParserState *parser);
LONGDOUBLE extractPeriod(ParserState *parser);
LONGDOUBLE extractOrbitSpeed(ParserState *parser);
LONGDOUBLE extractAphelion(ParserState *parser);
LONGDOUBLE extractPerihelion(ParserState *parser);
LONGDOUBLE extractRadius(ParserState *parser);
LONGDOUBLE extractERadius(ParserState *parser);
LONGDOUBLE extractMass(ParserState *parser);
LONGDOUBLE extractGravity(ParserState *parser);

// Helper function to reduce code duplication in extractX functions
static LONGDOUBLE extractValue(ParserState *parser, const char *element_name_str) {
    LONGDOUBLE result = -1.0L;
    char *copied_data = NULL;
    char *element_id = NULL;
    int start_idx_data = -1, end_idx_data = -1;
    int start_idx_tag = -1, end_idx_tag = -1;

    if (parser == NULL) {
        return result;
    }

    skipWhiteSpace(parser);
    if (!atChar(parser, '{')) {
        printf("!!Failed to locate opening brace for %s\n", element_name_str);
        return result;
    }
    if (skipLength(parser, 1) == -1) { // Skip '{'
        printf("!!Failed to skip opening brace for %s\n", element_name_str);
        return result;
    }

    skipWhiteSpace(parser);
    getIndex(parser, &start_idx_tag); // Get start index of element ID
    end_idx_tag = skipToNonAlphaNum(parser); // Get end index of element ID
    if (end_idx_tag == -1) {
        printf("!!Failed to locate the end of the element id for %s\n", element_name_str);
        return result;
    }

    element_id = copyData(parser, start_idx_tag, end_idx_tag);
    if (element_id == NULL) {
        printf("!!Copy element ID from %d to %d failed for %s\n", start_idx_tag, end_idx_tag, element_name_str);
        return result;
    }

    if (strcmp(element_id, element_name_str) != 0) {
        printf("!!Element id is not \"%s\", got \"%s\"\n", element_name_str, element_id);
        deallocate(element_id, strlen(element_id) + 1);
        return result;
    }
    deallocate(element_id, strlen(element_id) + 1);
    element_id = NULL; // Clear pointer after use

    skipWhiteSpace(parser);
    if (!atChar(parser, '}')) {
        printf("!!Failed to locate initial closing brace for %s\n", element_name_str);
        return result;
    }
    if (skipLength(parser, 1) == -1) { // Skip '}'
        printf("!!Failed to skip initial closing brace for %s\n", element_name_str);
        return result;
    }

    skipWhiteSpace(parser);
    getIndex(parser, &start_idx_data); // Get start index of data
    end_idx_data = skipFloat(parser); // Get end index of data
    if (end_idx_data == -1) {
        printf("!!Failed to locate the end of the %s data\n", element_name_str);
        return result;
    }

    skipWhiteSpace(parser);
    if (!atChar(parser, '{')) {
        printf("!!Failed to locate the final opening brace for %s\n", element_name_str);
        return result;
    }
    if (skipLength(parser, 1) == -1) { // Skip '{'
        printf("!!Failed to skip the final opening brace for %s\n", element_name_str);
        return result;
    }

    skipWhiteSpace(parser);
    if (!atChar(parser, '#')) {
        printf("!!Failed to locate the closing mark for %s\n", element_name_str);
        return result;
    }
    if (skipLength(parser, 1) == -1) { // Skip '#'
        printf("!!Failed to skip closing mark for %s\n", element_name_str);
        return result;
    }

    getIndex(parser, &start_idx_tag); // Get start index of closing element ID
    end_idx_tag = skipToNonAlphaNum(parser); // Get end index of closing element ID
    if (end_idx_tag == -1) {
        printf("!!Failed to locate the end of the closing element id for %s\n", element_name_str);
        return result;
    }

    element_id = copyData(parser, start_idx_tag, end_idx_tag);
    if (element_id == NULL) {
        printf("!!Failed to copy closing element ID for %s\n", element_name_str);
        return result;
    }

    if (strcmp(element_id, element_name_str) != 0) {
        printf("!!Invalid closing element id: \"%s\", expected \"%s\"\n", element_id, element_name_str);
        deallocate(element_id, strlen(element_id) + 1);
        return result;
    }
    deallocate(element_id, strlen(element_id) + 1);
    element_id = NULL;

    skipWhiteSpace(parser);
    if (!atChar(parser, '}')) {
        printf("!!Failed to locate final closing brace for %s\n", element_name_str);
        return result;
    }
    skipLength(parser, 1); // Skip the final '}'

    copied_data = copyData(parser, start_idx_data, end_idx_data);
    if (copied_data == NULL) {
        printf("!!Failed to copy %s data\n", element_name_str);
        return result;
    }

    result = atof(copied_data);
    deallocate(copied_data, strlen(copied_data) + 1);

    return result;
}


// Function: planetMenu
int planetMenu(Planet *planet) {
    int choice;
    char input_buffer[30];
    Country *new_country_ptr = NULL; // For allocate output
    int i; // Loop counter

    if (planet == NULL) {
        return 0; // Return 0 to indicate exit/failure
    }

    choice = 0; // Initialize to enter the loop
    while (choice != 14 && choice != 13) {
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

        receive_until(input_buffer, 10, sizeof(input_buffer) - 1); // 10 is newline, max_len is buffer size - 1
        choice = atoi(input_buffer);

        switch (choice) {
            case 1:
                printPlanetInfo(planet);
                break;
            case 2: {
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                planet->period = atof(input_buffer);
                break;
            }
            case 3: {
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                planet->orbit_speed = atof(input_buffer);
                break;
            }
            case 4: {
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                planet->aphelion = atof(input_buffer);
                break;
            }
            case 5: {
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                planet->perihelion = atof(input_buffer);
                break;
            }
            case 6: {
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                planet->mean_radius = atof(input_buffer);
                break;
            }
            case 7: {
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                planet->equatorial_radius = atof(input_buffer);
                break;
            }
            case 8: {
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                planet->mass = atof(input_buffer);
                break;
            }
            case 9: {
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                planet->gravity = atof(input_buffer);
                break;
            }
            case 10: {
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                planet->population = atoi(input_buffer);
                break;
            }
            case 11: // Add Country
                if (planet->country_count < MAX_COUNTRIES) {
                    printf("\nNew Name: ");
                    receive_until(input_buffer, 10, PLANET_NAME_MAX_LEN); // Max 19 chars for country name

                    // Find first available slot
                    int slot_idx = -1;
                    for (i = 0; i < MAX_COUNTRIES; i++) {
                        if (planet->countries[i] == NULL) {
                            slot_idx = i;
                            break;
                        }
                    }

                    if (slot_idx == -1) { // Should not happen if country_count < MAX_COUNTRIES
                        printf("!!No country slots\n");
                    } else {
                        // allocate(0x218,0,&local_38);
                        if (allocate(sizeof(Country), 0, (void**)&new_country_ptr) == 0 && new_country_ptr != NULL) {
                            initCountry(new_country_ptr);
                            planet->countries[slot_idx] = new_country_ptr;

                            // Copy name, only alphanumeric chars
                            for (i = 0; i < (int)strlen(input_buffer) && i < sizeof(new_country_ptr->name) - 1; i++) {
                                if (isalnum((int)input_buffer[i])) {
                                    new_country_ptr->name[i] = input_buffer[i];
                                } else {
                                    // Stop copying if non-alphanumeric char encountered
                                    break;
                                }
                            }
                            new_country_ptr->name[i] = '\0'; // Null-terminate
                            planet->country_count++;
                        } else {
                            printf("!!Failed to allocate structure\n");
                        }
                    }
                } else {
                    printf("Too many countries\n");
                }
                break;
            case 12: // Select country
                printf("\nCountries:\n");
                for (i = 0; i < MAX_COUNTRIES; i++) { // Iterate through all possible slots
                    if (planet->countries[i] != NULL) {
                        printf("%d) %s\n", i + 1, planet->countries[i]->name);
                    }
                }
                memset(input_buffer, 0, sizeof(input_buffer));
                printf("\n-> ");
                receive_until(input_buffer, 10, sizeof(input_buffer) - 1);
                int country_choice = atoi(input_buffer);

                if (country_choice < 1 || country_choice > MAX_COUNTRIES) {
                    printf("Invalid choice...\n");
                } else if (planet->countries[country_choice - 1] == NULL) {
                    printf("Invalid choice...\n");
                } else {
                    // countryMenu returns 0 if country was deleted, 1 otherwise
                    if (countryMenu(planet->countries[country_choice - 1]) == 0) {
                        planet->countries[country_choice - 1] = NULL; // Mark as deleted
                        planet->country_count--;
                    }
                }
                break;
            case 13: // Delete Planet and exit menu
                freePlanet(planet);
                return 0; // Indicate planet deleted and menu exited
            case 14: // Exit menu
                return 1; // Indicate menu exited, planet not deleted
            default:
                printf("Invalid...\n");
                break;
        }
    }
    return 1; // Should not be reached if 13 or 14 are chosen, but as a fallback
}

// Function: freePlanet
void freePlanet(Planet *planet) {
    if (planet != NULL) {
        for (int i = 0; i < MAX_COUNTRIES; i++) {
            if (planet->countries[i] != NULL) {
                freeCountry(planet->countries[i]);
                planet->countries[i] = NULL;
            }
        }
        deallocate(planet, sizeof(Planet));
    }
}

// Function: printPlanetInfo
void printPlanetInfo(Planet *planet) {
    if (planet != NULL) {
        if (planet->name[0] == '\0') {
            printf("Name: Unknown\n");
        } else {
            printf("Name: %s\n", planet->name);
        }
        if (planet->population >= 0) {
            printf("\tPopulation: %d\n", planet->population);
        }
        if (planet->period >= 0.0) {
            printf("\tPeriod: %lf\n", planet->period);
        }
        if (planet->orbit_speed >= 0.0) {
            printf("\tOrbit Speed: %lf\n", planet->orbit_speed);
        }
        if (planet->aphelion >= 0.0) {
            printf("\tAphelion: %lf\n", planet->aphelion);
        }
        if (planet->perihelion >= 0.0) {
            printf("\tPerihelion: %lf\n", planet->perihelion);
        }
        if (planet->mean_radius >= 0.0) {
            printf("\tRadius: %lf\n", planet->mean_radius);
        }
        if (planet->equatorial_radius >= 0.0) {
            printf("\tERadius: %lf\n", planet->equatorial_radius);
        }
        if (planet->mass >= 0.0) {
            printf("\tMass: %lf\n", planet->mass);
        }
        if (planet->gravity >= 0.0) {
            printf("\tGravity: %lf\n", planet->gravity);
        }
        for (int i = 0; i < MAX_COUNTRIES; i++) {
            if (planet->countries[i] != NULL) {
                printCountryInfo(planet->countries[i]);
            }
        }
    }
}

// Function: initPlanet
void initPlanet(Planet *planet) {
    if (planet != NULL) {
        memset(planet->name, 0, sizeof(planet->name));
        planet->period = -1.0;
        planet->orbit_speed = -1.0;
        planet->aphelion = -1.0;
        planet->perihelion = -1.0;
        planet->mean_radius = -1.0;
        planet->equatorial_radius = -1.0;
        planet->mass = -1.0;
        planet->gravity = -1.0;
        planet->population = -1;
        planet->country_count = 0;
        for (int i = 0; i < MAX_COUNTRIES; i++) {
            planet->countries[i] = NULL;
        }
    }
}

// Function: planetTopLevel
Planet *planetTopLevel(ParserState *parser) {
    Planet *new_planet = NULL;
    char *element_name = NULL;
    int initial_parser_index;
    bool success = false; // Flag to track overall success

    if (parser == NULL) {
        return NULL;
    }

    skipWhiteSpace(parser);
    initial_parser_index = parser->current_index; // Store original index for rollback on error

    // Check for "{Planet}" tag
    if (parser->length - parser->current_index >= 8) { // 8 is length of "{Planet}"
        const char *planet_tag = "{Planet}";
        bool tag_match = true;
        for (int i = 0; i < 8; i++) {
            if (parser->buffer[parser->current_index + i] != planet_tag[i]) {
                tag_match = false;
                break;
            }
        }
        if (!tag_match) {
            return NULL;
        }
        parser->current_index += 8;
        skipWhiteSpace(parser);

        // Allocate Planet structure
        if (allocate(sizeof(Planet), 0, (void**)&new_planet) == 0 && new_planet != NULL) {
            initPlanet(new_planet);
            success = true; // Assume success initially, will set false on error

            element_name = pullNextElementName(parser);
            while (element_name != NULL && success) {
                enum PlanetElement element_type = elementNameToEnum(element_name);
                deallocate(element_name, strlen(element_name) + 1); // Free element_name
                element_name = NULL; // Clear for next iteration

                switch (element_type) {
                    case ELEMENT_NAME:
                        element_name = extractName(parser);
                        if (element_name != NULL) {
                            strncpy(new_planet->name, element_name, PLANET_NAME_MAX_LEN);
                            new_planet->name[PLANET_NAME_MAX_LEN] = '\0'; // Ensure null-termination
                            deallocate(element_name, strlen(element_name) + 1);
                            element_name = NULL;
                        } else {
                            success = false;
                        }
                        break;
                    case ELEMENT_PERIOD:
                        new_planet->period = (double)extractPeriod(parser);
                        if (new_planet->period < 0.0) success = false;
                        break;
                    case ELEMENT_ORBIT_SPEED:
                        new_planet->orbit_speed = (double)extractOrbitSpeed(parser);
                        if (new_planet->orbit_speed < 0.0) success = false;
                        break;
                    case ELEMENT_APHELION:
                        new_planet->aphelion = (double)extractAphelion(parser);
                        if (new_planet->aphelion < 0.0) success = false;
                        break;
                    case ELEMENT_PERIHELION:
                        new_planet->perihelion = (double)extractPerihelion(parser);
                        if (new_planet->perihelion < 0.0) success = false;
                        break;
                    case ELEMENT_MEAN_RADIUS:
                        new_planet->mean_radius = (double)extractRadius(parser);
                        if (new_planet->mean_radius < 0.0) success = false;
                        break;
                    case ELEMENT_EQUATORIAL_RADIUS:
                        new_planet->equatorial_radius = (double)extractERadius(parser);
                        if (new_planet->equatorial_radius < 0.0) success = false;
                        break;
                    case ELEMENT_MASS:
                        new_planet->mass = (double)extractMass(parser);
                        if (new_planet->mass < 0.0) success = false;
                        break;
                    case ELEMENT_GRAVITY:
                        new_planet->gravity = (double)extractGravity(parser);
                        if (new_planet->gravity < 0.0) success = false;
                        break;
                    case ELEMENT_POPULATION:
                        new_planet->population = extractPopulation(parser);
                        if (new_planet->population < 0) success = false;
                        break;
                    case ELEMENT_COUNTRY:
                        if (new_planet->country_count < MAX_COUNTRIES) {
                            Country *country_ptr = (Country *)countryTopLevel(parser); // Assume countryTopLevel returns Country*
                            if (country_ptr != NULL) {
                                // Find first empty slot for the new country
                                int slot_found = -1;
                                for(int k = 0; k < MAX_COUNTRIES; k++) {
                                    if (new_planet->countries[k] == NULL) {
                                        new_planet->countries[k] = country_ptr;
                                        slot_found = k;
                                        break;
                                    }
                                }
                                if (slot_found != -1) {
                                    new_planet->country_count++;
                                } else { // Should not happen if country_count < MAX_COUNTRIES
                                    printf("!!No country slot found despite count being less than max.\n");
                                    freeCountry(country_ptr); // Free allocated country
                                    success = false;
                                }
                            } else {
                                success = false; // countryTopLevel failed
                            }
                        } else {
                            printf("!!Only %d countries allowed\n", MAX_COUNTRIES);
                            success = false; // Too many countries, treat as error for parsing
                        }
                        break;
                    default:
                        printf("Not allowed under Planet: %s\n", element_name);
                        success = false;
                        break;
                }
                if (success) {
                    element_name = pullNextElementName(parser);
                } else {
                    // If an error occurred, break out of the loop
                    if (element_name != NULL) { // Free any pending element_name
                        deallocate(element_name, strlen(element_name) + 1);
                    }
                    break;
                }
            }
            if (element_name != NULL) { // Free any remaining element_name if loop exited due to error
                deallocate(element_name, strlen(element_name) + 1);
            }

            // After parsing elements, check closing tag
            if (success) {
                skipWhiteSpace(parser);
                if (!atChar(parser, '{')) {
                    printf("!!Closing value failed for Planet\n");
                    success = false;
                } else if (incChar(parser) == -1) { // Skip '{'
                    printf("!!Failed to skip opening brace for closing tag\n");
                    success = false;
                } else {
                    skipWhiteSpace(parser);
                    if (!atChar(parser, '#')) {
                        printf("!!Malformed closing element\n");
                        success = false;
                    } else if (incChar(parser) == -1) { // Skip '#'
                        printf("!!Failed to skip closing mark\n");
                        success = false;
                    } else {
                        int start_idx_tag;
                        getIndex(parser, &start_idx_tag);
                        int end_idx_tag = skipAlpha(parser);
                        if (end_idx_tag == -1) {
                            printf("!!Failed to locate end of closing element ID\n");
                            success = false;
                        } else {
                            char *closing_tag_name = copyData(parser, start_idx_tag, end_idx_tag);
                            if (closing_tag_name != NULL) {
                                if (strcmp(closing_tag_name, "Planet") == 0) {
                                    deallocate(closing_tag_name, strlen(closing_tag_name) + 1);
                                    skipWhiteSpace(parser);
                                    if (!atChar(parser, '}')) {
                                        printf("!!Failed to find final closing brace\n");
                                        success = false;
                                    } else {
                                        incChar(parser); // Skip '}'
                                        // Final success!
                                    }
                                } else {
                                    printf("!!Closing element malformed, expected 'Planet' but got '%s'\n", closing_tag_name);
                                    deallocate(closing_tag_name, strlen(closing_tag_name) + 1);
                                    success = false;
                                }
                            } else {
                                printf("!!Failed to copy closing tag name\n");
                                success = false;
                            }
                        }
                    }
                }
            }
        } else {
            printf("!!Failed to allocate Planet structure\n");
            return NULL;
        }
    }

    if (!success) {
        parser->current_index = initial_parser_index; // Rollback parser state
        printf("!!Error at: %s\n", parser->buffer + parser->current_index);
        if (new_planet != NULL) {
            freePlanet(new_planet);
            new_planet = NULL;
        }
    }
    return new_planet;
}

// Function: extractPeriod
LONGDOUBLE extractPeriod(ParserState *parser) {
    return extractValue(parser, "Period");
}

// Function: extractOrbitSpeed
LONGDOUBLE extractOrbitSpeed(ParserState *parser) {
    return extractValue(parser, "OrbitSpeed");
}

// Function: extractAphelion
LONGDOUBLE extractAphelion(ParserState *parser) {
    return extractValue(parser, "Aphelion");
}

// Function: extractPerihelion
LONGDOUBLE extractPerihelion(ParserState *parser) {
    return extractValue(parser, "Perihelion");
}

// Function: extractRadius
LONGDOUBLE extractRadius(ParserState *parser) {
    return extractValue(parser, "Radius");
}

// Function: extractERadius
LONGDOUBLE extractERadius(ParserState *parser) {
    return extractValue(parser, "ERadius");
}

// Function: extractMass
LONGDOUBLE extractMass(ParserState *parser) {
    return extractValue(parser, "Mass");
}

// Function: extractGravity
LONGDOUBLE extractGravity(ParserState *parser) {
    return extractValue(parser, "Gravity");
}