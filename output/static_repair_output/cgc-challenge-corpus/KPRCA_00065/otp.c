#include <stdio.h>    // For FILE, stdout, printf, fwrite, NULL
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For memcpy, memset, memcmp
#include <stdint.h>   // For uint32_t, uint8_t, uint16_t
#include <unistd.h>   // For read, STDIN_FILENO, ssize_t
#include <sys/random.h> // For getrandom (Linux-specific)

// Type definitions to match the decompiler's output
typedef uint32_t uint;
typedef uint8_t byte;
typedef uint16_t ushort;

// Global variables (placeholders for external data)
// Assuming 'secret' is an array of bytes used as a source for OTP generation.
// The size 0x1000 (4096) bytes is derived from _otp_populate.
// The loop in otp_handshake suggests it can be read as uint32_t.
byte secret[4096]; // Placeholder for global secret array.
char DAT_00015004 = 'O'; // Placeholder for success indicator char
char DAT_00015006 = 'F'; // Placeholder for failure indicator char

// Helper function: Reads exactly 'count' bytes from 'fd' into 'buf'.
// Returns the number of bytes read, or -1 on error.
ssize_t read_n(int fd, void *buf, size_t count) {
    ssize_t bytes_read = 0;
    ssize_t total_read = 0;
    while (total_read < count) {
        bytes_read = read(fd, (byte *)buf + total_read, count - total_read);
        if (bytes_read == -1) {
            return -1; // Error
        }
        if (bytes_read == 0) {
            break; // EOF
        }
        total_read += bytes_read;
    }
    return total_read;
}

// Helper function: Reads from 'fd' into 'buf' until a newline character is encountered
// or 'count' bytes have been read. Returns the number of bytes read (excluding newline),
// or -1 on error.
ssize_t read_until(int fd, void *buf, size_t count) {
    ssize_t total_read = 0;
    byte *b = (byte *)buf;
    while (total_read < count) {
        ssize_t bytes_read_single = read(fd, b + total_read, 1);
        if (bytes_read_single == -1) {
            return -1; // Error
        }
        if (bytes_read_single == 0) {
            break; // EOF
        }
        if (b[total_read] == '\n') {
            break; // Newline found
        }
        total_read++;
    }
    return total_read;
}

// Function: _otp_populate
// Populates the OTP buffer within the provided OTP structure.
// otp_struct layout:
// otp_struct[0]: Current OTP length/offset
// otp_struct[1]...otp_struct[0x20]: OTP data buffer (0x80 bytes)
// otp_struct[0x21]: Seed 1 (offset 0x84)
// otp_struct[0x22]: Seed 2 (offset 0x88)
// ((byte*)otp_struct)[0x8c]: Session counter
void _otp_populate(uint *otp_struct) {
    byte temp_otp_buffer[4096]; // Temporary buffer for OTP generation (local_1014)
    uint current_seed_offset = otp_struct[0x22] & 0xfff; // Seed 2 value, masked (local_14, first usage)

    // Copy a portion of the global 'secret' array into 'temp_otp_buffer'.
    // This handles the main part of a circular buffer fill.
    memcpy(temp_otp_buffer, secret + current_seed_offset, 0x1000 - current_seed_offset);

    // If the offset is not zero, copy the initial part of 'secret' to fill the wrap-around
    // portion of 'temp_otp_buffer', effectively making 'temp_otp_buffer' a circular copy of 'secret'.
    if (current_seed_offset != 0) {
        memcpy(temp_otp_buffer + (0x1000 - current_seed_offset), secret, current_seed_offset);
    }

    uint otp_data_length = otp_struct[0]; // Current OTP data length (local_14, second usage, renamed for clarity)
    
    // Fill the OTP data portion of the 'otp_struct'.
    // The OTP data starts at (byte*)(otp_struct + 1), which is 4 bytes after otp_struct[0].
    for (uint i = otp_data_length; i < 0x80; ++i) { // i corresponds to local_10
        // Calculate the OTP byte: XOR a byte from Seed 1 with a byte from the temp_otp_buffer.
        // ((byte*)&otp_struct[0x21])[(i & 3)] accesses bytes within Seed 1 (otp_struct[0x21]).
        // temp_otp_buffer[otp_data_length & 0xfff] accesses a byte from the circularly copied secret.
        ((byte *)otp_struct)[i + 4] =
            ((byte *)&otp_struct[0x21])[(i & 3)] ^ temp_otp_buffer[otp_data_length & 0xfff];
        otp_data_length += 2; // Increment for the next OTP byte generation
    }

    otp_struct[0x22] = otp_data_length; // Update Seed 2 in the struct with the new offset
    otp_struct[0] = 0x80;              // Set the current OTP data length to its maximum (0x80 bytes)
}

