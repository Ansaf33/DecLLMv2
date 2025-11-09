#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Global error variable
int g_lasterror;

// Define types from decompiled code to standard C types
typedef unsigned int uint;

// Structure to hold particle data
typedef struct {
    double pos_x, pos_y;
    double vel_x, vel_y;
    double mass;
    double radius;
} Particle;

// Global simulation context, inferred from main's variable access patterns
// Offsets are based on the original decompiled code's access patterns.
struct SimContext {
    char _padding1[0x25bc];      // Padding to reach specific offset for min_coord
    double min_coord;            // iVar2 + 0x25bc
    double max_x_coord;          // iVar2 + 0x25c4
    double min_y_coord;          // iVar2 + 0x25cc
    double max_y_coord;          // iVar2 + 0x25d4
    char _padding2[0x4368 - (0x25d4 + sizeof(double))]; 
    int current_particle_count;  // iVar2 + 0x4368
    char _padding3[0x43a4 - (0x4368 + sizeof(int))];
    Particle particle_array[10]; // iVar2 + 0x43a4 (array of 10 particles)
} g_sim_context = {
    .min_coord = 0.0,
    .max_x_coord = 100.0,
    .min_y_coord = 0.0,
    .max_y_coord = 100.0,
    .current_particle_count = 0
};

// Dummy function declarations to satisfy compilation
void init_render_grid() { printf("Initializing render grid.\n"); }
void display_simulation_data() { printf("Displaying simulation data.\n"); }
void simulation_run(int steps) { printf("Running simulation for %d steps.\n", steps); }
uint get_simulation_frames() { return 1000; }
uint get_simulation_time() { return 100; }
uint get_collision_count() { return 50; }
int is_colliding(const Particle *p1, const Particle *p2);

// Function: simulation_add_particle
// Logic derived from the complex tail section of the original main function.
int simulation_add_particle(double pos_x, double pos_y, double vel_x, double vel_y, double mass, double radius) {
    if (g_sim_context.current_particle_count >= 10) {
        return -1;
    }

    if ((g_sim_context.max_x_coord <= pos_x) || (pos_x <= g_sim_context.min_coord)) {
        return -1;
    }
    if ((g_sim_context.max_x_coord <= pos_y) || (pos_y <= g_sim_context.min_coord)) {
        return -1;
    }
    if ((g_sim_context.max_y_coord < vel_x) || (vel_x < g_sim_context.min_y_coord)) {
        return -1;
    }
    if ((g_sim_context.max_y_coord < vel_y) || (vel_y < g_sim_context.min_y_coord)) {
        return -1;
    }
    if ((g_sim_context.max_y_coord < mass) || (mass < 1.0)) {
        return -1;
    }
    if ((g_sim_context.max_y_coord < radius) || (radius < 1.0)) {
        return -1;
    }
    
    if (((g_sim_context.max_x_coord < pos_x + radius) || (pos_x - radius < g_sim_context.min_coord)) ||
        ((g_sim_context.max_x_coord < pos_y + radius) || (pos_y - radius < g_sim_context.min_coord))) {
        return -1;
    }

    Particle new_p = {pos_x, pos_y, vel_x, vel_y, mass, radius};

    for (int i = 0; i < g_sim_context.current_particle_count; ++i) {
        if (is_colliding(&new_p, &g_sim_context.particle_array[i])) {
            return -1;
        }
    }

    int idx = g_sim_context.current_particle_count;
    g_sim_context.particle_array[idx] = new_p;
    g_sim_context.current_particle_count++;
    return idx;
}

// Dummy collision detection
int is_colliding(const Particle *p1, const Particle *p2) {
    double dx = p1->pos_x - p2->pos_x;
    double dy = p1->pos_y - p2->pos_y;
    double distance_sq = dx*dx + dy*dy;
    double min_distance_for_collision = p1->radius + p2->radius;
    
    if (distance_sq < (min_distance_for_collision * min_distance_for_collision)) {
        return 1;
    }
    return 0;
}


// Function: readLine
// Reads a line from a file descriptor into a buffer, stopping at newline or max_len.
// Returns number of characters read (excluding null terminator), or 0xFFFFFFFF on error/EOF.
uint readLine(int fd, char *buffer, uint max_len) {
  char c;
  uint i;
  
  if (max_len == 0) {
      return 0;
  }

  for (i = 0; i < max_len - 1; i++) {
    ssize_t bytes_received = read(fd, &c, 1);
    
    if (bytes_received <= 0) {
      if (bytes_received < 0) {
        g_lasterror = bytes_received;
      }
      buffer[i] = '\0';
      return 0xFFFFFFFF;
    }
    
    if (c == '\n') {
        break;
    }
    buffer[i] = c;
  }
  buffer[i] = '\0';
  return i;
}

