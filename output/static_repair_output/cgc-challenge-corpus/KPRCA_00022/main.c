#include <stdio.h>    // For fprintf, vfprintf, stdout, stderr, perror
#include <stdlib.h>   // For calloc, exit, atoi
#include <string.h>   // For strlen, memcmp, strcpy, memset, strsep, strchr
#include <unistd.h>   // For read, STDIN_FILENO
#include <stdarg.h>   // For va_list, va_start, va_end

// Assume these are global variables as implied by the snippet
char g_user_resp[1024]; // Buffer for user input (0x400 = 1024 bytes)
char g_page_idx = 0;    // Current page index
int g_last_page_completed = -1; // Index of the last successfully completed page

// Define the structure for form fields
// The original code implies 0x18 (24 bytes) per field, suggesting 6 pointers or equivalent.
typedef struct {
    const char* name;           // Field name (e.g., "First Name")
    const char* prompt;         // Prompt for user input (e.g., "Enter your first name")
    char type_char;             // A flag character, e.g., '\0' for specific behavior (empty input allowed)
    size_t max_len;             // Maximum length of the input string (including null terminator)
    char* value_ptr;            // Pointer to allocated memory for the field's value
    int (*validator)(const char* input, size_t max_len); // Validator function for the input
} FormField;

// --- Helper Functions and Data ---

// Placeholder validator function (for fields that don't need specific validation)
int default_validator(const char* input, size_t max_len) {
    return strlen(input) < max_len ? 0 : -1; // 0 for success, -1 for failure
}

// Dummy data for DAT_0001500a (used in memcmp with 1 byte, likely a null character)
const char DAT_0001500a = '\0';

// Strings for commands (replacing DAT_xxxxxx)
const char* CMD_PREV = "prev";
const char* CMD_NEXT = "next";
const char* CMD_UPDATE = "update "; // Note the space for parsing argument
const char* CMD_HELP = "help";
const char* CMD_EXIT = "exit";

// Global string literals (replacing DAT_xxxxxx)
const char* GREETING_MSG = "Welcome to the application!\n"; // DAT_000153e8

// Helper for fdprintf (assuming it means fprintf to stdout/stderr)
int fdprintf(int fd, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = 0;
    if (fd == 1) { // stdout
        ret = vfprintf(stdout, format, args);
    } else if (fd == 2) { // stderr
        ret = vfprintf(stderr, format, args);
    } else {
        // Default to stdout for other FDs, or handle as an error
        ret = vfprintf(stdout, format, args);
    }
    va_end(args);
    return ret;
}

// --- Validator Functions (Examples) ---
int validate_name(const char* input, size_t max_len) {
    if (strlen(input) == 0 || strlen(input) >= max_len) return -1;
    // Basic check: only letters and spaces
    for (size_t i = 0; i < strlen(input); ++i) {
        if (!((input[i] >= 'a' && input[i] <= 'z') || (input[i] >= 'A' && input[i] <= 'Z') || input[i] == ' ')) {
            return -1;
        }
    }
    return 0;
}

int validate_email(const char* input, size_t max_len) {
    if (strlen(input) == 0 || strlen(input) >= max_len) return -1;
    if (!strchr(input, '@') || !strchr(input, '.')) return -1; // Simple email check
    return 0;
}

int validate_age(const char* input, size_t max_len) {
    if (strlen(input) == 0 || strlen(input) >= max_len) return -1;
    for (size_t i = 0; i < strlen(input); ++i) {
        if (input[i] < '0' || input[i] > '9') return -1;
    }
    int age = atoi(input);
    if (age < 18 || age > 100) return -1; // Example age range
    return 0;
}

// --- Form Field Data ---
// These arrays hold the definitions for each form page's fields.
// The counts (13, 4, 7, 12, 14, 2) are derived from the original snippet.

FormField applicant_id_fields[] = {
    {"First Name", "Enter your first name", '\0', 64, NULL, validate_name},
    {"Last Name", "Enter your last name", '\0', 64, NULL, validate_name},
    {"Middle Initial", "Enter your middle initial (optional)", '?', 2, NULL, default_validator},
    {"Date of Birth", "Enter your date of birth (MM/DD/YYYY)", '\0', 11, NULL, default_validator},
    {"Social Security Number", "Enter your SSN (XXX-XX-XXXX)", '\0', 12, NULL, default_validator},
    {"Age", "Enter your age", '\0', 4, NULL, validate_age},
    {"Gender", "Enter your gender (M/F/Other)", '\0', 10, NULL, default_validator},
    {"Nationality", "Enter your nationality", '\0', 64, NULL, default_validator},
    {"Marital Status", "Enter your marital status", '\0', 32, NULL, default_validator},
    {"Dependents", "Number of dependents", '\0', 4, NULL, default_validator},
    {"Phone Number", "Enter your phone number", '\0', 16, NULL, default_validator},
    {"Email Address", "Enter your email address", '\0', 128, NULL, validate_email},
    {"Preferred Contact", "Preferred contact method", '\0', 32, NULL, default_validator}
};
const int NUM_APPLICANT_ID_FIELDS = sizeof(applicant_id_fields) / sizeof(FormField);

