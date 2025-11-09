#include <stdio.h>   // For fprintf, stderr, printf
#include <string.h>  // For strlen, strncpy
#include <stdlib.h>  // For EXIT_SUCCESS, EXIT_FAILURE
#include <stddef.h>  // For size_t (though included by stdio.h on many systems)

// Constants derived from the original code's memory access patterns
#define MAX_JOKES_COUNT 256 // 0x100 - Maximum number of jokes
// JOKE_ENTRY_INTS_STRIDE (0x81 = 129) is the size of one joke entry in 'int' units.
// This means each joke entry occupies 129 * sizeof(int) = 129 * 4 = 516 bytes.
#define JOKE_ENTRY_INTS_STRIDE 129

// Within each 516-byte joke entry, based on pointer arithmetic:
// The ID is stored at offset 1 * sizeof(int) from the start of the entry.
#define JOKE_ID_OFFSET_INTS 1
// The joke text starts at offset 2 * sizeof(int) from the start of the entry.
#define JOKE_TEXT_OFFSET_INTS 2

// The actual buffer size for the text within an entry:
// (JOKE_ENTRY_INTS_STRIDE - JOKE_TEXT_OFFSET_INTS) * sizeof(int)
// (129 - 2) * 4 = 127 * 4 = 508 bytes.
#define JOKE_TEXT_BUFFER_SIZE ((JOKE_ENTRY_INTS_STRIDE - JOKE_TEXT_OFFSET_INTS) * sizeof(int))
// Maximum characters that can be stored (excluding the null terminator).
// This is consistent with the derived buffer size, even if the original
// code's `0x1ff` (511) might suggest a slightly larger conceptual limit.
#define MAX_JOKE_TEXT_LEN (JOKE_TEXT_BUFFER_SIZE - 1) // 508 - 1 = 507

// Global storage for jokes.
// jokes_storage[0] holds the current joke count.
// jokes_storage[1] onwards holds the actual joke entries.
// Each entry is JOKE_ENTRY_INTS_STRIDE integers long.
static int jokes_storage[1 + MAX_JOKES_COUNT * JOKE_ENTRY_INTS_STRIDE];

// Function: insert_joke
// param_1 is expected to be a pointer to the global jokes_storage array.
int insert_joke(int *param_1, const char *param_2) {
  // param_1[0] holds the current number of jokes.
  if (param_1[0] < MAX_JOKES_COUNT) {
    size_t joke_len = strlen(param_2);

    // Check if the joke is too long for the allocated buffer.
    // The original code used 0x1ff (511) as a length check.
    // However, the memory layout (0x81 stride) implies a buffer of 508 bytes.
    // We use MAX_JOKE_TEXT_LEN (507) for safety with strncpy and null termination.
    if (joke_len <= MAX_JOKE_TEXT_LEN) {
      int current_joke_idx = param_1[0];

      // Calculate the base address for the current joke entry.
      // +1 to skip the joke count at param_1[0].
      // + (current_joke_idx * JOKE_ENTRY_INTS_STRIDE) to get to the start of the current entry.
      int *joke_entry_base = param_1 + 1 + (current_joke_idx * JOKE_ENTRY_INTS_STRIDE);

      // Store the joke index (ID) at its designated offset within the entry.
      // Original: param_1[iVar1 * 0x81 + 1] = iVar1; (relative to start of entire block)
      // This translates to joke_entry_base[JOKE_ID_OFFSET_INTS] when relative to entry base.
      joke_entry_base[JOKE_ID_OFFSET_INTS] = current_joke_idx;

      // Copy the joke text at its designated offset within the entry.
      // Original: strncpy((char *)(param_1 + iVar1 * 0x81 + 2), param_2, sVar2);
      // This translates to (char *)(joke_entry_base + JOKE_TEXT_OFFSET_INTS).
      char *joke_text_ptr = (char *)(joke_entry_base + JOKE_TEXT_OFFSET_INTS);
      strncpy(joke_text_ptr, param_2, MAX_JOKE_TEXT_LEN);
      joke_text_ptr[MAX_JOKE_TEXT_LEN] = '\0'; // Ensure null termination

      // Increment the total joke count.
      param_1[0]++;
      return 0; // Success
    } else {
      // Using fprintf to stderr as a generic logging mechanism, replacing syslog.
      fprintf(stderr, "syslog(3): Joke was too long -->\n");
      fprintf(stderr, "syslog(3): %s\n", param_2);
      return -2; // Joke too long
    }
  } else {
    return -1; // Max jokes reached
  }
}

