#define _GNU_SOURCE // Required for strsep and vdprintf on some systems

#include <stdio.h>    // For vdprintf, fprintf, printf, perror, stdin, stdout
#include <string.h>   // For strlen, memcmp, strcpy, strsep, memset
#include <stdlib.h>   // For calloc
#include <unistd.h>   // For read, STDIN_FILENO
#include <stdarg.h>   // For va_list in vdprintf

// --- Global Variables ---
char g_user_resp[1024]; // Buffer for user input, size 0x400
char g_page_idx = 0;
char g_last_page_completed = -1; // Initialize to -1, as page 0 is the first completed.

// --- String literals (DAT_ variables) ---
const char DAT_000152d3[] = "prev";
const char DAT_000152d8[] = "next";
const char DAT_000152e5[] = "help";
const char DAT_000152ea[] = "exit";
const char DAT_000153e8[] = "Welcome to the Sea Eye Association Application\n";
const char DAT_000155a0[] = "\n*********%s*********\n";
const char DAT_0001500a[] = "N";

// --- Custom function definitions ---

// Emulate original fdprintf behavior (writing to file descriptor)
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vdprintf(fd, format, args);
    va_end(args);
    return ret;
}

// Emulate the custom receive function behavior
// Returns 0 on success (bytes_read_out set to bytes read), -1 on error.
// Original implies receive returns 0 for a successful single byte read,
// and bytes_read_out is set to 1.
int custom_receive(int fd, char *buf, size_t count, int *bytes_read_out) {
    ssize_t bytes_read = read(fd, buf, count);
    if (bytes_read > 0) {
        *bytes_read_out = (int)bytes_read;
        return 0; // Success
    } else if (bytes_read == 0) {
        *bytes_read_out = 0;
        return 0; // EOF
    }
    *bytes_read_out = -1; // Error
    return -1;
}

// --- FormField structure and validator ---
// Function pointer for field validation
typedef int (*ValidatorFunc)(const char*, int);

typedef struct {
    const char *field_name;
    const char *field_prompt;
    char validation_type;          // e.g., '\0' for specific 'N' check, or other for custom validation
    int max_len;                   // Max length of input buffer (including null terminator)
    char *response_buffer;         // Pointer to allocated buffer for user response
    ValidatorFunc validator;       // Function to validate input
} FormField;

// Default validator: always accepts input if not empty and within max_len
int default_validator(const char *input, int max_len) {
    if (input == NULL || strlen(input) == 0) {
        return -1; // Bad input (empty)
    }
    if (strlen(input) >= max_len) {
        return -1; // Input too long
    }
    return 0; // Valid
}

// Example validator for numbers
int number_validator(const char *input, int max_len) {
    if (input == NULL || strlen(input) == 0) return -1;
    if (strlen(input) >= max_len) return -1;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] < '0' || input[i] > '9') {
            return -1; // Not a digit
        }
    }
    return 0; // Valid number
}

// Example validator for email (very basic)
int email_validator(const char *input, int max_len) {
    if (input == NULL || strlen(input) == 0) return -1;
    if (strlen(input) >= max_len) return -1;
    if (strchr(input, '@') == NULL || strchr(input, '.') == NULL) {
        return -1; // Missing @ or .
    }
    return 0; // Basic email valid
}

// --- FormField Arrays ---
FormField applicant_id[] = {
    {"First Name", "Enter your first name", '\0', 64, NULL, default_validator},
    {"Last Name", "Enter your last name", '\0', 64, NULL, default_validator},
    {"Middle Initial", "Enter your middle initial (optional, leave blank if none)", '\0', 2, NULL, default_validator},
    {"Date of Birth", "Enter your date of birth (MM/DD/YYYY)", '\0', 11, NULL, default_validator},
    {"SSN", "Enter your Social Security Number (XXX-XX-XXXX)", '\0', 12, NULL, default_validator},
    {"Gender", "Enter your gender (M/F/O)", '\0', 2, NULL, default_validator},
    {"Race", "Enter your race/ethnicity", '\0', 32, NULL, default_validator},
    {"Nationality", "Enter your nationality", '\0', 32, NULL, default_validator},
    {"Veteran Status", "Are you a veteran? (Y/N)", '\0', 2, NULL, default_validator},
    {"Disability Status", "Do you have a disability? (Y/N)", '\0', 2, NULL, default_validator},
    {"Marital Status", "Enter your marital status", '\0', 16, NULL, default_validator},
    {"Dependents", "Number of dependents", '\0', 4, NULL, number_validator},
    {"ID Type", "Type of ID (e.g., Driver's License)", '\0', 32, NULL, default_validator}
};
const int NUM_APPLICANT_ID_FIELDS = sizeof(applicant_id) / sizeof(FormField);

