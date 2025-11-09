#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Helper function to generate a random integer within a specified range (inclusive).
static int random_in_range(int min, int max) {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    if (min == max) {
        return min;
    }
    return rand() % (max - min + 1) + min;
}

// Helper function to get an integer answer from the user.
// Returns 1 on successful input, 0 otherwise.
static int get_user_answer(int *answer) {
    printf("Your answer: ");
    if (scanf("%d", answer) == 1) {
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return 1; // Success
    }
    // Clear input buffer in case of non-integer input
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    return 0; // Failure
}

// Function: request_perimeter
int request_perimeter(void) {
  int choice = random_in_range(0, 2);
  int userAnswer;
  int correctAnswer = 0;
  int val1 = 0, val2 = 0; // For printing question parameters

  if (choice == 2) { // Polygon
    val1 = random_in_range(0, 0x4d2); // sides
    val2 = random_in_range(0, 0x8707); // length
    correctAnswer = val1 * val2;
    printf("find perimeter of object with %d sides of length %d\n", val1, val2);
  } else if (choice == 0) { // Circle
    val1 = random_in_range(0, 0x8707); // radius
    correctAnswer = val1 * 6; // Approximation for 2*pi*r, with pi=3
    printf("find perimeter of circle with radius %d\n", val1);
  } else if (choice == 1) { // Rectangle
    val1 = random_in_range(0, 0x8707); // base
    val2 = random_in_range(0, 0x8707); // side
    correctAnswer = (val1 + val2) * 2;
    printf("find perimeter of rectangle with a base of %d and sides of %d\n", val1, val2);
  } else {
    // This case should not be reached with random_in_range(0,2)
    return 1;
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0; // User input failed
  }

  if (correctAnswer == userAnswer) {
    printf("you got it\n");
    return 1;
  } else {
    printf("incorrect\n");
    return 0;
  }
}

// Function: request_2d_peri_dim
int request_2d_peri_dim(void) {
  int choice = random_in_range(0, 2);
  int userAnswer;
  int correctAnswer = 0;
  int val1 = 0, val2 = 0, val3 = 0; // For printing question parameters

  if (choice == 2) { // Polygon
    val1 = random_in_range(0, 0x4d2); // sides
    val2 = random_in_range(0, 0x8707); // length
    val3 = val1 * val2; // perimeter
    int sub_choice = random_in_range(0, 2);

    if (sub_choice == 0) { // Find perimeter
      printf("find perimeter of object with %d sides of length %d\n", val1, val2);
      correctAnswer = val3;
    } else if (sub_choice == 1) { // Find number of sides
      printf("find the number of sides of object with %d perimeter and the sides are of length %d\n", val3, val2);
      correctAnswer = val1;
    } else { // sub_choice == 2: Find length of sides
      printf("find length of the sides of object with %d perimeter with %d sides\n", val3, val1);
      correctAnswer = val2;
    }
  } else if (choice == 0) { // Circle
    val1 = random_in_range(0, 0x8707); // radius
    val2 = val1 * 6; // perimeter (approximation)
    int sub_choice = random_in_range(0, 1);

    if (sub_choice == 0) { // Find perimeter
      printf("find perimeter of circle with radius %d\n", val1);
      correctAnswer = val2;
    } else { // sub_choice == 1: Find radius
      printf("find radius of circle with perimeter %d\n", val2);
      correctAnswer = val1;
    }
  } else if (choice == 1) { // Rectangle
    val1 = random_in_range(0, 0x8707); // base
    val2 = random_in_range(0, 0x8707); // side
    val3 = (val1 + val2) * 2; // perimeter
    int sub_choice = random_in_range(0, 2);

    if (sub_choice == 0) { // Find perimeter
      printf("find perimeter of rectangle with a base of %d and sides of %d\n", val1, val2);
      correctAnswer = val3;
    } else if (sub_choice == 1) { // Find side
      printf("find side of rectangle with a base of %d and perimeter of %d\n", val1, val3);
      correctAnswer = val2;
    } else { // sub_choice == 2: Find base
      printf("find base of rectangle with a side of %d and perimeter of %d\n", val2, val3);
      correctAnswer = val1;
    }
  } else {
      return 1; // Unhandled choice
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0;
  }

  if (correctAnswer == userAnswer) {
    printf("you got it\n");
    return 1;
  } else {
    printf("incorrect\n");
    return 0;
  }
}

