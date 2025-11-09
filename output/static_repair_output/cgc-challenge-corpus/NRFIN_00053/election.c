#include <stdbool.h> // For bool type
#include <stdio.h>   // For snprintf
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For strlen, memcpy, memset

// --- Mock functions and global variables (replace with actual implementations if available) ---
// These are placeholders to make the provided code compilable.
// In a real system, these would be provided by other modules or system libraries.

// Global variables, inferred from usage
int e = 1; // Represents election state (e.g., 1=init, 2=created, 4=open, 8=closed)
void *DAT_00019844; // List for candidates
void *DAT_00019848; // Hash table for voters
void *DAT_0001984c; // Hash table for votes
int DAT_00019850 = 0; // Currently logged-in user (0 for none)
int DAT_00019854 = 0x9a2112; // Election manager ID/auth key part
int DAT_00019858 = 0xbc4ff2; // Next voter ID
int DAT_0001985c = 1; // Next candidate ID
unsigned char DAT_00019860 = 0; // Number of winners desired
unsigned char DAT_00019861 = 0; // Max number of candidates allowed
unsigned char DAT_00019862 = '\x00'; // Can voters write-in new candidates?
void *DAT_00019864 = NULL; // Election manager structure pointer
unsigned int _DAT_4347c000 = 0; // Used in get_next_vote_id, likely a random seed or counter

// Dummy data for `streq` related strings
char DAT_0001633b[] = "NO ";
char DAT_0001633f[] = "YES";

// Placeholder for _terminate function
void _terminate(int status) {
  fprintf(stderr, "Program terminated with status: %d\n", status);
  exit(status);
}

// Placeholder for send_bytes function
int send_bytes(int fd, const char *buf, size_t count) {
  // In a real application, this would send data over a socket or to a file descriptor.
  // For this exercise, we'll just print to stdout.
  if (buf && count > 0) {
    fwrite(buf, 1, count, stdout);
    fflush(stdout);
  }
  return count; // Assume all bytes are sent successfully
}

// Overloaded send_bytes for calls without fd (assuming fd=1 for stdout)
int send_bytes_str(const char *buf, size_t count) {
  return send_bytes(1, buf, count);
}
// Helper for `send_bytes()` without count, assumes strlen
int send_bytes_auto(const char *buf) {
  if (!buf) return 0;
  return send_bytes(1, buf, strlen(buf));
}

// Placeholder for recv_until_delim_n function
int recv_until_delim_n(int fd, char delim, char *buffer, size_t max_len) {
  // Simulate reading from stdin for user input
  if (fgets(buffer, max_len, stdin) != NULL) {
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == delim) {
      buffer[len - 1] = '\0'; // Remove delimiter
      return len - 1;
    }
    return len;
  }
  return -1; // Error or EOF
}

// Placeholder for strtou function
unsigned long strtou(const char *nptr, char **endptr, int base) {
  return strtoul(nptr, endptr, base);
}

// Placeholder for list_length function
int list_length(void *list) {
  // Dummy implementation: always return 0 or a small number for testing
  if (list == DAT_00019844) return 1; // Example: Assume 1 candidate for testing
  return 0;
}

// Placeholder for list_head_node function
void *list_head_node(void *list) {
  // Dummy implementation
  return (void *)1; // Non-NULL dummy
}

// Placeholder for list_end_marker function
void *list_end_marker(void *list) {
  // Dummy implementation
  return (void *)2; // Different non-NULL dummy
}

// Placeholder for list_next_node function
void *list_next_node(void *node) {
  // Dummy implementation
  if (node == (void *)1) return (void *)2; // Simulate one node then end
  return NULL;
}

// Placeholder for list_find_node_with_data function
void *list_find_node_with_data(void *list, void *compare_func, void *data) {
  // Dummy implementation
  return (void *)100; // Return a dummy pointer
}

// Placeholder for list_insert_sorted function
void list_insert_sorted(void *list, int compare_func, void *data) {
  // Dummy implementation
}

// Placeholder for list_insert_at_end function
void list_insert_at_end(void *list, void *data) {
  // Dummy implementation
}

// Placeholder for list_init function
void list_init(void *list, void *free_func) {
  // Dummy implementation
}

// Placeholder for list_destroy function
void list_destroy(void *list) {
  // Dummy implementation
}

// Placeholder for streq function
int streq(const char *s1, const char *s2) {
  return strcmp(s1, s2); // 0 if equal, non-zero otherwise
}

// Placeholder for ht_pair_iter_start function
void *ht_pair_iter_start(void *ht) {
  // Dummy implementation
  return (void *)1; // Non-NULL dummy
}

// Placeholder for ht_pair_iter_next function
void *ht_pair_iter_next(void *iter) {
  // Dummy implementation
  if (iter == (void *)1) return (void *)2; // Simulate one pair then end
  return NULL;
}

