#include <stdio.h>   // For printf, stdin
#include <stdlib.h>  // For malloc, free, exit, strtoul, srandom, random
#include <string.h>  // For strlen, strcmp, strcpy, strdup, strncpy
#include <time.h>    // For time (used with srandom)
#include <unistd.h>  // For read (used as receive), STDIN_FILENO

// Type definitions for clarity and compatibility
typedef unsigned char byte;

// Movie struct definition
typedef struct Movie {
    char *title;
    char *description;
    int year;
    int review_score;
    int genre; // 1:Action, 2:Romance, 3:Comedy, 4:Horror, 5:Other
    int rating; // 1:G, 2:PG, 3:PG-13, 4:R, 5:Unknown
    void (*print_func)(int, struct Movie*); // Function pointer for printing detail
} Movie;

// Linked list node for movies
typedef struct MovieNode {
    Movie *movie;
    struct MovieNode *next;
} MovieNode;

// Global variables
char g_password[22]; // 0x16 (22) characters + null terminator
MovieNode *movies_full = NULL;
MovieNode *movies_rented = NULL;
int g_num_movies = 0; // Tracks total number of movies in full list

// Forward declarations for functions
void quit(void);
int receive(int fd, char *buf, int len, int *bytes_read_ptr);
void print_movie_detail(int id, Movie *movie);

// Default movie data (mocked based on usage in initialize)
Movie default_movie_data[] = {
    {"The Shawshank Redemption", "Two imprisoned men bond over a number of years...", 1994, 93, 1, 4, NULL},
    {"The Godfather", "The aging patriarch of an organized crime dynasty...", 1972, 92, 1, 4, NULL},
    {"The Dark Knight", "When the menace known as The Joker emerges...", 2008, 90, 1, 3, NULL},
    {"Pulp Fiction", "The lives of two mob hitmen, a boxer, a gangster and his wife...", 1994, 89, 2, 4, NULL},
    {"Forrest Gump", "The presidencies of Kennedy and Johnson, the Vietnam War...", 1994, 88, 3, 3, NULL},
    {"Inception", "A thief who steals corporate secrets through use of dream-sharing technology...", 2010, 87, 1, 3, NULL},
    {"The Matrix", "A computer hacker learns from mysterious rebels about the true nature of his reality...", 1999, 87, 1, 4, NULL},
    {"Interstellar", "A team of explorers travel through a wormhole in space...", 2014, 86, 1, 3, NULL},
    {"Gladiator", "A Roman General is betrayed and his family murdered by an emperor's corrupt son...", 2000, 86, 1, 4, NULL},
    {"The Lion King", "Lion cub and future king Simba searches for his identity.", 1994, 85, 5, 2, NULL}
};
#define NUM_DEFAULT_MOVIES (sizeof(default_movie_data) / sizeof(Movie))

// Helper function to free a Movie struct's allocated strings and the struct itself
void free_movie(Movie *movie) {
    if (movie) {
        free(movie->title);
        free(movie->description);
        free(movie);
    }
}

// Helper function to add a movie to a linked list
// Returns 0 on success, -1 on failure
int movie_add(MovieNode **head, Movie *new_movie) {
    if (!new_movie) return -1;

    MovieNode *new_node = (MovieNode *)malloc(sizeof(MovieNode));
    if (!new_node) {
        free_movie(new_movie); // Free movie if node allocation fails
        return -1;
    }
    new_node->movie = new_movie;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        MovieNode *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
    g_num_movies++; // Increment global movie count
    return 0;
}

// Helper function to find a movie by its ID (1-based index) in a linked list
// Returns the Movie* pointer on success, NULL on failure
Movie *movie_find_by_id(MovieNode *head, unsigned long id) {
    if (id == 0) return NULL;
    MovieNode *current = head;
    unsigned long count = 1;
    while (current != NULL) {
        if (count == id) {
            return current->movie;
        }
        current = current->next;
        count++;
    }
    return NULL;
}