// Function: request_area
int request_area(void) {
  int choice = random_in_range(0, 2);
  int userAnswer;
  int correctAnswer = 0;
  int val1 = 0, val2 = 0; // For printing question parameters

  if (choice == 2) { // Square
    val1 = random_in_range(0, 0x8707); // side
    correctAnswer = val1 * val1;
    printf("find area of square with sides of %d\n", val1);
  } else if (choice == 0) { // Circle
    val1 = random_in_range(0, 15000); // radius
    correctAnswer = val1 * val1 * 3; // Approximation for pi*r^2, with pi=3
    printf("find area of circle with radius %d\n", val1);
  } else if (choice == 1) { // Rectangle
    val1 = random_in_range(0, 0x8707); // base
    val2 = random_in_range(0, 0x8707); // side
    correctAnswer = val1 * val2;
    printf("find area of rectangle with a base of %d and sides of %d\n", val1, val2);
  } else {
      return 1; // Unhandled choice
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0;
  }

  if (correctAnswer == userAnswer) {
    printf("you got it\n");
    return 1;
  } else {
    printf("incorrect\n");
    return 0;
  }
}

// Function: request_2d_area_dim
int request_2d_area_dim(void) {
  int choice = random_in_range(0, 1);
  int userAnswer;
  int correctAnswer = 0;
  int val1 = 0, val2 = 0, val3 = 0; // For printing question parameters

  if (choice == 0) { // Circle
    val1 = random_in_range(0, 15000); // radius
    val2 = val1 * val1 * 3; // area (approximation)
    int sub_choice = random_in_range(0, 1);

    if (sub_choice == 0) { // Find area
      printf("find area of circle with radius %d\n", val1);
      correctAnswer = val2;
    } else { // sub_choice == 1: Find radius
      printf("find radius of circle with area %d\n", val2);
      correctAnswer = val1;
    }
  } else if (choice == 1) { // Rectangle
    val1 = random_in_range(0, 0x8707); // base
    val2 = random_in_range(0, 0x8707); // side
    val3 = val1 * val2; // area
    int sub_choice = random_in_range(0, 2);

    if (sub_choice == 0) { // Find area
      printf("find area of rectangle with a base of %d and sides of %d\n", val1, val2);
      correctAnswer = val3;
    } else if (sub_choice == 1) { // Find side
      printf("find side of rectangle with a base of %d and area of %d\n", val1, val3);
      correctAnswer = val2;
    } else { // sub_choice == 2: Find base
      printf("find base of rectangle with a side of %d and area of %d\n", val2, val3);
      correctAnswer = val1;
    }
  } else {
      return 1; // Unhandled choice
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0;
  }

  if (correctAnswer == userAnswer) {
    printf("you got it\n");
    return 1;
  } else {
    printf("incorrect\n");
    return 0;
  }
}