// Placeholder for ht_int_init function
void *ht_int_init(int size) {
  return malloc(size); // Dummy allocation
}

// Placeholder for ht_str_init function
void *ht_str_init(int size) {
  return malloc(size); // Dummy allocation
}

// Placeholder for ht_pair_insert function
void *ht_pair_insert(void *ht, void *key_value_pair) {
  return NULL; // Dummy, no collision
}

// Placeholder for ht_is_re_hash_needed function
int ht_is_re_hash_needed(void *ht) {
  return 0; // Dummy, no rehash needed
}

// Placeholder for ht_re_hash function
void *ht_re_hash(void *ht) {
  return ht; // Dummy, return same ht
}

// Placeholder for ht_length function
int ht_length(void *ht) {
  // Dummy implementation
  if (ht == DAT_00019848) return 1; // Example: 1 voter
  if (ht == DAT_0001984c) return 1; // Example: 1 vote
  return 0;
}

// Placeholder for ht_value_as_voidp function
void *ht_value_as_voidp(void *ht, int key) {
  // Dummy implementation for login voter lookup
  if (key == 0xbc4ff2) { // Example voter ID
    // Simulate a voter structure
    static char voter_data[0x80C];
    memset(voter_data, 0, sizeof(voter_data));
    // Simulate last name at offset 0x404
    strcpy(voter_data + 0x404, "Doe");
    // Simulate voter ID at offset 0
    *(int*)voter_data = 0xbc4ff2;
    return voter_data;
  }
  return NULL;
}

// Function: print_login_menu
void print_login_menu(int param_1, int param_2) {
  if (param_1 != 1 && param_2 == 4) {
    if (send_bytes_str("14: Login\n", 10) != 10) {
      _terminate(0xfffffff7);
    }
  }
  if (param_1 != 1 && param_2 != 4) {
    if (send_bytes_str("15: Main Menu\n", 14) != 14) {
      _terminate(0xfffffff7);
    }
  }
}

// Function: print_admin_menu
void print_admin_menu(int param_1, int param_2) {
  if (param_1 == 1 && param_2 == 4) {
    if (send_bytes_str("17: Create Election\n", 20) != 20) {
      _terminate(0xfffffff7);
    }
  }
  if (param_1 == 2 && param_2 == 1) {
    if (send_bytes_str("18. Open Voting\n", 16) != 16) {
      _terminate(0xfffffff7);
    }
  }
  if (param_1 == 4 && param_2 == 1) {
    if (send_bytes_str("20. Close Voting\n", 17) != 17) {
      _terminate(0xfffffff7);
    }
  }
  if ((param_1 == 4 || param_1 == 2) && param_2 == 1) {
    if (send_bytes_str("22. Election Status\n", 20) != 20) {
      _terminate(0xfffffff7);
    }
  }
}

// Function: print_voting_menu
void print_voting_menu(int param_1, int param_2) {
  if (param_1 == 4 && param_2 == 2) {
    if (send_bytes_str("36. Vote\n", 9) != 9) {
      _terminate(0xfffffff7);
    }
  }
  if ((param_1 == 2 || param_1 == 4) && param_2 == 4) {
    if (send_bytes_str("38. Register Voter\n", 19) != 19) {
      _terminate(0xfffffff7);
    }
  }
  if ((param_1 == 4 || param_1 == 2) && (param_2 == 1 || (param_2 == 2 && DAT_00019862 == '\x01'))) {
    if (list_length(DAT_00019844) < DAT_00019861) {
      if (send_bytes_str("54. Add Candidate\n", 18) != 18) {
        _terminate(0xfffffff7);
      }
    }
  }
}

// Function: print_results_menu
void print_results_menu(int param_1, int param_2) {
  if (param_1 == 8) {
    if (param_2 == 1 || param_2 == 2) {
      if (send_bytes_str("56. Voting Results\n", 19) != 19) {
        _terminate(0xfffffff7);
      }
    }
    if (param_2 == 1) {
      if (send_bytes_str("72. Voter Turnout\n", 18) != 18) {
        _terminate(0xfffffff7);
      }
      if (send_bytes_str("88. Candidate Summary\n", 22) != 22) {
        _terminate(0xfffffff7);
      }
    }
  }
}

// Function: receive_letter
void receive_letter(char *param_1) {
  char local_e[2]; // Buffer for single char + null terminator
  int iVar1 = recv_until_delim_n(0, '\n', local_e, 2);
  if (iVar1 < 0) {
    _terminate(0xfffffff5);
  }
  *param_1 = local_e[0];
}

// Function: receive_number
void receive_number(unsigned int *param_1) {
  char local_1b[19]; // Buffer for number string
  int iVar1 = recv_until_delim_n(0, '\n', local_1b, sizeof(local_1b));
  if (iVar1 < 0) {
    _terminate(0xfffffff5);
  }
  *param_1 = (unsigned int)strtou(local_1b, NULL, 10);
}

