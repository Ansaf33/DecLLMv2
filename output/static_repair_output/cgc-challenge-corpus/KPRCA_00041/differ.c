#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h> // For uint and intptr_t

// Define uint for consistency if not provided by stdint.h
#ifndef uint
typedef unsigned int uint;
#endif

// Custom fdprintf wrapper for stdout
// Original code used fdprintf(1, ...), which typically means stdout.
// For simplicity, we'll map it to fprintf(stdout, ...).
#define fdprintf(fd, format, ...) fprintf(stdout, format, ##__VA_ARGS__)

// Global variables (forward declarations)
typedef struct LineNode LineNode;
typedef struct MatchData MatchData;
typedef struct MatchNode MatchNode;

LineNode *g_lfile_cmp = NULL;
LineNode *g_rfile_cmp = NULL;
MatchNode *g_best_match = NULL;
int g_cached = 0;
char ee_str[256] = {0}; // Assuming a reasonable size for ee_str

// Type definitions for clarity based on usage in the snippet
struct LineNode {
    char **line_data; // Points to an array of char* (the actual line string at line_data[0] or line_data[1], plus metadata)
    struct LineNode *next;
};

struct MatchData {
    uint left_idx;  // Start index in left file (0-based)
    uint right_idx; // Start index in right file (0-based)
    uint length;    // Length of the match
};

struct MatchNode {
    MatchData *data;
    struct MatchNode *next;
};

// --- Function Stubs (if original source not provided) ---
// These functions are called but not defined in the snippet.
// They are needed for compilation.
LineNode *pre_process(void *file_info_ptr, void *arg2, void *arg3) {
    // Placeholder implementation. In a real scenario, this would load and process file lines.
    // For now, return NULL or a dummy list if needed for testing.
    (void)file_info_ptr;
    (void)arg2;
    (void)arg3;
    return NULL;
}

void free_linecmp_list(LineNode **list_head_ptr) {
    LineNode *current = *list_head_ptr;
    while (current != NULL) {
        LineNode *next = current->next;
        // The structure of line_data (char**) is not fully clear from the snippet.
        // Assuming current->line_data is a single malloc'd block containing char* pointers.
        // If the actual strings pointed to by current->line_data[i] are also malloc'd, they would need freeing here.
        if (current->line_data != NULL) {
            // Example if line_data[0] is the malloc'd string:
            // if (current->line_data[0] != NULL) free(current->line_data[0]);
            free(current->line_data); // Free the array of char*
        }
        free(current); // Free the node itself
        current = next;
    }
    *list_head_ptr = NULL;
}
// --- End Function Stubs ---


// Function: compare_binary_files
// Param_1 and param_2 are used in pointer arithmetic, suggesting they are pointers to file info structures.
// Changed signature from int to void* for correct 64-bit pointer handling.
void compare_binary_files(void *file1_info, void *file2_info) {
    bool identical = true;
    unsigned int size1 = *(unsigned int *)((char *)file1_info + 0x14);
    unsigned int size2 = *(unsigned int *)((char *)file2_info + 0x14);

    if (size1 != size2) {
        identical = false;
    } else {
        for (uint i = 0; i < size1; ++i) {
            if (*(char *)((char *)file1_info + 0x18 + i) != *(char *)((char *)file2_info + 0x18 + i)) {
                identical = false;
                break;
            }
        }
    }

    if (identical) {
        fdprintf(1, "Binary files match\n");
    } else {
        // Assuming (char*)file1_info + 4 points to a filename string
        fdprintf(1, "Binary files %s and %s are not identical\n", (char *)file1_info + 4, (char *)file2_info + 4);
    }
}

// Function: compare_line
int compare_line(char **param_1, char **param_2, int param_3) {
    const char *str1;
    const char *str2;
    size_t len1, len2;

    if (param_3 == 0) {
        str1 = *param_1;
        str2 = *param_2;
        len1 = strlen(str1);
        len2 = strlen(str2);

        if (len1 != 0 && len2 != 0) {
            // Original logic compares pointer values param_1[2/3] == param_2[2/3]
            if (len1 == len2 && param_1[2] == param_2[2] && param_1[3] == param_2[3]) {
                return strcmp(str1, str2);
            }
        }
    } else { // param_3 != 0
        str1 = param_1[1];
        str2 = param_2[1];
        len1 = strlen(str1);
        len2 = strlen(str2);

        if (len1 != 0 && len2 != 0) {
            // Original logic compares pointer values param_1[4/5] == param_2[4/5]
            if (len1 == len2 && param_1[4] == param_2[4] && param_1[5] == param_2[5]) {
                return strcmp(str1, str2);
            }
        }
    }
    return -1; // No match or empty string
}

