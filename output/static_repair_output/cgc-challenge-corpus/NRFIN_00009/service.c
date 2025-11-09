#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h> // For srandom/random or other system calls
#include <time.h>   // For time() in srandom

// Define types from decompiled code
typedef uint8_t byte;
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4; // Assuming 4-byte integer

// Decompiled data references
static const char DAT_00013058 = '\n';
static const char DAT_0001305a = ',';
static const char DAT_00013087[] = "term";
static const char DAT_0001309c[] = "\n";
static const char DAT_0001309e[] = "100"; // Default limit
static const char DAT_000130a1[] = "balance,";

// Assuming these are the attributes for search
static const char *attributes[] = {"sid", "price", "artist", "album", "song"};
#define NUM_ATTRIBUTES (sizeof(attributes) / sizeof(attributes[0]))

// Stubs for external functions
void _terminate(int code) {
    fprintf(stderr, "Terminating with code %d\n", code);
    exit(code);
}

// Global buffer for recvline to simplify argument passing
static char recv_buffer[512];
int recvline(void) {
    if (fgets(recv_buffer, sizeof(recv_buffer), stdin) != NULL) {
        // Remove trailing newline if present
        recv_buffer[strcspn(recv_buffer, "\n")] = 0;
        return strlen(recv_buffer);
    }
    return -1; // Error or EOF
}

// Explicit transmit function
int transmit_all_impl(const char *data, size_t len) {
    printf("TRANSMIT: %.*s\n", (int)len, data);
    return 0; // Success
}

// Allocate/Deallocate wrappers for malloc/free
char* allocate(size_t size) {
    char* ptr = (char*)malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed for size %zu\n", size);
    }
    return ptr;
}

int deallocate(char* ptr) {
    if (ptr != NULL) {
        free(ptr);
        return 0; // Success
    }
    return 1; // Failure (already null or invalid pointer)
}

int str2int(const char *s) {
    if (s == NULL || *s == '\0') return 0; // Handle empty/null string
    return atoi(s);
}

// Structure to hold song search results
// This is an interpretation based on offsets in parseSearchResult
typedef struct {
    char sid[50];
    char artist[50];
    char album[50];
    char song[50];
    char price[50]; // Stored as string, converted to int when needed
} SongResult;

// Structure to hold balance information
// This is an interpretation based on offsets in getBalance and purchaseSong
typedef struct {
    int purchased_count;
    char *purchased_songs_base; // Base address of dynamically allocated SongResult array
    byte current_balance;
} BalanceInfo;

// Function: setValue
char * setValue(char *param_1, void *param_2) {
  size_t len = 0;
  char *current = param_1;
  
  // Loop until null terminator or DAT_00013058 (newline) or max length 0x32 (50)
  for (len = 0; (*current != '\0' && (*current != DAT_00013058)) && (len < 0x32); len++) {
    current++;
  }
  memcpy(param_2, param_1, len);
  // Ensure null termination for the copied string, assuming param_2 is a char buffer
  ((char*)param_2)[len] = '\0'; 
  
  // Return pointer after the delimiter or null terminator.
  return current + 1;
}

// Function: parseResultSize
int parseResultSize(char *param_1) {
  char token_buffer[20 + 1]; // Max 0x14 characters + null terminator
  char value_buffer[50 + 1]; // Max 0x32 characters + null terminator
  size_t len = 0;
  char *current_pos = param_1;
  
  memset(token_buffer, 0, sizeof(token_buffer));

  // Extract token up to DAT_0001305a (comma) or null, max 0x14 chars
  for (len = 0; (*current_pos != '\0' && (*current_pos != DAT_0001305a)) && (len < 0x14); len++) {
    current_pos++;
  }
  memcpy(token_buffer, param_1, len);
  token_buffer[len] = '\0'; // Ensure null termination

  if (strcmp(token_buffer, "NUMBER") == 0) {
    if (*current_pos == DAT_0001305a) current_pos++; // Skip the delimiter (DAT_0001305a)
    // setValue copies the actual number string into value_buffer
    setValue(current_pos, value_buffer);
    return str2int(value_buffer);
  }
  return 0;
}

