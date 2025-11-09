#include <stdio.h>    // For dprintf, fprintf, exit, va_list, va_start, va_end
#include <stdlib.h>   // For malloc, free, exit, EXIT_FAILURE
#include <string.h>   // For strdup, strcpy, strcat, strlen
#include <unistd.h>   // For dprintf (on Linux systems, this typically provides dprintf)
#include <stdint.h>   // For intptr_t

// Define custom types based on the original snippet's 'undefined' types
// Assuming a 32-bit environment where pointers are 4 bytes, consistent with malloc(0x24) for Course.
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

// Forward declarations for custom functions (stubs)
// In a real system, these would be implemented elsewhere.
int fdprintf(int fd, const char* format, ...);
void daystostr(uint days_bitmap, char* buffer);
void* create_ptrlist(void);
void* get_pdata(void* list, uint index);
int add_item(void* list, void* item);
void* get_department(uint dept_id);
void* get_professor(uint prof_id);
void add_course_to_professor(void* prof, void* course);
void add_course_to_department(void* dept, void* course);
int days_intersect(uint days1, uint days2);
int timecmp(ushort time1, ushort time2);
// Function to print a course list, declared here to resolve circular dependency
void print_course_list(void* param_1);

// Global variables, assumed to be pointers to ptrlist structures or similar.
// Renamed from iRamXXXX to more descriptive names.
static void* s_all_courses_master_list = NULL; // Corresponds to iRam00010147 and iRam00010451
static void* s_global_course_registration_list = NULL; // Corresponds to iRam00010276

// Structure definition for Course based on memory offsets and sizes in the original code.
// This structure is designed for a 32-bit environment where char* is 4 bytes.
typedef struct Course {
    uint id;             // offset 0x00
    uint professor_id;   // offset 0x04
    uint department_id;  // offset 0x08
    ushort course_num;   // offset 0x0c
    ushort section;      // offset 0x0e
    ushort credits;      // offset 0x10
    ushort max_students; // offset 0x12
    uint days_bitmap;    // offset 0x14
    uchar start_hour;    // offset 0x18
    uchar start_minute;  // offset 0x19
    uchar end_hour;      // offset 0x1a
    uchar end_minute;    // offset 0x1b
    char* description;   // offset 0x1c (pointer, 4 bytes on 32-bit)
    void* print_func_ptr; // offset 0x20 (function pointer, 4 bytes on 32-bit)
} Course; // Total size: 0x24 (36 bytes)

// --- Stubs for external/unprovided functions ---
// These are minimal implementations to allow the provided code to compile.
// In a real application, these would have proper logic.

// A simple dprintf wrapper using fprintf for stderr (fd 2) or stdout (fd 1)
int fdprintf(int fd, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret;
    if (fd == 1) {
        ret = vfprintf(stdout, format, args);
    } else { // Assume other fds go to stderr for this stub
        ret = vfprintf(stderr, format, args);
    }
    va_end(args);
    return ret;
}

// Placeholder for converting days bitmap to string
void daystostr(uint days_bitmap, char* buffer) {
    strcpy(buffer, "");
    if (days_bitmap & 0x01) strcat(buffer, "Sun");
    if (days_bitmap & 0x02) strcat(buffer, "Mon");
    if (days_bitmap & 0x04) strcat(buffer, "Tue");
    if (days_bitmap & 0x08) strcat(buffer, "Wed");
    if (days_bitmap & 0x10) strcat(buffer, "Thu");
    if (days_bitmap & 0x20) strcat(buffer, "Fri");
    if (days_bitmap & 0x40) strcat(buffer, "Sat");
    if (strlen(buffer) == 0) strcpy(buffer, "N/A");
}

// Simple dynamic array / list for pointers.
// In a real system, this would be more robust.
typedef struct PtrList {
    void** items;
    uint count;
    uint capacity;
} PtrList;

void* create_ptrlist(void) {
    PtrList* list = (PtrList*)malloc(sizeof(PtrList));
    if (!list) {
        perror("Failed to create ptrlist");
        exit(EXIT_FAILURE);
    }
    list->count = 0;
    list->capacity = 10; // Initial capacity
    list->items = (void**)malloc(sizeof(void*) * list->capacity);
    if (!list->items) {
        perror("Failed to allocate ptrlist items");
        free(list);
        exit(EXIT_FAILURE);
    }
    return list;
}