// Function: generate_matches
MatchNode *generate_matches(LineNode *file1_lines, LineNode *file2_lines, int param_3) {
    MatchNode *result_list_head = NULL;
    int left_file_idx = 0;
    LineNode *current_left_node = file1_lines;

    while (current_left_node != NULL) {
        LineNode *current_right_node = file2_lines;
        int right_file_idx = 0;

        while (current_right_node != NULL) {
            LineNode *temp_left_node = current_left_node;
            LineNode *temp_right_node = current_right_node;
            int sequence_start_left_idx = 0;
            int sequence_start_right_idx = 0;
            int sequence_length = 0;
            bool sequence_found = false;

            // Compare lines to find a sequence of matches
            while (temp_left_node != NULL && temp_right_node != NULL &&
                   compare_line(temp_left_node->line_data, temp_right_node->line_data, param_3) == 0) {
                if (!sequence_found) {
                    sequence_start_left_idx = left_file_idx;
                    sequence_start_right_idx = right_file_idx;
                    sequence_length = 1;
                    sequence_found = true;
                } else {
                    sequence_length++;
                }
                temp_left_node = temp_left_node->next;
                temp_right_node = temp_right_node->next;
                // Note: right_file_idx is not incremented here for the sequence, only for the inner loop iteration below.
                // This preserves the original algorithm's behavior where right_file_idx advances by 1 per inner loop iteration.
            }

            if (sequence_found) {
                // A match sequence was found, create a new MatchNode
                MatchData *new_match_data = (MatchData *)malloc(sizeof(MatchData));
                if (new_match_data == NULL) {
                    // Malloc failed, return partial list
                    return result_list_head;
                }
                new_match_data->left_idx = sequence_start_left_idx;
                new_match_data->right_idx = sequence_start_right_idx;
                new_match_data->length = sequence_length;

                MatchNode *new_match_node = (MatchNode *)malloc(sizeof(MatchNode));
                if (new_match_node == NULL) {
                    free(new_match_data);
                    return result_list_head;
                }
                new_match_node->data = new_match_data;
                new_match_node->next = NULL;

                // Insert new_match_node into result_list_head, maintaining sorted order and handling overlaps
                if (result_list_head == NULL) {
                    result_list_head = new_match_node;
                } else {
                    MatchNode *prev_node_in_list = NULL;
                    MatchNode *current_node_in_list = result_list_head;
                    bool inserted = false;

                    while (current_node_in_list != NULL) {
                        // Sort by left_idx primarily
                        if (new_match_data->left_idx <= current_node_in_list->data->left_idx) {
                            if (prev_node_in_list == NULL) { // Insert at head
                                new_match_node->next = result_list_head;
                                result_list_head = new_match_node;
                            } else { // Insert in middle
                                new_match_node->next = current_node_in_list;
                                prev_node_in_list->next = new_match_node;
                            }
                            inserted = true;
                            break;
                        }

                        // Overlap check and discard logic:
                        // If new match starts after current_node_in_list (from previous condition)
                        // AND new match's end is <= current_node_in_list's end, discard new match.
                        if (new_match_data->left_idx + new_match_data->length <=
                            current_node_in_list->data->left_idx + current_node_in_list->data->length) {
                            free(new_match_data);
                            free(new_match_node);
                            inserted = true; // Mark as handled (discarded)
                            break;
                        }

                        prev_node_in_list = current_node_in_list;
                        current_node_in_list = current_node_in_list->next;
                    }

                    if (!inserted) { // Append to end if not inserted yet
                        prev_node_in_list->next = new_match_node;
                    }
                }
            }
            // Advance right file pointer for next inner loop iteration, regardless of whether a sequence was found
            current_right_node = current_right_node->next;
            right_file_idx++;
        }
        // Advance left file pointer for next outer loop iteration
        current_left_node = current_left_node->next;
        left_file_idx++;
    }
    return result_list_head;
}

