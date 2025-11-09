#include <stdio.h>   // For NULL, if needed for general program structure
#include <string.h>  // For strlen, strncmp
#include <stdlib.h>  // For strtol
#include <ctype.h>   // For isdigit, isprint, isalnum, isspace, ispunct

// Define a common integer type for boolean-like return values (0 or 1)
// and other integer parameters that were 'undefined4'.
// Typically, 'int' is used for this purpose in C.
typedef int boolean;

// Global arrays for validation functions
char *valid_education_levels[] = {
    "High School", "GED", "Associate", "Bachelor", "Master", "PhD", NULL
};
char *valid_eye_colors[] = {
    "Blue", "Green", "Brown", "Hazel", "Gray", NULL
};
char *valid_hair_colors[] = {
    "Black", "Brown", "Blonde", "Red", "Gray", "White", "Bald", NULL
};
char *valid_states[] = {
    "AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "FL", "GA",
    "HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD",
    "MA", "MI", "MN", "MS", "MO", "MT", "NE", "NV", "NH", "NJ",
    "NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "RI", "SC",
    "SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY", NULL
};
char *valid_suffixes[] = {
    "Jr.", "Sr.", "II", "III", "IV", NULL
};

// Function: contains
boolean contains(char **str_list, char *target_str) {
  char **current_str = str_list;
  while (*current_str != NULL) {
    size_t len = strlen(*current_str);
    // Check for exact match: same content AND same length
    if (strncmp(*current_str, target_str, len) == 0 && strlen(target_str) == len) {
      return 1;
    }
    current_str++;
  }
  return 0;
}

// Function: validate_edu
void validate_edu(char *param_1) {
  contains(valid_education_levels, param_1); // Original returned void, keeping it.
  return;
}

// Function: validate_eye_color
void validate_eye_color(char *param_1) {
  contains(valid_eye_colors, param_1); // Original returned void, keeping it.
  return;
}

// Function: validate_hair_color
void validate_hair_color(char *param_1) {
  contains(valid_hair_colors, param_1); // Original returned void, keeping it.
  return;
}

// Function: validate_state
void validate_state(char *param_1) {
  contains(valid_states, param_1); // Original returned void, keeping it.
  return;
}

// Function: validate_suffix
void validate_suffix(char *param_1) {
  contains(valid_suffixes, param_1); // Original returned void, keeping it.
  return;
}

// Function: within
boolean within(int min, int max, int value) {
  return (value >= min && value <= max);
}

// Function: all_digits
boolean all_digits(char *str) {
  for (size_t i = 0; str[i] != '\0'; i++) {
    if (!isdigit((unsigned char)str[i])) {
      return 0;
    }
  }
  return 1;
}

// Function: validate_day
boolean validate_day(char *str) {
  if (!all_digits(str)) {
    return 0;
  }
  long val = strtol(str, NULL, 10);
  return within(1, 31, (int)val);
}

// Function: validate_month
boolean validate_month(char *str) {
  if (!all_digits(str)) {
    return 0;
  }
  long val = strtol(str, NULL, 10);
  return within(1, 12, (int)val);
}

// Function: validate_year
boolean validate_year(char *str) {
  if (!all_digits(str)) {
    return 0;
  }
  long val = strtol(str, NULL, 10);
  return within(1900, 2014, (int)val); // 0x76c is 1900, 0x7de is 2014
}

// Function: validate_height_inches
boolean validate_height_inches(char *str) {
  if (!all_digits(str)) {
    return 0;
  }
  long val = strtol(str, NULL, 10);
  return within(0, 11, (int)val); // 0xb is 11
}

// Function: validate_height_feet
boolean validate_height_feet(char *str) {
  if (!all_digits(str)) {
    return 0;
  }
  long val = strtol(str, NULL, 10);
  return within(1, 9, (int)val);
}

// Function: validate_weight
boolean validate_weight(char *str) {
  if (!all_digits(str)) {
    return 0;
  }
  long val = strtol(str, NULL, 10);
  return within(50, 500, (int)val); // 0x32 is 50
}

// Function: validate_name
boolean validate_name(char *str) {
  for (size_t i = 0; str[i] != '\0'; i++) {
    if (!isprint((unsigned char)str[i]) && str[i] != '\'') {
      return 0;
    }
  }
  return 1;
}