// Function: _otp_consume
// Consumes a specified number of OTP bytes from the OTP structure.
// otp_struct: Pointer to the OTP structure.
// length_to_consume: The number of OTP bytes to consume.
void _otp_consume(uint *otp_struct, uint length_to_consume) {
    // Check if there is enough OTP data available to consume.
    if (length_to_consume <= otp_struct[0]) {
        // Shift the remaining OTP data within the buffer.
        // The OTP data starts at (byte*)(otp_struct + 1).
        // Source is the data after the consumed part: (byte*)(otp_struct + 1) + length_to_consume.
        // Destination is the beginning of the OTP data buffer: (byte*)(otp_struct + 1).
        // Size to move is the total current length minus the length consumed.
        memcpy((byte *)(otp_struct + 1), (byte *)(otp_struct + 1) + length_to_consume,
               otp_struct[0] - length_to_consume);
        otp_struct[0] -= length_to_consume; // Update the current OTP data length.
    }
    // The original code returned a pointer, but its usage implies a void return is appropriate.
}

// Function: otp_handshake
// Performs an OTP handshake, involving challenge-response and session setup.
// otp_session_ptr: Pointer to a void* which will store the allocated OTP session structure.
void otp_handshake(void **otp_session_ptr) {
    byte challenge_local[8];  // Buffer for the generated challenge
    byte response_local[8];   // Buffer for the received response
    uint seed1_val;           // Value for Seed 1, read from input
    void *otp_struct_ptr;     // Pointer to the newly allocated OTP session structure

    // If an existing session pointer is not NULL, free the old session memory.
    if (*otp_session_ptr != NULL) {
        free(*otp_session_ptr);
        *otp_session_ptr = NULL;
    }

    // Generate 8 random bytes for the challenge.
    // getrandom is a Linux-specific system call for cryptographic randomness.
    getrandom(challenge_local, 8, 0);

    // Write the challenge to standard output.
    fwrite(challenge_local, 1, 8, stdout);

    // Apply a nibble swap to each byte of the challenge in-place.
    for (uint i = 0; i < 8; ++i) {
        challenge_local[i] = (challenge_local[i] >> 4) | (challenge_local[i] << 4);
    }

    // Read 8 bytes for the response from standard input.
    if (read_n(STDIN_FILENO, response_local, 8) != 8) {
        *otp_session_ptr = NULL;
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure
        return;
    }

    // Read 4 bytes for Seed 1 value from standard input.
    if (read_n(STDIN_FILENO, &seed1_val, 4) != 4) {
        *otp_session_ptr = NULL;
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure
        return;
    }

    // Verify the response: it must match either the modified challenge or the global secret.
    if (memcmp(challenge_local, response_local, 8) == 0 || memcmp(secret, response_local, 8) == 0) {
        // Allocate memory for the OTP session structure (0x90 bytes).
        otp_struct_ptr = malloc(0x90);
        if (otp_struct_ptr == NULL) {
            *otp_session_ptr = NULL;
            fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure due to allocation error
            return;
        }
        memset(otp_struct_ptr, 0, 0x90); // Initialize the structure to zeros.

        // Set Seed 1 in the structure (at offset 0x84, which is otp_struct_ptr[0x21]).
        ((uint *)otp_struct_ptr)[0x21] = seed1_val;

        // Calculate Seed 2 (at offset 0x88, which is otp_struct_ptr[0x22]).
        // It's a sum of uints from the global 'secret' array, masked by 0xfff.
        ((uint *)otp_struct_ptr)[0x22] = 0; // Initialize directly to reduce variable count
        for (uint i = 0; i < 0x400; ++i) {
            ((uint *)otp_struct_ptr)[0x22] += ((uint *)secret)[i];
        }
        ((uint *)otp_struct_ptr)[0x22] &= 0xfff;

        // Write the calculated Seed 2 to standard output.
        fwrite(&((uint *)otp_struct_ptr)[0x22], 1, 4, stdout);
        fwrite(&DAT_00015004, 1, 1, stdout); // Indicate success.

        // Set the session counter in the structure (at offset 0x8c) to 3.
        ((byte *)otp_struct_ptr)[0x8c] = 3;

        *otp_session_ptr = otp_struct_ptr; // Store the new session structure pointer.
        return;
    }

    // If verification fails, set the session pointer to NULL and indicate failure.
    *otp_session_ptr = NULL;
    fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
}

