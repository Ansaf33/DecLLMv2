#include <stdio.h>    // For printf, sprintf
#include <stdlib.h>   // For malloc, free, strtoul
#include <string.h>   // For strlen, strcpy, strncpy, strsep, memset, strcmp, strncmp
#include <unistd.h>   // For read, ssize_t
#include <stdbool.h>  // For bool type

// Decompiler types replaced with standard C types
typedef unsigned int uint;
typedef unsigned long ulong;

// Assume a job slot is 512 bytes (0x200)
#define JOB_SLOT_SIZE 0x200
#define MAX_JOBS 1000

// Offsets for the overall printer state
#define PRINTER_STATE_OFFSET_INT 0 // This is *param_1 in cmd_print_jobs, printer_tick

// The `0x104` offset is where the main job data structure starts within the printer memory block.
#define JOB_DATA_STRUCT_OFFSET 0x104

// Offsets for fields within the JobData structure (relative to JOB_DATA_STRUCT_OFFSET)
// These are `int` offsets from `(int*)job_data_ptr`
#define JOB_STATUS_FIELD_INT_OFFSET 0       // int: Job status (0=Invalid, 1=Waiting, 2=Queued, 3=Printing)
#define JOB_DATA_PTR_FIELD_INT_OFFSET 1     // int: Pointer to data file content (void*)
#define JOB_DATA_SIZE_FIELD_INT_OFFSET 2    // int: Size of data file
#define JOB_TIMER_FIELD_INT_OFFSET 3        // int: Countdown timer (used in printer_tick)
#define JOB_NAME_FIELD_INT_OFFSET 4         // char[32]: 'C' command (0x104 + 4*4 = 0x114)
#define JOB_HOST_FIELD_INT_OFFSET 12        // char[32]: 'H' command (0x104 + 12*4 = 0x134)
#define JOB_ID_INT_FIELD_INT_OFFSET 20      // int: 'I' command (0x104 + 20*4 = 0x154)
#define JOB_ID_STR_FIELD_INT_OFFSET 21      // char[100]: 'J' command (0x104 + 21*4 = 0x158)
#define JOB_SIZE_LOW_FIELD_INT_OFFSET 104   // unsigned int: 'S' command (0x104 + 104*4 = 0x2A4)
#define JOB_SIZE_HIGH_FIELD_INT_OFFSET 105  // unsigned int: 'S' command (0x104 + 105*4 = 0x2A8)
#define JOB_TITLE_FIELD_INT_OFFSET 106      // char[256]: 'T' command (0x104 + 106*4 = 0x2AC)
#define JOB_TIME_FIELD_INT_OFFSET 126       // unsigned int: 'S' command related (0x104 + 126*4 = 0x2FC)
#define JOB_CLASS_FIELD_INT_OFFSET 127      // char: 'S' command related (0x104 + 127*4 = 0x300)

// Offsets for fields within the JobData structure (relative to JOB_DATA_STRUCT_OFFSET), using byte offsets directly
#define JOB_USER_FIELD_BYTE_OFFSET (JOB_DATA_STRUCT_OFFSET + 0xB9)      // char[32]: 'L' command (0x1BD)
#define JOB_USER_FLAG_FIELD_BYTE_OFFSET (JOB_DATA_STRUCT_OFFSET + 0x78) // char: 'L' command related (0x17C)
#define JOB_MAIL_USER_FIELD_BYTE_OFFSET (JOB_DATA_STRUCT_OFFSET + 0xFA) // char[32]: 'M' command (0x1FE)
#define JOB_MAIL_FLAG_FIELD_BYTE_OFFSET (JOB_DATA_STRUCT_OFFSET + 0xF9) // char: 'M' command related (0x1FD)
#define JOB_PRINTER_FIELD_BYTE_OFFSET (JOB_DATA_STRUCT_OFFSET + 0xD9)   // char[32]: 'P' command (0x1DD)
#define JOB_FILENAME_FIELD_BYTE_OFFSET (JOB_DATA_STRUCT_OFFSET + 0x11A) // char[131]: 'N' command (0x21E)


