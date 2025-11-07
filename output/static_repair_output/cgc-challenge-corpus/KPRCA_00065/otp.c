#include <stdio.h>   // For FILE, stdout, fwrite, printf
#include <stdlib.h>  // For malloc, free, random
#include <string.h>  // For memcpy, memset, memcmp
#include <unistd.h>  // For read (if read_n uses it)

// Type definitions to match the decompiler's output closely
typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned char undefined;
typedef unsigned int undefined4;
typedef unsigned short ushort;

// Dummy implementations for missing external functions
// In a real scenario, these would be provided by the environment or linked libraries.
// For compilation, we need placeholders.
// read_n is typically used for exact byte reads.
ssize_t read_n(int fd, void *buf, size_t count) {
    ssize_t bytes_read = 0;
    ssize_t total_read = 0;
    while (total_read < count) {
        bytes_read = read(fd, (char *)buf + total_read, count - total_read);
        if (bytes_read <= 0) { // EOF or error
            break;
        }
        total_read += bytes_read;
    }
    return total_read;
}

// read_until is typically used to read until a specific character or max count.
ssize_t read_until(int fd, void *buf, size_t count) {
    ssize_t total_read = 0;
    char *cbuf = (char *)buf;
    while (total_read < count) {
        ssize_t bytes_read = read(fd, cbuf + total_read, 1);
        if (bytes_read <= 0) { // EOF or error
            break;
        }
        if (cbuf[total_read] == '\n') { // Read until newline
            total_read++;
            break;
        }
        total_read += bytes_read;
    }
    return total_read;
}

// Global variables, assuming placeholder values or external linkage
// `secret` is used as an array of unsigned int (0x400 elements = 0x1000 bytes)
// and also as a byte array for memcpy.
static unsigned int secret[0x400] = {0x11223344, /* ... initial secret data ... */};
// DAT_00015004 and DAT_00015006 are likely single characters or pointers to single characters
static const char DAT_00015004 = 'S'; // Success indicator
static const char DAT_00015006 = 'E'; // Error indicator

// Function: _otp_populate
void _otp_populate(uint *param_1) {
  byte local_otp_data_buffer[4096]; // 0x1000 bytes for rotating secret
  uint initial_secret_offset;
  uint otp_current_index;
  uint loop_counter;

  // param_1[0x22] is uint, so 0x22 * sizeof(uint) = 0x88 bytes offset.
  initial_secret_offset = param_1[0x22] & 0xfff; // This is a byte offset into the secret

  // Copy 0x1000 - initial_secret_offset bytes from secret, starting at initial_secret_offset
  // This is the first part of a circular copy.
  memcpy(local_otp_data_buffer, (byte *)secret + initial_secret_offset, 0x1000 - initial_secret_offset);

  // If initial_secret_offset is not 0, copy the remaining initial_secret_offset bytes from the beginning of secret.
  // This completes the circular copy of 0x1000 bytes from 'secret' into 'local_otp_data_buffer',
  // effectively rotating the secret by 'initial_secret_offset' bytes.
  if (initial_secret_offset != 0) {
    memcpy(local_otp_data_buffer + (0x1000 - initial_secret_offset), (byte *)secret, initial_secret_offset);
  }

  // *param_1 stores the current length/index for the OTP buffer within the session object.
  otp_current_index = *param_1;

  // Populate the OTP buffer located at param_1 + 1 (i.e., bytes 4 to 0x83 relative to param_1).
  // The loop runs from the current OTP index up to 0x80 (128 bytes).
  for (loop_counter = otp_current_index; loop_counter < 0x80; loop_counter++) {
    // Destination: The OTP byte at (param_1 + 1) + loop_counter.
    // Source 1: A byte from param_1[0x21] (seed 1) based on (loop_counter & 3).
    // Source 2: A byte from the rotated secret buffer (local_otp_data_buffer)
    //           indexed by otp_current_index, wrapped around 0xfff.
    ((byte *)(param_1 + 1))[loop_counter] =
         ((byte *)(param_1 + 0x21))[(loop_counter & 3)] ^ local_otp_data_buffer[otp_current_index & 0xfff];
    otp_current_index += 2; // Increment the index for accessing local_otp_data_buffer
  }

  // Update param_1[0x22] (seed 2) with the final otp_current_index.
  param_1[0x22] = otp_current_index;
  // Set *param_1 to 0x80, indicating the OTP buffer is now full (128 bytes are available).
  *param_1 = 0x80;
}

// Function: _otp_consume
// param_1 is a pointer to the session object (uint *), where *param_1 is the current OTP length.
// bytes_to_consume is the number of OTP bytes to consume.
uint * _otp_consume(uint *param_1, uint bytes_to_consume) {
  // Check if there are enough OTP bytes available to consume.
  if (bytes_to_consume <= *param_1) {
    // Shift the OTP buffer content to the left.
    // The OTP data starts at (param_1 + 1) (i.e., byte offset 4 from param_1).
    // Source: Starts at (param_1 + 1) + bytes_to_consume (byte offset).
    // Destination: (param_1 + 1).
    // Number of bytes to copy: The remaining OTP bytes.
    memcpy((byte *)(param_1 + 1),
           (byte *)(param_1 + 1) + bytes_to_consume,
           *param_1 - bytes_to_consume);
    // Update the current OTP length stored in *param_1.
    *param_1 -= bytes_to_consume;
    return param_1; // Return the session object pointer on success.
  }
  // If not enough bytes, return NULL to indicate failure.
  return NULL;
}

