#include <stdio.h> // Required for printf in main

// Global variables
int bowl_set;
int units;
int ready;

// Function: init_dispenser
void init_dispenser(void) {
  bowl_set = 0;
  units = 0;
  ready = 1;
}

// Function: dispense_bowl
void dispense_bowl(void) {
  bowl_set = 1;
}

// Function: completion_buzzer
void completion_buzzer(void) {
  ready = 0;
}

// Function: dispense_dairy_milk
void dispense_dairy_milk(void) {
  units++;
}

// Function: dispense_dairy_cream
void dispense_dairy_cream(void) {
  units++;
}

// Function: dispense_water
void dispense_water(void) {
  units++;
}

// Function: dispense_soy_milk
void dispense_soy_milk(void) {
  units++;
}

// Function: dispense_almond_milk
void dispense_almond_milk(void) {
  units++;
}

// Function: dispense_sugar_loops
void dispense_sugar_loops(void) {
  units++;
}

// Function: dispense_maize_flakes
void dispense_maize_flakes(void) {
  units++;
}

// Function: dispense_marshmallow_figuringes
void dispense_marshmallow_figuringes(void) {
  units++;
}

// Function: dispense_chocolate_rice_pellets
void dispense_chocolate_rice_pellets(void) {
  units++;
}

// Function: dispense_oohs_of_oats
void dispense_oohs_of_oats(void) {
  units++;
}

// Function: dispense_crunchy_puffs
void dispense_crunchy_puffs(void) {
  units++;
}

// Function: dispense_frutiz_n_nuts
void dispense_frutiz_n_nuts(void) {
  units++;
}

// Function: dispense_chocolate_drops
void dispense_chocolate_drops(void) {
  units++;
}

// Function: dispense_blueberries
void dispense_blueberries(void) {
  units++;
}

// Function: dispense_berry_medley
void dispense_berry_medley(void) {
  units++;
}

// Function: dispense_sugar_cube
void dispense_sugar_cube(void) {
  units++;
}

// Function: dispense_strawberries
void dispense_strawberries(void) {
  units++;
}

// Main function to demonstrate usage
int main(void) {
  printf("Initializing dispenser...\n");
  init_dispenser();
  printf("Initial state: bowl_set=%d, units=%d, ready=%d\n", bowl_set, units, ready);

  printf("Dispensing bowl...\n");
  dispense_bowl();
  printf("After dispense_bowl: bowl_set=%d, units=%d, ready=%d\n", bowl_set, units, ready);

  printf("Dispensing some items...\n");
  dispense_dairy_milk();
  dispense_chocolate_drops();
  dispense_sugar_loops();
  dispense_maize_flakes();
  printf("After dispensing items: bowl_set=%d, units=%d, ready=%d\n", bowl_set, units, ready);

  printf("Completing dispensing with buzzer...\n");
  completion_buzzer();
  printf("Final state: bowl_set=%d, units=%d, ready=%d\n", bowl_set, units, ready);

  return 0;
}