// Function: parseBalanceResult
int parseBalanceResult(char *param_1) {
  char token_buffer[20 + 1]; // Max 0x14 characters + null terminator
  char value_buffer[50 + 1]; // Max 0x32 characters + null terminator
  size_t len = 0;
  char *current_pos = param_1;
  
  memset(token_buffer, 0, sizeof(token_buffer));

  // Extract token up to DAT_0001305a (comma) or null, max 0x14 chars
  for (len = 0; (*current_pos != '\0' && (*current_pos != DAT_0001305a)) && (len < 0x14); len++) {
    current_pos++;
  }
  memcpy(token_buffer, param_1, len);
  token_buffer[len] = '\0'; // Ensure null termination

  if (strcmp(token_buffer, "BALANCE") == 0) {
    if (*current_pos == DAT_0001305a) current_pos++; // Skip the delimiter (DAT_0001305a)
    // setValue copies the actual balance string into value_buffer
    setValue(current_pos, value_buffer);
    int balance = str2int(value_buffer);
    // Original check: (iVar1 < 0) || (0xff < iVar1)
    if (balance < 0 || balance > 0xff) { // Balance limited to 0-255
      return 0;
    }
    return balance;
  }
  return 0;
}

// Function: parseSearchResult
int parseSearchResult(char *param_1, void *param_2) {
  char key_buffer[20 + 1]; // Max 0x14 characters + null terminator
  char *current_key_start = param_1;
  char *current_value_start;
  size_t key_len;
  const char *delimiter = "=";
  
  key_len = 0;
  current_value_start = current_key_start;
  // Find the '=' delimiter for the key
  for (key_len = 0; (*current_value_start != '\0' && (*current_value_start != *delimiter)) && (key_len < 0x14); key_len++) {
    current_value_start++;
  }

  // If no '=' found, or empty key, return error
  if (*current_value_start != *delimiter || key_len == 0) {
      return 1; // Error or no valid key-value pair
  }

  memcpy(key_buffer, current_key_start, key_len);
  key_buffer[key_len] = '\0'; // Null terminate the key

  current_value_start++; // Move past '='

  // Use the SongResult structure for param_2
  SongResult *result = (SongResult *)param_2;

  if (strcmp(key_buffer, "sid") == 0) {
    setValue(current_value_start, result->sid);
  } else if (strcmp(key_buffer, "price") == 0) {
    setValue(current_value_start, result->price);
  } else if (strcmp(key_buffer, "artist") == 0) {
    setValue(current_value_start, result->artist);
  } else if (strcmp(key_buffer, "album") == 0) {
    setValue(current_value_start, result->album);
  } else if (strcmp(key_buffer, "song") == 0) {
    setValue(current_value_start, result->song);
  } else {
    return 1; // Unknown key, return error
  }
  
  return 0; // Success
}

// Function: receiveBalance
int receiveBalance(void) { // param_1 was unused
  int bytes_read = recvline();
  if (bytes_read < 0) {
    _terminate(2); // Error reading
  }
  if (bytes_read == 0) {
    return 0; // No data received
  }
  return parseBalanceResult(recv_buffer); // Pass the global buffer
}

// Function: receiveNumResults
int receiveNumResults(void) { // param_1 was unused
  int bytes_read = recvline();
  if (bytes_read < 0) {
    _terminate(2); // Error reading
  }
  if (bytes_read == 0) {
    return 0; // No data received
  }
  int num_results = parseResultSize(recv_buffer);
  if (num_results < 1) {
    return 0;
  }
  return num_results;
}

