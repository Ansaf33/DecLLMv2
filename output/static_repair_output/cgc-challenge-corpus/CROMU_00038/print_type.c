#include <stdio.h> // Required for printf

// Function: print_type
void print_type(unsigned short param_1) { // Replaced undefined2 with unsigned short
  switch(param_1) {
  default:
    printf("UNKNOWN");
    break;
  case 1:
    printf("BYTE");
    break;
  case 2:
    printf("ASCII");
    break;
  case 3:
    printf("SHORT");
    break;
  case 4:
    printf("LONG");
    break;
  case 5:
    printf("RATIONAL");
    break;
  case 7:
    printf("UNDEFINED");
    break;
  case 9:
    printf("SLONG");
    break;
  case 10:
    printf("SRATIONAL");
    break; // Added missing break for consistency
  }
}

// Main function to make it a complete, compilable program
int main() {
    printf("Type 1: ");
    print_type(1);
    printf("\n");

    printf("Type 2: ");
    print_type(2);
    printf("\n");

    printf("Type 3: ");
    print_type(3);
    printf("\n");

    printf("Type 4: ");
    print_type(4);
    printf("\n");

    printf("Type 5: ");
    print_type(5);
    printf("\n");

    printf("Type 7: ");
    print_type(7);
    printf("\n");

    printf("Type 9: ");
    print_type(9);
    printf("\n");

    printf("Type 10: ");
    print_type(10);
    printf("\n");

    printf("Type 6 (unknown): ");
    print_type(6);
    printf("\n");

    printf("Type 0 (unknown): ");
    print_type(0);
    printf("\n");

    return 0;
}