FormField contact_info_fields[] = {
    {"Emergency Contact Name", "Emergency contact full name", '\0', 128, NULL, validate_name},
    {"Emergency Contact Relation", "Emergency contact relationship", '\0', 64, NULL, default_validator},
    {"Emergency Contact Phone", "Emergency contact phone number", '\0', 16, NULL, default_validator},
    {"Emergency Contact Email", "Emergency contact email", '\0', 128, NULL, validate_email}
};
const int NUM_CONTACT_INFO_FIELDS = sizeof(contact_info_fields) / sizeof(FormField);

FormField current_address_fields[] = {
    {"Street Address 1", "Street address line 1", '\0', 128, NULL, default_validator},
    {"Street Address 2", "Street address line 2 (optional)", '?', 128, NULL, default_validator},
    {"City", "City", '\0', 64, NULL, default_validator},
    {"State/Province", "State/Province", '\0', 64, NULL, default_validator},
    {"Zip/Postal Code", "Zip/Postal Code", '\0', 16, NULL, default_validator},
    {"Country", "Country", '\0', 64, NULL, default_validator},
    {"Years at Address", "Years at current address", '\0', 4, NULL, default_validator}
};
const int NUM_CURRENT_ADDRESS_FIELDS = sizeof(current_address_fields) / sizeof(FormField);

FormField highest_education_fields[] = {
    {"Degree Type", "Highest degree obtained", '\0', 64, NULL, default_validator},
    {"Major/Field", "Major or field of study", '\0', 128, NULL, default_validator},
    {"Institution Name", "Name of institution", '\0', 128, NULL, default_validator},
    {"Graduation Year", "Year of graduation", '\0', 8, NULL, default_validator},
    {"GPA", "GPA (optional)", '?', 8, NULL, default_validator},
    {"Certifications", "Relevant certifications (comma-separated)", '?', 256, NULL, default_validator},
    {"Skills", "Key skills (comma-separated)", '?', 512, NULL, default_validator},
    {"Languages", "Languages spoken (comma-separated)", '?', 256, NULL, default_validator},
    {"Awards", "Awards/Honors (optional)", '?', 256, NULL, default_validator},
    {"Thesis Title", "Thesis title (if applicable)", '?', 256, NULL, default_validator},
    {"Research Interests", "Research interests (optional)", '?', 512, NULL, default_validator},
    {"Publications", "Publications (optional)", '?', 512, NULL, default_validator}
};
const int NUM_HIGHEST_EDUCATION_FIELDS = sizeof(highest_education_fields) / sizeof(FormField);

FormField last_employer_fields[] = {
    {"Company Name", "Most recent employer name", '\0', 128, NULL, default_validator},
    {"Job Title", "Your job title", '\0', 128, NULL, default_validator},
    {"Start Date", "Employment start date (MM/YYYY)", '\0', 8, NULL, default_validator},
    {"End Date", "Employment end date (MM/YYYY) or 'Present'", '\0', 8, NULL, default_validator},
    {"Supervisor Name", "Supervisor's name", '\0', 128, NULL, default_validator},
    {"Supervisor Contact", "Supervisor's contact information", '\0', 128, NULL, default_validator},
    {"Reason for Leaving", "Reason for leaving", '\0', 256, NULL, default_validator},
    {"Responsibilities", "Key responsibilities", '\0', 512, NULL, default_validator},
    {"Achievements", "Major achievements", '\0', 512, NULL, default_validator},
    {"Salary", "Final salary (optional)", '?', 32, NULL, default_validator},
    {"References", "References provided (optional)", '?', 256, NULL, default_validator},
    {"Industry", "Industry of employer", '\0', 64, NULL, default_validator},
    {"Employment Type", "Employment type (full-time, part-time, etc.)", '\0', 32, NULL, default_validator},
    {"Location", "Employer location (City, State)", '\0', 128, NULL, default_validator}
};
const int NUM_LAST_EMPLOYER_FIELDS = sizeof(last_employer_fields) / sizeof(FormField);