// Helper function to find a movie by its title in a linked list
// Returns the Movie* pointer on success, NULL on failure
Movie *movie_find(MovieNode *head, const char *title) {
    MovieNode *current = head;
    while (current != NULL) {
        if (current->movie && current->movie->title && strcmp(current->movie->title, title) == 0) {
            return current->movie;
        }
        current = current->next;
    }
    return NULL;
}

// Helper function to delete a movie node by ID (1-based index) from a linked list
// Returns 0 on success, -1 on failure
int movie_delete(MovieNode **head, unsigned long id) {
    if (id == 0 || *head == NULL) return -1;

    MovieNode *current = *head;
    MovieNode *prev = NULL;
    unsigned long count = 1;

    while (current != NULL && count != id) {
        prev = current;
        current = current->next;
        count++;
    }

    if (current == NULL) { // Not found
        return -1;
    }

    if (prev == NULL) { // First node
        *head = current->next;
    } else {
        prev->next = current->next;
    }
    free_movie(current->movie); // Free the movie data
    free(current); // Free the node itself
    g_num_movies--; // Decrement global movie count
    return 0;
}

// Helper functions for genre and rating to string conversion
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

// Function: print_movie_detail
void print_movie_detail(int id, Movie *movie) {
    if (movie == NULL) {
        return;
    }

    const char *rating_str = movie_r2s(movie->rating);
    const char *genre_str = movie_g2s(movie->genre);

    // Reconstructed format string based on original fragments and printf arguments
    printf("[ %d] %s (%d, %s) - %s [ %d/100] %s\n",
           id, movie->title, movie->year, rating_str, genre_str, movie->review_score, movie->description);
}

// Function: receive (mocked with read from stdin)
int receive(int fd, char *buf, int len, int *bytes_read_ptr) {
    ssize_t bytes_read = read(fd, buf, len);
    if (bytes_read == -1) {
        perror("read"); // Print error message for debugging
        *bytes_read_ptr = 0;
        return -1; // Error
    }
    *bytes_read_ptr = (int)bytes_read;
    return 0; // Success
}

// Function: readuntil
int readuntil(int fd, char *buffer, unsigned int max_len, char delimiter) {
    char *current_pos = buffer;
    unsigned int bytes_read_total = 0;
    int single_byte_read_count;

    while (bytes_read_total < max_len) {
        if (receive(fd, current_pos, 1, &single_byte_read_count) == -1 || single_byte_read_count == 0) {
            // Error or EOF
            break;
        }

        if (*current_pos == delimiter) {
            break;
        }

        current_pos++;
        bytes_read_total++;
    }

    // Null-terminate the string.
    *current_pos = '\0';
    return (int)(current_pos - buffer); // Return number of characters read, excluding delimiter
}

// Function: initialize
void initialize(void) {
    printf("\n   . . . Initializing the inventory . . .\n         (movie info from IMDb.com)\n");

    srandom(time(NULL)); // Seed the random number generator

    for (int i = 0; i < 21; ++i) { // 0x16 is 22. Loop 0 to 20 for 21 characters.
        long rand_val = random();
        if ((rand_val % 2) == 0) { // Randomly choose lowercase or uppercase
            g_password[i] = (char)(rand_val % 26) + 'a';
        } else {
            g_password[i] = (char)(rand_val % 26) + 'A';
        }
    }
    g_password[21] = '\0'; // Null-terminate g_password

    for (unsigned int i = 0; i < NUM_DEFAULT_MOVIES; ++i) {
        Movie *current_movie = (Movie *)malloc(sizeof(Movie));
        if (current_movie == NULL) {
            printf("[ERROR] Initialization failed: malloc Movie. Exit.\n");
            quit();
        }

        current_movie->title = strdup(default_movie_data[i].title);
        current_movie->description = strdup(default_movie_data[i].description);

        if (current_movie->title == NULL || current_movie->description == NULL) {
            printf("[ERROR] Initialization failed: strdup. Exit.\n");
            free_movie(current_movie); // Free what was allocated
            quit();
        }

        current_movie->year = default_movie_data[i].year;
        current_movie->review_score = default_movie_data[i].review_score;
        current_movie->genre = default_movie_data[i].genre;
        current_movie->rating = default_movie_data[i].rating;
        current_movie->print_func = print_movie_detail;

        if (movie_add(&movies_full, current_movie) != 0) {
            printf("[ERROR] Initialization failed: movie_add. Exit.\n");
            free_movie(current_movie); // Ensure movie is freed if adding to list fails
            quit();
        }
    }
}

