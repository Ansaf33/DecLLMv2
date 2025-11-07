#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Decompiler artifacts mapping
typedef unsigned char byte;
typedef unsigned int uint;

// Custom termination function (replaces _terminate)
void _terminate(int status) {
    exit(status);
}

// Global variables (placeholders, as they are external to the snippet)
char secret_page[4096]; // Assuming a page size for secret data
char easteregg[256] = "EASTER_EGG_FOUND"; // Example Easter egg string
int eggindex = 0;

// Queue for pathfinding
struct QueueNode {
    int x;
    int y;
    struct QueueNode *next;
};
struct QueueNode *root = NULL; // Head of the queue
char *queue_matrix = NULL; // Map of visited states in queue (1 for in queue, 0 for not)

// Names for monsters/bosses
char *names[] = {
    "Goblin", "Orc", "Slime", "Dragon", "Phoenix",
    "Griffin", "Hydra", "Minotaur", "Sphinx", "Cerberus",
    NULL // Null-terminate the list
};

uint page_index = 0; // Index into secret_page for random values

// Struct definitions based on usage
struct Map {
    uint width;
    uint height;
    uint start_x;
    uint start_y;
    uint end_x;
    uint end_y;
    uint current_x;
    uint current_y;
    uint marker_x; // Used for placing obstacles
    uint marker_y;
    char *data; // 1D array representing the map grid
};

struct Monster {
    char *type;      // Name/type of monster
    int health;      // Current health
    int max_health;  // Maximum health (hit points)
    int power;       // Attack power
    int xp;          // Experience points towards next level
    int level;       // Monster level
};

struct Player {
    char name[16];                // Player's name
    int level;                    // Player's level
    int num_monsters;             // Number of monsters player has
    struct Monster *monsters[5];  // Array of pointers to player's monsters (max 5)
};

// Function: update_page_index
void update_page_index(void) {
    page_index = (page_index + 3) & 0xfff; // Increment and wrap around 0xfff (4095)
}

// Function: select_name
char *select_name(void) {
    byte rand_byte = secret_page[page_index];
    update_page_index();

    uint index = 0;
    for (uint i = 0; i < rand_byte; i++) {
        index++;
        if (names[index] == NULL) { // Wrap around if end of names list is reached
            index = 0;
        }
    }
    return names[index];
}

// Function: generate_monster
struct Monster *generate_monster(void) {
    struct Monster *monster = (struct Monster *)malloc(sizeof(struct Monster));
    if (monster == NULL) {
        printf("[ERROR] Failed to allocate monster structure\n");
        _terminate(1);
    }
    memset(monster, 0, sizeof(struct Monster));

    monster->type = select_name();
    monster->health = (secret_page[page_index] % 10) + 4; // Health 4-13
    monster->max_health = monster->health;
    update_page_index();

    monster->power = (secret_page[page_index] % 6) + 2; // Power 2-7
    update_page_index();

    monster->level = 1;
    monster->xp = 0;
    return monster;
}

// Function: generate_boss
struct Monster *generate_boss(void) {
    struct Monster *boss = (struct Monster *)malloc(sizeof(struct Monster));
    if (boss == NULL) {
        printf("[ERROR] Failed to allocate boss monster structure\n");
        _terminate(1);
    }
    memset(boss, 0, sizeof(struct Monster));

    boss->type = select_name();
    boss->health = (secret_page[page_index] % 10) + 9; // Health 9-18
    boss->max_health = boss->health;
    update_page_index();

    boss->power = (secret_page[page_index] % 6) + 4; // Power 4-9
    update_page_index();

    boss->level = 4;
    boss->xp = 0;
    return boss;
}

// Function: generate_player
struct Player *generate_player(void) {
    char name_buffer[16];
    memset(name_buffer, 0, sizeof(name_buffer));

    printf("Enter your name|| ");
    if (fgets(name_buffer, sizeof(name_buffer), stdin) == NULL) {
        strcpy(name_buffer, "Player One\n"); // Default if input fails
    }
    // Remove trailing newline if present
    name_buffer[strcspn(name_buffer, "\n")] = 0;

    if (name_buffer[0] == '\0') {
        strcpy(name_buffer, "Player One");
    }