FormField screening_questions_fields[] = {
    {"Criminal Record", "Do you have any criminal record? (Yes/No)", '\0', 4, NULL, default_validator},
    {"Security Clearance", "Do you hold any security clearance? (Yes/No)", '\0', 4, NULL, default_validator}
};
const int NUM_SCREENING_QUESTIONS_FIELDS = sizeof(screening_questions_fields) / sizeof(FormField);


// Pointers to the actual form field arrays, as used in the original snippet's `form` calls
FormField* applicant_id = applicant_id_fields;
FormField* contact_info = contact_info_fields;
FormField* current_address = current_address_fields;
FormField* highest_education = highest_education_fields;
FormField* last_employer = last_employer_fields;
FormField* screening_questions = screening_questions_fields;

// --- Function Prototypes ---
// Page functions must return int as they are called in main and return get_response values.
int get_response(void);
void print_menu(void);
int fill_out_form(FormField* fields, int num_fields);
int update_field(char *field_name_to_update, FormField* fields, int num_fields);
void print_page(const char* header_text, FormField* fields, int num_fields);
int form(const char* header_text, FormField* fields, int num_fields);

int candidate_info(void);
int contact(void);
int address(void);
int education(void);
int employment_history(void);
int final_screening(void);
int finished(void);

// Array of function pointers for pages
int (*job_application_page[])(void) = {
    candidate_info,
    contact,
    address,
    education,
    employment_history,
    final_screening,
    finished
};
const int NUM_PAGES = sizeof(job_application_page) / sizeof(job_application_page[0]);

// --- Core Logic Functions ---

int get_response(void) {
    size_t bytes_read_count = 0;
    ssize_t bytes_read_this_iter;
    char* current_char_ptr = g_user_resp;
    const size_t max_len = sizeof(g_user_resp) - 1; // Leave space for null terminator

    while (bytes_read_count < max_len) {
        bytes_read_this_iter = read(STDIN_FILENO, current_char_ptr, 1);

        if (bytes_read_this_iter <= 0) {
            // Error or EOF
            if (bytes_read_this_iter == 0) { // EOF
                break;
            }
            // Error reading from stdin
            return -1; // Original 0xffffffff
        }

        if (*current_char_ptr == '\n') {
            break;
        }

        current_char_ptr++;
        bytes_read_count++;
    }

    *current_char_ptr = '\0'; // Null-terminate the string

    if (bytes_read_count == max_len && *current_char_ptr != '\n') {
        // Buffer full, and no newline found
        return -1; // Original 0xffffffff
    }

    if (*g_user_resp == '\0') { // Check if empty input (just newline)
        return 1; // Valid, but empty input
    }

    // Check for commands
    if (bytes_read_count >= 2 && g_user_resp[0] == '*' && g_user_resp[1] == '*') {
        const char* command_start = g_user_resp + 2;
        size_t cmd_len;

        cmd_len = strlen(CMD_PREV);
        if (memcmp(command_start, CMD_PREV, cmd_len) == 0 && command_start[cmd_len] == '\0') {
            return 2; // Prev
        }
        cmd_len = strlen(CMD_NEXT);
        if (memcmp(command_start, CMD_NEXT, cmd_len) == 0 && command_start[cmd_len] == '\0') {
            return 3; // Next
        }
        cmd_len = strlen(CMD_UPDATE);
        if (memcmp(command_start, CMD_UPDATE, cmd_len) == 0) {
            // The original code checks only for the prefix, implying arguments follow
            return 4; // Update
        }
        cmd_len = strlen(CMD_HELP);
        if (memcmp(command_start, CMD_HELP, cmd_len) == 0 && command_start[cmd_len] == '\0') {
            return 5; // Help
        }
        cmd_len = strlen(CMD_EXIT);
        if (memcmp(command_start, CMD_EXIT, cmd_len) == 0 && command_start[cmd_len] == '\0') {
            return 6; // Exit
        }
    }

    return 1; // Default response for valid, non-command user input
}

void print_menu(void) {
    fdprintf(1, "All commands begin with \'**\' and may be entered at any time\n");
    fdprintf(1, "**prev <Return to the previous page>\n");
    fdprintf(1, "**next <Move to the next page>\n");
    fdprintf(1, "**update [id] <Update field, ex: \"**update First Name\">\n");
    fdprintf(1, "**help <Print this dialogue>\n");
    fdprintf(1, "**exit <Exit application>\n");
}

