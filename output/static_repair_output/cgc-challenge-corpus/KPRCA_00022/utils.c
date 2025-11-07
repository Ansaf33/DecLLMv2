#include <stdio.h>    // For printf in main
#include <string.h>   // For strlen, strcmp, memcmp
#include <stdlib.h>   // For strtol
#include <ctype.h>    // For isalpha, isalnum, isdigit, isprint

// Define global arrays for choices. These were missing definitions in the original snippet.
// The sizes are also defined for convenience.
const char *valid_hair_colors[] = {
    "Black", "Brown", "Blonde", "Red", "Gray", "White", "Other"
};
const unsigned int NUM_VALID_HAIR_COLORS = sizeof(valid_hair_colors) / sizeof(valid_hair_colors[0]);

const char *valid_eye_colors[] = {
    "Blue", "Green", "Brown", "Hazel", "Gray", "Other"
};
const unsigned int NUM_VALID_EYE_COLORS = sizeof(valid_eye_colors) / sizeof(valid_eye_colors[0]);

const char *valid_suffixes[] = {
    "Jr.", "Sr.", "II", "III", "IV"
};
const unsigned int NUM_VALID_SUFFIXES = sizeof(valid_suffixes) / sizeof(valid_suffixes[0]);

const char *valid_states[] = {
    "AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "FL", "GA",
    "HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD",
    "MA", "MI", "MN", "MS", "MO", "MT", "NE", "NV", "NH", "NJ",
    "NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "RI", "SC",
    "SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY",
    "DC" // Added DC for common state list
};
const unsigned int NUM_VALID_STATES = sizeof(valid_states) / sizeof(valid_states[0]);

const char *education_levels[] = {
    "High School", "Bachelors", "Masters", "PhD"
};
const unsigned int NUM_EDUCATION_LEVELS = sizeof(education_levels) / sizeof(education_levels[0]);


// Function: verify_choice
// param_1: An array of string pointers (const char * const *).
// param_2: The input string to verify against the choices (const char *).
// param_3: The number of choices in the array (unsigned int).
// Returns 0 on success (match found), -1 on failure (no match).
int verify_choice(const char * const *choices, const char *input, unsigned int num_choices) {
  for (unsigned int i = 0; i < num_choices; ++i) {
    if (strcmp(choices[i], input) == 0) {
      return 0; // Found an exact match
    }
  }
  return -1; // No match found
}

// Function: verify_hair
// param_1: The hair color string to verify.
// Returns 0 on success, -1 on failure.
int verify_hair(const char *param_1) {
  return verify_choice(valid_hair_colors, param_1, NUM_VALID_HAIR_COLORS);
}

// Function: verify_eye
// param_1: The eye color string to verify.
// Returns 0 on success, -1 on failure.
int verify_eye(const char *param_1) {
  return verify_choice(valid_eye_colors, param_1, NUM_VALID_EYE_COLORS);
}

// Function: verify_suffix
// param_1: The suffix string to verify.
// Returns 0 on success, -1 on failure.
int verify_suffix(const char *param_1) {
  return verify_choice(valid_suffixes, param_1, NUM_VALID_SUFFIXES);
}

// Function: verify_state
// param_1: The state abbreviation string to verify.
// Returns 0 on success, -1 on failure.
int verify_state(const char *param_1) {
  return verify_choice(valid_states, param_1, NUM_VALID_STATES);
}

// Function: verify_education
// param_1: The education level string to verify.
// Returns 0 on success, -1 on failure.
int verify_education(const char *param_1) {
  return verify_choice(education_levels, param_1, NUM_EDUCATION_LEVELS);
}

// Function: verify_month
// param_1: String containing the month number.
// param_2: Expected total length of the string including the null terminator.
// Returns 0 on success, -1 on failure.
int verify_month(char *param_1, int param_2) {
  if (strlen(param_1) + 1 != (size_t)param_2) {
    return -1;
  }
  long val = strtol(param_1, NULL, 10);
  if (val < 1 || val > 12) {
    return -1;
  }
  return 0;
}

