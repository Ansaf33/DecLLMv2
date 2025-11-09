#include <stdio.h>   // For printf, getchar, NULL
#include <stdlib.h>  // For malloc, free, exit, srandom, random, strtoul
#include <string.h>  // For strlen, strcpy, strcmp, memset
#include <time.h>    // For time (used with srandom)
#include <ctype.h>   // For isprint (not explicitly used but good for char handling)

// Forward declarations for types
typedef struct Movie Movie;
typedef struct MovieNode MovieNode;

// Function pointer type for print_func
typedef void (*PrintMovieDetailFunc)(int, Movie*);

// Movie structure definition (runtime)
struct Movie {
    char *title;
    char *description;
    int year;
    int review_score;
    int genre; // 1:Action, 2:Romance, 3:Comedy, 4:Horror, 5:Other
    int rating; // 1:G, 2:PG, 3:PG13, 4:R, 5:Unknown
    PrintMovieDetailFunc print_func; // Function pointer for printing
};

// MovieNode for linked lists
struct MovieNode {
    Movie *movie;
    MovieNode *next;
};

// Global variables
char g_password[23]; // 0x16 + null terminator
MovieNode *movies_full = NULL;
MovieNode *movies_rented = NULL;
int g_num_movies = 0; // Tracks number of movies in movies_full

// Default movie data for initialization
// This structure holds the initial data that will be copied into dynamically allocated Movie structs.
typedef struct {
    const char *title;
    const char *description;
    int year;
    int review_score;
    int genre;
    int rating;
} DefaultMovieData;

#define NUM_DEFAULT_MOVIES 10
const DefaultMovieData DEFAULT_MOVIES_ARRAY[NUM_DEFAULT_MOVIES] = {
    {"The Shawshank Redemption", "Two imprisoned men bond over a number of years...", 1994, 93, 1, 4}, // Action, R
    {"The Godfather", "The aging patriarch of an organized crime dynasty...", 1972, 92, 5, 4}, // Other, R
    {"The Dark Knight", "When the menace known as the Joker emerges...", 2008, 90, 1, 3}, // Action, PG-13
    {"Pulp Fiction", "The lives of two mob hitmen, a boxer, a gangster and his wife...", 1994, 89, 5, 4}, // Other, R
    {"Forrest Gump", "The presidencies of Kennedy and Johnson...", 1994, 88, 3, 3}, // Comedy, PG-13
    {"Inception", "A thief who steals corporate secrets through use of dream-sharing technology...", 2010, 87, 1, 3}, // Action, PG-13
    {"The Matrix", "A computer hacker learns from mysterious rebels...", 1999, 87, 1, 4}, // Action, R
    {"Spirited Away", "During her family's move to the suburbs, a sullen 10-year-old girl...", 2001, 86, 5, 2}, // Other, PG
    {"Interstellar", "A team of explorers travel through a wormhole in space...", 2014, 86, 5, 3}, // Other, PG-13
    {"Parasite", "Greed and class discrimination threaten the newly formed symbiotic relationship...", 2019, 85, 5, 4} // Other, R
};

// Function declarations (from snippet or stubs)
void quit(void);
int receive(int fd, char *buf, size_t count, int *bytes_read);
void free_movie_data(Movie *movie);
int movie_add(MovieNode **head, Movie *new_movie);
MovieNode *movie_find_by_id(MovieNode *head, unsigned int id);
MovieNode *movie_find(MovieNode *head, Movie *target_movie);
int movie_remove_node(MovieNode **head, unsigned int id, Movie **out_movie);
int movie_delete_full(MovieNode **head, unsigned int id);
const char *movie_g2s(int genre_code);
const char *movie_r2s(int rating_code);
void print_movie_detail(int id, Movie *movie); // Declaration for use in Movie struct

// Stubs for functions not fully provided in the snippet
void quit(void) {
    printf("Bye!\n");
    exit(0);
}

int receive(int fd, char *buf, size_t count, int *bytes_read) {
    if (fd == 0) { // stdin
        int c = getchar();
        if (c == EOF) {
            *bytes_read = 0;
            return -1; // Indicate error or EOF
        }
        *buf = (char)c;
        *bytes_read = 1;
        return 0; // Success
    }
    // For other FDs, simulate failure or return 0
    *bytes_read = 0;
    return -1; // Error
}