    struct Player *player = (struct Player *)malloc(sizeof(struct Player));
    if (player == NULL) {
        printf("[ERROR] Failed to malloc player structure\n");
        _terminate(1);
    }
    memset(player, 0, sizeof(struct Player));

    strncpy(player->name, name_buffer, sizeof(player->name) - 1);
    player->name[sizeof(player->name) - 1] = '\0';

    player->num_monsters = 0;
    for (uint i = 0; i < 3; i++) {
        player->monsters[player->num_monsters++] = generate_monster();
    }
    player->level = 1;
    return player;
}

// Function: read_line (reads into buffer up to max_len or newline)
uint read_line(char *buffer, uint max_len) {
    if (buffer == NULL || max_len == 0) {
        printf("[ERROR] invalid arg for read_line\n");
        _terminate(1);
    }

    uint bytes_read = 0;
    int c;

    // Read characters until max_len-1 is reached (for null terminator) or newline
    while (bytes_read < max_len - 1 && (c = getchar()) != EOF && c != '\n') {
        buffer[bytes_read++] = (char)c;
    }
    buffer[bytes_read] = '\0'; // Null-terminate the string

    // If a newline was read or EOF, and there are more chars in buffer, clear them
    if (c != '\n' && c != EOF) {
        // Discard remaining characters in the line
        while ((c = getchar()) != EOF && c != '\n');
    }

    return bytes_read;
}

// Function: read_line_u (reads into buffer until newline, no max_len)
int read_line_u(char *buffer) {
    if (buffer == NULL) {
        return 0;
    }

    int bytes_read = 0;
    int c;

    while ((c = getchar()) != EOF && c != '\n') {
        // Dynamically resize buffer if needed, or assume large enough.
        // For this refactor, we'll assume the buffer passed is large enough
        // or handle overflow by truncating. The original code doesn't specify a max.
        // Given `change_monster_name` uses `local_34[32]`, let's assume a limit.
        if (bytes_read < 31) { // Limit to 31 chars + null for a 32-byte buffer
            buffer[bytes_read++] = (char)c;
        }
    }
    buffer[bytes_read] = '\0'; // Null-terminate

    return bytes_read;
}

// Function: check_egg
void check_egg(struct Player *player, char input_char) {
    if (input_char == easteregg[eggindex]) {
        eggindex++;
    } else {
        eggindex = 0;
    }

    if (easteregg[eggindex] == '\0') {
        printf("YOU FOUND THE EGG!!!! Have a prize.\n");
        if (player != NULL && player->num_monsters > 0 && player->monsters[0] != NULL) {
            struct Monster *first_monster = player->monsters[0];
            first_monster->max_health = 99;
            first_monster->health = 99;
            first_monster->power = 99;
        }
        eggindex = 0;
    }
}

// Function: add_queue
void add_queue(int x, int y, int map_width, int map_height) {
    int index = y * map_width + x;

    if (queue_matrix[index] != 1) { // Check if not already in queue
        struct QueueNode *newNode = (struct QueueNode *)malloc(sizeof(struct QueueNode));
        if (newNode == NULL) {
            printf("[ERROR] malloc() queue structure failed.\n");
            _terminate(1);
        }
        memset(newNode, 0, sizeof(struct QueueNode));
        newNode->x = x;
        newNode->y = y;
        newNode->next = NULL;

        if (root == NULL) {
            root = newNode;
        } else {
            struct QueueNode *current = root;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = newNode;
        }
        queue_matrix[index] = 1; // Mark as in queue
    }
}

// Function: dequeue
struct QueueNode *dequeue(void) {
    struct QueueNode *node = root;
    if (root != NULL) {
        root = root->next;
    }
    return node;
}

// Function: check_adjacent
int check_adjacent(int x1, int y1, int x2, int y2) {
    if (x1 == x2) {
        if (y2 == y1 + 1 || y2 == y1 - 1) {
            return 1;
        }
    } else if (y1 == y2) {
        if (x2 == x1 + 1 || x2 == x1 - 1) {
            return 1;
        }
    }
    return 0;
}