int fill_out_form(FormField* fields, int num_fields) {
    for (int i = 0; i < num_fields; ++i) {
        FormField* field = &fields[i];

        // Only prompt if the field has not been filled yet
        while (field->value_ptr == NULL) {
            fdprintf(1, "%s (%s): ", field->name, field->prompt);
            int response = get_response();

            if (response != 1) { // Not a valid user input (e.g., command, read error)
                return response;
            }

            // Check if input is empty and field allows it (type_char == '\0')
            if (field->type_char == '\0' && g_user_resp[0] == DAT_0001500a) {
                field->value_ptr = (char*)calloc(1, field->max_len);
                if (!field->value_ptr) {
                    perror("calloc failed");
                    exit(EXIT_FAILURE);
                }
                break; // Field filled (with empty string)
            }

            // Validate the input
            if (field->validator(g_user_resp, field->max_len) == 0) {
                field->value_ptr = (char*)calloc(1, field->max_len);
                if (!field->value_ptr) {
                    perror("calloc failed");
                    exit(EXIT_FAILURE);
                }
                strcpy(field->value_ptr, g_user_resp);
                break; // Field filled
            } else {
                fdprintf(1, "Invalid input for '%s'. Please try again.\n", field->name);
            }
        }
    }
    return 0; // All fields filled or skipped
}

int update_field(char *field_name_to_update, FormField* fields, int num_fields) {
    if (!field_name_to_update || strlen(field_name_to_update) == 0) {
        fdprintf(1, "Bad field name provided for update.\n");
        return -1; // Original 0xffffffff
    }

    for (int i = 0; i < num_fields; ++i) {
        FormField* field = &fields[i];
        size_t name_len = strlen(field_name_to_update);

        if (name_len == strlen(field->name) &&
            memcmp(field_name_to_update, field->name, name_len) == 0) {

            // Found the field
            fdprintf(1, "%s (%s): ", field->name, field->prompt);
            int response = get_response();

            if (response != 1) { // Not a valid user input
                fdprintf(1, "Bad command for field '%s'.\n", field->name);
                return -2; // Original 0xfffffffe
            }

            // Check if input is empty and field allows it
            if (field->type_char == '\0' && g_user_resp[0] == DAT_0001500a) {
                if (field->value_ptr) { // If previously allocated, clear it
                    memset(field->value_ptr, 0, field->max_len);
                } else { // Allocate if not already
                    field->value_ptr = (char*)calloc(1, field->max_len);
                    if (!field->value_ptr) {
                        perror("calloc failed");
                        exit(EXIT_FAILURE);
                    }
                }
                return 0; // Successfully updated (to empty string)
            }

            // Validate and update
            if (field->validator(g_user_resp, field->max_len) == 0) {
                if (!field->value_ptr) { // Allocate if not already
                    field->value_ptr = (char*)calloc(1, field->max_len);
                    if (!field->value_ptr) {
                        perror("calloc failed");
                        exit(EXIT_FAILURE);
                    }
                }
                memset(field->value_ptr, 0, field->max_len); // Clear existing content
                strcpy(field->value_ptr, g_user_resp);
                return 0; // Successfully updated
            } else {
                fdprintf(1, "Bad input for field '%s'.\n", field->name);
                return -4; // Original 0xfffffffc
            }
        }
    }

    fdprintf(1, "Could not find specified field '%s'.\n", field_name_to_update);
    return -1; // Original 0xffffffff
}

void print_page(const char* header_text, FormField* fields, int num_fields) {
    if (header_text) {
        fdprintf(1, "%s", header_text);
    }
    for (int i = 0; i < num_fields; ++i) {
        FormField* field = &fields[i];
        fdprintf(1, "%s=%s\n", field->name, field->value_ptr ? field->value_ptr : "[UNFILLED]");
    }
}