// Function: load_default_jokes
// joke_storage_ptr is expected to be a pointer to the global jokes_storage array.
void load_default_jokes(int *joke_storage_ptr) {
  insert_joke(joke_storage_ptr, "When Alexander Bell invented the telephone he had 3 missed calls from Chuck Norris.");
  insert_joke(joke_storage_ptr, "Fear of spiders is aracnaphobia, fear of tight spaces is chlaustraphobia, fear of Chuck Norris is called Logic");
  insert_joke(joke_storage_ptr, "There used to be a street named after Chuck Norris, but it was changed because nobody crosses Chuck Norris and lives.");
  insert_joke(joke_storage_ptr, "Chuck Norris has a grizzly bear carpet in his room. The bear isn\'t dead it is just afriad to move.");
  insert_joke(joke_storage_ptr, "Chuck Norris died 20 years ago, Death just hasn\'t built up the courage to tell him yet.");
  insert_joke(joke_storage_ptr, "Chuck Norris has already been to Mars; that\'s why there are no signs of life.");
  insert_joke(joke_storage_ptr, "Chuck Norris doesn\'t call the wrong number. You answer the wrong phone.");
  insert_joke(joke_storage_ptr, "Some magicans can walk on water, Chuck Norris can swim through land.");
  insert_joke(joke_storage_ptr, "Chuck Norris and Superman once fought each other on a bet. The loser had to start wearing his underwear on the outside of his pants.");
  insert_joke(joke_storage_ptr, "Chuck Norris once urinated in a semi truck\'s gas tank as a joke....that truck is now known as Optimus Prime.");
  insert_joke(joke_storage_ptr, "Chuck Norris can cut through a hot knife with butter");
  insert_joke(joke_storage_ptr, "Chuck Norris counted to infinity - twice.");
  insert_joke(joke_storage_ptr, "Chuck Norris is the reason why Waldo is hiding.");
  insert_joke(joke_storage_ptr, "When the Boogeyman goes to sleep every night, he checks his closet for Chuck Norris.");
  insert_joke(joke_storage_ptr, "Death once had a near-Chuck Norris experience");
  insert_joke(joke_storage_ptr, "Chuck Norris can slam a revolving door.");
  insert_joke(joke_storage_ptr, "Chuck Norris once kicked a horse in the chin. Its decendants are known today as Giraffes.");
  insert_joke(joke_storage_ptr, "Chuck Norris will never have a heart attack. His heart isn\'t nearly foolish enough to attack him.");
  insert_joke(joke_storage_ptr, "Chuck Norris can win a game of Connect Four in only three moves.");
  insert_joke(joke_storage_ptr, "Chuck Norris once got bit by a rattle snake........ After three days of pain and agony ..................the rattle snake died");
  insert_joke(joke_storage_ptr, "There is no theory of evolution. Just a list of animals Chuck Norris allows to live.");
  insert_joke(joke_storage_ptr, "Chuck Norris can light a fire by rubbing two ice-cubes together.");
  insert_joke(joke_storage_ptr, "When Chuck Norris does a pushup, he isn\'t lifting himself up, he\'s pushing the Earth down.");
  insert_joke(joke_storage_ptr, "Chuck Norris is so fast, he can dodge raindrops.");
  insert_joke(joke_storage_ptr, "Chuck Norris doesn\'t wear a watch. HE decides what time it is.");
  insert_joke(joke_storage_ptr, "The original title for Alien vs. Predator was Alien and Predator vs Chuck Norris.");
  insert_joke(joke_storage_ptr, "Chuck Norris doesn\'t read books. He stares them down until he gets the information he wants.");
  insert_joke(joke_storage_ptr, "If Chuck Norris were to ever run out of ammo, his weapon would continue to fire out of fear of disappointing Chuck Norris.");
  insert_joke(joke_storage_ptr, "Chuck Norris\' hand is the only hand that can beat a Royal Flush.");
  insert_joke(joke_storage_ptr, "Chuck Norris made a Happy Meal cry.");
  insert_joke(joke_storage_ptr, "If you spell Chuck Norris in Scrabble, you win. Forever.");
  insert_joke(joke_storage_ptr, "Fear of Chuck Norris is also called common sense.");
  insert_joke(joke_storage_ptr, "Outer space exists because it\'s afraid to be on the same planet with Chuck Norris.");
  insert_joke(joke_storage_ptr, "Chuck Norris destroyed the periodic table, because Chuck Norris only recognizes the element of surprise.");
  insert_joke(joke_storage_ptr, "Some people wear Superman pajamas. Superman wears Chuck Norris pajamas.");
  insert_joke(joke_storage_ptr, "Einstein said you can\'t move at the speed of light. Obviously he was never kicked by Chuck Norris...");
  insert_joke(joke_storage_ptr, "Chuck Norris does not sleep. He waits.");
  insert_joke(joke_storage_ptr, "Chuck Norris has no shadow....nothing\'s stupid enough to follow Chuck Norris.");
  insert_joke(joke_storage_ptr, "Chuck Norris is so intimidating even his reflection won\'t look back at him.");
  insert_joke(joke_storage_ptr, "When Chuck Norris throws you into a bottomless pit, you hit the bottom.");
  insert_joke(joke_storage_ptr, "Some kids pee their name in snow. Chuck Norris pees his name in concrete.");
  insert_joke(joke_storage_ptr, "Chuck Norris has the greatest Poker-Face of all time.");
  insert_joke(joke_storage_ptr, "Chuck Norris played Russian Roulette with a fully loaded gun and won.");
  insert_joke(joke_storage_ptr, "Chuck Norris doesn\'t fold paper airplanes, he folds airplanes into paper.");
  insert_joke(joke_storage_ptr, "Chuck Norris knows the last digit of pi.");
}

