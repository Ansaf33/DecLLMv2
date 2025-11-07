#include <stdio.h>   // For vfprintf, stdout
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For strdup
#include <stdarg.h>  // For va_list, va_start, va_end
#include <stdint.h>  // For fixed-width integer types like intptr_t (not directly used but good practice)

// --- Custom types and declarations for compilation ---
// Assuming a 32-bit environment based on the offsets and malloc size (0x24 = 36 bytes)
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef char int8;
typedef unsigned char uint8;

// Placeholder for fdprintf. In a real system, this might be dprintf or fprintf.
// The string IDs (e.g., 0x1002f) are likely offsets into a string table.
// For compilation, we'll use placeholder strings.
int fdprintf(int fd, int string_id, ...) {
    va_list args;
    va_start(args, string_id);
    const char *format_str;
    switch (string_id) {
        case 0x1002f: format_str = "Error: Course pointer is NULL.\n"; break;
        case 0x1003b: format_str = "Course ID: %d, Professor: %p, Department: %p, Number: %d, Year: %d, Start Time: %d, End Time: %d, Days: %s, Campus: %c, Building: %c, Room: %c, Capacity: %c, Name: %s\n"; break;
        case 0x10073: format_str = "----------------------------------------\n"; break;
        case 0x102bc: format_str = "Error: Failed to add course to list.\n"; break;
        case 0x106b5: format_str = "--- Course List ---\nID\tPROF\tDEPT\tNUM\tYEAR\tSTART\tEND\tDAYS\tCAMP\tBLDG\tROOM\tCAP\tNAME\n"; break;
        case 0x10727: format_str = "%d.\t"; break;
        // 0x102e6 is a value assigned, not a format string ID for fdprintf.
        default: format_str = "Unknown format string ID: %x\n"; break;
    }
    int ret = vfprintf(fd == 1 ? stdout : stderr, format_str, args);
    va_end(args);
    return ret;
}

// Forward declaration for Course struct to define function pointer
struct Course;

// Define the Course structure based on the inferred layout (32-bit pointers assumed)
typedef struct Course {
    int32 id;             // 0x00
    void *professor_ptr;  // 0x04
    void *department_ptr; // 0x08
    int16 num;            // 0x0c
    int16 year;           // 0x0e
    int16 start_time;     // 0x10
    int16 end_time;       // 0x12
    int32 days_mask;      // 0x14
    int8 campus;          // 0x18
    int8 building;        // 0x19
    int8 room;            // 0x1a
    int8 capacity;        // 0x1b
    char *name_ptr;       // 0x1c
    void (*print_func)(struct Course *); // 0x20 - Function pointer for printing
} Course;

// Dummy declarations for external functions
// These would be defined elsewhere in a real program.
void *create_ptrlist() {
    // Dummy implementation: returns a pointer to a small allocated block
    // to simulate a list head that stores at least a count.
    void *list = malloc(sizeof(void*) + sizeof(uint32));
    if (list) {
        *(uint32*)((char*)list + sizeof(void*)) = 0; // Initialize count to 0
    }
    return list;
}
void *get_pdata(void *list, uint32 index) { (void)list; (void)index; return NULL; } // Dummy
int add_item(void *list, void *item) { (void)list; (void)item; return 0; } // Dummy
void *get_professor(int id) { (void)id; return (void*)(intptr_t)id; } // Dummy: return id as a pointer
void *get_department(int id) { (void)id; return (void*)(intptr_t)id; } // Dummy: return id as a pointer
void add_course_to_professor(void *prof, void *course) { (void)prof; (void)course; } // Dummy
void add_course_to_department(void *dept, void *course) { (void)dept; (void)course; } // Dummy
void daystostr(int32 days_mask, char *buffer) { sprintf(buffer, "DAYS_%X", days_mask); } // Dummy
int days_intersect(int32 days1, int32 days2) { return (days1 & days2) != 0; } // Dummy
int timecmp(int16 time1, int16 time2) { return (time1 < time2) ? -1 : ((time1 == time2) ? 0 : 1); } // Dummy