int form(const char* header_text, FormField* fields, int num_fields) {
    int response = 0;

    // Check if the user is trying to skip pages
    if (g_last_page_completed + 1 < (int)g_page_idx) {
        fdprintf(1, "You must complete the previous page before proceeding to this page.\n");
        g_page_idx = g_last_page_completed + 1; // Revert to the next uncompleted page
        return -1; // Indicate an issue to main loop
    }

    int all_fields_filled = 1;
    for (int i = 0; i < num_fields; ++i) {
        if (fields[i].value_ptr == NULL) {
            all_fields_filled = 0;
            break;
        }
    }

    // If this is the current page being filled or if there are unfilled fields
    if ((int)g_page_idx == g_last_page_completed + 1 || !all_fields_filled) {
        response = fill_out_form(fields, num_fields);
        if (response != 0) {
            if (response == 4) { // Update command encountered during fill_out_form
                fdprintf(1, "Cannot update field until all fields are inputted on this page.\n");
            }
            return response; // Return command/error from fill_out_form
        }
        g_last_page_completed = g_page_idx; // Mark this page as completed
    }

    print_page(header_text, fields, num_fields);
    fdprintf(1, "\nType **next to continue\n");

    response = get_response();

    if (response == 4) { // Update command
        char *update_arg_ptr = g_user_resp;
        strsep(&update_arg_ptr, " "); // Discard "**update" part
        if (update_arg_ptr) { // Check if there's an argument after "update "
            int update_result = update_field(update_arg_ptr, fields, num_fields);
            if (update_result != 0) {
                fdprintf(1, "Update Unsuccessful: %d\n", update_result);
            }
        } else {
            fdprintf(1, "Update Unsuccessful: No field name provided.\n");
        }
    }
    return response;
}

// --- Page Functions ---

int candidate_info(void) {
    fdprintf(1, "\nCandidate Info Form\n");
    return form("\n*********Candidate Info:*********\n", applicant_id, NUM_APPLICANT_ID_FIELDS);
}

int contact(void) {
    fdprintf(1, "\nContact Info Form\n");
    return form("\n*********Contact Info:*********\n", contact_info, NUM_CONTACT_INFO_FIELDS);
}

int address(void) {
    fdprintf(1, "\nAddress Form\n");
    return form("\n*********Address:*********\n", current_address, NUM_CURRENT_ADDRESS_FIELDS);
}

int education(void) {
    fdprintf(1, "\nEducation Form\n");
    return form("\n*********Highest Education:*********\n", highest_education, NUM_HIGHEST_EDUCATION_FIELDS);
}

int employment_history(void) {
    fdprintf(1, "\nEmployment Form\n");
    return form("\n*********Most Recent Employer:*********\n", last_employer, NUM_LAST_EMPLOYER_FIELDS);
}

int final_screening(void) {
    fdprintf(1, "\nFinal Questions\n");
    return form("\n*********Final Screening:*********\n", screening_questions, NUM_SCREENING_QUESTIONS_FIELDS);
}

int finished(void) {
    fdprintf(1, "\n\nYou have completed your application with the Sea Eye Association.\n");
    fdprintf(1, "You may review the form. Navigate through the application with **prev and **next.\n");
    fdprintf(1, "Once your are satisfied type **exit to exit and submit the form\n");
    fdprintf(1, "If you wish to discard your application, please use Control-C\n");

    if (g_last_page_completed + 1 < (int)g_page_idx) {
        fdprintf(1, "You must complete the previous page before proceeding to this page\n");
        g_page_idx = g_last_page_completed + 1;
        return -1; // Original 0xffffffff
    } else {
        return get_response();
    }
}

// --- Main Function ---

int main(void) {
    int response;

    fdprintf(1, GREETING_MSG);
    fdprintf(1, "Thanks for your interest in the Sea Eye Association.\n");
    fdprintf(1, "In order to be considered for the job complete the preliminary online background check\n");
    fdprintf(1, "Due to the secure nature of the position you are applying for you may be asked to\n");
    fdprintf(1, "submit additional paperwork after your preliminary background check has been approved.\n");
    fdprintf(1, "Thank you for your cooperation\n");

    g_page_idx = 0; // Start at the first page (index 0)
    g_last_page_completed = -1; // No pages completed initially

    while (1) { // Loop until exit command (response == 6)
        // Ensure g_page_idx is within valid bounds
        if (g_page_idx < 0) g_page_idx = 0;
        if (g_page_idx >= NUM_PAGES) g_page_idx = NUM_PAGES - 1;

        // Call the current page's function
        response = job_application_page[g_page_idx]();

        if (response == 6) { // Exit command
            break;
        } else if (response == 5) { // Help command
            print_menu();
        } else if (response == 2) { // Prev command
            if (g_page_idx > 0) {
                g_page_idx--;
            }
        } else if (response == 3) { // Next command
            if (g_page_idx < NUM_PAGES - 1) { // NUM_PAGES - 1 is the last valid index
                g_page_idx++;
            }
        }
        // Other responses (1 for valid input, 4 for update, or errors)
        // result in staying on the current page to re-display or re-prompt.
    }

    fdprintf(1, "Thank you!\n");
    return 0;
}