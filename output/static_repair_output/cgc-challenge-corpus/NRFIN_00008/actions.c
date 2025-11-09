#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h> // For write, read, ssize_t

// --- Stubs for external functions/variables ---
// These functions are not provided in the snippet and are defined
// or replaced with standard equivalents for compilation.

// `streq` compares two strings.
int streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

// `int2str` converts an integer to a string.
void int2str(int num, char *buf, size_t len) {
    snprintf(buf, len, "%d", num);
}

// `sendall` sends data to stdout (fd 1).
ssize_t sendall(int fd, const void *buf, size_t len) {
    return write(fd, buf, len);
}

// `sendline` sends data to stdout (fd 1) followed by a newline.
ssize_t sendline(int fd, const void *buf, size_t len) {
    ssize_t res = write(fd, buf, len);
    if (res >= 0) {
        write(fd, "\n", 1);
    }
    return res;
}

// `recvline` reads a line from stdin (fd 0).
// Simplified: reads up to `len-1` bytes or until newline, null-terminates.
ssize_t recvline(int fd, char *buf, size_t len) {
    ssize_t bytes_read = 0;
    if (len == 0) return 0;

    for (size_t i = 0; i < len - 1; ++i) {
        if (read(fd, &buf[i], 1) <= 0) {
            // Error or EOF
            if (i == 0) return -1; // No bytes read
            break;
        }
        if (buf[i] == '\n') {
            bytes_read = i;
            break;
        }
        bytes_read = i + 1;
    }
    buf[bytes_read] = '\0';
    return bytes_read;
}

// `_terminate` exits the program with a given exit code.
void _terminate(int exit_code) {
    exit(exit_code);
}

// `allocate` allocates memory. Returns a pointer to the allocated memory or NULL on failure.
void *allocate(size_t size) {
    return malloc(size);
}

// `deallocate` frees memory. Returns 0 on success, non-zero on failure.
// The `size` parameter is likely for internal tracking in the original system.
int deallocate(void *ptr, size_t size) {
    (void)size; // Suppress unused parameter warning
    free(ptr);
    return 0; // Assume free always succeeds for this stub
}

// `decode` is a placeholder for a decoding function.
void decode(void *data_ptr, uint32_t key) {
    // Placeholder implementation, actual logic unknown
    (void)data_ptr;
    (void)key;
}

// `str2uint` converts a string to an unsigned integer.
unsigned int str2uint(const char *s) {
    return (unsigned int)strtoul(s, NULL, 10);
}

// `b2hex` converts a byte to a 2-character hex string.
void b2hex(uint8_t b, char *buf) {
    snprintf(buf, 3, "%02x", b);
}

// --- Global variables ---
// Using char* to represent raw memory blocks for Video structs,
// as offsets in the decompiled code suggest byte-level manipulation.
typedef char Video_RawMem; // Represents a dynamically allocated block for a video entry

// Global head of the linked list
Video_RawMem *vhead = NULL;

// Placeholder for `r_bin` and `r_bin_len`
uint8_t *r_bin = (uint8_t*) "DUMMY_R_BIN_DATA_FOR_TESTING_1234567890ABCDEF"; // Dummy data
size_t r_bin_len = 40; // Dummy length

// Placeholder for `DAT_00013000`
const char DAT_00013000[] = " - "; // Assuming this is the value based on length usage

// --- Helper macros for accessing fields within the Video_RawMem blocks ---
// These macros cast the `Video_RawMem *` (which is `char *`) to appropriate pointer types
// to access data at specific byte offsets.

#define GET_NEXT_PTR(v_ptr)         (*(Video_RawMem**)((v_ptr) + 0x0))
#define SET_NEXT_PTR(v_ptr, val)    (*(Video_RawMem**)((v_ptr) + 0x0) = (val))

#define GET_PREV_PTR(v_ptr)         (*(Video_RawMem**)((v_ptr) + 0x4))
#define SET_PREV_PTR(v_ptr, val)    (*(Video_RawMem**)((v_ptr) + 0x4) = (val))

#define GET_NAME_PTR(v_ptr)         ((char*)((v_ptr) + 0x8))

#define GET_ACTIVE_FLAG(v_ptr)      (((uint8_t*)((v_ptr) + 0x27))[0])
#define SET_ACTIVE_FLAG(v_ptr, val) (((uint8_t*)((v_ptr) + 0x27))[0] = (val))

