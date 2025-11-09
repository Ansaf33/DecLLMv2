#include <stdio.h>    // For fprintf, stderr
#include <stdlib.h>   // For calloc, free, exit
#include <string.h>   // For memcpy, memcmp, memset, strlen, memmove
#include <stdbool.h>  // For bool
#include <stdint.h>   // For uint32_t, uint16_t, intptr_t
#include <unistd.h>   // For standard socket functions (if recv_all/send_response were wrappers)

// --- Type Definitions ---
typedef int32_t audit_id_t;        // Represents integer IDs, possibly pointers in original context
typedef uint32_t response_status_t; // Represents function return status (0 for success, 0xffffffff for fail)
typedef unsigned int uint;         // For 'uint' from original snippet

// --- Global Variables (from decompiled code) ---
// These are likely pointers to linked list heads or similar data structures.
// They are initialized to NULL for compilation.
struct AuditRecord *audit_list = NULL;
void *tp_list = NULL;
void *s_list = NULL;

// --- Custom Structures (inferred from usage) ---

// AuditRecord structure based on calloc(0x10) and next pointer at offset 0xc
typedef struct AuditRecord {
    audit_id_t taxpayer_id;
    int years[2]; // Implicitly 2 years based on 0x10 allocation and 0xc offset for 'next'
    struct AuditRecord *next; // At offset 0xc (assuming int is 4 bytes)
} AuditRecord;

// Session structure inferred from memcmp operations in session_auth and logout
typedef struct Session {
    char username_creds[0x18]; // 0xc bytes for username, another 0xc bytes at offset 0x18 for more creds
    void *data_at_0x2e;        // Pointer to dynamically allocated data, freed on logout
    // Other fields would fill the struct to a certain size
} Session;

// RequestBuffer structure for incoming commands
// Total size 0x2e (46) bytes as used in main and recv_cmd_data
typedef struct {
    char command_name_and_key[12]; // Command string (e.g., "LOGIN") and part of a key/ID
    char data[0x2e - 12];          // Remaining data for the command arguments
} RequestBuffer;

// ResponseBuffer structure for outgoing responses
// Total size 0x94 (148) bytes as used in main for send
typedef struct {
    char key_data[12];             // Copied from request_buffer by response_set_key
    char message_and_data[148 - 12]; // Buffer for response messages and any additional data
} ResponseBuffer;

// --- External Function Declarations ---
// These functions are not defined in the snippet but are called.
// Dummy implementations are provided below for compilation purposes.

// Error termination function
void _terminate(int status) {
    fprintf(stderr, "Program terminated with status: %d\n", status);
    exit(status);
}

// Audit related
extern int taxpayer_count_submitted_tax_years(audit_id_t taxpayer_id);
extern void tenfourd_get_last_three_from_list(uint32_t taxpayer_data_field, int count, int *years_buffer);

// Taxpayer/Session related
extern audit_id_t taxpayer_get_by_username(void *list_head, const void *username_data);
extern audit_id_t session_get_by_username(void *list_head, const void *username_data);
extern void session_append(void **list_head, const void *data);
extern void *session_remove(void **list_head, const void *username_data);

extern int taxpayer_get_refund(audit_id_t taxpayer_id, const void *param_2_data, uint32_t *buffer, uint *size);
extern int taxpayer_pay_taxes(audit_id_t taxpayer_id, const void *param_2_data, uint32_t *buffer, uint *size);
extern int taxpayer_list_submitted_tax_years(audit_id_t taxpayer_id, const void *param_2_data, uint32_t *buffer, uint32_t *size);
extern int taxpayer_sum_taxes_due(audit_id_t taxpayer_id, const void *param_2_data, uint32_t *amount_due);
extern int taxpayer_add_tenfourdee(audit_id_t taxpayer_id, const void *param_2_data, size_t data_size);
extern int taxpayer_compare_creds(audit_id_t taxpayer_id, const void *creds_ptr);
extern void taxpayer_new(const void *param_1_data, const void *param_2_data, void **tp_list_head);