// Function: receiveSearchResults
// result_info is assumed to be an array: [current_count, (char*)results_base_address]
int receiveSearchResults(int *result_info, uint max_results) {
  SongResult *results_base = (SongResult *)(intptr_t)result_info[1]; // Base address of results array
  int num_to_receive = receiveNumResults();
  
  while (num_to_receive > 0) {
    int bytes_read = recvline();
    if (bytes_read < 0) {
      _terminate(2); // Error reading
    }
    if (bytes_read == 0) {
      break; // No more data
    }
    
    if (result_info[0] < max_results) { // Check if current count is less than max allowed
      SongResult *current_result_slot = &results_base[result_info[0]];
      
      // Clear the current result slot before parsing
      memset(current_result_slot, 0, sizeof(SongResult)); // 0xfa bytes
      
      int parse_status = parseSearchResult(recv_buffer, current_result_slot);
      if (parse_status != 0) { // parseSearchResult returns 0 on success
        _terminate(10); // Error parsing search result
      }
      result_info[0]++; // Increment the count of received results
    }
    num_to_receive--;
  }
  return 0;
}

// Function: createSearchString
// Returns a dynamically allocated string containing the search request template.
// The template will have placeholders for term, attribute, and limit.
char* createSearchString(void) {
  // Max size calculation:
  // "term," + "%s" + "\n" + "attribute," + "%s" + "\n" + "limit," + "%s" + "\n" + "\0"
  // Total: 5 + 2 + 1 + 10 + 2 + 1 + 6 + 2 + 1 + 1 = 33 bytes.
  // Original allocated 0x89 (137) bytes, which is generous.
  size_t buffer_size = 0x89; 
  char *search_string = allocate(buffer_size);
  if (search_string == NULL) {
    _terminate(3); // Memory allocation failure
  }
  
  memset(search_string, 0, buffer_size);
  
  // Build the template string "term,%s\nattribute,%s\nlimit,%s\n"
  strcat(search_string, DAT_00013087); // "term"
  strcat(search_string, &DAT_0001305a); // ","
  strcat(search_string, "%s"); // Placeholder for term value
  strcat(search_string, &DAT_00013058); // "\n"
  
  strcat(search_string, "attribute");
  strcat(search_string, &DAT_0001305a); // ","
  strcat(search_string, "%s"); // Placeholder for attribute value
  strcat(search_string, &DAT_00013058); // "\n"
  
  strcat(search_string, "limit");
  strcat(search_string, &DAT_0001305a); // ","
  strcat(search_string, "%s"); // Placeholder for limit value (string "100")
  strcat(search_string, DAT_0001309c); // "\n"
  
  return search_string;
}

// Function: getRandomNumber
uint getRandomNumber(uint limit) {
  // Initialize random seed once
  static int seeded = 0;
  if (!seeded) {
      srandom(time(NULL));
      seeded = 1;
  }
  if (limit == 0) return 0; // Avoid division by zero
  return (uint)random() % limit;
}

// Function: getRandomAttribute
void getRandomAttribute(char *buffer) {
  int index = getRandomNumber(NUM_ATTRIBUTES);
  const char *attribute_str = attributes[index];
  strcpy(buffer, attribute_str); // Copy attribute string to buffer
}

// Function: getRandomTerm
void getRandomTerm(char *buffer) {
  const char *char_set = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  size_t char_set_len = strlen(char_set);
  uint term_len = 0;
  while (term_len == 0) { // Ensure term length is not zero
    term_len = getRandomNumber(0x32); // Max 0x32 (50) characters
  }
  
  for (uint i = 0; i < term_len; i++) {
    int char_idx = getRandomNumber(char_set_len);
    buffer[i] = char_set[char_idx];
  }
  buffer[term_len] = '\0'; // Null terminate the term
}

// Function: createRandomRequest
void createRandomRequest(char *buffer) {
  // buffer is assumed to be `char request_data[104];` from main.
  // 0x68 (104) bytes total.
  // term: buffer[0]
  // attribute: buffer[0x32] (50)
  // limit: buffer[100]
  memset(buffer, 0, 0x68);
  getRandomTerm(buffer); // Puts term at buffer[0]
  getRandomAttribute(buffer + 0x32); // Puts attribute at buffer[50]
  strcpy(buffer + 100, DAT_0001309e); // Puts "100" at buffer[100]
}

