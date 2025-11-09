#include <stdio.h>   // For printf, vprintf
#include <stdlib.h>  // For strtol, free, strdup
#include <string.h>  // For strlen, memcmp
#include <stdarg.h>  // For va_list

// Custom fdprintf function, assuming fd 1 is stdout
void fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    if (fd == 1) { // stdout
        vprintf(format, args);
    } else {
        // Handle other file descriptors if necessary, or just ignore/error
    }
    va_end(args);
}

// External function declarations (assuming these are defined elsewhere)
extern int readline(int fd, char *buf, int max_len);
extern void print_genres(void);
extern int get_genre(long choice);
extern void print_ratings(void);
extern int get_rating(long choice);
extern void add_movie(char *title, char *desc, int year, int score, int genre_id, int rating_id);
extern unsigned int get_list_length(void);
extern void list_owned_movies(void);
extern void delete_entry(unsigned int id);
extern void *get_entry(unsigned int id); // Returns a pointer to the movie data structure

// Function: add_new_movie
void add_new_movie(void) {
    char title_buf[1024];
    char desc_buf[1024];
    char input_buf[1024];
    long year_val = 0;   // Initialize to an invalid value to enter loop
    long score_val = 1000; // Initialize to an invalid value to enter loop
    int genre_id = 0;    // Initialize to an invalid value
    int rating_id = 0;   // Initialize to an invalid value
    long temp_choice;
    int read_len;

    fdprintf(1, "Add a movie\n");
    fdprintf(1, "--------------\n");

    fdprintf(1, "Enter new title: ");
    readline(0, title_buf, sizeof(title_buf));

    fdprintf(1, "Enter new description: ");
    readline(0, desc_buf, sizeof(desc_buf));

    // Year input loop
    do {
        fdprintf(1, "Enter new year: ");
        read_len = readline(0, input_buf, sizeof(input_buf));
        if (read_len < 0) {
            return; // Error or EOF
        }
        temp_choice = strtol(input_buf, NULL, 10);
        // Original range: (local_18 < 0x708 || (0x7df < local_18))
        // 0x708 = 1800, 0x7df = 2047
        if (temp_choice < 1800 || temp_choice > 2047) {
            fdprintf(1, "[ERROR] Invalid year. Try again.\n");
            year_val = 0; // Keep year_val invalid to re-enter loop
        } else {
            year_val = temp_choice;
        }
    } while (year_val < 1800 || year_val > 2047);

    // Review score input loop
    do {
        fdprintf(1, "Enter new review score: ");
        read_len = readline(0, input_buf, sizeof(input_buf));
        if (read_len < 0) {
            return; // Error or EOF
        }
        temp_choice = strtol(input_buf, NULL, 10);
        if (temp_choice < 0 || temp_choice > 100) {
            fdprintf(1, "[ERROR] Invalid rating. Try again.\n");
            score_val = 1000; // Keep score_val invalid to re-enter loop
        } else {
            score_val = temp_choice;
        }
    } while (score_val < 0 || score_val > 100);

    // Genre selection loop
    do {
        fdprintf(1, "Select a genre: ");
        print_genres();
        fdprintf(1, "Choice: ");
        read_len = readline(0, input_buf, sizeof(input_buf));
        if (read_len < 0) {
            return; // Error or EOF
        }
        temp_choice = strtol(input_buf, NULL, 10);
        genre_id = get_genre(temp_choice);
        if (genre_id == 0) { // 0 is an invalid genre
            fdprintf(1, "Invalid genre. Try again.\n");
        }
    } while (genre_id == 0);

    // Film rating selection loop
    do {
        fdprintf(1, "Select a film rating: ");
        print_ratings();
        fdprintf(1, "Choice: ");
        read_len = readline(0, input_buf, sizeof(input_buf));
        if (read_len < 0) {
            return; // Error or EOF
        }
        temp_choice = strtol(input_buf, NULL, 10);
        rating_id = get_rating(temp_choice);
        if (rating_id == 0) { // 0 is an invalid rating
            fdprintf(1, "Invalid film rating. Try again.\n");
        }
    } while (rating_id == 0);

    add_movie(title_buf, desc_buf, (int)year_val, (int)score_val, genre_id, rating_id);
}

