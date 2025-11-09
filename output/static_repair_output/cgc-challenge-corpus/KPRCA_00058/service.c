#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // For size_t

// Define the PacketContext structure based on observed offsets in the original code.
// The data_buffer is at offset 0x80 from the start of the PacketContext struct.
// Other fields are at fixed offsets after that.
#define PACKET_DATA_OFFSET 0x80
#define PACKET_BUFFER_MAX_SIZE 0x88B9 // Max size mentioned in recv_bytes, write_byte, etc.

// Forward declaration for the handler function type.
// The actual arguments for the handler are context and packet_type (char).
typedef void (*PacketHandlerFunc)(void*, char);

// Structure for packet context.
// The layout mimics the observed memory accesses (param_1 + offset).
typedef struct {
    uint8_t _padding[PACKET_DATA_OFFSET]; // Padding to align data_buffer at 0x80
    uint8_t data_buffer[PACKET_BUFFER_MAX_SIZE]; // Data buffer itself
    uint32_t read_offset;   // At offset 0x8938 from struct start
    uint32_t total_len;     // At offset 0x893c from struct start
    uint32_t packet_id;     // At offset 0x8940 from struct start
    PacketHandlerFunc handler_func; // At offset 0x8944 from struct start
    bool authenticated;     // At offset 0x8948 from struct start
} PacketContext;

// Structure for credentials, used by init_creds and userauth_password.
// The original code uses `param_1 + local_14 * 8` and `*(char **)(local_18 + 4)`.
// This implies a struct like { char* username; char* password; } where each field is 4 bytes on 32-bit systems,
// making the total size 8 bytes. On 64-bit, this would be 16 bytes. Assuming 32-bit compilation for offset matching.
// For Linux compilable, use `char*` which is pointer-sized.
typedef struct {
    char *username;
    char *password;
} Credential;

// Global array for credentials, as implied by `userauth_handler` not explicitly passing it.
Credential global_creds_array[16];

// Function: mystrndup
// Duplicates a substring of param_1 up to param_2 length.
// If param_1[param_2] is null, it increments the returned length.
size_t mystrndup(const char *param_1, size_t param_2, void **param_3) {
  size_t len = strlen(param_1);
  if (len < param_2) {
    param_2 = len;
  }

  if (param_2 == 0) {
    *param_3 = NULL;
    return 0;
  }

  void *dest = malloc(param_2 + 1);
  if (dest == NULL) {
    *param_3 = NULL;
    return 0;
  }

  memcpy(dest, param_1, param_2);
  ((char *)dest)[param_2] = '\0';
  *param_3 = dest;

  // Preserve original specific logic: if the character *after* the copied
  // portion in the *source* string is null, increment param_2.
  if (param_1[param_2] == '\0') {
    param_2++;
  }
  return param_2;
}

// Function: init_creds
// Initializes an array of Credential structures by parsing a source string.
// Skips single characters until a string can be duplicated.
Credential* init_creds(Credential *creds_array, const char *source_string) {
  char *current_pos = (char *)source_string;
  size_t bytes_read;
  void *temp_str_ptr;

  for (uint32_t i = 0; i < 16; i++) { // 0x10 is 16
    // Get username
    while (true) {
      bytes_read = mystrndup(current_pos, 8, &temp_str_ptr);
      if (bytes_read != 0) break;
      current_pos++; // Skip one character and try again
    }
    creds_array[i].username = (char *)temp_str_ptr;
    current_pos += bytes_read;

    // Get password
    while (true) {
      bytes_read = mystrndup(current_pos, 8, &temp_str_ptr);
      if (bytes_read != 0) break;
      current_pos++; // Skip one character and try again
    }
    creds_array[i].password = (char *)temp_str_ptr;
    current_pos += bytes_read;
  }
  return creds_array;
}

// Function: recv_bytes
// Receives a specified number of bytes from stdin into the packet buffer.
uint32_t recv_bytes(PacketContext *ctx, uint32_t count) {
  // Align count to the next multiple of 8
  if ((count & 7) != 0) {
    count = (count + 7) & 0xfffffff8;
  }

  // Check for buffer overflow before reading
  if (count + ctx->total_len >= PACKET_BUFFER_MAX_SIZE) {
    return 0;
  }

  // Read data into the buffer at the current total_len offset
  size_t bytes_read = fread(ctx->data_buffer + ctx->total_len, 1, count, stdin);
  if (bytes_read == count) {
    ctx->total_len += count;
  } else {
    count = 0; // Return 0 on partial read
  }
  return count;
}