#define GET_DATA_BUFFER_PTR_FIELD(v_ptr)  (*(uint32_t*)((v_ptr) + 0x88))
#define SET_DATA_BUFFER_PTR_FIELD(v_ptr, val) (*(uint32_t*)((v_ptr) + 0x88) = (val))

#define GET_DATA_LENGTH_FIELD(v_ptr)      (*(uint32_t*)((v_ptr) + 0x8c))
#define SET_DATA_LENGTH_FIELD(v_ptr, val) (*(uint32_t*)((v_ptr) + 0x8c) = (val))

#define GET_PLAYBACK_DATA1_FIELD(v_ptr)   (*(uint32_t*)((v_ptr) + 0x90))
#define SET_PLAYBACK_DATA1_FIELD(v_ptr, val) (*(uint32_t*)((v_ptr) + 0x90) = (val))

#define GET_PLAYBACK_DATA2_FIELD(v_ptr)   (*(uint32_t*)((v_ptr) + 0x94))
#define SET_PLAYBACK_DATA2_FIELD(v_ptr, val) (*(uint32_t*)((v_ptr) + 0x94) = (val))

#define GET_ACCESS_KEY(v_ptr)       (((uint8_t*)((v_ptr) + 0x97))[0])
#define SET_ACCESS_KEY(v_ptr, val)  (((uint8_t*)((v_ptr) + 0x97))[0] = (val))

#define GET_UNKNOWN_VAL_0x98_FIELD(v_ptr) (*(uint32_t*)((v_ptr) + 0x98))
#define SET_UNKNOWN_VAL_0x98_FIELD(v_ptr, val) (*(uint32_t*)((v_ptr) + 0x98) = (val))

// --- Main functions as requested ---

// Function: get_video_by_name
Video_RawMem * get_video_by_name(const char *name_param) {
  Video_RawMem *current = vhead;
  while(current != NULL) {
    if (streq(GET_NAME_PTR(current), name_param)) {
      return current;
    }
    current = GET_NEXT_PTR(current);
  }
  return NULL;
}

// Function: list
void list(void) {
  char num_str[16]; // Buffer for integer to string conversion
  int video_idx = 1;
  
  for (Video_RawMem *current = vhead; current != NULL; current = GET_NEXT_PTR(current)) {
    int2str(video_idx, num_str, sizeof(num_str));
    if (sendall(1, num_str, strlen(num_str)) < 0) {
      _terminate(3); // Original exit code 3
    }

    if (sendall(1, DAT_00013000, 3) < 0) { // DAT_00013000 is " - ", 3 bytes long
      _terminate(3); // Original exit code 3
    }

    if (sendline(1, GET_NAME_PTR(current), strlen(GET_NAME_PTR(current))) < 0) {
      _terminate(6); // Original exit code 6
    }
    video_idx++;
  }
}

