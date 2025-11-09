#include <stdlib.h> // For calloc, free, exit
#include <string.h> // For strlen, strncpy
#include <stdio.h>  // For printf

// Define custom exit function for _terminate
#define _terminate() exit(EXIT_FAILURE)

// Struct definitions based on memory access patterns
typedef unsigned short ushort;

// This struct is used for both Product Backlog Items and Sprint Backlog Items
struct BacklogItem {
    ushort id;
    ushort quantity;
    ushort status; // Used for SBI, not for product backlog, but struct is shared
    char* description;
    struct BacklogItem* next;
}; // Expected size: 16 bytes (0x10)

struct Sprint {
    ushort id;
    char* description;
    void* unknown_field_8;  // Placeholder for 4 bytes, based on offsets
    void* unknown_field_12; // Placeholder for 4 bytes, based on offsets
    struct BacklogItem* items; // Offset 16 (0x10)
    struct Sprint* next;       // Offset 20 (0x14)
}; // Expected size: 24 bytes (0x18)

struct Product {
    short id;
    char* name;
    struct BacklogItem* backlog_head;
    struct Sprint* sprints_head;
    struct Product* next;
}; // Expected size: 20 bytes (0x14)

// Placeholder for obf_strings function
// In a real scenario, this would retrieve obfuscated strings from a resource.
const char* obf_strings(const char* key) {
    if (strcmp(key, "List_Products") == 0) return "  Product Name: %s\n";
    if (strcmp(key, "Prod_Title") == 0) return "Product: %s\n";
    if (strcmp(key, "Prod_ID") == 0) return "ID: %u\n";
    if (strcmp(key, "Prod_Backlog") == 0) return "\n  Product Backlog:\n";
    if (strcmp(key, "Prod_Backlog2") == 0) return "    ID | Qty | Description\n";
    if (strcmp(key, "Prod_Backlog3") == 0) return "    %u | %u   | %s\n"; // Modified to include description
    if (strcmp(key, "Sprints_Title") == 0) return "\n  Sprints:\n";
    if (strcmp(key, "Sprint_Entry") == 0) return "    Sprint %u: %s\n";
    if (strcmp(key, "SBI_Entry") == 0) return "      SBI ID: %u, Qty: %u, Status: %u, Desc: %s\n";
    return "";
}

// Helper function to free a list of BacklogItem or SprintBacklogItem
static void free_backlog_items(struct BacklogItem* head) {
    struct BacklogItem* current = head;
    while (current != NULL) {
        struct BacklogItem* next_item = current->next;
        if (current->description != NULL) {
            free(current->description);
        }
        free(current);
        current = next_item;
    }
}

// Helper function to free a list of Sprints
static void free_sprints(struct Sprint* head) {
    struct Sprint* current = head;
    while (current != NULL) {
        struct Sprint* next_sprint = current->next;
        if (current->description != NULL) {
            free(current->description);
        }
        free_backlog_items(current->items); // Free SBI list within sprint
        free(current);
        current = next_sprint;
    }
}

// Function: create_product
int create_product(short **param_1, const short *param_2) {
    struct Product **head_ptr = (struct Product **)param_1;
    const short product_id_val = *param_2;
    // Assuming the product name string immediately follows the short ID in param_2's memory
    const char *product_name_str = (const char *)(param_2 + 1); 

    struct Product *current_product;

    if (*head_ptr == NULL) { // If product list is empty
        current_product = (struct Product *)calloc(1, sizeof(struct Product));
        if (current_product == NULL) {
            _terminate(); // Allocation failed
        }
        *head_ptr = current_product;
    } else { // Product list not empty, traverse to find existing ID or end of list
        current_product = *head_ptr;
        while (current_product->next != NULL && current_product->id != product_id_val) {
            current_product = current_product->next;
        }

        if (current_product->id == product_id_val) { // Product with this ID already exists
            return -1; // 0xffffffff
        }

        // Append new product to the end of the list
        struct Product *new_product = (struct Product *)calloc(1, sizeof(struct Product));
        if (new_product == NULL) {
            _terminate(); // Allocation failed
        }
        current_product->next = new_product;
        current_product = new_product;
    }

    // Initialize new product fields
    current_product->next = NULL;
    current_product->id = product_id_val;
    current_product->backlog_head = NULL;
    current_product->sprints_head = NULL;

    // Handle product name
    size_t name_len = strlen(product_name_str);
    current_product->name = (char *)calloc(name_len + 1, sizeof(char)); // +1 for null terminator
    if (current_product->name == NULL) {
        _terminate(); // Allocation failed
    }
    strncpy(current_product->name, product_name_str, name_len);
    current_product->name[name_len] = '\0'; // Ensure null termination

    return 0;
}