// Function: set_first_last_name
void set_first_last_name(int param_1) {
  char buffer[0x400]; // Max size for name + null terminator
  int bytes_received;

  if (send_bytes_str("\tEnter first name: ", 19) != 19) {
    _terminate(0xfffffff7);
  }
  bytes_received = recv_until_delim_n(0, '\n', buffer, sizeof(buffer));
  if (bytes_received < 1) {
    _terminate(0xfffffff5);
  }
  // Copy to param_1 + 4
  strncpy((char *)(param_1 + 4), buffer, sizeof(buffer) - 1);
  ((char *)(param_1 + 4))[sizeof(buffer) - 1] = '\0';


  if (send_bytes_str("\tEnter last name: ", 18) != 18) {
    _terminate(0xfffffff7);
  }
  bytes_received = recv_until_delim_n(0, '\n', buffer, sizeof(buffer));
  if (bytes_received < 1) {
    _terminate(0xfffffff5);
  }
  // Copy to param_1 + 0x404
  strncpy((char *)(param_1 + 0x404), buffer, sizeof(buffer) - 1);
  ((char *)(param_1 + 0x404))[sizeof(buffer) - 1] = '\0';
}

// Function: validate_vote
unsigned int validate_vote(void) {
  return 1;
}

// Function: validate_voter
unsigned int validate_voter(int param_1) {
  if (strlen((char *)(param_1 + 4)) != 0 && strlen((char *)(param_1 + 0x404)) != 0) {
    return 1;
  }
  return 0;
}

// Function: validate_candidate
unsigned int validate_candidate(int param_1) {
  if (strlen((char *)(param_1 + 4)) == 0 || strlen((char *)(param_1 + 0x404)) == 0) {
    return 0;
  }

  // Simplified logic, as list_head_node/list_end_marker are mocks
  // In a real scenario, this would iterate through candidates to check for duplicates.
  if (list_length(DAT_00019844) == 0) {
    return 1;
  }
  return 1; // Dummy return for now
}

// Function: validate_emgr
unsigned int validate_emgr(int param_1) {
  if (strlen((char *)(param_1 + 4)) != 0 && strlen((char *)(param_1 + 0x404)) != 0) {
    return 1;
  }
  return 0;
}

// Function: create_election_mgr
unsigned int create_election_mgr(void) {
  if (send_bytes_str("## Create election manager ##\n", 30) != 30) {
    _terminate(0xfffffff7);
  }

  DAT_00019864 = malloc(0x80c); // Allocate space for EMGR structure
  if (DAT_00019864 == NULL) {
    _terminate(0xfffffff9);
  }

  *(unsigned int *)DAT_00019864 = DAT_00019854; // Set manager ID
  set_first_last_name((int)DAT_00019864);
  *(unsigned int *)(DAT_00019864 + 0x201 * sizeof(unsigned int)) = (unsigned int)validate_emgr;
  *(unsigned int *)(DAT_00019864 + 0x202 * sizeof(unsigned int)) = rand(); // Auth key

  if (((unsigned int (*)(void))(*(unsigned int *)(DAT_00019864 + 0x201 * sizeof(unsigned int))))() == 0) {
    return 0xffffffff;
  } else {
    char local_60[72];
    int len = snprintf(local_60, sizeof(local_60), "\tYour ID number is: %u\n\tYour auth key is: %u\n",
                       *(unsigned int *)DAT_00019864, *(unsigned int *)(DAT_00019864 + 0x202 * sizeof(unsigned int)));
    if (send_bytes(1, local_60, len) != len) {
      _terminate(0xfffffff7);
    }
  }
  return 0;
}

// Function: get_election_name
void get_election_name(void) {
  char buffer[0x400]; // Max size for election name + null terminator
  int bytes_received;

  if (send_bytes_str("Enter election name: ", 21) != 21) {
    _terminate(0xfffffff7);
  }

  bytes_received = recv_until_delim_n(0, '\n', buffer, sizeof(buffer));
  if (bytes_received < 1) {
    _terminate(0xfffffff5);
  }
  strncpy((char *)&e + 0x444, buffer, sizeof(buffer) - 1); // Assuming 'e' is a struct/global
  ((char *)&e + 0x444)[sizeof(buffer) - 1] = '\0';
}

// Function: set_election_conf
int set_election_conf(void) {
  unsigned int value;
  char response_char;

  if (send_bytes_str("Enter number of election winners desired: ", 41) != 41) {
    _terminate(0xfffffff7);
  }
  receive_number(&value);
  DAT_00019860 = (unsigned char)value;

  if (send_bytes_str("Enter the max number of candidates allowed: ", 44) != 44) {
    _terminate(0xfffffff7);
  }
  receive_number(&value);
  DAT_00019861 = (unsigned char)value;

  if (send_bytes_str("Can voters write-in new candidates? (Yy/Nn): ", 45) != 45) {
    _terminate(0xfffffff7);
  }
  receive_letter(&response_char);
  DAT_00019862 = (response_char == 'Y' || response_char == 'y') ? 1 : 0;

  return 0;
}