// Helper to free a Movie struct's dynamically allocated strings and the struct itself
void free_movie_data(Movie *movie) {
    if (movie == NULL) return;
    free(movie->title);
    free(movie->description);
    free(movie);
}

// Adds a movie to a linked list, returns 0 on success, -1 on failure
int movie_add(MovieNode **head, Movie *new_movie) {
    if (new_movie == NULL) return -1;
    MovieNode *newNode = (MovieNode *)malloc(sizeof(MovieNode));
    if (newNode == NULL) return -1;
    newNode->movie = new_movie;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
    } else {
        MovieNode *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    if (head == &movies_full) { // Only increment for movies_full
        g_num_movies++;
    }
    return 0;
}

// Finds a movie by its 1-based ID (index in the list)
MovieNode *movie_find_by_id(MovieNode *head, unsigned int id) {
    if (id == 0) return NULL;
    MovieNode *current = head;
    unsigned int count = 1;
    while (current != NULL) {
        if (count == id) {
            return current;
        }
        current = current->next;
        count++;
    }
    return NULL;
}

// Finds a movie by comparing its Movie* pointer (used for checking if already rented)
MovieNode *movie_find(MovieNode *head, Movie *target_movie) {
    MovieNode *current = head;
    while (current != NULL) {
        if (current->movie == target_movie) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Removes a movie node from the linked list by its 1-based ID, and returns the Movie*
// Does NOT free the Movie* data itself. Does NOT affect g_num_movies.
int movie_remove_node(MovieNode **head, unsigned int id, Movie **out_movie) {
    if (id == 0 || *head == NULL) return -1;

    MovieNode *current = *head;
    MovieNode *prev = NULL;
    unsigned int count = 1;

    while (current != NULL && count != id) {
        prev = current;
        current = current->next;
        count++;
    }

    if (current == NULL) { // Movie not found
        return -1;
    }

    if (prev == NULL) { // Deleting head
        *head = current->next;
    } else {
        prev->next = current->next;
    }
    
    *out_movie = current->movie; // Return the Movie*
    free(current); // Free the node
    return 0;
}

// Deletes a movie from the linked list by its 1-based ID, and frees the Movie* data
int movie_delete_full(MovieNode **head, unsigned int id) {
    Movie *movie_to_free = NULL;
    int result = movie_remove_node(head, id, &movie_to_free); // Use the generic remove node
    if (result == 0) {
        if (movie_to_free != NULL) {
            free_movie_data(movie_to_free);
        }
        if (head == &movies_full) { // Only decrement for movies_full
            g_num_movies--;
        }
    }
    return result;
}

// Converts genre code to string
const char *movie_g2s(int genre_code) {
    switch (genre_code) {
        case 1: return "Action";
        case 2: return "Romance";
        case 3: return "Comedy";
        case 4: return "Horror";
        case 5: return "Other";
        default: return "Unknown";
    }
}

// Converts rating code to string
const char *movie_r2s(int rating_code) {
    switch (rating_code) {
        case 1: return "G";
        case 2: return "PG";
        case 3: return "PG-13";
        case 4: return "R";
        case 5: return "Unknown";
        default: return "Unknown";
    }
}


// Function: initialize
void initialize(void) {
    printf("\n   . . . Initializing the inventory . . .\n         (movie info from IMDb.com)\n");

    for (size_t i = 0; i < sizeof(g_password) - 1; ++i) {
        g_password[i] = (char)(random() % 26) + ( (random() % 2 == 0) ? 'a' : 'A' );
    }
    g_password[sizeof(g_password) - 1] = '\0';

    for (size_t i = 0; i < NUM_DEFAULT_MOVIES; ++i) {
        Movie *new_movie = (Movie *)malloc(sizeof(Movie));
        if (new_movie == NULL) {
            printf("[ERROR] Initialization failed: Out of memory. Exit.\n");
            quit();
        }
        memset(new_movie, 0, sizeof(Movie)); // Initialize to zeros

        // Allocate and copy title
        size_t title_len = strlen(DEFAULT_MOVIES_ARRAY[i].title);
        new_movie->title = (char *)malloc(title_len + 1);
        if (new_movie->title == NULL) {
            printf("[ERROR] Initialization failed: Out of memory. Exit.\n");
            free(new_movie);
            quit();
        }
        strcpy(new_movie->title, DEFAULT_MOVIES_ARRAY[i].title);

        // Allocate and copy description
        size_t desc_len = strlen(DEFAULT_MOVIES_ARRAY[i].description);
        new_movie->description = (char *)malloc(desc_len + 1);
        if (new_movie->description == NULL) {
            printf("[ERROR] Initialization failed: Out of memory. Exit.\n");
            free(new_movie->title);
            free(new_movie);
            quit();
        }
        strcpy(new_movie->description, DEFAULT_MOVIES_ARRAY[i].description);

        // Assign integer fields directly
        new_movie->year = DEFAULT_MOVIES_ARRAY[i].year;
        new_movie->review_score = DEFAULT_MOVIES_ARRAY[i].review_score;
        new_movie->genre = DEFAULT_MOVIES_ARRAY[i].genre;
        new_movie->rating = DEFAULT_MOVIES_ARRAY[i].rating;
        new_movie->print_func = print_movie_detail; // Assign function pointer

        if (movie_add(&movies_full, new_movie) != 0) {
            printf("[ERROR] Initialization failed: Could not add movie. Exit.\n");
            free(new_movie->title);
            free(new_movie->description);
            free(new_movie);
            quit();
        }
    }
    return;
}

// Function: readuntil
int readuntil(int fd, char *buffer, unsigned int max_len, char delimiter) {
    char *ptr = buffer;
    unsigned int chars_read = 0; // Number of characters successfully read into buffer (excluding delimiter)

    while (chars_read < max_len) {
        int bytes_read_single;
        int ret = receive(fd, ptr, 1, &bytes_read_single);

        if (ret != 0) { // Error from receive
            *ptr = '\0';
            return -1; // Propagate error
        }
        if (bytes_read_single == 0) { // EOF
            *ptr = '\0';
            return chars_read; // Return current length, EOF reached
        }

        if (*ptr == delimiter) {
            *ptr = '\0'; // Replace delimiter with null terminator
            return chars_read; // Return number of chars before delimiter
        }
        
        ptr++;
        chars_read++;
    }
    *ptr = '\0'; // Null-terminate if max_len reached
    return chars_read; // Return number of chars read
}

// Function: print_movie_detail
void print_movie_detail(int id, Movie *movie) {
    if (movie == NULL) {
        return;
    }

    const char *rating_str = movie_r2s(movie->rating);
    const char *genre_str = movie_g2s(movie->genre);
    
    // The original `printf` arguments: param_1,*param_2,param_2[2],local_10,local_14,param_2[3],param_2[1]
    // which map to: id, movie->title, movie->year, rating_str, genre_str, movie->review_score, movie->description
    printf("[ %d ] %s (%d) [%s] - %s [%d/100] %s\n",
           id, movie->title, movie->year, rating_str, genre_str, movie->review_score, movie->description);
    return;
}

// Function: list_movies
void list_movies(void) {
    int movie_id = 0;
    printf("\nMovies (Full)\n--------------\n");
    for (MovieNode *current_node = movies_full; current_node != NULL; current_node = current_node->next) {
        movie_id++;
        current_node->movie->print_func(movie_id, current_node->movie);
    }
    printf("--------------\n%d movie(s)\n", movie_id);

    movie_id = 0;
    printf("\nMovies (Rented)\n--------------\n");
    for (MovieNode *current_node = movies_rented; current_node != NULL; current_node = current_node->next) {
        movie_id++;
        current_node->movie->print_func(movie_id, current_node->movie);
    }
    printf("--------------\n%d movie(s)\n", movie_id);
    return;
}

// Function: rent_movie
void rent_movie(void) {
    unsigned int num_full_movies = 0;
    printf("\nMovies (Full)\n--------------\n");
    for (MovieNode *current_node = movies_full; current_node != NULL; current_node = current_node->next) {
        num_full_movies++;
        current_node->movie->print_func(num_full_movies, current_node->movie);
    }
    printf("--------------\n%d movie(s)\n", num_full_movies);

    if (num_full_movies == 0) {
        printf("[ERROR] Movie list is empty. Please try again later.\n");
        return;
    }

    char input_buffer[256];
    unsigned long movie_id_to_rent;
    while (1) { // Loop for valid movie ID input
        printf("Enter movie id: ");
        if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            return; // Error reading input
        }
        
        movie_id_to_rent = strtoul(input_buffer, NULL, 10);

        if (movie_id_to_rent == 0 || movie_id_to_rent > num_full_movies) {
            printf("[ERROR] Invalid movie id. Try again.\n");
            continue; // Ask for ID again
        }

        MovieNode *node_to_rent_from_full = movie_find_by_id(movies_full, movie_id_to_rent);
        if (node_to_rent_from_full == NULL) {
            printf("Sorry, we have some issues here. Please try again later.\n");
            return;
        }

        // Check if already rented
        if (movie_find(movies_rented, node_to_rent_from_full->movie) != NULL) {
            printf("Sorry, [%s] is already rented at this time. Please try again later.\n",
                   node_to_rent_from_full->movie->title);
            return;
        }

        // Add the existing movie pointer to the rented list
        if (movie_add(&movies_rented, node_to_rent_from_full->movie) != 0) {
            printf("[ERROR] Failed to rent. Please try again later.\n");
            return;
        }
        printf("Successfully rented [%s]! Enjoy!\n", node_to_rent_from_full->movie->title);
        return;
    }
}

// Function: return_movie
void return_movie(void) {
    unsigned int num_rented_movies = 0;
    printf("\nMovies (Rented)\n--------------\n");
    for (MovieNode *current_node = movies_rented; current_node != NULL; current_node = current_node->next) {
        num_rented_movies++;
        current_node->movie->print_func(num_rented_movies, current_node->movie);
    }
    printf("--------------\n%d movie(s)\n", num_rented_movies);

    if (num_rented_movies == 0) {
        printf("[ERROR] All the movies are in our inventory.\n");
        return;
    }

    char input_buffer[256];
    unsigned long movie_id_to_return;
    while (1) { // Loop for valid movie ID input
        printf("Enter movie id: ");
        if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            return; // Error reading input
        }
        
        movie_id_to_return = strtoul(input_buffer, NULL, 10);

        if (movie_id_to_return == 0 || movie_id_to_return > num_rented_movies) {
            printf("[ERROR] Invalid movie id. Try again.\n");
            continue; // Ask for ID again
        }

        Movie *returned_movie_ptr = NULL;
        if (movie_remove_node(&movies_rented, movie_id_to_return, &returned_movie_ptr) != 0) {
            printf("Sorry, we have some issues here. Please try again later.\n");
            return;
        }
        
        if (returned_movie_ptr == NULL) { // Should not happen if movie_remove_node returned 0
            printf("Error: Movie pointer was NULL after removal. Please try again.\n");
            return;
        }

        printf("Successfully returned [%s]! Thank you!\n", returned_movie_ptr->title);
        return;
    }
}

