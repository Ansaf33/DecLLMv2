#include <stdio.h>   // For printf
#include <math.h>    // For fabs, sqrt, round
#include <stdbool.h> // For bool

// Define custom types if they are not standard C types in the target environment
// Assuming uint is unsigned int
// Assuming undefined4 is unsigned int (or int depending on context)
// Assuming undefined is int
// Assuming ulonglong is unsigned long long
typedef unsigned int uint;

// --- Global Constants and Structures ---

// Constants (based on typical physics simulation ranges and common values)
// These were DAT_0001xxxx variables in the original snippet.
const double POS_MIN = 0.0;           // DAT_00014260 (e.g., left/bottom boundary)
const double POS_MAX = 1.0;           // DAT_00014258 (e.g., right/top boundary, also used for rounding factor)
const double VEL_MIN = -1.0;          // DAT_00014268 (e.g., min velocity)
const double VEL_MAX = 1.0;           // DAT_00014270 (e.g., max velocity)
const double COLLISION_DAMPING = -0.5; // DAT_00014278 (e.g., coefficient of restitution for inelastic collision)
const double ROUNDING_FACTOR = POS_MAX; // Used in `simulation_add_particle` for input precision

#define MAX_PARTICLES 10

// Particle structure, matching the 0x30 byte size (6 doubles)
typedef struct {
    double x, y;    // Position
    double vx, vy;  // Velocity
    double mass;    // Mass
    double radius;  // Radius
} Particle;

// --- Global Variables ---
Particle g_particles[MAX_PARTICLES];
uint g_particleCount = 0;
uint g_collisionTotal = 0;
uint g_simulationTime = 0;
uint g_simulationFrames = 0;

// --- External Function Declarations (placeholders for rendering functions) ---
// These functions are not defined in the snippet but are called.
// Assuming they take a Particle* for add_render_grid.
void clear_render_grid(void);
void add_render_grid(const Particle *p); // Changed to Particle*
void display_render_grid(void);

// --- Function Implementations ---

// Function: is_colliding
// Checks if two particles are colliding.
bool is_colliding(const Particle *p1, const Particle *p2) {
    double dx = p1->x - p2->x;
    double dy = p1->y - p2->y;
    double distance_sq = dx * dx + dy * dy;
    double radii_sum = p1->radius + p2->radius;
    return distance_sq < radii_sum * radii_sum;
}

// Function: do_collision
// Resolves collision between two particles.
void do_collision(Particle *p1, Particle *p2) {
    double dx = p1->x - p2->x;
    double dy = p1->y - p2->y;
    double dist = sqrt(dx * dx + dy * dy);

    // Avoid division by zero if particles are exactly at the same position
    if (dist == 0.0) {
        return;
    }

    // Separate particles to prevent overlap
    double overlap_factor = ((p1->radius + p2->radius) / dist) - 1.0;
    double move_x = dx * overlap_factor;
    double move_y = dy * overlap_factor;

    double total_mass = p1->mass + p2->mass;
    double p1_move_ratio = p2->mass / total_mass;
    double p2_move_ratio = p1->mass / total_mass;

    p1->x += move_x * p1_move_ratio;
    p1->y += move_y * p1_move_ratio;
    p2->x -= move_x * p2_move_ratio;
    p2->y -= move_y * p2_move_ratio;

    // Resolve velocities
    double normal_x = dx / dist;
    double normal_y = dy / dist;

    double relative_vx = p1->vx - p2->vx;
    double relative_vy = p1->vy - p2->vy;

    double dot_product = relative_vx * normal_x + relative_vy * normal_y;

    // Only apply impulse if particles are moving towards each other
    if (dot_product < 0.0) {
        double impulse_magnitude = COLLISION_DAMPING * dot_product;

        p1->vx += impulse_magnitude * p1_move_ratio * normal_x;
        p1->vy += impulse_magnitude * p1_move_ratio * normal_y;
        p2->vx -= impulse_magnitude * p2_move_ratio * normal_x;
        p2->vy -= impulse_magnitude * p2_move_ratio * normal_y;
    }
}

