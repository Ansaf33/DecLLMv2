#include <stdio.h>    // For printf, NULL, perror
#include <stdlib.h>   // For malloc, free, atoi, atof, exit
#include <string.h>   // For bzero (replaced with memset), strcmp
#include <ctype.h>    // For isdigit
#include <unistd.h>   // For read, STDIN_FILENO
#include <stdint.h>   // For uint32_t, uint64_t, uintptr_t

// Define types based on common usage in decompiled code
typedef unsigned char byte;
typedef unsigned int uint;
typedef uint32_t undefined4;
typedef uint64_t undefined8;

// Forward declarations for structs
typedef struct Ingredient Ingredient;
typedef struct Instruction Instruction;
typedef struct Recipe Recipe;
typedef struct Pantry Pantry;

// Structs with explicit 32-bit pointer storage as implied by original offsets/sizes.
// This design choice is made to faithfully replicate the memory layout implied
// by the original decompiled code's fixed offsets (e.g., 0x1c for Recipe, 0x18 for Pantry),
// which likely assume 32-bit pointers even on a 64-bit system.
// On a 64-bit system, this requires casting between actual pointers and uint32_t storage.

// Ingredient struct (Size: 8+8+4+20+4 = 44 bytes = 0x2C)
struct Ingredient {
    double amount;
    double cost_per_unit;
    undefined4 type; // 1 for oz, 2 for gram
    char name[20]; // Original read_string uses 0xf max, but 0x14 offset for name and 0x28 for next implies name[20]
    uint32_t next_ptr_storage; // Store pointer as uint32_t
};

// Instruction struct (Size: 0x80+4+4 = 0x88)
struct Instruction {
    char text[0x80]; // Max 0x7f + null
    undefined4 cookie; // At 0x80
    uint32_t next_ptr_storage; // At 0x84
};

// Recipe struct (Size: 16+4+4+4 = 28 bytes = 0x1C)
// Assumes 32-bit pointers for ingredients_head and instructions_head
struct Recipe {
    char name[16];
    undefined4 cookie; // At 0x10
    uint32_t ingredients_head_ptr_storage; // At 0x14
    uint32_t instructions_head_ptr_storage; // At 0x18
};

// Pantry struct (Size: 16+4+4 = 24 bytes = 0x18)
// Assumes 32-bit pointer for ingredients_head
struct Pantry {
    char name[16];
    undefined4 cookie; // At 0x10
    uint32_t ingredients_head_ptr_storage; // At 0x14
};


// Global data buffer.
// Pantry data: 0x80 pantries * 0x18 bytes/pantry = 0xC00 bytes.
// Recipe data: 0x200 recipes * 0x1C bytes/recipe = 0x3800 bytes.
// Total needed: 0xC00 (for pantries) + 0x3800 (for recipes) = 0x4400 bytes.
char g[0x4400];

uint magic_index_g;
byte *magic_page_g; // Base address for magic values

// Placeholder for receive function, assuming it reads from stdin.
// Returns number of bytes read, or -1 on error.
int receive(char *buffer, int max_len) {
    ssize_t bytes_read = read(STDIN_FILENO, buffer, max_len);
    if (bytes_read == -1) {
        perror("read");
        return -1; // Error
    }
    return (int)bytes_read; // Return actual bytes read
}

// Placeholder for _terminate function
void _terminate(void) {
    exit(1);
}

// Function: read_selection
int read_selection(void) {
    char selection_buf[2];
    int total_bytes_read = 0;
    int remaining_bytes = 2;
    char *current_buf_ptr = selection_buf;

    while (remaining_bytes > 0) {
        int res = receive(current_buf_ptr, remaining_bytes);
        if (res == -1) { // Error
            printf("[ERROR] Failed to read data\n");
            _terminate();
        }
        if (res == 0) { // EOF or no data read
            printf("[ERROR] Error with reading selection (EOF/no data)\n");
            _terminate();
        }
        total_bytes_read += res;
        current_buf_ptr += res;
        remaining_bytes -= res;
    }

    if (!isdigit(selection_buf[0])) {
        printf("[ERROR] Selection must be a digit\n");
        return 0;
    } else if (selection_buf[1] == '\n') {
        selection_buf[1] = '\0'; // Null-terminate the string for atoi
        return atoi(selection_buf);
    } else {
        printf("[ERROR] Only single digits allowed in the selection\n");
        return 0;
    }
}

// Function: read_string
uint read_string(char *buffer, uint max_len) {
    uint i;
    char current_char;

    if (buffer == NULL) {
        return 0;
    }

    for (i = 0; i < max_len; i++) {
        int bytes_read = receive(&current_char, 1); // Read one character

        if (bytes_read == -1) { // Error
            printf("[ERROR] Failed to read string\n");
            _terminate();
        }
        if (bytes_read == 0) { // EOF or no data read
            printf("[ERROR] String read failure (EOF/no data)\n");
            _terminate();
        }
        
        if (current_char == '\n') {
            buffer[i] = '\0'; // Null-terminate the string
            return i; // Return number of characters read (excluding newline)
        }
        buffer[i] = current_char;
    }
    // If max_len characters are read without a newline, null-terminate
    if (max_len > 0) {
        buffer[max_len] = '\0'; // Null-terminate at max_len
    }
    return max_len;
}