// Function: find_single_match_set
MatchNode *find_single_match_set(MatchNode **all_matches_list_head_ptr, int *total_score) {
    MatchNode *single_match_set_head = NULL;
    MatchNode *single_match_set_tail = NULL;

    if (*all_matches_list_head_ptr == NULL) {
        *total_score = 0;
        return NULL;
    }

    MatchNode *current_node = *all_matches_list_head_ptr;
    MatchNode *prev_node_in_source_list = NULL; // Tracks the node *before* current_node in the source list
    uint max_right_end_idx = 0;
    uint max_left_end_idx = 0;
    *total_score = 0;

    while (current_node != NULL) {
        // Check for overlap with previously selected matches in the current set
        if (current_node->data->right_idx < max_right_end_idx || current_node->data->left_idx < max_left_end_idx) {
            // Overlapping match, skip it.
            prev_node_in_source_list = current_node;
            current_node = current_node->next;
        } else {
            // Non-overlapping match, select it.
            *total_score += (current_node->data->length * (current_node->data->length + 1) >> 1);
            max_right_end_idx = current_node->data->length + current_node->data->right_idx;
            max_left_end_idx = current_node->data->length + current_node->data->left_idx;

            // Remove current_node from `all_matches_list_head_ptr`
            MatchNode *node_to_move = current_node;
            if (prev_node_in_source_list == NULL) { // current_node is the head of the source list
                *all_matches_list_head_ptr = current_node->next;
            } else { // current_node is not the head
                prev_node_in_source_list->next = current_node->next;
            }
            current_node = current_node->next; // Advance current_node to the next node in the original list

            // Add node_to_move to `single_match_set_head`
            node_to_move->next = NULL; // Detach from original list
            if (single_match_set_head == NULL) {
                single_match_set_head = node_to_move;
                single_match_set_tail = node_to_move;
            } else {
                single_match_set_tail->next = node_to_move;
                single_match_set_tail = node_to_move;
            }
            // prev_node_in_source_list does not change here.
            // If the head was removed, prev_node_in_source_list remains NULL.
            // If a non-head node was removed, prev_node_in_source_list correctly points to the node *before* the one just moved.
        }
    }
    return single_match_set_head;
}

// Function: find_best_match_set
MatchNode *find_best_match_set(MatchNode *all_matches_list_head) {
    MatchNode *best_match_set = NULL;
    uint best_score = 0;
    int current_score;
    MatchNode *current_single_set;

    // Loop as long as find_single_match_set can extract a non-empty set
    while ((current_single_set = find_single_match_set(&all_matches_list_head, &current_score)) != NULL) {
        if ((uint)current_score > best_score) {
            // Current set is better, free the old best set
            MatchNode *temp_node = best_match_set;
            while (temp_node != NULL) {
                MatchNode *next_node = temp_node->next;
                free(temp_node->data); // Free MatchData
                free(temp_node);        // Free MatchNode
                temp_node = next_node;
            }
            best_score = (uint)current_score;
            best_match_set = current_single_set;
        } else {
            // Current set's score is not better, free it
            MatchNode *temp_node = current_single_set;
            while (temp_node != NULL) {
                MatchNode *next_node = temp_node->next;
                free(temp_node->data);
                free(temp_node);
                temp_node = next_node;
            }
        }
    }
    // Ensure any remaining matches in all_matches_list_head (those not picked for any single set) are freed
    MatchNode *temp_node = all_matches_list_head;
    while (temp_node != NULL) {
        MatchNode *next_node = temp_node->next;
        free(temp_node->data);
        free(temp_node);
        temp_node = next_node;
    }
    return best_match_set;
}

// Function: compare_lines
MatchNode *compare_lines(LineNode *file1_lines, LineNode *file2_lines, int param_3) {
    MatchNode *all_matches = generate_matches(file1_lines, file2_lines, param_3);
    MatchNode *best_matches = find_best_match_set(all_matches);
    return best_matches;
}

