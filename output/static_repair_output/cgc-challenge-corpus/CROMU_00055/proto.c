#include <stdio.h>   // For FILE, fopen, fclose, fread, fwrite, SEEK_END, SEEK_SET, ftell, fseek
#include <stdlib.h>  // For calloc, free, exit
#include <string.h>  // For strlen, strdup
#include <unistd.h>  // For _exit, though exit is used for simplicity

// Decompiled types mapping to standard C types
typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned char undefined; // Used for generic byte access
typedef unsigned int undefined4; // Used for generic 4-byte access (e.g., pointers or integers)
typedef unsigned short ushort;   // Used for generic 2-byte access

// Forward declarations for custom functions, adjusted based on inferred usage
// The original decompiled code used 'int' for pointer parameters,
// which is common in low-level code but dangerous. Using 'char*' for clarity.
int receive(char* buf, uint len, int* bytes_read_out);
int transmit(char* buf, uint len, int* bytes_written_out);
void _terminate(void); // Replaced with exit(1) for compilation
char CheckPasswd(char* username, char* password);
char Login(char* username);
char ListFiles(char **file_list_ptr); // file_list_ptr will point to a char* allocated by ListFiles
char DeleteFile(char* filename);
char RenameFile(char* old_filename, char* new_filename);

// Dummy implementations for compilation
int receive(char* buf, uint len, int* bytes_read_out) {
    if (len == 0) { *bytes_read_out = 0; return 0; }
    *bytes_read_out = (len > 10) ? 10 : len; // Simulate reading up to 10 bytes
    for (uint i = 0; i < *bytes_read_out; ++i) {
        buf[i] = (char)(i % 256); // Fill with dummy data
    }
    return 0; // Success
}
int transmit(char* buf, uint len, int* bytes_written_out) {
    *bytes_written_out = len; // Simulate writing all requested bytes
    return 0; // Success
}
void _terminate(void) {
    exit(1);
}
char CheckPasswd(char* username, char* password) { return 1; }
char Login(char* username) { return 1; }
char ListFiles(char **file_list_ptr) { *file_list_ptr = strdup("dummy_file1.txt\ndummy_file2.txt"); return 1; }
char DeleteFile(char* filename) { return 1; }
char RenameFile(char* old_filename, char* new_filename) { return 1; }

// DAT_0001801c is likely a placeholder for a size unit, typically 1 for byte-sized allocations.
#define DAT_0001801c 1

// A generic request/response buffer size, based on ReceiveRequest allocating 0x24 bytes.
#define MAX_REQ_RESP_SIZE 0x24

// Function: ReadBytes
uint ReadBytes(char *buffer, uint length) {
  int bytes_read_this_iter;
  uint total_bytes_read = 0;
  
  while (total_bytes_read < length) {
    int status = receive(buffer + total_bytes_read, length - total_bytes_read, &bytes_read_this_iter);
    if (status != 0) {
      _terminate(); // Error during receive
    }
    if (bytes_read_this_iter == 0) {
      _terminate(); // Peer closed connection or no data, but expected more
    }
    total_bytes_read += bytes_read_this_iter;
  }
  return total_bytes_read;
}

// Function: SendBytes
uint SendBytes(char *buffer, uint length) {
  int bytes_written_this_iter;
  uint total_bytes_written = 0;
  
  while (total_bytes_written < length) {
    int status = transmit(buffer + total_bytes_written, length - total_bytes_written, &bytes_written_this_iter);
    if (status != 0) {
      _terminate(); // Error during transmit
    }
    // Original code checks if (param_2 - local_10 != local_14)
    // This means (remaining_length != bytes_written_this_iter)
    // It's a strict check that transmit must write *all* requested bytes.
    if ((length - total_bytes_written) != bytes_written_this_iter) {
      _terminate(); // Did not write all expected bytes
    }
    total_bytes_written += bytes_written_this_iter;
  }
  return total_bytes_written;
}