// Function: get_next_voter_id
int get_next_voter_id(void) {
  return DAT_00019858++;
}

// Function: get_next_candidate_id
int get_next_candidate_id(void) {
  return DAT_0001985c++;
}

// Function: get_next_vote_id
unsigned int get_next_vote_id(unsigned int *param_1) {
  return _DAT_4347c000 ^ *param_1;
}

// Function: is_candidate
unsigned int is_candidate(int *param_1, int *param_2) {
  return *param_1 == *param_2; // Simplified, assuming candidate ID comparison
}

// Function: print_select_candidate_list
unsigned int print_select_candidate_list(void) {
  if (list_length(DAT_00019844) == 0) {
    if (send_bytes_str("There are no candidates. Go add some.\n", 38) != 38) {
      _terminate(0xfffffff7);
    }
  } else {
    if (send_bytes_str("Select one candidate from the following:\n", 41) != 41) {
      _terminate(0xfffffff7);
    }
    // Dummy loop for candidate listing
    // In a real scenario, this would iterate through the candidate list.
    char buffer[2065];
    snprintf(buffer, sizeof(buffer), "\t%u: %s %s\n", 1, "John", "Doe"); // Example candidate
    if (send_bytes(1, buffer, strlen(buffer)) != strlen(buffer)) {
      _terminate(0xfffffff7);
    }
  }
  return 0;
}

// Function: send_voting_receipt
unsigned int send_voting_receipt(unsigned int param_1, int param_2) {
  char local_854[2108];
  int len = snprintf(local_854, sizeof(local_854), "Confirmation #%u. Vote recorded for candidate: %s %s.\n",
                     param_1, (char *)(param_2 + 4), (char *)(param_2 + 0x404));
  if (send_bytes(1, local_854, len) != len) {
    _terminate(0xfffffff7);
  }
  return 0;
}

// Function: get_candidate_by_id
unsigned int get_candidate_by_id(unsigned int param_1, unsigned int *param_2) {
  // Dummy implementation: always succeed with a dummy candidate
  *param_2 = 100; // Dummy candidate pointer/ID
  return 0;
}

// Function: create_and_insert_vote
unsigned int create_and_insert_vote(unsigned int param_1, unsigned int *param_2) {
  void *vote_struct = malloc(0x101c); // Allocate space for vote structure
  if (vote_struct == NULL) {
    _terminate(0xffffffef); // Using a different error code as per original
  }

  *(unsigned int *)vote_struct = get_next_vote_id((unsigned int *)vote_struct);
  *(unsigned int *)(vote_struct + 0x406 * sizeof(unsigned int)) = (unsigned int)validate_vote;
  *param_2 = *(unsigned int *)vote_struct;
  *(unsigned int *)(DAT_00019850 + 0x808) = *(unsigned int *)vote_struct; // Mark voter as voted

  memcpy(vote_struct + 1 * sizeof(unsigned int), (void *)DAT_00019850, 0x80c); // Copy voter info
  memcpy(vote_struct + 0x204 * sizeof(unsigned int), (void *)param_1, 0x808); // Copy candidate info

  if (((unsigned int (*)(void))(*(unsigned int *)(vote_struct + 0x406 * sizeof(unsigned int))))() == 0) {
    free(vote_struct);
    return 0xffffffff;
  } else {
    // Simulate creating a key for the hash table
    size_t sVar5 = strlen((char *)(DAT_00019850 + 4));
    size_t sVar6 = strlen((char *)(DAT_00019850 + 0x404));
    int local_1c = sVar5 + sVar6 + 2; // For "FirstName LastName"

    char *local_18 = malloc(local_1c);
    if (local_18 == NULL) {
      free(vote_struct);
      _terminate(0xfffffff9);
    }
    snprintf(local_18, local_1c, "%s %s", (char *)(DAT_00019850 + 4), (char *)(DAT_00019850 + 0x404));

    void **local_20 = malloc(8); // For key-value pair
    if (local_20 == NULL) {
      free(local_18);
      free(vote_struct);
      _terminate(0xfffffff9);
    }
    *local_20 = local_18;
    local_20[1] = vote_struct;

    void *existing_pair = ht_pair_insert(DAT_0001984c, local_20);
    if (existing_pair != NULL) {
      free(*(void **)existing_pair);
      free(((void **)existing_pair)[1]);
      free(existing_pair);
    }

    if (ht_is_re_hash_needed(DAT_0001984c)) {
      DAT_0001984c = ht_re_hash(DAT_0001984c);
    }
  }
  return 0;
}

// Function: is_candidate_eq
unsigned int is_candidate_eq(int *param_1, int *param_2) {
  if (*param_1 == *param_2 &&
      streq((char *)(param_1 + 1), (char *)(param_2 + 1)) == 0 &&
      streq((char *)(param_1 + 0x101), (char *)(param_2 + 0x101)) == 0) {
    return 1;
  }
  return 0;
}

