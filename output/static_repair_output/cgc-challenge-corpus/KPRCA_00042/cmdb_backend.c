#include <stdio.h>   // For printf, vprintf, vfprintf
#include <stdlib.h>  // For malloc, free, strdup
#include <string.h>  // For strcmp, memcpy, memset
#include <stdarg.h>  // For va_list in fdprintf

// Custom fdprintf function to map to standard output/error
// Original code uses fdprintf(1, ...), which implies stdout.
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret;
    if (fd == 1) { // Standard output
        ret = vprintf(format, args);
    } else if (fd == 2) { // Standard error
        ret = vfprintf(stderr, format, args);
    } else {
        ret = -1; // Or handle other fds if needed
    }
    va_end(args);
    return ret;
}

// Type definitions for clarity, assuming 32-bit system for pointer sizes
typedef unsigned int uint;
typedef int status_t; // For functions returning 0, 1, or -1 (0xffffffff)

// Struct to represent a movie entry, matching the 0x18 (24 bytes) size and field offsets.
// This layout assumes a 32-bit system where pointers are 4 bytes.
typedef struct MovieEntry {
    char *title;        // Offset 0x00
    char *description;  // Offset 0x04
    short year;         // Offset 0x08
    char score;         // Offset 0x0A
    char padding1;      // Offset 0x0B (to ensure genre is at 0x0C on 32-bit)
    char *genre;        // Offset 0x0C
    char *mpaa_rating;  // Offset 0x10
    char is_rented;    // Offset 0x14
    char padding2[3];   // Offset 0x15, total size 0x18
} MovieEntry;

// Global variables (initialized to zero by default for static storage duration)
uint g_num_genres = 0;
char *g_all_genres[10]; // Max 10 genres
uint g_num_mpaa_ratings = 0;
char *g_all_mpaa_ratings[10]; // Max 10 MPAA ratings

uint g_list_length = 0; // Number of movies currently in the database
uint g_num_rented = 0; // Number of movies currently rented

MovieEntry *g_cmdb = NULL; // Pointer to the dynamically allocated movie database (array of MovieEntry)
uint g_list_size = 0; // Current allocated capacity of g_cmdb in terms of MovieEntry count

// This seems to be a pointer to a string literal, likely a newline.
const char DAT_00016aec[] = "\n";

// Function: check_genre
char* check_genre(char *param_1) {
  for (uint i = 0; i < g_num_genres; ++i) {
    if (strcmp(param_1, g_all_genres[i]) == 0) {
      return g_all_genres[i]; // Return pointer to the genre string
    }
  }
  return NULL; // Not found
}

// Function: check_rating
char* check_rating(char *param_1) {
  for (uint i = 0; i < g_num_mpaa_ratings; ++i) {
    if (strcmp(param_1, g_all_mpaa_ratings[i]) == 0) {
      return g_all_mpaa_ratings[i]; // Return pointer to the rating string
    }
  }
  return NULL; // Not found
}

// Function: get_genre
char* get_genre(uint param_1) {
  if (param_1 >= 1 && param_1 <= g_num_genres) {
    return g_all_genres[param_1 - 1];
  }
  return NULL;
}

// Function: get_rating
char* get_rating(uint param_1) {
  if (param_1 >= 1 && param_1 <= g_num_mpaa_ratings) {
    return g_all_mpaa_ratings[param_1 - 1];
  }
  return NULL;
}

// Function: get_list_length
uint get_list_length(void) {
  return g_list_length;
}

// Function: get_num_rented
uint get_num_rented(void) {
  return g_num_rented;
}

// Forward declaration for add_entry
status_t add_entry(MovieEntry *param_1);

