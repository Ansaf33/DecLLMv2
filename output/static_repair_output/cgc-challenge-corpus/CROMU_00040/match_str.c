#include <stdbool.h> // For bool type, or use int
#include <stddef.h>  // For NULL

// Function: match_str
int match_str(const char *pattern, const char *text) {
    const char *star_pattern_pos = NULL; // Stores pattern position after a '*'
    const char *star_text_pos = NULL;    // Stores text position corresponding to star_pattern_pos

    while (true) {
        // Case 1: Pattern exhausted
        if (*pattern == '\0') {
            // If text is also exhausted, it's a match
            if (*text == '\0') {
                return 1;
            }
            // If text is not exhausted, and we have a stored '*' for backtracking
            if (star_pattern_pos != NULL) {
                pattern = star_pattern_pos; // Reset pattern to the character after '*'
                star_text_pos++;            // Advance text from where '*' started matching
                text = star_text_pos;       // Reset text to the new position
                continue; // Continue with the updated pattern and text
            }
            return 0; // Pattern exhausted, text not, no '*' to backtrack
        }

        // Case 2: Text exhausted (pattern still has characters)
        if (*text == '\0') {
            // If remaining pattern characters are all '*', it's a match
            while (*pattern == '*') {
                pattern++;
            }
            return *pattern == '\0' ? 1 : 0;
        }

        // Case 3: Current characters match or '?' in pattern
        if (*pattern == *text || *pattern == '?') {
            pattern++;
            text++;
        }
        // Case 4: '*' in pattern
        else if (*pattern == '*') {
            star_pattern_pos = pattern + 1; // Store position after '*'
            star_text_pos = text;           // Store text position where '*' started matching
            pattern++;                      // Advance pattern past '*'
            // If '*' is at the end of the pattern, it matches everything remaining
            if (*pattern == '\0') {
                return 1;
            }
        }
        // Case 5: No match, and we have a stored '*' for backtracking
        else if (star_pattern_pos != NULL) {
            pattern = star_pattern_pos; // Reset pattern to the character after '*'
            star_text_pos++;            // Advance text from where '*' started matching
            text = star_text_pos;       // Reset text to the new position
        }
        // Case 6: No match, and no '*' to backtrack
        else {
            return 0; // No match
        }
    }
}

// Function: contains_wildcards
int contains_wildcards(const char *s) {
  while (*s != '\0') {
    if (*s == '*' || *s == '?') {
      return 1;
    }
    s++;
  }
  return 0;
}