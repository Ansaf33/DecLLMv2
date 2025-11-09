#include <stdio.h>
#include <stdlib.h> // For calloc, free, exit
#include <string.h> // For strlen, strcpy

// Forward declarations for linked lists
typedef struct Pbi Pbi;
typedef struct Sprint Sprint;
typedef struct Project Project;

// Pbi (Product Backlog Item) structure
// Interpreted from original code's memory accesses and calloc(0x10, ...)
// Assuming: short = 2 bytes, pointer = 4 bytes (32-bit architecture)
struct Pbi {
    short id;           // Offset 0
    short points;       // Offset 2
    short status;       // Offset 4
    short _padding1;    // 2 bytes padding to align description pointer to 4-byte boundary
    char *description;  // Offset 8
    Pbi *next;          // Offset 12
}; // Total size: 16 bytes (0x10)

// Sprint structure
// Interpreted from original code's memory accesses
// Assuming: short = 2 bytes, pointer = 4 bytes (32-bit architecture)
struct Sprint {
    short id;           // Offset 0
    short _padding0[4]; // 8 bytes padding to align total_points to offset 10
    short total_points; // Offset 10
    short _padding1[2]; // 4 bytes padding to align pbi_list_head pointer to 4-byte boundary (offset 16)
    Pbi *pbi_list_head; // Offset 16
    Sprint *next;       // Offset 20
}; // Total size: 24 bytes

// Project structure (Root node for the entire system)
// Interpreted from original code's memory accesses
// Assuming: short = 2 bytes, pointer = 4 bytes (32-bit architecture)
struct Project {
    short id;                   // Offset 0
    short _padding0[3];         // 6 bytes padding to align pbi_list_head pointer to 4-byte boundary (offset 8)
    Pbi *pbi_list_head;         // Offset 8
    Sprint *sprint_list_head;   // Offset 12
    Project *next;              // Offset 16
}; // Total size: 20 bytes

// Helper to find a Project node by ID
Project *find_project(Project *head, short project_id) {
    Project *current = head;
    while (current != NULL && current->id != project_id) {
        current = current->next;
    }
    return current;
}

// Helper to find a PBI node by ID in a list
Pbi *find_pbi(Pbi *head, short pbi_id) {
    Pbi *current = head;
    while (current != NULL && current->id != pbi_id) {
        current = current->next;
    }
    return current;
}

// Helper to find a Sprint node by ID in a list
Sprint *find_sprint(Sprint *head, short sprint_id) {
    Sprint *current = head;
    while (current != NULL && current->id != sprint_id) {
        current = current->next;
    }
    return current;
}

// Function: create_pbi
// project_list_head: Head of the global list of projects
// data_params: short array { project_id, pbi_id, pbi_points, ...description_string_as_shorts... }
int create_pbi(Project *project_list_head, short *data_params) {
    short project_id = data_params[0];
    short pbi_id = data_params[1];
    short pbi_points = data_params[2];
    char *pbi_description_src = (char *)(data_params + 3); // String starts after 3 shorts

    Project *project_node = find_project(project_list_head, project_id);
    if (project_node == NULL) {
        return -1; // Project not found
    }

    Pbi *new_pbi = (Pbi *)calloc(1, sizeof(Pbi));
    if (new_pbi == NULL) {
        perror("Failed to allocate PBI");
        exit(EXIT_FAILURE);
    }

    new_pbi->id = pbi_id;
    new_pbi->points = pbi_points;
    new_pbi->status = 0; // Initial status
    new_pbi->next = NULL;

    size_t desc_len = strlen(pbi_description_src);
    new_pbi->description = (char *)calloc(1, desc_len + 1);
    if (new_pbi->description == NULL) {
        perror("Failed to allocate PBI description");
        free(new_pbi); // Clean up partially allocated PBI
        exit(EXIT_FAILURE);
    }
    strcpy(new_pbi->description, pbi_description_src);

    if (project_node->pbi_list_head == NULL) {
        project_node->pbi_list_head = new_pbi;
    } else {
        Pbi *current_pbi = project_node->pbi_list_head;
        while (current_pbi->next != NULL) {
            current_pbi = current_pbi->next;
        }
        current_pbi->next = new_pbi;
    }
    return 0;
}