// Function: remove_movie
void remove_movie(void) {
    char input_buf[1024];
    unsigned int list_len = get_list_length();
    unsigned int movie_id = 0;
    long temp_id;
    int read_len;

    list_owned_movies();

    if (list_len == 0) {
        fdprintf(1, "[ERROR] Movie list is empty\n");
    } else {
        // Movie ID selection loop
        do {
            fdprintf(1, "Enter movie id: ");
            read_len = readline(0, input_buf, sizeof(input_buf));
            if (read_len < 0) {
                return; // Error or EOF
            }
            temp_id = strtol(input_buf, NULL, 10);
            if (temp_id <= 0 || (unsigned int)temp_id > list_len) {
                fdprintf(1, "[ERROR] Invalid movie id. Try again.\n");
                movie_id = 0; // Keep movie_id invalid to re-enter loop
            } else {
                movie_id = (unsigned int)temp_id;
            }
        } while (movie_id == 0 || movie_id > list_len);
        delete_entry(movie_id);
    }
}

// Function: update_movie
void update_movie(void) {
    char input_buf[1024];
    char title_buf[1024];
    char desc_buf[1024];
    unsigned int list_len = get_list_length();
    unsigned int movie_id = 0;
    long temp_choice;
    int read_len;

    void *movie_data_block = NULL; // Pointer to the raw movie data
    // Pointers to the specific fields within movie_data_block, based on original assembly access patterns
    char **movie_str_ptrs; // Used for title, description, genre, rating string pointers
    short *year_field_ptr; // Points to the short year value
    char *score_field_ptr; // Points to the char score value

    list_owned_movies();

    if (list_len == 0) {
        fdprintf(1, "[ERROR] Nothing to update.\n");
        return;
    }

    // Movie ID selection loop
    do {
        fdprintf(1, "Enter movie id: ");
        read_len = readline(0, input_buf, sizeof(input_buf));
        if (read_len < 0) return; // Error or EOF

        temp_choice = strtol(input_buf, NULL, 10);
        if (temp_choice <= 0 || (unsigned int)temp_choice > list_len) {
            fdprintf(1, "[ERROR] Invalid movie id. Try again.\n");
            movie_id = 0; // Keep invalid
        } else {
            movie_id = (unsigned int)temp_choice;
        }
    } while (movie_id == 0 || movie_id > list_len);

    movie_data_block = get_entry(movie_id);
    // Initialize specific field pointers based on the original code's access patterns
    // Assuming movie_data_block points to a structure where fields are accessed as follows:
    movie_str_ptrs = (char **)movie_data_block;
    // Original: `*(short *)(local_24 + 2)` where `local_24` is `char**`.
    // This implies `local_24[2]` is a pointer to a short.
    year_field_ptr = (short *)movie_str_ptrs[2];
    // Original: `*(char *)((int)local_24 + 10)`. Correcting `(int)` cast to `(char*)` for 64-bit safety.
    // This implies the score is at byte offset 10 from the start of the `movie_data_block`.
    score_field_ptr = (char *)((char *)movie_data_block + 10);

    fdprintf(1, "Just leave it empty to keep the old value.\n");

    // Title update
    fdprintf(1, "Enter new title (current: [%s]): ", movie_str_ptrs[0]);
    readline(0, title_buf, sizeof(title_buf));
    if (strlen(title_buf) != 0) {
        free(movie_str_ptrs[0]);
        movie_str_ptrs[0] = strdup(title_buf);
    }

    // Description update
    fdprintf(1, "Enter new description (current: [%s]): ", movie_str_ptrs[1]);
    readline(0, desc_buf, sizeof(desc_buf));
    if (strlen(desc_buf) != 0) {
        free(movie_str_ptrs[1]);
        movie_str_ptrs[1] = strdup(desc_buf);
    }

    // Year update loop
    long current_year = (long)*year_field_ptr;
    do {
        fdprintf(1, "Enter new year (current: [%ld]): ", current_year);
        read_len = readline(0, input_buf, sizeof(input_buf));
        if (read_len < 0) return; // Error or EOF
        if (strlen(input_buf) == 0) break; // Skip update if empty

        temp_choice = strtol(input_buf, NULL, 10);
        // Original range: (local_1c < 0x708 || (0x7df < local_1c))
        // 0x708 = 1800, 0x7df = 2047
        if (temp_choice < 1800 || temp_choice > 2047) {
            fdprintf(1, "[ERROR] Invalid year. Try again.\n");
        } else {
            *year_field_ptr = (short)temp_choice; // Update the year field
            break; // Valid input, exit loop
        }
    } while (1);

    // Review score update loop
    long current_score = (long)*score_field_ptr;
    do {
        fdprintf(1, "Enter new review score (current: [%ld/100]): ", current_score);
        read_len = readline(0, input_buf, sizeof(input_buf));
        if (read_len < 0) return; // Error or EOF
        if (strlen(input_buf) == 0) break; // Skip update if empty

        temp_choice = strtol(input_buf, NULL, 10);
        // Original range: (local_20 < 0) || (100 < local_20)
        if (temp_choice < 0 || temp_choice > 100) {
            fdprintf(1, "[ERROR] Invalid rating. Try again.\n");
        } else {
            *score_field_ptr = (char)temp_choice; // Update the score field
            break; // Valid input, exit loop
        }
    } while (1);

    // Genre selection loop
    char *current_genre_str = movie_str_ptrs[3];
    do {
        fdprintf(1, "Select a genre (current: [%s]): ", current_genre_str);
        print_genres();
        fdprintf(1, "Choice: ");
        read_len = readline(0, input_buf, sizeof(input_buf));
        if (read_len < 0) return; // Error or EOF
        if (strlen(input_buf) == 0) break; // Skip update if empty

        temp_choice = strtol(input_buf, NULL, 10);
        char *genre_result = get_genre(temp_choice);
        if (genre_result == NULL) { // 0x0 is NULL
            fdprintf(1, "Invalid genre. Try again.\n");
        } else {
            movie_str_ptrs[3] = genre_result; // Update the genre field
            break; // Valid input, exit loop
        }
    } while (1);

    // Film rating selection loop
    char *current_rating_str = movie_str_ptrs[4];
    do {
        fdprintf(1, "Select a film rating (current: [%s]): ", current_rating_str);
        print_ratings();
        fdprintf(1, "Choice: ");
        read_len = readline(0, input_buf, sizeof(input_buf));
        if (read_len < 0) return; // Error or EOF
        if (strlen(input_buf) == 0) break; // Skip update if empty

        temp_choice = strtol(input_buf, NULL, 10);
        char *rating_result = get_rating(temp_choice);
        if (rating_result == NULL) { // 0x0 is NULL
            fdprintf(1, "Invalid film rating. Try again.\n");
        } else {
            movie_str_ptrs[4] = rating_result; // Update the rating field
            break; // Valid input, exit loop
        }
    } while (1);

    fdprintf(1, "Successfully updated the movie information!\n");
}