// Function: verify_day
// param_1: String containing the day number.
// param_2: Expected total length of the string including the null terminator.
// Returns 0 on success, -1 on failure.
int verify_day(char *param_1, int param_2) {
  if (strlen(param_1) + 1 != (size_t)param_2) {
    return -1;
  }
  long val = strtol(param_1, NULL, 10);
  if (val < 1 || val > 31) {
    return -1;
  }
  return 0;
}

// Function: verify_year
// param_1: String containing the year number.
// param_2: Expected total length of the string including the null terminator.
// Returns 0 on success, -1 on failure.
int verify_year(char *param_1, int param_2) {
  if (strlen(param_1) + 1 != (size_t)param_2) {
    return -1;
  }
  long val = strtol(param_1, NULL, 10);
  if (val < 1900 || val > 2014) { // Original constants 0x76c (1900) and 0x7de (2014)
    return -1;
  }
  return 0;
}

// Function: verify_height_feet
// param_1: String containing the height in feet.
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_height_feet(char *param_1, unsigned int param_2) {
  if (param_2 < strlen(param_1) + 1) { // String is too long for the buffer
    return -1;
  }
  long val = strtol(param_1, NULL, 10);
  if (val < 1 || val > 9) {
    return -1;
  }
  return 0;
}

// Function: verify_height_inches
// param_1: String containing the height in inches.
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_height_inches(char *param_1, unsigned int param_2) {
  if (param_2 < strlen(param_1) + 1) { // String is too long for the buffer
    return -1;
  }
  long val = strtol(param_1, NULL, 10);
  if (val < 0 || val > 11) { // Original constant 0xb (11)
    return -1;
  }
  return 0;
}

// Function: verify_weight
// param_1: String containing the weight.
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_weight(char *param_1, unsigned int param_2) {
  if (param_2 < strlen(param_1) + 1) { // String is too long for the buffer
    return -1;
  }
  long val = strtol(param_1, NULL, 10);
  if (val < 50 || val > 500) { // Original constant 0x32 (50)
    return -1;
  }
  return 0;
}

// Function: verify_sex
// param_1: String containing the sex ('M' or 'F').
// param_2: Expected total length of the string including the null terminator.
// Returns 0 on success, -1 on failure.
int verify_sex(char *param_1, int param_2) {
  if (strlen(param_1) + 1 != (size_t)param_2) {
    return -1;
  }
  if (*param_1 == 'M' || *param_1 == 'F') {
    return 0;
  }
  return -1;
}

// Function: verify_yes_no
// param_1: String containing 'Y' or 'N'.
// param_2: Expected total length of the string including the null terminator.
// Returns 0 on success, -1 on failure.
int verify_yes_no(char *param_1, int param_2) {
  if (strlen(param_1) + 1 != (size_t)param_2) {
    return -1;
  }
  if (*param_1 == 'Y' || *param_1 == 'N') {
    return 0;
  }
  return -1;
}

// Function: verify_name
// param_1: String containing a name.
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_name(char *param_1, unsigned int param_2) {
  size_t len = strlen(param_1);
  if (param_2 < len + 1 || len + 1 < 2) { // String too long or too short (min 1 char + null)
    return -1;
  }
  for (unsigned int i = 0; i < len; ++i) {
    if (!isalpha((int)param_1[i]) && param_1[i] != '\'') {
      return -1;
    }
  }
  return 0;
}

// Function: verify_street
// param_1: String containing a street address.
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_street(char *param_1, unsigned int param_2) {
  size_t len = strlen(param_1);
  if (param_2 < len + 1 || len + 1 < 5) { // String too long or too short (min 4 chars + null)
    return -1;
  }
  for (unsigned int i = 0; i < len; ++i) {
    char c = param_1[i];
    if (!isalnum((int)c) && c != '\'' && c != ' ' && c != ',') {
      return -1;
    }
  }
  return 0;
}