// Function: delete_pbi
// project_list_head: Head of the global list of projects
// data_params: short array { project_id, pbi_id, ... }
int delete_pbi(Project *project_list_head, short *data_params) {
    short project_id = data_params[0];
    short pbi_id = data_params[1];

    Project *project_node = find_project(project_list_head, project_id);
    if (project_node == NULL) {
        return -1; // Project not found
    }

    Pbi *current_pbi = project_node->pbi_list_head;
    Pbi *prev_pbi = NULL;

    while (current_pbi != NULL && current_pbi->id != pbi_id) {
        prev_pbi = current_pbi;
        current_pbi = current_pbi->next;
    }

    if (current_pbi == NULL) {
        return -1; // PBI not found
    }

    if (prev_pbi == NULL) { // PBI is the head
        project_node->pbi_list_head = current_pbi->next;
    } else {
        prev_pbi->next = current_pbi->next;
    }

    if (current_pbi->description != NULL) {
        free(current_pbi->description);
    }
    free(current_pbi);
    return 0;
}

// Function: move_pbi_to_sprint
// project_list_head: Head of the global list of projects
// data_params: short array { project_id, pbi_id, sprint_id, ... }
int move_pbi_to_sprint(Project *project_list_head, short *data_params) {
    short project_id = data_params[0];
    short pbi_id = data_params[1];
    short sprint_id = data_params[2];

    Project *project_node = find_project(project_list_head, project_id);
    if (project_node == NULL) {
        return -1; // Project not found
    }

    Sprint *target_sprint = find_sprint(project_node->sprint_list_head, sprint_id);
    if (target_sprint == NULL) {
        return -1; // Sprint not found
    }

    Pbi *current_pbi = project_node->pbi_list_head;
    Pbi *prev_pbi = NULL;

    while (current_pbi != NULL && current_pbi->id != pbi_id) {
        prev_pbi = current_pbi;
        current_pbi = current_pbi->next;
    }

    if (current_pbi == NULL) {
        return -1; // PBI not found in project's PBI list
    }

    // Remove PBI from project's PBI list
    if (prev_pbi == NULL) {
        project_node->pbi_list_head = current_pbi->next;
    } else {
        prev_pbi->next = current_pbi->next;
    }

    // Add PBI to sprint's PBI list
    current_pbi->next = NULL; // Ensure it's the last in the sprint's list
    if (target_sprint->pbi_list_head == NULL) {
        target_sprint->pbi_list_head = current_pbi;
    } else {
        Pbi *sprint_current_pbi = target_sprint->pbi_list_head;
        while (sprint_current_pbi->next != NULL) {
            sprint_current_pbi = sprint_current_pbi->next;
        }
        sprint_current_pbi->next = current_pbi;
    }
    target_sprint->total_points += current_pbi->points;
    return 0;
}

// Function: update_sbi_status (Update Sprint Backlog Item status)
// project_list_head: Head of the global list of projects
// data_params: short array { project_id, pbi_id, sprint_id, new_status }
int update_sbi_status(Project *project_list_head, short *data_params) {
    short project_id = data_params[0];
    short pbi_id = data_params[1];
    short sprint_id = data_params[2];
    short new_status = data_params[3];

    Project *project_node = find_project(project_list_head, project_id);
    if (project_node == NULL) {
        return -1; // Project not found
    }

    Sprint *target_sprint = find_sprint(project_node->sprint_list_head, sprint_id);
    if (target_sprint == NULL) {
        return -1; // Sprint not found
    }

    Pbi *target_pbi = find_pbi(target_sprint->pbi_list_head, pbi_id);
    if (target_pbi == NULL) {
        return -1; // PBI not found in sprint
    }

    target_pbi->status = new_status;
    return 0;
}

