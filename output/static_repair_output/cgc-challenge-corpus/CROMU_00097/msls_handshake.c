#include <stdlib.h> // For calloc, free, size_t
#include <string.h> // For memcpy
#include <stdint.h> // For fixed-width integer types
#include <stdio.h>  // For debug_print (if it's printf-like)

// --- Type Definitions (from decompiler analysis) ---
// These types are used to explicitly represent sizes based on decompiler output.
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;
typedef uint16_t ushort; // Original 'ushort'
typedef uint32_t uint;   // Original 'uint'

// --- External Function Prototypes ---
// These functions are assumed to be defined elsewhere in the project.
void msls_send_error_msg(int code, int sub_code);
void debug_print(const char* format, ...); // Assuming printf-like behavior
void msls_send_finish(uint32_t* connection_context);
void msls_send_server_hello(uint32_t* connection_context);
void msls_send_keyx(uint32_t* connection_context);
void msls_send_hello_done(uint32_t* connection_context);
void msls_send_msg(void* msg);
void msls_destroy_msg(void* msg);
uint32_t get_random();

// Forward declarations for functions defined later in this snippet
// Signatures corrected based on usage and common patterns for connection IDs.
int32_t* msls_lookup_context(int32_t server_context_ptr, uint32_t connection_id);
uint32_t* msls_get_connection(int32_t server_context_ptr, uint32_t connection_id);
void destroy_context(int32_t connection_context_ptr);

// --- Global/Constant Data Pointers ---
// These are hardcoded addresses from the original snippet.
// For compilation, they are replaced with pointers to static dummy arrays.
// In a real system, these would likely be mapped memory regions or global arrays.
static const uint32_t DUMMY_DATA_4347C600[0x80] = {0}; // Placeholder for 0x200 bytes
static const uint32_t DUMMY_DATA_4347C400[0x80] = {0}; // Placeholder for 0x200 bytes
static const uint32_t DUMMY_DATA_4347C000[0x80] = {0}; // Placeholder for 0x200 bytes
static const uint32_t DUMMY_DATA_4347C200[0x80] = {0}; // Placeholder for 0x200 bytes
static const uint32_t DUMMY_DATA_10D1F080[0x80] = {0}; // Placeholder for 0x200 bytes
static const uint32_t DUMMY_DATA_10D1F000[0x80] = {0}; // Placeholder for 0x200 bytes

const uint32_t* const_data_4347c600 = DUMMY_DATA_4347C600;
const uint32_t* const_data_4347c400 = DUMMY_DATA_4347C400;
const uint32_t* const_data_4347c000 = DUMMY_DATA_4347C000;
const uint32_t* const_data_4347c200 = DUMMY_DATA_4347C200;
const uint32_t* const_data_10d1f080 = DUMMY_DATA_10D1F080;
const uint32_t* const_data_10d1f000 = DUMMY_DATA_10D1F000;