// Function: otp_generate_otp
// Generates and prints a specified number of OTP bytes from the session.
// otp_struct: Pointer to the current OTP session structure.
void otp_generate_otp(uint *otp_struct) {
    uint requested_otp_length; // The number of OTP bytes requested by the user.
    
    // Read the requested OTP length from standard input.
    if (read_n(STDIN_FILENO, &requested_otp_length, 4) != 4) {
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
        return;
    }

    // Check for valid session, active session counter, and valid requested length.
    if (otp_struct != NULL && ((char *)otp_struct)[0x8c] != '\0' &&
        requested_otp_length > 0 && requested_otp_length < 0x81) {

        // If the current OTP buffer does not contain enough bytes, populate it.
        if (otp_struct[0] < requested_otp_length) {
            _otp_populate(otp_struct);
        }

        // Print the requested number of OTP bytes in hexadecimal format.
        // OTP data starts at (byte*)(otp_struct + 1).
        for (uint i = 0; i < requested_otp_length; ++i) {
            printf("%02X", (uint)((byte *)(otp_struct + 1))[i]);
        }

        _otp_consume(otp_struct, requested_otp_length); // Consume the generated OTP bytes.
        fwrite(&DAT_00015004, 1, 1, stdout);            // Indicate success.
        ((char *)otp_struct)[0x8c]--;                   // Decrement the session counter.
    } else {
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
    }
}

// Function: otp_extend_session
// Extends the OTP session with new data.
// otp_session_ptr: Pointer to the current OTP session structure.
void otp_extend_session(void *otp_session_ptr) {
    ushort data_length;       // Expected length of data to be read.
    byte temp_buffer[8192];   // Temporary buffer to read incoming data.
    byte *allocated_data = NULL; // Pointer for dynamically allocated data.
    ssize_t bytes_read;       // Actual number of bytes read by read_until.

    // Read the expected data length from standard input.
    if (read_n(STDIN_FILENO, &data_length, 2) != 2) {
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
        return;
    }

    // Read data from standard input until newline or buffer full.
    bytes_read = read_until(STDIN_FILENO, temp_buffer, sizeof(temp_buffer));
    if (bytes_read <= 0) { // Check for read error or no data.
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
        return;
    }

    // Check for valid session, non-zero data_length, and sufficient data read.
    if (otp_session_ptr != NULL && data_length > 0 && data_length <= bytes_read) {
        // Allocate memory for the data plus a null terminator.
        allocated_data = (byte *)malloc(data_length + 1);
        if (allocated_data == NULL) {
            fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure due to allocation error.
            return;
        }
        
        allocated_data[0] = 0; // Set the first byte to 0 (potential string prefix).
        memcpy(allocated_data + 1, temp_buffer, data_length); // Copy the actual data.
        
        // Update the session counter in the OTP structure (at offset 0x8c).
        ((byte *)otp_session_ptr)[0x8c] = 3;
        
        // Write the allocated data (including the leading null) to standard output.
        fwrite(allocated_data, 1, data_length + 1, stdout);
        free(allocated_data); // Free the allocated memory.
        return;
    }

    fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
}

