#include <stdio.h>    // For fputs, stdout
#include <string.h>   // For strlen, memcpy
#include <stdbool.h>  // For bool
#include <stdint.h>   // For standard integer types if specific sizes are needed, but not strictly for this code
#include <libintl.h>  // For gettext
#include <error.h>    // For error
#include <errno.h>    // For ERANGE

// Function: path_common_prefix
int path_common_prefix(const char *param_1, const char *param_2) {
  int current_len = 0;
  int return_len = 0;

  // The condition `(param_2[1] == '/') == (param_1[1] == '/')` checks if the second character
  // of both paths are either both '/' or both not '/'. This is an unusual path comparison logic.
  // Preserving the original behavior.
  if ((param_2[1] == '/') == (param_1[1] == '/')) {
    while (param_1[current_len] != '\0' && param_2[current_len] != '\0' && param_1[current_len] == param_2[current_len]) {
      if (param_1[current_len] == '/') {
        return_len = current_len + 1; // Store length *after* the common slash
      }
      current_len++;
    }

    // If the common prefix ends exactly at the end of one or both strings,
    // or at a slash boundary on the longer string, use the full current_len.
    if ((param_1[current_len] == '\0' && param_2[current_len] == '\0') ||
        (param_1[current_len] == '\0' && param_2[current_len] == '/') ||
        (param_2[current_len] == '\0' && param_1[current_len] == '/')) {
      return_len = current_len; // Use the full common prefix length
    }
  } else {
    return_len = 0; // Initial condition not met, no common prefix
  }
  return return_len;
}

// Function: buffer_or_output
// Changed undefined8 to int, ulong to unsigned long, void** for buffer pointer
int buffer_or_output(const char *str, void **buffer_ptr, unsigned long *buffer_size) {
  if (*buffer_ptr == NULL) {
    fputs(str, stdout); // Use standard fputs and stdout
  } else {
    size_t len = strlen(str);
    // Check if remaining buffer size is not enough for string + null terminator
    // The original code checks `*param_3 <= sVar1`, where sVar1 is strlen.
    // This implies it needs `len + 1` bytes and returns error if `*buffer_size` is exactly `len`.
    if (*buffer_size <= len) {
      return 1; // Buffer too small
    }
    memcpy(*buffer_ptr, str, len + 1); // Copy string including null terminator
    *buffer_ptr = (void *)((char *)*buffer_ptr + len); // Advance pointer to the start of the null terminator
    *buffer_size -= len; // Reduce remaining size by string length
  }
  return 0; // Success
}

// Function: relpath
// Changed long to const char*, undefined8 to void** and unsigned long*, bool for return type.
bool relpath(const char *param_1, const char *param_2, void **buffer_ptr, unsigned long *buffer_size) {
  bool error_occurred = false;
  int common_prefix_len;
  const char *path_src_suffix;  // Part of param_2 after common prefix
  const char *path_dest_suffix; // Part of param_1 after common prefix

  common_prefix_len = path_common_prefix(param_2, param_1);

  if (common_prefix_len == 0) {
    return false; // No common prefix, cannot make relative path
  }

  path_src_suffix = param_2 + common_prefix_len;
  path_dest_suffix = param_1 + common_prefix_len;

  // Skip leading slashes if present at the common prefix boundary
  if (*path_src_suffix == '/') {
    path_src_suffix++;
  }
  if (*path_dest_suffix == '/') {
    path_dest_suffix++;
  }

  if (*path_src_suffix == '\0') {
    // If source path ends at common prefix (or is just '/'), output destination suffix or "."
    const char *output_str = path_dest_suffix;
    if (*path_dest_suffix == '\0') {
      output_str = ".";
    }
    error_occurred |= (buffer_or_output(output_str, buffer_ptr, buffer_size) != 0);
  } else {
    // If source path has remaining segments, generate "up" parts and then "down" parts.
    // The original code outputs "." first. This initial "." is likely overwritten by subsequent
    // "../" if `path_src_suffix` contains any slashes, due to how `buffer_or_output` advances the pointer.
    // Preserving this specific behavior.
    error_occurred |= (buffer_or_output(".", buffer_ptr, buffer_size) != 0); // Original DAT_0010038e is "."

    for (const char *p = path_src_suffix; *p != '\0'; p++) {
      if (*p == '/') {
        error_occurred |= (buffer_or_output("../", buffer_ptr, buffer_size) != 0); // Original DAT_00100391 is "../"
      }
    }

    if (*path_dest_suffix != '\0') {
      error_occurred |= (buffer_or_output("/", buffer_ptr, buffer_size) != 0); // Original DAT_00100395 is "/"
      error_occurred |= (buffer_or_output(path_dest_suffix, buffer_ptr, buffer_size) != 0);
    }
  }

  if (error_occurred) {
    // Original error call: error(0, 0x24, &DAT_001003b2, uVar3);
    // Assuming 0x24 is ERANGE from errno.h, and DAT_001003b2 is a format string.
    // Using a placeholder format string for compilation.
    error(0, ERANGE, "relpath.c:?: %s", gettext("generating relative path"));
  }

  return !error_occurred; // Return true if no error, false otherwise
}