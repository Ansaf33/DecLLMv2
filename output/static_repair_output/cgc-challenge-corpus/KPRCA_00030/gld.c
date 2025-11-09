#include <stdio.h>   // For fprintf, stdout
#include <stdlib.h>  // For malloc, calloc, free
#include <stdbool.h> // For bool type
#include <string.h>  // For memcpy

// Forward declarations for structs
typedef struct Cell Cell;
typedef struct QuadtreeNode QuadtreeNode;

// Cell struct definition
struct Cell {
    int x;
    int y;
    int alive;      // 0 or 1
    int bomb_timer; // countdown for bomb, 0 if not a bomb or timer expired
    Cell *prev;     // For linked list in QuadtreeNode
    Cell *next;     // For linked list in QuadtreeNode
};

// QuadtreeNode struct definition
struct QuadtreeNode {
    int level;
    int capacity;         // max cells before subdividing
    int count;            // current number of cells
    Cell *cell_list_head; // head of linked list of cells in this node
    int x_min, y_min, x_max, y_max;
    int is_subdivided;    // 0 or 1

    // Child nodes
    QuadtreeNode *child_nw;
    QuadtreeNode *child_ne;
    QuadtreeNode *child_sw;
    QuadtreeNode *child_se;
};

// VisitedCell struct definition (used in conway_step)
typedef struct VisitedCell {
    int x;
    int y;
    struct VisitedCell *prev;
    struct VisitedCell *next;
} VisitedCell;

// Global quadtree instance
QuadtreeNode *g_conway_gld = NULL;

// Function prototypes (to resolve dependencies)
int within_region(int x, int y, int x_min, int y_min, int x_max, int y_max);
Cell *create_cell_list(QuadtreeNode *node, int filter_type);
int *create_adjacent_coords(int x, int y);
int bomb_collisions(Cell *bomb_list_head);
bool is_dead_cell_generation(int x, int y);
void conway_step(int x, int y, int alive, int bomb_timer,
                 Cell **new_alive_cells_list_head, Cell **new_dead_cells_list_head,
                 VisitedCell **visited_cells_list_head);
QuadtreeNode *find_subregion_tree(QuadtreeNode *node, int x, int y);
int subdivide(QuadtreeNode *node);
void move_pixels(QuadtreeNode *node, Cell *cells_to_move_head, int count_to_move);
int qt_undivide(QuadtreeNode *node);
int qt_delete(QuadtreeNode *node, int x, int y, int alive, int bomb_timer);
QuadtreeNode *qt_create(int level, int x_min, int y_min, int x_max, int y_max);
int qt_insert(QuadtreeNode *node, int x, int y, int alive, int bomb_timer);
Cell *qt_get_pixel(QuadtreeNode *node, int x, int y);
int cg_step(int max_steps);
int shoot_pixel(int x, int y);
bool set_bomb(int x, int y);
int gld_init_game(void);
void gld_clear_board(void);
void print_board_helper(QuadtreeNode *node, Cell **sorted_list_head);
void gld_print_board(int generation_num);
void qt_debug_print_tree(QuadtreeNode *node, const char *id_str);

// String literals for fdprintf and debug_print_tree
const char *GENERATION_STATUS_FORMAT = "\nConway's Game of Life Board Status (Generation %d):\n";
const char *DEBUG_NW = "NW";
const char *DEBUG_NE = "NE";
const char *DEBUG_SW = "SW";
const char *DEBUG_SE = "SE";

// Helper function to prepend a Cell to a linked list
void prepend_cell(Cell **head, Cell *new_cell) {
    if (new_cell == NULL) return;
    new_cell->next = *head;
    new_cell->prev = NULL;
    if (*head != NULL) {
        (*head)->prev = new_cell;
    }
    *head = new_cell;
}

// Helper function to prepend a VisitedCell to a linked list
void prepend_visited_cell(VisitedCell **head, VisitedCell *new_cell) {
    if (new_cell == NULL) return;
    new_cell->next = *head;
    new_cell->prev = NULL;
    if (*head != NULL) {
        (*head)->prev = new_cell;
    }
    *head = new_cell;
}

