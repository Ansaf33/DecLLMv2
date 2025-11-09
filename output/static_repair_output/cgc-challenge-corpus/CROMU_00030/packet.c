#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> // For write
#include <stdlib.h> // For rand

// Placeholder global variables
int current_connection_number = -1;
int last_connection_number = 0;
int current_packet_count_recvd = -1;
int current_packet_count_sent = -1;
int current_encryption = 0; // 0: none, 1: type1, 2: type2
int encryption_confirmed = 0;
int enc_chal = 0;
int challenge_type_global = 0; // Renamed from ram0x000184d1
char challenge_data_global[5]; // Renamed from DAT_000184cc

// Placeholder for game_stack.
#define MAX_GAME_PIECES 100
char game_stack[MAX_GAME_PIECES][9]; // Each piece is 9 bytes
int game_stack_count = 0; // Represents game_stack._1152_4_

int current_max_road_len = 0;

// Placeholder for VALID_CHARS and related
const char VALID_CHARS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const int LAST_CHAR_POS = sizeof(VALID_CHARS) - 1; // -1 for null terminator
const int offset = 3; // Example offset

// --- Function Prototypes ---
char random_in_range(int min, int max);
int create_random_piece(char *piece_buffer);
void piece_to_pkt(const char *piece_data, char *packet_data_field);
int get_piece(int index);
int connect_pieces(char *piece1, int conn1, char *piece2, int conn2);
int discard_piece();
int get_max_road_len();
int receive_until(char *buffer, int max_len, int terminator);
unsigned char calculateCS(const char *data, int len);
void set_checksum(char *packet);
char get_checksum(const char *packet);
int locate_char(char c);
int encrypt_data(char *data, int len, int type);
int decrypt_packet(char *data, int len, int type);
void send_packet_new(char *packet_buffer, int data_len);
void send_ack(char pkt_num_char);
void send_nack(char pkt_num_char, char reason_code_char);
void send_assoc_response(char conn_num_char);
void send_auth_challenge(int type);
void send_auth_response(char *param_1);
int handle_auth_challenge_resp(const char *challenge_resp_data);
int handle_disass_req(char param_1);
bool handle_deauth_req(char param_1);


// --- Placeholder Function Implementations ---
char random_in_range(int min, int max) {
    return (char)(min + (rand() % (max - min + 1)));
}

int create_random_piece(char *piece_buffer) {
    if (game_stack_count >= MAX_GAME_PIECES) return 0x21; // "nfull"
    for (int i = 0; i < 9; ++i) {
        piece_buffer[i] = random_in_range('a', 'z');
    }
    // Store the created piece in game_stack
    memcpy(game_stack[game_stack_count], piece_buffer, 9);
    game_stack_count++;
    return 1; // Success
}

void piece_to_pkt(const char *piece_data, char *packet_data_field) {
    memcpy(packet_data_field, piece_data, 9);
}

int get_piece(int index) {
    if (index < 0 || index >= game_stack_count) return -1;
    return index;
}

int connect_pieces(char *piece1_ptr, int conn1, char *piece2_ptr, int conn2) {
    printf("Connecting piece 1 (%s) at %d to piece 2 (%s) at %d\n", piece1_ptr, conn1, piece2_ptr, conn2);
    current_max_road_len++; // Example logic
    return 1;
}

int discard_piece() {
    if (game_stack_count > 0) {
        game_stack_count--;
        printf("Discarded a piece. Remaining: %d\n", game_stack_count);
        return 1;
    }
    printf("No pieces to discard.\n");
    return 0;
}

int get_max_road_len() {
    return current_max_road_len;
}