// Function: clear_packet
// Resets the read/write offsets for a packet.
void clear_packet(PacketContext *ctx) {
  ctx->read_offset = 0;
  ctx->total_len = 0;
}

// Function: init_packet
// Initializes a packet by clearing it and setting initial offsets.
void init_packet(PacketContext *ctx) {
  clear_packet(ctx);
  ctx->read_offset = 5;
  ctx->total_len = 5;
}

// Function: get_byte
// Reads a single byte from the packet buffer.
bool get_byte(PacketContext *ctx, uint8_t *byte_val) {
  if (ctx->read_offset + 1 <= ctx->total_len) {
    *byte_val = ctx->data_buffer[ctx->read_offset];
    ctx->read_offset++;
    return true;
  }
  return false;
}

// Function: get_boolean
// Reads a boolean value (single byte) from the packet buffer.
bool get_boolean(PacketContext *ctx, bool *bool_val) {
  uint8_t temp_byte;
  if (get_byte(ctx, &temp_byte)) {
    *bool_val = (temp_byte != 0);
    return true;
  }
  return false;
}

// Function: get_uint32
// Reads a 32-bit unsigned integer (big-endian) from the packet buffer.
bool get_uint32(PacketContext *ctx, uint32_t *val) {
  if (ctx->read_offset + 4 <= ctx->total_len) {
    uint32_t b1 = ctx->data_buffer[ctx->read_offset];
    uint32_t b2 = ctx->data_buffer[ctx->read_offset + 1];
    uint32_t b3 = ctx->data_buffer[ctx->read_offset + 2];
    uint32_t b4 = ctx->data_buffer[ctx->read_offset + 3];
    *val = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
    ctx->read_offset += 4;
    return true;
  }
  return false;
}

// Function: get_uint64
// Reads a 64-bit unsigned integer (big-endian) from the packet buffer.
bool get_uint64(PacketContext *ctx, uint64_t *val) {
  if (ctx->read_offset + 8 <= ctx->total_len) {
    uint64_t temp_val = 0;
    for (int i = 0; i < 8; i++) {
      temp_val = (temp_val << 8) | ctx->data_buffer[ctx->read_offset + i];
    }
    *val = temp_val;
    ctx->read_offset += 8;
    return true;
  }
  return false;
}

// Function: write_byte
// Writes a single byte to the packet buffer.
bool write_byte(PacketContext *ctx, uint8_t byte_val) {
  if (ctx->read_offset + 1 < PACKET_BUFFER_MAX_SIZE) { // Check for buffer capacity
    ctx->data_buffer[ctx->read_offset] = byte_val;
    ctx->read_offset++;
    if (ctx->total_len < ctx->read_offset) { // Expand total_len if new data is written
      ctx->total_len = ctx->read_offset;
    }
    return true;
  }
  return false;
}

// Function: write_boolean
// Writes a boolean value (as 0 or 1 byte) to the packet buffer.
bool write_boolean(PacketContext *ctx, bool bool_val) {
  return write_byte(ctx, bool_val ? 1 : 0);
}

// Function: write_uint32
// Writes a 32-bit unsigned integer (big-endian) to the packet buffer.
bool write_uint32(PacketContext *ctx, uint32_t val) {
  if (ctx->read_offset + 4 < PACKET_BUFFER_MAX_SIZE) { // Check for buffer capacity
    ctx->data_buffer[ctx->read_offset] = (uint8_t)(val >> 24);
    ctx->data_buffer[ctx->read_offset + 1] = (uint8_t)(val >> 16);
    ctx->data_buffer[ctx->read_offset + 2] = (uint8_t)(val >> 8);
    ctx->data_buffer[ctx->read_offset + 3] = (uint8_t)val;
    ctx->read_offset += 4;
    if (ctx->total_len < ctx->read_offset) { // Expand total_len if new data is written
      ctx->total_len = ctx->read_offset;
    }
    return true;
  }
  return false;
}