// Network/IO related wrappers (simplified for compilation)
size_t recv_all(void *buffer, size_t size) {
    memset(buffer, 0, size); // Simulate receiving zeroed data
    return size; // Assume all requested bytes are received
}

int send_response(const void *buffer, size_t size) {
    (void)buffer; // Unused parameter
    (void)size;   // Unused parameter
    return size; // Assume all bytes sent
}

// --- String constants (from main) ---
const char LOGIN_CMD[] = "LOGIN";
const char LOGOUT_CMD[] = "LOGOUT";
const char UPLOAD_FORM_CMD[] = "UPLOAD_FORM";
const char TAXES_DUE_CMD[] = "TAXES_DUE";
const char TAXES_SUBMITTED_CMD[] = "TAXES_SUBMITTED";
const char PAY_TAXES_CMD[] = "PAY_TAXES";
const char GET_REFUND_CMD[] = "GET_REFUND";
const char TRIGGER_AUDIT_CMD[] = "TRIGGER_AUDIT";
const char TRIGGER_AUDIT2_CMD[] = "TRIGGER_AUDIT2";
const char CREATE_ACCOUNT_CMD[] = "CREATE_ACCOUNT";

// --- Dummy Implementations for External Functions ---
// These are minimal implementations to allow the provided code to compile.
// In a real application, these would interact with actual data structures or external systems.
int taxpayer_count_submitted_tax_years(audit_id_t taxpayer_id) { return 1; }
void tenfourd_get_last_three_from_list(uint32_t taxpayer_data_field, int count, int *years_buffer) {
    if (years_buffer && count > 0) years_buffer[0] = 2020;
    if (years_buffer && count > 1) years_buffer[1] = 2021;
}
audit_id_t taxpayer_get_by_username(void *list_head, const void *username_data) { return 1001; }
audit_id_t session_get_by_username(void *list_head, const void *username_data) { return 0; }
void session_append(void **list_head, const void *data) {}
void *session_remove(void **list_head, const void *username_data) { return NULL; }
int taxpayer_get_refund(audit_id_t taxpayer_id, const void *param_2_data, uint32_t *buffer, uint *size) {
    if (size) *size = 4;
    if (buffer) *buffer = 500;
    return 0;
}
int taxpayer_pay_taxes(audit_id_t taxpayer_id, const void *param_2_data, uint32_t *buffer, uint *size) {
    if (size) *size = 4;
    if (buffer) *buffer = 1000;
    return 0;
}
int taxpayer_list_submitted_tax_years(audit_id_t taxpayer_id, const void *param_2_data, uint32_t *buffer, uint32_t *size) {
    if (size) *size = 8;
    if (buffer) { buffer[0] = 2020; buffer[1] = 2021; }
    return 0;
}
int taxpayer_sum_taxes_due(audit_id_t taxpayer_id, const void *param_2_data, uint32_t *amount_due) {
    if (amount_due) *amount_due = 1500;
    return 0;
}
int taxpayer_add_tenfourdee(audit_id_t taxpayer_id, const void *param_2_data, size_t data_size) { return 0; }
int taxpayer_compare_creds(audit_id_t taxpayer_id, const void *creds_ptr) { return 0; }
void taxpayer_new(const void *param_1_data, const void *param_2_data, void **tp_list_head) {}


// Function: get_new_audit_record
void * get_new_audit_record(void) {
  AuditRecord *new_record = (AuditRecord *)calloc(1, sizeof(AuditRecord));
  if (new_record == NULL) {
    _terminate(0xfffffffb);
  }
  return new_record;
}

// Function: add_auditrecord_to_audit_list
void add_auditrecord_to_audit_list(AuditRecord *new_audit_record) {
  if (audit_list == NULL) {
    audit_list = new_audit_record;
  } else {
    AuditRecord *current = audit_list;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = new_audit_record;
  }
}

// Function: is_taxpayer_scheduled_for_audit
response_status_t is_taxpayer_scheduled_for_audit(audit_id_t taxpayer_id) {
  AuditRecord *current = audit_list;
  while (current != NULL) {
    if (taxpayer_id == current->taxpayer_id) {
      return 1;
    }
    current = current->next;
  }
  return 0;
}