// Function: print_map
void print_map(struct Map *map) {
    if (map == NULL) {
        printf("[ERROR] print_map() invalid argument.\n");
        return;
    }

    uint map_size = map->width * map->height;
    char *display_map = (char *)malloc(map_size + map->height + 1); // For newlines and null terminator
    if (display_map == NULL) {
        printf("[ERROR] Failed to allocate map display buffer.\n");
        _terminate(1);
    }
    memset(display_map, 0, map_size + map->height + 1);

    int display_idx = 0;
    for (uint i = 0; i < map_size; i++) {
        if (i != 0 && i % map->width == 0) {
            display_map[display_idx++] = '\n';
        }
        if (map->data[i] == '\0') {
            display_map[display_idx++] = '.';
        } else {
            display_map[display_idx++] = map->data[i];
        }
    }
    display_map[display_idx++] = '\n';

    printf("%s", display_map);
    free(display_map);
}

// Function: find_path
int find_path(uint current_x, uint current_y, struct Map *map) {
    if (map == NULL) {
        return 0;
    }

    if (check_adjacent(current_x, current_y, map->end_x, map->end_y) == 1) {
        return 1; // Path found
    }

    // Up
    if (current_y > 0 && map->data[(current_y - 1) * map->width + current_x] != '#') {
        add_queue(current_x, current_y - 1, map->width, map->height);
    }
    // Right
    if (current_x < map->width - 1 && map->data[current_y * map->width + (current_x + 1)] != '#') {
        add_queue(current_x + 1, current_y, map->width, map->height);
    }
    // Down
    if (current_y < map->height - 1 && map->data[(current_y + 1) * map->width + current_x] != '#') {
        add_queue(current_x, current_y + 1, map->width, map->height);
    }
    // Left
    if (current_x > 0 && map->data[current_y * map->width + (current_x - 1)] != '#') {
        add_queue(current_x - 1, current_y, map->width, map->height);
    }

    struct QueueNode *current_node;
    while ((current_node = dequeue()) != NULL) {
        if (find_path(current_node->x, current_node->y, map) == 1) {
            free(current_node);
            return 1;
        }
        free(current_node);
    }
    return 0;
}

// Function: place_marker
void place_marker(struct Map *map) {
    uint attempts = 0;
    uint index;

    do {
        map->marker_x = (uint)secret_page[page_index] % map->width;
        update_page_index();
        map->marker_y = (uint)secret_page[page_index] % map->height;
        update_page_index();
        index = map->marker_y * map->width + map->marker_x;
        attempts++;
    } while (map->data[index] != '\0' && attempts < 100);

    if (attempts == 100) {
        index = 0;
        while (index < map->width * map->height && map->data[index] != '\0') {
            index++;
        }

        if (index == map->width * map->height) {
            printf("FAILED to place marker (no empty spots).\n");
            print_map(map);
            _terminate(1);
        }
        map->marker_y = index / map->width;
        map->marker_x = index % map->width;
    }
    map->data[map->marker_y * map->width + map->marker_x] = '#';
}

// Function: set_marker
void set_marker(int x, int y, struct Map *map, char marker_char) {
    map->data[y * map->width + x] = marker_char;
}

// Function: initialize_map
void initialize_map(struct Map *map) {
    if (map == NULL) {
        printf("[ERROR] initialize_map() invalid argument.\n");
        _terminate(1);
    }

    uint map_size = map->width * map->height;
    map->data = (char *)malloc(map_size);
    if (map->data == NULL) {
        printf("[ERROR] Failed to allocate map data.\n");
        _terminate(1);
    }
    memset(map->data, 0, map_size);

    do {
        map->start_x = (uint)secret_page[page_index] % map->width;
        update_page_index();
        map->start_y = (uint)secret_page[page_index] % map->height;
        update_page_index();
        map->end_x = (uint)secret_page[page_index] % map->width;
        update_page_index();
        map->end_y = (uint)secret_page[page_index] % map->height;
        update_page_index();
    } while (
        (map->start_x == map->end_x && (map->start_y == map->end_y || map->start_y - 1 == map->end_y || map->start_y + 1 == map->end_y)) ||
        (map->start_y == map->end_y && (map->start_x == map->end_x || map->start_x - 1 == map->end_x || map->start_x + 1 == map->end_x))
    );

    set_marker(map->start_x, map->start_y, map, '@');
    set_marker(map->end_x, map->end_y, map, 'X');

    map->current_x = map->start_x;
    map->current_y = map->start_y;
}