// Function: is_pair_vote_cnt_gteq
unsigned int is_pair_vote_cnt_gteq(unsigned int *param_1, unsigned int *param_2) {
  return *param_2 <= *param_1;
}

// Function: free_results_list_pair
void free_results_list_pair(void *param_1) {
  free(param_1);
}

// Function: calculate_voting_results
unsigned int calculate_voting_results(void *param_1) {
  // Dummy iteration over candidates and votes
  // In a real system, this would iterate through DAT_00019844 (candidates)
  // and DAT_0001984c (votes) to count votes for each candidate.
  if (list_length(DAT_00019844) > 0) {
    int *candidate_node = (int *)list_head_node(DAT_00019844);
    int *candidate_end = (int *)list_end_marker(DAT_00019844);

    while (candidate_node != NULL && candidate_node != candidate_end) {
      int current_candidate_ptr = *candidate_node;
      int vote_count = 0;

      void *vote_iter = ht_pair_iter_start(DAT_0001984c);
      while (vote_iter != NULL) {
        int vote_struct_ptr = *(int *)((char *)vote_iter + 4); // Assuming value is at offset 4
        // Simplified comparison: assuming candidate ID is directly comparable
        if (is_candidate_eq(&current_candidate_ptr, (int *)(vote_struct_ptr + 0x204 * sizeof(unsigned int)))) {
          vote_count++;
        }
        vote_iter = ht_pair_iter_next(vote_iter);
      }

      int *results_pair = malloc(sizeof(int) * 2); // [vote_count, candidate_ptr]
      if (results_pair == NULL) {
        _terminate(0xfffffff9);
      }
      results_pair[0] = vote_count;
      results_pair[1] = current_candidate_ptr;
      list_insert_sorted(param_1, (int)is_pair_vote_cnt_gteq, results_pair);

      candidate_node = (int *)list_next_node(candidate_node);
    }
  }
  return 0;
}

// Function: send_voting_results
unsigned int send_voting_results(void *param_1, bool show_all) {
  if (list_length(DAT_00019844) == 0) {
    if (send_bytes_str("\tThere were zero candidates.\n", 29) != 29) {
      _terminate(0xfffffff7);
    }
  } else {
    if (send_bytes_str("\t| Rank | Votes | Candidate Name |\n", 35) != 35) {
      _terminate(0xfffffff7);
    }

    unsigned int rank = 0;
    void *results_node = list_head_node(param_1);
    void *results_end = list_end_marker(param_1);

    while (results_node != NULL && results_node != results_end) {
      unsigned int *results_pair = (unsigned int *)*(unsigned int *)results_node; // [vote_count, candidate_ptr]
      rank++;

      if (rank <= DAT_00019860 || show_all) {
        char buffer[2079];
        int candidate_ptr = results_pair[1];
        unsigned int votes = results_pair[0];
        int len = snprintf(buffer, sizeof(buffer), "\t%u.\t%u\t%s %s\n",
                           rank, votes, (char *)(candidate_ptr + 4), (char *)(candidate_ptr + 0x404));
        if (send_bytes(1, buffer, len) != len) {
          _terminate(0xfffffff7);
        }
      }
      results_node = list_next_node(results_node);
    }
  }
  return 0;
}

// Function: send_voter_results
unsigned int send_voter_results(void) {
  if (ht_length(DAT_00019848) == 0) {
    if (send_bytes_str("\tThere were zero voters.\n", 26) != 26) {
      _terminate(0xfffffff7);
    }
  } else {
    if (send_bytes_str("\t| Voted? | Voter Name |\n", 25) != 25) {
      _terminate(0xfffffff7);
    }

    void *voter_iter = ht_pair_iter_start(DAT_00019848);
    while (voter_iter != NULL) {
      int voter_struct_ptr = *(int *)((char *)voter_iter + 4); // Assuming value is at offset 4

      char buffer[2056];
      const char *voted_status = (*(int *)(voter_struct_ptr + 0x808) == 0) ? DAT_0001633b : DAT_0001633f;
      int len = snprintf(buffer, sizeof(buffer), "\t%s\t%s %s\n",
                         voted_status, (char *)(voter_struct_ptr + 4), (char *)(voter_struct_ptr + 0x404));
      if (send_bytes(1, buffer, len) != len) {
        _terminate(0xfffffff7);
      }
      voter_iter = ht_pair_iter_next(voter_iter);
    }
  }
  return 0;
}

// Function: is_nobody_logged_in
bool is_nobody_logged_in(void) {
  return DAT_00019850 == 0;
}

// Function: is_emgr_logged_in
bool is_emgr_logged_in(void) {
  return DAT_00019864 == (void *)DAT_00019850;
}