// Function: add_movie
status_t add_movie(char *param_1, char *param_2, short param_3, char param_4, char *param_5, char *param_6) {
  char *genre_ptr = check_genre(param_5);
  char *rating_ptr = check_rating(param_6);

  if (genre_ptr == NULL || rating_ptr == NULL) {
    return -1; // Invalid genre or rating
  }
  // 0x708 (1800) to 0x7df (2015)
  if (param_3 < 1800 || param_3 > 2015) {
    return -1; // Invalid year
  }
  // '\0' (0) to 'd' (100)
  if (param_4 < 0 || param_4 > 100) {
    return -1; // Invalid score
  }

  MovieEntry *new_movie = (MovieEntry *)malloc(sizeof(MovieEntry));
  if (new_movie == NULL) {
    return -1; // Malloc failed
  }

  new_movie->title = strdup(param_1);
  new_movie->description = strdup(param_2);
  new_movie->year = param_3;
  new_movie->score = param_4;
  new_movie->padding1 = 0; // Initialize padding
  new_movie->genre = genre_ptr;
  new_movie->mpaa_rating = rating_ptr;
  new_movie->is_rented = 0; // Not rented by default
  memset(new_movie->padding2, 0, sizeof(new_movie->padding2)); // Initialize padding

  if (add_entry(new_movie) == 0) {
    free(new_movie); // Free the temporary MovieEntry struct, its contents are copied
    return 0; // Success
  } else {
    // If add_entry failed, free the allocated movie entry and its strings
    free(new_movie->title);
    free(new_movie->description);
    free(new_movie);
    return -1;
  }
}

// Function: add_entry
status_t add_entry(MovieEntry *param_1) {
  MovieEntry *old_cmdb = g_cmdb;

  if (g_list_size == 0) {
    // Initial allocation (0x180 bytes / 0x18 bytes per entry = 16 entries)
    g_cmdb = (MovieEntry *)malloc(sizeof(MovieEntry) * 16);
    if (g_cmdb == NULL) {
      return -1;
    }
    g_list_size = 16;
  } else if (g_list_length == g_list_size) {
    // Reallocate and resize (double the size)
    // Original code: malloc(g_list_size * 0x30). 0x30 is 2 * 0x18.
    MovieEntry *new_cmdb = (MovieEntry *)malloc(g_list_size * 2 * sizeof(MovieEntry));
    if (new_cmdb == NULL) {
      // Reallocation failed, retain the old database pointer and return error
      g_cmdb = old_cmdb; // This line seems to imply that old_cmdb was the current g_cmdb.
                         // It doesn't free old_cmdb, which is correct if new_cmdb failed.
      return -1;
    }
    memcpy(new_cmdb, old_cmdb, g_list_size * sizeof(MovieEntry));
    g_list_size *= 2;
    free(old_cmdb);
    g_cmdb = new_cmdb;
  }
  
  // Add the new entry by copying its contents
  memcpy(&g_cmdb[g_list_length], param_1, sizeof(MovieEntry));
  g_list_length++;
  return 0;
}

// Function: delete_entry
status_t delete_entry(uint param_1) {
  if (g_list_length == 0) {
    return 1; // No movies to delete
  }
  if (param_1 < 1 || param_1 > g_list_length) {
    return -1; // Invalid index
  }

  uint index_to_delete = param_1 - 1;

  // Free allocated strings for the movie being deleted
  free(g_cmdb[index_to_delete].title);
  free(g_cmdb[index_to_delete].description);

  // Shift subsequent entries if not the last one
  if (param_1 < g_list_length) {
    // The number of elements to move is (g_list_length - 1) - index_to_delete.
    // Which is (g_list_length - (index_to_delete + 1)).
    memcpy(&g_cmdb[index_to_delete], &g_cmdb[index_to_delete + 1],
           (g_list_length - (index_to_delete + 1)) * sizeof(MovieEntry));
  }
  g_list_length--;
  fdprintf(1,"Successfully removed the movie!\n");
  return 0;
}

// Function: rent_entry
status_t rent_entry(uint param_1) {
  if (param_1 < 1 || param_1 > g_list_length) {
    return -1; // Invalid index
  }

  uint index = param_1 - 1;
  if (g_cmdb[index].is_rented == 0) {
    g_cmdb[index].is_rented = 1;
    g_num_rented++;
    fdprintf(1,"Successfully rented [%s]! Enjoy!\n", g_cmdb[index].title);
    return 0;
  } else {
    fdprintf(1,"Sorry, [%s] is already rented at this time. Please try again later.\n", g_cmdb[index].title);
    return 1; // Already rented
  }
}