// Function: login
int login(void) {
    char username_input[64];
    char password_input[68]; // 0x44 -> 68 bytes

    printf("username: ");
    if (readuntil(0, username_input, sizeof(username_input) - 1, '\n') < 0) {
        return 0; // Error reading input
    }
    
    if (strcmp("admin", username_input) == 0) {
        printf("password: ");
        if (readuntil(0, password_input, sizeof(password_input) - 1, '\n') < 0) {
            return 0; // Error reading input
        }
        if (strcmp(g_password, password_input) == 0) {
            printf("\nWelcome, admin!\n");
            return 1; // Success
        } else {
            printf("[ERROR] Permission Denied: Wrong credentials\n");
        }
    } else {
        printf("[ERROR] Permission Denied: Wrong credentials\n");
    }
    return 0; // Failure
}

// Function: add_movie
void add_movie(void) {
    Movie *new_movie = NULL;
    char input_buffer[1024];
    int success = 0; // Flag to indicate overall success

    printf("Add a movie\n--------------\n");

    do { // Overall block for add_movie logic, breaks on first error
        new_movie = (Movie *)malloc(sizeof(Movie));
        if (new_movie == NULL) {
            printf("[ERROR] Failed to add a movie: Out of memory.\n");
            break;
        }
        memset(new_movie, 0, sizeof(Movie));

        // Title
        printf("Enter Title: ");
        if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) break;
        new_movie->title = (char *)malloc(strlen(input_buffer) + 1);
        if (new_movie->title == NULL) break;
        strcpy(new_movie->title, input_buffer);

        // Description
        printf("Enter Description: ");
        if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) break;
        new_movie->description = (char *)malloc(strlen(input_buffer) + 1);
        if (new_movie->description == NULL) break;
        strcpy(new_movie->description, input_buffer);

        // Year
        int year_input_status = 0; // 0: needs input, 1: valid, -1: error
        while (year_input_status == 0) {
            printf("Enter Year (1800-2015): ");
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
                year_input_status = -1;
                break;
            }
            long year_val = strtoul(input_buffer, NULL, 10);
            if (year_val >= 1800 && year_val <= 2015) {
                new_movie->year = (int)year_val;
                year_input_status = 1;
            } else {
                printf("[ERROR] Invalid year. Try again.\n");
            }
        }
        if (year_input_status == -1) break;

        // Review Score
        int score_input_status = 0;
        while (score_input_status == 0) {
            printf("Enter Review score (0-100): ");
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
                score_input_status = -1;
                break;
            }
            long score_val = strtoul(input_buffer, NULL, 10);
            if (score_val >= 0 && score_val <= 100) {
                new_movie->review_score = (int)score_val;
                score_input_status = 1;
            } else {
                printf("[ERROR] Invalid rating. Try again.\n");
            }
        }
        if (score_input_status == -1) break;

        // Genre
        int genre_input_status = 0;
        while (genre_input_status == 0) {
            printf("Select a genre\n 1. Action\n 2. Romance\n 3. Comedy\n 4. Horror\n 5. Other\nChoice: ");
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
                genre_input_status = -1;
                break;
            }
            long genre_val = strtoul(input_buffer, NULL, 10);
            if (genre_val >= 1 && genre_val <= 5) {
                new_movie->genre = (int)genre_val;
                genre_input_status = 1;
            } else {
                printf("[ERROR] Invalid genre. Try again.\n");
            }
        }
        if (genre_input_status == -1) break;

        // Film Rating
        int rating_input_status = 0;
        while (rating_input_status == 0) {
            printf("Select a film rating\n 1. G\n 2. PG\n 3. PG-13\n 4. R\n 5. Unknown\nChoice: ");
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
                rating_input_status = -1;
                break;
            }
            long rating_val = strtoul(input_buffer, NULL, 10);
            if (rating_val >= 1 && rating_val <= 5) {
                new_movie->rating = (int)rating_val;
                rating_input_status = 1;
            } else {
                printf("[ERROR] Invalid film rating. Try again.\n");
            }
        }
        if (rating_input_status == -1) break;

        new_movie->print_func = print_movie_detail;

        if (movie_add(&movies_full, new_movie) == 0) {
            printf("Successfully added the movie!\n");
            success = 1;
        } else {
            printf("[ERROR] Failed to add a movie.\n");
        }
    } while (0); // This do-while loop executes once for the main logic

    if (!success && new_movie != NULL) {
        free_movie_data(new_movie); // Free allocated memory on failure
    }
}

