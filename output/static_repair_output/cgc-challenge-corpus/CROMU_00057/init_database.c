#include <stdio.h>   // For printf (in main, if included)
#include <stdlib.h>  // For malloc, free, exit, NULL

// Define structs with __attribute__((packed)) to ensure byte-perfect
// alignment and sizes as implied by the raw memory offsets in the original code.
// This assumes a 32-bit architecture where pointers are 4 bytes.
// If compiling on a 64-bit system without __attribute__((packed)),
// default compiler padding would break the offset logic.

// Structure for the innermost list (NodeD, corresponding to local_1c)
typedef struct __attribute__((packed)) NodeD {
    char _padding_0_8[8];     // Placeholder for fields before offset 8
    void *data_to_free_d;     // Data pointer to free, at offset 8
    struct NodeD *next_d;     // Pointer to the next NodeD, at offset 0xc
} NodeD;

// Structure for the second nested list (NodeC, corresponding to local_14)
typedef struct __attribute__((packed)) NodeC {
    char _padding_0_4[4];     // Placeholder for fields before offset 4
    void *data_to_free_c;     // Data pointer to free, at offset 4
    char _padding_8_10[8];    // Placeholder for fields between offset 8 and 0x10
    NodeD *sublist_d;         // Pointer to NodeD sublist, at offset 0x10
    struct NodeC *next_c;     // Pointer to the next NodeC, at offset 0x14
} NodeC;

// Structure for the first nested list (NodeB, corresponding to local_18)
typedef struct __attribute__((packed)) NodeB {
    char _padding_0_8[8];     // Placeholder for fields before offset 8
    void *data_to_free_b;     // Data pointer to free, at offset 8
    struct NodeB *next_b;     // Pointer to the next NodeB, at offset 0xc
} NodeB;

// Structure for the main list (NodeA, corresponding to local_10)
typedef struct __attribute__((packed)) NodeA {
    char _padding_0_4[4];     // Placeholder for fields before offset 4
    void *data_to_free_a;     // Data pointer to free, at offset 4
    NodeB *sublist_b;         // Pointer to NodeB sublist, at offset 8
    NodeC *sublist_c;         // Pointer to NodeC sublist, at offset 0xc
    struct NodeA *next_a;     // Pointer to the next NodeA, at offset 0x10
} NodeA;

// Function: init_database
int init_database(int *param_1) {
  if (*param_1 == 0) {
    // Reduce intermediate variable 'pvVar1' by directly checking malloc's return.
    // 0x14 is 20 bytes.
    if (malloc(0x14) == NULL) {
      exit(EXIT_FAILURE); // Use standard exit for program termination on failure
    }
  }
  return 0;
}

// Function: destroy_database
int destroy_database(void **param_1) {
  NodeA *current_a = (NodeA *)*param_1; // Use NodeA struct for type safety and readability
  
  while (current_a != NULL) {
    // Iterate and free NodeB sublist
    NodeB *current_b = current_a->sublist_b;
    while (current_b != NULL) {
      if (current_b->data_to_free_b != NULL) {
        free(current_b->data_to_free_b);
      }
      NodeB *next_b = current_b->next_b; // Store next pointer before freeing current
      free(current_b);
      current_b = next_b;
    }

    // Iterate and free NodeC sublist
    NodeC *current_c = current_a->sublist_c;
    while (current_c != NULL) {
      // Iterate and free NodeD sub-sublist
      NodeD *current_d = current_c->sublist_d;
      while (current_d != NULL) {
        if (current_d->data_to_free_d != NULL) {
          free(current_d->data_to_free_d);
        }
        NodeD *next_d = current_d->next_d; // Store next pointer before freeing current
        free(current_d);
        current_d = next_d;
      }

      if (current_c->data_to_free_c != NULL) {
        free(current_c->data_to_free_c);
      }
      NodeC *next_c = current_c->next_c; // Store next pointer before freeing current
      free(current_c);
      current_c = next_c;
    }

    if (current_a->data_to_free_a != NULL) {
      free(current_a->data_to_free_a);
    }
    NodeA *next_a = current_a->next_a; // Store next pointer before freeing current
    free(current_a);
    current_a = next_a;
  }
  *param_1 = NULL; // Set the head pointer to NULL after freeing all nodes
  return 0;
}

// A minimal main function to make the code compilable and runnable
int main() {
    printf("Starting program...\n");

    // Test init_database
    int init_flag = 0;
    printf("Calling init_database with init_flag = %d\n", init_flag);
    init_database(&init_flag);
    printf("init_database completed.\n");

    // Test destroy_database with a dummy structure
    // This part demonstrates how destroy_database would be used.
    // In a real application, the database would be built up before being destroyed.
    void *database_head = NULL;

    // Create a dummy NodeA with sublists for demonstration
    NodeA *node_a1 = (NodeA *)malloc(sizeof(NodeA));
    if (node_a1 == NULL) exit(EXIT_FAILURE);
    node_a1->data_to_free_a = malloc(sizeof(int)); // Allocate dummy data
    if (node_a1->data_to_free_a == NULL) exit(EXIT_FAILURE);
    *((int*)node_a1->data_to_free_a) = 100;

    NodeB *node_b1 = (NodeB *)malloc(sizeof(NodeB));
    if (node_b1 == NULL) exit(EXIT_FAILURE);
    node_b1->data_to_free_b = malloc(sizeof(char) * 5); // Allocate dummy data
    if (node_b1->data_to_free_b == NULL) exit(EXIT_FAILURE);
    
    node_b1->next_b = NULL;
    node_a1->sublist_b = node_b1;

    NodeC *node_c1 = (NodeC *)malloc(sizeof(NodeC));
    if (node_c1 == NULL) exit(EXIT_FAILURE);
    node_c1->data_to_free_c = malloc(sizeof(float)); // Allocate dummy data
    if (node_c1->data_to_free_c == NULL) exit(EXIT_FAILURE);

    NodeD *node_d1 = (NodeD *)malloc(sizeof(NodeD));
    if (node_d1 == NULL) exit(EXIT_FAILURE);
    node_d1->data_to_free_d = malloc(sizeof(double)); // Allocate dummy data
    if (node_d1->data_to_free_d == NULL) exit(EXIT_FAILURE);

    node_d1->next_d = NULL;
    node_c1->sublist_d = node_d1;
    node_c1->next_c = NULL;
    node_a1->sublist_c = node_c1;

    node_a1->next_a = NULL;
    database_head = node_a1;

    printf("Dummy database structure created. Calling destroy_database.\n");
    destroy_database(&database_head);
    printf("destroy_database completed. database_head is now %p (should be NULL)\n", database_head);

    printf("Program finished.\n");
    return 0;
}