// Helper function to free a linked list of Cells
void free_cell_list(Cell **head) {
    Cell *current = *head;
    while (current != NULL) {
        Cell *next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

// Helper function to free a linked list of VisitedCells
void free_visited_cell_list(VisitedCell **head) {
    VisitedCell *current = *head;
    while (current != NULL) {
        VisitedCell *next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

// Function: within_region
int within_region(int x, int y, int x_min, int y_min, int x_max, int y_max) {
    return (x >= x_min && x < x_max && y >= y_min && y < y_max);
}

// Function: create_cell_list
Cell *create_cell_list(QuadtreeNode *node, int filter_type) {
    Cell *new_list_head = NULL;

    if (node->is_subdivided == 0) {
        Cell *current_cell = node->cell_list_head;
        while (current_cell != NULL) {
            bool condition_met = false;
            // Filter conditions:
            // filter_type == 0: all cells (alive or dead, bomb or not)
            // filter_type == 1: alive cells (alive != 0)
            // filter_type == 2: dead cells that are not bombs (alive == 0 && bomb_timer == 0)
            // filter_type == 3: bomb cells (bomb_timer != 0)
            if ((filter_type == 0) ||
                (filter_type == 1 && current_cell->alive != 0) ||
                (filter_type == 3 && current_cell->bomb_timer != 0) ||
                (filter_type == 2 && current_cell->alive == 0 && current_cell->bomb_timer == 0)) {
                condition_met = true;
            }

            if (condition_met) {
                Cell *new_cell = (Cell *)calloc(1, sizeof(Cell));
                if (new_cell == NULL) {
                    free_cell_list(&new_list_head);
                    return NULL;
                }
                new_cell->x = current_cell->x;
                new_cell->y = current_cell->y;
                new_cell->alive = current_cell->alive;
                new_cell->bomb_timer = current_cell->bomb_timer;
                
                prepend_cell(&new_list_head, new_cell);
            }
            current_cell = current_cell->next;
        }
    } else {
        Cell *list_nw = create_cell_list(node->child_nw, filter_type);
        Cell *list_ne = create_cell_list(node->child_ne, filter_type);
        Cell *list_sw = create_cell_list(node->child_sw, filter_type);
        Cell *list_se = create_cell_list(node->child_se, filter_type);

        // Concatenate lists: NW -> NE -> SW -> SE
        if (list_nw != NULL) {
            new_list_head = list_nw;
            Cell *tail = list_nw;
            while (tail->next != NULL) {
                tail = tail->next;
            }
            if (list_ne != NULL) {
                tail->next = list_ne;
                list_ne->prev = tail;
                tail = list_ne;
                while (tail->next != NULL) {
                    tail = tail->next;
                }
            }
            if (list_sw != NULL) {
                tail->next = list_sw;
                list_sw->prev = tail;
                tail = list_sw;
                while (tail->next != NULL) {
                    tail = tail->next;
                }
            }
            if (list_se != NULL) {
                tail->next = list_se;
                list_se->prev = tail;
            }
        } else if (list_ne != NULL) {
            new_list_head = list_ne;
            Cell *tail = list_ne;
            while (tail->next != NULL) {
                tail = tail->next;
            }
            if (list_sw != NULL) {
                tail->next = list_sw;
                list_sw->prev = tail;
                tail = list_sw;
                while (tail->next != NULL) {
                    tail = tail->next;
                }
            }
            if (list_se != NULL) {
                tail->next = list_se;
                list_se->prev = tail;
            }
        } else if (list_sw != NULL) {
            new_list_head = list_sw;
            Cell *tail = list_sw;
            while (tail->next != NULL) {
                tail = tail->next;
            }
            if (list_se != NULL) {
                tail->next = list_se;
                list_se->prev = tail;
            }
        } else {
            new_list_head = list_se;
        }
    }
    return new_list_head;
}

// Function: create_adjacent_coords
int *create_adjacent_coords(int x, int y) {
    int *coords = (int *)malloc(16 * sizeof(int)); // 8 (x,y) pairs
    if (coords == NULL) return NULL;

    coords[0] = x - 1; coords[1] = y - 1;
    coords[2] = x;     coords[3] = y - 1;
    coords[4] = x + 1; coords[5] = y - 1;
    coords[6] = x - 1; coords[7] = y;
    coords[8] = x + 1; coords[9] = y;
    coords[10] = x - 1; coords[11] = y + 1;
    coords[12] = x;     coords[13] = y + 1;
    coords[14] = x + 1; coords[15] = y + 1;
    return coords;
}

// Function: bomb_collisions
int bomb_collisions(Cell *bomb_list_head) {
    int total_score = 0;
    Cell *current_bomb = bomb_list_head;

    while (current_bomb != NULL) {
        if (current_bomb->bomb_timer != 0) { // Only process bombs whose timer just reached 0
            current_bomb = current_bomb->next;
            continue;
        }

        int current_bomb_score = 0;
        int collision_count_for_bomb = 0;

        int *adjacent_coords_arr = create_adjacent_coords(current_bomb->x, current_bomb->y);
        if (adjacent_coords_arr == NULL) {
            current_bomb = current_bomb->next;
            continue;
        }

        int extended_coords[18]; // 9 (x,y) pairs (8 adjacent + 1 self)
        memcpy(extended_coords, adjacent_coords_arr, 8 * sizeof(int) * 2);
        extended_coords[16] = current_bomb->x; // Bomb's own coordinate
        extended_coords[17] = current_bomb->y;
        free(adjacent_coords_arr);

        for (int i = 0; i < 9; ++i) { // Iterate through 9 positions
            int target_x = extended_coords[i * 2];
            int target_y = extended_coords[i * 2 + 1];

            Cell *target_cell = qt_get_pixel(g_conway_gld, target_x, target_y);

            if (target_cell != NULL && target_cell->alive != 0) {
                target_cell->alive = 0;
                collision_count_for_bomb++;
                current_bomb_score = current_bomb_score + collision_count_for_bomb * current_bomb_score + 100;
                
                if (target_cell->bomb_timer == 0) {
                    qt_delete(g_conway_gld, target_cell->x, target_cell->y, target_cell->alive, target_cell->bomb_timer);
                }
            }
        }
        total_score += current_bomb_score;
        current_bomb = current_bomb->next;
    }
    return total_score;
}

// Function: is_dead_cell_generation
bool is_dead_cell_generation(int x, int y) {
    int *adjacent_coords = create_adjacent_coords(x, y);
    if (adjacent_coords == NULL) return false;

    int alive_neighbors_count = 0;
    for (int i = 0; i < 8; ++i) {
        Cell *neighbor_cell = qt_get_pixel(g_conway_gld, adjacent_coords[i * 2], adjacent_coords[i * 2 + 1]);
        if (neighbor_cell != NULL && neighbor_cell->alive != 0) {
            alive_neighbors_count++;
        }
    }
    free(adjacent_coords);
    return alive_neighbors_count == 3;
}

// Function: conway_step
void conway_step(int x, int y, int alive, int bomb_timer,
                Cell **new_alive_cells_list_head, Cell **new_dead_cells_list_head,
                VisitedCell **visited_cells_list_head) {
    int alive_neighbors_count = 0;
    int *adjacent_coords = create_adjacent_coords(x, y);
    if (adjacent_coords == NULL) return;

    for (int i = 0; i < 8; ++i) {
        int neighbor_x = adjacent_coords[i * 2];
        int neighbor_y = adjacent_coords[i * 2 + 1];

        if (neighbor_x >= 0 && neighbor_y >= 0 && neighbor_x < 0x2001 && neighbor_y < 0x2001) {
            VisitedCell *current_visited_cell = *visited_cells_list_head;
            bool already_visited = false;
            while (current_visited_cell != NULL) {
                if (current_visited_cell->x == neighbor_x && current_visited_cell->y == neighbor_y) {
                    already_visited = true;
                    break;
                }
                current_visited_cell = current_visited_cell->next;
            }

            Cell *neighbor_cell = qt_get_pixel(g_conway_gld, neighbor_x, neighbor_y);

            if (neighbor_cell == NULL || neighbor_cell->alive == 0) {
                if (!already_visited) {
                    VisitedCell *new_visited = (VisitedCell *)calloc(1, sizeof(VisitedCell));
                    if (new_visited == NULL) {
                        free(adjacent_coords);
                        return;
                    }
                    new_visited->x = neighbor_x;
                    new_visited->y = neighbor_y;
                    prepend_visited_cell(visited_cells_list_head, new_visited);

                    if (is_dead_cell_generation(neighbor_x, neighbor_y)) {
                        Cell *new_alive_cell = (Cell *)calloc(1, sizeof(Cell));
                        if (new_alive_cell == NULL) {
                             free(adjacent_coords);
                             return;
                        }
                        new_alive_cell->x = neighbor_x;
                        new_alive_cell->y = neighbor_y;
                        new_alive_cell->alive = 1;
                        new_alive_cell->bomb_timer = 0;
                        prepend_cell(new_alive_cells_list_head, new_alive_cell);
                    }
                }
            } else {
                alive_neighbors_count++;
            }
        }
    }
    free(adjacent_coords);

    VisitedCell *current_cell_visited = (VisitedCell *)calloc(1, sizeof(VisitedCell));
    if (current_cell_visited == NULL) return;
    current_cell_visited->x = x;
    current_cell_visited->y = y;
    prepend_visited_cell(visited_cells_list_head, current_cell_visited);

    Cell *processed_cell = (Cell *)calloc(1, sizeof(Cell));
    if (processed_cell == NULL) return;
    processed_cell->x = x;
    processed_cell->y = y;
    processed_cell->bomb_timer = bomb_timer;

    if (alive != 0) {
        if (alive_neighbors_count == 2 || alive_neighbors_count == 3) {
            processed_cell->alive = 1;
            prepend_cell(new_alive_cells_list_head, processed_cell);
        } else {
            processed_cell->alive = 0;
            prepend_cell(new_dead_cells_list_head, processed_cell);
        }
    } else {
        if (alive_neighbors_count == 3) {
            processed_cell->alive = 1;
            prepend_cell(new_alive_cells_list_head, processed_cell);
        } else {
            processed_cell->alive = 0;
            prepend_cell(new_dead_cells_list_head, processed_cell);
        }
    }
    return;
}

// Function: find_subregion_tree
QuadtreeNode *find_subregion_tree(QuadtreeNode *node, int x, int y) {
    if (node->is_subdivided == 0) {
        return NULL;
    }

    if (within_region(x, y, node->child_nw->x_min, node->child_nw->y_min,
                      node->child_nw->x_max, node->child_nw->y_max)) {
        return node->child_nw;
    }
    if (within_region(x, y, node->child_ne->x_min, node->child_ne->y_min,
                      node->child_ne->x_max, node->child_ne->y_max)) {
        return node->child_ne;
    }
    if (within_region(x, y, node->child_sw->x_min, node->child_sw->y_min,
                      node->child_sw->x_max, node->child_sw->y_max)) {
        return node->child_sw;
    }
    if (within_region(x, y, node->child_se->x_min, node->child_se->y_min,
                      node->child_se->x_max, node->child_se->y_max)) {
        return node->child_se;
    }
    return NULL;
}

// Function: subdivide
int subdivide(QuadtreeNode *node) {
    if (node->level == 0) {
        return -1;
    }

    int half_width = 1 << (node->level - 1);

    node->child_nw = qt_create(node->level - 1, node->x_min, node->y_min,
                               node->x_min + half_width, node->y_min + half_width);
    node->child_ne = qt_create(node->level - 1, node->x_min + half_width, node->y_min,
                               node->x_max, node->y_min + half_width);
    node->child_sw = qt_create(node->level - 1, node->x_min, node->y_min + half_width,
                               node->x_min + half_width, node->y_max);
    node->child_se = qt_create(node->level - 1, node->x_min + half_width, node->y_min + half_width,
                               node->x_max, node->y_max);

    if (!node->child_nw || !node->child_ne || !node->child_sw || !node->child_se) {
        if (node->child_nw) free(node->child_nw);
        if (node->child_ne) free(node->child_ne);
        if (node->child_sw) free(node->child_sw);
        if (node->child_se) free(node->child_se);
        node->child_nw = node->child_ne = node->child_sw = node->child_se = NULL;
        return -1;
    }

    node->is_subdivided = 1;
    return 0;
}

// Function: move_pixels
void move_pixels(QuadtreeNode *node, Cell *cells_to_move_head, int count_to_move) {
    if (count_to_move == 0 || cells_to_move_head == NULL) {
        return;
    }

    if (node->cell_list_head == NULL) {
        node->cell_list_head = cells_to_move_head;
    } else {
        Cell *current_tail = node->cell_list_head;
        while (current_tail->next != NULL) {
            current_tail = current_tail->next;
        }
        current_tail->next = cells_to_move_head;
        cells_to_move_head->prev = current_tail;
    }
    node->count += count_to_move;
}

// Function: qt_undivide
int qt_undivide(QuadtreeNode *node) {
    if (node->is_subdivided == 0) {
        return -1;
    }

    if (node->child_nw->is_subdivided == 0 &&
        node->child_ne->is_subdivided == 0 &&
        node->child_sw->is_subdivided == 0 &&
        node->child_se->is_subdivided == 0) {

        int total_child_cells = node->child_nw->count + node->child_ne->count +
                                node->child_sw->count + node->child_se->count;
        if (node->capacity < (unsigned int)(node->count + total_child_cells)) {
            return -1;
        }

        move_pixels(node, node->child_nw->cell_list_head, node->child_nw->count);
        move_pixels(node, node->child_ne->cell_list_head, node->child_ne->count);
        move_pixels(node, node->child_sw->cell_list_head, node->child_sw->count);
        move_pixels(node, node->child_se->cell_list_head, node->child_se->count);

        node->child_nw->cell_list_head = NULL;
        free(node->child_nw);
        node->child_ne->cell_list_head = NULL;
        free(node->child_ne);
        node->child_sw->cell_list_head = NULL;
        free(node->child_sw);
        node->child_se->cell_list_head = NULL;
        free(node->child_se);

        node->child_nw = NULL;
        node->child_ne = NULL;
        node->child_sw = NULL;
        node->child_se = NULL;
        node->is_subdivided = 0;
        return 0;
    }
    return -1;
}

// Function: qt_delete
int qt_delete(QuadtreeNode *node, int x, int y, int alive, int bomb_timer) {
    if (!within_region(x, y, node->x_min, node->y_min, node->x_max, node->y_max)) {
        return -1;
    }

    QuadtreeNode *subregion = find_subregion_tree(node, x, y);
    if (subregion == NULL) {
        Cell *current_cell = node->cell_list_head;
        while (current_cell != NULL) {
            if (x == current_cell->x && y == current_cell->y) {
                if (current_cell->prev != NULL) {
                    current_cell->prev->next = current_cell->next;
                } else {
                    node->cell_list_head = current_cell->next;
                }
                if (current_cell->next != NULL) {
                    current_cell->next->prev = current_cell->prev;
                }
                free(current_cell);
                node->count--;
                return 0;
            }
            current_cell = current_cell->next;
        }
        return -1;
    } else {
        int result = qt_delete(subregion, x, y, alive, bomb_timer);
        if (result == 0) {
            return qt_undivide(node);
        }
        return -1;
    }
}

// Function: qt_create
QuadtreeNode *qt_create(int level, int x_min, int y_min, int x_max, int y_max) {
    QuadtreeNode *new_node = (QuadtreeNode *)malloc(sizeof(QuadtreeNode));
    if (new_node == NULL) return NULL;

    new_node->level = level;
    new_node->capacity = 16; // 0x10
    new_node->count = 0;
    new_node->cell_list_head = NULL;
    new_node->x_min = x_min;
    new_node->y_min = y_min;
    new_node->x_max = x_max;
    new_node->y_max = y_max;
    new_node->is_subdivided = 0;

    new_node->child_nw = NULL;
    new_node->child_ne = NULL;
    new_node->child_sw = NULL;
    new_node->child_se = NULL;

    return new_node;
}

// Function: qt_insert
int qt_insert(QuadtreeNode *node, int x, int y, int alive, int bomb_timer) {
    if (!within_region(x, y, node->x_min, node->y_min, node->x_max, node->y_max)) {
        return -1;
    }

    QuadtreeNode *subregion = find_subregion_tree(node, x, y);
    if (subregion == NULL) {
        Cell *current_cell = node->cell_list_head;
        while (current_cell != NULL) {
            if (x == current_cell->x && y == current_cell->y) {
                current_cell->alive = alive;
                current_cell->bomb_timer = bomb_timer;
                return 0;
            }
            current_cell = current_cell->next;
        }

        if (node->count < node->capacity) {
            Cell *new_cell = (Cell *)calloc(1, sizeof(Cell));
            if (new_cell == NULL) return -1;

            new_cell->x = x;
            new_cell->y = y;
            new_cell->alive = alive;
            new_cell->bomb_timer = bomb_timer;
            prepend_cell(&node->cell_list_head, new_cell);
            node->count++;
            return 0;
        } else {
            if (subdivide(node) != 0) {
                return -1;
            }

            Cell *current_list_head = node->cell_list_head;
            node->cell_list_head = NULL;
            node->count = 0;

            Cell *temp_cell;
            while (current_list_head != NULL) {
                temp_cell = current_list_head;
                current_list_head = current_list_head->next;
                temp_cell->next = NULL;
                temp_cell->prev = NULL;

                QuadtreeNode *target_child = find_subregion_tree(node, temp_cell->x, temp_cell->y);
                if (target_child == NULL || qt_insert(target_child, temp_cell->x, temp_cell->y,
                                                      temp_cell->alive, temp_cell->bomb_timer) != 0) {
                    free(temp_cell);
                    return -1;
                }
                free(temp_cell);
            }
            subregion = find_subregion_tree(node, x, y);
            if (subregion == NULL) return -1;
            return qt_insert(subregion, x, y, alive, bomb_timer);
        }
    } else {
        return qt_insert(subregion, x, y, alive, bomb_timer);
    }
}

// Function: qt_get_pixel
Cell *qt_get_pixel(QuadtreeNode *node, int x, int y) {
    if (!within_region(x, y, node->x_min, node->y_min, node->x_max, node->y_max)) {
        return NULL;
    }

    QuadtreeNode *subregion = find_subregion_tree(node, x, y);
    if (subregion == NULL) {
        Cell *current_cell = node->cell_list_head;
        while (current_cell != NULL) {
            if (x == current_cell->x && y == current_cell->y) {
                return current_cell;
            }
            current_cell = current_cell->next;
        }
        return NULL;
    } else {
        return qt_get_pixel(subregion, x, y);
    }
}

// Function: cg_step
int cg_step(int max_steps) {
    int total_score = 0;

    for (int current_step = 0; current_step < max_steps; ++current_step) {
        Cell *alive_cells_in_gen = create_cell_list(g_conway_gld, 1);
        Cell *new_alive_cells_list_head = NULL;
        Cell *new_dead_cells_list_head = NULL;
        VisitedCell *visited_cells_list_head = NULL;

        Cell *current_cell_in_list = alive_cells_in_gen;
        while (current_cell_in_list != NULL) {
            conway_step(current_cell_in_list->x, current_cell_in_list->y,
                        current_cell_in_list->alive, current_cell_in_list->bomb_timer,
                        &new_alive_cells_list_head, &new_dead_cells_list_head,
                        &visited_cells_list_head);
            current_cell_in_list = current_cell_in_list->next;
        }
        free_cell_list(&alive_cells_in_gen);
        free_visited_cell_list(&visited_cells_list_head);

        current_cell_in_list = new_alive_cells_list_head;
        while (current_cell_in_list != NULL) {
            qt_insert(g_conway_gld, current_cell_in_list->x, current_cell_in_list->y,
                      current_cell_in_list->alive, current_cell_in_list->bomb_timer);
            current_cell_in_list = current_cell_in_list->next;
        }
        free_cell_list(&new_alive_cells_list_head);

        current_cell_in_list = new_dead_cells_list_head;
        while (current_cell_in_list != NULL) {
            qt_insert(g_conway_gld, current_cell_in_list->x, current_cell_in_list->y,
                      current_cell_in_list->alive, current_cell_in_list->bomb_timer);
            current_cell_in_list = current_cell_in_list->next;
        }
        free_cell_list(&new_dead_cells_list_head);

        Cell *bomb_cells_list = create_cell_list(g_conway_gld, 3);
        Cell *sorted_bomb_list_head = NULL;

        current_cell_in_list = bomb_cells_list;
        while (current_cell_in_list != NULL) {
            current_cell_in_list->bomb_timer--;
            qt_insert(g_conway_gld, current_cell_in_list->x, current_cell_in_list->y,
                      current_cell_in_list->alive, current_cell_in_list->bomb_timer);

            Cell *new_bomb_cell_for_sorted_list = (Cell *)calloc(1, sizeof(Cell));
            if (new_bomb_cell_for_sorted_list == NULL) {
                free_cell_list(&sorted_bomb_list_head);
                free_cell_list(&bomb_cells_list);
                return -1;
            }
            memcpy(new_bomb_cell_for_sorted_list, current_cell_in_list, sizeof(Cell) - 2 * sizeof(Cell*));

            if (sorted_bomb_list_head == NULL) {
                sorted_bomb_list_head = new_bomb_cell_for_sorted_list;
            } else {
                Cell *current_sorted = sorted_bomb_list_head;
                Cell *prev_sorted = NULL;
                while (current_sorted != NULL) {
                    if (new_bomb_cell_for_sorted_list->x < current_sorted->x ||
                        (new_bomb_cell_for_sorted_list->x == current_sorted->x && new_bomb_cell_for_sorted_list->y <= current_sorted->y)) {
                        break;
                    }
                    prev_sorted = current_sorted;
                    current_sorted = current_sorted->next;
                }
                if (prev_sorted == NULL) {
                    new_bomb_cell_for_sorted_list->next = sorted_bomb_list_head;
                    sorted_bomb_list_head->prev = new_bomb_cell_for_sorted_list;
                    sorted_bomb_list_head = new_bomb_cell_for_sorted_list;
                } else {
                    new_bomb_cell_for_sorted_list->next = current_sorted;
                    new_bomb_cell_for_sorted_list->prev = prev_sorted;
                    prev_sorted->next = new_bomb_cell_for_sorted_list;
                    if (current_sorted != NULL) {
                        current_sorted->prev = new_bomb_cell_for_sorted_list;
                    }
                }
            }
            current_cell_in_list = current_cell_in_list->next;
        }
        free_cell_list(&bomb_cells_list);

        total_score += bomb_collisions(sorted_bomb_list_head);
        free_cell_list(&sorted_bomb_list_head);

        Cell *dead_non_bomb_cells = create_cell_list(g_conway_gld, 2);
        current_cell_in_list = dead_non_bomb_cells;
        while (current_cell_in_list != NULL) {
            qt_delete(g_conway_gld, current_cell_in_list->x, current_cell_in_list->y,
                      current_cell_in_list->alive, current_cell_in_list->bomb_timer);
            current_cell_in_list = current_cell_in_list->next;
        }
        free_cell_list(&dead_non_bomb_cells);
    }
    return total_score;
}

// Function: shoot_pixel
int shoot_pixel(int x, int y) {
    Cell *target_cell = qt_get_pixel(g_conway_gld, x, y);
    if (target_cell == NULL || target_cell->alive == 0) {
        return 0;
    } else {
        target_cell->alive = 0;
        if (target_cell->bomb_timer == 0) {
            qt_delete(g_conway_gld, target_cell->x, target_cell->y, target_cell->alive, target_cell->bomb_timer);
        }
        return 100;
    }
}

// Function: set_bomb
bool set_bomb(int x, int y) {
    Cell *target_cell = qt_get_pixel(g_conway_gld, x, y);
    if (target_cell == NULL) {
        return qt_insert(g_conway_gld, x, y, 0, 10) == 0;
    } else if (target_cell->bomb_timer == 0) {
        target_cell->bomb_timer = 10;
        return true;
    } else {
        return false;
    }
}

// Function: gld_init_game
int gld_init_game(void) {
    if (g_conway_gld == NULL) {
        g_conway_gld = qt_create(13, 0, 0, 0x2000, 0x2000); // Level 13, region 0,0 to 8192,8192
    }
    return g_conway_gld != NULL ? 0 : -1;
}

// Function: gld_clear_board
void gld_clear_board(void) {
    if (g_conway_gld == NULL) return;

    Cell *all_cells_list = create_cell_list(g_conway_gld, 0);
    Cell *current_cell = all_cells_list;
    while (current_cell != NULL) {
        qt_delete(g_conway_gld, current_cell->x, current_cell->y, current_cell->alive, current_cell->bomb_timer);
        current_cell = current_cell->next;
    }
    free_cell_list(&all_cells_list);
}

// Function: print_board_helper
void print_board_helper(QuadtreeNode *node, Cell **sorted_list_head) {
    if (node == NULL) return;

    if (node->is_subdivided != 0) {
        print_board_helper(node->child_nw, sorted_list_head);
        print_board_helper(node->child_ne, sorted_list_head);
        print_board_helper(node->child_sw, sorted_list_head);
        print_board_helper(node->child_se, sorted_list_head);
    }

    Cell *current_cell = node->cell_list_head;
    while (current_cell != NULL) {
        if (current_cell->alive != 0 || current_cell->bomb_timer != 0) {
            Cell *new_cell = (Cell *)calloc(1, sizeof(Cell));
            if (new_cell == NULL) return;

            new_cell->x = current_cell->x;
            new_cell->y = current_cell->y;
            new_cell->alive = current_cell->alive;
            new_cell->bomb_timer = current_cell->bomb_timer;

            if (*sorted_list_head == NULL) {
                *sorted_list_head = new_cell;
            } else {
                Cell *current_sorted = *sorted_list_head;
                Cell *prev_sorted = NULL;
                while (current_sorted != NULL) {
                    if (new_cell->x < current_sorted->x ||
                        (new_cell->x == current_sorted->x && new_cell->y <= current_sorted->y)) {
                        break;
                    }
                    prev_sorted = current_sorted;
                    current_sorted = current_sorted->next;
                }
                if (prev_sorted == NULL) {
                    new_cell->next = *sorted_list_head;
                    (*sorted_list_head)->prev = new_cell;
                    *sorted_list_head = new_cell;
                } else {
                    new_cell->next = current_sorted;
                    new_cell->prev = prev_sorted;
                    prev_sorted->next = new_cell;
                    if (current_sorted != NULL) {
                        current_sorted->prev = new_cell;
                    }
                }
            }
        }
        current_cell = current_cell->next;
    }
}

// Function: gld_print_board
void gld_print_board(int generation_num) {
    Cell *sorted_cells_to_print = NULL;
    print_board_helper(g_conway_gld, &sorted_cells_to_print);

    fprintf(stdout, GENERATION_STATUS_FORMAT, generation_num);
    Cell *current_cell = sorted_cells_to_print;
    while (current_cell != NULL) {
        fprintf(stdout, "--| (x,y) = (%d,%d) | Alive=%d | Bomb=%d |\n",
                current_cell->x, current_cell->y, current_cell->alive, current_cell->bomb_timer);
        current_cell = current_cell->next;
    }
    free_cell_list(&sorted_cells_to_print);
}

// Function: qt_debug_print_tree
void qt_debug_print_tree(QuadtreeNode *node, const char *id_str) {
    if (node == NULL) return;

    if (node->count != 0) {
        int indent_level = 13 - node->level;
        for (int i = 0; i < indent_level; ++i) {
            fprintf(stdout, "    ");
        }
        fprintf(stdout, "Level: %d, ID: %s, Region: (%d,%d)-(%d,%d), Cells: %d\n",
                node->level, id_str, node->x_min, node->y_min, node->x_max, node->y_max, node->count);

        Cell *current_cell = node->cell_list_head;
        while (current_cell != NULL) {
            for (int i = 0; i < indent_level; ++i) {
                fprintf(stdout, "    ");
            }
            fprintf(stdout, "    >--| (x,y) = (%d,%d) | Alive=%d | Bomb=%d |\n",
                    current_cell->x, current_cell->y, current_cell->alive, current_cell->bomb_timer);
            current_cell = current_cell->next;
        }
    }

    if (node->is_subdivided) {
        qt_debug_print_tree(node->child_nw, DEBUG_NW);
        qt_debug_print_tree(node->child_ne, DEBUG_NE);
        qt_debug_print_tree(node->child_sw, DEBUG_SW);
        qt_debug_print_tree(node->child_se, DEBUG_SE);
    }
}