// Function: write_uint64
// Writes a 64-bit unsigned integer (big-endian) to the packet buffer.
// Original signature took two uint32_t, where param_3 was MSB and param_2 was LSB.
bool write_uint64(PacketContext *ctx, uint32_t val_lsb, uint32_t val_msb) {
  if (ctx->read_offset + 8 < PACKET_BUFFER_MAX_SIZE) { // Check for buffer capacity
    uint64_t val = ((uint64_t)val_msb << 32) | val_lsb;
    for (int i = 0; i < 8; i++) {
      ctx->data_buffer[ctx->read_offset + i] = (uint8_t)(val >> (56 - i * 8));
    }
    ctx->read_offset += 8;
    if (ctx->total_len < ctx->read_offset) { // Expand total_len if new data is written
      ctx->total_len = ctx->read_offset;
    }
    return true;
  }
  return false;
}

// Function: write_string
// Writes a length-prefixed string to the packet buffer.
bool write_string(PacketContext *ctx, const void *str_data, size_t len) {
  if (!write_uint32(ctx, (uint32_t)len)) { // Write length prefix
    return false;
  }

  if (ctx->read_offset + len < PACKET_BUFFER_MAX_SIZE) { // Check for buffer capacity
    memcpy(ctx->data_buffer + ctx->read_offset, str_data, len);
    ctx->read_offset += len;
    if (ctx->total_len < ctx->read_offset) { // Expand total_len if new data is written
      ctx->total_len = ctx->read_offset;
    }
    return true;
  }
  return false;
}

// Function: safe_memcpy
// Copies 'len' bytes from packet buffer to 'dest', truncating if it exceeds buffer limits.
// Always returns true in original, even if truncated.
bool safe_memcpy(PacketContext *ctx, void *dest, uint16_t len) {
  uint32_t max_data_len = PACKET_BUFFER_MAX_SIZE - PACKET_DATA_OFFSET;
  uint16_t bytes_available_in_packet = 0;

  if (ctx->read_offset < max_data_len) {
    bytes_available_in_packet = (uint16_t)(max_data_len - ctx->read_offset);
  }

  uint16_t effective_len = (len < bytes_available_in_packet) ? len : bytes_available_in_packet;
  
  memcpy(dest, ctx->data_buffer + ctx->read_offset, effective_len);
  ctx->read_offset += effective_len;
  return true; // Original always returns 1.
}

// Function: safe_strdup
// Allocates memory for a string and copies 'len' bytes from the packet buffer.
bool safe_strdup(PacketContext *ctx, char **dest_str_ptr, uint16_t len) {
  char *new_str = (char *)malloc(len + 1);
  if (new_str == NULL) {
    *dest_str_ptr = NULL;
    return false;
  }

  // safe_memcpy copies bytes and advances read_offset. It might truncate.
  if (!safe_memcpy(ctx, new_str, len)) {
    free(new_str);
    *dest_str_ptr = NULL;
    return false;
  }
  new_str[len] = '\0';
  *dest_str_ptr = new_str;
  return true;
}

// Function: get_string
// Reads a length-prefixed string from the packet buffer into a provided buffer.
bool get_string(PacketContext *ctx, char *dest_buf, uint32_t max_len) {
  uint32_t str_len;
  if (!get_uint32(ctx, &str_len)) { // Read length prefix
    return false;
  }

  // Check if destination buffer is large enough for string + null terminator
  if (max_len < str_len + 1) {
    return false;
  }
  // Check if there's enough data remaining in the packet buffer
  if (ctx->read_offset + str_len > ctx->total_len) {
    return false;
  }

  if (!safe_memcpy(ctx, dest_buf, (uint16_t)str_len)) {
    return false;
  }
  dest_buf[str_len] = '\0';
  return true;
}

// Function: get_string_alloc
// Reads a length-prefixed string from the packet buffer, allocating memory for it.
bool get_string_alloc(PacketContext *ctx, char **dest_str_ptr) {
  uint32_t str_len;
  if (!get_uint32(ctx, &str_len)) { // Read length prefix
    *dest_str_ptr = NULL;
    return false;
  }

  // Check if there's enough data remaining in the packet buffer
  if (ctx->read_offset + str_len > ctx->total_len) {
    *dest_str_ptr = NULL;
    return false;
  }

  // safe_strdup allocates memory, copies string, null-terminates, and advances read_offset
  return safe_strdup(ctx, dest_str_ptr, (uint16_t)str_len);
}