// Function: read_cookie
undefined4 read_cookie(void) {
    uint uVar1_aligned = magic_index_g & 0xfffffffc; // Align to 4 bytes
    magic_index_g += 4;
    return *(undefined4 *)(magic_page_g + uVar1_aligned);
}

// Function: display_banner
void display_banner(void) {
    unsigned char day_of_week_val = magic_page_g[0x32];
    unsigned char month_val = magic_page_g[0];
    unsigned char hour_val = magic_page_g[0x14];
    unsigned char minute_val = magic_page_g[0x1e];
    unsigned char second_val = magic_page_g[0x28];

    printf("Today: ");
    switch (day_of_week_val % 7) {
        case 0: printf("Sun "); break;
        case 1: printf("Mon "); break;
        case 2: printf("Tue "); break;
        case 3: printf("Wed "); break;
        case 4: printf("Thu "); break;
        case 5: printf("Fri "); break;
        case 6: printf("Sat "); break;
    }
    switch (month_val % 12) {
        case 0: printf("Jan "); break;
        case 1: printf("Feb "); break;
        case 2: printf("Mar "); break;
        case 3: printf("Apr "); break;
        case 4: printf("May "); break;
        case 5: printf("Jun "); break;
        case 6: printf("Jul "); break;
        case 7: printf("Aug "); break;
        case 8: printf("Sep "); break;
        case 9: printf("Oct "); break;
        case 10: printf("Nov "); break;
        case 11: printf("Dec "); break;
    }
    printf("%.2u:%.2u:%.2u %d\n\n",
           hour_val % 24,
           minute_val % 60,
           second_val % 60,
           2016); // 0x7e0 is 2016
}

// Function: instruction_prompt
Instruction *instruction_prompt(void) {
    Instruction *instruction_obj = (Instruction *)malloc(sizeof(Instruction));
    if (instruction_obj != NULL) {
        memset(instruction_obj, 0, sizeof(Instruction));
        printf("Enter Instruction: ");
        uint bytes_read = read_string(instruction_obj->text, 0x7f); // 0x7f max chars
        if (bytes_read == 0) {
            free(instruction_obj);
            instruction_obj = NULL;
        } else {
            if (strcmp(instruction_obj->text, "done") == 0) {
                free(instruction_obj);
                instruction_obj = NULL;
            } else {
                instruction_obj->cookie = read_cookie();
            }
        }
    }
    return instruction_obj;
}

// Function: recipe_menu
void recipe_menu(void) {
  printf("1) Add Recipe\n");
  printf("2) Remove Recipe\n");
  printf("3) List Recipe\n");
  printf("4) List All\n");
  printf("5) Print Name\n");
  printf("6) Recipe Costs\n");
  printf("7) Leave Menu\n");
  printf("-> ");
}

// Function: ingredient_prompt
Ingredient *ingredient_prompt(void) {
    Ingredient *ingredient = (Ingredient *)malloc(sizeof(Ingredient));
    if (ingredient != NULL) {
        memset(ingredient, 0, sizeof(Ingredient));
        printf("Enter ingredient name or a \"done\" to quit.\n");
        printf(": ");
        uint bytes_read = read_string(ingredient->name, 0xf); // 0xf max chars
        if (bytes_read == 0) {
            printf("[ERROR] Invalid ingredient name\n");
            free(ingredient);
            ingredient = NULL;
        } else if (strcmp(ingredient->name, "done") == 0) {
            free(ingredient);
            ingredient = NULL;
        } else {
            char amount_buf[6]; // Max 5 chars + null
            memset(amount_buf, 0, sizeof(amount_buf));
            printf("Enter the amount: ");
            bytes_read = read_string(amount_buf, 5);
            if (bytes_read == 0) {
                printf("[ERROR] Invalid amount\n");
                free(ingredient);
                ingredient = NULL;
            } else {
                ingredient->amount = atof(amount_buf);

                char cost_buf[6];
                memset(cost_buf, 0, sizeof(cost_buf));
                printf("Enter the cost per unit: ");
                bytes_read = read_string(cost_buf, 5);
                if (bytes_read == 0) {
                    printf("[ERROR] Invalid cost\n");
                    free(ingredient);
                    ingredient = NULL;
                } else {
                    ingredient->cost_per_unit = atof(cost_buf);

                    char type_buf[6];
                    memset(type_buf, 0, sizeof(type_buf));
                    printf("Enter the type (oz/gram): ");
                    bytes_read = read_string(type_buf, 5);
                    if (bytes_read == 0) {
                        printf("[ERROR] Failed to read the measurement type.\n");
                        free(ingredient);
                        ingredient = NULL;
                    } else {
                        if (strcmp(type_buf, "oz") == 0) {
                            ingredient->type = 1;
                        } else if (strcmp(type_buf, "gram") == 0) {
                            ingredient->type = 2;
                        } else {
                            printf("[ERROR] Invalid type\n");
                            free(ingredient);
                            ingredient = NULL;
                        }
                    }
                }
            }
        }
    }
    return ingredient;
}