// Function: CallocAndRead
void CallocAndRead(void **target_ptr, byte length_minus_null) {
  void *allocated_buffer;
  byte actual_length_with_null = length_minus_null + 1; // +1 for null terminator

  allocated_buffer = calloc(actual_length_with_null, 1); // Allocate 'length_minus_null + 1' bytes, each of size 1
  *target_ptr = allocated_buffer;

  if (*target_ptr == NULL) { // Check if allocation failed
    _terminate();
  }
  
  // Read 'length_minus_null' bytes into the allocated buffer
  ReadBytes((char *)*target_ptr, length_minus_null);
  return;
}

// Function: ReceiveRequest
byte * ReceiveRequest(void) {
  char *request_buffer = (char *)calloc(MAX_REQ_RESP_SIZE, 1); // Allocate 0x24 bytes
  if (request_buffer == NULL) {
    _terminate();
  }

  byte request_type;
  ReadBytes((char *)&request_type, 1); // Read 1 byte into request_type
  request_buffer[0] = request_type; // Store request type in the buffer

  switch(request_type) {
  case 0: { // Login request
    unsigned short login_lengths; // Combined length for username and password
    ReadBytes((char *)&login_lengths, 2); // Read 2 bytes
    
    // Store username pointer at offset 4, password pointer at offset 8
    CallocAndRead((void **)(request_buffer + 4), (byte)(login_lengths & 0xFF)); // Username length (first byte)
    CallocAndRead((void **)(request_buffer + 8), (byte)(login_lengths >> 8)); // Password length (second byte)
    break;
  }
  case 1: { // Dir request
    // No additional fields to read for this request type
    break;
  }
  case 2: { // Read request
    unsigned char temp_read_params[6]; // Temporary buffer to read 6 bytes
    ReadBytes((char *)temp_read_params, 6);

    // Store filename pointer at offset 0xc
    CallocAndRead((void **)(request_buffer + 0xc), temp_read_params[5]); // Filename length from 6th byte
    // Store read offset at offset 0x1c (4 bytes)
    *(uint *)(request_buffer + 0x1c) = *(uint *)temp_read_params; // Read offset from first 4 bytes
    // Store read length at offset 0x20 (1 byte)
    request_buffer[0x20] = temp_read_params[4]; // Read length from 5th byte
    break;
  }
  case 3: // Write request
  case 4: { // WriteAppend request
    unsigned char temp_write_params[2]; // Temporary buffer to read 2 bytes
    ReadBytes((char *)temp_write_params, 2);

    // Store filename pointer at offset 0xc
    CallocAndRead((void **)(request_buffer + 0xc), temp_write_params[1]); // Filename length from 2nd byte
    // Store write data length at offset 0x14 (1 byte)
    *(byte *)(request_buffer + 0x14) = temp_write_params[0]; // Write data length from 1st byte
    // Store write data pointer at offset 0x18
    CallocAndRead((void **)(request_buffer + 0x18), temp_write_params[0]); // Write data length from 1st byte
    // Also store 1st byte at 0x20 (overlaps with read_len in case 2, but specific to this type)
    request_buffer[0x20] = temp_write_params[0];
    break;
  }
  case 5: { // Delete request
    byte filename_len;
    ReadBytes((char *)&filename_len, 1);
    // Store filename pointer at offset 0xc
    CallocAndRead((void **)(request_buffer + 0xc), filename_len); // Filename length
    break;
  }
  case 6: { // Rename request
    unsigned short rename_lengths; // Combined length for old and new filenames
    ReadBytes((char *)&rename_lengths, 2);

    // Store old filename pointer at offset 0xc
    CallocAndRead((void **)(request_buffer + 0xc), (byte)(rename_lengths & 0xFF)); // Old filename length (first byte)
    // Store new filename pointer at offset 0x10
    CallocAndRead((void **)(request_buffer + 0x10), (byte)(rename_lengths >> 8)); // New filename length (second byte)
    break;
  }
  default:
    free(request_buffer);
    request_buffer = NULL;
  }
  return (byte *)request_buffer;
}

