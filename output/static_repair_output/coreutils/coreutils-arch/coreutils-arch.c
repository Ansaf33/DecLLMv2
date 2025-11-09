// Fix missing headers (none strictly needed for this snippet's direct functionality, but standard types are used)

// Global variable declaration for _uname_mode
// Assuming it's an integer type based on assignment.
int _uname_mode;

// Forward declaration for single_binary_main_uname
// Assuming undefined4 maps to unsigned int and undefined8 maps to unsigned long long
void single_binary_main_uname(unsigned int param_1, unsigned long long param_2);

// Function: single_binary_main_arch
// Replacing undefined4 with unsigned int and undefined8 with unsigned long long
void single_binary_main_arch(unsigned int param_1, unsigned long long param_2) {
  _uname_mode = 2;
  single_binary_main_uname(param_1, param_2);
  return;
}