// Function: link_recipe_ingredient
Recipe *link_recipe_ingredient(Recipe *recipe, Ingredient *ingredient) {
    if (ingredient != NULL && recipe != NULL) {
        ingredient->next_ptr_storage = recipe->ingredients_head_ptr_storage;
        recipe->ingredients_head_ptr_storage = (uint32_t)(uintptr_t)ingredient;
    }
    return recipe;
}

// Function: check_ingredient_exists
undefined4 check_ingredient_exists(uint32_t head_ptr_storage, Ingredient *new_ingredient) {
    if (head_ptr_storage != 0 && new_ingredient != NULL) {
        Ingredient *current = (Ingredient*)(uintptr_t)head_ptr_storage;
        while (current != NULL) {
            if (strcmp(current->name, new_ingredient->name) == 0) {
                printf("Ingredient already exists. Adding the additional amount.\n");
                current->amount += new_ingredient->amount;
                return 1; // Found and updated
            }
            current = (Ingredient*)(uintptr_t)current->next_ptr_storage;
        }
    }
    return 0; // Not found
}

// Function: link_recipe_instruction
Recipe *link_recipe_instruction(Recipe *recipe, Instruction *instruction) {
    if (instruction != NULL && recipe != NULL) {
        instruction->next_ptr_storage = 0; // Ensure new instruction is the last one (NULL)

        if (recipe->instructions_head_ptr_storage == 0) { // If head is NULL
            recipe->instructions_head_ptr_storage = (uint32_t)(uintptr_t)instruction;
        } else {
            Instruction *current = (Instruction*)(uintptr_t)recipe->instructions_head_ptr_storage;
            while (current->next_ptr_storage != 0) {
                current = (Instruction*)(uintptr_t)current->next_ptr_storage;
            }
            current->next_ptr_storage = (uint32_t)(uintptr_t)instruction;
        }
    }
    return recipe;
}

// Function to free an ingredient list
void free_ingredient_list(uint32_t head_ptr_storage) {
    Ingredient *current = (Ingredient*)(uintptr_t)head_ptr_storage;
    while (current != NULL) {
        Ingredient *next_ingredient = (Ingredient*)(uintptr_t)current->next_ptr_storage;
        free(current);
        current = next_ingredient;
    }
}

// Function to free an instruction list
void free_instruction_list(uint32_t head_ptr_storage) {
    Instruction *current = (Instruction*)(uintptr_t)head_ptr_storage;
    while (current != NULL) {
        Instruction *next_instruction = (Instruction*)(uintptr_t)current->next_ptr_storage;
        free(current);
        current = next_instruction;
    }
}

// Function: add_recipe
void add_recipe(void) {
    int recipe_idx = -1;
    // Find an empty slot for a new recipe (cookie == 0 means empty)
    for (int i = 0; i < 0x200; i++) { // 0x200 recipes
        Recipe *current_recipe_slot = (Recipe *)(g + i * sizeof(Recipe) + 0xc00);
        if (current_recipe_slot->cookie == 0) { // Check if slot is empty
            recipe_idx = i;
            break;
        }
    }

    if (recipe_idx == -1) { // No empty slot found
        printf("[ERROR] Recipe List is full\n");
        return;
    }

    Recipe *new_recipe_slot = (Recipe *)(g + recipe_idx * sizeof(Recipe) + 0xc00);
    memset(new_recipe_slot, 0, sizeof(Recipe)); // Clear the slot

    printf("Enter recipe name: ");
    uint bytes_read = read_string(new_recipe_slot->name, 0xf); // Max 0xf chars for name
    if (bytes_read == 0) {
        printf("[ERROR] Invalid recipe name\n");
        memset(new_recipe_slot->name, 0, sizeof(new_recipe_slot->name)); // Clear name
        return;
    }

    // Check for duplicate recipe names
    for (int i = 0; i < 0x200; i++) {
        Recipe *other_recipe_slot = (Recipe *)(g + i * sizeof(Recipe) + 0xc00);
        if (i != recipe_idx && other_recipe_slot->cookie != 0 &&
            strcmp(other_recipe_slot->name, new_recipe_slot->name) == 0) {
            printf("[ERROR] Recipe already exists with that name.\n");
            memset(new_recipe_slot->name, 0, sizeof(new_recipe_slot->name)); // Clear name
            return;
        }
    }

    new_recipe_slot->cookie = read_cookie();

    // Add ingredients
    Ingredient *new_ingredient;
    do {
        new_ingredient = ingredient_prompt();
        if (new_ingredient != NULL) {
            if (check_ingredient_exists(new_recipe_slot->ingredients_head_ptr_storage, new_ingredient) == 0) {
                link_recipe_ingredient(new_recipe_slot, new_ingredient);
            } else {
                free(new_ingredient); // Duplicate, free the new ingredient
            }
        }
    } while (new_ingredient != NULL);

    // Add instructions
    Instruction *new_instruction;
    do {
        new_instruction = instruction_prompt();
        link_recipe_instruction(new_recipe_slot, new_instruction);
    } while (new_instruction != NULL);
}

