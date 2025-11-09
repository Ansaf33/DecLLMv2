#include <stdlib.h> // Required for malloc and free
#include <stdio.h>  // Required for printf in the main function

// Struct for general card list nodes (using int for value, consistent with undefined4 -> int)
typedef struct CardNode {
    int value;
    struct CardNode *next;
} CardNode;

// Struct for ordered card list nodes (using unsigned int for value, consistent with uint -> unsigned int)
typedef struct OrderedCardNode {
    unsigned int value;
    struct OrderedCardNode *next;
} OrderedCardNode;

// Helper function to print a CardNode list
void print_card_list(const char *name, CardNode *head) {
    printf("%s: [", name);
    CardNode *current = head;
    while (current != NULL) {
        printf("%d", current->value);
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
}

// Helper function to print an OrderedCardNode list
void print_ordered_card_list(const char *name, OrderedCardNode *head) {
    printf("%s: [", name);
    OrderedCardNode *current = head;
    while (current != NULL) {
        printf("%u", current->value);
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
}

// Function: get_card - Appends a new card to the end of a CardNode list.
// Returns 0 on success, -1 on failure (e.g., invalid card_value or memory allocation failure).
int get_card(CardNode **head, int card_value) {
    if (card_value <= 0) { // Assuming 0 or negative is an invalid card value
        return -1;
    }

    CardNode *newNode = (CardNode *)malloc(sizeof(CardNode));
    if (newNode == NULL) {
        return -1; // Memory allocation failed
    }

    newNode->value = card_value;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode; // List is empty, new node becomes head
    } else {
        CardNode *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode; // Append to the end
    }
    return 0;
}

// Function: get_card_in_order - Inserts a new card into an OrderedCardNode list, maintaining sorted order.
// Returns 0 on success, -1 on failure (e.g., invalid card_value or memory allocation failure).
int get_card_in_order(OrderedCardNode **head, unsigned int card_value) {
    if (card_value == 0) { // Assuming 0 is an invalid card value for unsigned
        return -1;
    }

    OrderedCardNode *newNode = (OrderedCardNode *)malloc(sizeof(OrderedCardNode));
    if (newNode == NULL) {
        return -1; // Memory allocation failed
    }

    newNode->value = card_value;
    newNode->next = NULL;

    // Handle empty list or insertion at the beginning
    if (*head == NULL || newNode->value < (*head)->value) {
        newNode->next = *head;
        *head = newNode;
        return 0;
    }

    // Traverse to find the correct insertion point
    OrderedCardNode *current = *head;
    while (current->next != NULL && newNode->value >= current->next->value) {
        current = current->next;
    }

    // Insert newNode after current
    newNode->next = current->next;
    current->next = newNode;

    return 0;
}

// Function: replace_card - Replaces the value of a card in a CardNode list.
// Returns 0 on success, -1 if the target_value is not found or list is empty.
int replace_card(CardNode **head, int new_value, int target_value) {
    if (*head == NULL) {
        return -1; // List is empty
    }

    CardNode *current = *head;
    while (current != NULL) {
        if (target_value == current->value) {
            current->value = new_value;
            return 0;
        }
        current = current->next;
    }
    return -1; // Card not found
}

// Function: remove_card - Removes the first occurrence of a card with a specific value from a CardNode list.
// Returns 0 on success, -1 if the card_value_to_remove is not found or list is empty.
int remove_card(CardNode **head, int card_value_to_remove) {
    if (*head == NULL) {
        return -1; // List is empty
    }

    // Special case: head node contains the value
    if ((*head)->value == card_value_to_remove) {
        CardNode *temp = *head;
        *head = (*head)->next;
        free(temp);
        return 0;
    }

    CardNode *current = *head;
    while (current->next != NULL) {
        if (current->next->value == card_value_to_remove) {
            CardNode *temp = current->next;
            current->next = temp->next;
            free(temp);
            return 0;
        }
        current = current->next;
    }
    return -1; // Card not found
}

// Function: discard_hand - Frees all nodes in a CardNode list and sets the head to NULL.
void discard_hand(CardNode **head) {
    if (head == NULL || *head == NULL) {
        return;
    }

    CardNode *current = *head;
    while (current != NULL) {
        CardNode *next_node = current->next; // Store next before freeing current
        free(current);
        current = next_node;
    }
    *head = NULL; // Set head to NULL after discarding all nodes
}

// Function: discard_ordered_hand - Frees all nodes in an OrderedCardNode list and sets the head to NULL.
void discard_ordered_hand(OrderedCardNode **head) {
    if (head == NULL || *head == NULL) {
        return;
    }

    OrderedCardNode *current = *head;
    while (current != NULL) {
        OrderedCardNode *next_node = current->next; // Store next before freeing current
        free(current);
        current = next_node;
    }
    *head = NULL; // Set head to NULL after discarding all nodes
}

int main() {
    CardNode *hand = NULL;
    OrderedCardNode *ordered_hand = NULL;

    printf("--- Demonstrating get_card (append to general hand) ---\n");
    get_card(&hand, 5);
    get_card(&hand, 10);
    get_card(&hand, 3);
    print_card_list("General Hand (appended)", hand); // Expected: [5, 10, 3]

    printf("\n--- Demonstrating get_card_in_order (insert into ordered hand) ---\n");
    get_card_in_order(&ordered_hand, 5U);
    get_card_in_order(&ordered_hand, 10U);
    get_card_in_order(&ordered_hand, 3U);
    get_card_in_order(&ordered_hand, 7U);
    get_card_in_order(&ordered_hand, 1U);
    print_ordered_card_list("Ordered Hand", ordered_hand); // Expected: [1, 3, 5, 7, 10]

    printf("\n--- Demonstrating replace_card ---\n");
    replace_card(&hand, 8, 10); // Replace card with value 10 with 8
    print_card_list("General Hand (after replacing 10 with 8)", hand); // Expected: [5, 8, 3]
    if (replace_card(&hand, 99, 100) == -1) {
        printf("Attempted to replace non-existent card 100. (Correctly failed)\n");
    }

    printf("\n--- Demonstrating remove_card ---\n");
    remove_card(&hand, 5); // Remove card with value 5 (head)
    print_card_list("General Hand (after removing 5)", hand); // Expected: [8, 3]
    remove_card(&hand, 3); // Remove card with value 3 (tail)
    print_card_list("General Hand (after removing 3)", hand); // Expected: [8]
    remove_card(&hand, 8); // Remove card with value 8 (last element)
    print_card_list("General Hand (after removing 8)", hand); // Expected: []
    if (remove_card(&hand, 100) == -1) {
        printf("Attempted to remove non-existent card 100 from empty list. (Correctly failed)\n");
    }

    printf("\n--- Demonstrating discard_hand ---\n");
    get_card(&hand, 1);
    get_card(&hand, 2);
    get_card(&hand, 3);
    print_card_list("General Hand before discard", hand);
    discard_hand(&hand);
    print_card_list("General Hand after discard", hand); // Expected: []

    print_ordered_card_list("Ordered Hand before discard", ordered_hand);
    discard_ordered_hand(&ordered_hand);
    print_ordered_card_list("Ordered Hand after discard", ordered_hand); // Expected: []

    return 0;
}