// Function: is_voter_logged_in
unsigned int is_voter_logged_in(void) {
  return (DAT_00019850 != 0 && DAT_00019864 != (void *)DAT_00019850);
}

// Function: login
unsigned int login(void) {
  if (!is_nobody_logged_in()) {
    return 4; // Already logged in
  }

  DAT_00019850 = 0; // Clear current login before attempting new login

  if (send_bytes_str("\tEnter your ID number: ", 23) != 23) {
    _terminate(0xfffffff7);
  }
  unsigned int id_number;
  receive_number(&id_number);

  if (send_bytes_str("\tEnter last name: ", 18) != 18) {
    _terminate(0xfffffff7);
  }
  char last_name_buffer[1025];
  int bytes_received = recv_until_delim_n(0, '\n', last_name_buffer, sizeof(last_name_buffer));
  if (bytes_received < 1) {
    _terminate(0xfffffff5);
  }

  if (id_number == DAT_00019854) { // Check if ID matches election manager
    if (streq((char *)(DAT_00019864 + 0x404), last_name_buffer) == 0) { // Compare last name
      if (send_bytes_str("\tEnter auth key: ", 17) != 17) {
        _terminate(0xfffffff7);
      }
      unsigned int auth_key;
      receive_number(&auth_key);

      if (*(unsigned int *)(DAT_00019864 + 0x808) == auth_key) { // Compare auth key
        DAT_00019850 = (int)DAT_00019864;
        return 1; // Election manager logged in
      }
    }
  } else if (ht_length(DAT_00019848) != 0) { // Check voters hash table
    void *voter_ptr = ht_value_as_voidp(DAT_00019848, id_number);
    if (voter_ptr != NULL) {
      if (streq((char *)(voter_ptr + 0x404), last_name_buffer) == 0) { // Compare last name
        DAT_00019850 = (int)voter_ptr;
        return 2; // Voter logged in
      }
    }
  }
  return 4; // Login failed
}

// Function: create_election
int create_election(void) {
  int result = create_election_mgr();
  if (result == 0) {
    get_election_name();
    result = set_election_conf();
    if (result == 0) {
      e = 2; // Set election state to 'created'
      if (send_bytes_str("Election Created.\n", 18) != 18) {
        _terminate(0xfffffff7);
      }
    }
  }
  return result;
}

// Function: open_voting
unsigned int open_voting(void) {
  if (!is_emgr_logged_in()) {
    return 0xffffffff;
  }
  e = 4; // Set election state to 'open'
  if (send_bytes_str("Voting Open.\n", 13) != 13) {
    _terminate(0xfffffff7);
  }
  return 0;
}

// Function: close_voting
unsigned int close_voting(void) {
  if (!is_emgr_logged_in()) {
    return 0xffffffff;
  }
  e = 8; // Set election state to 'closed'
  if (send_bytes_str("Voting Closed.\n", 15) != 15) {
    _terminate(0xfffffff7);
  }
  return 0;
}

// Function: election_status
unsigned int election_status(void) {
  if (!is_emgr_logged_in()) {
    return 0xffffffff;
  }
  char local_88[100];
  int len = snprintf(local_88, sizeof(local_88), "\tNumber of voters: %u\n\tNumber of votes: %u\n\tNumber of candidates: %u\n",
                     ht_length(DAT_00019848), ht_length(DAT_0001984c), list_length(DAT_00019844));
  if (send_bytes(1, local_88, len) != len) {
    _terminate(0xfffffff7);
  }
  return 0;
}

// Function: vote
int vote(void) {
  if (!is_voter_logged_in()) {
    return -1;
  }

  if (list_length(DAT_00019844) == 0) {
    if (send_bytes_str("There are no candidates. Go add some.\n", 38) != 38) {
      _terminate(0xfffffff7);
    }
    return 0;
  }

  int result = print_select_candidate_list();
  if (result != 0) return result;

  unsigned int candidate_id;
  get_choice(&candidate_id);

  unsigned int candidate_ptr;
  result = get_candidate_by_id(candidate_id, &candidate_ptr);
  if (result != 0) return result;

  unsigned int vote_id;
  result = create_and_insert_vote(candidate_ptr, &vote_id);
  if (result != 0) return result;

  result = send_voting_receipt(vote_id, candidate_ptr);
  return result;
}