// Function: play
void play(const char *param_1) {
  if (streq("trolololo A", param_1)) {
    // Branch 1: "trolololo A" case
    Video_RawMem *temp_video = (Video_RawMem *)allocate(0xA0); // Allocate header only
    if (temp_video == NULL) {
      if (sendline(1, "Clever girl.", 12) < 0) { /* If this fails, _terminate will handle */ }
      _terminate(9); // Original exit code 9
    }

    SET_ACCESS_KEY(temp_video, 0x41);
    SET_DATA_BUFFER_PTR_FIELD(temp_video, (uint32_t)r_bin);
    SET_DATA_LENGTH_FIELD(temp_video, r_bin_len - 4);
    // Assuming r_bin is the base address used in `UNK_00014ffc` reference.
    SET_UNKNOWN_VAL_0x98_FIELD(temp_video, *(uint32_t*)(r_bin + r_bin_len)); 

    decode(temp_video, 0x41); // Decode with key 0x41

    if (GET_PLAYBACK_DATA1_FIELD(temp_video) != 0) {
      // Playback successful
      if (sendline(1, (void*)GET_PLAYBACK_DATA1_FIELD(temp_video), GET_PLAYBACK_DATA2_FIELD(temp_video) & 0xffffff) < 0) { _terminate(6); }
      if (sendline(1, "PLAYBACK FINISHED", 17) < 0) { _terminate(6); }

      // Deallocate data buffer first
      if (deallocate((void*)GET_PLAYBACK_DATA1_FIELD(temp_video), GET_PLAYBACK_DATA2_FIELD(temp_video) & 0xffffff) != 0) {
        if (sendline(1, "Clever girl.", 12) < 0) { /* If this fails, _terminate will handle */ }
        _terminate(0x12); // Original exit code 0x12
      }
    } else {
      // Playback failed
      if (sendline(1, "I hate this hacker crap!", 24) < 0) { _terminate(6); }
    }

    // Deallocate the video struct header
    if (deallocate(temp_video, 0xA0) != 0) {
      if (sendline(1, "Clever girl.", 12) < 0) { /* If this fails, _terminate will handle */ }
      _terminate(0x12); // Original exit code 0x12
    }
  } else {
    // Branch 2: General play command (e.g., "video_name X")
    char *mutable_param = strdup(param_1);
    if (mutable_param == NULL) { _terminate(1); } // Malloc failed, exit with arbitrary code

    char *space_char_ptr = strchr(mutable_param, ' ');
    if (space_char_ptr == NULL) {
      if (sendline(1, "I hate this hacker crap!", 24) < 0) { _terminate(6); }
      free(mutable_param);
      return;
    }

    *space_char_ptr = '\0'; // Null-terminate the video name part
    char key_char = space_char_ptr[1]; // Get the character after the space

    // Check if there's exactly one character after the space
    if (key_char == '\0' || space_char_ptr[2] != '\0') {
      if (sendline(1, "I hate this hacker crap!", 24) < 0) { _terminate(6); }
      free(mutable_param);
      return;
    }

    Video_RawMem *found_video = get_video_by_name(mutable_param);
    if (found_video == NULL) {
      if (sendline(1, "I hate this hacker crap!", 24) < 0) { _terminate(6); }
      free(mutable_param);
      return;
    }
    free(mutable_param); // Done with mutable_param

    decode(found_video, key_char); // Decode with the provided key_char

    if (GET_PLAYBACK_DATA1_FIELD(found_video) != 0) {
      // Playback successful
      if (sendline(1, (void*)GET_PLAYBACK_DATA1_FIELD(found_video), GET_PLAYBACK_DATA2_FIELD(found_video) & 0xffffff) < 0) { _terminate(6); }
      if (sendline(1, "PLAYBACK FINISHED", 17) < 0) { _terminate(6); }
    } else {
      // Playback failed
      if (sendline(1, "I hate this hacker crap!", 24) < 0) { _terminate(6); }
    }
  }
}