// Function: list_movies
void list_movies(void) {
    int movie_id_counter;

    movie_id_counter = 0;
    printf("\nMovies (Full)\n--------------\n");
    for (MovieNode *current_node = movies_full; current_node != NULL; current_node = current_node->next) {
        movie_id_counter++;
        current_node->movie->print_func(movie_id_counter, current_node->movie);
    }
    printf("--------------\n%d movie(s)\n", movie_id_counter);

    movie_id_counter = 0; // Reset counter for rented movies
    printf("\nMovies (Rented)\n--------------\n");
    for (MovieNode *current_node = movies_rented; current_node != NULL; current_node = current_node->next) {
        movie_id_counter++;
        current_node->movie->print_func(movie_id_counter, current_node->movie);
    }
    printf("--------------\n%d movie(s)\n", movie_id_counter);
}

// Function: rent_movie
void rent_movie(void) {
    char input_buffer[256];
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

    while (1) {
        printf("Enter movie id: ");
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            return; // Error or EOF
        }

        unsigned long movie_id = strtoul(input_buffer, NULL, 10);

        if (movie_id != 0 && movie_id <= num_full_movies) {
            Movie *movie_to_rent = movie_find_by_id(movies_full, movie_id);
            if (movie_to_rent == NULL) {
                printf("Sorry, we have some issues here. Please try again later.\n");
                return;
            }

            if (movie_find(movies_rented, movie_to_rent->title) != NULL) {
                printf("Sorry, [%s] is already rented at this time. Please try again later.\n", movie_to_rent->title);
                return;
            }
            
            // Create a deep copy of the movie to add to the rented list
            Movie *rented_movie_copy = (Movie *)malloc(sizeof(Movie));
            if (rented_movie_copy == NULL) {
                printf("[ERROR] Failed to rent: malloc. Please try again later.\n");
                return;
            }
            rented_movie_copy->title = strdup(movie_to_rent->title);
            rented_movie_copy->description = strdup(movie_to_rent->description);
            if (rented_movie_copy->title == NULL || rented_movie_copy->description == NULL) {
                printf("[ERROR] Failed to rent: strdup. Please try again later.\n");
                free_movie(rented_movie_copy);
                return;
            }
            rented_movie_copy->year = movie_to_rent->year;
            rented_movie_copy->review_score = movie_to_rent->review_score;
            rented_movie_copy->genre = movie_to_rent->genre;
            rented_movie_copy->rating = movie_to_rent->rating;
            rented_movie_copy->print_func = movie_to_rent->print_func;


            if (movie_add(&movies_rented, rented_movie_copy) != 0) {
                printf("[ERROR] Failed to rent. Please try again later.\n");
                free_movie(rented_movie_copy); // Free the copy if adding fails
                return;
            }
            printf("Successfully rented [%s]! Enjoy!\n", rented_movie_copy->title);
            return;
        }
        printf("[ERROR] Invalid movie id. Try again.\n");
    }
}

// Function: return_movie
void return_movie(void) {
    char input_buffer[256];
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

    while (1) {
        printf("Enter movie id: ");
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            return; // Error or EOF
        }

        unsigned long movie_id = strtoul(input_buffer, NULL, 10);

        if (movie_id != 0 && movie_id <= num_rented_movies) {
            // Get the movie title *before* deleting, as deletion frees the movie struct.
            Movie *movie_to_return = movie_find_by_id(movies_rented, movie_id);
            if (movie_to_return == NULL) {
                printf("Sorry, we have some issues here. Please try again later.\n");
                return;
            }
            char movie_title_copy[256]; // Temporary buffer for title
            strncpy(movie_title_copy, movie_to_return->title, sizeof(movie_title_copy) - 1);
            movie_title_copy[sizeof(movie_title_copy) - 1] = '\0';

            if (movie_delete(&movies_rented, movie_id) != 0) {
                printf("[ERROR] Failed to return the movie. Please try again.\n");
                return;
            }
            printf("Successfully returned [%s]! Thank you!\n", movie_title_copy);
            return;
        }
        printf("[ERROR] Invalid movie id. Try again.\n");
    }
}

