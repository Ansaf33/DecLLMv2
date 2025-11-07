#include <stdio.h>  // For printf, scanf
#include <stdlib.h> // For rand, srand
#include <time.h>   // For time

// Helper function to initialize random seed
static void init_rand_once(void) {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
}

// Generates a random integer in the range [min, max]
static int random_in_range(int min, int max) {
    init_rand_once();
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    // Ensure positive range for modulo operation
    if (max - min + 1 <= 0) return min; // Handle empty or single-value range
    return rand() % (max - min + 1) + min;
}

// Prompts user for an answer and stores it. Returns 1 on success, 0 on failure.
static int get_user_answer(int *answer) {
    printf("Your answer: ");
    if (scanf("%d", answer) == 1) {
        // Clear remaining input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return 1;
    }
    // Clear remaining input buffer in case of non-integer input
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    return 0; // Failed to read an integer
}

// Function: request_perimeter
int request_perimeter(void) {
  int shape_type = random_in_range(0, 2);
  int userAnswer = 0;
  int correct_answer = 0;

  if (shape_type == 2) { // Polygon
    int sides = random_in_range(1, 0x4d2); // Min 1 to avoid 0 sides
    int length = random_in_range(1, 0x8707); // Min 1 to avoid 0 length
    correct_answer = sides * length;
    printf("Find perimeter of object with %d sides of length %d\n", sides, length);
  } else if (shape_type == 0) { // Circle
    int radius = random_in_range(1, 0x8707); // Min 1
    correct_answer = radius * 6; // Approximation for 2 * pi * r (pi approx 3)
    printf("Find perimeter of circle with radius %d\n", radius);
  } else if (shape_type == 1) { // Rectangle
    int base = random_in_range(1, 0x8707); // Min 1
    int side = random_in_range(1, 0x8707); // Min 1
    correct_answer = (base + side) * 2;
    printf("Find perimeter of rectangle with a base of %d and sides of %d\n", base, side);
  } else {
      return 1; // Should not be reached with random_in_range(0,2)
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0; // Input error
  }

  if (correct_answer == userAnswer) {
    printf("You got it\n");
    return 1;
  } else {
    printf("Incorrect\n");
    return 0;
  }
}

// Function: request_2d_peri_dim
int request_2d_peri_dim(void) {
  int shape_type = random_in_range(0, 2); // 0: circle, 1: rectangle, 2: polygon
  int correct_answer = 0;
  int userAnswer = 0;

  if (shape_type == 2) { // Polygon (sides, length, perimeter)
    int num_sides = random_in_range(1, 0x4d2);
    int side_length = random_in_range(1, 0x8707);
    int calculated_perimeter = num_sides * side_length;
    int question_type = random_in_range(0, 2);

    if (question_type == 0) { // Find perimeter
      printf("Find perimeter of object with %d sides of length %d\n", num_sides, side_length);
      correct_answer = calculated_perimeter;
    } else if (question_type == 1) { // Find number of sides
      printf("Find the number of sides of object with %d perimeter and the sides are of length %d\n",
             calculated_perimeter, side_length);
      correct_answer = num_sides;
    } else { // question_type == 2, Find side length
      printf("Find length of the sides of object with %d perimeter with %d sides\n",
             calculated_perimeter, num_sides);
      correct_answer = side_length;
    }
  } else if (shape_type == 0) { // Circle (radius, perimeter)
    int radius = random_in_range(1, 0x8707);
    int calculated_perimeter = radius * 6; // Approximation
    int question_type = random_in_range(0, 1);

    if (question_type == 0) { // Find perimeter
      printf("Find perimeter of circle with radius %d\n", radius);
      correct_answer = calculated_perimeter;
    } else { // question_type == 1, Find radius
      printf("Find radius of circle with perimeter %d\n", calculated_perimeter);
      correct_answer = radius;
    }
  } else if (shape_type == 1) { // Rectangle (base, side, perimeter)
    int base = random_in_range(1, 0x8707);
    int side = random_in_range(1, 0x8707);
    int calculated_perimeter = (base + side) * 2;
    int question_type = random_in_range(0, 2);

    if (question_type == 0) { // Find perimeter
      printf("Find perimeter of rectangle with a base of %d and sides of %d\n", base, side);
      correct_answer = calculated_perimeter;
    } else if (question_type == 1) { // Find side
      printf("Find side of rectangle with a base of %d and perimeter of %d\n", base, calculated_perimeter);
      correct_answer = side;
    } else { // question_type == 2, Find base
      printf("Find base of rectangle with a side of %d and perimeter of %d\n", side, calculated_perimeter);
      correct_answer = base;
    }
  } else {
      return 1; // Should not be reached
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0; // Input error
  }

  if (correct_answer == userAnswer) {
    printf("You got it\n");
    return 1;
  } else {
    printf("Incorrect\n");
    return 0;
  }
}