// Function: find_three_tax_years_for_audit
AuditRecord * find_three_tax_years_for_audit(audit_id_t taxpayer_ptr_as_id) {
  int submitted_years_count = taxpayer_count_submitted_tax_years(taxpayer_ptr_as_id);
  if (submitted_years_count == 0) {
    return NULL;
  }

  AuditRecord *record = (AuditRecord *)get_new_audit_record();
  record->taxpayer_id = taxpayer_ptr_as_id;
  // `taxpayer_ptr_as_id` is treated as a pointer to a taxpayer structure
  uint32_t taxpayer_specific_data = *(uint32_t *)((char *)(intptr_t)taxpayer_ptr_as_id + 0x2a0);
  tenfourd_get_last_three_from_list(taxpayer_specific_data, submitted_years_count, record->years);
  return record;
}

// Function: we_have_a_winner
response_status_t we_have_a_winner(const void *username_data, ResponseBuffer *response) {
  audit_id_t taxpayer_id = taxpayer_get_by_username(tp_list, username_data);

  if (taxpayer_id != 0 && is_taxpayer_scheduled_for_audit(taxpayer_id) == 0) {
    AuditRecord *audit_record = find_three_tax_years_for_audit(taxpayer_id);
    if (audit_record == NULL) {
      memcpy(response->message_and_data, "LUCKY DAY!", 10);
    } else {
      audit_record->taxpayer_id = taxpayer_id;
      add_auditrecord_to_audit_list(audit_record);
      memcpy(response->message_and_data, "YOU WON A NEW AUDIT!", 0x14);
      // Original copies 6 bytes from `audit_record->years`.
      // This is suspicious for `int years[2]` as it copies years[0] (4 bytes) and half of years[1] (2 bytes).
      // Keeping it faithful to the original snippet's explicit size.
      memcpy(response->message_and_data + (0x20 - 0xc), audit_record->years, 6);
    }
    return 0;
  }

  memcpy(response->message_and_data, "AUDIT_FAIL", 10);
  return 0xffffffff;
}

// Function: recv_cmd_data
response_status_t recv_cmd_data(RequestBuffer *request, size_t expected_data_size) {
  // A short integer at offset 0x2c in the request buffer holds the expected size.
  uint16_t size_from_request_header = *(uint16_t *)((char *)request + 0x2c);

  if (expected_data_size == size_from_request_header) {
    if (expected_data_size != 0) {
      void *data_buffer = calloc(1, expected_data_size);
      if (data_buffer == NULL) {
        _terminate(1); // Memory allocation failure
      }
      // Store the allocated buffer pointer in the request object at offset 0x2e.
      *(void **)((char *)request + 0x2e) = data_buffer;

      size_t bytes_received = recv_all(data_buffer, expected_data_size);
      if (expected_data_size != bytes_received) {
        _terminate(1); // Incomplete data received
      }
    }
    return 0; // Success
  }
  return 0xffffffff; // Mismatch in expected data size
}

// Function: session_auth
response_status_t session_auth(const void *username_data) {
  // `username_data` is treated as a session-like object containing username and other data
  Session *session_obj = (Session *)session_get_by_username(s_list, username_data);

  if (session_obj != NULL &&
      memcmp(username_data, session_obj, 0xc) == 0 &&
      memcmp((const char *)username_data + 0x18, (const char *)session_obj + 0x18, 0xc) == 0) {
    return 0; // Authenticated
  }
  return 0xffffffff; // Authentication failed
}

// Function: response_set_key
void response_set_key(ResponseBuffer *response, const void *key_source) {
  memcpy(response->key_data, key_source, 0xc);
}