// Function: verify_city
// param_1: String containing a city name.
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_city(char *param_1, unsigned int param_2) {
  size_t len = strlen(param_1);
  if (param_2 < len + 1 || len + 1 < 5) { // String too long or too short (min 4 chars + null)
    return -1;
  }
  for (unsigned int i = 0; i < len; ++i) {
    char c = param_1[i];
    if (!isalpha((int)c) && c != '\'' && c != ' ') {
      return -1;
    }
  }
  return 0;
}

// Function: verify_zip_code
// param_1: String containing a zip code.
// param_2: Expected total length of the string including the null terminator.
// Returns 0 on success, -1 on failure.
int verify_zip_code(char *param_1, int param_2) {
  size_t len = strlen(param_1);
  if (len + 1 != (size_t)param_2) {
    return -1;
  }
  if (*param_1 == '0') { // Specific rule: zip codes don't start with '0'
    return -1;
  }
  for (unsigned int i = 0; i < len; ++i) {
    if (!isdigit((int)param_1[i])) {
      return -1;
    }
  }
  return 0;
}

// Function: verify_gpa
// param_1: String containing a GPA (e.g., "3.5").
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_gpa(char *param_1, unsigned int param_2) {
  size_t len = strlen(param_1);
  if (param_2 < len + 1) { // String too long for the buffer
    return -1;
  }
  // This logic implies a format like D.D or D.DD etc. where the second character must be a dot.
  for (unsigned int i = 0; i < len; ++i) {
    if (i == 1) {
      if (param_1[i] != '.') {
        return -1;
      }
    } else {
      if (!isdigit((int)param_1[i])) {
        return -1;
      }
    }
  }
  return 0;
}

// Function: verify_email
// param_1: String containing an email address.
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_email(char *param_1, unsigned int param_2) {
  size_t len = strlen(param_1);
  int at_count = 0;
  if (param_2 < len + 1 || len + 1 < 3) { // String too long or too short (e.g., "a@b" is min 2 chars + null)
    return -1;
  }
  for (unsigned int i = 0; i < len; ++i) {
    char c = param_1[i];
    if (!isalnum((int)c) && c != '@' && c != '-' && c != '.' && c != '_') {
      return -1;
    }
    if (c == '@') {
      at_count++;
      if (at_count == 2) { // More than one '@' character is not allowed
        return -1;
      }
    }
  }
  if (at_count == 1) { // Must contain exactly one '@'
    return 0;
  }
  return -1;
}

// Function: verify_phone
// param_1: String containing a phone number (e.g., "123-456-7890").
// param_2: Expected total length of the string including the null terminator.
// Returns 0 on success, -1 on failure.
int verify_phone(char *param_1, int param_2) {
  if (strlen(param_1) + 1 != (size_t)param_2) {
    return -1;
  }
  for (unsigned int i = 0; i < strlen(param_1); ++i) {
    if (i == 3 || i == 7) {
      if (param_1[i] != '-') {
        return -1;
      }
    } else {
      if (!isdigit((int)param_1[i])) {
        return -1;
      }
    }
  }
  return 0;
}

// Function: verify_number
// param_1: String containing an integer number.
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_number(char *param_1, unsigned int param_2) {
  if (param_2 < strlen(param_1) + 1) { // String too long for the buffer
    return -1;
  }
  for (unsigned int i = 0; i < strlen(param_1); ++i) {
    if (!isdigit((int)param_1[i])) {
      return -1;
    }
  }
  return 0;
}