// Function: update_sbi_points (Update Sprint Backlog Item points)
// project_list_head: Head of the global list of projects
// data_params: short array { project_id, pbi_id, sprint_id, new_points }
int update_sbi_points(Project *project_list_head, short *data_params) {
    short project_id = data_params[0];
    short pbi_id = data_params[1];
    short sprint_id = data_params[2];
    short new_points = data_params[3];

    Project *project_node = find_project(project_list_head, project_id);
    if (project_node == NULL) {
        return -1; // Project not found
    }

    Sprint *target_sprint = find_sprint(project_node->sprint_list_head, sprint_id);
    if (target_sprint == NULL) {
        return -1; // Sprint not found
    }

    Pbi *target_pbi = find_pbi(target_sprint->pbi_list_head, pbi_id);
    if (target_pbi == NULL) {
        return -1; // PBI not found in sprint
    }

    target_sprint->total_points -= target_pbi->points;
    target_sprint->total_points += new_points;
    target_pbi->points = new_points;
    return 0;
}

// Function: update_sbi_description (Update Sprint Backlog Item description)
// project_list_head: Head of the global list of projects
// data_params: short array { project_id, pbi_id, sprint_id, ...description_string_as_shorts... }
int update_sbi_description(Project *project_list_head, short *data_params) {
    short project_id = data_params[0];
    short pbi_id = data_params[1];
    short sprint_id = data_params[2];
    char *new_description_src = (char *)(data_params + 3);

    Project *project_node = find_project(project_list_head, project_id);
    if (project_node == NULL) {
        return -1; // Project not found
    }

    Sprint *target_sprint = find_sprint(project_node->sprint_list_head, sprint_id);
    if (target_sprint == NULL) {
        return -1; // Sprint not found
    }

    Pbi *target_pbi = find_pbi(target_sprint->pbi_list_head, pbi_id);
    if (target_pbi == NULL) {
        return -1; // PBI not found in sprint
    }

    // Free old description and allocate new one
    free(target_pbi->description);
    size_t new_desc_len = strlen(new_description_src);
    target_pbi->description = (char *)calloc(1, new_desc_len + 1);
    if (target_pbi->description == NULL) {
        perror("Failed to allocate new PBI description");
        exit(EXIT_FAILURE);
    }
    strcpy(target_pbi->description, new_description_src);
    return 0;
}

// Function: move_sbi_to_pbl (Move Sprint Backlog Item back to Product Backlog)
// project_list_head: Head of the global list of projects
// data_params: short array { project_id, pbi_id, sprint_id, ... }
int move_sbi_to_pbl(Project *project_list_head, short *data_params) {
    short project_id = data_params[0];
    short pbi_id = data_params[1];
    short sprint_id = data_params[2];

    Project *project_node = find_project(project_list_head, project_id);
    if (project_node == NULL) {
        return -1; // Project not found
    }

    Sprint *source_sprint = find_sprint(project_node->sprint_list_head, sprint_id);
    if (source_sprint == NULL) {
        return -1; // Sprint not found
    }

    Pbi *current_pbi = source_sprint->pbi_list_head;
    Pbi *prev_pbi = NULL;

    while (current_pbi != NULL && current_pbi->id != pbi_id) {
        prev_pbi = current_pbi;
        current_pbi = current_pbi->next;
    }

    if (current_pbi == NULL) {
        return -1; // PBI not found in sprint
    }

    // Remove PBI from sprint's PBI list
    if (prev_pbi == NULL) {
        source_sprint->pbi_list_head = current_pbi->next;
    } else {
        prev_pbi->next = current_pbi->next;
    }
    source_sprint->total_points -= current_pbi->points;

    // Add PBI to project's PBI list
    current_pbi->next = NULL; // Ensure it's the last in the project's list
    if (project_node->pbi_list_head == NULL) {
        project_node->pbi_list_head = current_pbi;
    } else {
        Pbi *project_current_pbi = project_node->pbi_list_head;
        while (project_current_pbi->next != NULL) {
            project_current_pbi = project_current_pbi->next;
        }
        project_current_pbi->next = current_pbi;
    }
    return 0;
}