// Function: recv_packet
// Receives a full packet from stdin, parses its length and padding.
// Returns the number of remaining unread bytes in the packet, or 0 on error.
int recv_packet(PacketContext *ctx) {
  clear_packet(ctx);

  // Read initial 8 bytes (which should contain packet length + padding length, and some data)
  if (recv_bytes(ctx, 8) == 0) {
    return 0;
  }

  uint32_t payload_len; // Length of the packet data following this length field
  if (!get_uint32(ctx, &payload_len)) {
    return 0;
  }

  uint8_t padding_len;
  if (!get_byte(ctx, &padding_len)) {
    return 0;
  }

  // Validate payload length and padding
  if ((payload_len == 0) || (payload_len < padding_len + 1)) {
    return 0;
  }

  // Read the rest of the packet data (payload_len includes padding_len and the actual data)
  // The first 4 bytes were payload_len, the next 1 byte was padding_len.
  // So `payload_len - 4` is the total length of data + padding to read after the length field itself.
  // We've already read 1 byte of that (padding_len), so `payload_len - 4 - 1` bytes remain from `payload_len`.
  // However, the original code uses `payload_len - 4` to signify the total length of the remaining data
  // (including the byte that was just read for padding_len).
  // This means the initial 8 bytes read contained the 4-byte length, and 4 bytes of payload.
  // `get_uint32` consumed the 4-byte length. `get_byte` consumed 1 byte of payload.
  // So 3 bytes of payload are left from the initial 8 bytes.
  // The remaining data to read is `payload_len - 4` which is the total payload after the length field.
  // If `payload_len` is 5, we read `5-4=1` byte. This 1 byte is what `get_byte` just read.
  // If `payload_len` is greater than 4, it means there's more data to read beyond what `get_byte` consumed.
  if (payload_len > 4) {
    if (recv_bytes(ctx, payload_len - 4) == 0) {
      return 0;
    }
  }
  
  // Adjust total_len by subtracting the padding_len
  ctx->total_len -= padding_len;
  return ctx->total_len - ctx->read_offset; // Return remaining unread bytes
}

// Function: send_packet
// Sends the current packet buffer content to stdout.
bool send_packet(PacketContext *ctx) {
  if (ctx->total_len >= 0x8001) { // Check max packet size (32769 bytes)
    return false;
  }

  // Calculate padding to make total_len a multiple of 8.
  // Original logic: `8 - (ctx->total_len & 7)`. If `ctx->total_len` is already a multiple of 8,
  // `ctx->total_len & 7` is 0, so padding_len becomes 8.
  uint8_t padding_len = 8 - (ctx->total_len & 7);
  
  // The original code does not explicitly fill padding bytes with random data,
  // it just increments total_len and then fwrite's the buffer.
  // So, whatever garbage is in memory for those bytes will be sent.
  ctx->total_len += padding_len; // Update total_len to include padding

  ctx->read_offset = 0; // Reset read_offset to write header fields at the beginning

  // Write the length of the packet (excluding the 4-byte length field itself)
  if (!write_uint32(ctx, ctx->total_len - 4)) {
    return false;
  }
  // Write the padding length
  if (!write_byte(ctx, padding_len)) {
    return false;
  }

  // Write the entire packet buffer (including padding) to stdout
  fwrite(ctx->data_buffer, 1, ctx->total_len, stdout);
  fflush(stdout); // Ensure data is sent immediately
  return true;
}

// Function: send_unrecognized
// Sends an "unrecognized" packet response.
void send_unrecognized(PacketContext *ctx) {
  init_packet(ctx);
  write_byte(ctx, 0x66); // Packet type 'unrecognized' (SSH_MSG_UNIMPLEMENTED)
  write_uint32(ctx, ctx->packet_id - 1); // Some ID field (sequence number?)
  send_packet(ctx);
}

// Function: send_disconnect
// Sends a "disconnect" packet.
void send_disconnect(PacketContext *ctx) {
  init_packet(ctx);
  write_byte(ctx, 100); // Packet type 'disconnect' (0x64)
  send_packet(ctx);
}