// Function: verify_text
// param_1: String containing generic printable text.
// param_2: Maximum allowed buffer size for the string (including null terminator).
// Returns 0 on success, -1 on failure.
int verify_text(char *param_1, unsigned int param_2) {
  if (param_2 < strlen(param_1) + 1) { // String too long for the buffer
    return -1;
  }
  for (unsigned int i = 0; i < strlen(param_1); ++i) {
    if (!isprint((int)param_1[i])) {
      return -1;
    }
  }
  return 0;
}

// Main function to demonstrate the usage of the verification functions.
int main() {
    printf("--- Verification Tests ---\n");

    // Test verify_choice and related functions
    printf("Verify Hair 'Brown': %s\n", verify_hair("Brown") == 0 ? "Success" : "Failure");
    printf("Verify Hair 'Purple': %s\n", verify_hair("Purple") == 0 ? "Success" : "Failure");
    printf("Verify State 'CA': %s\n", verify_state("CA") == 0 ? "Success" : "Failure");
    printf("Verify State 'XX': %s\n", verify_state("XX") == 0 ? "Success" : "Failure");

    // Test verify_month
    printf("Verify Month '6' (len 2): %s\n", verify_month("6", 2) == 0 ? "Success" : "Failure");
    printf("Verify Month '13' (len 3): %s\n", verify_month("13", 3) == 0 ? "Success" : "Failure");
    printf("Verify Month 'June' (len 5): %s\n", verify_month("June", 5) == 0 ? "Success" : "Failure");

    // Test verify_year
    printf("Verify Year '1990' (len 5): %s\n", verify_year("1990", 5) == 0 ? "Success" : "Failure");
    printf("Verify Year '1899' (len 5): %s\n", verify_year("1899", 5) == 0 ? "Success" : "Failure");

    // Test verify_height_feet
    printf("Verify Height Feet '5' (buf 2): %s\n", verify_height_feet("5", 2) == 0 ? "Success" : "Failure");
    printf("Verify Height Feet '10' (buf 3): %s\n", verify_height_feet("10", 3) == 0 ? "Success" : "Failure");

    // Test verify_sex
    printf("Verify Sex 'M' (len 2): %s\n", verify_sex("M", 2) == 0 ? "Success" : "Failure");
    printf("Verify Sex 'X' (len 2): %s\n", verify_sex("X", 2) == 0 ? "Success" : "Failure");

    // Test verify_name
    printf("Verify Name 'John' (buf 5): %s\n", verify_name("John", 5) == 0 ? "Success" : "Failure");
    printf("Verify Name 'O''Malley' (buf 9): %s\n", verify_name("O''Malley", 9) == 0 ? "Success" : "Failure");
    printf("Verify Name 'John123' (buf 8): %s\n", verify_name("John123", 8) == 0 ? "Success" : "Failure");

    // Test verify_email
    printf("Verify Email 'test@example.com' (buf 17): %s\n", verify_email("test@example.com", 17) == 0 ? "Success" : "Failure");
    printf("Verify Email 'bad@email@com' (buf 14): %s\n", verify_email("bad@email@com", 14) == 0 ? "Success" : "Failure");
    printf("Verify Email 'no_at.com' (buf 10): %s\n", verify_email("no_at.com", 10) == 0 ? "Success" : "Failure");

    // Test verify_phone
    printf("Verify Phone '123-456-7890' (len 13): %s\n", verify_phone("123-456-7890", 13) == 0 ? "Success" : "Failure");
    printf("Verify Phone '1234567890' (len 11): %s\n", verify_phone("1234567890", 11) == 0 ? "Success" : "Failure");

    // Test verify_gpa
    printf("Verify GPA '3.5' (buf 4): %s\n", verify_gpa("3.5", 4) == 0 ? "Success" : "Failure");
    printf("Verify GPA '3,5' (buf 4): %s\n", verify_gpa("3,5", 4) == 0 ? "Success" : "Failure");
    printf("Verify GPA '3.55' (buf 5): %s\n", verify_gpa("3.55", 5) == 0 ? "Success" : "Failure"); // Should pass based on original logic

    return 0;
}