// Function: print_recipe
void print_recipe(int recipe_idx) {
    if (recipe_idx < 0 || recipe_idx >= 0x200) {
        printf("[ERROR] Invalid index: %d\n", recipe_idx);
        return;
    }

    Recipe *recipe = (Recipe *)(g + recipe_idx * sizeof(Recipe) + 0xc00);

    if (recipe->cookie == 0) { // Check if slot is empty
        printf("[ERROR] Empty recipe: %d\n", recipe_idx + 1);
        return;
    }

    printf("Name: %s\n", recipe->name);
    printf("Ingredients:\n");
    Ingredient *current_ingredient = (Ingredient*)(uintptr_t)recipe->ingredients_head_ptr_storage;
    while (current_ingredient != NULL) {
        printf("\t%f ", current_ingredient->amount);
        if (current_ingredient->type == 1) { // oz
            printf("oz\t");
        } else { // gram
            printf("g\t");
        }
        printf("%s\n", current_ingredient->name);
        current_ingredient = (Ingredient*)(uintptr_t)current_ingredient->next_ptr_storage;
    }

    printf("\nInstructions:\n");
    Instruction *current_instruction = (Instruction*)(uintptr_t)recipe->instructions_head_ptr_storage;
    int instruction_num = 1;
    while (current_instruction != NULL) {
        printf("\t%d) %s\n", instruction_num, current_instruction->text);
        instruction_num++;
        current_instruction = (Instruction*)(uintptr_t)current_instruction->next_ptr_storage;
    }
    printf("\n\n");
}

// Function: list_recipe
void list_recipe(void) {
    char index_buf[5]; // Max 4 chars + null
    memset(index_buf, 0, sizeof(index_buf));
    printf("Recipe Index [1..512]: ");
    uint bytes_read = read_string(index_buf, 4);
    if (bytes_read != 0) {
        int recipe_idx = atoi(index_buf);
        if (recipe_idx < 1 || recipe_idx > 0x200) { // 0x200 is 512
            printf("[ERROR] Must be between 1 and 512\n");
        } else {
            print_recipe(recipe_idx - 1); // Adjust to 0-based index
        }
    }
}

// Function: list_all
void list_all(void) {
    for (int i = 0; i < 0x200; i++) {
        Recipe *recipe = (Recipe *)(g + i * sizeof(Recipe) + 0xc00);
        if (recipe->cookie != 0) { // If recipe slot is not empty
            printf("Index: %d\n", i + 1);
            print_recipe(i);
        }
    }
}

// Function: remove_recipe
void remove_recipe(void) {
    char index_buf[4]; // Max 3 chars + null
    memset(index_buf, 0, sizeof(index_buf));
    list_all(); // Show available recipes
    printf("Recipe Index [1..512]: ");
    uint bytes_read = read_string(index_buf, 3);
    if (bytes_read != 0) {
        int recipe_idx = atoi(index_buf);
        if (recipe_idx < 1 || recipe_idx > 0x200) {
            printf("[ERROR] Must be between 1 and 512\n");
        } else {
            Recipe *recipe_to_remove = (Recipe *)(g + (recipe_idx - 1) * sizeof(Recipe) + 0xc00);
            if (recipe_to_remove->cookie == 0) { // Check if slot is empty
                printf("[ERROR] Empty recipe\n");
            } else {
                // Free associated ingredients and instructions
                free_ingredient_list(recipe_to_remove->ingredients_head_ptr_storage);
                free_instruction_list(recipe_to_remove->instructions_head_ptr_storage);
                // Clear the recipe slot (effectively deleting it)
                memset(recipe_to_remove, 0, sizeof(Recipe));
                printf("Recipe %d removed.\n", recipe_idx);
            }
        }
    }
}

// Function: print_recipe_costs
void print_recipe_costs(void) {
    printf("Recipe Costs:\n");
    for (int i = 0; i < 0x200; i++) {
        Recipe *recipe = (Recipe *)(g + i * sizeof(Recipe) + 0xc00);
        if (recipe->cookie != 0) { // If recipe slot is not empty
            double total_cost = 0.0;
            Ingredient *current_ingredient = (Ingredient*)(uintptr_t)recipe->ingredients_head_ptr_storage;
            while (current_ingredient != NULL) {
                total_cost += current_ingredient->amount * current_ingredient->cost_per_unit;
                current_ingredient = (Ingredient*)(uintptr_t)current_ingredient->next_ptr_storage;
            }
            printf("\t%d) %s - %.5f\n", i + 1, recipe->name, total_cost);
        }
    }
}