// Function: HandleLogin
undefined4 HandleLogin(char *request_ptr, char *response_ptr) {
  if ((request_ptr == NULL) || (response_ptr == NULL)) {
    return 0;
  }
  // Offsets from ReceiveRequest for type 0:
  // request_ptr + 4: username pointer
  // request_ptr + 8: password pointer
  char result_check = CheckPasswd(*(char **)(request_ptr + 4), *(char **)(request_ptr + 8));
  if (result_check == '\0') {
    *(byte *)(response_ptr + 1) = 1; // Status: Authentication failed
    return 0;
  } else {
    char result_login = Login(*(char **)(request_ptr + 4));
    if (result_login == '\0') {
      *(byte *)(response_ptr + 1) = 2; // Status: Login failed
      return 0;
    } else {
      *(byte *)(response_ptr + 1) = 0; // Status: Success
      return 1;
    }
  }
}

// Function: HandleDir
undefined4 HandleDir(char *request_ptr, char *response_ptr) {
  if ((request_ptr == NULL) || (response_ptr == NULL)) {
    return 0;
  }
  char *file_list = NULL;
  char result_list = ListFiles(&file_list);
  if (result_list == '\0') {
    *(byte *)(response_ptr + 1) = 2; // Status: Error listing files
    return 0;
  } else {
    *(byte *)(response_ptr + 1) = 0; // Status: Success
    size_t list_len = strlen(file_list);
    *(ushort *)(response_ptr + 2) = (ushort)list_len; // Data length
    *(char **)(response_ptr + 4) = file_list; // Data pointer
    return 1;
  }
}

// Function: HandleRead
undefined4 HandleRead(char *request_ptr, char *response_ptr) {
  if ((request_ptr == NULL) || (response_ptr == NULL)) {
    return 0;
  }

  char *filename = *(char **)(request_ptr + 0xc);
  uint read_offset = *(uint *)(request_ptr + 0x1c);
  byte read_length_byte = *(byte *)(request_ptr + 0x20); // Length to read into new buffer

  if (filename == NULL) {
    *(byte *)(response_ptr + 1) = 3; // Status: Invalid filename
    return 0;
  }

  FILE *fp = fopen(filename, "rb"); // Use "rb" for binary read
  if (fp == NULL) {
    *(byte *)(response_ptr + 1) = 3; // Status: File not found or permission error
    return 0;
  }

  // Seek to the specified offset
  if (fseek(fp, read_offset, SEEK_SET) != 0) {
      *(byte *)(response_ptr + 1) = 2; // Status: Seek error
      fclose(fp);
      return 0;
  }

  void *read_buffer = calloc(read_length_byte, DAT_0001801c);
  *(void **)(response_ptr + 4) = read_buffer; // Store allocated buffer pointer

  if (read_buffer == NULL) {
    *(byte *)(response_ptr + 1) = 2; // Status: Memory allocation failed
    fclose(fp);
    return 0;
  }

  size_t bytes_read = fread(read_buffer, DAT_0001801c, read_length_byte, fp);
  if (bytes_read != read_length_byte && ferror(fp)) { // Check for read errors if not all bytes read
    *(byte *)(response_ptr + 1) = 2; // Status: Read error
    free(read_buffer);
    *(void **)(response_ptr + 4) = NULL;
    fclose(fp);
    return 0;
  }

  *(ushort *)(response_ptr + 2) = (ushort)bytes_read; // Actual bytes read
  fclose(fp);
  *(byte *)(response_ptr + 1) = 0; // Status: Success
  return 1;
}

// Function: HandleWrite
undefined4 HandleWrite(char *request_ptr, char *response_ptr) {
  if ((request_ptr == NULL) || (response_ptr == NULL)) {
    return 0;
  }
  char *filename = *(char **)(request_ptr + 0xc);
  byte write_length = *(byte *)(request_ptr + 0x14);
  void *write_data = *(void **)(request_ptr + 0x18);

  if (filename == NULL) {
    *(byte *)(response_ptr + 1) = 3; // Status: Invalid filename
    return 0;
  }

  FILE *fp = fopen(filename, "wb"); // Use "wb" for binary write (truncates file if exists)
  if (fp == NULL) {
    *(byte *)(response_ptr + 1) = 3; // Status: File not found or permission error
    return 0;
  }

  size_t bytes_written = fwrite(write_data, DAT_0001801c, write_length, fp);
  if (bytes_written != write_length) {
    *(byte *)(response_ptr + 1) = 2; // Status: Write error
    fclose(fp);
    return 0;
  }

  fclose(fp);
  *(byte *)(response_ptr + 1) = 0; // Status: Success
  return 1;
}