// Function: login
int login(void) {
    char username_input[64];
    char password_input[68]; // 0x44 = 68
    const char admin_user[] = "admin";

    printf("username: ");
    readuntil(STDIN_FILENO, username_input, sizeof(username_input) - 1, '\n');

    if (strcmp(admin_user, username_input) == 0) {
        printf("password: ");
        readuntil(STDIN_FILENO, password_input, sizeof(password_input) - 1, '\n');
        if (strcmp(g_password, password_input) == 0) {
            printf("\nWelcome, admin!\n");
            return 1; // Logged in
        } else {
            printf("[ERROR] Permission Denied: Wrong credentials\n");
        }
    } else {
        printf("[ERROR] Permission Denied: Wrong credentials\n");
    }
    return 0; // Not logged in
}

// Function: add_movie
void add_movie(void) {
    char input_buffer[1024];
    Movie *new_movie = (Movie *)malloc(sizeof(Movie));
    if (new_movie == NULL) {
        printf("[ERROR] Failed to add a movie: malloc Movie.\n");
        return;
    }
    // Initialize pointers to NULL to safely free later if strdup fails
    new_movie->title = NULL;
    new_movie->description = NULL;

    printf("Add a movie\n--------------\n");

    printf("Enter Title: ");
    if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
        printf("[ERROR] Failed to read title.\n");
        free_movie(new_movie);
        return;
    }
    new_movie->title = strdup(input_buffer);
    if (new_movie->title == NULL) {
        printf("[ERROR] Failed to add a movie: strdup title.\n");
        free_movie(new_movie);
        return;
    }

    printf("Enter Description: ");
    if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
        printf("[ERROR] Failed to read description.\n");
        free_movie(new_movie);
        return;
    }
    new_movie->description = strdup(input_buffer);
    if (new_movie->description == NULL) {
        printf("[ERROR] Failed to add a movie: strdup description.\n");
        free_movie(new_movie);
        return;
    }

    while (1) {
        printf("Enter Year (1800-2015): ");
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            printf("[ERROR] Failed to read year.\n");
            free_movie(new_movie);
            return;
        }
        unsigned long year_val = strtoul(input_buffer, NULL, 10);
        if (year_val >= 1800 && year_val <= 2015) {
            new_movie->year = (int)year_val;
            break;
        }
        printf("[ERROR] Invalid year. Try again.\n");
    }

    while (1) {
        printf("Enter Review score (0-100): ");
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            printf("[ERROR] Failed to read review score.\n");
            free_movie(new_movie);
            return;
        }
        unsigned long score_val = strtoul(input_buffer, NULL, 10);
        if (score_val <= 100) { // Score must be 0-100
            new_movie->review_score = (int)score_val;
            break;
        }
        printf("[ERROR] Invalid rating. Try again.\n");
    }

    while (1) {
        printf("Select a genre\n 1. Action\n 2. Romance\n 3. Comedy\n 4. Horror\n 5. Other\nChoice: ");
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            printf("[ERROR] Failed to read genre.\n");
            free_movie(new_movie);
            return;
        }
        unsigned long genre_val = strtoul(input_buffer, NULL, 10);
        if (genre_val >= 1 && genre_val <= 5) {
            new_movie->genre = (int)genre_val;
            break;
        }
        printf("[ERROR] Invalid genre. Try again.\n");
    }

    while (1) {
        printf("Select a film rating\n 1. G\n 2. PG\n 3. PG-13\n 4. R\n 5. Unknown\nChoice: ");
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            printf("[ERROR] Failed to read film rating.\n");
            free_movie(new_movie);
            return;
        }
        unsigned long rating_val = strtoul(input_buffer, NULL, 10);
        if (rating_val >= 1 && rating_val <= 5) {
            new_movie->rating = (int)rating_val;
            break;
        }
        printf("[ERROR] Invalid film rating. Try again.\n");
    }

    new_movie->print_func = print_movie_detail;

    if (movie_add(&movies_full, new_movie) != 0) {
        printf("[ERROR] Failed to add a movie to the list.\n");
        free_movie(new_movie); // Free movie if adding to list fails
        return;
    }
    printf("Successfully added [%s]!\n", new_movie->title);
}