// Function: print_recipe_name
void print_recipe_name(void) {
    char index_buf[5]; // Max 4 chars + null
    memset(index_buf, 0, sizeof(index_buf));
    printf("Recipe Index [1..512]: ");
    uint bytes_read = read_string(index_buf, 4);
    if (bytes_read != 0) {
        int recipe_idx = atoi(index_buf);
        if (recipe_idx < 1 || recipe_idx > 0x200) { // 0x200 is 512
            printf("[ERROR] Invalid index: %d\n", recipe_idx);
        } else {
            Recipe *recipe = (Recipe *)(g + (recipe_idx - 1) * sizeof(Recipe) + 0xc00);
            if (recipe->cookie == 0) { // Check if slot is empty
                printf("[ERROR] Empty recipe: %d\n", recipe_idx);
            } else {
                printf("Single Name: %s\n", recipe->name);
            }
        }
    }
}

// Function: handle_recipe
void handle_recipe(void) {
    int selection;
    do {
        recipe_menu();
        selection = read_selection();
        switch (selection) {
            default:
                printf("Invalid selection.\n");
                break;
            case 1:
                add_recipe();
                break;
            case 2:
                remove_recipe();
                break;
            case 3:
                list_recipe();
                break;
            case 4:
                list_all();
                break;
            case 5:
                print_recipe_name();
                break;
            case 6:
                print_recipe_costs();
                break;
            case 7:
                printf("Leave\n");
                return;
        }
    } while (1);
}

// Function: pantry_menu
void pantry_menu(void) {
  printf("1) Create Pantry\n");
  printf("2) Delete Pantry\n");
  printf("3) Print Pantry\n");
  printf("4) Print All Pantries\n");
  printf("5) Update Pantry\n");
  printf("6) Leave Menu\n");
  printf("-> ");
}

// Function: init_globals
void init_globals(undefined4 magic_page_addr) {
    // Clear pantry data area (0x80 pantries * 0x18 bytes/pantry = 0xC00 bytes)
    memset(g, 0, 0xc00);
    // Clear recipe data area (0x200 recipes * 0x1C bytes/recipe = 0x3800 bytes)
    memset(g + 0xc00, 0, 0x3800);
    magic_index_g = 0;
    magic_page_g = (byte *)(uintptr_t)magic_page_addr; // Cast undefined4 to byte*
}

// Function: link_pantry_ingredient
Pantry *link_pantry_ingredient(Pantry *pantry, Ingredient *ingredient) {
    if (ingredient != NULL && pantry != NULL) {
        ingredient->next_ptr_storage = pantry->ingredients_head_ptr_storage;
        pantry->ingredients_head_ptr_storage = (uint32_t)(uintptr_t)ingredient;
    }
    return pantry;
}

// Function: print_single_pantry
void print_single_pantry(uint pantry_idx) {
    if (pantry_idx >= 0x80) { // 0x80 is 128
        printf("[ERROR] Invalid index: %u\n", pantry_idx);
        return;
    }

    Pantry *pantry = (Pantry *)(g + pantry_idx * sizeof(Pantry));

    if (pantry->cookie == 0) { // Check if slot is empty
        printf("[ERROR] Empty pantry: %u\n", pantry_idx + 1);
        return;
    }

    printf("Name: %s\n", pantry->name);
    printf("Item:\n");
    Ingredient *current_ingredient = (Ingredient*)(uintptr_t)pantry->ingredients_head_ptr_storage;
    while (current_ingredient != NULL) {
        printf("\t%f ", current_ingredient->amount);
        if (current_ingredient->type == 1) { // oz
            printf("oz\t");
        } else { // gram
            printf("g\t");
        }
        printf("%s\n", current_ingredient->name);
        current_ingredient = (Ingredient*)(uintptr_t)current_ingredient->next_ptr_storage;
    }
    printf("\n\n");
}

// Function: print_pantry
void print_pantry(void) {
    char index_buf[4]; // Max 3 chars + null
    memset(index_buf, 0, sizeof(index_buf));
    printf("Pantry Index [1..128]: ");
    uint bytes_read = read_string(index_buf, 3);
    if (bytes_read != 0) {
        int pantry_idx = atoi(index_buf);
        if (pantry_idx < 1 || pantry_idx > 0x80) { // 0x80 is 128
            printf("[ERROR] Must be between 1 and 128\n");
        } else {
            print_single_pantry(pantry_idx - 1); // Adjust to 0-based index
        }
    }
}