// Function: register_voter
unsigned int register_voter(void) {
  if (!is_nobody_logged_in()) {
    return 0xffffffff; // Not nobody logged in
  }

  if (send_bytes_str("## Create voter ##\n", 19) != 19) {
    _terminate(0xfffffff7);
  }

  void *voter_struct = malloc(0x80c); // Allocate space for voter structure
  if (voter_struct == NULL) {
    _terminate(0xfffffff9);
  }

  *(unsigned int *)voter_struct = get_next_voter_id();
  *(unsigned int *)(voter_struct + 0x202 * sizeof(unsigned int)) = 0; // Has not voted
  *(unsigned int *)(voter_struct + 0x201 * sizeof(unsigned int)) = (unsigned int)validate_voter;
  set_first_last_name((int)voter_struct);

  if (((unsigned int (*)(void))(*(unsigned int *)(voter_struct + 0x201 * sizeof(unsigned int))))() == 0) {
    free(voter_struct);
    return 0xffffffff;
  } else {
    void **voter_pair = malloc(8); // For key-value pair
    if (voter_pair == NULL) {
      free(voter_struct);
      _terminate(0xfffffff9);
    }
    *voter_pair = (void *)*(unsigned int *)voter_struct; // Key is voter ID
    voter_pair[1] = voter_struct;                       // Value is voter struct

    void *existing_pair = ht_pair_insert(DAT_00019848, voter_pair);
    if (existing_pair != NULL) {
      free(*(void **)existing_pair); // Free old key (if dynamically allocated)
      free(((void **)existing_pair)[1]); // Free old voter struct
      free(existing_pair);
    }

    char local_43[35];
    int len = snprintf(local_43, sizeof(local_43), "\tYour ID number is: %u\n", *(unsigned int *)voter_struct);
    if (send_bytes(1, local_43, len) != len) {
      _terminate(0xfffffff7);
    }

    if (ht_is_re_hash_needed(DAT_00019848)) {
      DAT_00019848 = ht_re_hash(DAT_00019848);
    }
  }
  return 0;
}

// Function: add_candidate
unsigned int add_candidate(void) {
  if (is_nobody_logged_in() && !is_voter_logged_in()) {
    return 0xffffffff; // Not logged in as EMGR or voter (with write-in allowed)
  }

  if (list_length(DAT_00019844) >= DAT_00019861) {
    if (send_bytes_str("Not accepting additional candidates.\n", 37) != 37) {
      _terminate(0xfffffff7);
    }
    return 0;
  }

  if (send_bytes_str("## Create candidate ##\n", 23) != 23) {
    _terminate(0xfffffff7);
  }

  void *candidate_struct = malloc(0x808); // Allocate space for candidate structure
  if (candidate_struct == NULL) {
    _terminate(0xfffffff9);
  }

  set_first_last_name((int)candidate_struct);
  *(unsigned int *)(candidate_struct + 0x201 * sizeof(unsigned int)) = (unsigned int)validate_candidate;

  if (((unsigned int (*)(void))(*(unsigned int *)(candidate_struct + 0x201 * sizeof(unsigned int))))() == 0) {
    free(candidate_struct);
    if (send_bytes_str("New candidate invalid\n", 22) != 22) {
      _terminate(0xfffffff7);
    }
    return 0;
  }

  *(unsigned int *)candidate_struct = get_next_candidate_id();
  list_insert_at_end(DAT_00019844, candidate_struct);

  if (send_bytes_str("Candidate Added.\n", 17) != 17) {
    _terminate(0xfffffff7);
  }
  return 0;
}

// Function: voting_results
int voting_results(void) {
  if (is_nobody_logged_in()) {
    return -1;
  }

  if (send_bytes_str("## Voting Results ##\n", 21) != 21) {
    _terminate(0xfffffff7);
  }

  if (list_length(DAT_00019844) == 0) {
    if (send_bytes_str("\tThere were zero candidates.\n", 29) != 29) {
      _terminate(0xfffffff7);
    }
    return 0;
  }

  void *results_list = malloc(0x14); // Dummy list allocation
  if (results_list == NULL) _terminate(0xfffffff9); // Placeholder
  list_init(results_list, free_results_list_pair);

  int result = calculate_voting_results(results_list);
  if (result == 0) {
    result = send_voting_results(results_list, false);
    if (result == 0) {
      list_destroy(results_list);
    }
  }
  free(results_list); // Free the dummy list
  return result;
}

// Function: voter_turnout
int voter_turnout(void) {
  if (!is_emgr_logged_in()) {
    return -1;
  }

  if (send_bytes_str("## Voter Turnout ##\n", 20) != 20) {
    _terminate(0xfffffff7);
  }

  if (ht_length(DAT_00019848) == 0) {
    if (send_bytes_str("\tThere were zero voters.\n", 26) != 26) {
      _terminate(0xfffffff7);
    }
    return 0;
  }

  char local_64[76];
  int len = snprintf(local_64, sizeof(local_64), "\tNumber of voters: %u\n\tNumber of votes: %u\n",
                     ht_length(DAT_00019848), ht_length(DAT_0001984c));
  if (send_bytes(1, local_64, len) != len) {
    _terminate(0xfffffff7);
  }

  int result = send_voter_results();
  return result;
}