// Function: otp_handshake
void otp_handshake(void **param_1) {
  byte challenge[8];       // Stores a random challenge
  undefined response[8];   // Stores the client's response
  undefined4 seed2_value;  // Stores a seed value read from client
  uint loop_counter;
  uint *session_obj;       // Pointer to the newly created session object

  // If an existing session object is pointed to by *param_1, free it.
  if (*param_1 != NULL) {
    free(*param_1);
    *param_1 = NULL;
  }

  // Generate 8 random bytes for the challenge.
  for (loop_counter = 0; loop_counter < 8; loop_counter++) {
      challenge[loop_counter] = (byte)random(); // random() returns long, cast to byte
  }

  // Write the generated challenge to standard output.
  fwrite(challenge, 1, 8, stdout);

  // Perform a nibble swap on each byte of the challenge.
  for (loop_counter = 0; loop_counter < 8; loop_counter++) {
    challenge[loop_counter] = (challenge[loop_counter] >> 4) | (challenge[loop_counter] << 4);
  }

  // Read 8 bytes of response from standard input.
  if (read_n(0, response, 8) == 8) {
    seed2_value = 0;
    // Read 4 bytes for seed2_value from standard input.
    if ((read_n(0, &seed2_value, 4) == 4) &&
       // Check if the client's response matches the (nibble-swapped) challenge OR the global secret.
       ((memcmp(challenge, response, 8) == 0) || (memcmp(secret, response, 8) == 0))) {

      // Allocate memory for the session object (0x90 bytes).
      session_obj = (uint *)malloc(0x90);
      if (session_obj == NULL) { // Handle malloc failure
          *param_1 = NULL;
          fwrite(&DAT_00015006, 1, 1, stdout); // Write error message
          return;
      }
      memset(session_obj, 0, 0x90); // Initialize the session object to zero.

      // Store seed2_value into param_1[0x21] (byte offset 0x84).
      session_obj[0x21] = seed2_value;

      // Calculate a sum of all unsigned int elements in the global 'secret' array
      // and store it (masked) into param_1[0x22] (byte offset 0x88), which serves as seed 1.
      for (loop_counter = 0; loop_counter < 0x400; loop_counter++) {
        session_obj[0x22] += secret[loop_counter];
      }
      session_obj[0x22] &= 0xfff; // Apply a mask to the sum.

      // Write the calculated seed 1 (masked sum) to standard output.
      fwrite(&session_obj[0x22], 1, 4, stdout);
      fwrite(&DAT_00015004, 1, 1, stdout); // Write success indicator.

      // Set a session counter/status byte at offset 0x8c (param_1[0x23]) to 3.
      ((byte *)session_obj)[0x8c] = 3;

      *param_1 = session_obj; // Store the address of the new session object.
      return;
    }
  }
  // If any condition fails (read error, mismatch), set *param_1 to NULL and send an error.
  *param_1 = NULL;
  fwrite(&DAT_00015006, 1, 1, stdout); // Write error indicator.
}

// Function: otp_generate_otp
void otp_generate_otp(uint *param_1) {
  uint requested_otp_length; // Stores the desired length of OTP to generate.
  uint loop_counter;         // Loop counter for printing OTP bytes.

  requested_otp_length = 0;
  // Read the requested OTP length (4 bytes) from standard input.
  // Check various conditions for a valid request:
  // - Read successful.
  // - Session object (param_1) is not NULL.
  // - Session counter (byte at param_1[0x23], offset 0x8c) is not zero.
  // - Requested length is within bounds (1 to 128 bytes).
  if (read_n(0, &requested_otp_length, 4) == 4 &&
      param_1 != NULL &&
      ((byte *)param_1)[0x8c] != '\0' &&
      requested_otp_length < 0x81 &&
      requested_otp_length != 0) {

    // If the current OTP buffer (*param_1) has fewer bytes than requested, repopulate it.
    if (*param_1 < requested_otp_length) {
      _otp_populate(param_1);
    }

    // Print the requested number of OTP bytes in hexadecimal format.
    // The OTP buffer starts at (param_1 + 1), which is byte offset 4 from param_1.
    for (loop_counter = 0; loop_counter < requested_otp_length; loop_counter++) {
      printf("%02X", (uint)((byte *)(param_1 + 1))[loop_counter]);
    }
    _otp_consume(param_1, requested_otp_length); // Consume the printed OTP bytes.
    fwrite(&DAT_00015004, 1, 1, stdout);         // Write success indicator.
    ((byte *)param_1)[0x8c]--;                   // Decrement the session counter.
  } else {
    fwrite(&DAT_00015006, 1, 1, stdout); // Write error indicator for invalid request.
  }
}