// Function: joke_count
// param_1 is expected to be a pointer to the global jokes_storage array.
int joke_count(const int *param_1) {
  return param_1[0]; // The count is stored at the first element.
}

// Main function to demonstrate usage
int main() {
  // Initialize the joke count to 0 before loading.
  jokes_storage[0] = 0;

  // Load default jokes into the storage.
  load_default_jokes(jokes_storage);

  printf("Loaded %d Chuck Norris jokes.\n", joke_count(jokes_storage));

  // Example of retrieving and printing the first and last loaded jokes.
  if (joke_count(jokes_storage) > 0) {
    // Retrieve the first joke (index 0).
    int first_joke_idx = 0;
    int *first_joke_entry_base = jokes_storage + 1 + (first_joke_idx * JOKE_ENTRY_INTS_STRIDE);
    int joke_id = first_joke_entry_base[JOKE_ID_OFFSET_INTS];
    char *joke_text = (char *)(first_joke_entry_base + JOKE_TEXT_OFFSET_INTS);
    printf("\nFirst Joke (ID %d): %s\n", joke_id, joke_text);

    // Retrieve the last joke.
    int last_joke_idx = joke_count(jokes_storage) - 1;
    if (last_joke_idx >= 0) {
      int *last_joke_entry_base = jokes_storage + 1 + (last_joke_idx * JOKE_ENTRY_INTS_STRIDE);
      joke_id = last_joke_entry_base[JOKE_ID_OFFSET_INTS];
      joke_text = (char *)(last_joke_entry_base + JOKE_TEXT_OFFSET_INTS);
      printf("Last Joke (ID %d): %s\n", joke_id, joke_text);
    }
  }

  // Demonstrate handling a joke that's too long.
  char long_joke[JOKE_TEXT_BUFFER_SIZE + 10]; // Create a string longer than the buffer.
  memset(long_joke, 'A', sizeof(long_joke) - 1);
  long_joke[sizeof(long_joke) - 1] = '\0';
  printf("\nAttempting to insert a very long joke (length %zu)...\n", strlen(long_joke));
  insert_joke(jokes_storage, long_joke);

  // Demonstrate handling the case where max joke count is reached.
  printf("\nAttempting to fill up all joke slots beyond the limit...\n");
  for (int i = joke_count(jokes_storage); i < MAX_JOKES_COUNT + 5; ++i) {
      int result = insert_joke(jokes_storage, "This is a test joke to fill slots.");
      if (result == -1) {
          printf("Maximum jokes reached at %d jokes.\n", joke_count(jokes_storage));
          break;
      }
  }
  printf("Final joke count: %d\n", joke_count(jokes_storage));


  return EXIT_SUCCESS;
}