// Function: candidate_summary
int candidate_summary(void) {
  if (!is_emgr_logged_in()) {
    return -1;
  }

  if (send_bytes_str("## Candidate Summary ##\n", 24) != 24) {
    _terminate(0xfffffff7);
  }

  int num_candidates = list_length(DAT_00019844);
  char local_43[39];
  int len = snprintf(local_43, sizeof(local_43), "\tNumber of candidates: %u\n", num_candidates);
  if (send_bytes(1, local_43, len) != len) {
    _terminate(0xfffffff7);
  }

  if (num_candidates == 0) {
    if (send_bytes_str("\tThere were zero candidates.\n", 29) != 29) {
      _terminate(0xfffffff7);
    }
    return 0;
  }

  void *results_list = malloc(0x14); // Dummy list allocation
  if (results_list == NULL) _terminate(0xfffffff9); // Placeholder
  list_init(results_list, free_results_list_pair);

  int result = calculate_voting_results(results_list);
  if (result == 0) {
    result = send_voting_results(results_list, true); // True to show all candidates
    if (result == 0) {
      list_destroy(results_list);
    }
  }
  free(results_list); // Free the dummy list
  return result;
}

// Function: decider
unsigned int decider(unsigned int param_1) {
  if (!((param_1 & 0xff) & e)) { // Check if the option is valid for the current election state
    DAT_00019850 = 0; // Log out on invalid option
    return 0xffffffff;
  }

  unsigned int result = 0; // Default success
  switch (param_1 & 0xff) {
  case 0xe: // 14: Login
    result = login();
    break;
  case 0xf: // 15: Main Menu (Logout)
    DAT_00019850 = 0;
    break;
  case 0x11: // 17: Create Election
    result = create_election();
    DAT_00019850 = 0; // Log out after creation
    break;
  case 0x12: // 18: Open Voting
    result = open_voting();
    DAT_00019850 = 0; // Log out after action
    break;
  case 0x14: // 20: Close Voting
    result = close_voting();
    DAT_00019850 = 0; // Log out after action
    break;
  case 0x16: // 22: Election Status
    result = election_status();
    DAT_00019850 = 0; // Log out after action
    break;
  case 0x24: // 36: Vote
    result = vote();
    DAT_00019850 = 0; // Log out after action
    break;
  case 0x26: // 38: Register Voter
    result = register_voter();
    DAT_00019850 = 0; // Log out after registration
    break;
  case 0x36: // 54: Add Candidate
    result = add_candidate();
    DAT_00019850 = 0; // Log out after action
    break;
  case 0x38: // 56: Voting Results
    result = voting_results();
    DAT_00019850 = 0; // Log out after action
    break;
  case 0x48: // 72: Voter Turnout
    result = voter_turnout();
    DAT_00019850 = 0; // Log out after action
    break;
  case 0x58: // 88: Candidate Summary
    result = candidate_summary();
    DAT_00019850 = 0; // Log out after action
    break;
  case 0x5f: // 95: Quit
    result = 2; // Special return for quit
    DAT_00019850 = 0;
    break;
  default:
    result = 0xffffffff; // Invalid option
    DAT_00019850 = 0;    // Log out on invalid option
    break;
  }
  return result;
}

// Function: init_election
void init_election(void) {
  e = 1; // Initial state
  DAT_00019854 = 0x9a2112; // Default EMGR ID
  DAT_00019858 = 0xbc4ff2; // Starting voter ID
  DAT_0001985c = 1;         // Starting candidate ID
  DAT_00019850 = 0;         // No one logged in initially

  DAT_00019844 = malloc(0x14); // Allocate for candidate list
  if (DAT_00019844 == NULL) {
    _terminate(0xfffffff9);
  }
  list_init(DAT_00019844, NULL); // Assuming NULL for free_func if not needed for simple types

  DAT_00019848 = ht_int_init(0x32); // Initialize voter hash table
  DAT_0001984c = ht_str_init(0x32); // Initialize vote hash table
}

// Function: print_menu
void print_menu(unsigned int param_1) {
  print_login_menu(e, param_1);
  print_admin_menu(e, param_1);
  print_voting_menu(e, param_1);
  print_results_menu(e, param_1);
  if (send_bytes_str("95. Quit\n", 9) != 9) {
    _terminate(0xfffffff7);
  }
}

// Function: get_choice
void get_choice(unsigned int *param_1) {
  if (send_bytes_str("Choose an option: ", 18) != 18) {
    _terminate(0xfffffff7);
  }
  receive_number(param_1);
}

// main function to simulate the program flow
int main() {
  init_election();

  unsigned int choice;
  int status = 0;

  do {
    print_menu(DAT_00019850); // Pass current login status to print dynamic menu
    get_choice(&choice);
    status = decider(choice);

    if (status == 0xffffffff) {
      send_bytes_auto("Invalid option or action not allowed.\n");
    }

  } while (status != 2); // Loop until decider returns 2 (Quit)

  send_bytes_auto("Exiting...\n");

  // Cleanup (if mocks allocated memory)
  free(DAT_00019844);
  free(DAT_00019848);
  free(DAT_0001984c);
  if (DAT_00019864) free(DAT_00019864);

  return 0;
}