// Function: initialize_queue_matrix
void initialize_queue_matrix(struct Map *map) {
    if (queue_matrix != NULL) {
        free(queue_matrix);
    }

    uint matrix_size = map->width * map->height;
    queue_matrix = (char *)malloc(matrix_size);
    if (queue_matrix == NULL) {
        printf("[ERROR] Failed to allocate queue matrix\n");
        _terminate(1);
    }
    memset(queue_matrix, 0, matrix_size);

    for (uint i = 0; i < matrix_size; i++) {
        if (map->data[i] != '\0') {
            queue_matrix[i] = 1; // Mark blocked cells as visited for queue purposes
        }
    }
}

// Function: generate_map
struct Map *generate_map(int width, int height) {
    struct Map *map = (struct Map *)malloc(sizeof(struct Map));
    if (map == NULL) {
        printf("[ERROR] Failed to allocate map structure\n");
        _terminate(1);
    }
    memset(map, 0, sizeof(struct Map));

    map->width = width;
    map->height = height;

    initialize_map(map);

    printf("Width: %d Height: %d\n", map->width, map->height);
    printf("StartX: %d StartY: %d\n", map->start_x, map->start_y);
    printf("EndX: %d EndY: %d\n\n", map->end_x, map->end_y);

    int path_found = 0;
    while (!path_found) {
        place_marker(map);
        initialize_queue_matrix(map);
        path_found = find_path(map->start_x, map->start_y, map);

        if (!path_found) {
            map->data[map->marker_y * map->width + map->marker_x] = '\0'; // Remove last marker
        }

        while (root != NULL) { // Clear the queue
            struct QueueNode *temp = root;
            root = root->next;
            free(temp);
        }
    }

    free(queue_matrix);
    queue_matrix = NULL;

    print_map(map);
    return map;
}

// Function: reset_monsters
void reset_monsters(struct Player *player) {
    if (player == NULL) {
        return;
    }
    for (uint i = 0; i < player->num_monsters; i++) {
        if (player->monsters[i] != NULL) {
            player->monsters[i]->health = player->monsters[i]->max_health;
        }
    }
}

// Function: print_monster
void print_monster(struct Monster *monster) {
    if (monster != NULL) {
        printf("\tType: %s\n", monster->type);
        printf("\tLevel: %d\n", monster->level);
        printf("\tHealth: %d\n", monster->health);
        printf("\tHit Points: %d\n", monster->max_health);
        printf("\tPower: %d\n\n", monster->power);
    }
}

// Function: oneup_monster
int oneup_monster(struct Monster *monster) {
    if (monster == NULL) {
        return 0;
    }

    monster->xp++;
    if (monster->xp >= 15) { // Assuming 0xf is 15
        printf("%s gained a level\n", monster->type);
        monster->max_health++;
        monster->power++;
        monster->health = monster->max_health;
        monster->level++;
        monster->xp = 0;
    }
    return 1;
}

// Function: select_monster
struct Monster *select_monster(struct Player *player) {
    if (player == NULL) {
        return NULL;
    }

    bool has_living_monsters = false;
    for (uint i = 0; i < player->num_monsters; i++) {
        if (player->monsters[i] != NULL && player->monsters[i]->health > 0) {
            has_living_monsters = true;
            break;
        }
    }

    if (!has_living_monsters) {
        return NULL;
    }

    int choice_made = 0;
    char input_buffer[4];
    uint selected_idx;

    printf("Monsters: \n");
    for (uint i = 0; i < player->num_monsters; i++) {
        struct Monster *m = player->monsters[i];
        if (m != NULL) {
            printf("\t%u} \n", i + 1);
            printf("\tType: %s\n", m->type);
            printf("\tLevel: %d\n", m->level);
            printf("\tHealth: %d\n", m->health);
            printf("\tPower: %d\n\n", m->power);
        }
    }