// Helper function to read exactly n bytes from a file descriptor
ssize_t read_n(int fd, void *buf, size_t count) {
    size_t bytes_read = 0;
    while (bytes_read < count) {
        ssize_t result = read(fd, (char *)buf + bytes_read, count - bytes_read);
        if (result == -1) {
            return -1; // Error
        } else if (result == 0) {
            break; // EOF
        }
        bytes_read += result;
    }
    return bytes_read;
}

// Function: jstate_str
char *jstate_str(unsigned int status) {
    if (status == 3) {
        return "Printing";
    }
    if (status == 2) {
        return "Queued";
    }
    if (status == 1) {
        return "Waiting";
    }
    if (status == 0) {
        return "Invalid";
    }
    return "Unknown";
}

// Function: print_ws
void print_ws(int count) {
    if (count <= 0) {
        return;
    }
    // Allocate a small buffer on stack, fill with spaces, print.
    char buffer[32];
    if (count >= sizeof(buffer)) {
        count = sizeof(buffer) - 1;
    }
    memset(buffer, ' ', count);
    buffer[count] = '\0';
    printf("%s", buffer);
}

// Function: cmd_print_jobs
unsigned int cmd_print_jobs(int *printer_mem_base) {
    // printer_mem_base is assumed to be the base pointer to the overall printer state
    // *printer_mem_base is the main printer state (e.g., 0, 1, 2, 3)

    int printer_state = *printer_mem_base;

    for (int i = 0; i < MAX_JOBS; ++i) {
        // Calculate the pointer to the job's status field.
        int *job_status_ptr = (int *)((char *)printer_mem_base + JOB_DATA_STRUCT_OFFSET + (i * JOB_SLOT_SIZE) + (JOB_STATUS_FIELD_INT_OFFSET * sizeof(int)));

        if (*job_status_ptr == 2) {
            *job_status_ptr = 3; // Set job status to printing
            if (printer_state != 3) { // If printer state is not already 3 (printing)
                *printer_mem_base = 3; // Set printer state to printing
                printer_state = 3; // Update cached state
            }
        }
    }
    return 0;
}

// Function: cmd_recv_job
unsigned int cmd_recv_job(int *printer_mem_base) {
    // printer_mem_base is assumed to be the base pointer to the overall printer state
    // *printer_mem_base is the main printer state
    return (*printer_mem_base == 3) ? 0xffffffff : (*printer_mem_base = 2, 0);
}

// Function: cmd_abort_job
unsigned int cmd_abort_job(int *printer_mem_base) {
    // printer_mem_base is assumed to be the base pointer to the overall printer state
    // *printer_mem_base is the main printer state
    bool has_printing_jobs = false;

    for (int i = 0; i < MAX_JOBS; ++i) {
        // Calculate base pointers for the current job slot
        char *job_slot_base = (char *)printer_mem_base + JOB_DATA_STRUCT_OFFSET + (i * JOB_SLOT_SIZE);
        int *job_status_ptr = (int *)(job_slot_base + (JOB_STATUS_FIELD_INT_OFFSET * sizeof(int)));
        void **job_data_ptr_field = (void **)(job_slot_base + (JOB_DATA_PTR_FIELD_INT_OFFSET * sizeof(int)));

        if (*job_status_ptr == 1 || *job_status_ptr == 2) { // If Waiting or Queued
            if (*job_data_ptr_field != NULL) {
                free(*job_data_ptr_field);
                *job_data_ptr_field = NULL; // Clear pointer after freeing
            }
            // Clear the entire job slot and reset status
            memset(job_slot_base, 0, JOB_SLOT_SIZE);
        } else if (*job_status_ptr == 3) { // If Printing
            has_printing_jobs = true;
        }
    }

    if (!has_printing_jobs) {
        *printer_mem_base = 0; // Set printer state to 0 (Invalid/Idle)
    }
    return 0;
}