FormField contact_info[] = {
    {"Email", "Enter your email address", '\0', 128, NULL, email_validator},
    {"Phone", "Enter your phone number (XXX-XXX-XXXX)", '\0', 16, NULL, default_validator},
    {"Emergency Contact Name", "Enter emergency contact name", '\0', 64, NULL, default_validator},
    {"Emergency Contact Phone", "Enter emergency contact phone", '\0', 16, NULL, default_validator}
};
const int NUM_CONTACT_INFO_FIELDS = sizeof(contact_info) / sizeof(FormField);

FormField current_address[] = {
    {"Street Address 1", "Enter street address line 1", '\0', 128, NULL, default_validator},
    {"Street Address 2", "Enter street address line 2 (optional)", '\0', 128, NULL, default_validator},
    {"City", "Enter city", '\0', 64, NULL, default_validator},
    {"State", "Enter state (2-letter abbreviation)", '\0', 3, NULL, default_validator},
    {"Zip Code", "Enter zip code", '\0', 11, NULL, default_validator},
    {"Country", "Enter country", '\0', 32, NULL, default_validator},
    {"Time at Address", "Years at current address", '\0', 4, NULL, number_validator}
};
const int NUM_CURRENT_ADDRESS_FIELDS = sizeof(current_address) / sizeof(FormField);

FormField highest_education[] = {
    {"Degree", "Highest degree obtained", '\0', 64, NULL, default_validator},
    {"Major", "Major/Field of Study", '\0', 64, NULL, default_validator},
    {"Institution", "Name of institution", '\0', 128, NULL, default_validator},
    {"Graduation Year", "Year of graduation", '\0', 5, NULL, number_validator},
    {"GPA", "GPA (optional)", '\0', 5, NULL, default_validator},
    {"Certifications", "Any relevant certifications", '\0', 128, NULL, default_validator},
    {"Skills", "Key skills relevant to position", '\0', 256, NULL, default_validator},
    {"Languages", "Languages spoken", '\0', 128, NULL, default_validator},
    {"Awards", "Awards or honors received", '\0', 128, NULL, default_validator},
    {"Publications", "Publications (if any)", '\0', 256, NULL, default_validator},
    {"Research Experience", "Briefly describe research experience", '\0', 256, NULL, default_validator},
    {"Thesis Title", "Thesis title (if applicable)", '\0', 256, NULL, default_validator}
};
const int NUM_HIGHEST_EDUCATION_FIELDS = sizeof(highest_education) / sizeof(FormField);

FormField last_employer[] = {
    {"Company Name", "Most recent employer's name", '\0', 128, NULL, default_validator},
    {"Job Title", "Your job title", '\0', 64, NULL, default_validator},
    {"Start Date", "Employment start date (MM/YYYY)", '\0', 8, NULL, default_validator},
    {"End Date", "Employment end date (MM/YYYY) or 'Present'", '\0', 8, NULL, default_validator},
    {"Responsibilities", "Key responsibilities", '\0', 512, NULL, default_validator},
    {"Salary", "Annual salary (optional)", '\0', 16, NULL, default_validator},
    {"Reason for Leaving", "Reason for leaving", '\0', 256, NULL, default_validator},
    {"Supervisor Name", "Supervisor's name", '\0', 64, NULL, default_validator},
    {"Supervisor Phone", "Supervisor's phone number", '\0', 16, NULL, default_validator},
    {"Can Contact", "Can we contact this employer? (Y/N)", '\0', 2, NULL, default_validator},
    {"Previous Employer 1", "Previous employer's name", '\0', 128, NULL, default_validator},
    {"Previous Job Title 1", "Previous job title", '\0', 64, NULL, default_validator},
    {"Previous Employer 2", "Second previous employer's name", '\0', 128, NULL, default_validator},
    {"Previous Job Title 2", "Second previous job title", '\0', 64, NULL, default_validator}
};
const int NUM_LAST_EMPLOYER_FIELDS = sizeof(last_employer) / sizeof(FormField);