// Function: request_area
int request_area(void) {
  int shape_type = random_in_range(0, 2); // 0: circle, 1: rectangle, 2: square
  int userAnswer = 0;
  int correct_answer = 0;

  if (shape_type == 2) { // Square
    int side = random_in_range(1, 0x8707); // Min 1
    correct_answer = side * side;
    printf("Find area of square with sides of %d\n", side);
  } else if (shape_type == 0) { // Circle
    int radius = random_in_range(1, 15000); // Min 1
    correct_answer = radius * radius * 3; // Approximation for pi * r^2 (pi approx 3)
    printf("Find area of circle with radius %d\n", radius);
  } else if (shape_type == 1) { // Rectangle
    int base = random_in_range(1, 0x8707); // Min 1
    int side = random_in_range(1, 0x8707); // Min 1
    correct_answer = base * side;
    printf("Find area of rectangle with a base of %d and sides of %d\n", base, side);
  } else {
      return 1; // Should not be reached
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0; // Input error
  }

  if (correct_answer == userAnswer) {
    printf("You got it\n");
    return 1;
  } else {
    printf("Incorrect\n");
    return 0;
  }
}

// Function: request_2d_area_dim
int request_2d_area_dim(void) {
  int shape_type = random_in_range(0, 1); // 0: circle, 1: rectangle
  int correct_answer = 0;
  int userAnswer = 0;
  int result = 0; // Return value for the function

  if (shape_type == 0) { // Circle (radius, area)
    int radius = random_in_range(1, 15000);
    int calculated_area = radius * radius * 3; // Approximation
    int question_type = random_in_range(0, 1);

    if (question_type == 0) { // Find area
      printf("Find area of circle with radius %d\n", radius);
      correct_answer = calculated_area;
    } else { // question_type == 1, Find radius
      printf("Find radius of circle with area %d\n", calculated_area);
      correct_answer = radius;
    }
  } else if (shape_type == 1) { // Rectangle (base, side, area)
    int side = random_in_range(1, 0x8707);
    int base = random_in_range(1, 0x8707);
    int calculated_area = side * base;
    int question_type = random_in_range(0, 2);

    if (question_type == 0) { // Find area
      printf("Find area of rectangle with a base of %d and sides of %d\n", base, side);
      correct_answer = calculated_area;
    } else if (question_type == 1) { // Find side
      printf("Find side of rectangle with a base of %d and area of %d\n", base, calculated_area);
      correct_answer = side;
    } else { // question_type == 2, Find base
      printf("Find base of rectangle with a side of %d and area of %d\n", side, calculated_area);
      correct_answer = base;
    }
  } else {
      return 1; // Should not be reached
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0; // Input error
  }

  if (correct_answer == userAnswer) {
    printf("You got it\n");
    result = 1;
  } else {
    printf("Incorrect\n");
    result = 0;
  }
  return result;
}

