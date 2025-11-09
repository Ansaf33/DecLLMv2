#include <stdio.h>   // For printf
#include <stdbool.h> // For bool type
#include <string.h>  // For strlen
#include <stdlib.h>  // For EXIT_SUCCESS

// Function: get_str_end
// Finds the end of a string or the first newline character.
// If a newline is found, it's replaced with a null terminator.
// Returns the index of the null terminator or the modified newline.
int get_str_end(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            str[i] = '\0'; // Replace newline with null terminator
            return i;
        }
        i++;
    }
    return i; // Return length if no newline found
}

// Function: reverse
// Reverses a string in-place.
// str: Pointer to the string.
// len: Length of the string (excluding null terminator).
void reverse(char *str, int len) {
    int left = 0;
    int right = len - 1;
    while (left < right) {
        char temp = str[left];
        str[left] = str[right];
        str[right] = temp;
        left++;
        right--;
    }
}

// Function: itos (integer to string)
// Converts an integer to its string representation.
// value: The integer value to convert.
// buffer: A character buffer to store the string. Must be large enough.
// Returns 1 on success.
int itos(int value, char *buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }

    bool is_negative = false;
    if (value < 0) {
        is_negative = true;
        value = -value;
    }

    int i = 0;
    // Extract digits in reverse order
    while (value != 0) {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }

    // Add negative sign if applicable
    if (is_negative) {
        buffer[i++] = '-';
    }

    // Reverse the string to get correct order
    reverse(buffer, i);
    buffer[i] = '\0'; // Null-terminate the string
    return 1;
}

// Function: stoi (string to integer)
// Converts a string of digits to an integer.
// str: Pointer to the string.
// length_read: Pointer to an int where the number of parsed digits will be stored.
// Returns the converted integer value.
int stoi(const char *str, int *length_read) {
    int i = 0;
    // Determine the number of leading digits
    while (str[i] >= '0' && str[i] <= '9') {
        i++;
    }
    *length_read = i; // Store the count of digits read

    int result = 0;
    // Convert digits to integer value
    for (int j = 0; j < i; j++) {
        result = result * 10 + (str[j] - '0');
    }
    return result;
}

// Main function to demonstrate usage
int main() {
    // --- Test get_str_end ---
    char s1[] = "Hello World\nThis is a test.";
    char s2[] = "Another string without newline.";
    char s3[] = "\nStarts with newline.";
    char s4[] = "";

    printf("--- Testing get_str_end ---\n");
    printf("Original s1: \"%s\"\n", s1);
    int end1 = get_str_end(s1);
    printf("s1 after get_str_end (length %d): \"%s\"\n", end1, s1);

    printf("Original s2: \"%s\"\n", s2);
    int end2 = get_str_end(s2);
    printf("s2 after get_str_end (length %d): \"%s\"\n", end2, s2);

    printf("Original s3: \"%s\"\n", s3);
    int end3 = get_str_end(s3);
    printf("s3 after get_str_end (length %d): \"%s\"\n", end3, s3);

    printf("Original s4: \"%s\"\n", s4);
    int end4 = get_str_end(s4);
    printf("s4 after get_str_end (length %d): \"%s\"\n", end4, s4);


    // --- Test reverse ---
    char rev_str1[] = "hello";
    char rev_str2[] = "world";
    char rev_str3[] = "a";
    char rev_str4[] = "";
    char rev_str5[] = "ab";

    printf("\n--- Testing reverse ---\n");
    printf("Original: \"%s\", Length: %lu\n", rev_str1, strlen(rev_str1));
    reverse(rev_str1, strlen(rev_str1));
    printf("Reversed: \"%s\"\n", rev_str1);

    printf("Original: \"%s\", Length: %lu\n", rev_str2, strlen(rev_str2));
    reverse(rev_str2, strlen(rev_str2));
    printf("Reversed: \"%s\"\n", rev_str2);

    printf("Original: \"%s\", Length: %lu\n", rev_str3, strlen(rev_str3));
    reverse(rev_str3, strlen(rev_str3));
    printf("Reversed: \"%s\"\n", rev_str3);

    printf("Original: \"%s\", Length: %lu\n", rev_str4, strlen(rev_str4));
    reverse(rev_str4, strlen(rev_str4));
    printf("Reversed: \"%s\"\n", rev_str4);

    printf("Original: \"%s\", Length: %lu\n", rev_str5, strlen(rev_str5));
    reverse(rev_str5, strlen(rev_str5));
    printf("Reversed: \"%s\"\n", rev_str5);


    // --- Test itos ---
    char buffer[32]; // Sufficiently large buffer for int values
    int num1 = 12345;
    int num2 = -6789;
    int num3 = 0;
    int num4 = 1;
    int num5 = -1;
    int num6 = 2147483647; // INT_MAX
    int num7 = -123; // Example: -123

    printf("\n--- Testing itos ---\n");
    itos(num1, buffer);
    printf("itos(%d) -> \"%s\"\n", num1, buffer);

    itos(num2, buffer);
    printf("itos(%d) -> \"%s\"\n", num2, buffer);

    itos(num3, buffer);
    printf("itos(%d) -> \"%s\"\n", num3, buffer);

    itos(num4, buffer);
    printf("itos(%d) -> \"%s\"\n", num4, buffer);

    itos(num5, buffer);
    printf("itos(%d) -> \"%s\"\n", num5, buffer);

    itos(num6, buffer);
    printf("itos(%d) -> \"%s\"\n", num6, buffer);

    itos(num7, buffer);
    printf("itos(%d) -> \"%s\"\n", num7, buffer);


    // --- Test stoi ---
    const char *str_num1 = "12345abc";
    const char *str_num2 = "0";
    const char *str_num3 = "987";
    const char *str_num4 = "hello";
    const char *str_num5 = "2147483647"; // INT_MAX as string

    int parsed_len;
    int s_val;

    printf("\n--- Testing stoi ---\n");
    s_val = stoi(str_num1, &parsed_len);
    printf("stoi(\"%s\") -> Value: %d, Parsed Length: %d\n", str_num1, s_val, parsed_len);

    s_val = stoi(str_num2, &parsed_len);
    printf("stoi(\"%s\") -> Value: %d, Parsed Length: %d\n", str_num2, s_val, parsed_len);

    s_val = stoi(str_num3, &parsed_len);
    printf("stoi(\"%s\") -> Value: %d, Parsed Length: %d\n", str_num3, s_val, parsed_len);

    s_val = stoi(str_num4, &parsed_len);
    printf("stoi(\"%s\") -> Value: %d, Parsed Length: %d\n", str_num4, s_val, parsed_len);

    s_val = stoi(str_num5, &parsed_len);
    printf("stoi(\"%s\") -> Value: %d, Parsed Length: %d\n", str_num5, s_val, parsed_len);


    return EXIT_SUCCESS;
}