FormField screening_questions[] = {
    {"Criminal Record", "Do you have a criminal record? (Y/N)", '\0', 2, NULL, default_validator},
    {"Security Clearance", "Do you have any security clearances? (Y/N)", '\0', 2, NULL, default_validator}
};
const int NUM_SCREENING_QUESTIONS_FIELDS = sizeof(screening_questions) / sizeof(FormField);


// --- Function Prototypes (for job_application_page array) ---
int candidate_info(void);
int contact(void);
int address(void);
int education(void);
int employment_history(void);
int final_screening(void);
int finished(void);

// --- Function Pointers for application pages ---
typedef int (*PageFunc)(void);
PageFunc job_application_page[] = {
    candidate_info,
    contact,
    address,
    education,
    employment_history,
    final_screening,
    finished
};
const int NUM_APPLICATION_PAGES = sizeof(job_application_page) / sizeof(PageFunc);


// --- get_response function ---
// Returns -1 for buffer full/no newline, -2 for read error,
// 1 for generic input, 2-6 for special commands.
int get_response(void) {
    int bytes_read_count = 0;
    char *current_pos = g_user_resp;
    int len = 0;
    const int max_response_len = sizeof(g_user_resp);

    // Read input character by character until newline, buffer full, or read error
    while (len < max_response_len - 1) { // Leave space for null terminator
        int read_status = custom_receive(STDIN_FILENO, current_pos, 1, &bytes_read_count);
        if (read_status != 0) { // Error reading
            return -2; // 0xfffffffe
        }
        if (bytes_read_count == 0) { // EOF, no more input
            break;
        }
        if (*current_pos == '\n') {
            break;
        }
        current_pos++;
        len++;
    }

    if (len == max_response_len - 1 && *current_pos != '\n') {
        // Buffer full and no newline found
        // Clear remaining input from buffer to avoid issues with next read
        char discard_char;
        while (read(STDIN_FILENO, &discard_char, 1) > 0 && discard_char != '\n');
        return -1; // 0xffffffff
    }
    
    // Null-terminate the string
    *current_pos = '\0';

    // Check for special commands
    if (len > 2 && g_user_resp[0] == '*' && g_user_resp[1] == '*') {
        if (memcmp(g_user_resp + 2, DAT_000152d3, strlen(DAT_000152d3)) == 0) { // "**prev"
            return 2;
        }
        if (memcmp(g_user_resp + 2, DAT_000152d8, strlen(DAT_000152d8)) == 0) { // "**next"
            return 3;
        }
        if (memcmp(g_user_resp + 2, "update ", strlen("update ")) == 0) { // "**update "
            return 4;
        }
        if (memcmp(g_user_resp + 2, DAT_000152e5, strlen(DAT_000152e5)) == 0) { // "**help"
            return 5;
        }
        if (memcmp(g_user_resp + 2, DAT_000152ea, strlen(DAT_000152ea)) == 0) { // "**exit"
            return 6;
        }
    }
    
    return 1; // Generic input
}


// --- print_menu function ---
void print_menu(void) {
  fdprintf(1,"All commands begin with \'**\' and may be entered at any time\n");
  fdprintf(1,"**prev <Return to the previous page>\n");
  fdprintf(1,"**next <Move to the next page>\n");
  fdprintf(1,"**update [id] <Update field, ex: \"Update First Name\">\n");
  fdprintf(1,"**help <Print this dialogue>\n");
  fdprintf(1,"**exit <Exit application>\n");
  return;
}

// --- fill_out_form function ---
int fill_out_form(FormField *fields_array, int num_fields) {
    for (int i = 0; i < num_fields; i++) {
        FormField *field = &fields_array[i];
        while (field->response_buffer == NULL) { // Loop until this field is filled
            fdprintf(1,"%s%s: ", field->field_name, field->field_prompt);
            int response_code = get_response();

            if (response_code != 1) { // Not generic input, could be a command or error
                return response_code;
            }

            // Check for specific 'N' response if validation_type is '\0'
            if (field->validation_type == '\0' && memcmp(g_user_resp, DAT_0001500a, 1) == 0 && strlen(g_user_resp) == 1) {
                field->response_buffer = calloc(1, field->max_len); // Allocate and zero-fill
                if (!field->response_buffer) {
                    perror("calloc failed");
                    return -1;
                }
                break; // Move to next field
            }

            // Validate input using the field's validator function
            if (field->validator) {
                int validation_result = field->validator(g_user_resp, field->max_len);
                if (validation_result == 0) { // Validation successful
                    field->response_buffer = calloc(1, field->max_len);
                    if (!field->response_buffer) {
                        perror("calloc failed");
                        return -1;
                    }
                    strcpy(field->response_buffer, g_user_resp);
                    break; // Move to next field
                } else {
                    fdprintf(1, "Invalid input for %s. Please try again.\n", field->field_name);
                }
            } else { // No specific validator, use default logic (if not 'N')
                if (strlen(g_user_resp) < field->max_len) {
                     field->response_buffer = calloc(1, field->max_len);
                    if (!field->response_buffer) {
                        perror("calloc failed");
                        return -1;
                    }
                    strcpy(field->response_buffer, g_user_resp);
                    break; // Move to next field
                } else {
                    fdprintf(1, "Input too long for %s (max %d chars). Please try again.\n", field->field_name, field->max_len -1);
                }
            }
        }
    }
    return 0; // All fields processed
}