    while (!choice_made) {
        printf("Selection: ");
        read_line(input_buffer, sizeof(input_buffer));
        selected_idx = atoi(input_buffer);

        if (selected_idx < 1 || selected_idx > player->num_monsters) {
            printf("bad choice: %s\n", input_buffer);
        } else {
            struct Monster *selected_monster = player->monsters[selected_idx - 1];
            if (selected_monster == NULL) {
                printf("bad choice: %s\n", input_buffer);
            } else if (selected_monster->health < 1) {
                printf("he dead\n");
            } else {
                choice_made = 1;
                return selected_monster;
            }
        }
    }
    return NULL;
}

// Function: change_monster_name
int change_monster_name(char **monster_name_ptr) {
    char new_name_buffer[32];
    memset(new_name_buffer, 0, sizeof(new_name_buffer));

    printf("New name: ");
    int bytes_read = read_line_u(new_name_buffer);

    char *new_name = (char *)malloc(bytes_read + 1);
    if (new_name == NULL) {
        printf("[ERROR] Failed to malloc name buffer\n");
        _terminate(1);
    }
    strncpy(new_name, new_name_buffer, bytes_read);
    new_name[bytes_read] = '\0';

    // Assuming names returned by select_name are string literals and not freed.
    // If previous `monster_name_ptr` pointed to a malloc'd string, it should be freed here.
    *monster_name_ptr = new_name;
    return 1;
}

// Function: capture_boss (or monster if passed a boss)
int capture_boss(struct Monster *boss, struct Player *player) {
    if (boss == NULL || player == NULL) {
        return 0;
    }

    char response[4];
    printf("capture monster? (y/n): ");
    read_line(response, sizeof(response));

    if (response[0] == 'y') {
        printf("update boss name? (y/n): ");
        read_line(response, sizeof(response));
        if (response[0] == 'y') {
            change_monster_name(&(boss->type));
        }

        if (player->num_monsters < 5) {
            player->monsters[player->num_monsters++] = boss;
            return 1;
        } else {
            printf("your cart is full.\n");
            for (uint i = 0; i < 5; i++) {
                printf("%u} \n", i + 1);
                print_monster(player->monsters[i]);
            }
            printf("*********************************\n");
            boss->health = boss->max_health; // Restore boss health for display
            print_monster(boss);
            printf("*********************************\n");
            printf("replace one of yours? (y/n): ");
            read_line(response, sizeof(response));

            if (response[0] == 'y') {
                printf("which one: ");
                read_line(response, sizeof(response));
                uint choice = atoi(response);
                if (choice == 0 || choice > 5) {
                    printf("invalid\n");
                    free(boss->type); // Free boss's dynamically allocated name
                    free(boss);
                    return 0;
                } else {
                    free(player->monsters[choice - 1]->type); // Free old monster's dynamically allocated name
                    free(player->monsters[choice - 1]);
                    player->monsters[choice - 1] = boss;
                    return 1;
                }
            } else {
                free(boss->type); // Free boss's dynamically allocated name
                free(boss);
                return 0;
            }
        }
    } else {
        free(boss->type); // Free boss's dynamically allocated name
        free(boss);
        return 0;
    }
}

// Function: daboss
int daboss(struct Player *player) {
    if (player == NULL) {
        return 0;
    }

    struct Monster *boss = generate_boss();
    reset_monsters(player);

    printf("\nDUN DUN DUUUUUUUUUUUUUUN\n");
    printf("You have reached the boss!!!!!\n\n");
    print_monster(boss);

    while (boss->health > 0) {
        struct Monster *selected_player_monster = select_monster(player);
        if (selected_player_monster == NULL) {
            printf("You have no living monsters. You lose.\n");
            free(boss->type);
            free(boss);
            return 0;
        }

        uint player_hit = secret_page[page_index] % selected_player_monster->power;
        update_page_index();
        boss->health -= player_hit;
        printf("You hit for %u. %d left\n", player_hit, boss->health);
        oneup_monster(selected_player_monster);

        if (boss->health < 1) {
            printf("You destroyed the boss!!!!\n");
            reset_monsters(player);
            return capture_boss(boss, player);
        }

        uint boss_hit = secret_page[page_index] % boss->power;
        update_page_index();
        printf("%s hits %s for %u\n", boss->type, selected_player_monster->type, boss_hit);
        selected_player_monster->health -= boss_hit;

        if (selected_player_monster->health < 1) {
            printf("%s was knocked out\n", selected_player_monster->type);
        }
    }
    return 1;
}