// Function: create_pantry
void create_pantry(void) {
    int pantry_idx = -1;
    // Find an empty slot for a new pantry (cookie == 0 means empty)
    for (int i = 0; i < 0x80; i++) { // 0x80 pantries
        Pantry *current_pantry_slot = (Pantry *)(g + i * sizeof(Pantry));
        if (current_pantry_slot->cookie == 0) { // Check if slot is empty
            pantry_idx = i;
            break;
        }
    }

    if (pantry_idx == -1) { // No empty slot found
        printf("[ERROR] Pantry List is full\n");
        return;
    }

    Pantry *new_pantry_slot = (Pantry *)(g + pantry_idx * sizeof(Pantry));
    memset(new_pantry_slot, 0, sizeof(Pantry)); // Clear the slot

    printf("Enter pantry name: ");
    uint bytes_read = read_string(new_pantry_slot->name, 0xf); // Max 0xf chars for name
    if (bytes_read == 0) {
        printf("[ERROR] Invalid pantry name\n");
        memset(new_pantry_slot->name, 0, sizeof(new_pantry_slot->name)); // Clear name
        return;
    }

    // Check for duplicate pantry names
    for (int i = 0; i < 0x80; i++) {
        Pantry *other_pantry_slot = (Pantry *)(g + i * sizeof(Pantry));
        if (i != pantry_idx && other_pantry_slot->cookie != 0 &&
            strcmp(other_pantry_slot->name, new_pantry_slot->name) == 0) {
            printf("[ERROR] Pantry already exists with that name.\n");
            memset(new_pantry_slot->name, 0, sizeof(new_pantry_slot->name)); // Clear name
            return;
        }
    }

    new_pantry_slot->cookie = read_cookie();

    // Add ingredients
    Ingredient *new_ingredient;
    do {
        new_ingredient = ingredient_prompt();
        if (new_ingredient != NULL) {
            if (check_ingredient_exists(new_pantry_slot->ingredients_head_ptr_storage, new_ingredient) == 0) {
                link_pantry_ingredient(new_pantry_slot, new_ingredient);
            } else {
                free(new_ingredient); // Duplicate, free the new ingredient
            }
        }
    } while (new_ingredient != NULL);
}

// Function: list_all_pantries
void list_all_pantries(void) {
    for (int i = 0; i < 0x80; i++) {
        Pantry *pantry = (Pantry *)(g + i * sizeof(Pantry));
        if (pantry->cookie != 0) { // If pantry slot is not empty
            printf("Index: %d\n", i + 1);
            print_single_pantry(i);
        }
    }
}

// Function: delete_pantry
void delete_pantry(void) {
    char index_buf[4]; // Max 3 chars + null
    memset(index_buf, 0, sizeof(index_buf));
    list_all_pantries(); // Show available pantries
    printf("Pantry Index [1..128]: ");
    uint bytes_read = read_string(index_buf, 3);
    if (bytes_read != 0) {
        int pantry_idx = atoi(index_buf);
        if (pantry_idx < 1 || pantry_idx > 0x80) {
            printf("[ERROR] Must be between 1 and 128\n");
        } else {
            Pantry *pantry_to_remove = (Pantry *)(g + (pantry_idx - 1) * sizeof(Pantry));
            if (pantry_to_remove->cookie == 0) { // Check if slot is empty
                printf("[ERROR] Empty pantry\n");
            } else {
                // Free associated ingredients
                free_ingredient_list(pantry_to_remove->ingredients_head_ptr_storage);
                // Clear the pantry slot (effectively deleting it)
                memset(pantry_to_remove, 0, sizeof(Pantry));
                printf("Pantry %d removed.\n", pantry_idx);
            }
        }
    }
}

// Function: update_pantry
void update_pantry(void) {
    char index_buf[4]; // Max 3 chars + null
    memset(index_buf, 0, sizeof(index_buf));
    list_all_pantries();
    printf("Pantry Index [1..128]: ");
    uint bytes_read = read_string(index_buf, 3);
    if (bytes_read == 0) {
        return; // No input
    }

    int pantry_idx = atoi(index_buf);
    if (pantry_idx < 1 || pantry_idx > 0x80) {
        printf("[ERROR] Must be between 1 and 128\n");
        return;
    }

    Pantry *pantry = (Pantry *)(g + (pantry_idx - 1) * sizeof(Pantry));
    if (pantry->cookie == 0) {
        printf("[ERROR] Empty pantry\n");
        return;
    }

    print_single_pantry(pantry_idx - 1); // Show current pantry items

    char item_name_buf[20]; // Max 0xf chars + null, using 20 as per Ingredient struct name size
    memset(item_name_buf, 0, sizeof(item_name_buf));
    printf("Which item to update: ");
    bytes_read = read_string(item_name_buf, 0xf);
    if (bytes_read == 0) {
        printf("[ERROR] Invalid item name\n");
        return;
    }

    Ingredient *found_ingredient = NULL;
    Ingredient *current_ingredient = (Ingredient*)(uintptr_t)pantry->ingredients_head_ptr_storage;
    while (current_ingredient != NULL) {
        if (strcmp(current_ingredient->name, item_name_buf) == 0) {
            found_ingredient = current_ingredient;
            break;
        }
        current_ingredient = (Ingredient*)(uintptr_t)current_ingredient->next_ptr_storage;
    }

    if (found_ingredient == NULL) {
        printf("This pantry does not have %s\n", item_name_buf);
    } else {
        char amount_buf[6]; // Max 5 chars + null
        memset(amount_buf, 0, sizeof(amount_buf));
        printf("Enter the amount: ");
        bytes_read = read_string(amount_buf, 5);
        if (bytes_read == 0) {
            printf("[ERROR] Invalid amount\n");
        } else {
            found_ingredient->amount = atof(amount_buf);

            char type_buf[6];
            memset(type_buf, 0, sizeof(type_buf));
            printf("Enter the type (oz/gram): ");
            bytes_read = read_string(type_buf, 5);
            if (bytes_read == 0) {
                printf("[ERROR] Failed to read the measurement type.\n");
            } else {
                if (strcmp(type_buf, "oz") == 0) {
                    found_ingredient->type = 1;
                } else if (strcmp(type_buf, "gram") == 0) {
                    found_ingredient->type = 2;
                } else {
                    printf("[ERROR] Invalid type\n");
                }
            }
        }
    }
}