// Function: msls_handle_handshake
void msls_handle_handshake(int32_t server_context_ptr, int32_t incoming_msg_ptr) {
  if (server_context_ptr == 0 || incoming_msg_ptr == 0) {
    return;
  }

  // Cast incoming_msg_ptr to a byte pointer for correct offset arithmetic
  uint8_t* msg_base = (uint8_t*)incoming_msg_ptr;

  // Check message length in the header
  if (*(ushort *)(msg_base + 0x10) < 0xc) {
    msls_send_error_msg(0xe0, 0xa1);
    return;
  }

  uint16_t* p_msg_data = *(uint16_t**)(msg_base + 0x14); // Pointer to the actual message payload
  uint32_t connection_id_from_msg = *(uint32_t*)(p_msg_data + 3);
  uint32_t connection_id_from_param = *(uint32_t*)(msg_base + 0xc);

  if (connection_id_from_msg != connection_id_from_param) {
    debug_print("client_id %x != connection id %x\n", connection_id_from_msg, connection_id_from_param);
    msls_send_error_msg(0xe3, 0xa1);
    return;
  }

  // Look up the connection context using the server context and connection ID
  int32_t* conn_context_ptr = msls_lookup_context(server_context_ptr, connection_id_from_param);

  ushort message_type = *p_msg_data; // The first field in the message payload is the message type

  if (message_type == 0x59) { // Received Client Finished
    debug_print("received client finished\n");
    if (conn_context_ptr == 0) {
      msls_send_error_msg(0xe3, 0xa1);
    } else if (((*(uint8_t *)(conn_context_ptr + 4) == 0) || (*(uint8_t *)(conn_context_ptr + 0xe) == 0)) ||
               (*(uint8_t *)(conn_context_ptr + 0xd) == 0)) {
      msls_send_error_msg(0xe5, 0xa0);
    } else if (p_msg_data[5] == 0x200) { // Check expected payload length
      int32_t msg_seq_num = *(int32_t*)(p_msg_data + 1);
      *(uint32_t*)((uint8_t*)conn_context_ptr + 0x8) += 1; // Increment sequence number

      if (msg_seq_num == *(int32_t*)((uint8_t*)conn_context_ptr + 0x8)) {
        for (int i = 0; i < 0x80; ++i) {
          if (*(uint32_t*)(p_msg_data + i * 2 + 6) !=
              (*(uint32_t*)(*(int32_t*)((uint8_t*)conn_context_ptr + 0x24) + i * 4) ^
               *(uint32_t*)(*(int32_t*)((uint8_t*)conn_context_ptr + 0x20) + i * 4) ^
               *(uint32_t*)(*(int32_t*)((uint8_t*)conn_context_ptr + 0x18) + i * 4))) {
            msls_send_error_msg(0xe7, 0xa1);
            return;
          }
        }
        *(uint8_t*)((uint8_t*)conn_context_ptr + 0xc) = 1; // Mark as finished
        *(uint8_t*)((uint8_t*)conn_context_ptr + 4) = 0;   // Reset some state flag
      } else {
        msls_send_error_msg(0xe6, 0xa0);
      }
    } else {
      debug_print("expected length: %d got length: %d\n", 0x200, p_msg_data[5]);
      msls_send_error_msg(0xe0, 0xa1);
    }
  } else if (message_type < 0x5a) {
    if (message_type == 0x58) { // Received Client Done
      debug_print("Received Client Done\n");
      if (conn_context_ptr == 0) {
        msls_send_error_msg(0xe3, 0xa1);
        return;
      }
      if ((*(uint8_t*)((uint8_t*)conn_context_ptr + 4) == 0) || (*(uint8_t*)((uint8_t*)conn_context_ptr + 0xe) == 0)) {
        msls_send_error_msg(0xe5, 0xa0);
        return;
      }
      if (p_msg_data[5] != 0) { // Expected payload length 0
        debug_print("expected length: %d got length: %d\n", 0, p_msg_data[5]);
        msls_send_error_msg(0xe0, 0xa1);
        return;
      }
      int32_t msg_seq_num = *(int32_t*)(p_msg_data + 1);
      *(uint32_t*)((uint8_t*)conn_context_ptr + 0x8) += 1; // Increment sequence number
      if (msg_seq_num != *(int32_t*)((uint8_t*)conn_context_ptr + 0x8)) {
        msls_send_error_msg(0xe6, 0xa0);
        return;
      }
      msls_send_finish((uint32_t*)conn_context_ptr);
      *(uint8_t*)((uint8_t*)conn_context_ptr + 0xd) = 1; // Mark as done
      return;
    }
    if (message_type < 0x59) {
      if (message_type == 0x51) { // Received Client Hello
        debug_print("Received Client Hello\n");
        if (p_msg_data[5] != 0x222) { // Check expected payload length
          msls_send_error_msg(0xe0, 0xa1);
          return;
        }
        for (int i = 0; i < 0x80; ++i) {
          if (*(uint32_t*)(p_msg_data + i * 2 + 0xb) !=
              (connection_id_from_param ^ *(uint32_t*)(*(int32_t*)((uint8_t*)server_context_ptr + 0x88) + i * 4))) {
            debug_print("Cookie failed\n");
            msls_send_hello_verify(server_context_ptr, connection_id_from_param);
            return;
          }
        }
        debug_print("Done checking cookie\n");
        // Get or create connection context
        uint32_t* new_conn_context_ptr = msls_get_connection(server_context_ptr, connection_id_from_param);
        if (new_conn_context_ptr == 0) {
          msls_send_error_msg(0xe3, 0xa2);
          *(uint8_t*)((uint8_t*)server_context_ptr + 1) = 1; // Set error flag on server context
          return;
        }
        *(uint8_t*)((uint8_t*)new_conn_context_ptr + 0xc) = 0; // Reset state flags
        *(uint8_t*)((uint8_t*)new_conn_context_ptr + 4) = 1;
        *(uint8_t*)((uint8_t*)new_conn_context_ptr + 0xe) = 0;
        *(uint8_t*)((uint8_t*)new_conn_context_ptr + 0xd) = 0;
        *(uint32_t*)((uint8_t*)new_conn_context_ptr + 0x8) = 0; // Reset sequence number
        uint32_t random_val = get_random();
        // *(ushort *)(iVar3 + 0x10) = puVar5[uVar4 % 0xb + 0x10b];
        *(ushort*)((uint8_t*)new_conn_context_ptr + 0x10) = p_msg_data[(random_val % 0xb) + 0x10b];
        msls_send_server_hello(new_conn_context_ptr);
        msls_send_keyx(new_conn_context_ptr);
        msls_send_hello_done(new_conn_context_ptr);
        if (*(int32_t*)((uint8_t*)new_conn_context_ptr + 0x28) != 0) {
          free(*(void**)((uint8_t*)new_conn_context_ptr + 0x28));
          *(uint32_t*)((uint8_t*)new_conn_context_ptr + 0x28) = 0;
        }
        return;
      }
      if (message_type == 0x56) { // Received Client Keyx
        debug_print("Received Client Keyx\n");
        if (conn_context_ptr == 0) {
          msls_send_error_msg(0xe3, 0xa1);
          return;
        }
        if (*(uint8_t*)((uint8_t*)conn_context_ptr + 4) == 0) { // Check state flag
          msls_send_error_msg(0xe5, 0xa0);
          return;
        }
        if (p_msg_data[5] != 0x400) { // Check expected payload length
          msls_send_error_msg(0xe0, 0xa1);
          return;
        }
        int32_t msg_seq_num = *(int32_t*)(p_msg_data + 1);
        *(uint32_t*)((uint8_t*)conn_context_ptr + 0x8) += 1; // Increment sequence number
        if (msg_seq_num != *(int32_t*)((uint8_t*)conn_context_ptr + 0x8)) {
          msls_send_error_msg(0xe6, 0xa0);
          return;
        }
        uint16_t* p_keyx_data = p_msg_data + 6; // Offset to actual key exchange data

        // Allocate memory for client_keyx_data and copy
        void* client_keyx_data_ptr = calloc(1, 0x200);
        *(void**)((uint8_t*)conn_context_ptr + 0x14) = client_keyx_data_ptr;
        memcpy(client_keyx_data_ptr, p_keyx_data, 0x200);
        *(uint8_t*)((uint8_t*)conn_context_ptr + 0xe) = 1; // Mark keyx received

        // Allocate memory for client_keyx_material and compute
        void* client_keyx_material_ptr = calloc(1, 0x200);
        *(void**)((uint8_t*)conn_context_ptr + 0x1c) = client_keyx_material_ptr;
        for (int i = 0; i < 0x80; ++i) {
          *(uint32_t*)((uint8_t*)client_keyx_material_ptr + i * 4) =
               *(uint32_t*)(p_keyx_data + (i + 0x80) * 2) ^
               *(uint32_t*)(*(int32_t*)((uint8_t*)conn_context_ptr + 0x18) + i * 4);
        }
        return;
      }
    }
  }
  // Default error for unhandled message type or unexpected flow
  msls_send_error_msg(0xe4, 0xa2);
  *(uint8_t*)((uint8_t*)server_context_ptr + 1) = 1; // Set error flag on server context
}