// Function: parse_float_pair
// Parses a string containing two comma-separated floats (e.g., "10.5,20.0").
// Stores them into out_val1 and out_val2.
// Returns 0 on success, -1 on failure.
int parse_float_pair(const char *input_str, double *out_val1, double *out_val2) {
  char buffer1[1024];
  char buffer2[1024];
  int i;
  int j = 0;

  for (i = 0; i < sizeof(buffer1) - 1; i++) {
    if (input_str[i] == '\0') {
      return -1;
    }
    if (input_str[i] == ',') {
      break;
    }
    buffer1[i] = input_str[i];
  }

  if (i == sizeof(buffer1) - 1) {
    return -1;
  }
  buffer1[i] = '\0';

  i++;
  int input_len = strlen(input_str);
  while (i < input_len && j < sizeof(buffer2) - 1) {
    if (input_str[i] == '\0') {
        break;
    }
    buffer2[j] = input_str[i];
    j++;
    i++;
  }
  buffer2[j] = '\0';

  if (j == 0) {
      return -1;
  }

  *out_val1 = atof(buffer1);
  *out_val2 = atof(buffer2);

  return 0;
}

// Function: main
int main(void) {
  char input_buffer[1024];
  int particles_to_simulate;
  uint read_result;
  
  double pos_x, pos_y;
  double vel_x, vel_y;
  double mass;
  double radius;

  printf("2D Particle Simulator\nEnter the number of particles to simulate (1-10):\n");
  
  read_result = readLine(STDIN_FILENO, input_buffer, sizeof(input_buffer));

  if (read_result == 0xFFFFFFFF) {
    fprintf(stderr, "Error reading input for particle count.\n");
    exit(2);
  }

  particles_to_simulate = atoi(input_buffer);

  if (particles_to_simulate <= 0 || particles_to_simulate > 10) {
    printf("Goodbye\n");
    exit(0);
  }

  for (int i = 0; i < particles_to_simulate; i++) {
    printf("Enter Position (x,y):\n");
    read_result = readLine(STDIN_FILENO, input_buffer, sizeof(input_buffer));
    if (read_result == 0xFFFFFFFF) {
      fprintf(stderr, "Error reading position input.\n");
      exit(2);
    }
    if (parse_float_pair(input_buffer, &pos_x, &pos_y) != 0) {
      printf("Invalid position. Try again.\n");
      i--;
      continue;
    }

    printf("Enter Velocity (x,y):\n");
    read_result = readLine(STDIN_FILENO, input_buffer, sizeof(input_buffer));
    if (read_result == 0xFFFFFFFF) {
      fprintf(stderr, "Error reading velocity input.\n");
      exit(2);
    }
    if (parse_float_pair(input_buffer, &vel_x, &vel_y) != 0) {
      printf("Invalid velocity. Try again.\n");
      i--;
      continue;
    }

    printf("Enter Mass:\n");
    read_result = readLine(STDIN_FILENO, input_buffer, sizeof(input_buffer));
    if (read_result == 0xFFFFFFFF) {
      fprintf(stderr, "Error reading mass input.\n");
      exit(2);
    }
    mass = atof(input_buffer);
    if (mass == 0.0) {
      printf("Invalid mass. Try again.\n");
      i--;
      continue;
    }

    printf("Enter Radius:\n");
    read_result = readLine(STDIN_FILENO, input_buffer, sizeof(input_buffer));
    if (read_result == 0xFFFFFFFF) {
      fprintf(stderr, "Error reading radius input.\n");
      exit(2);
    }
    radius = atof(input_buffer);
    if (radius == 0.0) {
      printf("Invalid radius. Try again.\n");
      i--;
      continue;
    }

    int particle_idx = simulation_add_particle(pos_x, pos_y, vel_x, vel_y, mass, radius);
    if (particle_idx < 0) {
      printf("Invalid simulation data (e.g., collision, out of bounds). Try again.\n");
      i--;
      continue;
    }
    
    printf("Particle #%d added at (%.2f,%.2f) velocity(%.2f,%.2f) mass(%.2f) radius(%.2f).\n",
           particle_idx + 1, pos_x, pos_y, vel_x, vel_y, mass, radius);
  }

  printf("Running simulation with...\n");
  init_render_grid();
  display_simulation_data();
  simulation_run(10);

  uint frames = get_simulation_frames();
  uint time = get_simulation_time();
  uint collisions = get_collision_count();
  
  printf("Simulation complete, %u collisions simulated over %u seconds in %u frames.\n",
         collisions, time, frames);
  
  display_simulation_data();
  printf("Goodbye\n");
  
  exit(0);
}