// Function: handle_pantry
void handle_pantry(void) {
    int selection;
    do {
        pantry_menu();
        selection = read_selection();
        switch (selection) {
            default:
                printf("Invalid selection\n");
                break;
            case 1:
                create_pantry();
                break;
            case 2:
                delete_pantry();
                break;
            case 3:
                print_pantry();
                break;
            case 4:
                list_all_pantries();
                break;
            case 5:
                update_pantry();
                break;
            case 6:
                printf("Leave.\n");
                return;
        }
    } while (1);
}

// Function: query_recipes
void query_recipes(void) {
    printf("Select a pantry:\n");
    for (int i = 0; i < 0x80; i++) {
        Pantry *pantry = (Pantry *)(g + i * sizeof(Pantry));
        if (pantry->cookie != 0) {
            printf("%d) %s\n", i + 1, pantry->name);
        }
    }
    printf("-> ");

    char index_buf[4]; // Max 3 chars + null
    memset(index_buf, 0, sizeof(index_buf));
    uint bytes_read = read_string(index_buf, 3);
    if (bytes_read == 0) {
        return; // No input
    }

    int selected_pantry_idx = atoi(index_buf);
    if (selected_pantry_idx < 1 || selected_pantry_idx > 0x80) {
        printf("[ERROR] Invalid selection\n");
        return;
    }

    Pantry *selected_pantry = (Pantry *)(g + (selected_pantry_idx - 1) * sizeof(Pantry));
    if (selected_pantry->cookie == 0) {
        printf("[ERROR] Empty pantry\n");
        return;
    }

    printf("You can make:\n");
    for (int recipe_i = 0; recipe_i < 0x200; recipe_i++) {
        Recipe *recipe = (Recipe *)(g + recipe_i * sizeof(Recipe) + 0xc00);
        if (recipe->cookie != 0) { // If recipe slot is not empty
            int can_make_recipe = 1; // Assume we can make it initially
            Ingredient *recipe_ingredient = (Ingredient*)(uintptr_t)recipe->ingredients_head_ptr_storage;

            while (recipe_ingredient != NULL) {
                int ingredient_found_and_sufficient = 0;
                Ingredient *pantry_ingredient = (Ingredient*)(uintptr_t)selected_pantry->ingredients_head_ptr_storage;

                while (pantry_ingredient != NULL) {
                    if (strcmp(pantry_ingredient->name, recipe_ingredient->name) == 0 &&
                        pantry_ingredient->amount >= recipe_ingredient->amount &&
                        pantry_ingredient->type == recipe_ingredient->type) { // Also check type
                        ingredient_found_and_sufficient = 1;
                        break;
                    }
                    pantry_ingredient = (Ingredient*)(uintptr_t)pantry_ingredient->next_ptr_storage;
                }

                if (ingredient_found_and_sufficient == 0) {
                    can_make_recipe = 0; // Missing or insufficient ingredient
                    break;
                }
                recipe_ingredient = (Ingredient*)(uintptr_t)recipe_ingredient->next_ptr_storage;
            }

            if (can_make_recipe) {
                printf("%s\n", recipe->name);
            }
        }
    }
}

// Function: query_by_price
void query_by_price(void) {
    char price_buf[10]; // Max 9 chars + null
    memset(price_buf, 0, sizeof(price_buf));
    printf("How much can you spend: ");
    uint bytes_read = read_string(price_buf, 9);
    if (bytes_read == 0) {
        return; // No input
    }

    double max_spending = atof(price_buf);
    printf("You can make: \n");

    for (int i = 0; i < 0x200; i++) {
        Recipe *recipe = (Recipe *)(g + i * sizeof(Recipe) + 0xc00);
        if (recipe->cookie != 0) { // If recipe slot is not empty
            double total_cost = 0.0;
            Ingredient *current_ingredient = (Ingredient*)(uintptr_t)recipe->ingredients_head_ptr_storage;
            while (current_ingredient != NULL) {
                total_cost += current_ingredient->amount * current_ingredient->cost_per_unit;
                current_ingredient = (Ingredient*)(uintptr_t)current_ingredient->next_ptr_storage;
            }

            if (total_cost <= max_spending) {
                printf("\t%d) %s\n", i + 1, recipe->name);
            }
        }
    }
}