// Function: msls_send_hello_verify
void msls_send_hello_verify(int32_t server_context_ptr, uint32_t connection_id) {
  // calloc(size_in_bytes, 1) is used instead of decompiler's `in_stack_ffffffd8`
  uint16_t* p_msg_container = (uint16_t*)calloc(1, 0x18);
  uint16_t* p_handshake_msg = (uint16_t*)calloc(1, 0x10);
  uint16_t* p_data_payload = (uint16_t*)calloc(1, 0x202);

  *p_data_payload = 0xff01; // Magic/version
  for (int i = 0; i < 0x80; ++i) {
    *(uint32_t*)(p_data_payload + i * 2 + 1) =
         *(uint32_t*)(*(int32_t*)((uint8_t*)server_context_ptr + 0x88) + i * 4) ^ connection_id;
  }

  *p_handshake_msg = 0x52; // Message type (Server Hello Verify)
  *(uint32_t*)(p_handshake_msg + 1) = 0; // Sequence number (assuming 0 for verify)
  *(uint32_t*)(p_handshake_msg + 3) = connection_id;
  p_handshake_msg[5] = 0x202; // Length of payload
  *(uint16_t**)(p_handshake_msg + 6) = p_data_payload; // Pointer to payload

  *p_msg_container = 0x21d; // Total length of message
  p_msg_container[1] = 0xff01; // Version
  *(uint8_t*)(p_msg_container + 2) = 3; // Type
  *(uint32_t*)(p_msg_container + 6) = connection_id;
  p_msg_container[8] = 0x20e; // Length of sub-message (handshake message + its header)
  *(uint16_t**)(p_msg_container + 10) = p_handshake_msg; // Pointer to handshake message

  msls_send_msg(p_msg_container);
  msls_destroy_msg(p_msg_container); // This should ideally free p_handshake_msg and p_data_payload as well
  return;
}