// Minimal main function for compilation and demonstration
int main() {
    Project *project_list = NULL;

    // Create a dummy project
    Project *proj1 = (Project *)calloc(1, sizeof(Project));
    if (proj1 == NULL) {
        perror("Failed to allocate project");
        return EXIT_FAILURE;
    }
    proj1->id = 100;
    proj1->pbi_list_head = NULL;
    proj1->sprint_list_head = NULL;
    proj1->next = NULL;
    project_list = proj1;

    // Create a dummy sprint for proj1
    Sprint *sprint1_proj1 = (Sprint *)calloc(1, sizeof(Sprint));
    if (sprint1_proj1 == NULL) {
        perror("Failed to allocate sprint");
        return EXIT_FAILURE;
    }
    sprint1_proj1->id = 200;
    sprint1_proj1->total_points = 0;
    sprint1_proj1->pbi_list_head = NULL;
    sprint1_proj1->next = NULL;
    proj1->sprint_list_head = sprint1_proj1;

    printf("--- Creating PBI ---\n");
    // Project 100, PBI 1, Points 5, Desc "Desc1"
    short create_params[] = {100, 1, 5, 'D', 'e', 's', 'c', '1', '\0'};
    if (create_pbi(project_list, create_params) == 0) {
        printf("PBI 1 created successfully in Project 100.\n");
    } else {
        printf("Failed to create PBI 1.\n");
    }

    printf("--- Moving PBI to Sprint ---\n");
    // Project 100, PBI 1, Sprint 200
    short move_params[] = {100, 1, 200};
    if (move_pbi_to_sprint(project_list, move_params) == 0) {
        printf("PBI 1 moved to Sprint 200 successfully. Sprint Total Points: %d\n", sprint1_proj1->total_points);
    } else {
        printf("Failed to move PBI 1 to Sprint 200.\n");
    }

    printf("--- Update SBI Status ---\n");
    // Project 100, PBI 1, Sprint 200, Status 1
    short update_status_params[] = {100, 1, 200, 1};
    if (update_sbi_status(project_list, update_status_params) == 0) {
        printf("SBI 1 status updated to 1 in Sprint 200.\n");
    } else {
        printf("Failed to update SBI 1 status.\n");
    }

    printf("--- Update SBI Points ---\n");
    // Project 100, PBI 1, Sprint 200, New Points 8
    short update_points_params[] = {100, 1, 200, 8};
    if (update_sbi_points(project_list, update_points_params) == 0) {
        printf("SBI 1 points updated to 8 in Sprint 200. New sprint total: %d\n", sprint1_proj1->total_points);
    } else {
        printf("Failed to update SBI 1 points.\n");
    }

    printf("--- Update SBI Description ---\n");
    // Project 100, PBI 1, Sprint 200, New Desc "NewDesc"
    short update_desc_params[] = {100, 1, 200, 'N', 'e', 'w', 'D', 'e', 's', 'c', '\0'};
    if (update_sbi_description(project_list, update_desc_params) == 0) {
        printf("SBI 1 description updated to '%s' in Sprint 200.\n", sprint1_proj1->pbi_list_head->description);
    } else {
        printf("Failed to update SBI 1 description.\n");
    }
    
    printf("--- Move SBI to PBL ---\n");
    // Project 100, PBI 1, Sprint 200
    short move_to_pbl_params[] = {100, 1, 200};
    if (move_sbi_to_pbl(project_list, move_to_pbl_params) == 0) {
        printf("SBI 1 moved back to Project 100 PBL. New sprint total: %d\n", sprint1_proj1->total_points);
    } else {
        printf("Failed to move SBI 1 back to PBL.\n");
    }

    printf("--- Delete PBI ---\n");
    // Project 100, PBI 1
    short delete_params[] = {100, 1};
    if (delete_pbi(project_list, delete_params) == 0) {
        printf("PBI 1 deleted successfully from Project 100.\n");
    } else {
        printf("Failed to delete PBI 1.\n");
    }
    
    // Basic cleanup of allocated memory for the example
    // In a full application, a comprehensive cleanup function would iterate and free all nodes.
    if (sprint1_proj1->pbi_list_head != NULL) {
        // This should not happen if move_sbi_to_pbl and delete_pbi worked correctly for PBI 1
        free(sprint1_proj1->pbi_list_head->description);
        free(sprint1_proj1->pbi_list_head);
    }
    free(sprint1_proj1);
    
    if (proj1->pbi_list_head != NULL) {
        // This should not happen if delete_pbi worked correctly for PBI 1
        free(proj1->pbi_list_head->description);
        free(proj1->pbi_list_head);
    }
    free(proj1);

    return EXIT_SUCCESS;
}