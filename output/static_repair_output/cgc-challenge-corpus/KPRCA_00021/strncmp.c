#include <stddef.h> // For size_t
#include <stdio.h>  // For printf

// Function: strncmp
int strncmp(char *__s1, char *__s2, size_t __n) {
    while (__n > 0 && *__s1 == *__s2 && *__s1 != '\0') {
        __s1++;
        __s2++;
        __n--;
    }

    if (__n == 0) {
        return 0;
    } else {
        // The original code returned (int)*__s2 - (int)*__s1.
        // Cast to unsigned char to ensure correct comparison for all char values,
        // especially when char is signed and characters have values > 127.
        return (int)(unsigned char)*__s2 - (int)(unsigned char)*__s1;
    }
}

// Main function to test strncmp
int main() {
    char s1[] = "hello world";
    char s2[] = "hello there";
    char s3[] = "hello";
    char s4[] = "hell";
    char s5[] = "hello world!";
    char s6[] = "";
    char s7[] = "a";
    char s8[] = "b";

    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: 0)\n", s1, s2, (size_t)5, strncmp(s1, s2, 5));
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: 0)\n", s1, s2, (size_t)6, strncmp(s1, s2, 6)); // "hello " vs "hello "
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: 0)\n", s1, s3, (size_t)5, strncmp(s1, s3, 5));
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: <0)\n", s3, s4, (size_t)5, strncmp(s3, s4, 5)); // 'o' vs '\0' => '\0' - 'o'
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: 0)\n", s3, s4, (size_t)4, strncmp(s3, s4, 4));
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: 0)\n", s1, s5, (size_t)11, strncmp(s1, s5, 11));
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: >0)\n", s1, s5, (size_t)12, strncmp(s1, s5, 12)); // '\0' vs '!' => '!' - '\0'
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: 0)\n", s1, s2, (size_t)0, strncmp(s1, s2, 0));
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: 0)\n", s6, s1, (size_t)0, strncmp(s6, s1, 0));
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: 0)\n", s6, s6, (size_t)1, strncmp(s6, s6, 1)); // '\0' vs '\0'
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: >0)\n", s7, s8, (size_t)1, strncmp(s7, s8, 1)); // 'a' vs 'b' => 'b' - 'a'
    printf("strncmp(\"%s\", \"%s\", %zu) = %d (Expected: <0)\n", s8, s7, (size_t)1, strncmp(s8, s7, 1)); // 'b' vs 'a' => 'a' - 'b'
    
    return 0;
}