// Function: HandleWriteAppend
undefined4 HandleWriteAppend(char *request_ptr, char *response_ptr) {
  if ((request_ptr == NULL) || (response_ptr == NULL)) {
    return 0;
  }
  char *filename = *(char **)(request_ptr + 0xc);
  byte append_length = *(byte *)(request_ptr + 0x14);
  void *append_data = *(void **)(request_ptr + 0x18);

  if (filename == NULL) {
    *(byte *)(response_ptr + 1) = 3; // Status: Invalid filename
    return 0;
  }

  FILE *fp = fopen(filename, "rb"); // Open for reading to get original content
  if (fp == NULL) {
    // If file doesn't exist, the original code returns an error.
    *(byte *)(response_ptr + 1) = 3; // Status: File not found or permission error
    return 0;
  }

  // Get file size
  fseek(fp, 0, SEEK_END);
  long original_file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  void *original_content = NULL;
  if (original_file_size > 0) {
      original_content = calloc(original_file_size, DAT_0001801c);
      if (original_content == NULL) {
        *(byte *)(response_ptr + 1) = 2; // Status: Memory allocation failed
        fclose(fp);
        return 0;
      }

      size_t bytes_read = fread(original_content, DAT_0001801c, original_file_size, fp);
      if (bytes_read != original_file_size) {
        *(byte *)(response_ptr + 1) = 2; // Status: Read error
        free(original_content);
        fclose(fp);
        return 0;
      }
  }
  fclose(fp); // Close read handle

  // Re-open in write mode to truncate and write original content + new data
  fp = fopen(filename, "wb");
  if (fp == NULL) {
    *(byte *)(response_ptr + 1) = 2; // Status: Write error (cannot open for write)
    if (original_content) free(original_content);
    return 0;
  }

  if (original_content && original_file_size > 0) {
      size_t bytes_written = fwrite(original_content, DAT_0001801c, original_file_size, fp);
      if (bytes_written != original_file_size) {
        *(byte *)(response_ptr + 1) = 2; // Status: Write error (original content)
        free(original_content);
        fclose(fp);
        return 0;
      }
      free(original_content); // Free buffer after writing
  }

  // Append new data
  size_t bytes_appended = fwrite(append_data, DAT_0001801c, append_length, fp);
  if (bytes_appended != append_length) {
    *(byte *)(response_ptr + 1) = 2; // Status: Append write error
    fclose(fp);
    return 0;
  }

  fclose(fp);
  *(byte *)(response_ptr + 1) = 0; // Status: Success
  return 1;
}

// Function: HandleDel
undefined4 HandleDel(char *request_ptr, char *response_ptr) {
  if ((request_ptr == NULL) || (response_ptr == NULL)) {
    return 0;
  }
  char *filename = *(char **)(request_ptr + 0xc);
  char result = DeleteFile(filename); // Assuming DeleteFile takes char*
  if (result == '\0') {
    *(byte *)(response_ptr + 1) = 4; // Status: Delete failed
    return 0;
  } else {
    *(byte *)(response_ptr + 1) = 0; // Status: Success
    return 1;
  }
}

// Function: HandleRename
undefined4 HandleRename(char *request_ptr, char *response_ptr) {
  if ((request_ptr == NULL) || (response_ptr == NULL)) {
    return 0;
  }
  char *old_filename = *(char **)(request_ptr + 0xc);
  char *new_filename = *(char **)(request_ptr + 0x10);

  char result = RenameFile(old_filename, new_filename); // Assuming RenameFile takes char*, char*
  if (result == '\0') {
    *(byte *)(response_ptr + 1) = 5; // Status: Rename failed
    return 0;
  } else {
    *(byte *)(response_ptr + 1) = 0; // Status: Success
    return 1;
  }
}