// Function: cmd_recv_control_file
unsigned int cmd_recv_control_file(char *printer_base, char *command_line_args) {
    char *size_str = strsep(&command_line_args, " ");
    char *job_info_str = command_line_args;

    if (size_str == NULL || job_info_str == NULL) {
        return 0xffffffff;
    }

    ulong control_file_size = strtoul(size_str, NULL, 10);
    if (control_file_size >= 0x401) { // Max 1024 bytes
        return 0xffffffff;
    }

    if (strncmp(job_info_str, "cfA", 3) != 0) {
        return 0xffffffff;
    }

    char job_index_str[4] = {0};
    strncpy(job_index_str, job_info_str + 3, 3);
    ulong job_index = strtoul(job_index_str, NULL, 10);

    if (job_index >= MAX_JOBS) {
        return 0xffffffff;
    }

    char control_file_buffer[1024];
    if (read_n(0, control_file_buffer, control_file_size) < 1) {
        return 0xffffffff;
    }

    // Calculate base pointers for the specific job slot
    char *current_job_slot_base = printer_base + JOB_DATA_STRUCT_OFFSET + (job_index * JOB_SLOT_SIZE);
    int *job_data_fields = (int *)(current_job_slot_base); // Points to the start of the JobData struct within the slot

    // Initial setup for job slot (offsets 0x2FC and 0x110 relative to printer_base)
    *(unsigned int *)(current_job_slot_base + (JOB_TIME_FIELD_INT_OFFSET * sizeof(int))) = 0x84;
    *(unsigned int *)(current_job_slot_base + (JOB_TIMER_FIELD_INT_OFFSET * sizeof(int))) = 10;

    char *line_ptr = control_file_buffer;
    char *current_line;

    // Iterate through lines in the control file
    while ((current_line = strsep(&line_ptr, "\n")) != NULL && line_ptr != NULL) {
        size_t line_len = strlen(current_line);
        if (line_len == 0) continue;

        char command = current_line[0];
        char *arg = current_line + 1;
        size_t arg_len = line_len - 1;

        switch (command) {
            case 'C':
                if (arg_len < 32) {
                    strcpy((char *)(job_data_fields + JOB_NAME_FIELD_INT_OFFSET), arg);
                }
                break;
            case 'H':
                if (arg_len < 32) {
                    strcpy((char *)(job_data_fields + JOB_HOST_FIELD_INT_OFFSET), arg);
                }
                break;
            case 'I':
                job_data_fields[JOB_ID_INT_FIELD_INT_OFFSET] = strtoul(arg, NULL, 10);
                if ((uint)job_data_fields[JOB_TIME_FIELD_INT_OFFSET] <= (uint)job_data_fields[JOB_ID_INT_FIELD_INT_OFFSET]) {
                    job_data_fields[JOB_ID_INT_FIELD_INT_OFFSET] = 0;
                }
                break;
            case 'J':
                if (arg_len < 100) {
                    strcpy((char *)(job_data_fields + JOB_ID_STR_FIELD_INT_OFFSET), arg);
                }
                break;
            case 'L':
                if (arg_len < 32) {
                    strcpy((char *)(current_job_slot_base + (JOB_USER_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET)), arg);
                }
                *(char *)(current_job_slot_base + (JOB_USER_FLAG_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET)) = 1;
                break;
            case 'M':
                if (arg_len < 32) {
                    strcpy((char *)(current_job_slot_base + (JOB_MAIL_USER_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET)), arg);
                }
                *(char *)(current_job_slot_base + (JOB_MAIL_FLAG_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET)) = 1;
                break;
            case 'N':
                if (arg_len < 0x83) { // 131
                    strcpy((char *)(current_job_slot_base + (JOB_FILENAME_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET)), arg);
                }
                break;
            case 'P':
                if (arg_len < 32) {
                    strcpy((char *)(current_job_slot_base + (JOB_PRINTER_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET)), arg);
                }
                break;
            case 'S': {
                char *size_part1_str = strsep(&arg, " ");
                char *size_part2_str = arg;
                if (size_part1_str != NULL && size_part2_str != NULL &&
                    strlen(size_part1_str) > 1 && strlen(size_part2_str) > 1) {
                    job_data_fields[JOB_SIZE_LOW_FIELD_INT_OFFSET] = strtoul(size_part1_str + 1, NULL, 10);
                    job_data_fields[JOB_SIZE_HIGH_FIELD_INT_OFFSET] = strtoul(size_part2_str, NULL, 10);
                }
                break;
            }
            case 'T':
                strcpy((char *)(job_data_fields + JOB_TITLE_FIELD_INT_OFFSET), arg);
                break;
            default: {
                uint mapped_bit = 1 << ((command + 0xabU) & 0x1f);
                if ((mapped_bit & 0x2e86c000) == 0) {
                    if ((mapped_bit & 4) != 0) { // This corresponds to 'W' command (ASCII 87)
                        job_data_fields[JOB_TIME_FIELD_INT_OFFSET] = strtoul(arg, NULL, 10);
                    }
                } else { // If command maps to a bit in the mask
                    *(char *)(job_data_fields + JOB_CLASS_FIELD_INT_OFFSET) = command;
                }
            }
        }
    }

    // Final checks and status update
    if (strlen((char *)(job_data_fields + JOB_HOST_FIELD_INT_OFFSET)) == 0 ||
        strlen((char *)(current_job_slot_base + (JOB_PRINTER_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET))) == 0) {
        return 0xffffffff;
    }

    if (job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] == 0) {
        job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] = 1; // Set to Waiting
    } else if (job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] == 1) {
        job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] = 2; // Set to Queued
    } else {
        return 0xffffffff; // Invalid state transition
    }

    return 0;
}

