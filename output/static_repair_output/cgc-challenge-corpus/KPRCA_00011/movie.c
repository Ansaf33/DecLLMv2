#include <stdint.h> // For uint32_t
#include <stdlib.h> // For malloc, free, NULL
#include <string.h> // For strcmp

// Define the linked list node structure.
// This structure is inferred from the original code's memory access patterns
// and the use of strcmp.
typedef struct MovieNode {
    char *movie_title; // Pointer to the movie title string (assumed to be dynamically allocated by caller, e.g., via strdup)
    struct MovieNode *next; // Pointer to the next node in the list
} MovieNode;

// Function: movie_g2s
char * movie_g2s(uint32_t param_1) {
  if (param_1 == 4) {
    return "Horror";
  }
  if (param_1 == 3) {
    return "Comedy";
  }
  if (param_1 == 1) {
    return "Action";
  }
  if (param_1 == 2) {
    return "Romance";
  }
  return "Other";
}

// Function: movie_r2s
char * movie_r2s(uint32_t param_1) {
  if (param_1 == 4) {
    return "R";
  }
  if (param_1 == 3) {
    return "PG13";
  }
  if (param_1 == 1) {
    return "G";
  }
  if (param_1 == 2) {
    return "PG";
  }
  return "Unknown";
}

// Function: movie_find
// Finds a movie node by title.
// Returns a pointer to the MovieNode if found, otherwise NULL.
MovieNode *movie_find(MovieNode *head, const char *target_title) {
  if (head == NULL || target_title == NULL) {
    return NULL;
  }

  MovieNode *current = head;
  while (current != NULL) {
    if (current->movie_title != NULL && strcmp(current->movie_title, target_title) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

// Function: free_movie
// Frees the resources associated with a single MovieNode.
// Specifically, it frees the movie title string (if dynamically allocated)
// and then the MovieNode structure itself.
// NOTE: The original function had a bug attempting to free param_1[1] (node->next),
// which would corrupt the linked list. This has been removed.
void free_movie(MovieNode *node) {
  if (node != NULL) {
    if (node->movie_title != NULL) {
      free(node->movie_title); // Assume movie_title was dynamically allocated by caller (e.g., via strdup)
    }
    free(node); // Free the MovieNode structure
  }
}

// Function: movie_add
// Adds a new movie node to the linked list.
// param_1: Pointer to the head pointer of the list (MovieNode **).
// param_2: The movie title string (char *). Assumed to be dynamically allocated by caller.
// Returns 0 on success, 0xffffffff on error (e.g., malloc failure, movie already exists).
uint32_t movie_add(MovieNode **head, char *title_to_add) {
  if (head == NULL || title_to_add == NULL) {
    return 0xffffffff;
  }

  // Check if movie already exists before allocating
  if (*head != NULL && movie_find(*head, title_to_add) != NULL) {
    return 0xffffffff; // Error: Movie with this title already exists
  }

  MovieNode *newNode = (MovieNode *)malloc(sizeof(MovieNode));
  if (newNode == NULL) {
    return 0xffffffff; // Error: malloc failed
  }

  newNode->movie_title = title_to_add; // Store the pointer to the title string
  newNode->next = NULL;

  if (*head == NULL) { // If the list is empty, new node becomes the head
    *head = newNode;
  } else { // Append to the end of the list
    MovieNode *current = *head;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = newNode;
  }
  return 0; // Success
}

// Function: movie_delete
// Deletes a movie node from the list by its 1-based index.
// param_1: Pointer to the head pointer of the list (MovieNode **).
// param_2: The 1-based index of the movie to delete.
// Returns 0 on success, 0xffffffff if the list is invalid or index not found.
uint32_t movie_delete(MovieNode **head, int index_to_delete) {
  if (head == NULL || *head == NULL || index_to_delete <= 0) {
    return 0xffffffff; // Error: Invalid head, empty list, or invalid index
  }

  if (index_to_delete == 1) { // Deleting the head node
    MovieNode *nodeToDelete = *head;
    *head = nodeToDelete->next; // Update head to the next node
    free_movie(nodeToDelete); // Free the deleted node's resources
    return 0;
  }

  MovieNode *prev = *head;
  MovieNode *current = prev->next;
  int current_index = 2; // Start from the second node

  while (current != NULL) {
    if (current_index == index_to_delete) {
      prev->next = current->next; // Link previous node to current's next
      free_movie(current); // Free the deleted node's resources
      return 0;
    }
    prev = current;
    current = current->next;
    current_index++;
  }

  return 0xffffffff; // Movie not found at the given index
}

// Function: movie_update
// Placeholder function, returns 0.
uint32_t movie_update(void) {
  return 0;
}

// Function: movie_find_by_id
// Finds a movie node by its 1-based index.
// param_1: Head of the linked list (MovieNode *).
// param_2: The 1-based index of the movie to find.
// Returns a pointer to the MovieNode if found, otherwise NULL.
MovieNode *movie_find_by_id(MovieNode *head, int target_id) {
  if (head == NULL || target_id <= 0) {
    return NULL;
  }

  MovieNode *current = head;
  int current_id = 1;
  while (current != NULL) {
    if (target_id == current_id) {
      return current;
    }
    current = current->next;
    current_id++;
  }
  return NULL; // Movie not found at the given index
}