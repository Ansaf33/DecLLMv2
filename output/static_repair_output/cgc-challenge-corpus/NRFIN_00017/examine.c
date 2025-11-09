#include <stdbool.h> // For bool type

// Define DAT_00014030, assuming it's a derating factor like 0.8 for continuous loads.
// Adjust this value if the actual constant is different in the original context.
const double DERATING_FACTOR = 0.8;

// --- Stub Declarations for undefined functions ---
// These declarations are necessary for the code to compile.
// Replace them with actual function definitions if available.

// Load Center functions
double get_total_amp_load_on_load_center(void);
int get_amp_rating_of_load_center(void);

// Breaker functions
double get_total_amp_load_on_breaker_by_breaker_id(int breaker_id);
int get_amp_rating_of_breaker(int breaker_id); // Changed return type to int for clarity
unsigned int get_count_outlets_on_breaker(int breaker_id);

// Outlet functions
double get_total_amp_load_on_outlet_by_outlet_id(int outlet_id);
int get_amp_rating_of_outlet(int outlet_id); // Changed return type to int for clarity
double get_max_receptacle_amp_load_on_outlet_by_outlet_id(int outlet_id);

// Splitter functions
double get_total_amp_load_on_splitter_by_splitter_id(int splitter_id);
int get_amp_rating_of_splitter(int splitter_id); // Changed return type to int for clarity
double get_max_receptacle_amp_load_on_splitter_by_splitter_id(int splitter_id);

// Light String functions
double get_total_amp_load_on_light_string_by_light_string_id(int light_string_id);
double get_amp_rating_of_light_string(int light_string_id); // Kept as double due to long double casting

// Receptacle functions
double get_total_amp_load_on_receptacle_by_receptacle_id(int receptacle_id);
int get_amp_rating_of_receptacle(int receptacle_id); // Changed return type to int for clarity

// --- Fixed Functions ---

// Function: is_too_much_amp_load_on_load_center
bool is_too_much_amp_load_on_load_center(void) {
  return (float)get_amp_rating_of_load_center() < (float)get_total_amp_load_on_load_center();
}

// Function: is_too_much_amp_load_on_breaker
bool is_too_much_amp_load_on_breaker(int breaker_id) {
  return DERATING_FACTOR * (double)get_amp_rating_of_breaker(breaker_id) < (double)get_total_amp_load_on_breaker_by_breaker_id(breaker_id);
}

// Function: is_too_many_outlets_on_breaker
bool is_too_many_outlets_on_breaker(int breaker_id) {
  unsigned int outlet_count = get_count_outlets_on_breaker(breaker_id);
  int amp_rating = get_amp_rating_of_breaker(breaker_id); // Using int as per stub

  if (amp_rating == 15) {
    return outlet_count > 8;
  } else if (amp_rating == 20) {
    return outlet_count > 10;
  }
  return false;
}

// Function: is_too_much_amp_load_on_outlet
bool is_too_much_amp_load_on_outlet(int outlet_id) {
  return DERATING_FACTOR * (double)get_amp_rating_of_outlet(outlet_id) < (double)get_total_amp_load_on_outlet_by_outlet_id(outlet_id);
}

// Function: is_too_much_amp_load_on_one_outlet_receptacle
bool is_too_much_amp_load_on_one_outlet_receptacle(int outlet_id) {
  return DERATING_FACTOR * (double)get_amp_rating_of_outlet(outlet_id) < (double)get_max_receptacle_amp_load_on_outlet_by_outlet_id(outlet_id);
}

// Function: is_too_much_amp_load_on_splitter
bool is_too_much_amp_load_on_splitter(int splitter_id) {
  return DERATING_FACTOR * (double)get_amp_rating_of_splitter(splitter_id) < (double)get_total_amp_load_on_splitter_by_splitter_id(splitter_id);
}

// Function: is_too_much_amp_load_on_one_splitter_receptacle
bool is_too_much_amp_load_on_one_splitter_receptacle(int splitter_id) {
  return DERATING_FACTOR * (double)get_amp_rating_of_splitter(splitter_id) < (double)get_max_receptacle_amp_load_on_splitter_by_splitter_id(splitter_id);
}

// Function: is_too_much_amp_load_on_light_string
bool is_too_much_amp_load_on_light_string(int light_string_id) {
  // Retaining the original explicit float cast before long double for precision matching
  return (long double)get_amp_rating_of_light_string(light_string_id) < (long double)(float)get_total_amp_load_on_light_string_by_light_string_id(light_string_id);
}

// Function: is_too_much_amp_load_on_receptacle
bool is_too_much_amp_load_on_receptacle(int receptacle_id) {
  return (float)get_amp_rating_of_receptacle(receptacle_id) < (float)get_total_amp_load_on_receptacle_by_receptacle_id(receptacle_id);
}

// Function: is_electrical_panel_overloaded
// Changed return type to bool to reflect the result of the called function.
bool is_electrical_panel_overloaded(void) {
  return is_too_much_amp_load_on_load_center();
}

// Function: is_breaker_overloaded
// Changed return type to bool and simplified logic.
bool is_breaker_overloaded(int breaker_id) {
  return is_too_many_outlets_on_breaker(breaker_id) || is_too_much_amp_load_on_breaker(breaker_id);
}

// Function: is_outlet_overloaded
// Changed return type to bool and simplified logic.
bool is_outlet_overloaded(int outlet_id) {
  return is_too_much_amp_load_on_outlet(outlet_id) || is_too_much_amp_load_on_one_outlet_receptacle(outlet_id);
}

// Function: is_splitter_overloaded
// Changed return type to bool and simplified logic.
bool is_splitter_overloaded(int splitter_id) {
  return is_too_much_amp_load_on_splitter(splitter_id) || is_too_much_amp_load_on_one_splitter_receptacle(splitter_id);
}

// Function: is_light_string_overloaded
bool is_light_string_overloaded(int light_string_id) {
  return is_too_much_amp_load_on_light_string(light_string_id);
}

// Function: is_receptacle_overloaded
bool is_receptacle_overloaded(int receptacle_id) {
  return is_too_much_amp_load_on_receptacle(receptacle_id);
}