// Function: get_refund
response_status_t get_refund(const void *username_data, ResponseBuffer *response) {
  uint32_t refund_buffer[32];
  uint refund_size = 0x71; // Initial max size for refund_buffer

  memset(refund_buffer, 0, sizeof(refund_buffer));

  audit_id_t taxpayer_id = taxpayer_get_by_username(tp_list, username_data);
  if (taxpayer_id == 0) {
      memcpy(response->message_and_data, "GET_REFUND_FAIL", 0xf);
      return 0xffffffff;
  }

  int result = taxpayer_get_refund(taxpayer_id, username_data, refund_buffer, &refund_size);
  if (result == 0 && refund_size < 0x71) {
    memcpy(response->message_and_data, "GET_REFUND_OK", 0xd);
    // 0x19 offset from start of the *full* response buffer.
    memcpy(response->message_and_data + (0x19 - 0xc), refund_buffer, refund_size);
    return 0;
  }

  memcpy(response->message_and_data, "GET_REFUND_FAIL", 0xf);
  return 0xffffffff;
}

// Function: pay_taxes
response_status_t pay_taxes(RequestBuffer *request, ResponseBuffer *response) {
  uint32_t tax_buffer[32];
  uint tax_data_size = 0x72; // Initial max size for tax_buffer

  memset(tax_buffer, 0, sizeof(tax_buffer));

  response_status_t recv_status = recv_cmd_data(request, 4); // Expecting 4 bytes of data
  if (recv_status != 0) {
    memcpy(response->message_and_data, "PAY_TAXES_FAIL", 0xe);
    return 0xffffffff;
  }

  audit_id_t taxpayer_id = taxpayer_get_by_username(tp_list, request);
  if (taxpayer_id == 0) {
    memcpy(response->message_and_data, "PAY_TAXES_FAIL", 0xe);
    return 0xffffffff;
  }

  void *payment_data = *(void **)((char *)request + 0x2e);
  int result = taxpayer_pay_taxes(taxpayer_id, payment_data, tax_buffer, &tax_data_size);
  if (result == 0 && tax_data_size < 0x72) {
    memcpy(response->message_and_data, "PAY_TAXES_OK", 0xc);
    // 0x18 offset from start of the *full* response buffer.
    memcpy(response->message_and_data + (0x18 - 0xc), tax_buffer, tax_data_size);
    return 0;
  }

  memcpy(response->message_and_data, "PAY_TAXES_FAIL", 0xe);
  return 0xffffffff;
}

// Function: copy_yr_list_into_answer
void copy_yr_list_into_answer(ResponseBuffer *response, const void *year_list_data, size_t data_size) {
  memcpy(response->message_and_data, "TAXES_SUBMITTED_OK", 0x12);
  // 0x1e offset from start of the *full* response buffer.
  memcpy(response->message_and_data + (0x1e - 0xc), year_list_data, data_size);
}

// Function: taxes_submitted
response_status_t taxes_submitted(RequestBuffer *request, ResponseBuffer *response) {
  uint32_t years_buffer[32];
  uint32_t years_count = 0; // Output size for years

  memset(years_buffer, 0, sizeof(years_buffer));

  response_status_t recv_status = recv_cmd_data(request, 4); // Expecting 4 bytes of data
  if (recv_status != 0) {
    memcpy(response->message_and_data, "TAXES_SUBMITTED_FAIL", 0x15);
    return 0xffffffff;
  }

  audit_id_t taxpayer_id = taxpayer_get_by_username(tp_list, request);
  if (taxpayer_id == 0) {
    memcpy(response->message_and_data, "TAXES_SUBMITTED_FAIL", 0x15);
    return 0xffffffff;
  }

  int result = taxpayer_list_submitted_tax_years(taxpayer_id, request, years_buffer, &years_count);
  if (result == 0) {
    copy_yr_list_into_answer(response, years_buffer, years_count);
    return 0;
  }

  memcpy(response->message_and_data, "TAXES_SUBMITTED_FAIL", 0x15);
  return 0xffffffff;
}