// Function: cmd_recv_data_file
unsigned int cmd_recv_data_file(char *printer_base, char *command_line_args) {
    char *size_str = strsep(&command_line_args, " ");
    char *job_info_str = command_line_args;

    if (size_str == NULL || job_info_str == NULL) {
        return 0xffffffff;
    }

    ulong data_file_size = strtoul(size_str, NULL, 10);

    if (strncmp(job_info_str, "dfA", 3) != 0) {
        return 0xffffffff;
    }

    char job_index_str[4] = {0};
    strncpy(job_index_str, job_info_str + 3, 3);
    ulong job_index = strtoul(job_index_str, NULL, 10);

    if (job_index >= MAX_JOBS) {
        return 0xffffffff;
    }

    // Allocate memory for data file content + null terminator
    void *data_buffer = malloc(data_file_size + 1);
    if (data_buffer == NULL) {
        return 0xffffffff;
    }
    memset(data_buffer, 0, data_file_size + 1); // Ensure null termination and clear buffer

    if (read_n(0, data_buffer, data_file_size) < 1 ||
        *(char *)((char *)data_buffer + data_file_size) != '\0') {
        free(data_buffer);
        return 0xffffffff;
    }

    // Calculate base pointers for the specific job slot
    char *current_job_slot_base = printer_base + JOB_DATA_STRUCT_OFFSET + (job_index * JOB_SLOT_SIZE);
    int *job_data_fields = (int *)(current_job_slot_base); // Points to the start of the JobData struct within the slot
    void **job_data_ptr_field = (void **)(job_data_fields + JOB_DATA_PTR_FIELD_INT_OFFSET);

    // Free existing data if any
    if (*job_data_ptr_field != NULL) {
        free(*job_data_ptr_field);
    }

    // Store new data pointer and size
    *job_data_ptr_field = data_buffer;
    job_data_fields[JOB_DATA_SIZE_FIELD_INT_OFFSET] = data_file_size;

    // Update job status
    if (job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] == 0) {
        job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] = 1; // Set to Waiting
    } else if (job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] == 1) {
        job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] = 2; // Set to Queued
    } else {
        return 0xffffffff; // Invalid state transition
    }

    return 0;
}

// Function: cmd_send_queue_state
unsigned int cmd_send_queue_state(char *printer_base) {
    // printer_base is assumed to be the base pointer to the overall printer memory block.
    // printer_name is at offset 4 bytes.
    // job_queue starts at offset 0x104 bytes.

    char *printer_name = printer_base + 4;
    char *job_queue_base = printer_base + JOB_DATA_STRUCT_OFFSET;

    printf("Queue State for %s\n\n", printer_name);
    printf("Owner       Status      Jobname             Job-ID      Size  \n");
    printf("==============================================================\n");

    for (int i = 0; i < MAX_JOBS; ++i) {
        char *current_job_slot_base = job_queue_base + (i * JOB_SLOT_SIZE);
        int *job_data_fields = (int *)(current_job_slot_base); // Points to the start of the JobData struct within the slot

        if (job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] != 0) { // If job is not Invalid
            // Owner
            char *owner_str = (char *)(current_job_slot_base + (JOB_PRINTER_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET));
            printf("%s", owner_str);
            print_ws(12 - strlen(owner_str));

            // Status
            char *status_str = jstate_str(job_data_fields[JOB_STATUS_FIELD_INT_OFFSET]);
            printf("%s", status_str);
            print_ws(12 - strlen(status_str));

            // Jobname (using Filename field for Jobname display)
            char *jobname_str = (char *)(current_job_slot_base + (JOB_FILENAME_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET));
            printf("%s", jobname_str);
            print_ws(20 - strlen(jobname_str));

            // Job-ID
            char job_id_buffer[4]; // "%03d" format -> max 3 digits + null
            sprintf(job_id_buffer, "%03d", i);
            printf("%s", job_id_buffer);
            print_ws(12 - strlen(job_id_buffer));

            // Size
            printf("%d\n", job_data_fields[JOB_DATA_SIZE_FIELD_INT_OFFSET]);
        }
    }
    printf("\n\n");
    return 0;
}