// Function: remove_movie
void remove_movie(void) {
    char input_buffer[256];
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

    while (1) {
        printf("Enter movie id: ");
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            return; // Error or EOF
        }

        unsigned long movie_id = strtoul(input_buffer, NULL, 10);

        if (movie_id != 0 && movie_id <= num_full_movies) {
            // Get the movie title *before* deleting, as deletion frees the movie struct.
            Movie *movie_to_remove = movie_find_by_id(movies_full, movie_id);
            if (movie_to_remove == NULL) {
                printf("Sorry, we have some issues here. Please try again later.\n");
                return;
            }
            char movie_title_copy[256]; // Temporary buffer for title
            strncpy(movie_title_copy, movie_to_remove->title, sizeof(movie_title_copy) - 1);
            movie_title_copy[sizeof(movie_title_copy) - 1] = '\0';

            if (movie_delete(&movies_full, movie_id) != 0) {
                printf("[ERROR] Failed to remove. Please try again.\n");
                return;
            }
            printf("Successfully removed [%s]!\n", movie_title_copy);
            return;
        }
        printf("[ERROR] Invalid movie id. Try again.\n");
    }
}

// Function: update_movie
void update_movie(void) {
    char input_buffer[1024];
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

    Movie *movie_to_update = NULL;
    while (1) {
        printf("Enter movie id: ");
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
            return; // Error or EOF
        }

        unsigned long movie_id = strtoul(input_buffer, NULL, 10);

        if (movie_id != 0 && movie_id <= num_full_movies) {
            movie_to_update = movie_find_by_id(movies_full, movie_id);
            if (movie_to_update == NULL) {
                printf("Sorry, we have some issues here. Please try again later.\n");
                return;
            }
            break;
        }
        printf("[ERROR] Invalid movie id. Try again.\n");
    }

    printf("\nUpdate a movie\n--------------\nJust leave it empty to keep the old value.\n");

    // Update Title
    printf("Enter new title (current: [%s]): ", movie_to_update->title);
    if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) return;
    if (strlen(input_buffer) != 0) {
        char *new_title = strdup(input_buffer);
        if (new_title == NULL) {
            printf("[ERROR] Failed to update title: strdup.\n");
            return;
        }
        free(movie_to_update->title);
        movie_to_update->title = new_title;
    }

    // Update Description
    printf("Enter new description (current: [%s]): ", movie_to_update->description);
    if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) return;
    if (strlen(input_buffer) != 0) {
        char *new_description = strdup(input_buffer);
        if (new_description == NULL) {
            printf("[ERROR] Failed to update description: strdup.\n");
            return;
        }
        free(movie_to_update->description);
        movie_to_update->description = new_description;
    }

    // Update Year
    while (1) {
        printf("Enter new year (1800-2015) (current: [%d]): ", movie_to_update->year);
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) return;
        if (strlen(input_buffer) == 0) break; // Keep old value
        
        unsigned long year_val = strtoul(input_buffer, NULL, 10);
        if (year_val >= 1800 && year_val <= 2015) {
            movie_to_update->year = (int)year_val;
            break;
        }
        printf("[ERROR] Invalid year. Try again.\n");
    }

    // Update Review Score
    while (1) {
        printf("Enter new review score (0-100) (current: [%d/100]): ", movie_to_update->review_score);
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) return;
        if (strlen(input_buffer) == 0) break; // Keep old value

        unsigned long score_val = strtoul(input_buffer, NULL, 10);
        if (score_val <= 100) {
            movie_to_update->review_score = (int)score_val;
            break;
        }
        printf("[ERROR] Invalid rating. Try again.\n");
    }

    // Update Genre
    while (1) {
        printf("Select a genre (current: [%s])\n 1. Action\n 2. Romance\n 3. Comedy\n 4. Horror\n 5. Other\nChoice: ",
               movie_g2s(movie_to_update->genre));
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) return;
        if (strlen(input_buffer) == 0) break; // Keep old value
        
        unsigned long genre_val = strtoul(input_buffer, NULL, 10);
        if (genre_val >= 1 && genre_val <= 5) {
            movie_to_update->genre = (int)genre_val;
            break;
        }
        printf("[ERROR] Invalid genre. Try again.\n");
    }

    // Update Film Rating
    while (1) {
        printf("Select a film rating (current: [%s])\n 1. G\n 2. PG\n 3. PG-13\n 4. R\n 5. Unknown\nChoice: ",
               movie_r2s(movie_to_update->rating));
        if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) return;
        if (strlen(input_buffer) == 0) break; // Keep old value
        
        unsigned long rating_val = strtoul(input_buffer, NULL, 10);
        if (rating_val >= 1 && rating_val <= 5) {
            movie_to_update->rating = (int)rating_val;
            break;
        }
        printf("[ERROR] Invalid film rating. Try again.\n");
    }

    printf("Successfully updated the movie information!\n");
}