// Function: add
void add(char *param_1) {
  char access_key_hex[3]; // For b2hex output (2 chars + null)
  char length_str_buf[16]; // For recvline and str2uint, max 10 chars + null
  unsigned int data_len;
  Video_RawMem *new_video = NULL;
  Video_RawMem *last_video = NULL;

  size_t name_len = strlen(param_1);
  if (name_len > 0x7f || name_len == 0) {
    if (sendline(1, "You didn't say the magic word.", 30) < 0) { /* If this fails, _terminate will handle */ }
    _terminate(0xe); // Original exit code 0xe
  }

  // Check if video already exists or name contains space
  if (get_video_by_name(param_1) != NULL || strchr(param_1, ' ') != NULL) {
    if (sendline(1, "It's a UNIX system, I know this!", 32) < 0) { /* If this fails, _terminate will handle */ }
    return; // Original code returns if sendline successful
  }

  // Prompt for length
  if (sendall(1, "Length: ", 8) < 0) { _terminate(3); } // Original exit code 3

  // Receive length
  if (recvline(0, length_str_buf, sizeof(length_str_buf)) < 0) {
    if (sendline(1, "You didn't say the magic word.", 30) < 0) { /* If this fails, _terminate will handle */ }
    _terminate(2); // Original exit code 2
  }

  // Convert and validate length
  data_len = str2uint(length_str_buf);
  if (data_len > 0x20400 || data_len < 8 || (data_len & 3) != 0) {
    if (sendline(1, "You didn't say the magic word.", 30) < 0) { /* If this fails, _terminate will handle */ }
    _terminate(0xd); // Original exit code 0xd
  }

  // Allocate memory for video struct header (0xA0 bytes) + data
  new_video = (Video_RawMem *)allocate(data_len + 0xA0);
  if (new_video == NULL) {
    if (sendline(1, "Clever girl.", 12) < 0) { /* If this fails, _terminate will handle */ }
    _terminate(9); // Original exit code 9
  }

  SET_ACCESS_KEY(new_video, 0x42);
  b2hex(GET_ACCESS_KEY(new_video), access_key_hex);

  // Send access key info
  if (sendall(1, "Your personal access key is: 0x", 31) < 0) { _terminate(3); } // Original exit code 3
  if (sendline(1, access_key_hex, 2) < 0) { _terminate(6); } // Original exit code 6

  SET_ACTIVE_FLAG(new_video, 1);
  strcpy(GET_NAME_PTR(new_video), param_1);

  // Receive data into the allocated block, starting at offset 0x28
  if (recv(0, (uint8_t*)new_video + 0x28, data_len, 0) < 0) {
    if (sendline(1, "You didn't say the magic word.", 30) < 0) { /* If this fails, _terminate will handle */ }
    _terminate(4); // Original exit code 4
  }

  // Set data-related fields in the struct
  SET_DATA_LENGTH_FIELD(new_video, data_len - 4);
  SET_DATA_BUFFER_PTR_FIELD(new_video, (uint32_t)((uint8_t*)new_video + 0x28));
  SET_UNKNOWN_VAL_0x98_FIELD(new_video, *(uint32_t*)((uint8_t*)new_video + 0x28 + (data_len - 4)));

  // Add to linked list
  if (vhead == NULL) {
    vhead = new_video;
  } else {
    for (last_video = vhead; GET_NEXT_PTR(last_video) != NULL; last_video = GET_NEXT_PTR(last_video));
    SET_NEXT_PTR(last_video, new_video);
    SET_PREV_PTR(new_video, last_video);
  }
  SET_NEXT_PTR(new_video, NULL); // Ensure new_video is the last element
}

// Function: remove_video (renamed to avoid conflict with stdio.h remove)
int remove_video(char *filename_param) {
  Video_RawMem *video_to_remove = get_video_by_name(filename_param);
  if (video_to_remove == NULL) {
    if (sendline(1, "I hate this hacker crap!", 24) < 0) { _terminate(6); } // Original exit code 6
    return -1; // Indicate error
  }

  if (GET_ACTIVE_FLAG(video_to_remove) == 0) {
    if (sendline(1, "I hate this hacker crap!", 24) < 0) { _terminate(6); } // Original exit code 6
    return -1; // Indicate error
  }

  // Remove from linked list
  if (GET_PREV_PTR(video_to_remove) != NULL) { // If not head
    SET_NEXT_PTR(GET_PREV_PTR(video_to_remove), GET_NEXT_PTR(video_to_remove));
  } else { // If head
    vhead = GET_NEXT_PTR(video_to_remove);
  }
  if (GET_NEXT_PTR(video_to_remove) != NULL) { // If not tail
    SET_PREV_PTR(GET_NEXT_PTR(video_to_remove), GET_PREV_PTR(video_to_remove));
  }

  // Deallocate associated data buffer if it exists
  if (GET_PLAYBACK_DATA1_FIELD(video_to_remove) != 0) {
    if (deallocate((void*)GET_PLAYBACK_DATA1_FIELD(video_to_remove), GET_PLAYBACK_DATA2_FIELD(video_to_remove) & 0xffffff) != 0) {
      if (sendline(1, "Clever girl.", 12) < 0) { /* If this fails, _terminate will handle */ }
      _terminate(0x12); // Original exit code 0x12
    }
  }

  // Deallocate the video struct itself (header + embedded data)
  if (deallocate(video_to_remove, GET_DATA_LENGTH_FIELD(video_to_remove) + 0xA0) != 0) {
    if (sendline(1, "Clever girl.", 12) < 0) { /* If this fails, _terminate will handle */ }
    _terminate(0x12); // Original exit code 0x12
  }

  if (sendline(1, "Creation is an act of sheer will. Next time it'll be flawless.", 62) < 0) { _terminate(6); } // Original exit code 6
  return 0; // Success
}

// Function: quit
void quit(void) {
  if (sendline(1, "God help us, we're in the hands of engineers...", 47) < 0) {
    _terminate(6); // Original exit code 6
  }
  _terminate(0); // Original exit code 0
}