// Function: cmd_remove_jobs
unsigned int cmd_remove_jobs(char *printer_base, char *owner_name, int job_idx) {
    // printer_base is assumed to be the base pointer to the overall printer memory block.
    // job_queue starts at offset 0x104 bytes.

    char *job_queue_base = printer_base + JOB_DATA_STRUCT_OFFSET;
    char *current_job_slot_base = job_queue_base + (job_idx * JOB_SLOT_SIZE);
    int *job_data_fields = (int *)(current_job_slot_base); // Points to the start of the JobData struct within the slot
    void **job_data_ptr_field = (void **)(job_data_fields + JOB_DATA_PTR_FIELD_INT_OFFSET);
    char *job_owner_field = (char *)(current_job_slot_base + (JOB_PRINTER_FIELD_BYTE_OFFSET - JOB_DATA_STRUCT_OFFSET));

    if (job_data_fields[JOB_STATUS_FIELD_INT_OFFSET] == 0) {
        return 0xffffffff; // Job is already invalid
    }

    // Compare owner with owner_name or "root"
    if (strcmp(job_owner_field, owner_name) != 0 &&
        strcmp(owner_name, "root") != 0) {
        return 0xffffffff; // Mismatch owner and not root
    }

    if (*job_data_ptr_field != NULL) {
        free(*job_data_ptr_field);
        *job_data_ptr_field = NULL; // Clear pointer after freeing
    }

    // Clear the entire job slot
    memset(current_job_slot_base, 0, JOB_SLOT_SIZE);

    return 0;
}

// Function: printer_tick
void printer_tick(int *printer_mem_base) {
    // printer_mem_base is assumed to be the base pointer to the overall printer state
    // *printer_mem_base is the main printer state

    if (*printer_mem_base == 0) { // If printer is idle, nothing to do
        return;
    }

    int active_printing_jobs_timer_sum = 0;

    for (int i = 0; i < MAX_JOBS; ++i) {
        char *job_slot_base = (char *)printer_mem_base + JOB_DATA_STRUCT_OFFSET + (i * JOB_SLOT_SIZE);
        int *job_status_ptr = (int *)(job_slot_base + (JOB_STATUS_FIELD_INT_OFFSET * sizeof(int)));
        int *job_timer_ptr = (int *)(job_slot_base + (JOB_TIMER_FIELD_INT_OFFSET * sizeof(int)));
        void **job_data_ptr_field = (void **)(job_slot_base + (JOB_DATA_PTR_FIELD_INT_OFFSET * sizeof(int)));

        if (*job_status_ptr == 3) { // If Printing
            (*job_timer_ptr)--;
            active_printing_jobs_timer_sum += *job_timer_ptr; // Sum remaining timer values

            if (*job_timer_ptr == 0) { // Job finished printing
                if (*job_data_ptr_field != NULL) {
                    free(*job_data_ptr_field);
                    *job_data_ptr_field = NULL; // Clear pointer after freeing
                }
                // Clear the entire job slot
                memset(job_slot_base, 0, JOB_SLOT_SIZE);
            }
        }
    }

    if (active_printing_jobs_timer_sum == 0) {
        *printer_mem_base = 0; // Set printer state to 0 (Idle) if no jobs are printing or have timers left
    }
}