// Function: getRandomGiftCard
// Returns a dynamically allocated string for the gift card.
char* getRandomGiftCard(void) {
  size_t card_len = 0x20; // 32 characters
  char *gift_card_str = allocate(card_len + 1); // 0x21 bytes
  if (gift_card_str == NULL) {
    _terminate(3); // Memory allocation failure
  }
  
  memset(gift_card_str, 0, card_len + 1);
  
  const char *char_set = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  size_t char_set_len = strlen(char_set);
  
  for (size_t i = 0; i < card_len; i++) {
    int char_idx = getRandomNumber(char_set_len);
    gift_card_str[i] = char_set[char_idx];
  }
  gift_card_str[card_len] = '\0'; // Null terminate
  
  return gift_card_str;
}

// Function: purchaseSong
int purchaseSong(char *song_data, BalanceInfo *balance_info) {
  // Price is at offset 200 within song_data (SongResult.price)
  int price = str2int(((SongResult*)song_data)->price);
  
  if (balance_info->current_balance < price) {
    return 1; // Insufficient funds
  } else {
    balance_info->current_balance -= price;
    balance_info->purchased_count++;
    
    // Copy song data to purchased_songs_base + (purchased_count - 1) * sizeof(SongResult)
    // sizeof(SongResult) is 0xfa (250) bytes.
    SongResult *target_slot = (SongResult *)(balance_info->purchased_songs_base + (balance_info->purchased_count - 1) * 0xfa);
    memcpy(target_slot, song_data, 0x96); // Copy first 0x96 (150) bytes (sid, artist, album, song)
                                         // Original copies only up to song (150 bytes), not price.
    
    // Transmit a newline character as confirmation
    transmit_all_impl(DAT_0001309c, strlen(DAT_0001309c));
    
    return 0; // Success
  }
}

// Function: sendSearchString
int sendSearchString(char *formatted_search_string) {
  size_t len = strlen(formatted_search_string);
  int transmit_status = transmit_all_impl(formatted_search_string, len);
  if (transmit_status != 0) {
    _terminate(10); // Error transmitting
  }
  return 0;
}

// Function: getBalance
int getBalance(BalanceInfo *balance_info) {
  char *gift_card_str = getRandomGiftCard(); // Get a random gift card string
  char *request_str = NULL;

  // Max length for "balance," + gift_card_str (32 chars) + "\n" + "\0" = 8 + 32 + 1 + 1 = 42 bytes.
  request_str = allocate(42);
  if (request_str == NULL) {
    deallocate(gift_card_str);
    _terminate(3); // Memory allocation failure
  }
  memset(request_str, 0, 42);
  
  // Build the request string: "balance," + gift_card_str + "\n"
  strcpy(request_str, DAT_000130a1);
  strcat(request_str, gift_card_str);
  strcat(request_str, DAT_0001309c);

  int transmit_status = transmit_all_impl(request_str, strlen(request_str));
  if (transmit_status != 0) {
    deallocate(gift_card_str);
    deallocate(request_str);
    _terminate(10); // Error transmitting
  }

  deallocate(gift_card_str); // Free the gift card string
  deallocate(request_str); // Free the request string

  balance_info->current_balance = (byte)receiveBalance(); // Get balance value
  
  // Allocate buffer for purchased songs
  // Size: current_balance * 0xfa (250) bytes
  balance_info->purchased_songs_base = allocate((size_t)balance_info->current_balance * 0xfa);
  if (balance_info->purchased_songs_base == NULL) {
    _terminate(3); // Memory allocation failure
  }
  
  balance_info->purchased_count = 0; // Initialize purchased count
  return 0;
}