// Function: msls_send_server_hello
void msls_send_server_hello(uint32_t* connection_context) {
  uint16_t* p_msg_container = (uint16_t*)calloc(1, 0x18);
  uint16_t* p_handshake_msg = (uint16_t*)calloc(1, 0x10);
  uint16_t* p_data_payload = (uint16_t*)calloc(1, 8); // 8 bytes

  *p_data_payload = 0xff01; // Magic/version
  uint32_t random_val = get_random();
  *(uint32_t*)(p_data_payload + 1) = random_val;
  // Accessing connection_context fields using byte offsets for clarity with decompiler output
  p_data_payload[3] = *(uint16_t*)((uint8_t*)connection_context + 0x10);

  *p_handshake_msg = 0x53; // Message type (Server Hello)
  connection_context[2] += 1; // Increment sequence number at offset 0x8 (if connection_context is uint32_t*)
  *(uint32_t*)(p_handshake_msg + 1) = connection_context[2];
  *(uint32_t*)(p_handshake_msg + 3) = connection_context[0]; // Connection ID
  p_handshake_msg[5] = 8; // Length of payload
  *(uint16_t**)(p_handshake_msg + 6) = p_data_payload; // Pointer to payload

  *p_msg_container = 0x23; // Total length of message
  p_msg_container[1] = 0xff01; // Version
  *(uint8_t*)(p_msg_container + 2) = 3; // Type
  *(uint32_t*)(p_msg_container + 6) = connection_context[0]; // Connection ID
  p_msg_container[8] = 0x14; // Length of sub-message
  *(uint16_t**)(p_msg_container + 10) = p_handshake_msg; // Pointer to handshake message

  msls_send_msg(p_msg_container);
  msls_destroy_msg(p_msg_container);
  return;
}

// Function: msls_send_keyx
void msls_send_keyx(uint32_t* connection_context) {
  uint16_t* p_msg_container = (uint16_t*)calloc(1, 0x18);
  uint16_t* p_handshake_msg = (uint16_t*)calloc(1, 0x10);
  void* p_data_payload = calloc(1, 0x200);
  void* server_keyx_material_ptr = calloc(1, 0x200);

  // Store pointer to server key exchange material in connection_context (offset 0x18)
  *(void**)((uint8_t*)connection_context + 0x18) = server_keyx_material_ptr;

  for (int i = 0; i < 0x80; ++i) {
    *(uint32_t*)((uint8_t*)server_keyx_material_ptr + i * 4) =
         const_data_4347c600[i] ^ const_data_4347c400[i];
    // Copy the generated material to the payload
    *(uint32_t*)((uint8_t*)p_data_payload + i * 4) = *(uint32_t*)((uint8_t*)server_keyx_material_ptr + i * 4);
  }

  *p_handshake_msg = 0x55; // Message type (Server Key Exchange)
  connection_context[2] += 1; // Increment sequence number
  *(uint32_t*)(p_handshake_msg + 1) = connection_context[2];
  *(uint32_t*)(p_handshake_msg + 3) = connection_context[0]; // Connection ID
  p_handshake_msg[5] = 0x200; // Length of payload
  *(void**)(p_handshake_msg + 6) = p_data_payload; // Pointer to payload

  *p_msg_container = 0x21b; // Total length of message
  p_msg_container[1] = 0xff01; // Version
  *(uint8_t*)(p_msg_container + 2) = 3; // Type
  *(uint32_t*)(p_msg_container + 6) = connection_context[0]; // Connection ID
  p_msg_container[8] = 0x20c; // Length of sub-message
  *(uint16_t**)(p_msg_container + 10) = p_handshake_msg; // Pointer to handshake message

  msls_send_msg(p_msg_container);
  msls_destroy_msg(p_msg_container);
  return;
}