// --- update_field function ---
int update_field(char *field_name_to_find, FormField *fields_array, int num_fields) {
    if (field_name_to_find == NULL || strlen(field_name_to_find) == 0) {
        fdprintf(1,"Bad field\n");
        return -1;
    }

    for (int i = 0; i < num_fields; i++) {
        FormField *field = &fields_array[i];
        if (strlen(field_name_to_find) == strlen(field->field_name) &&
            memcmp(field_name_to_find, field->field_name, strlen(field->field_name)) == 0) {
            
            fdprintf(1,"%s%s: ", field->field_name, field->field_prompt);
            int response_code = get_response();

            if (response_code != 1) { // Not generic input
                fdprintf(1,"Bad command\n");
                return -2;
            }

            // Specific 'N' check
            if (field->validation_type == '\0' && memcmp(g_user_resp, DAT_0001500a, 1) == 0 && strlen(g_user_resp) == 1) {
                if (field->response_buffer) {
                    memset(field->response_buffer, 0, field->max_len); // Clear existing data
                } else {
                    field->response_buffer = calloc(1, field->max_len); // Allocate if not already
                    if (!field->response_buffer) {
                        perror("calloc failed");
                        return -1;
                    }
                }
                return 0; // Successfully updated to 'N' (empty)
            }

            // Validate and update
            if (field->validator) {
                int validation_result = field->validator(g_user_resp, field->max_len);
                if (validation_result != 0) {
                    fdprintf(1,"Bad input.\n");
                    return -3;
                }
            } else { // No specific validator, default check
                if (strlen(g_user_resp) >= field->max_len) {
                    fdprintf(1,"Input too long.\n");
                    return -3;
                }
            }

            // If validation passed, update the field
            if (field->response_buffer == NULL) {
                field->response_buffer = calloc(1, field->max_len);
                if (!field->response_buffer) {
                    perror("calloc failed");
                    return -1;
                }
            } else {
                memset(field->response_buffer, 0, field->max_len); // Clear old data
            }
            strcpy(field->response_buffer, g_user_resp);
            return 0; // Successful update
        }
    }

    fdprintf(1,"Could not find specified field\n");
    return -1;
}

// --- print_page function ---
void print_page(const char *page_title, FormField *fields_array, int num_fields) {
    if (page_title != NULL) {
        fdprintf(1, DAT_000155a0, page_title);
    }
    for (int i = 0; i < num_fields; i++) {
        FormField *field = &fields_array[i];
        fdprintf(1,"%s=%s\n", field->field_name, field->response_buffer ? field->response_buffer : "[NOT SET]");
    }
    return;
}