// Function: do_wall_collision
// Handles collisions of a particle with the simulation boundaries (walls).
void do_wall_collision(Particle *p) {
    // X-axis collisions
    if (p->x + p->radius > POS_MAX) { // Right wall
        double overlap = (p->x + p->radius) - POS_MAX;
        p->x -= overlap * 2.0;
        if (p->vx > 0.0) {
            p->vx = -p->vx;
        }
    } else if (p->x - p->radius < POS_MIN) { // Left wall
        double overlap = POS_MIN - (p->x - p->radius);
        p->x += overlap * 2.0;
        if (p->vx < 0.0) {
            p->vx = -p->vx;
        }
    }

    // Y-axis collisions
    if (p->y + p->radius > POS_MAX) { // Top wall
        double overlap = (p->y + p->radius) - POS_MAX;
        p->y -= overlap * 2.0;
        if (p->vy > 0.0) {
            p->vy = -p->vy;
        }
    } else if (p->y - p->radius < POS_MIN) { // Bottom wall
        double overlap = POS_MIN - (p->y - p->radius);
        p->y += overlap * 2.0;
        if (p->vy < 0.0) {
            p->vy = -p->vy;
        }
    }
}

// Function: simulation_add_particle
// Adds a new particle to the simulation if conditions are met.
// Parameters are assumed to be direct double values, replacing the CONCAT44 logic.
uint simulation_add_particle(double x, double y, double vx, double vy, double mass, double radius) {
    // Rounding parameters to a fixed precision, as implied by original code
    double rx = round(ROUNDING_FACTOR * x) / ROUNDING_FACTOR;
    double ry = round(ROUNDING_FACTOR * y) / ROUNDING_FACTOR;
    double rvx = round(ROUNDING_FACTOR * vx) / ROUNDING_FACTOR;
    double rvy = round(ROUNDING_FACTOR * vy) / ROUNDING_FACTOR;
    double rmass = round(ROUNDING_FACTOR * mass) / ROUNDING_FACTOR;
    double rradius = round(ROUNDING_FACTOR * radius) / ROUNDING_FACTOR;

    if (g_particleCount >= MAX_PARTICLES) {
        return 0xffffffff; // Max particles reached
    }

    // Validate particle properties based on common physics constraints and re-interpreted original logic
    if (rx < POS_MIN || rx > POS_MAX ||
        ry < POS_MIN || ry > POS_MAX ||
        rvx < VEL_MIN || rvx > VEL_MAX ||
        rvy < VEL_MIN || rvy > VEL_MAX ||
        rmass <= 0.0 || rmass > POS_MAX || // Mass should be positive, and within reasonable bounds (e.g., 0 to 1)
        rradius <= 0.0 || rradius > POS_MAX) { // Radius should be positive, and within reasonable bounds (e.g., 0 to 1)
        return 0xffffffff; // Invalid property
    }

    // Check if the particle (with its radius) fits within the simulation boundaries
    if ((rx + rradius > POS_MAX) || (rx - rradius < POS_MIN) ||
        (ry + rradius > POS_MAX) || (ry - rradius < POS_MIN)) {
        return 0xffffffff; // Particle extends beyond boundaries
    }

    // Assign particle properties to the new particle slot
    Particle *new_particle = &g_particles[g_particleCount];
    new_particle->x = rx;
    new_particle->y = ry;
    new_particle->vx = rvx;
    new_particle->vy = rvy;
    new_particle->mass = rmass;
    new_particle->radius = rradius;

    // Check for collision with existing particles
    for (uint i = 0; i < g_particleCount; ++i) {
        if (is_colliding(&g_particles[i], new_particle)) {
            return 0xffffffff; // New particle collides with an existing one
        }
    }

    g_particleCount++;
    return g_particleCount; // Return the new total particle count
}