// Function: HandleRequest
byte * HandleRequest(byte *request_ptr) {
  if (request_ptr == NULL) {
    return NULL;
  }
  
  char *response_buffer = (char *)calloc(MAX_REQ_RESP_SIZE, 1); // Allocate 0x24 bytes for consistency
  if (response_buffer == NULL) {
    _terminate(); // Memory allocation failed
  }
  
  response_buffer[0] = request_ptr[0]; // Copy request type to response

  switch(request_ptr[0]) { // Use request_ptr[0] as the type
  case 0: // Login
    HandleLogin(request_ptr, response_buffer);
    break;
  case 1: // Dir
    HandleDir(request_ptr, response_buffer);
    break;
  case 2: // Read
    HandleRead(request_ptr, response_buffer);
    break;
  case 3: // Write
    HandleWrite(request_ptr, response_buffer);
    break;
  case 4: // WriteAppend
    HandleWriteAppend(request_ptr, response_buffer);
    break;
  case 5: // Del
    HandleDel(request_ptr, response_buffer);
    break;
  case 6: // Rename
    HandleRename(request_ptr, response_buffer);
    break;
  default:
    free(response_buffer);
    response_buffer = NULL;
  }
  return (byte *)response_buffer;
}

// Function: SendResponse
undefined4 SendResponse(char *response_ptr) {
  if (response_ptr == NULL) {
    return 0;
  }
  
  // Send the header: type (1 byte), status (1 byte), data_len (2 bytes) = 4 bytes
  int bytes_sent_header = SendBytes(response_ptr, 4);
  if (bytes_sent_header != 4) {
    return 0; // Failed to send header
  }

  ushort data_len = *(ushort *)(response_ptr + 2);
  if (data_len == 0) {
    return 1; // Success, no data to send
  } else {
    // Send the actual data
    char *data_ptr = *(char **)(response_ptr + 4);
    uint bytes_sent_data = SendBytes(data_ptr, data_len);
    if (bytes_sent_data == data_len) {
      return 1; // Success
    } else {
      return 0; // Failed to send all data
    }
  }
}

// Function: FreeRequest
undefined4 FreeRequest(void *request_ptr_void) {
  if (request_ptr_void == NULL) {
    return 0;
  }
  char *request_ptr = (char *)request_ptr_void;
  
  // Free pointers to dynamically allocated data within the request buffer
  // Offsets correspond to where CallocAndRead or other allocations would have stored pointers
  
  // Login username (offset 4)
  if (*(char **)(request_ptr + 4) != NULL) {
    free(*(void **)(request_ptr + 4));
    *(char **)(request_ptr + 4) = NULL;
  }
  // Login password (offset 8)
  if (*(char **)(request_ptr + 8) != NULL) {
    free(*(void **)(request_ptr + 8));
    *(char **)(request_ptr + 8) = NULL;
  }
  // Filename for various operations (offset 0xc)
  if (*(char **)(request_ptr + 0xc) != NULL) {
    free(*(void **)(request_ptr + 0xc));
    *(char **)(request_ptr + 0xc) = NULL;
  }
  // New filename for rename (offset 0x10)
  if (*(char **)(request_ptr + 0x10) != NULL) {
    free(*(void **)(request_ptr + 0x10));
    *(char **)(request_ptr + 0x10) = NULL;
  }
  // Write data (offset 0x18)
  if (*(char **)(request_ptr + 0x18) != NULL) {
    free(*(void **)(request_ptr + 0x18));
    *(char **)(request_ptr + 0x18) = NULL;
  }
  
  free(request_ptr); // Free the main request buffer
  return 1;
}

// Function: FreeResponse
undefined4 FreeResponse(void *response_ptr_void) {
  if (response_ptr_void == NULL) {
    return 0;
  }
  char *response_ptr = (char *)response_ptr_void;
  
  // Data pointer (e.g., file list, read data) (offset 4)
  if (*(char **)(response_ptr + 4) != NULL) {
    free(*(void **)(response_ptr + 4));
    *(char **)(response_ptr + 4) = NULL;
  }
  
  free(response_ptr); // Free the main response buffer
  return 1;
}