void* get_pdata(void* list_ptr, uint index) {
    PtrList* list = (PtrList*)list_ptr;
    if (!list || index >= list->count) {
        return NULL; // Or handle error appropriately
    }
    return list->items[index];
}

int add_item(void* list_ptr, void* item) {
    PtrList* list = (PtrList*)list_ptr;
    if (!list) return -1;

    if (list->count == list->capacity) {
        list->capacity *= 2;
        void** new_items = (void**)realloc(list->items, sizeof(void*) * list->capacity);
        if (!new_items) {
            perror("Failed to reallocate ptrlist items");
            return -1;
        }
        list->items = new_items;
    }
    list->items[list->count++] = item;
    return 0; // Success
}

// Dummy functions for professor and department
// These use static arrays to simulate basic objects with a list pointer at offset 4.
void* get_department(uint dept_id) {
    static int dummy_dept_data[2] = {1, 0}; // {id, courses_list_ptr}
    if (dept_id == 123) {
        if (dummy_dept_data[1] == 0) {
            dummy_dept_data[1] = (int)(intptr_t)create_ptrlist();
        }
        return &dummy_dept_data;
    }
    return NULL;
}

void* get_professor(uint prof_id) {
    static int dummy_prof_data[2] = {1, 0}; // {id, courses_list_ptr}
    if (prof_id == 456) {
        if (dummy_prof_data[1] == 0) {
            dummy_prof_data[1] = (int)(intptr_t)create_ptrlist();
        }
        return &dummy_prof_data;
    }
    return NULL;
}

void add_course_to_professor(void* prof, void* course) {
    int* prof_data = (int*)prof;
    if (prof_data && prof_data[1]) {
        add_item((void*)(intptr_t)prof_data[1], course);
    }
}

void add_course_to_department(void* dept, void* course) {
    int* dept_data = (int*)dept;
    if (dept_data && dept_data[1]) {
        add_item((void*)(intptr_t)dept_data[1], course);
    }
}

int days_intersect(uint days1, uint days2) {
    return (days1 & days2) != 0;
}

int timecmp(ushort time1, ushort time2) {
    // Assuming time is packed as (hour | (minute << 8))
    // Returns 1 if time1 > time2, 0 if time1 == time2, -1 if time1 < time2
    if (time1 > time2) return 1;
    if (time1 < time2) return -1;
    return 0;
}
// --- End of stubs ---


// Function: _print_course
void _print_course(Course *course) {
  char days_str[21];
  
  if (course == NULL) {
    fdprintf(1, "Error: Course pointer is NULL.\n");
  } else {
    daystostr(course->days_bitmap, days_str);
    fdprintf(1, "Course ID: %u | Prof ID: %u | Dept ID: %u | Num: %hu | Sec: %hu | Crd: %hu | Max: %hu | Days: %s | Start: %02hhu:%02hhu | End: %02hhu:%02hhu | Description: %s\n",
             course->id,
             course->professor_id,
             course->department_id,
             course->course_num,
             course->section,
             course->credits,
             course->max_students,
             days_str,
             course->start_hour,
             course->start_minute,
             course->end_hour,
             course->end_minute,
             course->description);
    fdprintf(1, "------------------------------------------------------------------------------------------------------------------\n");
  }
  return;
}

// Function: find_courses_by_id
void* find_courses_by_id(uint course_id) {
  if (s_all_courses_master_list == NULL) {
    return NULL;
  }
  
  void* result_list = create_ptrlist();
  PtrList* master_list = (PtrList*)s_all_courses_master_list;
  
  for (uint i = 0; i < master_list->count; ++i) {
    Course* course_ptr = (Course*)get_pdata(master_list, i);
    if (course_ptr != NULL && course_ptr->id == course_id) {
      add_item(result_list, course_ptr);
    }
  }
  return result_list;
}