// Function: taxes_due
response_status_t taxes_due(RequestBuffer *request, ResponseBuffer *response) {
  uint32_t amount_due = 0;

  response_status_t recv_status = recv_cmd_data(request, 4); // Expecting 4 bytes of data
  if (recv_status != 0) {
    memcpy(response->message_and_data, "TAXES_DUE_FAIL", 0xe);
    // Original copies amount_due even on fail, so keeping this behavior
    memcpy(response->message_and_data + (0x1a - 0xc), &amount_due, 4);
    return 0xffffffff;
  }

  audit_id_t taxpayer_id = taxpayer_get_by_username(tp_list, request);
  if (taxpayer_id == 0) {
    memcpy(response->message_and_data, "TAXES_DUE_FAIL", 0xe);
    memcpy(response->message_and_data + (0x1a - 0xc), &amount_due, 4);
    return 0xffffffff;
  }

  int result = taxpayer_sum_taxes_due(taxpayer_id, request, &amount_due);
  if (result == 0) {
    memcpy(response->message_and_data, "TAXES_DUE_OK", 0xc);
    // 0x18 offset from start of the *full* response buffer.
    memcpy(response->message_and_data + (0x18 - 0xc), &amount_due, 4);
    return 0;
  }

  memcpy(response->message_and_data, "TAXES_DUE_FAIL", 0xe);
  memcpy(response->message_and_data + (0x1a - 0xc), &amount_due, 4); // Original copies amount_due even on fail
  return 0xffffffff;
}

// Function: upload_form
response_status_t upload_form(RequestBuffer *request, ResponseBuffer *response) {
  response_status_t recv_status = recv_cmd_data(request, 0x2eb); // Expecting 0x2eb bytes of data
  if (recv_status != 0) {
    memcpy(response->message_and_data, "UPLOAD_FAIL", 0xb);
    return 0xffffffff;
  }

  audit_id_t taxpayer_id = taxpayer_get_by_username(tp_list, request);
  if (taxpayer_id == 0) {
    memcpy(response->message_and_data, "UPLOAD_FAIL", 0xb);
    return 0xffffffff;
  }

  void *form_data = *(void **)((char *)request + 0x2e);
  int result = taxpayer_add_tenfourdee(taxpayer_id, form_data, 0x2eb);
  if (result == 0) {
    memcpy(response->message_and_data, "UPLOAD_OK", 9);
    return 0;
  }

  memcpy(response->message_and_data, "UPLOAD_FAIL", 0xb);
  return 0xffffffff;
}

// Function: login
response_status_t login(const void *creds_data, ResponseBuffer *response) {
  audit_id_t taxpayer_id = taxpayer_get_by_username(tp_list, creds_data);

  if (taxpayer_id == 0 || taxpayer_compare_creds(taxpayer_id, creds_data) != 0) {
    memcpy(response->message_and_data, "LOGIN_FAIL", 10);
    return 0xffffffff;
  }

  // Check if session already exists AND the new credentials do not match the existing session data
  Session *existing_session = (Session *)session_get_by_username(s_list, creds_data);
  if (existing_session != NULL && memcmp(creds_data, existing_session, 0xc) != 0) {
     memcpy(response->message_and_data, "LOGIN_FAIL", 10);
     return 0xffffffff;
  }

  session_append(&s_list, creds_data);
  memcpy(response->message_and_data, "LOGIN_OK", 8);
  return 0;
}

// Function: logout
response_status_t logout(const void *username_data, ResponseBuffer *response) {
  Session *removed_session = (Session *)session_remove(&s_list, username_data);
  if (removed_session != NULL) {
    if (removed_session->data_at_0x2e != NULL) {
      free(removed_session->data_at_0x2e);
    }
    free(removed_session);
  }
  memcpy(response->message_and_data, "LOGOUT_OK", 9);
  return 0;
}

// Function: create_account
int create_account(RequestBuffer *request, ResponseBuffer *response) {
  response_status_t recv_status = recv_cmd_data(request, 0x288); // Expecting 0x288 bytes of data
  if (recv_status != 0) {
    return -1;
  }

  audit_id_t existing_taxpayer = taxpayer_get_by_username(tp_list, request);
  if (existing_taxpayer == 0) {
    void *new_taxpayer_data = *(void **)((char *)request + 0x2e);
    taxpayer_new(request, new_taxpayer_data, &tp_list);
    return 0;
  }
  return -1; // Account already exists
}

