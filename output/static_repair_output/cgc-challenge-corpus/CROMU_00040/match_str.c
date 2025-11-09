#include <stdbool.h> // For bool return type, or use int for 0/1
#include <stddef.h>  // For NULL

// Function: contains_wildcards
int contains_wildcards(const char *pattern) {
  while (*pattern != '\0') {
    if (*pattern == '*' || *pattern == '?') {
      return 1;
    }
    pattern++;
  }
  return 0;
}

// Function: match_str
int match_str(const char *pattern, const char *text) {
    const char *p = pattern;
    const char *t = text;
    const char *star_pattern_anchor = NULL; // Stores pattern pointer *after* a '*' for backtracking
    const char *star_text_anchor = NULL;    // Stores text pointer where the '*' started matching for backtracking

    while (true) {
        // If text is exhausted
        if (*t == '\0') {
            // Consume any remaining '*' at the end of the pattern
            while (*p == '*') {
                p++;
            }
            return *p == '\0'; // Match if pattern is also exhausted
        }

        // If pattern is exhausted but text is not
        if (*p == '\0') {
            if (star_pattern_anchor != NULL) {
                // Backtrack: reset pattern to after the last '*'
                p = star_pattern_anchor;
                // Advance text pointer: '*' matches one more character
                t = ++star_text_anchor;
                // Continue loop from the new position
                continue;
            } else {
                return 0; // No '*' to backtrack, definite mismatch
            }
        }

        // Handle pattern characters
        if (*p == '*') {
            // Specific `*` handling from the original code
            if (p[1] == '\0') {
                return 1; // '*' is the last character in pattern, matches everything
            }
            
            // Store this '*' position for potential future backtracking by the `else` block
            star_pattern_anchor = p + 1; // Character *after* '*'
            star_text_anchor = t;        // Text position where this '*' was encountered

            // Original logic for `*X`: greedily consume text until `X` is found.
            const char *next_char_after_star = p + 1;
            while (*t != '\0' && *next_char_after_star != *t) {
                t++;
            }
            if (*t == '\0') {
                return 0; // Text exhausted, but `next_char_after_star` not found
            }
            // Found a match for `next_char_after_star` at `*t`.
            p = next_char_after_star + 1; // Advance pattern past `*` and `next_char_after_star`
            t++;                          // Advance text past matched character
            
            // Original `break` condition: if both pattern and text ended here, it's a match.
            // The `(param_1 = local_8, local_8 == (char *)0x0)` part is a decompiler artifact and removed.
            if (*p == '\0' && *t == '\0') {
                return 1;
            }
            // If not both ended, continue the main loop with new p and t.
            continue; 
        } else if (*p == '?') {
            p++;
            t++;
        } else if (*p == *t) {
            p++;
            t++;
        } else { // Mismatch (non-wildcard)
            if (star_pattern_anchor != NULL) {
                // Backtrack: reset pattern to after the last '*'
                p = star_pattern_anchor;
                // Advance text pointer: '*' matches one more character
                t = ++star_text_anchor;
                // Continue loop from the new position
                continue;
            } else {
                return 0; // No '*' to backtrack, definite mismatch
            }
        }
    }
}