// Function: print_cheapest
void print_cheapest(void) {
    int cheapest_recipe_idx = 0;
    double min_cost = -1.0; // Use -1.0 to indicate not found yet

    for (int i = 0; i < 0x200; i++) {
        Recipe *recipe = (Recipe *)(g + i * sizeof(Recipe) + 0xc00);
        if (recipe->cookie != 0) { // If recipe slot is not empty
            double current_recipe_cost = 0.0;
            Ingredient *current_ingredient = (Ingredient*)(uintptr_t)recipe->ingredients_head_ptr_storage;
            while (current_ingredient != NULL) {
                current_recipe_cost += current_ingredient->amount * current_ingredient->cost_per_unit;
                current_ingredient = (Ingredient*)(uintptr_t)current_ingredient->next_ptr_storage;
            }

            if (min_cost == -1.0 || current_recipe_cost < min_cost) {
                min_cost = current_recipe_cost;
                cheapest_recipe_idx = i + 1; // Store 1-based index
            }
        }
    }

    if (cheapest_recipe_idx == 0) { // No recipes found
        printf("No cheapest recipe found\n");
    } else {
        printf("Cheapest: %d at %f\n", cheapest_recipe_idx, min_cost);
    }
}

// Function: print_expensivest
void print_expensivest(void) {
    int expensive_recipe_idx = 0;
    double max_cost = -1.0; // Use -1.0 to indicate not found yet

    for (int i = 0; i < 0x200; i++) {
        Recipe *recipe = (Recipe *)(g + i * sizeof(Recipe) + 0xc00);
        if (recipe->cookie != 0) { // If recipe slot is not empty
            double current_recipe_cost = 0.0;
            Ingredient *current_ingredient = (Ingredient*)(uintptr_t)recipe->ingredients_head_ptr_storage;
            while (current_ingredient != NULL) {
                current_recipe_cost += current_ingredient->amount * current_ingredient->cost_per_unit;
                current_ingredient = (Ingredient*)(uintptr_t)current_ingredient->next_ptr_storage;
            }

            if (current_recipe_cost > max_cost) {
                max_cost = current_recipe_cost;
                expensive_recipe_idx = i + 1; // Store 1-based index
            }
        }
    }

    if (expensive_recipe_idx == 0) { // No recipes found
        printf("The most expensive recipe was not found\n");
    } else {
        printf("Most expensive: %d at %f\n", expensive_recipe_idx, max_cost);
    }
}

// Function: query_menu
void query_menu(void) {
    int selection;
    do {
        printf("1) List Doable Recipes by Pantry\n");
        printf("2) List Doable Recipes by Cost\n");
        printf("3) Print Cheapest Recipe\n");
        printf("4) Print Most Expensive Recipe\n");
        printf("5) Leave Menu\n");
        printf("-> ");
        selection = read_selection();
        switch (selection) {
            default:
                printf("[ERROR] Invalid selection\n");
                break;
            case 1:
                query_recipes();
                break;
            case 2:
                query_by_price();
                break;
            case 3:
                print_cheapest();
                break;
            case 4:
                print_expensivest();
                break;
            case 5:
                printf("Leaving..\n");
                return;
        }
    } while (1);
}

// Main menu function
void main_menu(void) {
    printf("1) Recipe Menu\n");
    printf("2) Pantry Menu\n");
    printf("3) Query Menu\n");
    printf("4) Display Banner\n");
    printf("5) Exit\n");
    printf("-> ");
}

// Main function
int main(void) {
    // Initialize globals. magic_page_g needs an address.
    // For a simple compilable example, let's use a dummy magic page.
    byte dummy_magic_page[0x33]; // Needs to be at least 0x32+1 = 51 bytes for indices used.
    // Fill with some dummy data for display_banner to work
    for (int i = 0; i < sizeof(dummy_magic_page); i++) {
        dummy_magic_page[i] = i; // Example: 0, 1, 2, ..., 50
    }
    init_globals((undefined4)(uintptr_t)dummy_magic_page);

    int selection;
    do {
        main_menu();
        selection = read_selection();
        switch (selection) {
            default:
                printf("Invalid selection.\n");
                break;
            case 1:
                handle_recipe();
                break;
            case 2:
                handle_pantry();
                break;
            case 3:
                query_menu();
                break;
            case 4:
                display_banner();
                break;
            case 5:
                printf("Exiting...\n");
                return 0;
        }
    } while (1);

    return 0;
}