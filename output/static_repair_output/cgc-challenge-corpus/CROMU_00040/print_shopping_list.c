#include <stdio.h>  // For printf
#include <stdlib.h> // For malloc, free, exit

// Forward declarations of structures
typedef struct Product Product;
typedef struct ItemDetails ItemDetails;
typedef struct ShoppingItem ShoppingItem;

// Dummy declaration for the external sort function
// A real implementation would be required for actual sorting.
void sort_shopping_list(ShoppingItem *head);

// Structure for a node in the shopping list
struct ShoppingItem {
    int item_value;         // The actual item identifier or value
    ShoppingItem *next;     // Pointer to the next item in the list
};

// Structure representing details of an item within a product's list
// The original code accessed item_id at offset 0x14 and next_item_detail at 0x78.
// In standard C, we declare fields and let the compiler determine layout.
// Assuming these are the relevant fields for the logic.
struct ItemDetails {
    int item_id;            // Identifier for this specific item detail
    ItemDetails *next_item_detail; // Pointer to the next item detail in the product's internal list
    // Other fields that might exist in the original structure are not explicitly used here.
};

// Structure representing a product
// The original code accessed is_purchasable at offset 200, items_list_head at 0xcc,
// and next_product at 0xd4. In standard C, we declare fields and let the compiler
// determine layout. Assuming these are the relevant fields for the logic.
struct Product {
    int is_purchasable;     // Flag indicating if the product can be purchased (e.g., == 1)
    ItemDetails *items_list_head; // Head of a linked list of ItemDetails for this product
    Product *next_product;  // Pointer to the next product in the main product list
    // Other fields that might exist in the original structure are not explicitly used here.
};

// Function: build_shopping_list
// Creates a flattened shopping list (linked list of ShoppingItem) from a list of products.
// Only items from "purchasable" products are included.
ShoppingItem *build_shopping_list(Product *product_list_head) {
    ShoppingItem *head = NULL;
    // `current_link` is a pointer to the `next` field of the last `ShoppingItem`
    // added to the list, or to `head` if the list is empty. This simplifies
    // appending new nodes.
    ShoppingItem **current_link = &head;

    // Iterate through the main list of products
    while (product_list_head != NULL) {
        // Check if the current product is marked as purchasable
        if (product_list_head->is_purchasable == 1) {
            // Iterate through the list of item details for this purchasable product
            ItemDetails *current_item_detail = product_list_head->items_list_head;
            while (current_item_detail != NULL) {
                // Allocate memory for a new shopping list item
                *current_link = (ShoppingItem *)malloc(sizeof(ShoppingItem));
                if (*current_link == NULL) {
                    // Handle memory allocation failure
                    printf("Error: unable to malloc memory for shopping list item.\n");
                    exit(EXIT_FAILURE); // Replaced _terminate() with standard exit
                }

                // Initialize the new shopping list item
                (*current_link)->item_value = current_item_detail->item_id;
                (*current_link)->next = NULL; // New node is initially the last

                // Advance `current_link` to point to the `next` field of the newly added node.
                // This prepares it for linking the *next* new node.
                current_link = &(*current_link)->next;

                // Move to the next item detail in the product's internal list
                current_item_detail = current_item_detail->next_item_detail;
            }
        }
        // Move to the next product in the main product list
        product_list_head = product_list_head->next_product;
    }
    return head;
}

// Function: print_shopping_list
// Generates a shopping list, prints it, and then frees the allocated memory.
void print_shopping_list(Product *product_list_head) {
    ShoppingItem *shopping_list_head;

    shopping_list_head = build_shopping_list(product_list_head);

    printf("\n");
    printf("Shopping List\n");
    printf("-------------\n");

    if (shopping_list_head != NULL) {
        // Sort the shopping list (assuming sort_shopping_list is implemented elsewhere)
        sort_shopping_list(shopping_list_head);

        // Print each item in the sorted shopping list
        ShoppingItem *current_node;
        for (current_node = shopping_list_head; current_node != NULL; current_node = current_node->next) {
            printf("%d\n", current_node->item_value); // Assuming item_value is an integer ID
        }

        // Free the dynamically allocated shopping list
        ShoppingItem *temp_node;
        current_node = shopping_list_head; // Start from the head
        while (current_node != NULL) {
            temp_node = current_node->next; // Save pointer to the next node
            free(current_node);             // Free the current node
            current_node = temp_node;       // Move to the next node
        }
    }
    printf("\n");
    return;
}

// Dummy implementation for sort_shopping_list to make the code compilable.
// In a real application, this function would contain logic to sort the linked list.
void sort_shopping_list(ShoppingItem *head) {
    // This is a placeholder. A real implementation would sort the list.
    (void)head; // Suppress unused parameter warning
}