// Function: delete_product
int delete_product(short **param_1, const short *param_2) {
    struct Product **head_ptr = (struct Product **)param_1;
    const short product_id_to_delete = *param_2;

    if (*head_ptr == NULL) { // List is empty
        return -1; // 0xffffffff
    }

    struct Product *product_to_delete = NULL;
    struct Product *prev_product = NULL;

    if ((*head_ptr)->id == product_id_to_delete) { // Head of list matches ID
        product_to_delete = *head_ptr;
        *head_ptr = product_to_delete->next; // Update head to the next product
    } else {
        prev_product = *head_ptr;
        product_to_delete = prev_product->next;
        while (product_to_delete != NULL && product_to_delete->id != product_id_to_delete) {
            prev_product = product_to_delete;
            product_to_delete = product_to_delete->next;
        }

        if (product_to_delete == NULL) { // Product not found
            return -1; // 0xffffffff
        }
        // Product found, relink previous product's next pointer
        prev_product->next = product_to_delete->next;
    }

    // Free all associated memory for the product_to_delete
    if (product_to_delete->name != NULL) {
        free(product_to_delete->name);
    }
    free_backlog_items(product_to_delete->backlog_head);
    free_sprints(product_to_delete->sprints_head);
    free(product_to_delete);

    return 0;
}

// Function: list_all_products
// param_1 is effectively a (struct Product *) pointer, but passed as int in original
int list_all_products(int param_1) {
    struct Product *head = (struct Product *)param_1;

    if (head == NULL) {
        return -1; // 0xffffffff
    }

    struct Product *current = head;
    while (current != NULL) {
        printf(obf_strings("List_Products"), current->name);
        current = current->next;
    }
    printf("\n");
    return 0;
}

// Function: list_product
// param_1 is effectively a (struct Product *) pointer, param_2 is (const ushort *) for product ID
int list_product(ushort *param_1, const ushort *param_2) {
    struct Product *head = (struct Product *)param_1;
    const ushort target_id = *param_2;

    if (target_id == 0) { // If ID is 0, list all products
        return list_all_products((int)head); // Cast back to int for original function signature
    }

    struct Product *product = head;
    while (product != NULL && product->id != target_id) {
        product = product->next;
    }

    if (product == NULL) { // Product not found
        return -1; // 0xffffffff
    } else { // Product found
        printf("\n");
        printf(obf_strings("Prod_Title"), product->name);
        printf(obf_strings("Prod_ID"), (unsigned int)product->id);
        printf("\n");

        printf(obf_strings("Prod_Backlog"));
        printf(obf_strings("Prod_Backlog2"));
        struct BacklogItem *item = product->backlog_head;
        while (item != NULL) {
            printf(obf_strings("Prod_Backlog3"), (unsigned int)item->id, (unsigned int)item->quantity, item->description);
            item = item->next;
        }
        printf("\n");

        printf(obf_strings("Sprints_Title"));
        struct Sprint *sprint = product->sprints_head;
        while (sprint != NULL) {
            printf(obf_strings("Sprint_Entry"), (unsigned int)sprint->id, sprint->description);

            struct BacklogItem *sbi = sprint->items;
            while (sbi != NULL) {
                printf(obf_strings("SBI_Entry"), (unsigned int)sbi->id, (unsigned int)sbi->quantity, (unsigned int)sbi->status, sbi->description);
                sbi = sbi->next;
            }
            printf("\n");
            sprint = sprint->next;
        }
        printf("\n");
        return 0;
    }
}