// Function: find_courses_by_num
void* find_courses_by_num(uint department_id, ushort course_num) {
  void* department_ptr = get_department(department_id);
  if (department_ptr == NULL) {
    return NULL;
  }
  
  void* result_list = create_ptrlist();
  // Assuming department_ptr points to a struct where the courses list pointer is at offset +4
  // and that courses list pointer points to a PtrList struct.
  PtrList* dept_courses_list = (PtrList*)(*(int*)((char*)department_ptr + 4));
  
  if (dept_courses_list == NULL) {
      return result_list; // Return empty list
  }

  for (uint i = 0; i < dept_courses_list->count; ++i) {
    Course* course_ptr = (Course*)get_pdata(dept_courses_list, i);
    if (course_ptr != NULL && course_ptr->course_num == course_num) {
      add_item(result_list, course_ptr);
    }
  }
  return result_list;
}

// Function: add_course
void add_course(Course* course_to_add) {
  if (course_to_add != NULL) {
    if (s_global_course_registration_list == NULL) {
      s_global_course_registration_list = create_ptrlist();
    }
    if (add_item(s_global_course_registration_list, course_to_add) != 0) {
      fdprintf(1, "Error: Failed to add course to global list. Exiting.\n");
      exit(EXIT_FAILURE);
    }
  }
  return;
}

// Function: create_course
Course* create_course(uint id, uint professor_id, uint department_id, ushort course_num, ushort section,
             ushort credits, ushort max_students, uint days_bitmap, ushort start_time_packed,
             ushort end_time_packed, char *description_str) {
  
  if (professor_id == 0 || department_id == 0 || description_str == NULL) {
    return NULL;
  }
  
  void* professor_ptr = get_professor(professor_id);
  if (professor_ptr == NULL) {
    return NULL;
  }
  
  void* department_ptr = get_department(department_id);
  if (department_ptr == NULL) {
    return NULL;
  }
  
  Course* new_course = (Course*)malloc(sizeof(Course));
  if (new_course == NULL) {
    return NULL;
  }
  
  // Initialize Course fields
  new_course->id = id;
  // In a real system, these would likely be pointers to actual Professor/Department objects,
  // or just the IDs if the objects are managed elsewhere.
  // For this translation, we store the ID or a dummy pointer.
  new_course->professor_id = professor_id; 
  new_course->department_id = department_id;
  new_course->course_num = course_num;
  new_course->section = section;
  new_course->credits = credits;
  new_course->max_students = max_students;
  new_course->days_bitmap = days_bitmap;
  
  // Unpack start_time_packed (hour | (minute << 8))
  new_course->start_hour = (uchar)(start_time_packed & 0xFF);
  new_course->start_minute = (uchar)((start_time_packed >> 8) & 0xFF);
  
  // Unpack end_time_packed (hour | (minute << 8))
  new_course->end_hour = (uchar)(end_time_packed & 0xFF);
  new_course->end_minute = (uchar)((end_time_packed >> 8) & 0xFF);
  
  char* duplicated_description = strdup(description_str);
  if (duplicated_description == NULL) {
      free(new_course);
      return NULL;
  }
  new_course->description = duplicated_description;
  
  new_course->print_func_ptr = (void*)(intptr_t)_print_course;
  
  // Add to global list of all courses (if not already handled by department/professor lists)
  if (s_all_courses_master_list == NULL) {
      s_all_courses_master_list = create_ptrlist();
  }
  add_item(s_all_courses_master_list, new_course);

  add_course(new_course); // Adds to s_global_course_registration_list
  add_course_to_professor(professor_ptr, new_course);
  add_course_to_department(department_ptr, new_course);
  
  return new_course;
}

// Function: get_courses
// This function appears to be a duplicate of find_courses_by_id.
// It searches for courses by ID from the s_all_courses_master_list.
void* get_courses(uint course_id) {
  if (s_all_courses_master_list == NULL) {
    return NULL;
  }
  
  void* result_list = create_ptrlist();
  PtrList* master_list = (PtrList*)s_all_courses_master_list;
  
  for (uint i = 0; i < master_list->count; ++i) {
    Course* course_ptr = (Course*)get_pdata(master_list, i);
    if (course_ptr != NULL && course_ptr->id == course_id) {
      add_item(result_list, course_ptr);
    }
  }
  return result_list;
}