// Function: request_volume
int request_volume(void) {
  int choice = random_in_range(0, 2);
  int userAnswer;
  int correctAnswer = 0;
  int val1 = 0, val2 = 0, val3 = 0; // For printing question parameters

  if (choice == 2) { // Cube
    val1 = random_in_range(0, 0x508); // side
    correctAnswer = val1 * val1 * val1;
    printf("find volume of cube with sides of %d\n", val1);
  } else if (choice == 0) { // Sphere
    val1 = random_in_range(0, 0x1ae); // radius
    correctAnswer = ((val1 * val1 * val1) / 3) * 0xc; // Approximation for (4/3)*pi*r^3, with pi=3
    printf("find volume of sphere with radius %d\n", val1);
  } else if (choice == 1) { // Rectangular Prism
    val1 = random_in_range(0, 0x508); // base
    val2 = random_in_range(0, 0x508); // side
    val3 = random_in_range(0, 0x508); // height
    correctAnswer = val1 * val2 * val3;
    printf("find volume of rectangular prism with a base of %d, side of %d, and height of %d\n", val1, val2, val3);
  } else {
      return 1; // Unhandled choice
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0;
  }

  if (correctAnswer == userAnswer) {
    printf("you got it\n");
    return 1;
  } else {
    printf("incorrect\n");
    return 0;
  }
}

// Function: request_3d_dim_vol
int request_3d_dim_vol(void) {
  int choice = random_in_range(0, 2);
  int userAnswer;
  int correctAnswer = 0;
  int val1 = 0, val2 = 0, val3 = 0, val4 = 0; // For printing question parameters

  if (choice == 2) { // Cube
    val1 = random_in_range(0, 0x508); // side
    val2 = val1 * val1 * val1; // volume
    int sub_choice = random_in_range(0, 1);

    if (sub_choice == 0) { // Find volume
      printf("find volume of cube with a side of %d\n", val1);
      correctAnswer = val2;
    } else { // sub_choice == 1: Find side
      printf("find side of cube with a volume of %d\n", val2);
      correctAnswer = val1;
    }
  } else if (choice == 0) { // Sphere
    val1 = random_in_range(0, 0x1ae); // radius
    val2 = ((val1 * val1 * val1) / 3) * 0xc; // volume (approximation)
    int sub_choice = random_in_range(0, 1);

    if (sub_choice == 0) { // Find volume
      printf("find volume of sphere with radius %d\n", val1);
      correctAnswer = val2;
    } else { // sub_choice == 1: Find radius
      printf("find radius of sphere with volume %d\n", val2);
      correctAnswer = val1;
    }
  } else if (choice == 1) { // Rectangular Prism
    val1 = random_in_range(0, 0x508); // base
    val2 = random_in_range(0, 0x508); // side
    val3 = random_in_range(0, 0x508); // height
    val4 = val1 * val2 * val3; // volume
    int sub_choice = random_in_range(0, 3);

    if (sub_choice == 0) { // Find volume
      printf("find volume of rectangular prism with a base of %d, side of %d, and height of %d\n", val1, val2, val3);
      correctAnswer = val4;
    } else if (sub_choice == 1) { // Find side
      printf("find side of rectangular prism with a base of %d, volume of %d, and height of %d\n", val1, val4, val3);
      correctAnswer = val2;
    } else if (sub_choice == 2) { // Find base
      printf("find base of rectangular prism with a volume of %d, side of %d, and height of %d\n", val4, val2, val3);
      correctAnswer = val1;
    } else { // sub_choice == 3: Find height
      printf("find height of rectangular prism with a volume of %d, side of %d, and base of %d\n", val4, val2, val1);
      correctAnswer = val3;
    }
  } else {
      return 1; // Unhandled choice
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0;
  }

  if (correctAnswer == userAnswer) {
    printf("you got it\n");
    return 1;
  } else {
    printf("incorrect\n");
    return 0;
  }
}

// Main function to demonstrate compilation and execution
int main() {
    srand(time(NULL)); // Seed the random number generator

    printf("--- Request Perimeter ---\n");
    request_perimeter();
    printf("\n--- Request 2D Perimeter Dimension ---\n");
    request_2d_peri_dim();
    printf("\n--- Request Area ---\n");
    request_area();
    printf("\n--- Request 2D Area Dimension ---\n");
    request_2d_area_dim();
    printf("\n--- Request Volume ---\n");
    request_volume();
    printf("\n--- Request 3D Dimension Volume ---\n");
    request_3d_dim_vol();

    return 0;
}