// Function: msls_send_hello_done
void msls_send_hello_done(uint32_t* connection_context) {
  uint16_t* p_msg_container = (uint16_t*)calloc(1, 0x18);
  uint16_t* p_handshake_msg = (uint16_t*)calloc(1, 0x10);

  *p_handshake_msg = 0x57; // Message type (Server Hello Done)
  connection_context[2] += 1; // Increment sequence number
  *(uint32_t*)(p_handshake_msg + 1) = connection_context[2];
  *(uint32_t*)(p_handshake_msg + 3) = connection_context[0]; // Connection ID
  p_handshake_msg[5] = 0; // Length of payload (0 for Hello Done)
  *(uint32_t*)(p_handshake_msg + 6) = 0; // No payload pointer

  *p_msg_container = 0x1b; // Total length of message
  p_msg_container[1] = 0xff01; // Version
  *(uint8_t*)(p_msg_container + 2) = 3; // Type
  *(uint32_t*)(p_msg_container + 6) = connection_context[0]; // Connection ID
  p_msg_container[8] = 0xc; // Length of sub-message
  *(uint16_t**)(p_msg_container + 10) = p_handshake_msg; // Pointer to handshake message

  msls_send_msg(p_msg_container);
  msls_destroy_msg(p_msg_container);
  return;
}

// Function: msls_send_finish
void msls_send_finish(uint32_t* connection_context) {
  uint16_t* p_msg_container = (uint16_t*)calloc(1, 0x18);
  uint16_t* p_handshake_msg = (uint16_t*)calloc(1, 0x10);
  void* p_data_payload = calloc(1, 0x200);
  void* server_finish_material_ptr = calloc(1, 0x200);

  // Store pointer to server finish material in connection_context (offset 0x20)
  *(void**)((uint8_t*)connection_context + 0x20) = server_finish_material_ptr;

  for (int i = 0; i < 0x80; ++i) {
    *(uint32_t*)((uint8_t*)server_finish_material_ptr + i * 4) =
         const_data_4347c400[i] ^ const_data_4347c200[i];
  }

  for (int i = 0; i < 0x80; ++i) {
    *(uint32_t*)((uint8_t*)p_data_payload + i * 4) =
         *(uint32_t*)((uint8_t*)server_finish_material_ptr + i * 4) ^
         *(uint32_t*)(*(int32_t*)((uint8_t*)connection_context + 0x1c) + i * 4) ^ // Client keyx material
         *(uint32_t*)(*(int32_t*)((uint8_t*)connection_context + 0x14) + i * 4) ^ // Client keyx data
         *(uint32_t*)(*(int32_t*)((uint8_t*)connection_context + 0x24) + i * 4);   // Unknown material
  }

  *p_handshake_msg = 0x59; // Message type (Server Finished)
  connection_context[2] += 1; // Increment sequence number
  *(uint32_t*)(p_handshake_msg + 1) = connection_context[2];
  *(uint32_t*)(p_handshake_msg + 3) = connection_context[0]; // Connection ID
  p_handshake_msg[5] = 0x200; // Length of payload
  *(void**)(p_handshake_msg + 6) = p_data_payload; // Pointer to payload

  *p_msg_container = 0x21b; // Total length of message
  p_msg_container[1] = 0xff01; // Version
  *(uint8_t*)(p_msg_container + 2) = 3; // Type
  *(uint32_t*)(p_msg_container + 6) = connection_context[0]; // Connection ID
  p_msg_container[8] = 0x20c; // Length of sub-message
  *(uint16_t**)(p_msg_container + 10) = p_handshake_msg; // Pointer to handshake message

  msls_send_msg(p_msg_container);
  msls_destroy_msg(p_msg_container);
  return;
}