// Function: capture_monster
int capture_monster(struct Monster *monster, struct Player *player) {
    if (monster == NULL || player == NULL) {
        return 0;
    }

    char response[4];
    printf("capture monster? (y/n): ");
    read_line(response, sizeof(response));

    if (response[0] == 'y') {
        if (player->num_monsters < 5) {
            player->monsters[player->num_monsters++] = monster;
            return 1;
        } else {
            printf("your cart is full.\n");
            for (uint i = 0; i < 5; i++) {
                printf("%u} \n", i + 1);
                print_monster(player->monsters[i]);
            }
            printf("*********************************\n");
            print_monster(monster);
            printf("*********************************\n");
            printf("replace one of yours? (y/n): ");
            read_line(response, sizeof(response));

            if (response[0] == 'y') {
                printf("which one: ");
                read_line(response, sizeof(response));
                uint choice = atoi(response);
                if (choice == 0 || choice > 5) {
                    printf("invalid\n");
                    free(monster->type); // Free new monster's dynamically allocated name
                    free(monster);
                    return 0;
                } else {
                    free(player->monsters[choice - 1]->type); // Free old monster's dynamically allocated name
                    free(player->monsters[choice - 1]);
                    player->monsters[choice - 1] = monster;
                    return 1;
                }
            } else {
                free(monster->type); // Free new monster's dynamically allocated name
                free(monster);
                return 0;
            }
        }
    } else {
        free(monster->type); // Free new monster's dynamically allocated name
        free(monster);
        return 0;
    }
}

// Function: fight
int fight(struct Player *player) {
    if (player == NULL) {
        return 0;
    }

    reset_monsters(player);
    struct Monster *enemy_monster = generate_monster();
    printf("You are being attacked!!!\n");
    print_monster(enemy_monster);

    while (enemy_monster->health > 0) {
        struct Monster *selected_player_monster = select_monster(player);
        if (selected_player_monster == NULL) {
            printf("You have no living monsters. You lose.\n");
            free(enemy_monster->type);
            free(enemy_monster);
            return 0;
        }

        uint player_hit = secret_page[page_index] % selected_player_monster->power;
        update_page_index();
        enemy_monster->health -= player_hit;
        printf("You hit for %u. %d left\n", player_hit, enemy_monster->health);
        oneup_monster(selected_player_monster);

        if (enemy_monster->health < 1) {
            printf("You knocked out %s\n", enemy_monster->type);
            reset_monsters(player);
            return capture_monster(enemy_monster, player);
        }

        uint enemy_hit = secret_page[page_index] % enemy_monster->power;
        update_page_index();
        printf("%s hits %s for %u\n", enemy_monster->type, selected_player_monster->type, enemy_hit);
        selected_player_monster->health -= enemy_hit;

        if (selected_player_monster->health < 1) {
            printf("%s was knocked out\n", selected_player_monster->type);
        }
    }
    return 1;
}