// Function: request_volume
int request_volume(void) {
  int shape_type = random_in_range(0, 2); // 0: sphere, 1: rectangular prism, 2: cube
  int userAnswer = 0;
  int correct_answer = 0;

  if (shape_type == 2) { // Cube
    int side = random_in_range(1, 0x508); // Min 1
    correct_answer = side * side * side;
    printf("Find volume of cube with sides of %d\n", side);
  } else if (shape_type == 0) { // Sphere
    int radius = random_in_range(1, 0x1ae); // Min 1
    correct_answer = (radius * radius * radius / 3) * 12; // Approximation for (4/3) * pi * r^3 (pi approx 3) = 4 * r^3
    printf("Find volume of sphere with radius %d\n", radius);
  } else if (shape_type == 1) { // Rectangular Prism
    int base = random_in_range(1, 0x508);
    int side = random_in_range(1, 0x508);
    int height = random_in_range(1, 0x508);
    correct_answer = base * side * height;
    printf("Find volume of rectangular prism with a base of %d, side of %d, and height of %d\n",
           base, side, height);
  } else {
      return 1; // Should not be reached
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0; // Input error
  }

  if (correct_answer == userAnswer) {
    printf("You got it\n");
    return 1;
  } else {
    printf("Incorrect\n");
    return 0;
  }
}

// Function: request_3d_dim_vol
int request_3d_dim_vol(void) {
  int shape_type = random_in_range(0, 2); // 0: sphere, 1: rectangular prism, 2: cube
  int correct_answer = 0;
  int userAnswer = 0;

  if (shape_type == 2) { // Cube (side, volume)
    int side = random_in_range(1, 0x508);
    int calculated_volume = side * side * side;
    int question_type = random_in_range(0, 1);

    if (question_type == 0) { // Find volume
      printf("Find volume of cube with a side of %d\n", side);
      correct_answer = calculated_volume;
    } else { // question_type == 1, Find side
      printf("Find side of cube with a volume of %d\n", calculated_volume);
      correct_answer = side;
    }
  } else if (shape_type == 0) { // Sphere (radius, volume)
    int radius = random_in_range(1, 0x1ae);
    int calculated_volume = (radius * radius * radius / 3) * 12; // Approximation
    int question_type = random_in_range(0, 1);

    if (question_type == 0) { // Find volume
      printf("Find volume of sphere with radius %d\n", radius);
      correct_answer = calculated_volume;
    } else { // question_type == 1, Find radius
      printf("Find radius of sphere with volume %d\n", calculated_volume);
      correct_answer = radius;
    }
  } else if (shape_type == 1) { // Rectangular Prism (base, side, height, volume)
    int base = random_in_range(1, 0x508);
    int side = random_in_range(1, 0x508);
    int height = random_in_range(1, 0x508);
    int calculated_volume = base * side * height;
    int question_type = random_in_range(0, 3);

    if (question_type == 0) { // Find volume
      printf("Find volume of rectangular prism with a base of %d, side of %d, and height of %d\n",
             base, side, height);
      correct_answer = calculated_volume;
    } else if (question_type == 1) { // Find side
      printf("Find side of rectangular prism with a base of %d, volume of %d, and height of %d\n",
             base, calculated_volume, height);
      correct_answer = side;
    } else if (question_type == 2) { // Find base
      printf("Find base of rectangular prism with a volume of %d, side of %d, and height of %d\n",
             calculated_volume, side, height);
      correct_answer = base;
    } else { // question_type == 3, Find height
      printf("Find height of rectangular prism with a volume of %d, side of %d, and base of %d\n",
             calculated_volume, side, base);
      correct_answer = height;
    }
  } else {
      return 1; // Should not be reached
  }

  if (get_user_answer(&userAnswer) != 1) {
    return 0; // Input error
  }

  if (correct_answer == userAnswer) {
    printf("You got it\n");
    return 1;
  } else {
    printf("Incorrect\n");
    return 0;
  }
}