// Function: simulation_reset
// Resets the simulation by clearing the particle count.
void simulation_reset(void) {
    g_particleCount = 0;
}

// Function: simulation_run
// Runs the simulation for a specified number of frames.
void simulation_run(uint num_frames) {
    g_collisionTotal = 0;
    g_simulationTime = num_frames;
    g_simulationFrames = 0;

    uint steps_per_frame = 1; // Controls the granularity of physics updates per frame

    for (uint current_frame = 0; current_frame < num_frames; ++current_frame) {
        uint max_substeps_needed = 1;

        // Determine the maximum number of substeps required based on particle velocities and radii
        for (uint i = 0; i < g_particleCount; ++i) {
            // If a particle's velocity is greater than its radius, it might "tunnel" through other particles
            // or walls in a single step. More substeps are needed.
            if (g_particles[i].radius < fabs(g_particles[i].vx)) {
                uint substeps = (uint)round(fabs(g_particles[i].vx) / g_particles[i].radius) + 1;
                if (max_substeps_needed < substeps) {
                    max_substeps_needed = substeps;
                }
            }
            if (g_particles[i].radius < fabs(g_particles[i].vy)) {
                uint substeps = (uint)round(fabs(g_particles[i].vy) / g_particles[i].radius) + 1;
                if (max_substeps_needed < substeps) {
                    max_substeps_needed = substeps;
                }
            }
        }

        // Adjust steps_per_frame to be a power of 2, greater than or equal to max_substeps_needed
        while (steps_per_frame < max_substeps_needed) {
            steps_per_frame <<= 1; // Double steps_per_frame
            // The original code also doubled `num_frames` here (`local_1c = local_1c << 1;`),
            // which would incorrectly extend the simulation duration. This has been removed.
        }

        double dt = 1.0 / (double)steps_per_frame; // Time step duration for each substep

        for (uint step = 0; step < steps_per_frame; ++step) {
            // Update particle positions
            for (uint i = 0; i < g_particleCount; ++i) {
                g_particles[i].x += g_particles[i].vx * dt;
                g_particles[i].y += g_particles[i].vy * dt;
            }

            uint collisions_this_step = 0;

            // Handle particle-particle collisions
            for (uint i = 0; i < g_particleCount; ++i) {
                for (uint j = i + 1; j < g_particleCount; ++j) { // Check each unique pair once
                    if (is_colliding(&g_particles[i], &g_particles[j])) {
                        do_collision(&g_particles[i], &g_particles[j]);
                        collisions_this_step++;
                    }
                }
                // Handle particle-wall collisions after particle-particle collisions
                do_wall_collision(&g_particles[i]);
            }
            g_collisionTotal += collisions_this_step;
        }
    }
    g_simulationFrames = num_frames; // The number of frames actually processed
}

// Function: get_collision_count
// Returns the total number of collisions recorded.
uint get_collision_count(void) {
    return g_collisionTotal;
}

// Function: get_simulation_time
// Returns the total simulation time (intended frames).
uint get_simulation_time(void) {
    return g_simulationTime;
}

// Function: get_simulation_frames
// Returns the total number of simulation frames processed.
uint get_simulation_frames(void) {
    return g_simulationFrames;
}

// Function: display_simulation_data
// Clears the rendering grid, prints particle data, and displays the grid.
void display_simulation_data(void) {
    clear_render_grid();
    printf("%u total particles:\n", g_particleCount);

    for (uint i = 0; i < g_particleCount; ++i) {
        const Particle *p = &g_particles[i]; // Use const Particle* for reading
        // Corrected printf format specifiers and arguments for doubles
        printf("%u: Position (%f,%f) Velocity (%f,%f) mass (%f) radius (%f).\n",
               i, p->x, p->y, p->vx, p->vy, p->mass, p->radius);
        add_render_grid(p);
    }
    display_render_grid();
}