// Function: selectSong
// Selects a song from the results buffer based on index.
// Returns a pointer to the selected SongResult.
char* selectSong(uint result_count, SongResult *results_buffer, uint selected_index) {
  if (selected_index < result_count) {
    return (char*)&results_buffer[selected_index]; // Return pointer to the selected song
  }
  return NULL; // Invalid index
}

// Function: main
int main(void) {
  // Initialize random seed
  srandom(time(NULL));

  // local_a70 (SongResult local_a70[625]; total 156250 bytes) -> dynamically allocated
  SongResult *all_search_results = NULL; 
  // local_ac (char local_ac[104];) -> buffer for request data
  char random_request_data[104]; 
  
  // local_3c (uint selected_index)
  uint selected_index = 0; // Default to first song if multiple results
  
  // local_38 (BalanceInfo balance_info;)
  BalanceInfo balance_info;
  balance_info.purchased_count = 0;
  balance_info.purchased_songs_base = NULL;
  balance_info.current_balance = 0;

  // local_2c (char* selected_song_ptr)
  char *selected_song_ptr = NULL;

  // local_28 (int max_results_to_receive)
  int max_results_to_receive = 0; 
  
  // local_44 (int actual_results_count)
  int actual_results_count = 0; 

  // Allocate a large buffer for search results
  // Max results is 0x271 (625) from original array size
  all_search_results = (SongResult*)allocate(0x271 * sizeof(SongResult));
  if (all_search_results == NULL) {
      _terminate(3);
  }
  memset(all_search_results, 0, 0x271 * sizeof(SongResult));

  // Get initial balance and allocate purchased songs buffer
  getBalance(&balance_info);
  
  // Update max_results_to_receive from initial balance
  max_results_to_receive = balance_info.current_balance;

  // Create search string template once
  char *search_template = createSearchString();
  char formatted_search_string[256]; // Buffer for the formatted string
  
  // Loop while balance is not zero
  while (balance_info.current_balance > 0) {
    actual_results_count = 0; // Reset count for new search
    
    // Create random term, attribute, limit (e.g., "100") in random_request_data
    createRandomRequest(random_request_data);
    
    // Format the search string with random data
    // term: random_request_data[0]
    // attribute: random_request_data[0x32]
    // limit: random_request_data[100]
    snprintf(formatted_search_string, sizeof(formatted_search_string),
             search_template,
             random_request_data,
             random_request_data + 0x32,
             random_request_data + 100);

    // Send the formatted search string
    sendSearchString(formatted_search_string);
    
    // Prepare result_info for receiveSearchResults: [count, base_address]
    int result_info_array[2];
    result_info_array[0] = actual_results_count; // Current count
    result_info_array[1] = (intptr_t)all_search_results; // Base address of results buffer
    
    // Receive search results into all_search_results buffer
    receiveSearchResults(result_info_array, max_results_to_receive);
    actual_results_count = result_info_array[0]; // Update actual count
    
    // Select a song (defaulting to the first result if available)
    if (actual_results_count > 0) {
        selected_song_ptr = selectSong(actual_results_count, all_search_results, selected_index);
    } else {
        selected_song_ptr = NULL; // No results to select
    }

    if (selected_song_ptr == NULL) {
        fprintf(stderr, "No song selected or available. Exiting loop.\n");
        break; // No song to purchase, break loop
    }

    // Attempt to purchase the selected song
    int purchase_status = purchaseSong(selected_song_ptr, &balance_info);
    if (purchase_status == 1) { // 1 means insufficient funds
      fprintf(stderr, "Insufficient funds to purchase song. Exiting loop.\n");
      break; // Stop purchasing if funds are low
    }
    
    // Update balance after purchase
    balance_info.current_balance = (byte)receiveBalance();
  }
  
  // Deallocate memory
  deallocate(all_search_results);
  deallocate(balance_info.purchased_songs_base);
  deallocate(search_template);

  return 0;
}