// Function: print_diff
void print_diff(void) {
    bool changes_found = false;
    uint left_current_line_idx = 0;  // Corresponds to local_18 in original
    uint right_current_line_idx = 0; // Corresponds to local_1c in original
    int mystery_flag = 1;            // Corresponds to local_14 in original

    LineNode *left_file_ptr = g_lfile_cmp;  // Corresponds to local_24 in original
    LineNode *right_file_ptr = g_rfile_cmp; // Corresponds to local_28 in original
    MatchNode *current_match_node = g_best_match; // Corresponds to local_2c in original

    const char *separator_str = "---\n"; // Assuming DAT_00015097

    if (g_best_match == NULL) {
        fdprintf(1, "Files don\'t match at all\n");
    } else {
        while (current_match_node != NULL) {
            uint match_left_start = current_match_node->data->left_idx;
            uint match_right_start = current_match_node->data->right_idx;
            uint match_length = current_match_node->data->length;

            // The original diff logic is non-standard in how it determines 'r', 'a', 'c' operations
            // and the ranges involved. This implementation preserves the original logic flow
            // and variable usage.

            // Handle lines unique to left file before this match block
            if (left_current_line_idx < match_left_start) {
                changes_found = true;
                // Original format: "%d,%dr%d,%d\n", local_18, (*local_2c)[1], local_1c, **local_2c
                // Interpreted as: left_curr+1, match_right_start+1 'r' right_curr+1, match_left_start+1
                fdprintf(1, "%d,%dr%d,%d\n", left_current_line_idx + 1, match_left_start, right_current_line_idx + 1, right_current_line_idx);
                for (uint i = 0; i < match_left_start - left_current_line_idx; i++) {
                    fdprintf(1, "< %s\n", left_file_ptr->line_data[0]);
                    left_file_ptr = left_file_ptr->next;
                }
                left_current_line_idx = match_left_start;
            }

            // Handle lines unique to right file before this match block
            if (right_current_line_idx < match_right_start) {
                changes_found = true;
                // Original format: "%d,%da%d,%d\n", local_18, (*local_2c)[1], local_1c, **local_2c
                // Interpreted as: left_curr+1, match_right_start+1 'a' right_curr+1, match_left_start+1
                fdprintf(1, "%d,%da%d,%d\n", left_current_line_idx + 1, left_current_line_idx, right_current_line_idx + 1, match_right_start);
                for (uint i = 0; i < match_right_start - right_current_line_idx; i++) {
                    fdprintf(1, "> %s\n", right_file_ptr->line_data[0]);
                    right_file_ptr = right_file_ptr->next;
                }
                right_current_line_idx = match_right_start;
            }
            
            // Handle 'r', 'a', 'c' operations based on specific match start index comparisons
            if (match_left_start < match_right_start) { // Original: **local_2c < (*local_2c)[1]
                changes_found = true;
                // Original printf arguments are: local_18, (*local_2c)[1], local_1c, **local_2c
                // The range for printing '<' lines is (*local_2c)[1] - local_18
                fdprintf(1, "%d,%dr%d,%d\n", left_current_line_idx + 1, match_right_start + 1, right_current_line_idx + 1, match_left_start + 1);
                for (uint i = 0; i < match_right_start - left_current_line_idx; i++) {
                    fdprintf(1, "< %s\n", left_file_ptr->line_data[0]);
                    left_file_ptr = left_file_ptr->next;
                }
                left_current_line_idx = match_right_start; // Update `left_current_line_idx` as per original logic
            } else if (match_right_start < match_left_start) { // Original: (*local_2c)[1] < **local_2c
                changes_found = true;
                // Original printf arguments are: local_18, (*local_2c)[1], local_1c, **local_2c
                // The range for printing '>' lines is **local_2c - local_1c
                fdprintf(1, "%d,%da%d,%d\n", left_current_line_idx + 1, match_right_start + 1, right_current_line_idx + 1, match_left_start + 1);
                for (uint i = 0; i < match_left_start - right_current_line_idx; i++) {
                    fdprintf(1, "> %s\n", right_file_ptr->line_data[0]);
                    right_file_ptr = right_file_ptr->next;
                }
                right_current_line_idx = match_left_start; // Update `right_current_line_idx` as per original logic
            } else if (left_current_line_idx != match_right_start || right_current_line_idx != match_left_start) {
                // Original: (local_18 != (*local_2c)[1]) || (local_1c != **local_2c)
                // This condition applies if match_left_start == match_right_start,
                // but current_left_line_idx or current_right_line_idx are not yet aligned.
                // This implies a 'c' (change) operation.
                changes_found = true;
                if (mystery_flag != 0) {
                    if ((uint)(mystery_flag * mystery_flag) == left_current_line_idx) {
                        mystery_flag++;
                    } else {
                        mystery_flag = 0;
                    }
                    if (mystery_flag == 0x20) { // 32
                        strcpy(ee_str, left_file_ptr->line_data[0]);
                    }
                }
                // Original printf arguments are: local_18, (*local_2c)[1], local_1c, **local_2c
                // The ranges for printing '<' lines is (*local_2c)[1] - local_18
                // The ranges for printing '>' lines is **local_2c - local_1c
                fdprintf(1, "%d,%dc%d,%d\n", left_current_line_idx + 1, match_right_start + 1, right_current_line_idx + 1, match_left_start + 1);
                for (uint i = 0; i < match_right_start - left_current_line_idx; i++) {
                    fdprintf(1, "< %s\n", left_file_ptr->line_data[0]);
                    left_file_ptr = left_file_ptr->next;
                }
                left_current_line_idx = match_right_start; // Update `left_current_line_idx` as per original logic
                fdprintf(1, "%s", separator_str);
                for (uint i = 0; i < match_left_start - right_current_line_idx; i++) {
                    fdprintf(1, "> %s\n", right_file_ptr->line_data[0]);
                    right_file_ptr = right_file_ptr->next;
                }
                right_current_line_idx = match_left_start; // Update `right_current_line_idx` as per original logic
            }

            // Advance file pointers past the matched block
            for (; left_current_line_idx < match_left_start + match_length; left_current_line_idx++) {
                left_file_ptr = left_file_ptr->next;
            }
            for (; right_current_line_idx < match_right_start + match_length; right_current_line_idx++) {
                right_file_ptr = right_file_ptr->next;
            }
            current_match_node = current_match_node->next;
        }

        // Handle remaining unmatched lines after all matches have been processed
        if (left_file_ptr != NULL || right_file_ptr != NULL) {
            changes_found = true;
            fdprintf(1, "%d,Ea%d,E\n", left_current_line_idx + 1, right_current_line_idx + 1);

            while (left_file_ptr != NULL) {
                size_t line_len = strlen(left_file_ptr->line_data[0]);
                if (line_len != 0) {
                    fdprintf(1, "< %s\n", left_file_ptr->line_data[0]);
                }
                left_file_ptr = left_file_ptr->next;
            }
            while (right_file_ptr != NULL) {
                size_t line_len = strlen(right_file_ptr->line_data[0]);
                if (line_len != 0) {
                    fdprintf(1, "> %s\n", right_file_ptr->line_data[0]);
                }
                right_file_ptr = right_file_ptr->next;
            }
        }

        // Print ee_str if mystery_flag reached 32
        if (mystery_flag == 0x20) {
            size_t ee_str_len = strlen(ee_str);
            if (ee_str_len != 0) {
                fdprintf(1, ":D = %s\n", ee_str);
            }
        }

        if (!changes_found) {
            fdprintf(1, "Files are identical\n");
        }
    }
}