// Function: send_service_accept
// Sends a "service accept" packet.
bool send_service_accept(PacketContext *ctx, const char *service_name) {
  init_packet(ctx);
  write_byte(ctx, 0x68); // Packet type 'service accept' (SSH_MSG_SERVICE_ACCEPT)
  write_string(ctx, service_name, strlen(service_name));
  return send_packet(ctx);
}

// Function: send_userauth_failure
// Sends a "user authentication failure" packet.
bool send_userauth_failure(PacketContext *ctx) {
  init_packet(ctx);
  write_byte(ctx, 0x6f); // Packet type 'userauth failure' (SSH_MSG_USERAUTH_FAILURE)
  write_string(ctx, "password", 8); // Authentication methods that can continue
  write_boolean(ctx, false); // Partial success?
  return send_packet(ctx);
}

// Function: send_userauth_success
// Sends a "user authentication success" packet.
bool send_userauth_success(PacketContext *ctx) {
  init_packet(ctx);
  write_byte(ctx, 0x70); // Packet type 'userauth success' (SSH_MSG_USERAUTH_SUCCESS)
  return send_packet(ctx);
}

// Function: send_userauth_changereq
// Sends a "user authentication password change request" packet.
bool send_userauth_changereq(PacketContext *ctx, const char *prompt) {
  init_packet(ctx);
  write_byte(ctx, 0x78); // Packet type 'userauth password change request' (SSH_MSG_USERAUTH_PASSWD_CHANGEREQ)
  write_string(ctx, prompt, strlen(prompt));
  return send_packet(ctx);
}

// Function: userauth_password
// Handles a password authentication request.
// `username_from_packet` is the username provided by the client.
// `creds_array` is the array of valid credentials.
bool userauth_password(PacketContext *ctx, Credential *creds_array, char *username_from_packet) {
  bool auth_success = false;
  char *client_password = NULL;
  char *new_password = NULL;
  bool change_password_requested = false;
  Credential *found_cred = NULL;

  if (!get_boolean(ctx, &change_password_requested)) {
    return false;
  }
  if (!get_string_alloc(ctx, &client_password)) {
    return false;
  }

  if (change_password_requested) {
    if (!get_string_alloc(ctx, &new_password)) {
      free(client_password);
      return false;
    }
  }

  for (int i = 0; i < 16; i++) {
    if (strcmp(username_from_packet, creds_array[i].username) == 0) {
      found_cred = &creds_array[i];
      break;
    }
  }

  fprintf(stderr, "Found user %p (username=%s)\n", (void*)found_cred, username_from_packet);

  const char *stored_password;
  if (found_cred == NULL) {
    stored_password = "fakepassword"; // Default password for unknown users
  } else {
    stored_password = found_cred->password;
  }

  if (strcmp(client_password, stored_password) == 0 && found_cred != NULL) {
    auth_success = true;
  }

  if (!auth_success || new_password == NULL) { // No password change requested OR auth failed
    if (!auth_success) {
      send_userauth_failure(ctx);
    } else { // Auth success, no change request
      send_userauth_success(ctx);
      ctx->authenticated = true;
    }
  } else { // Auth success AND password change requested
    if (strlen(new_password) < 8) {
      send_userauth_changereq(ctx, "Too short");
    } else {
      free(found_cred->password); // Free old password
      found_cred->password = new_password; // Assign new password
      new_password = NULL; // Prevent double free
      send_userauth_success(ctx);
      ctx->authenticated = true;
    }
  }

  free(client_password);
  free(new_password); // Only frees if new_password was allocated but not used (e.g., too short)
  return true; // The function successfully processed the request.
}

// Function: userauth_handler
// Dispatches user authentication requests based on method name.
bool userauth_handler(PacketContext *ctx, uint8_t packet_type) {
  bool result = false;
  char *username = NULL;
  char *service_name = NULL;
  char *method_name = NULL;

  if (packet_type == 0x6e) { // SSH_MSG_USERAUTH_REQUEST ('n')
    if (get_string_alloc(ctx, &username) &&
        get_string_alloc(ctx, &service_name) && // Service name is read but not used by userauth_password
        get_string_alloc(ctx, &method_name)) {
      if (strcmp(method_name, "password") == 0) {
        result = userauth_password(ctx, global_creds_array, username);
      } else {
        result = send_userauth_failure(ctx); // Unknown authentication method
      }
    }
  } else {
    send_unrecognized(ctx); // Unrecognized packet type for userauth service
    result = false; // Sending unrecognized is a response, but the request wasn't handled as expected.
  }

  free(username);
  free(service_name);
  free(method_name);
  return result;
}