// Function: select_course_id
void* select_course_id(uint course_id, uint index) {
  void* selected_course = NULL;
  void* course_list = find_courses_by_id(course_id);
  
  if (course_list != NULL) {
    PtrList* list = (PtrList*)course_list;
    if (index < list->count) {
      selected_course = get_pdata(list, index);
    }
    free(list->items);
    free(list);
  }
  return selected_course;
}

// Function: select_course_num
void* select_course_num(uint department_id, ushort course_num, uint index) {
  void* selected_course = NULL;
  void* course_list = find_courses_by_num(department_id, course_num);
  
  if (course_list != NULL) {
    PtrList* list = (PtrList*)course_list;
    if (index < list->count) {
      selected_course = get_pdata(list, index);
    }
    free(list->items);
    free(list);
  }
  return selected_course;
}

// Function: list_courses_by_num
void list_courses_by_num(uint department_id, ushort course_num) {
  void* course_list = find_courses_by_num(department_id, course_num);
  print_course_list(course_list);
  if (course_list != NULL) {
    PtrList* list = (PtrList*)course_list;
    free(list->items);
    free(list);
  }
  return;
}

// Function: list_courses_by_id
void list_courses_by_id(uint course_id) {
  void* course_list = find_courses_by_id(course_id);
  print_course_list(course_list);
  if (course_list != NULL) {
    PtrList* list = (PtrList*)course_list;
    free(list->items);
    free(list);
  }
  return;
}

// Function: print_course_banner
void print_course_banner(void) {
  fdprintf(1, "Course ID | Prof ID | Dept ID | Num | Sec | Crd | Max | Days | Start  | End    | Description\n");
  return;
}

// Function: print_course_list
void print_course_list(void* course_list_ptr) {
  if (course_list_ptr != NULL) {
    print_course_banner();
    PtrList* list = (PtrList*)course_list_ptr;
    for (uint i = 0; i < list->count; ++i) {
      Course* course_ptr = (Course*)get_pdata(list, i);
      fdprintf(1, "%2u. ", i + 1);
      if (course_ptr != NULL && course_ptr->print_func_ptr != NULL) {
          ((void (*)(Course*))course_ptr->print_func_ptr)(course_ptr);
      }
    }
  }
  return;
}

// Function: check_time_conflict
uint check_time_conflict(Course* course1, Course* course2) {
    // Assuming return 0xffffffff (UINT_MAX) means NO CONFLICT, and 0 means CONFLICT.
    // This is a direct translation of the original logic.

    ushort course1_start_time_packed = (ushort)(course1->start_hour | (course1->start_minute << 8));
    ushort course1_end_time_packed = (ushort)(course1->end_hour | (course1->end_minute << 8));
    ushort course2_start_time_packed = (ushort)(course2->start_hour | (course2->start_minute << 8));
    ushort course2_end_time_packed = (ushort)(course2->end_hour | (course2->end_minute << 8));

    if (days_intersect(course1->days_bitmap, course2->days_bitmap) != 0) {
        int time_status;

        // Check if course1 is entirely after course2 (or touching)
        // Condition: (course1_start_time >= course2_end_time) && (course1_end_time <= course2_start_time)
        time_status = timecmp(course1_start_time_packed, course2_end_time_packed);
        if (time_status >= 0) { // course1_start_time >= course2_end_time
            time_status = timecmp(course1_end_time_packed, course2_start_time_packed);
            if (time_status <= 0) { // course1_end_time <= course2_start_time
                return 0xffffffff; // No conflict
            }
        }

        // Check if course2 is entirely after course1 (or touching)
        // Condition: (course2_start_time >= course1_end_time) && (course2_end_time <= course1_start_time)
        time_status = timecmp(course2_start_time_packed, course1_end_time_packed);
        if (time_status >= 0) { // course2_start_time >= course1_end_time
            time_status = timecmp(course2_end_time_packed, course1_start_time_packed);
            if (time_status <= 0) { // course2_end_time <= course1_start_time
                return 0xffffffff; // No conflict
            }
        }
    }
    return 0; // Conflict (if days intersect and no non-overlapping time condition was met)
}