// Function: quit
void quit(void) {
    printf("Bye!\n");
    // Free all allocated movie data before exiting
    MovieNode *current;
    MovieNode *next_node;

    current = movies_full;
    while (current != NULL) {
        next_node = current->next;
        free_movie(current->movie); // Free the Movie struct and its strings
        free(current); // Free the MovieNode
        current = next_node;
    }
    movies_full = NULL;

    current = movies_rented;
    while (current != NULL) {
        next_node = current->next;
        free_movie(current->movie);
        free(current);
        current = next_node;
    }
    movies_rented = NULL;

    exit(0);
}

// Function: main
int main(void) { // main should return int
    // Menu strings
    const char *main_menu_str =
        "\n=============================================Movie Rental Service v1.0=============================================\n"
        "\n1. List movies\n2. Rent movie\n3. Return movie\n4. Admin mode\n5. Exit\n\nChoice: ";
    const char *admin_menu_str =
        "\n1. Add movie\n2. Remove movie\n3. Update movie\n4. Quit admin mode\n\nChoice: ";

    char input_buffer[4096];
    int admin_mode = 0; // 0 for user mode, 1 for admin mode

    printf("%s", main_menu_str); // Print banner + initial menu
    initialize();
    printf("=============================================\n");

    while (1) {
        if (admin_mode == 0) {
            printf("%s", main_menu_str);
            if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
                quit();
            }

            switch (input_buffer[0]) {
                case '1':
                    list_movies();
                    break;
                case '2':
                    rent_movie();
                    break;
                case '3':
                    return_movie();
                    break;
                case '4':
                    admin_mode = login();
                    break;
                case '5':
                    quit();
                    break;
                default:
                    printf("[ERROR] Invalid menu. Please select again.\n");
                    break;
            }
        } else { // admin_mode == 1
            printf("%s", admin_menu_str);
            if (readuntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n') < 0) {
                quit();
            }

            switch (input_buffer[0]) {
                case '1':
                    add_movie();
                    break;
                case '2':
                    remove_movie();
                    break;
                case '3':
                    update_movie();
                    break;
                case '4':
                    admin_mode = 0; // Quit admin mode
                    printf("Exiting admin mode.\n");
                    break;
                default:
                    printf("[ERROR] Invalid menu. Please select again.\n");
                    break;
            }
        }
    }
    return 0; // Should not be reached due to quit()
}