// Function: otp_extend_session
void otp_extend_session(uint *param_1) { // param_1 is the session object pointer
  byte received_data[8192];       // Buffer for received data (0x2000 bytes)
  ushort data_length;             // Length of data to extend
  byte *extended_data_buffer;     // Buffer for the extended data to be printed
  ssize_t bytes_read_until;       // Result of read_until call

  data_length = 0;
  // Read the length of the data to extend (2 bytes) from standard input.
  if (read_n(0, &data_length, 2) == 2) {
    // Read the actual data from standard input, up to 0x2000 bytes.
    bytes_read_until = read_until(0, received_data, 0x2000);

    // Check conditions for valid extension:
    // - Data was actually read.
    // - Session object (param_1) is not NULL.
    // - Data length is not zero.
    if (bytes_read_until != 0 && param_1 != NULL && data_length != 0) {
      // Allocate memory for the extended data buffer, plus one byte for a null prefix.
      extended_data_buffer = (byte *)malloc(data_length + 1);
      if (extended_data_buffer == NULL) { // Handle malloc failure
          fwrite(&DAT_00015006, 1, 1, stdout); // Write error message
          return;
      }
      extended_data_buffer[0] = 0; // Prefix the data with a null byte.
      memcpy(extended_data_buffer + 1, received_data, data_length); // Copy the received data.
      ((byte *)param_1)[0x8c] = 3; // Reset the session counter to 3.
      // Write the extended data (including the null prefix) to standard output.
      fwrite(extended_data_buffer, 1, data_length + 1, stdout);
      free(extended_data_buffer); // Free the allocated buffer.
      return;
    }
  }
  fwrite(&DAT_00015006, 1, 1, stdout); // Write error indicator for invalid extension.
}

// Function: otp_set_seeds
void otp_set_seeds(uint *param_1) { // param_1 is the session object pointer
  uint seed2_value; // Stores seed 2 read from client.
  uint seed1_value; // Stores seed 1 read from client.

  seed1_value = 0;
  seed2_value = 0;
  // Read seed 1 (4 bytes) and seed 2 (4 bytes) from standard input.
  // Check if both reads were successful and the session object (param_1) is not NULL.
  if (read_n(0, &seed1_value, 4) == 4 &&
      read_n(0, &seed2_value, 4) == 4 &&
      param_1 != NULL) {

    param_1[0x21] = seed1_value; // Store seed 1 into param_1[0x21].
    param_1[0x22] = seed2_value; // Store seed 2 into param_1[0x22].
    *param_1 = 0;                // Reset the current OTP buffer length to 0.
    // Clear the OTP buffer within the session object (128 bytes starting at param_1 + 1).
    memset(param_1 + 1, 0, 0x80);
    _otp_populate(param_1);      // Repopulate the OTP buffer with the new seeds.
    fwrite(&DAT_00015004, 1, 1, stdout); // Write success indicator.
  } else {
    fwrite(&DAT_00015006, 1, 1, stdout); // Write error indicator for invalid input or NULL session.
  }
}

// Function: otp_verify_otp
void otp_verify_otp(void) {
  // Declare a temporary session object on the stack (0x90 bytes = 36 uints).
  uint temp_session_obj[0x90 / sizeof(uint)];
  uint seed1_value;      // Stores seed 1 read from client.
  uint seed2_value;      // Stores seed 2 read from client.
  uint otp_length;       // Stores the length of the OTP to verify.
  byte received_otp[132]; // Buffer for the OTP received from client (max 132 bytes).

  seed1_value = 0;
  seed2_value = 0;
  otp_length = 0;

  // Read seed 2, seed 1, OTP length, and the received OTP data from standard input.
  // Check various conditions for a valid verification request:
  // - All reads successful.
  // - OTP length is within bounds (1 to 128 bytes).
  if (read_n(0, &seed2_value, 4) == 4 &&
      read_n(0, &seed1_value, 4) == 4 &&
      read_n(0, &otp_length, 4) == 4 &&
      read_n(0, received_otp, otp_length) == otp_length &&
      otp_length < 0x81 &&
      otp_length != 0) {

    // Initialize the temporary session object to zero.
    memset(temp_session_obj, 0, sizeof(temp_session_obj));

    // Set the seed values in the temporary session object.
    temp_session_obj[0x21] = seed1_value;
    temp_session_obj[0x22] = seed2_value;
    temp_session_obj[0] = 0; // Initialize current OTP length to 0.

    _otp_populate(temp_session_obj); // Populate the OTP buffer within temp_session_obj.

    // Compare the generated OTP (starting at temp_session_obj + 1, i.e., byte offset 4)
    // with the received OTP.
    if (memcmp((byte *)(temp_session_obj + 1), received_otp, otp_length) == 0) {
      fwrite(&DAT_00015004, 1, 1, stdout); // Write success indicator if OTP matches.
      return;
    }
  }
  fwrite(&DAT_00015006, 1, 1, stdout); // Write error indicator for any failure.
}