// Function: return_entry
status_t return_entry(uint param_1) {
  // param_1 is a 1-based index among currently rented movies
  if (param_1 == 0 || param_1 > g_num_rented) {
    return -1; // Invalid index
  }

  uint current_rented_idx = 0;
  for (uint i = 0; i < g_list_length; ++i) {
    if (g_cmdb[i].is_rented != 0) {
      current_rented_idx++;
      if (current_rented_idx == param_1) {
        g_cmdb[i].is_rented = 0;
        g_num_rented--;
        fdprintf(1,"Successfully returned [%s]! Thank you!\n", g_cmdb[i].title);
        return 0;
      }
    }
  }
  return -1; // Should theoretically not be reached if param_1 is valid based on g_num_rented
}

// Function: find_entry
MovieEntry* find_entry(char *param_1) {
  for (uint i = 0; i < g_list_length; ++i) {
    if (strcmp(g_cmdb[i].title, param_1) == 0) {
      return &g_cmdb[i];
    }
  }
  return NULL; // Not found
}

// Function: get_entry
MovieEntry* get_entry(uint param_1) {
  if (param_1 >= 1 && param_1 <= g_list_length) {
    return &g_cmdb[param_1 - 1];
  }
  return NULL; // Invalid index
}

// Function: print_entry
void print_entry(MovieEntry *param_1) {
  fdprintf(1,"%s (%d, %s) - %s [%d/100]\n",
           param_1->title,
           (int)param_1->year,
           param_1->mpaa_rating,
           param_1->genre,
           (int)param_1->score);
  fdprintf(1,"  => %s\n", param_1->description);
}

// Function: print_movies
void print_movies(int param_1) {
  if (param_1 == 0 || param_1 == 1) {
    fdprintf(1,"\nMovies (Full)\n");
    fdprintf(1,"--------------\n");
    for (uint i = 0; i < g_list_length; ++i) {
      fdprintf(1,"[%d] ", i + 1);
      print_entry(&g_cmdb[i]);
    }
    fdprintf(1,"--------------\n");
    fdprintf(1,"%d movie(s)\n", g_list_length);
  }
  if (param_1 == 0 || param_1 == 2) {
    int rented_count_display = 0;
    fdprintf(1,"\nMovies (Rented)\n");
    fdprintf(1,"--------------\n");
    for (uint i = 0; i < g_list_length; ++i) {
      if (g_cmdb[i].is_rented != 0) {
        rented_count_display++;
        fdprintf(1,"[%d] ", rented_count_display);
        print_entry(&g_cmdb[i]);
      }
    }
    fdprintf(1,"--------------\n");
    fdprintf(1,"%d movie(s)\n", g_num_rented);
  }
}

// Function: print_genres
void print_genres(void) {
  for (uint i = 0; i < g_num_genres; ++i) {
    fdprintf(1,"\n %d. %s", i + 1, g_all_genres[i]);
  }
  fdprintf(1, DAT_00016aec); // Print newline
}

// Function: print_ratings
void print_ratings(void) {
  for (uint i = 0; i < g_num_mpaa_ratings; ++i) {
    fdprintf(1,"\n %d. %s", i + 1, g_all_mpaa_ratings[i]);
  }
  fdprintf(1, DAT_00016aec); // Print newline
}

// Function: dag (Add Genre)
void dag(char *param_1) {
  if (param_1 != NULL && g_num_genres < 10) {
    g_all_genres[g_num_genres++] = strdup(param_1);
  }
}

// Function: dar (Add Rating)
void dar(char *param_1) {
  if (param_1 != NULL && g_num_mpaa_ratings < 10) {
    g_all_mpaa_ratings[g_num_mpaa_ratings++] = strdup(param_1);
  }
}