// Function: validate_email
boolean validate_email(char *str) {
  size_t len = strlen(str);
  if (len < 3 || len > 64) { // 0x40 is 64
    return 0;
  }

  int at_count = 0;
  for (size_t i = 0; i < len; i++) {
    char c = str[i];
    if (c == '@') {
      if (at_count > 0) { // Already found an '@'
        return 0;
      }
      at_count = 1;
    } else {
      // Valid characters: alphanumeric, '.', '-', '_'
      if (!isalnum((unsigned char)c) && c != '.' && c != '-' && c != '_') {
        return 0;
      }
    }
  }
  return at_count; // Returns 1 if exactly one '@' was found, 0 otherwise
}

// Function: validate_street
boolean validate_street(char *str) {
  size_t len = strlen(str);
  if (len < 3 || len > 32) { // 0x21 is 33, so < 0x21 means <= 32. 2 < sVar1 means >= 3.
    return 0;
  }

  for (size_t i = 0; str[i] != '\0'; i++) {
    char c = str[i];
    if (!isalnum((unsigned char)c) && !isspace((unsigned char)c) && !ispunct((unsigned char)c)) {
      return 0;
    }
  }
  return 1;
}

// Function: validate_city
boolean validate_city(char *str) {
  size_t len = strlen(str);
  if (len < 4 || len > 32) { // 0x21 is 33, so < 0x21 means <= 32. 3 < sVar1 means >= 4.
    return 0;
  }

  for (size_t i = 0; str[i] != '\0'; i++) {
    char c = str[i];
    if (!isalnum((unsigned char)c) && !isspace((unsigned char)c) && !ispunct((unsigned char)c)) {
      return 0;
    }
  }
  return 1;
}

// Function: validate_zip
boolean validate_zip(char *str) {
  if (strlen(str) != 5) {
    return 0;
  }
  for (int i = 0; i < 5; i++) {
    if (!isdigit((unsigned char)str[i])) {
      return 0;
    }
  }
  return 1;
}

// Function: validate_phone
boolean validate_phone(char *str) {
  if (strlen(str) != 12) { // 0xc is 12
    return 0;
  }
  for (int i = 0; i < 12; i++) {
    if (i == 3 || i == 7) {
      if (str[i] != '-') {
        return 0;
      }
    } else {
      if (!isdigit((unsigned char)str[i])) {
        return 0;
      }
    }
  }
  return 1;
}

// Function: validate_sex
boolean validate_sex(char *str) {
  if (strlen(str) != 1) {
    return 0;
  }
  char c = str[0];
  return (c == 'M' || c == 'F');
}

// Function: validate_yes_no
boolean validate_yes_no(char *str) {
  if (strlen(str) != 1) {
    return 0;
  }
  char c = str[0];
  return (c == 'Y' || c == 'N');
}

// Function: validate_gpa
boolean validate_gpa(char *str) {
  if (strlen(str) != 4) {
    return 0;
  }
  // Expected format: X.XX
  if (!isdigit((unsigned char)str[0]) ||
      str[1] != '.' ||
      !isdigit((unsigned char)str[2]) ||
      !isdigit((unsigned char)str[3])) {
    return 0;
  }
  return 1;
}

// Structure definitions for init_af86
struct Field; // Forward declaration

struct Section {
    struct Section *prev_section;
    struct Section *next_section;
    struct Field *first_field;
};

struct Field {
    struct Field *prev_field;
    struct Field *next_field;
};

// Global instances of the structures
struct Section candidate, contact, address, education, employment, final, fin;
struct Field last_name, first_name, middle_name, suffix, dob_month, dob_day, dob_year, height_feet, height_inches, weight, sex, hair_color, eye_color;
struct Field pemail, wemail, hphone, wphone;
struct Field mim, miy, street, city, state, zip, owned;
struct Field highest_edu, esm, esy, eem, eey, school_name, estreet, ecity, estate, ezip, gpa, major;
struct Field employeer, wsm, wsy, wem, wey, wstreet, wcity, wstate, wzip, sup_last_name, sup_first_name, sup_title, sphone, semail;
struct Field drugz, background;