int receive_until(char *buffer, int max_len, int terminator) {
    printf("Enter name: ");
    if (fgets(buffer, max_len, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        return (int)len;
    }
    return 0;
}

// Function: populate_packet
int populate_packet(void *param_1, void *param_2, int param_3) {
  if (param_3 != 16) {
    printf("not proper packet length: got %d, should be %d\n", param_3, 16);
    return 0;
  }
  
  memcpy(param_1, param_2, 16);
  int len_val = ((char *)param_1)[4] - '0';
  if (len_val >= 0 && len_val < 11) {
    return 1;
  } else {
    printf("length field outside valid range\n");
    return 0;
  }
}

// Function: validate_packet
int validate_packet(char *param_1) {
  int is_valid = 1;

  // Type check: assuming 'a','b','c' are the only valid packet types
  if (param_1[0] < 'a' || param_1[0] > 'c') {
    printf("type is out of range (%c)\n", param_1[0]);
    is_valid = 0;
  }

  // Subtype check
  if (param_1[0] == 'c') {
    if (param_1[1] < 'v' || param_1[1] > 'z') { // 'v' through 'z'
      printf("data subtype is out of range (%c)\n", param_1[1]);
      is_valid = 0;
    }
  } else { // For types 'a' or 'b'
    if (param_1[1] < 'd' || param_1[1] > 'q') { // 'd' through 'q'
      printf("subtype is out of range (%c)\n", param_1[1]);
      is_valid = 0;
    }
  }

  // Length check (data length, char '0'-'9', max 10 bytes)
  int length_val = param_1[4] - '0';
  if (length_val < 0 || length_val > 10) {
    printf("length is out of range (%c)\n", param_1[4]);
    is_valid = 0;
  }

  // Connection number check
  if (param_1[1] != 'd' && (param_1[3] - '0' != current_connection_number)) {
    printf("connection number not valid: %c vs %c\n", param_1[3], (char)(current_connection_number + '0'));
    is_valid = 0;
  }

  // Checksum check
  char calculated_checksum = get_checksum(param_1);
  if (calculated_checksum != param_1[15]) {
    printf("checksum does not match %c vs %c\n", calculated_checksum, param_1[15]);
    is_valid = 0;
  }
  return is_valid;
}

// Function: packet_handler
int packet_handler(char *param_1) {
  char response_packet[16];
  memset(response_packet, 0, 16); // Initialize packet buffer

  if (param_1[0] == 'b') {
    if (param_1[1] == 'd') {
      send_assoc_response(param_1[5]); // param_1[5] is connection number char
      return 1;
    }
    if (param_1[1] == 'e') {
      return 0;
    }
    if (param_1[1] == 'f') {
      send_auth_response(param_1 + 5);
      return 1;
    }
    if (param_1[1] == 'i') {
      return 0;
    }
    if (param_1[1] == 'h') {
      handle_auth_challenge_resp(param_1 + 5);
      return 1;
    }
    if (param_1[1] == 'j') {
      if (handle_disass_req(param_1[5]) == 0) {
        printf("Failed disass\n");
        return 2;
      }
      return 1;
    }
    if (param_1[1] == 'k') {
      printf("Dissasociation response received\n");
      return 0;
    }
    if (param_1[1] == 'l') {
      if (handle_deauth_req(param_1[5]) == 0) {
        printf("failed deauth...leaving\n");
        return 2;
      }
      return 1;
    }
    if (param_1[1] == 'm') {
      printf("Dissasociation response received\n");
      return 0;
    }
  }
  else if (param_1[0] == 'c') {
    if ((param_1[2] - '0') != (current_packet_count_recvd + 1)) {
      printf("packet number incorrect: %c vs %c\n", param_1[2], (char)(current_packet_count_recvd + '0' + 1));
      send_nack(param_1[2], '1'); // '1' as reason code
      return 0;
    }
    send_ack(param_1[2]);

    if (param_1[1] == 'x') {
      char piece_data_buffer[9];
      int piece_status = create_random_piece(piece_data_buffer);
      response_packet[0] = 'c';
      response_packet[1] = 'x';

      if (piece_status == 1) {
        piece_to_pkt(piece_data_buffer, &response_packet[5]);
        send_packet_new(response_packet, 9);
        return 1;
      }
      if (piece_status == 0x21) { // "nfull"
        strncpy(&response_packet[5], "nfull", strlen("nfull"));
        send_packet_new(response_packet, strlen("nfull"));
        return 1;
      }
      if (piece_status == 0x16) { // "nusetop"
        strncpy(&response_packet[5], "nusetop", 7);
        send_packet_new(response_packet, 7);
        return 1;
      }
      // Default case "nconfused"
      strncpy(&response_packet[5], "nconfused", 9);
      send_packet_new(response_packet, 9);
      return 1;
    }
    if (param_1[1] == 'z') {
      if (current_encryption == 1) {
        decrypt_packet(param_1 + 5, param_1[4] - '0', 1);
      } else if (current_encryption == 2) {
        decrypt_packet(param_1 + 5, param_1[4] - '0', 2);
      } else {
        return 0; // No encryption or unknown type
      }

      if (param_1[4] != '5') { // Expecting data length of 5
        return 0;
      }
      if (game_stack_count < 1) {
        response_packet[0] = 'c';
        response_packet[1] = 'z';
        memcpy(&response_packet[5], "NO,PCE", 6);
        send_packet_new(response_packet, 6);
        return 1;
      }
      
      int piece1_idx = get_piece(param_1[5] - '0');
      if (piece1_idx == -1) {
        printf("Out of bounds piece requested A: %d vs %d...\n", param_1[5] - '0', game_stack_count);
        return 0;
      }
      int piece2_idx = get_piece(param_1[8] - '0');
      if (piece2_idx == -1) {
        printf("Out of bounds piece requested B: %d vs %d...\n", param_1[8] - '0', game_stack_count);
        return 0;
      }

      if (connect_pieces(game_stack[piece1_idx], param_1[6] - '0',
                         game_stack[piece2_idx], param_1[9] - '0') == 0) {
        response_packet[0] = 'c';
        response_packet[1] = 'z';
        memcpy(&response_packet[5], "NO,ERROR", 8);
        send_packet_new(response_packet, 8);
        return 1;
      }
      response_packet[0] = 'c';
      response_packet[1] = 'z';
      response_packet[5] = (char)(current_max_road_len + '0');
      encrypt_data(&response_packet[5], 1, current_encryption); // Encrypt only the data part
      send_packet_new(response_packet, 1);
      return 1;
    }
    if (param_1[1] == 'w') {
      response_packet[0] = 'c';
      response_packet[1] = 'w';
      response_packet[5] = (char)(current_max_road_len + '0');
      encrypt_data(&response_packet[5], 1, current_encryption);
      send_packet_new(response_packet, 1);
      return 1;
    }
    if (param_1[1] == 'v') {
      response_packet[0] = 'c';
      response_packet[1] = 'v';
      if (discard_piece() == 1) {
        response_packet[5] = 'y'; // 'y' for yes
        send_packet_new(response_packet, 1);
        return 1;
      }
      strncpy(&response_packet[5], "nplaced", 7); // 'nplaced' for no
      send_packet_new(response_packet, 7);
      return 1;
    }
  }
  return 1; // Default return if no specific handler matches
}

// Function: send_ack
void send_ack(char pkt_num_char) {
  char packet[16];
  memset(packet, 0, 16);
  packet[0] = 'a';
  packet[1] = 'p';
  packet[5] = pkt_num_char; // Data is the acknowledged packet number char
  current_packet_count_recvd++;
  send_packet_new(packet, 1); // Data length 1
}

// Function: send_nack
void send_nack(char pkt_num_char, char reason_code_char) {
  char packet[16];
  memset(packet, 0, 16);
  packet[0] = 'a';
  packet[1] = 'q';
  packet[5] = pkt_num_char; // Data is the nack'd packet number char
  packet[6] = reason_code_char; // Reason code char
  current_packet_count_recvd++;
  send_packet_new(packet, 2); // Data length 2
}

// Function: send_assoc_response
void send_assoc_response(char conn_num_char) {
  char packet[16];
  memset(packet, 0, 16);
  packet[0] = 'b';
  packet[1] = 'e'; // Subtype 'e' for association response

  if ((conn_num_char - '0') == last_connection_number + 1) {
    last_connection_number++;
    current_packet_count_recvd = -1; // Reset for new connection
    current_connection_number = conn_num_char - '0';
    packet[5] = '1'; // Success
    send_packet_new(packet, 1);
    printf("SUCCESSFUL association\n");
  } else {
    packet[5] = '0'; // Failure
    send_packet_new(packet, 1);
  }
}

// Function: send_auth_response
void send_auth_response(char *param_1) {
  if (*param_1 == 'a') {
    send_auth_challenge(1);
  } else if (*param_1 == 'x') {
    send_auth_challenge(2);
  } else {
    printf("AUTH RESP sent: fail\n");
    printf("requested: %s\n", param_1);
  }
}

// Function: send_auth_challenge
void send_auth_challenge(int type) {
  char packet[16];
  memset(packet, 0, 16);
  char challenge_data[5];
  
  enc_chal = 1;
  for (int i = 0; i < 5; i++) {
    challenge_data[i] = random_in_range('a', 'p'); // Example range
  }
  challenge_type_global = type; // Store the type for later validation
  
  packet[0] = 'b';
  packet[1] = 'g'; // Subtype 'g' for auth challenge
  memcpy(&packet[5], challenge_data, 5); // Data is the challenge
  send_packet_new(packet, 5); // Data length 5
  
  // Encrypt the challenge data and store it globally for verification
  encrypt_data(challenge_data, 5, type);
  memcpy(challenge_data_global, challenge_data, 5);
}

// Function: handle_auth_challenge_resp
int handle_auth_challenge_resp(const char *challenge_resp_data) {
  char packet[16];
  memset(packet, 0, 16);
  packet[0] = 'b';
  packet[1] = 'i'; // Subtype 'i' for auth challenge response

  bool match = true;
  for (int i = 0; i < 5; i++) {
    if (challenge_resp_data[i] != challenge_data_global[i]) {
      match = false;
      break;
    }
  }

  if (match) {
    enc_chal = 0; // Challenge handled
    packet[5] = '1'; // Success
    send_packet_new(packet, 1);
    current_encryption = challenge_type_global; // Set encryption type
    return 1;
  } else {
    printf("failed to match (%s)\n", challenge_data_global);
    packet[5] = '0'; // Failure
    send_packet_new(packet, 1);
    return 0;
  }
}

// Function: handle_disass_req
int handle_disass_req(char param_1) {
  char packet[16];
  memset(packet, 0, 16);
  packet[0] = 'b';
  packet[1] = 'k'; // Subtype 'k' for disassociation response

  if (current_encryption == -1 && encryption_confirmed == 0) { // This condition implies no encryption is active or confirmed
    if (param_1 == '3') { // Assuming '3' is a valid request for disassociation
      packet[5] = '1'; // Success
      send_packet_new(packet, 1);
      current_packet_count_recvd = -1;
      current_packet_count_sent = -1;
      last_connection_number = current_connection_number;
      current_connection_number = -1;
      return 1;
    } else {
      packet[5] = '0'; // Failure
      send_packet_new(packet, 1);
      return 0;
    }
  } else {
    printf("FAILED1\n"); // Original message, suggests invalid state for disassociation
    return 0;
  }
}

// Function: handle_deauth_req
bool handle_deauth_req(char param_1) {
  char packet[16];
  memset(packet, 0, 16);
  packet[0] = 'b';
  packet[1] = 'm'; // Subtype 'm' for deauthentication response

  if (param_1 == '7') { // Assuming '7' is a valid request for deauthentication
    packet[5] = '1'; // Success
    send_packet_new(packet, 1);
    current_encryption = -1; // Reset encryption state
    encryption_confirmed = 0;
    return true;
  } else {
    packet[5] = '0'; // Failure
    send_packet_new(packet, 1);
    return false;
  }
}

// Function: send_packet (Original, likely unused or placeholder)
void send_packet(int param_1) {
  // This function's original implementation was highly problematic.
  // Assuming it's a placeholder or not used.
  // If it's meant to send a pre-constructed packet, it needs a buffer and length.
  // For now, it will just print a warning.
  printf("WARNING: send_packet called, original implementation was faulty. Doing nothing.\n");
  return;
}

// Function: send_packet_new
void send_packet_new(char *packet_buffer, int data_len) {
  // Packet structure: type(1), subtype(1), pkt_num(1), conn_num(1), data_len_char(1), data(10), checksum(1) = 16 bytes total
  
  if (packet_buffer[0] == 'c') { // Only 'c' type packets increment packet count
    current_packet_count_sent++;
    packet_buffer[2] = (char)(current_packet_count_sent % 10 + '0'); // Packet number as char '0'-'9'
  } else {
    packet_buffer[2] = '0'; // Default packet number for non-'c' packets
  }
  
  packet_buffer[3] = (char)(current_connection_number % 10 + '0'); // Connection number as char '0'-'9'
  
  // Fill remaining data field (indices 5 to 14) with '0's if data_len is less than 10
  for (int i = data_len; i < 10; i++) {
    packet_buffer[i + 5] = '0';
  }
  
  packet_buffer[4] = (char)(data_len % 10 + '0'); // Data length as char '0'-'9' (max 10, so single digit is fine)
  
  set_checksum(packet_buffer);
  
  // Write the complete 16-byte packet to stdout
  write(STDOUT_FILENO, packet_buffer, 16);
  printf("\n"); // Add a newline for readability in console output
}

// Function: calculateCS
unsigned char calculateCS(const char *data, int len) {
  unsigned char checksum = 0;
  for (int i = 0; i < len; i++) {
    checksum ^= (unsigned char)data[i];
  }
  return checksum;
}

// Function: set_checksum
void set_checksum(char *packet) {
  packet[15] = (char)calculateCS(packet, 15); // Checksum is the 16th byte (index 15)
}

// Function: get_checksum
char get_checksum(const char *packet) {
  return (char)calculateCS(packet, 15); // Calculate checksum for first 15 bytes
}

// Function: locate_char
int locate_char(char c) {
  for (int i = 0; i < LAST_CHAR_POS; i++) {
    if (c == VALID_CHARS[i]) {
      return i;
    }
  }
  return -1; // Not found
}

// Function: encrypt_data
int encrypt_data(char *data, int len, int type) {
  if (type == 2) {
    for (int i = 0; i < len; i++) {
      int char_idx = locate_char(data[i]);
      if (char_idx == -1) return 0; // Invalid character
      
      if ((i & 1) == 0) { // Even index: add offset
        data[i] = VALID_CHARS[(char_idx + offset) % LAST_CHAR_POS];
      } else { // Odd index: subtract offset
        char_idx -= offset;
        if (char_idx < 0) char_idx += LAST_CHAR_POS;
        data[i] = VALID_CHARS[char_idx];
      }
    }
  } else if (type == 1) {
    for (int i = 0; i < len; i++) {
      int char_idx = locate_char(data[i]);
      if (char_idx == -1) return 0; // Invalid character
      
      if ((i & 1) == 0) { // Even index: subtract offset
        char_idx -= offset;
        if (char_idx < 0) char_idx += LAST_CHAR_POS;
        data[i] = VALID_CHARS[char_idx];
      } else { // Odd index: add offset
        data[i] = VALID_CHARS[(char_idx + offset) % LAST_CHAR_POS];
      }
    }
  }
  return 1; // Success
}

// Function: decrypt_packet
int decrypt_packet(char *data, int len, int type) {
  if (type == 2) { // Decryption for type 2 is inverse of encryption type 2
    for (int i = 0; i < len; i++) {
      int char_idx = locate_char(data[i]);
      if (char_idx == -1) return 0; // Invalid character
      
      if ((i & 1) == 0) { // Even index: subtract offset
        char_idx -= offset;
        if (char_idx < 0) char_idx += LAST_CHAR_POS;
        data[i] = VALID_CHARS[char_idx];
      } else { // Odd index: add offset
        data[i] = VALID_CHARS[(char_idx + offset) % LAST_CHAR_POS];
      }
    }
  } else if (type == 1) { // Decryption for type 1 is inverse of encryption type 1
    for (int i = 0; i < len; i++) {
      int char_idx = locate_char(data[i]);
      if (char_idx == -1) return 0; // Invalid character
      
      if ((i & 1) == 0) { // Even index: add offset
        data[i] = VALID_CHARS[(char_idx + offset) % LAST_CHAR_POS];
      } else { // Odd index: subtract offset
        char_idx -= offset;
        if (char_idx < 0) char_idx += LAST_CHAR_POS;
        data[i] = VALID_CHARS[char_idx];
      }
    }
  }
  return 1; // Success
}

// Function: check_win
bool check_win(int param_1) {
  if (param_1 <= get_max_road_len()) {
    printf("YOU WIN, what is your name?\n");
    char name_buffer[11]; // Max 10 chars + null terminator
    int bytes_read = receive_until(name_buffer, sizeof(name_buffer) - 1, 0x19); // 0x19 is not used in receive_until
    name_buffer[bytes_read] = '\0'; // Ensure null termination
    printf("Well done, %s\n", name_buffer);
    return true;
  }
  return false;
}