// Function: remove_movie
void remove_movie(void) {
    unsigned int num_full_movies = 0;
    printf("\nMovies (Full)\n--------------\n");
    for (MovieNode *current_node = movies_full; current_node != NULL; current_node = current_node->next) {
        num_full_movies++;
        current_node->movie->print_func(num_full_movies, current_node->movie);
    }
    printf("--------------\n%d movie(s)\n", num_full_movies);

    if (num_full_movies == 0) {
        printf("[ERROR] Movie list is empty.\n");
        return;
    }

    char input_buffer[256];
    unsigned long movie_id_to_remove;
    while (1) { // Loop for valid movie ID input
        printf("Enter movie id: ");
        if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            return; // Error reading input
        }
        
        movie_id_to_remove = strtoul(input_buffer, NULL, 10);

        if (movie_id_to_remove == 0 || movie_id_to_remove > num_full_movies) {
            printf("[ERROR] Invalid movie id. Try again.\n");
            continue; // Ask for ID again
        }

        MovieNode *node_to_remove_from_full = movie_find_by_id(movies_full, movie_id_to_remove);
        if (node_to_remove_from_full == NULL) { // Should not happen if id is valid
            printf("Sorry, we have some issues here. Please try again later.\n");
            return;
        }
        
        Movie *removed_movie_title_ptr = node_to_remove_from_full->movie; // Get title before delete for print

        if (movie_delete_full(&movies_full, movie_id_to_remove) != 0) {
            printf("[ERROR] Failed to remove. Please try again.\n");
            return;
        }
        printf("Successfully removed [%s]!\n", removed_movie_title_ptr->title);
        return;
    }
}