// Function: init_af86
void init_af86(void) {
  candidate.prev_section = NULL;
  candidate.next_section = &contact;
  contact.prev_section = &candidate;
  contact.next_section = &address;
  address.prev_section = &contact;
  address.next_section = &education;
  education.prev_section = &address;
  education.next_section = &employment;
  employment.prev_section = &education;
  employment.next_section = &final;
  final.prev_section = &employment;
  final.next_section = &fin;
  fin.prev_section = &final;
  fin.next_section = NULL;

  candidate.first_field = &last_name;
  contact.first_field = &pemail;
  address.first_field = &mim;
  education.first_field = &highest_edu;
  employment.first_field = &employeer;
  final.first_field = &drugz;

  last_name.prev_field = NULL;
  last_name.next_field = &first_name;
  first_name.prev_field = &last_name;
  first_name.next_field = &middle_name;
  middle_name.prev_field = &first_name;
  middle_name.next_field = &suffix;
  suffix.prev_field = &middle_name;
  suffix.next_field = &dob_month;
  dob_month.prev_field = &suffix;
  dob_month.next_field = &dob_day;
  dob_day.prev_field = &dob_month;
  dob_day.next_field = &dob_year;
  dob_year.prev_field = &dob_day;
  dob_year.next_field = &height_feet;
  height_feet.prev_field = &dob_year;
  height_feet.next_field = &height_inches;
  height_inches.prev_field = &height_feet;
  height_inches.next_field = &weight;
  weight.prev_field = &height_inches;
  weight.next_field = &sex;
  sex.prev_field = &weight;
  sex.next_field = &hair_color;
  hair_color.prev_field = &sex;
  hair_color.next_field = &eye_color;
  eye_color.prev_field = &hair_color;
  eye_color.next_field = NULL;

  pemail.prev_field = NULL;
  pemail.next_field = &wemail;
  wemail.prev_field = &pemail;
  wemail.next_field = &hphone;
  hphone.prev_field = &wemail;
  hphone.next_field = &wphone;
  wphone.prev_field = &hphone;
  wphone.next_field = NULL;

  mim.prev_field = NULL;
  mim.next_field = &miy;
  miy.prev_field = &mim;
  miy.next_field = &street;
  street.prev_field = &miy;
  street.next_field = &city;
  city.prev_field = &street;
  city.next_field = &state;
  state.prev_field = &city;
  state.next_field = &zip;
  zip.prev_field = &state;
  zip.next_field = &owned;
  owned.prev_field = &zip;
  owned.next_field = NULL;

  highest_edu.prev_field = NULL;
  highest_edu.next_field = &esm;
  esm.prev_field = &highest_edu;
  esm.next_field = &esy;
  esy.prev_field = &esm;
  esy.next_field = &eem;
  eem.prev_field = &esy;
  eem.next_field = &eey;
  eey.prev_field = &eem;
  eey.next_field = &school_name;
  school_name.prev_field = &eey;
  school_name.next_field = &estreet;
  estreet.prev_field = &school_name;
  estreet.next_field = &ecity;
  ecity.prev_field = &estreet;
  ecity.next_field = &estate;
  estate.prev_field = &ecity;
  estate.next_field = &ezip;
  ezip.prev_field = &estate;
  ezip.next_field = &gpa;
  gpa.prev_field = &ezip;
  gpa.next_field = &major;
  major.prev_field = &gpa;
  major.next_field = NULL;

  employeer.prev_field = NULL;
  employeer.next_field = &wsm;
  wsm.prev_field = &employeer;
  wsm.next_field = &wsy;
  wsy.prev_field = &wsm;
  wsy.next_field = &wem;
  wem.prev_field = &wsy;
  wem.next_field = &wey;
  wey.prev_field = &wem;
  wey.next_field = &wstreet;
  wstreet.prev_field = &wey;
  wstreet.next_field = &wcity;
  wcity.prev_field = &wstreet;
  wcity.next_field = &wstate;
  wstate.prev_field = &wcity;
  wstate.next_field = &wzip;
  wzip.prev_field = &wstate;
  wzip.next_field = &sup_last_name;
  sup_last_name.prev_field = &wzip;
  sup_last_name.next_field = &sup_first_name;
  sup_first_name.prev_field = &sup_last_name;
  sup_first_name.next_field = &sup_title;
  sup_title.prev_field = &sup_first_name;
  sup_title.next_field = &sphone;
  sphone.prev_field = &sup_title;
  sphone.next_field = &semail;
  semail.prev_field = &sphone;
  semail.next_field = NULL;

  drugz.prev_field = NULL;
  drugz.next_field = &background;
  background.prev_field = &drugz;
  background.next_field = NULL;
  return;
}