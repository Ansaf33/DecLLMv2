#include <stdbool.h> // For bool

// Assuming score_cards is an external function that returns an int score.
// The second parameter `cards` likely points to an array of card values.
// Card values: 1 for Ace, 2-10 for number cards, 10 for J, Q, K.
extern int score_cards(const char *cards);

// Use standard C types:
// undefined4 -> char (since it returns 'H' or 'S' which are char values 0x48, 0x53)
// undefined -> char
// byte -> unsigned char
// uint -> unsigned int

// Function: matchDealer
char matchDealer(int param_1, const char *param_2) {
  return (score_cards(param_2) < 17) ? 'H' : 'S';
}

// Function: neverBustAlgo
char neverBustAlgo(int param_1, const char *param_2) {
  return (score_cards(param_2) < 12) ? 'H' : 'S';
}

// Function: basicAlgo
char basicAlgo(unsigned char param_1, const char *param_2) {
  bool has_ace = false;
  for (int i = 0; i < 10 && param_2[i] != '\0'; ++i) {
    if (param_2[i] == 1) { // '\x01' is 1 for Ace
      has_ace = true;
      break;
    }
  }

  int score = score_cards(param_2);
  char result;

  if (!has_ace) { // Original `if (bVar1)` means no ace (hard hand)
    if (score < 12) {
      result = 'H';
    } else if (score < 17) { // 12 <= score < 17
      // Original complex condition simplified: (param_1 < 7) && ((score != 12) || (param_1 > 3))
      if ((param_1 < 7) && ((score != 12) || (param_1 > 3))) {
          result = 'S';
      } else {
          result = 'H';
      }
    } else { // score >= 17
      result = 'S';
    }
  } else { // Soft hand (has_ace is true)
    if (score < 18) {
      result = 'H';
    } else if (score == 18 && param_1 > 8 && param_1 < 11) { // score == 18, dealer 9 or 10
      result = 'H';
    } else {
      result = 'S';
    }
  }
  return result;
}

// Function: simpleAlgo
char simpleAlgo(unsigned char param_1, const char *param_2) {
  bool is_soft = false; // `local_14 = 0` in original meant soft hand
  for (int i = 0; i < 10 && param_2[i] != '\0'; ++i) {
    if (param_2[i] == 1) { // '\x01' is 1 for Ace
      is_soft = true;
      break;
    }
  }

  int score = score_cards(param_2);
  char result;

  if (is_soft) { // `local_14 == 0` in original
    if (score < 18) {
      result = 'H';
    } else if (score == 18 && param_1 < 7) { // Dealer up-card < 7 (2-6)
      result = 'S';
    } else if (score == 18 && param_1 > 7) { // Dealer up-card > 7 (8-A)
      result = 'H';
    } else { // score > 18
      result = 'S';
    }
  } else { // Hard hand (`local_14 != 0` in original)
    if (score < 12) {
      result = 'H';
    } else if (score < 17) { // 12 <= score < 17
      if (param_1 < 7) { // Dealer up-card < 7 (2-6)
        result = 'S';
      } else { // Dealer up-card >= 7 (7-A)
        result = 'H';
      }
    } else { // score >= 17
      result = 'S';
    }
  }
  return result;
}

// Function: superDuperAlgo
// The original `local_274` was initialized with a string of 294 characters.
// This string is `14` repetitions of `HHHHHHHHHHHHHHHHHSSSS` (17 'H's, 4 'S's).
// This represents a "Hit on <17, Stand on >=17" strategy.
// The indexing `score + (param_1 & 0xff) * 0x15` implies `0x15` (21) is the width of each dealer column block.
static const char soft_strategy_table_superduper[] =
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 0 (e.g., 2)
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 1 (e.g., 3)
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 2 (e.g., 4)
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 3 (e.g., 5)
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 4 (e.g., 6)
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 5 (e.g., 7)
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 6 (e.g., 8)
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 7 (e.g., 9)
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 8 (e.g., 10)
    "HHHHHHHHHHHHHHHHHSSSS" // Dealer 9 (e.g., A)
    // Original string had 14 repetitions, so 4 more
    "HHHHHHHHHHHHHHHHHSSSS"
    "HHHHHHHHHHHHHHHHHSSSS"
    "HHHHHHHHHHHHHHHHHSSSS"
    "HHHHHHHHHHHHHHHHHSSSS"; // Total 14 * 21 = 294 characters

char superDuperAlgo(unsigned int param_1, const char *param_2) {
  bool is_soft = false; // `local_24 = 1` initially (hard), `0` if Ace found (soft)
  for (int i = 0; i < 10 && param_2[i] != '\0'; ++i) {
    if (param_2[i] == 1) { // '\x01' is 1 for Ace
      is_soft = true;
      break;
    }
  }

  int score = score_cards(param_2);
  char result;

  // param_1 is dealer upcard (2-11). Map to 0-9 index. Clamp to 0-9.
  int dealer_idx = (param_1 >= 2 && param_1 <= 11) ? (param_1 == 11 ? 9 : param_1 - 2) : 0;
  // Ensure score is within valid bounds for table lookup (e.g., 0-20).
  int table_score_idx = (score < 0) ? 0 : (score > 20 ? 20 : score);

  int table_index = table_score_idx + dealer_idx * 21;

  if (is_soft) { // `local_24 == 0` in original
    // Use the provided soft strategy table
    result = soft_strategy_table_superduper[table_index];
  } else { // Hard hand (`local_24 != 0` in original). `local_14e` was problematic.
    // Implement a standard hard strategy directly.
    if (score < 12) {
      result = 'H';
    } else if (score == 12) {
      // Hit vs 2,3,7,8,9,10,A. Stand vs 4,5,6.
      if (param_1 >= 4 && param_1 <= 6) { // Dealer 4,5,6
        result = 'S';
      } else {
        result = 'H';
      }
    } else if (score >= 13 && score <= 16) {
      // Stand vs 2-6. Hit vs 7,8,9,10,A.
      if (param_1 >= 2 && param_1 <= 6) { // Dealer 2-6
        result = 'S';
      } else {
        result = 'H';
      }
    } else { // score >= 17
      result = 'S';
    }
  }
  return result;
}