// Function: update_movie
void update_movie(void) {
    unsigned int num_full_movies = 0;
    printf("\nMovies (Full)\n--------------\n");
    for (MovieNode *current_node = movies_full; current_node != NULL; current_node = current_node->next) {
        num_full_movies++;
        current_node->movie->print_func(num_full_movies, current_node->movie);
    }
    printf("--------------\n%d movie(s)\n", num_full_movies);

    if (num_full_movies == 0) {
        printf("[ERROR] Nothing to update.\n");
        return;
    }

    char input_buffer[1024];
    unsigned long movie_id_to_update;
    Movie *movie_to_update = NULL;

    while (1) { // Outer loop for movie ID selection
        printf("Enter movie id: ");
        if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            return; // Error reading input, exit function
        }
        movie_id_to_update = strtoul(input_buffer, NULL, 10);
        if (movie_id_to_update != 0 && movie_id_to_update <= num_full_movies) {
            break; // Valid ID entered, proceed to update fields
        }
        printf("[ERROR] Invalid movie id. Try again.\n");
    }

    MovieNode *node = movie_find_by_id(movies_full, movie_id_to_update);
    if (node == NULL) { // Should not happen if ID was validated
        printf("Error: Movie not found. Please retry.\n");
        return;
    }
    movie_to_update = node->movie;

    printf("\nUpdate a movie\n--------------\nJust leave it empty to keep the old value.\n");

    char *temp_new_title = NULL;       // Temporarily hold new string allocations
    char *temp_new_description = NULL; // before they are assigned to movie_to_update
    int update_failed = 0;             // Flag for errors during the update process

    do { // Inner loop for updating fields, breaks on error
        // Title
        printf("Enter new title (current: [%s]): ", movie_to_update->title);
        if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) { update_failed = 1; break; }
        if (strlen(input_buffer) > 0) {
            temp_new_title = (char *)malloc(strlen(input_buffer) + 1);
            if (temp_new_title == NULL) { update_failed = 1; break; }
            strcpy(temp_new_title, input_buffer);
            free(movie_to_update->title);
            movie_to_update->title = temp_new_title;
            temp_new_title = NULL; // Ownership transferred
        }

        // Description
        printf("Enter new description (current: [%s]): ", movie_to_update->description);
        if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) { update_failed = 1; break; }
        if (strlen(input_buffer) > 0) {
            temp_new_description = (char *)malloc(strlen(input_buffer) + 1);
            if (temp_new_description == NULL) { update_failed = 1; break; }
            strcpy(temp_new_description, input_buffer);
            free(movie_to_update->description);
            movie_to_update->description = temp_new_description;
            temp_new_description = NULL; // Ownership transferred
        }

        // Year
        int year_input_status = 0; // 0: needs input, 1: valid/skipped, -1: error
        while (year_input_status == 0) {
            printf("Enter new year (1800-2015) (current: [%d]): ", movie_to_update->year);
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) { year_input_status = -1; break; }
            if (strlen(input_buffer) == 0) { year_input_status = 1; continue; } // Keep old value
            long year_val = strtoul(input_buffer, NULL, 10);
            if (year_val >= 1800 && year_val <= 2015) {
                movie_to_update->year = (int)year_val;
                year_input_status = 1;
            } else {
                printf("[ERROR] Invalid year. Try again.\n");
            }
        }
        if (year_input_status == -1) { update_failed = 1; break; }

        // Review Score
        int score_input_status = 0;
        while (score_input_status == 0) {
            printf("Enter new review score (0-100) (current: [%d/100]): ", movie_to_update->review_score);
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) { score_input_status = -1; break; }
            if (strlen(input_buffer) == 0) { score_input_status = 1; continue; } // Keep old value
            long score_val = strtoul(input_buffer, NULL, 10);
            if (score_val >= 0 && score_val <= 100) {
                movie_to_update->review_score = (int)score_val;
                score_input_status = 1;
            } else {
                printf("[ERROR] Invalid rating. Try again.\n");
            }
        }
        if (score_input_status == -1) { update_failed = 1; break; }

        // Genre
        int genre_input_status = 0;
        while (genre_input_status == 0) {
            printf("Select a genre (current: [%s])\n 1. Action\n 2. Romance\n 3. Comedy\n 4. Horror\n 5. Other\nChoice: ",
                   movie_g2s(movie_to_update->genre));
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) { genre_input_status = -1; break; }
            if (strlen(input_buffer) == 0) { genre_input_status = 1; continue; } // Keep old value
            long genre_val = strtoul(input_buffer, NULL, 10);
            if (genre_val >= 1 && genre_val <= 5) {
                movie_to_update->genre = (int)genre_val;
                genre_input_status = 1;
            } else {
                printf("[ERROR] Invalid genre. Try again.\n");
            }
        }
        if (genre_input_status == -1) { update_failed = 1; break; }

        // Film Rating
        int rating_input_status = 0;
        while (rating_input_status == 0) {
            printf("Select a film rating (current: [%s])\n 1. G\n 2. PG\n 3. PG-13\n 4. R\n 5. Unknown\nChoice: ",
                   movie_r2s(movie_to_update->rating));
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) { rating_input_status = -1; break; }
            if (strlen(input_buffer) == 0) { rating_input_status = 1; continue; } // Keep old value
            long rating_val = strtoul(input_buffer, NULL, 10);
            if (rating_val >= 1 && rating_val <= 5) {
                movie_to_update->rating = (int)rating_val;
                rating_input_status = 1;
            } else {
                printf("[ERROR] Invalid film rating. Try again.\n");
            }
        }
        if (rating_input_status == -1) { update_failed = 1; break; }

        printf("Successfully updated the movie information!\n");
    } while(0); // This inner do-while loop executes once for the fields update

    if (update_failed) {
        printf("[ERROR] Update failed due to memory allocation or input error.\n");
        // Free any temporarily allocated strings that might not have been assigned
        if (temp_new_title) free(temp_new_title);
        if (temp_new_description) free(temp_new_description);
    }
    return; // Exit update_movie
}