// Function: otp_set_seeds
// Sets new seed values for the OTP generation.
// otp_struct: Pointer to the current OTP session structure.
void otp_set_seeds(uint *otp_struct) {
    uint new_seed1_val; // New value for Seed 1.
    uint new_seed2_val; // New value for Seed 2.

    // Read new Seed 1 value from standard input.
    if (read_n(STDIN_FILENO, &new_seed1_val, 4) != 4) {
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
        return;
    }
    // Read new Seed 2 value from standard input.
    if (read_n(STDIN_FILENO, &new_seed2_val, 4) != 4) {
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
        return;
    }

    // Check for a valid OTP structure.
    if (otp_struct != NULL) {
        otp_struct[0x21] = new_seed1_val; // Set Seed 1 (at offset 0x84).
        otp_struct[0x22] = new_seed2_val; // Set Seed 2 (at offset 0x88).
        otp_struct[0] = 0;               // Reset the current OTP data length to 0.
        // Clear the OTP data buffer (0x80 bytes starting at otp_struct[1]).
        memset(otp_struct + 1, 0, 0x80);
        
        _otp_populate(otp_struct);       // Repopulate the OTP buffer with new seeds.
        fwrite(&DAT_00015004, 1, 1, stdout); // Indicate success.
    } else {
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
    }
}

// Function: otp_verify_otp
// Verifies a received OTP against a locally generated one using provided seeds.
void otp_verify_otp(void) {
    uint seed1_received;    // Seed 1 value received from input.
    uint seed2_received;    // Seed 2 value received from input.
    uint otp_length;        // Length of the OTP to verify.
    byte received_otp[132]; // Buffer to store the received OTP.
    
    // Define a local OTP structure for verification purposes.
    // It needs to be 0x90 bytes, similar to the session structure.
    uint local_otp_struct[0x90 / sizeof(uint)]; 

    // Read Seed 1 value from standard input.
    if (read_n(STDIN_FILENO, &seed1_received, 4) != 4) {
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
        return;
    }
    // Read Seed 2 value from standard input.
    if (read_n(STDIN_FILENO, &seed2_received, 4) != 4) {
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
        return;
    }
    // Read the OTP length from standard input.
    if (read_n(STDIN_FILENO, &otp_length, 4) != 4) {
        fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure.
        return;
    }

    // Check for valid OTP length and read the received OTP data.
    if (otp_length > 0 && otp_length < 0x81 &&
        read_n(STDIN_FILENO, received_otp, otp_length) == otp_length) {

        memset(local_otp_struct, 0, 0x90); // Initialize the local OTP structure to zeros.

        // Set the received seeds into the local OTP structure.
        local_otp_struct[0x21] = seed1_received; // Offset 0x84
        local_otp_struct[0x22] = seed2_received; // Offset 0x88

        // Populate the OTP buffer within the local structure using the received seeds.
        _otp_populate(local_otp_struct);

        // Compare the locally generated OTP with the received OTP.
        // The OTP data starts at (byte*)(local_otp_struct + 1), which is 4 bytes after local_otp_struct[0].
        if (memcmp(((byte *)local_otp_struct) + 4, received_otp, otp_length) == 0) {
            fwrite(&DAT_00015004, 1, 1, stdout); // Indicate success if they match.
            return;
        }
    }
    
    fwrite(&DAT_00015006, 1, 1, stdout); // Indicate failure if any condition fails or OTPs don't match.
}