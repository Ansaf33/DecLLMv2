#include <stddef.h> // For size_t

// Assuming these external functions exist in another compilation unit or a linked library.
// Their exact signatures are inferred from usage.
// `param_1` in the original snippet was `undefined4`, which is typically a 32-bit integer.
// Assuming movie IDs are integers for `rent_movie` and `return_movie`.
extern void add_movie(const char *title, const char *description, int year, char rating, const char *genre, const char *misc_data);
extern void rent_entry(int movie_id);
extern void return_entry(int movie_id);
extern void print_movies(int list_type); // 0 for all, 1 for owned, 2 for rented
extern void get_list_length(void);
extern void get_num_rented(void);

// External data references.
// The original code used `&DAT_000163fa`, implying DAT_000163fa is a char (or similar small type),
// and its address is passed as `const char*`. This interpretation is maintained for direct translation.
extern const char DAT_000163fa;
extern const char DAT_00016566;
extern const char DAT_00016722;

// Define a Movie structure to organize the data for clarity and reduced intermediate variables.
typedef struct {
    const char *title;
    const char *description;
    int year;
    char rating;
    const char *genre;
    const char *misc_data; // Pointer to a char (e.g., &DAT_000163fa)
} Movie;

// Function: init_movies
void init_movies(void) {
    // Array of movie data, initialized directly. This replaces numerous individual local
    // variables and the complex, stack-layout-dependent indexing logic of the original snippet.
    const Movie movies[] = {
        {
            "The Shawshank Redemption",
            "Two imprisoned men bond over a number of years, finding solace and eventual\n     redemption through acts of common decency.",
            1994, // Original: 0x7ca
            ']',  // Original: ASCII 0x5d
            "Other",
            &DAT_000163fa
        },
        {
            "The Godfather",
            "The aging patriarch of an organized crime dynasty transfers control of his clandestine\n     empire to his reluctant son.",
            1972, // Original: 0x7b4
            '\\', // Original: ASCII 0x5c
            "Other",
            &DAT_000163fa
        },
        {
            "The Dark Knight",
            "When Batman, Gordon have Harvey Dent launch an assault on the mob, they let the clown\n     out of the box, the Joker, bent on turning Gotham on itself and bringing any heroes\n     down to his level.",
            2008, // Original: 0x7d8
            'Z',  // Original: ASCII 0x5a
            "Action",
            &DAT_00016566
        },
        {
            "Inception",
            "A thief who steals corporate secrets through use of dream-sharing technology is given the\n     inverse task of planting an idea into the mind of a CEO.",
            2010, // Original: 0x7da
            'X',  // Original: ASCII 0x58
            "Action",
            &DAT_00016566
        },
        {
            "The Matrix",
            "A computer hacker learns from mysterious rebels about the true nature of his reality and\n     his role in the war against its controllers.",
            1999, // Original: 1999 (0x7cf)
            'W',  // Original: ASCII 0x57
            "Action",
            &DAT_000163fa
        },
        {
            "Modern Times",
            "The Tramp struggles to live in modern industrial society with the help of a young\n     homeless woman.",
            1936, // Original: 0x790
            'V',  // Original: ASCII 0x56
            "Comedy",
            &DAT_00016722
        },
        {
            "About Time",
            "At the age of 21, Tim discovers he can travel in time and change what happens and has\n     happened in his own life. His decision to make his world a better place by getting\n     a girlfriend turns out not to be as easy as you might think.",
            2013, // Original: 0x7dd
            'N',  // Original: ASCII 0x4e
            "Romance",
            &DAT_000163fa
        },
        {
            "Yes Man",
            "A guy challenges himself to say \"yes\" to everything for an entire year.",
            2008, // Original: 0x7d8
            'E',  // Original: ASCII 0x45
            "Comedy",
            &DAT_00016566
        },
        {
            "The Conjuring",
            "Paranormal investigators Ed and Lorraine Warren work to help a family terrorized\n     by a dark presence in their farmhouse.",
            2013, // Original: 0x7dd
            'K',  // Original: ASCII 0x4b
            "Horror",
            &DAT_000163fa
        },
        {
            "The Lion King",
            "Lion cub and future king Simba searches for his identity. His eagerness to please others\n     and penchant for testing his boundaries sometimes gets him into trouble.",
            1994, // Original: 0x7ca
            'U',  // Original: ASCII 0x55
            "Other",
            &DAT_00016722
        }
    };

    // Iterate through the array and add each movie using the structured data.
    for (size_t i = 0; i < sizeof(movies) / sizeof(movies[0]); ++i) {
        add_movie(movies[i].title, movies[i].description, movies[i].year,
                  movies[i].rating, movies[i].genre, movies[i].misc_data);
    }
}

// Function: rent_movie
void rent_movie(int movie_id) {
  rent_entry(movie_id);
}

// Function: return_movie
void return_movie(int movie_id) {
  return_entry(movie_id);
}

// Function: list_all_movies
void list_all_movies(void) {
  print_movies(0);
}

// Function: list_owned_movies
void list_owned_movies(void) {
  print_movies(1);
}

// Function: list_rented_movies
void list_rented_movies(void) {
  print_movies(2);
}

// Function: get_num_owned_movies
void get_num_owned_movies(void) {
  get_list_length();
}

// Function: get_num_rented_movies
void get_num_rented_movies(void) {
  get_num_rented();
}