// Function: msls_get_connection
uint32_t* msls_get_connection(int32_t server_context_ptr, uint32_t connection_id) {
  if (server_context_ptr == 0) {
    return NULL;
  }

  uint8_t* server_base = (uint8_t*)server_context_ptr;
  uint32_t* conn_context_ptr = (uint32_t*)msls_lookup_context(server_context_ptr, connection_id);

  if (conn_context_ptr == NULL) {
    // Connection not found, try to create a new one
    for (int i = 0; i < 0x20; ++i) {
      // Check if slot `i` in the server's connection array is empty (value 0)
      if (*(int32_t*)(server_base + 8 + i * 4) == 0) {
        // Allocate 0x2c bytes for the connection context structure
        conn_context_ptr = (uint32_t*)calloc(1, 0x2c);

        debug_print("Created connection at %p\n", (void*)conn_context_ptr);
        conn_context_ptr[0] = connection_id; // Store connection ID at the beginning of the context
        *(uint32_t**)(server_base + 8 + i * 4) = conn_context_ptr; // Store pointer in server's array

        void* dest_ptr1 = calloc(1, 0x200);
        void* dest_ptr2 = calloc(1, 0x200);

        memcpy(dest_ptr1, (void*)const_data_4347c000, 0x200);
        memcpy(dest_ptr2, (void*)const_data_4347c200, 0x200);

        for (int j = 0; j < 0x80; ++j) {
          *(uint32_t*)((uint8_t*)dest_ptr1 + j * 4) =
               *(uint32_t*)((uint8_t*)dest_ptr2 + j * 4) ^ *(uint32_t*)((uint8_t*)dest_ptr1 + j * 4) ^
               connection_id;
        }
        debug_print("leaving some magic at: %p\n", dest_ptr2);
        conn_context_ptr[10] = (uint32_t)(uintptr_t)dest_ptr2; // Store pointer at offset 0x28 (10*4)
        conn_context_ptr[9] = (uint32_t)(uintptr_t)dest_ptr1;  // Store pointer at offset 0x24 (9*4)
        return conn_context_ptr;
      }
    }
    return NULL; // No empty slot found
  }
  return conn_context_ptr; // Existing connection found
}

// Function: msls_lookup_context
// Looks up a connection context within a server context by connection ID.
int32_t* msls_lookup_context(int32_t server_context_ptr, uint32_t connection_id) {
  if (server_context_ptr != 0) {
    uint8_t* server_base = (uint8_t*)server_context_ptr;
    for (int i = 0; i < 0x20; ++i) {
      int32_t* current_conn_context_ptr = *(int32_t**)(server_base + 8 + i * 4);
      if ((current_conn_context_ptr != NULL) && (connection_id == *(uint32_t*)current_conn_context_ptr)) {
        return current_conn_context_ptr;
      }
    }
  }
  return NULL;
}

// Function: msls_destroy_connection
// Destroys a connection context given the server context and the connection ID.
undefined** msls_destroy_connection(undefined** server_context_ptr, uint32_t connection_id_to_destroy) {
  if (server_context_ptr != NULL) {
    for (int i = 0; i < 0x20; ++i) {
      undefined** current_conn_context_ptr = (undefined**)server_context_ptr[i + 2]; // Access connection context pointer
      if ((current_conn_context_ptr != NULL) &&
          (connection_id_to_destroy == *(uint32_t*)current_conn_context_ptr)) { // Compare connection ID
        destroy_context((int32_t)(uintptr_t)current_conn_context_ptr); // Destroy the context's allocated memory
        server_context_ptr[i + 2] = NULL; // Clear the slot in the server's array
        return server_context_ptr; // Return the server context pointer
      }
    }
  }
  return NULL; // Return NULL if not found or server_context_ptr is NULL
}

// Function: msls_set_cookie
void msls_set_cookie(int32_t server_context_ptr) {
  void* cookie_data_ptr = calloc(1, 0x200);
  *(void**)((uint8_t*)server_context_ptr + 0x88) = cookie_data_ptr; // Store pointer to cookie data

  for (int i = 0; i < 0x80; ++i) {
    *(uint32_t*)((uint8_t*)cookie_data_ptr + i * 4) =
         const_data_10d1f080[i] ^ const_data_10d1f000[i];
  }
  return;
}

// Function: destroy_context
// Frees memory associated with a connection context.
void destroy_context(int32_t connection_context_ptr) {
  if (connection_context_ptr != 0) {
    uint8_t* conn_base = (uint8_t*)connection_context_ptr;

    // Free dynamically allocated buffers within the connection context
    free(*(void**)(conn_base + 0x24)); // Corresponds to conn_context_ptr[9]
    free(*(void**)(conn_base + 0x14)); // Client key exchange data
    free(*(void**)(conn_base + 0x18)); // Server key exchange material
    free(*(void**)(conn_base + 0x20)); // Server finish material
    free(*(void**)(conn_base + 0x1c)); // Client key exchange material
    
    // Finally, free the connection context structure itself
    free((void*)connection_context_ptr);
  }
  return;
}