// Function: main
int main(void) {
    srandom(time(NULL)); // Seed random number generator

    const char *main_banner =
        "\n=============================================\n"
        "Movie Rental Service v1.0\n"
        "=============================================\n";

    const char *main_menu_str =
        "\n1. List movies\n"
        "2. Rent movie\n"
        "3. Return movie\n"
        "4. Admin mode\n"
        "5. Exit\n\n"
        "Choice: ";

    const char *admin_menu_str =
        "\n1. Add movie\n"
        "2. Remove movie\n"
        "3. Update movie\n"
        "4. Quit admin mode\n\n"
        "Choice: ";

    char input_buffer[4096];
    int admin_mode = 0; // 0 for normal user, 1 for admin

    printf("%s", main_banner);
    initialize();
    
    while (1) { // Main application loop
        if (admin_mode == 0) { // Normal user mode
            printf("%s", main_menu_str);
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
                quit(); // Input error, exit
            }
            switch (input_buffer[0]) {
                case '1': list_movies(); break;
                case '2': rent_movie(); break;
                case '3': return_movie(); break;
                case '4': admin_mode = login(); break;
                case '5': quit(); break;
                default: printf("[ERROR] Invalid menu. Please select again.\n"); break;
            }
        } else { // Admin mode
            printf("%s", admin_menu_str);
            if (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
                quit(); // Input error, exit
            }
            switch (input_buffer[0]) {
                case '1': add_movie(); break;
                case '2': remove_movie(); break;
                case '3': update_movie(); break;
                case '4': admin_mode = 0; break; // Quit admin mode
                default: printf("[ERROR] Invalid menu. Please select again.\n"); break;
            }
        }
    }

    return 0; // Should ideally not be reached due to quit()
}