// Global variables, assuming they are pointers to list heads or similar structures.
static void *iRam00010147 = NULL; // Likely a list of courses
static void *iRam00010276 = NULL; // Another list of courses
static void *iRam00010451 = NULL; // Yet another list of courses

// Forward declarations for functions used before definition
void print_course_list(void *course_list_ptr);
void add_course(Course *course);

// Function: _print_course
void _print_course(Course *course) {
  char days_str[21];
  
  if (course == NULL) {
    fdprintf(1, 0x1002f);
  } else {
    daystostr(course->days_mask, days_str);
    fdprintf(1, 0x1003b, course->id, course->professor_ptr, course->department_ptr,
             course->num, course->year, course->start_time, course->end_time,
             days_str, course->campus, course->building, course->room,
             course->capacity, course->name_ptr);
    fdprintf(1, 0x10073);
  }
}

// Function: find_courses_by_id
void *find_courses_by_id(int id) {
  void *course_list = NULL;
  if (iRam00010147 != NULL) {
    course_list = create_ptrlist();
    // Assuming *(uint32 *)((char *)iRam00010147 + 4) is the count in the list structure
    for (uint32 i = 0; i < *(uint32 *)((char *)iRam00010147 + 4); ++i) {
      Course *c = (Course *)get_pdata(iRam00010147, i);
      if (c != NULL && c->id == id) {
        add_item(course_list, c);
      }
    }
  }
  return course_list;
}

// Function: find_courses_by_num
void *find_courses_by_num(int dept_id, int16 course_num) {
  void *result_list = NULL;
  void *department_ptr = get_department(dept_id);
  if (department_ptr != NULL) {
    result_list = create_ptrlist();
    // Assuming department_ptr + 4 points to another list structure,
    // and its +4 offset contains the count.
    void *dept_courses_list = *(void **)((char *)department_ptr + 4); // Assuming 32-bit pointers
    if (dept_courses_list != NULL) {
        for (uint32 i = 0; i < *(uint32 *)((char *)dept_courses_list + 4); ++i) {
            Course *c = (Course *)get_pdata(dept_courses_list, i);
            if (c != NULL && c->num == course_num) {
                add_item(result_list, c);
            }
        }
    }
  }
  return result_list;
}

// Function: add_course
void add_course(Course *course) {
  if (course != NULL) {
    if (iRam00010276 == NULL) {
      iRam00010276 = create_ptrlist();
    }
    if (add_item(iRam00010276, course) != 0) {
      fdprintf(1, 0x102bc);
      exit(1);
    }
  }
}

// Function: create_course
Course *create_course(int32 id, int prof_id, int dept_id, int16 num, int16 year,
                      int16 start_time, int16 end_time, int32 days_mask,
                      uint16 packed_campus_building, uint16 packed_room_capacity, char *name) {
  if (prof_id == 0 || dept_id == 0 || name == NULL) {
    return NULL;
  }

  void *professor_ptr = get_professor(prof_id);
  if (professor_ptr == NULL) {
    return NULL;
  }

  void *department_ptr = get_department(dept_id);
  if (department_ptr == NULL) {
    return NULL;
  }

  Course *course = (Course *)malloc(sizeof(Course));
  if (course == NULL) {
    return NULL;
  }

  course->id = id;
  course->professor_ptr = professor_ptr;
  course->department_ptr = department_ptr;
  course->num = num;
  course->year = year;
  course->start_time = start_time;
  course->end_time = end_time;
  course->days_mask = days_mask;
  
  // Assuming little-endian packing for the char fields based on original `undefined2` assignments
  course->campus = (int8)(packed_campus_building & 0xFF);
  course->building = (int8)((packed_campus_building >> 8) & 0xFF);
  course->room = (int8)(packed_room_capacity & 0xFF);
  course->capacity = (int8)((packed_room_capacity >> 8) & 0xFF);

  course->name_ptr = strdup(name);
  if (course->name_ptr == NULL) {
      free(course);
      return NULL;
  }
  course->print_func = _print_course; // Assign the actual print function

  add_course(course);
  add_course_to_professor(professor_ptr, course);
  add_course_to_department(department_ptr, course);
  return course;
}