// --- form function ---
// Returns 0 for success, non-zero for special commands or error.
int form(const char *page_title, FormField *fields_array, unsigned int num_fields) {
    int result = 0;

    // Check if previous page is completed
    if (g_last_page_completed + 1 < g_page_idx) {
        fdprintf(1,"You must complete the previous page before proceeding to this page\n");
        g_page_idx = g_last_page_completed + 1;
        return -1; // Indicate error/needs to re-evaluate
    }

    int incomplete_fields_found = 0;
    for (unsigned int i = 0; i < num_fields; i++) {
        if (fields_array[i].response_buffer == NULL) {
            incomplete_fields_found = 1;
            g_last_page_completed = g_page_idx - 1; // Mark current page as incomplete
            break;
        }
    }

    // If current page is the next uncompleted page, or there are incomplete fields
    if ((g_page_idx == g_last_page_completed + 1) || (incomplete_fields_found != 0)) {
        result = fill_out_form(fields_array, num_fields);
        if (result != 0) {
            if (result == 4) { // Update command received
                fdprintf(1,"Cannot update field until all fields are inputted\n");
                return result;
            }
            return result; // Other commands or error from fill_out_form
        }
        g_last_page_completed = g_page_idx; // Mark current page as completed
        result = 0;
    }

    print_page(page_title, fields_array, num_fields);
    fdprintf(1,"\nType **next to continue\n");
    
    result = get_response();
    if (result == 4) { // Update command
        char *user_input_ptr = g_user_resp; // strsep modifies the pointer
        strsep(&user_input_ptr, " "); // Consume "update"
        char *field_name_token = user_input_ptr; // Remaining part is the field name
        
        int update_result = update_field(field_name_token, fields_array, num_fields);
        if (update_result != 0) {
            fdprintf(1,"Update Unsuccessful\n");
        }
    }
    return result;
}

// --- Page functions ---
int candidate_info(void) {
  fdprintf(1,"\nCandidate Info Form\n");
  return form("\n*********Candidate Info:*********\n", applicant_id, NUM_APPLICANT_ID_FIELDS);
}

int contact(void) {
  fdprintf(1,"\nContact Info Form\n");
  return form("\n*********Contact Info:*********\n", contact_info, NUM_CONTACT_INFO_FIELDS);
}

int address(void) {
  fdprintf(1,"\nAddress Form\n");
  return form("\n*********Address:*********\n", current_address, NUM_CURRENT_ADDRESS_FIELDS);
}

int education(void) {
  fdprintf(1,"\nEducation Form\n");
  return form("\n*********Highest Education:*********\n", highest_education, NUM_HIGHEST_EDUCATION_FIELDS);
}

int employment_history(void) {
  fdprintf(1,"\nEmployment Form\n");
  return form("\n*********Most Recent Employer:*********\n", last_employer, NUM_LAST_EMPLOYER_FIELDS);
}

int final_screening(void) {
  fdprintf(1,"\nFinal Questions\n");
  return form("\n*********Final Screening:*********\n", screening_questions, NUM_SCREENING_QUESTIONS_FIELDS);
}

int finished(void) {
    fdprintf(1,"\n\nYou have completed your application with the Sea Eye Association.\n");
    fdprintf(1,"You may review the form. Navigate through the application with **prev and **next.\n");
    fdprintf(1,"Once your are satisfied type **exit to exit and submit the form\n");
    fdprintf(1,"If you wish to discard your application, please use Control-C\n");

    if (g_last_page_completed + 1 < g_page_idx) {
        fdprintf(1,"You must complete the previous page before proceeding to this page\n");
        g_page_idx = g_last_page_completed + 1;
        return -1;
    }
    return get_response();
}

// --- main function ---
int main(void) {
    int command_result = 0;
    g_page_idx = 0; // Start at the first page

    fdprintf(1, DAT_000153e8); // Welcome message
    fdprintf(1,"Thanks for your interest in the Sea Eye Association.\n");
    fdprintf(1,"In order to be considered for the job complete the preliminary online background check\n");
    fdprintf(1,"Due to the secure nature of the position you are applying for you may be asked to\n");
    fdprintf(1,"submit additional paperwork after your preliminary background check has been approved.\n");
    fdprintf(1,"Thank you for your cooperation\n");

    while (command_result != 6) { // 6 is 'exit' command
        if (g_page_idx < 0 || g_page_idx >= NUM_APPLICATION_PAGES) {
            fdprintf(1, "Error: Invalid page index. Resetting to first page.\n");
            g_page_idx = 0;
            command_result = 0; // Reset command result to avoid immediate exit
            continue;
        }

        command_result = job_application_page[g_page_idx]();
        
        if (command_result == 5) { // 'help' command
            print_menu();
        } else if (command_result < 6) { // Not 'exit' or error code
            if (command_result == 2) { // 'prev' command
                if (g_page_idx > 0) {
                    g_page_idx--;
                }
            } else if (command_result == 3) { // 'next' command
                if (g_page_idx < NUM_APPLICATION_PAGES - 1) {
                    g_page_idx++;
                }
            }
            // If command_result is 1 (generic input), 4 (update), or a negative error code,
            // the page index does not change, and the loop will re-evaluate the current page.
        }
    }

    fdprintf(1,"Thank you!\n");
    return 0;
}