// Function: handle_service_request
// Handles a service request packet.
bool handle_service_request(PacketContext *ctx) {
  char service_name_buf[260]; // Buffer for service name
  if (!get_string(ctx, service_name_buf, sizeof(service_name_buf))) {
    return false;
  }

  if (strcmp(service_name_buf, "userauth") == 0) {
    ctx->handler_func = (PacketHandlerFunc)userauth_handler; // Assign userauth_handler
    return send_service_accept(ctx, "userauth");
  }
  return false; // Unknown service
}

// --- Main function for demonstration ---
// This main function is provided to make the code compilable and runnable.
// It sets up a basic PacketContext and initializes global_creds_array.
// It then simulates receiving and handling a packet.
int main() {
    PacketContext ctx = {0}; // Initialize all fields to zero
    ctx.packet_id = 1; // Example packet ID

    // Initialize some dummy credentials
    init_creds(global_creds_array, "user1pass1user2pass2");

    // Simulate a service request packet:
    // Length (4 bytes) | Type (1 byte) | Service Name Length (4 bytes) | Service Name (variable)
    // For "userauth":
    // Total Packet Length (excluding itself) = 1 (type) + 4 (service_name_len) + 8 (service_name) = 13
    // Padding to 8-byte multiple: 13 -> 16. Padding needed = 3 bytes.
    // Packet content will be:
    // [0x00,0x00,0x00,0x0D] (length = 13)
    // [0x03] (padding_len = 3)
    // [0x06] (SSH_MSG_SERVICE_REQUEST, 0x06) - This is the actual type of SSH_MSG_SERVICE_REQUEST
    // [0x00,0x00,0x00,0x08] (service_name_len = 8)
    // [u,s,e,r,a,u,t,h] (service_name = "userauth")
    // [P,A,D] (3 random padding bytes)

    // Example sequence of operations as if receiving and sending packets
    fprintf(stderr, "Simulating packet handling...\n");

    // Simulate input for an SSH_MSG_SERVICE_REQUEST for "userauth"
    // This data would normally come from stdin.
    // Length: 13 (0x0D)
    // Padding: 3 (0x03)
    // Packet Type: SSH_MSG_SERVICE_REQUEST (0x06)
    // Service Name Length: 8 (0x08)
    // Service Name: "userauth"
    uint8_t service_request_data[] = {
        0x00, 0x00, 0x00, 0x0D, // Packet Length (13 bytes following this field)
        0x03,                   // Padding Length (3 bytes)
        0x06,                   // SSH_MSG_SERVICE_REQUEST (0x06)
        0x00, 0x00, 0x00, 0x08, // Service Name Length (8)
        'u', 's', 'e', 'r', 'a', 'u', 't', 'h', // Service Name "userauth"
        0xDE, 0xAD, 0xBE,       // Padding bytes (value doesn't matter for recv)
        // Total 4+1+1+4+8+3 = 21 bytes.
        // But recv_bytes aligns to 8. Initial 8 bytes read by recv_packet, then rest.
        // In the `recv_packet` function, `recv_bytes(ctx, 8)` reads first 8 bytes.
        // Then `get_uint32` takes 4 bytes (0x0D). `get_byte` takes 1 byte (0x03).
        // `ctx->read_offset` is now 5. `ctx->total_len` is 8.
        // Remaining to read: `payload_len - 4 = 13 - 4 = 9` bytes.
        // `recv_bytes(ctx, 9)` would read the rest.
        // So the total length of the input should be 4 + 13 = 17 bytes (length field + payload).
        // Let's adjust the data to match the expected input for stdin.
        // The `recv_bytes` in `recv_packet` aligns, so `fread` might read more than 8 bytes if `payload_len` is large.
        // Let's just provide the exact bytes as if they were read from stdin.
    };
    // To simulate stdin, we need to redirect it or use a custom fread. For this example,
    // I'll manually set the buffer and offsets to simulate a received packet.

    // Simulate `recv_packet`
    clear_packet(&ctx);
    memcpy(ctx.data_buffer + PACKET_DATA_OFFSET, service_request_data, sizeof(service_request_data));
    ctx.total_len = sizeof(service_request_data) + PACKET_DATA_OFFSET;
    ctx.read_offset = PACKET_DATA_OFFSET;

    // Manually parse the packet header for simulation clarity
    uint32_t simulated_payload_len;
    uint8_t simulated_padding_len;
    
    // The `recv_packet` expects the buffer to be filled starting from `ctx->data_buffer`.
    // The `_padding` array is internal.
    // So the data should be put at `ctx.data_buffer` not `ctx.data_buffer + PACKET_DATA_OFFSET`.
    // Let's adjust the struct and functions to work with `ctx->data_buffer` as the actual starting point.
    // This is a major change to the struct definition if `param_1 + 0x80` is strictly adhered to.
    // Given the request for "Linux compilable C code", it's better to simplify the struct.

    // Re-evaluating struct:
    // If `param_1` is `PacketContext *ctx`, then `param_1 + 0x80` would be `(uint8_t*)ctx + 0x80`.
    // And `*(int *)(param_1 + 0x8938)` would be `*(int*)((uint8_t*)ctx + 0x8938)`.
    // My struct definition already maps this.
    // The `data_buffer` *starts* at `0x80` offset.
    // So `ctx->data_buffer[0]` is the byte at `ctx + 0x80`.

    // Simulating `recv_packet`
    // The `recv_bytes` function reads into `ctx->data_buffer + ctx->total_len`.
    // For the first `recv_bytes(ctx, 8)`, `ctx->total_len` is 0. So it reads into `ctx->data_buffer`.
    // So, `service_request_data` should be placed directly into `ctx.data_buffer`.
    // After `recv_bytes(ctx, 8)`, `ctx->total_len` becomes 8.
    // Then `get_uint32` reads from `ctx->data_buffer[0]` to `[3]`. `ctx->read_offset` becomes 4.
    // `get_byte` reads from `ctx->data_buffer[4]`. `ctx->read_offset` becomes 5.
    // So, the `service_request_data` should be placed at the beginning of `ctx.data_buffer`.

    clear_packet(&ctx);
    memcpy(ctx.data_buffer, service_request_data, sizeof(service_request_data));
    ctx.total_len = sizeof(service_request_data); // Total data bytes available in buffer
    ctx.read_offset = 0; // Start reading from the beginning

    // This simulates the actual parsing part of `recv_packet`.
    // `recv_packet` itself handles the `fread` and then parsing.
    // So, to truly simulate `recv_packet`, we need actual stdin.
    // Let's make a dummy stdin.
    FILE *original_stdin = stdin;
    FILE *dummy_stdin = fmemopen(service_request_data, sizeof(service_request_data), "rb");
    if (!dummy_stdin) {
        perror("fmemopen failed");
        return 1;
    }
    stdin = dummy_stdin;

    fprintf(stderr, "Calling recv_packet for service request...\n");
    int remaining_bytes = recv_packet(&ctx);
    if (remaining_bytes == 0) {
        fprintf(stderr, "recv_packet failed for service request.\n");
        fclose(dummy_stdin);
        stdin = original_stdin;
        return 1;
    }
    fprintf(stderr, "recv_packet successful. Remaining bytes: %d\n", remaining_bytes);

    // Now call the handler
    fprintf(stderr, "Calling handle_service_request...\n");
    if (!handle_service_request(&ctx)) {
        fprintf(stderr, "handle_service_request failed.\n");
        fclose(dummy_stdin);
        stdin = original_stdin;
        return 1;
    }
    fprintf(stderr, "handle_service_request successful. Handler assigned.\n");

    // Simulate an SSH_MSG_USERAUTH_REQUEST for "user1" with password "pass1"
    // Packet Type: SSH_MSG_USERAUTH_REQUEST (0x6E)
    // Username Length: 5 | Username: "user1"
    // Service Name Length: 8 | Service Name: "ssh-connection"
    // Method Name Length: 8 | Method Name: "password"
    // Change Password Request: 0x00 (false)
    // Password Length: 5 | Password: "pass1"

    uint8_t userauth_request_data[] = {
        0x00, 0x00, 0x00, 0x22, // Packet Length (34 bytes following this field)
        0x06,                   // Padding Length (6 bytes)
        0x6E,                   // SSH_MSG_USERAUTH_REQUEST (0x6E)
        0x00, 0x00, 0x00, 0x05, // Username Length (5)
        'u', 's', 'e', 'r', '1', // Username "user1"
        0x00, 0x00, 0x00, 0x0E, // Service Name Length (14)
        's', 's', 'h', '-', 'c', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', // Service Name "ssh-connection"
        0x00, 0x00, 0x00, 0x08, // Method Name Length (8)
        'p', 'a', 's', 's', 'w', 'o', 'r', 'd', // Method Name "password"
        0x00,                   // Change Password Request (false)
        0x00, 0x00, 0x00, 0x05, // Password Length (5)
        'p', 'a', 's', 's', '1', // Password "pass1"
        0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD // Padding
        // Total: 4 (len) + 1 (padlen) + 1 (type) + 4 (ulen) + 5 (user) + 4 (slen) + 14 (service) + 4 (mlen) + 8 (method) + 1 (changereq) + 4 (plen) + 5 (pass) + 6 (padding) = 61 bytes
        // Payload length = 61 - 4 = 57 (0x39)
        // Let's re-calculate:
        // Type: 1
        // Uname: 4+5=9
        // Sname: 4+14=18
        // Mname: 4+8=12
        // Change: 1
        // Pass: 4+5=9
        // Total payload excluding initial length = 1+9+18+12+1+9 = 50.
        // Let's use 50 (0x32) as payload_len.
        // 50 % 8 = 2. Padding needed = 6.
        // So `payload_len` should be 50. `padding_len` should be 6.
    };

    // Corrected userauth_request_data:
    uint8_t userauth_request_data_corrected[] = {
        0x00, 0x00, 0x00, 0x32, // Packet Length (50 bytes following this field)
        0x06,                   // Padding Length (6 bytes)
        0x6E,                   // SSH_MSG_USERAUTH_REQUEST (0x6E)
        0x00, 0x00, 0x00, 0x05, // Username Length (5)
        'u', 's', 'e', 'r', '1', // Username "user1"
        0x00, 0x00, 0x00, 0x0E, // Service Name Length (14)
        's', 's', 'h', '-', 'c', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', // Service Name "ssh-connection"
        0x00, 0x00, 0x00, 0x08, // Method Name Length (8)
        'p', 'a', 's', 's', 'w', 'o', 'r', 'd', // Method Name "password"
        0x00,                   // Change Password Request (false)
        0x00, 0x00, 0x00, 0x05, // Password Length (5)
        'p', 'a', 's', 's', '1', // Password "pass1"
        0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD // Padding (6 bytes)
    };

    fclose(dummy_stdin);
    dummy_stdin = fmemopen(userauth_request_data_corrected, sizeof(userauth_request_data_corrected), "rb");
    if (!dummy_stdin) {
        perror("fmemopen failed");
        return 1;
    }
    stdin = dummy_stdin;

    fprintf(stderr, "\nCalling recv_packet for userauth request...\n");
    remaining_bytes = recv_packet(&ctx);
    if (remaining_bytes == 0) {
        fprintf(stderr, "recv_packet failed for userauth request.\n");
        fclose(dummy_stdin);
        stdin = original_stdin;
        return 1;
    }
    fprintf(stderr, "recv_packet successful. Remaining bytes: %d\n", remaining_bytes);

    // Call the assigned handler (userauth_handler)
    fprintf(stderr, "Calling userauth_handler...\n");
    if (!ctx.handler_func) {
        fprintf(stderr, "No handler function assigned.\n");
        fclose(dummy_stdin);
        stdin = original_stdin;
        return 1;
    }
    ctx.handler_func(&ctx, 0x6E); // Pass the packet type for userauth_request

    fprintf(stderr, "Userauth handler processed. Authenticated: %s\n", ctx.authenticated ? "true" : "false");

    // Clean up
    fclose(dummy_stdin);
    stdin = original_stdin;

    // Free allocated credentials
    for (int i = 0; i < 16; i++) {
        free(global_creds_array[i].username);
        free(global_creds_array[i].password);
    }

    return 0;
}