// Function: get_courses
void *get_courses(int id) {
  void *course_list = NULL;
  if (iRam00010451 != NULL) {
    course_list = create_ptrlist();
    // Corrected to iterate over iRam00010451, not the newly created empty list
    for (uint32 i = 0; i < *(uint32 *)((char *)iRam00010451 + 4); ++i) {
      Course *c = (Course *)get_pdata(iRam00010451, i);
      if (c != NULL && c->id == id) {
        add_item(course_list, c);
      }
    }
  }
  return course_list;
}

// Function: select_course_id
Course *select_course_id(int id, uint32 index) {
  Course *selected_course = NULL;
  void *found_courses = find_courses_by_id(id);
  if (found_courses != NULL) {
    if (index < *(uint32 *)((char *)found_courses + 4)) {
      selected_course = (Course *)get_pdata(found_courses, index);
    }
    free(found_courses);
  }
  return selected_course;
}

// Function: select_course_num
Course *select_course_num(int dept_id, int16 course_num, uint32 index) {
  Course *selected_course = NULL;
  void *found_courses = find_courses_by_num(dept_id, course_num);
  if (found_courses != NULL) {
    if (index < *(uint32 *)((char *)found_courses + 4)) {
      selected_course = (Course *)get_pdata(found_courses, index);
    }
    free(found_courses);
  }
  return selected_course;
}

// Function: list_courses_by_num
void list_courses_by_num(int dept_id, int16 course_num) {
  void *found_courses = find_courses_by_num(dept_id, course_num);
  print_course_list(found_courses);
  if (found_courses != NULL) {
    free(found_courses);
  }
}

// Function: list_courses_by_id
void list_courses_by_id(int id) {
  void *found_courses = find_courses_by_id(id);
  print_course_list(found_courses);
  if (found_courses != NULL) {
    free(found_courses);
  }
}

// Function: print_course_banner
void print_course_banner(void) {
  fdprintf(1, 0x106b5);
}

// Function: print_course_list
void print_course_list(void *course_list_ptr) {
  if (course_list_ptr != NULL) {
    print_course_banner();
    for (uint32 i = 0; i < *(uint32 *)((char *)course_list_ptr + 4); ++i) {
      Course *c = (Course *)get_pdata(course_list_ptr, i);
      if (c != NULL) {
          fdprintf(1, 0x10727, i + 1);
          if (c->print_func != NULL) {
              c->print_func(c);
          } else {
              // Fallback print if function pointer is not set
              char days_str[21];
              daystostr(c->days_mask, days_str);
              fdprintf(1, 0x1003b, c->id, c->professor_ptr, c->department_ptr,
                       c->num, c->year, c->start_time, c->end_time,
                       days_str, c->campus, c->building, c->room,
                       c->capacity, c->name_ptr);
              fdprintf(1, 0x10073);
          }
      }
    }
  }
}

// Function: check_time_conflict
int check_time_conflict(Course *c1, Course *c2) {
  if (days_intersect(c1->days_mask, c2->days_mask)) {
    // Conflict if c1's time slot is entirely contained within c2's
    if (timecmp(c1->start_time, c2->start_time) >= 0 &&
        timecmp(c1->end_time, c2->end_time) <= 0) {
      return -1; // Conflict
    }
    // Conflict if c2's time slot is entirely contained within c1's
    if (timecmp(c2->start_time, c1->start_time) >= 0 &&
        timecmp(c2->end_time, c1->end_time) <= 0) {
      return -1; // Conflict
    }
  }
  return 0; // No conflict based on this specific "contained" logic
}