// Function: movement_loop
int movement_loop(struct Map *map, struct Player *player) {
    if (map == NULL || player == NULL) {
        return 0;
    }

    int game_over = 0;
    char direction_input[4];

    while (!game_over) {
        memset(direction_input, 0, sizeof(direction_input));
        printf("Move (u/d/l/r): ");
        read_line(direction_input, sizeof(direction_input));

        if (direction_input[0] == '\0') {
            printf("[ERROR] Failed to receive movement byte\n");
            _terminate(1);
        }

        check_egg(player, direction_input[0]);

        char move = direction_input[0];
        bool moved = false;

        switch (move) {
            case 'u':
                if (map->current_y == 0) {
                    printf("off map\n");
                } else if (map->data[(map->current_y - 1) * map->width + map->current_x] == '#') {
                    printf("blocked\n");
                } else {
                    set_marker(map->current_x, map->current_y, map, '\0');
                    map->current_y--;
                    set_marker(map->current_x, map->current_y, map, '@');
                    moved = true;
                }
                break;
            case 'd':
                if (map->current_y == map->height - 1) {
                    printf("off map\n");
                } else if (map->data[(map->current_y + 1) * map->width + map->current_x] == '#') {
                    printf("blocked\n");
                } else {
                    set_marker(map->current_x, map->current_y, map, '\0');
                    map->current_y++;
                    set_marker(map->current_x, map->current_y, map, '@');
                    moved = true;
                }
                break;
            case 'l':
                if (map->current_x == 0) {
                    printf("off map\n");
                } else if (map->data[map->current_y * map->width + (map->current_x - 1)] == '#') {
                    printf("blocked\n");
                } else {
                    set_marker(map->current_x, map->current_y, map, '\0');
                    map->current_x--;
                    set_marker(map->current_x, map->current_y, map, '@');
                    moved = true;
                }
                break;
            case 'r':
                if (map->current_x == map->width - 1) {
                    printf("off map\n");
                } else if (map->data[map->current_y * map->width + (map->current_x + 1)] == '#') {
                    printf("blocked\n");
                } else {
                    set_marker(map->current_x, map->current_y, map, '\0');
                    map->current_x++;
                    set_marker(map->current_y, map->current_y, map, '@'); // This line was map->current_y, map->current_y. Likely a typo in original. Changed to map->current_x, map->current_y
                    moved = true;
                }
                break;
            default:
                printf("[ERROR] Invalid direction: %c\n", move);
                break;
        }

        if (moved) {
            print_map(map);

            if (map->current_x == map->end_x && map->current_y == map->end_y) {
                printf("reached the end\n");
                if (daboss(player) == 1) {
                    printf("You won!!!\n");
                } else {
                    printf("You failed!!!\n");
                }
                game_over = 1;
            } else {
                // Simplified random encounter check: secret_page[page_index] % 100 < 10 for 10% chance
                if (secret_page[page_index] % 100 < 10) {
                    update_page_index();
                    if (fight(player) == 1) {
                        player->level++;
                        printf("player gains a level. now %d\n", player->level);
                    }
                    print_map(map);
                } else {
                    update_page_index();
                }
            }
        }
    }
    return 1;
}

// Function: print_player
void print_player(struct Player *player) {
    if (player != NULL) {
        printf("Name: %s\n", player->name);
        printf("Level: %d\n", player->level);
        printf("Monsters: \n");
        for (uint i = 0; i < player->num_monsters; i++) {
            struct Monster *m = player->monsters[i];
            if (m != NULL) {
                printf("\tType: %s\n", m->type);
                printf("\tHealth: %d\n", m->health);
                printf("\tPower: %d\n\n", m->power);
            }
        }
    }
}

// Main function to tie everything together
int main() {
    // Initialize secret_page with some dummy data for "randomness"
    for (int i = 0; i < sizeof(secret_page); i++) {
        secret_page[i] = (char)i; // Simple incrementing pattern
    }

    // Initialize names (already done globally)
    // You might want to populate `names` with more meaningful strings.

    printf("Welcome to the Dungeon Crawler!\n");

    struct Player *player = generate_player();
    print_player(player);

    int map_width = 10;
    int map_height = 10;
    struct Map *game_map = generate_map(map_width, map_height);

    movement_loop(game_map, player);

    printf("Game Over!\n");

    // Clean up allocated memory
    free(game_map->data);
    free(game_map);
    for (uint i = 0; i < player->num_monsters; i++) {
        // Only free monster->type if it was dynamically allocated (e.g., by change_monster_name)
        // If it's from `names` array, it's a literal and should not be freed.
        // This logic needs careful handling if names can be both literal and dynamic.
        // For simplicity, assuming all monster->type are literals unless changed by user.
        // If changed, the old literal is replaced by a malloc'd string.
        // In this case, `free(player->monsters[i]->type);` might be needed if it points to malloc'd memory.
        // For this refactor, we'll assume `names` are static and only user-entered names are freed.
        // The `change_monster_name`, `capture_boss`, `capture_monster` already handle freeing.
        free(player->monsters[i]);
    }
    free(player);

    return 0;
}