// Function: run_admin_mode
// param_mode is used to update a state variable in the caller.
// Assuming undefined4 translates to int.
int run_admin_mode(int *param_mode) {
    char input_buf[1024];
    long choice;
    int read_len;

    // Placeholder for the secret string from DAT_00016322
    // Original memcmp length 0xb means 11 characters.
    const char ADMIN_SECRET[] = "secret_admin"; // Example secret string

    fdprintf(1, "1. Add movie\n");
    fdprintf(1, "2. Remove movie\n");
    fdprintf(1, "3. Update movie\n");
    fdprintf(1, "4. Quit admin mode\n\n");
    fdprintf(1, "Choice: ");

    read_len = readline(0, input_buf, sizeof(input_buf));
    if (read_len < 0) {
        return -1; // Return 0xffffffff
    }

    choice = strtol(input_buf, NULL, 10);

    if (choice == 1) {
        add_new_movie();
    } else if (choice == 2) {
        remove_movie();
    } else if (choice == 3) {
        update_movie();
    } else if (choice == 4) {
        *param_mode = 2; // Set mode to quit admin
    } else {
        // Compare input with the secret string
        // The original code compared 0xb (11) bytes.
        if (memcmp(input_buf, ADMIN_SECRET, 11) == 0) {
            *param_mode = 0; // Set mode to 0 (perhaps a super-admin mode?)
        } else {
            fdprintf(1, "[ERROR] Invalid menu. Please select again.\n");
        }
    }
    return 0;
}