// Function: main
void main(void) {
  RequestBuffer request_buffer;
  ResponseBuffer response_buffer;
  int command_result;

  while (true) {
    // Reset buffers for each new command to ensure clean state
    memset(&request_buffer, 0, sizeof(request_buffer));
    memset(&response_buffer, 0, sizeof(response_buffer));

    // Receive command data (0x2e bytes) into request_buffer
    size_t bytes_read = recv_all(&request_buffer, sizeof(request_buffer));
    if (bytes_read != sizeof(request_buffer)) {
      _terminate(1); // Terminate if an incomplete command is received
    }

    // Authenticate session using data at the start of request_buffer
    command_result = session_auth(&request_buffer);

    if (command_result == 0) { // Session authenticated
      // Compare the command string (first part of request_buffer)
      if (memcmp(request_buffer.command_name_and_key, LOGOUT_CMD, strlen(LOGOUT_CMD)) == 0) {
        command_result = logout(&request_buffer, &response_buffer);
      } else if (memcmp(request_buffer.command_name_and_key, UPLOAD_FORM_CMD, strlen(UPLOAD_FORM_CMD)) == 0) {
        command_result = upload_form(&request_buffer, &response_buffer);
      } else if (memcmp(request_buffer.command_name_and_key, TAXES_DUE_CMD, strlen(TAXES_DUE_CMD)) == 0) {
        command_result = taxes_due(&request_buffer, &response_buffer);
      } else if (memcmp(request_buffer.command_name_and_key, TAXES_SUBMITTED_CMD, strlen(TAXES_SUBMITTED_CMD)) == 0) {
        command_result = taxes_submitted(&request_buffer, &response_buffer);
      } else if (memcmp(request_buffer.command_name_and_key, PAY_TAXES_CMD, strlen(PAY_TAXES_CMD)) == 0) {
        command_result = pay_taxes(&request_buffer, &response_buffer);
      } else if (memcmp(request_buffer.command_name_and_key, GET_REFUND_CMD, strlen(GET_REFUND_CMD)) == 0) {
        command_result = get_refund(&request_buffer, &response_buffer);
      } else if (memcmp(request_buffer.command_name_and_key, TRIGGER_AUDIT_CMD, strlen(TRIGGER_AUDIT_CMD)) == 0 ||
                 memcmp(request_buffer.command_name_and_key, TRIGGER_AUDIT2_CMD, strlen(TRIGGER_AUDIT2_CMD)) == 0) {
        command_result = we_have_a_winner(&request_buffer, &response_buffer);
      } else {
        // No specific command matched for an authenticated session
        memcpy(response_buffer.message_and_data, "NICE_TRY", 8);
        command_result = -1;
      }
    } else { // Session not authenticated
      if (memcmp(request_buffer.command_name_and_key, LOGIN_CMD, strlen(LOGIN_CMD)) == 0) {
        command_result = login(&request_buffer, &response_buffer);
      } else if (memcmp(request_buffer.command_name_and_key, CREATE_ACCOUNT_CMD, strlen(CREATE_ACCOUNT_CMD)) == 0) {
        command_result = create_account(&request_buffer, &response_buffer);
      } else {
        // No specific command matched for an unauthenticated session
        memcpy(response_buffer.message_and_data, "NICE_TRY", 8);
        command_result = -1;
      }
    }

    // Copy the first 0xc bytes (command name/key) from the request to the response key_data
    response_set_key(&response_buffer, &request_buffer);

    // Send the full response buffer
    send_response(&response_buffer, sizeof(response_buffer));

    // Free any dynamically allocated data from recv_cmd_data for the current request
    void *allocated_data = *(void **)((char *)&request_buffer + 0x2e);
    if (allocated_data != NULL) {
      free(allocated_data);
      *(void **)((char *)&request_buffer + 0x2e) = NULL; // Clear the pointer after freeing
    }
  }
}