// Function: compare_files
// Param_1 and param_2 are used as pointers to file info structures.
// Changed signature from int to void* for correct 64-bit pointer handling.
void compare_files(void *file1_info, void *file2_info, int param_3, int param_4) {
    // These are temporary buffers or output parameters for pre_process,
    // but their content is not used after the call in the provided snippet.
    // They are `undefined` in the original, so using char arrays as placeholders.
    char dummy_buffer1[8];
    char dummy_buffer2[4];
    char dummy_buffer3[4];
    char dummy_buffer4[4];

    if (file1_info == NULL || file2_info == NULL) {
        fdprintf(1, "Both files must be loaded before comparing\n");
        return;
    }

    if (param_4 == 0) { // If not "Treat as Ascii"
        // Assuming *(char *)(file1_info + 3) is the file type byte
        if (*(char *)((char *)file1_info + 3) != *(char *)((char *)file2_info + 3)) {
            fdprintf(1,
                     "File types must be identical in order to compare them.\nAlternatively, enable \"Treat as Ascii\"\n");
            return;
        }
        if (*(char *)((char *)file1_info + 3) == '\0') { // If file type is binary (0)
            compare_binary_files(file1_info, file2_info);
            return;
        }
    }

    if (g_cached == 0) {
        if (g_lfile_cmp == NULL) {
            g_lfile_cmp = pre_process(file1_info, dummy_buffer1, dummy_buffer2);
        }
        if (g_lfile_cmp == NULL) {
            fdprintf(1, "File 1 could not be processed\n");
            return;
        }
        if (g_rfile_cmp == NULL) {
            g_rfile_cmp = pre_process(file2_info, dummy_buffer3, dummy_buffer4);
        }
        if (g_rfile_cmp == NULL) {
            fdprintf(1, "File 2 could not be processed\n");
            return;
        }
        g_best_match = compare_lines(g_lfile_cmp, g_rfile_cmp, param_3);
        g_cached = 1;
    }
    print_diff();
}

// Function: clear_cache
void clear_cache(int param_1) {
    // Free g_best_match list
    MatchNode *current_match = g_best_match;
    while (current_match != NULL) {
        MatchNode *next_match = current_match->next;
        free(current_match->data); // Free MatchData
        free(current_match);        // Free MatchNode
        current_match = next_match;
    }
    g_best_match = NULL;

    // Free g_lfile_cmp or g_rfile_cmp based on param_1
    if (param_1 == 1) {
        free_linecmp_list(&g_lfile_cmp);
    } else if (param_1 == 2) {
        free_linecmp_list(&g_rfile_cmp);
    }
    g_cached = 0;
}