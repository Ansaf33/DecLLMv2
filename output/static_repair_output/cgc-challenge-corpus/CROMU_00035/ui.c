#include <stdio.h>    // For printf, puts
#include <stdlib.h>   // For atoi, malloc, free
#include <string.h>   // For strlen, strcmp, strncpy, memset, strcpy
#include <unistd.h>   // For write, ssize_t, STDOUT_FILENO
#include <stdbool.h>  // For bool type

// External function declarations (assuming these are defined elsewhere)
// These are placeholders for actual struct pointers.
typedef void SystemState;
typedef void Node;
typedef void User;
typedef void Group;
typedef void Permission;
typedef void FileDataChunk; // For file content linked list

// Specific external functions related to file system logic
extern void *mallocOrDie(size_t size, const char *msg);
extern void die(const char *msg);
extern int receive_until(char *buf, size_t max_len, int terminator);
extern int receive_bytes(void *buf, size_t num_bytes);
extern void str_of_path(char *buf, SystemState *sys_state, Node *current_dir);
extern void validate_current_perms(Node *node, SystemState *sys_state);
extern unsigned int get_file_size(FileDataChunk *file_data_head);
extern Group *find_group_by_number(int num, SystemState *sys_state);
extern Group *find_group_by_name(const char *name, SystemState *sys_state);
extern User *find_user_by_number(int num, SystemState *sys_state);
extern User *find_user_by_name(const char *name, SystemState *sys_state);
extern User *add_user(const char *username, SystemState *sys_state);
extern Group *add_group(const char *groupname, SystemState *sys_state);
extern Node *find_node_by_name(const char *name, Node *parent_dir_children_list);
extern Node *add_file(Node *current_dir, unsigned int date, unsigned int size, const char *filename, void *content_buf, User *user_ptr);
extern Node *add_directory(unsigned int date, const char *dirname, Node *parent_dir, User *user_ptr);
extern int is_user_in_group(User *user_ptr, Group *group_ptr); // Returns bool
extern void add_user_to_group(User *user_ptr, Group *group_ptr);
extern Permission *add_perm(User *user_ptr, Group *group_ptr, Node *node_ptr);
extern FileDataChunk *find_file_by_name(const char *filename, Node *current_dir); // Returns head of file data chunks
extern Node *find_file_node_by_name(const char *filename, Node *current_dir); // Returns the file node itself
extern FileDataChunk *add_file_chunk(void *content_buf, FileDataChunk *file_data_head, unsigned int size);
extern void remove_user(User *user_ptr, SystemState *sys_state);
extern void remove_group(Group *group_ptr, SystemState *sys_state);
extern Node *find_directory_by_name(const char *dirname, Node *current_dir);
extern void delete_node(Node *node_ptr, SystemState *sys_state);
extern void remove_user_from_group(User *user_ptr, Group *group_ptr);
extern Permission *find_perm_by_name(const char *name, Node *node_ptr, SystemState *sys_state);
extern void delete_perms(Node *node_ptr, Permission *perm_ptr);
extern void delete_file_bytes(FileDataChunk *file_data_head, unsigned int new_size);


// Function: strict_atoi
int strict_atoi(char *s) {
  for (size_t i = 0; s[i] != '\0'; i++) {
    if ((s[i] < '0') || (s[i] > '9')) {
      return 0;
    }
  }
  return atoi(s);
}

// Function: get_int
int get_int(const char *prompt) {
  char input_buffer[10];
  printf("%s :", prompt);
  int len = receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
  if (len < 0) return 0;
  input_buffer[len] = '\0';

  for (int i = 0; i < len; i++) {
    if ((input_buffer[i] < '0') || (input_buffer[i] > '9')) {
      input_buffer[i] = '0';
    }
  }
  return atoi(input_buffer);
}

// Function: get_string
int get_string(char *buf, size_t size, const char *prompt) {
  printf("%s >", prompt);
  int len = receive_until(buf, size - 1, '\n');
  if (len < 0) return 0;
  buf[len] = '\0';
  return len;
}

// Function: print_prompt
void print_prompt(SystemState *sys_state) {
  char path_buffer[4004];
  memset(path_buffer, 0, sizeof(path_buffer));
  str_of_path(path_buffer, sys_state, *(Node **)((char *)sys_state + 4));
  printf("%s@@SPIFF:%s/", *(char **)(*(User **)((char *)sys_state + 8) + 8), path_buffer);
}

// Function: print_help
void print_help(const char *const *commands, int count) {
  printf("< ");
  for (int i = 0; i < count - 1; i++) {
    printf("%s, ", commands[i]);
  }
  printf("%s >\n", commands[count - 1]);
}

// Function: parse_arg
int parse_arg(char args[16][256], char *input_string) {
  int arg_count = 0;
  int current_arg_start = 0;
  bool in_space = true;

  for (int i = 0; input_string[i] != '\0'; i++) {
    if (input_string[i] == ' ') {
      if (!in_space) {
        if (arg_count < 16) { // Ensure we don't write past array bounds
          memset(args[arg_count], 0, 256);
          strncpy(args[arg_count], input_string + current_arg_start, i - current_arg_start);
          args[arg_count][i - current_arg_start] = '\0';
          arg_count++;
        } else {
          break; // Max arguments reached
        }
      }
      in_space = true;
    } else {
      if (in_space) {
        current_arg_start = i;
      }
      in_space = false;
    }
  }

  if (!in_space && arg_count < 16) {
    memset(args[arg_count], 0, 256);
    strncpy(args[arg_count], input_string + current_arg_start, strlen(input_string) - current_arg_start);
    args[arg_count][strlen(input_string) - current_arg_start] = '\0';
    arg_count++;
  }
  return arg_count;
}

// Function: print_perms
void print_perms(Node *node, SystemState *sys_state) {
  Permission *current_perm = *(Permission **)((char *)node + 0x14);
  validate_current_perms(node, sys_state);
  for (; current_perm != NULL; current_perm = *(Permission **)((char *)current_perm + 4)) {
    if (*(User **)((char *)current_perm + 8) != NULL) {
      printf(" %s", *(char **)(*(User **)((char *)current_perm + 8) + 8));
    }
    if (*(Group **)((char *)current_perm + 0xc) != NULL) {
      printf(" %s", *(char **)(*(Group **)((char *)current_perm + 0xc) + 0x10));
    }
  }
}

// Function: print_user_list
void print_user_list(SystemState *sys_state) {
  int user_idx = 0;
  User *current_user = *(User **)((char *)sys_state + 0x10);
  printf("UID    NAME\n__________\n");
  for (; current_user != NULL; current_user = *(User **)((char *)current_user + 4)) {
    printf("%d %s\n", user_idx, *(char **)((char *)current_user + 8));
    user_idx++;
  }
}

// Function: print_group_list
void print_group_list(SystemState *sys_state) {
  int group_idx = 0;
  Group *current_group = *(Group **)((char *)sys_state + 0x14);
  printf("GUID     NAME\n____________________\n");
  for (; current_group != NULL; current_group = *(Group **)((char *)current_group + 8)) {
    printf("%d %s\n", group_idx, *(char **)((char *)current_group + 0x10));
    group_idx++;
  }
}

// Function: print_users_in_group
void print_users_in_group(Group *group) {
  int user_count = 0;
  User *current_user_link = *(User **)((char *)group + 0xc);
  printf("Count Name\n____________________\n");
  for (; current_user_link != NULL; current_user_link = *(User **)((char *)current_user_link + 4)) {
    printf("%d %s\n", user_count, *(char **)(*(User **)((char *)current_user_link + 8) + 8));
    user_count++;
  }
}

// Function: find_group
Group *find_group(char *name_or_id, SystemState *sys_state) {
  int id = strict_atoi(name_or_id);
  if (strcmp(name_or_id, "root") == 0 || strcmp(name_or_id, "0") == 0) {
    return *(Group **)((char *)sys_state + 0x14);
  } else if (id != 0) {
    Group *found_group = find_group_by_number(id, sys_state);
    if (found_group != NULL) {
      return found_group;
    }
  }
  return find_group_by_name(name_or_id, sys_state);
}

// Function: find_user
User *find_user(char *name_or_id, SystemState *sys_state) {
  int id = strict_atoi(name_or_id);
  if (strcmp(name_or_id, "root") == 0 || strcmp(name_or_id, "0") == 0) {
    return *(User **)((char *)sys_state + 0x10);
  } else if (id != 0) {
    User *found_user = find_user_by_number(id, sys_state);
    if (found_user != NULL) {
      return found_user;
    }
  }
  return find_user_by_name(name_or_id, sys_state);
}

// Function: print_working_dir
void print_working_dir(SystemState *sys_state) {
  Node *current_node = *(Node **)(*(Node **)((char *)sys_state + 4) + 0x24);
  printf("Type  Size     Date     Name       Perms\n_______________________________________\n");
  for (; current_node != NULL; current_node = *(Node **)((char *)current_node + 0)) {
    char node_type = *(char *)((char *)current_node + 8);
    unsigned int file_date = *(unsigned int *)((char *)current_node + 0x10);
    char *node_name = *(char **)((char *)current_node + 0x18);

    if (node_type == 'F') {
      unsigned int file_size = get_file_size(*(FileDataChunk **)((char *)current_node + 0x1c));
      printf("FILE %d  %x   %s  ", file_size, file_date, node_name);
      print_perms(current_node, sys_state);
      printf("\n");
    } else if (node_type == 'D') {
      printf("DIR  %d  %x   %s ", 0x2c, file_date, node_name);
      print_perms(current_node, sys_state);
      printf("\n");
    }
  }
}

// Function: make_epoch
unsigned int make_epoch(unsigned int year, int month, int day, int hour, int minute, int second) {
  int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
    days_in_month[1] = 29;
  }

  unsigned int total_seconds = 0;
  for (unsigned int y = 1970; y < year; y++) {
    if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) {
      total_seconds += 31557600;
    } else {
      total_seconds += 31536000;
    }
  }

  for (int m = 0; m < month - 1; m++) {
    total_seconds += days_in_month[m] * 86400;
  }
  
  total_seconds += (day - 1) * 86400;
  total_seconds += hour * 3600;
  total_seconds += minute * 60;
  total_seconds += second;
  
  return total_seconds;
}

// Function: date_from_epoch
void date_from_epoch(unsigned int epoch_time, unsigned int *date_components) {
  unsigned int year = 1970;
  unsigned int month = 1;
  unsigned int day = 1;
  unsigned int hour = 0;
  unsigned int minute = 0;
  unsigned int second = 0;
  unsigned int temp_epoch = epoch_time;

  while (true) {
    unsigned int seconds_in_year = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 31557600 : 31536000;
    if (seconds_in_year <= temp_epoch) {
      temp_epoch -= seconds_in_year;
      year++;
    } else {
      break;
    }
  }

  int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
    days_in_month[1] = 29;
  }

  for (int m_idx = 0; m_idx < 12; m_idx++) {
    unsigned int seconds_in_month = days_in_month[m_idx] * 86400;
    if (seconds_in_month <= temp_epoch) {
      temp_epoch -= seconds_in_month;
      month++;
    } else {
      break;
    }
  }

  day += temp_epoch / 86400;
  temp_epoch %= 86400;

  hour = temp_epoch / 3600;
  temp_epoch %= 3600;

  minute = temp_epoch / 60;
  second = temp_epoch % 60;

  date_components[0] = year;
  date_components[1] = month;
  date_components[2] = day;
  date_components[3] = hour;
  date_components[4] = minute;
  date_components[5] = second;
}

// Function: strofdate
void strofdate(unsigned int epoch_time) {
  unsigned int date_components[6];
  date_from_epoch(epoch_time, date_components);

  const char *day_names[] = {
    "Thursday", "Friday", "Saturday", "Sunday", "Monday", "Tuesday", "Wednesday"
  };
  const char *month_names[] = {
    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
  };

  unsigned int day_of_week_idx = (epoch_time / 86400) % 7;

  printf("%s, %s %d %d %d:%d:%d GMT\n",
         day_names[day_of_week_idx],
         month_names[date_components[1] - 1],
         date_components[2],
         date_components[0],
         date_components[3],
         date_components[4],
         date_components[5]);
}

// Function: get_time
unsigned int get_time(void) {
  unsigned int year = 0;
  int month = 0;
  int day = 0;
  int hour = 100;
  int minute = 100;
  int second = 100;

  puts("Please enter the date as prompted");
  while (year == 0) {
    year = get_int("Enter year between 1970 and 2105");
    if (year < 1970 || year > 2105) {
      puts("Bad value for year");
      year = 0;
    }
  }
  while (month == 0) {
    month = get_int("Enter month");
    if (month < 1 || month > 12) {
      puts("Bad value for month");
      month = 0;
    }
  }
  while (day == 0) {
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
      days_in_month[1] = 29;
    }
    day = get_int("Enter day");
    if (day < 1 || day > days_in_month[month - 1]) {
      puts("Bad value for day");
      day = 0;
    }
  }
  while (hour == 100) {
    hour = get_int("Enter hour (00-23)");
    if (hour < 0 || hour > 23) {
      puts("Bad value for hour");
      hour = 100;
    }
  }
  while (minute == 100) {
    minute = get_int("Enter Minute (00-59)");
    if (minute < 0 || minute > 59) {
      puts("Bad value for minute");
      minute = 100;
    }
  }
  while (second == 100) {
    second = get_int("Enter second (00-59)");
    if (second < 0 || second > 59) {
      puts("Bad value for second");
      second = 100;
    }
  }
  return make_epoch(year, month, day, hour, minute, second);
}

// Function: main_loop
void main_loop(SystemState *sys_state) {
  // Main command lookup table
  const char *main_commands[] = {"set", "show", "create", "delete", "exit", "help", "ls", "cat", "cd"};
  
  // Subcommand lookup tables for each main command
  const char *set_subcommands[] = {"date", "user", "group", "help"};
  const char *show_subcommands[] = {"date", "user", "group", "workingDir", "help", "userlist", "grouplist", "usersingroup"};
  const char *create_subcommands[] = {"user", "group", "file", "directory", "usertogroup", "perm", "appendfile", "help"};
  const char *delete_subcommands[] = {"user", "group", "file", "directory", "usertogroup", "perm", "filebytes", "help"};

  char input_line[256];
  char parsed_args[16][256];
  
  for (int i = 0; i < 16; i++) {
    memset(parsed_args[i], 0, 256);
  }

  while (true) {
    memset(input_line, 0, sizeof(input_line));
    print_prompt(sys_state);
    get_string(input_line, sizeof(input_line), "");

    for (int i = 0; i < 16; i++) {
      memset(parsed_args[i], 0, 256);
    }
    int arg_count = parse_arg(parsed_args, input_line);

    if (arg_count == 0) continue;

    int cmd_idx = -1; // Default to unknown command
    for (int i = 0; i < sizeof(main_commands) / sizeof(main_commands[0]); i++) {
      if (strcmp(main_commands[i], parsed_args[0]) == 0) {
        cmd_idx = i;
        break;
      }
    }

    const char *arg1 = parsed_args[1];
    const char *arg2 = parsed_args[2];
    const char *arg3 = parsed_args[3];
    const char *arg4 = parsed_args[4];

    switch (cmd_idx) {
    case 0: // "set"
      {
        int sub_cmd_idx = -1;
        for (int i = 0; i < sizeof(set_subcommands) / sizeof(set_subcommands[0]); i++) {
          if (strcmp(set_subcommands[i], arg1) == 0) {
            sub_cmd_idx = i;
            break;
          }
        }
        switch (sub_cmd_idx) {
        case 0: // "date"
          *(unsigned int *)((char *)sys_state + 0x18) = get_time();
          break;
        case 1: // "user"
          if (strcmp(arg2, "") == 0) {
            puts("missing user name or number");
          } else {
            User *user_ptr = find_user((char *)arg2, sys_state);
            if (user_ptr == NULL) {
              printf("unknown user: %s\n", arg2);
            } else {
              *(User **)((char *)sys_state + 8) = user_ptr;
            }
          }
          break;
        case 2: // "group"
          if (strcmp(arg2, "") == 0) {
            puts("missing group name or number");
          } else {
            Group *group_ptr = find_group((char *)arg2, sys_state);
            if (group_ptr == NULL) {
              printf("unknown group: %s\n", arg2);
            } else {
              *(Group **)((char *)sys_state + 0xc) = group_ptr;
            }
          }
          break;
        case 3: // "help"
          print_help(set_subcommands, sizeof(set_subcommands) / sizeof(set_subcommands[0]));
          break;
        default:
          printf("Invalid command: %s\n", arg1);
        }
      }
      break;

    case 1: // "show"
      {
        int sub_cmd_idx = -1;
        for (int i = 0; i < sizeof(show_subcommands) / sizeof(show_subcommands[0]); i++) {
          if (strcmp(show_subcommands[i], arg1) == 0) {
            sub_cmd_idx = i;
            break;
          }
        }
        switch (sub_cmd_idx) {
        case 0: // "date"
          strofdate(*(unsigned int *)((char *)sys_state + 0x18));
          break;
        case 1: // "user"
          printf("%s\n", *(char **)(*(User **)((char *)sys_state + 8) + 8));
          break;
        case 2: // "group"
          printf("%s\n", *(char **)(*(Group **)((char *)sys_state + 0xc) + 0x10));
          break;
        case 3: // "workingDir"
          {
            char path_buffer[4000];
            memset(path_buffer, 0, sizeof(path_buffer));
            str_of_path(path_buffer, sys_state, *(Node **)((char *)sys_state + 4));
            printf("%s/\n", path_buffer);
          }
          break;
        case 4: // "help"
          print_help(show_subcommands, sizeof(show_subcommands) / sizeof(show_subcommands[0]));
          break;
        case 5: // "userlist"
          print_user_list(sys_state);
          break;
        case 6: // "grouplist"
          print_group_list(sys_state);
          break;
        case 7: // "usersingroup"
          if (strcmp(arg2, "") == 0) {
            puts("missing group name or number");
          } else {
            Group *group_ptr = find_group((char *)arg2, sys_state);
            if (group_ptr == NULL) {
              printf("unknown group: %s\n", arg2);
            } else {
              print_users_in_group(group_ptr);
            }
          }
          break;
        default:
          printf("Invalid command: %s\n", arg1);
        }
      }
      break;

    case 2: // "create"
      {
        int sub_cmd_idx = -1;
        for (int i = 0; i < sizeof(create_subcommands) / sizeof(create_subcommands[0]); i++) {
          if (strcmp(create_subcommands[i], arg1) == 0) {
            sub_cmd_idx = i;
            break;
          }
        }
        switch (sub_cmd_idx) {
        case 0: // "user"
          if (strcmp(arg2, "") == 0) {
            char username_buf[256];
            memset(username_buf, 0, sizeof(username_buf));
            get_string(username_buf, sizeof(username_buf), "UserName");
            strncpy((char *)arg2, username_buf, 255); // Copy into parsed_args[2]
            ((char *)arg2)[255] = '\0';
          }
          if (find_user((char *)arg2, sys_state) == NULL) {
            add_user(arg2, sys_state);
          } else {
            puts("Username already in use");
          }
          break;
        case 1: // "group"
          if (strcmp(arg2, "") == 0) {
            char groupname_buf[256];
            memset(groupname_buf, 0, sizeof(groupname_buf));
            get_string(groupname_buf, sizeof(groupname_buf), "GroupName");
            strncpy((char *)arg2, groupname_buf, 255); // Copy into parsed_args[2]
            ((char *)arg2)[255] = '\0';
          }
          if (find_group((char *)arg2, sys_state) == NULL) {
            add_group(arg2, sys_state);
          } else {
            puts("Groupname already in use");
          }
          break;
        case 2: // "file"
          {
            int file_size = atoi(arg3);
            if (strcmp(arg2, "") == 0) {
              puts("Filename required");
            } else if (file_size < 0) {
              puts("File size cannot be negative");
            } else if (file_size >= 0xF4241) { // 0xF4241 is 1MB + 1 (too big)
              puts("Too big");
            } else {
              Node *node_ptr = find_node_by_name(arg2, *(Node **)(*(Node **)((char *)sys_state + 4) + 0x24));
              if (node_ptr == NULL) {
                void *temp_buf = NULL;
                if (file_size > 0) {
                  temp_buf = mallocOrDie(file_size, "Failed to allocate tempBuf");
                  puts("-----Begin File-----");
                  if (receive_bytes(temp_buf, file_size) != file_size) {
                    die("Failed to receive file");
                  }
                }
                add_file(*(Node **)((char *)sys_state + 4), *(unsigned int *)((char *)sys_state + 0x18),
                         file_size, arg2, temp_buf, *(User **)((char *)sys_state + 8));
                if (temp_buf) free(temp_buf);
              } else {
                puts("There is another file or directory with that name");
              }
            }
          }
          break;
        case 3: // "directory"
          if (strcmp(arg2, "") == 0) {
            puts("Directory name required");
          } else {
            Node *node_ptr = find_node_by_name(arg2, *(Node **)(*(Node **)((char *)sys_state + 4) + 0x24));
            if (node_ptr == NULL) {
              add_directory(*(unsigned int *)((char *)sys_state + 0x18), arg2,
                            *(Node **)((char *)sys_state + 4), *(User **)((char *)sys_state + 8));
            } else {
              puts("There is another file or directory with that name");
            }
          }
          break;
        case 4: // "usertogroup"
          if (strcmp(arg2, "") == 0) {
            puts("username or number required");
          } else {
            User *user_ptr = find_user((char *)arg2, sys_state);
            if (user_ptr == NULL) {
              puts("User does not exist, add user first");
            } else {
              Group *current_group = *(Group **)((char *)sys_state + 0xc);
              if (current_group == NULL) {
                puts("No current group set. Use 'set group <name_or_id>' first.");
              } else if (is_user_in_group(user_ptr, current_group)) {
                printf("%s is already in %s\n", arg2, *(char **)((char *)current_group + 0x10));
              } else {
                add_user_to_group(user_ptr, current_group);
              }
            }
          }
          break;
        case 5: // "perm"
          if (strcmp(arg2, "") == 0) {
            puts("name of file or directory required");
          } else {
            bool is_user_perm = (strcmp(arg3, "user") == 0);
            bool is_group_perm = (strcmp(arg3, "group") == 0);
            if (!is_user_perm && !is_group_perm) {
              puts("\'user\' or \'group\' expected for permission type");
            } else if (strcmp(arg4, "") == 0) {
              puts("user name, group name, or number required");
            } else {
              Node *node_ptr = find_node_by_name(arg2, *(Node **)(*(Node **)((char *)sys_state + 4) + 0x24));
              if (node_ptr == NULL) {
                puts("Invalid file or directory");
              } else {
                User *perm_user = NULL;
                Group *perm_group = NULL;
                if (is_user_perm) {
                  perm_user = find_user((char *)arg4, sys_state);
                  if (perm_user == NULL) {
                    printf("user %s not found\n", arg4);
                    break;
                  }
                } else {
                  perm_group = find_group((char *)arg4, sys_state);
                  if (perm_group == NULL) {
                    printf("group %s not found\n", arg4);
                    break;
                  }
                }
                validate_current_perms(node_ptr, sys_state);
                add_perm(perm_user, perm_group, node_ptr);
              }
            }
          }
          break;
        case 6: // "appendfile"
          {
            int bytes_to_append = atoi(arg3);
            if (strcmp(arg2, "") == 0) {
              puts("Filename required");
            } else if (bytes_to_append < 0) {
              puts("Cannot append negative bytes");
            } else if (bytes_to_append >= 0xF4241) {
              puts("Too big");
            } else {
              FileDataChunk *file_data_head = find_file_by_name(arg2, *(Node **)((char *)sys_state + 4));
              if (file_data_head == NULL) {
                puts("No file in working directory by that name");
              } else if (bytes_to_append == 0) {
                puts("Can not add 0 bytes to file");
              } else {
                void *temp_buf = mallocOrDie(bytes_to_append, "Failed to allocate tempBuf");
                puts("-----Begin File-----");
                if (receive_bytes(temp_buf, bytes_to_append) == bytes_to_append) {
                  add_file_chunk(temp_buf, file_data_head, bytes_to_append);
                } else {
                  die("Failed to receive file");
                }
                free(temp_buf);
              }
            }
          }
          break;
        case 7: // "help"
          print_help(create_subcommands, sizeof(create_subcommands) / sizeof(create_subcommands[0]));
          break;
        default:
          printf("Invalid command: %s\n", arg1);
        }
      }
      break;

    case 3: // "delete"
      {
        int sub_cmd_idx = -1;
        for (int i = 0; i < sizeof(delete_subcommands) / sizeof(delete_subcommands[0]); i++) {
          if (strcmp(delete_subcommands[i], arg1) == 0) {
            sub_cmd_idx = i;
            break;
          }
        }
        switch (sub_cmd_idx) {
        case 0: // "user"
          if (strcmp(arg2, "") == 0) {
            char user_name_or_id_buf[256];
            memset(user_name_or_id_buf, 0, sizeof(user_name_or_id_buf));
            get_string(user_name_or_id_buf, sizeof(user_name_or_id_buf), "User Name or number");
            strncpy((char *)arg2, user_name_or_id_buf, 255);
            ((char *)arg2)[255] = '\0';
          }
          User *user_to_delete = find_user((char *)arg2, sys_state);
          if (user_to_delete == NULL) {
            puts("No such user found");
          } else {
            remove_user(user_to_delete, sys_state);
          }
          break;
        case 1: // "group"
          if (strcmp(arg2, "") == 0) {
            char group_name_or_id_buf[256];
            memset(group_name_or_id_buf, 0, sizeof(group_name_or_id_buf));
            get_string(group_name_or_id_buf, sizeof(group_name_or_id_buf), "Group Name or number");
            strncpy((char *)arg2, group_name_or_id_buf, 255);
            ((char *)arg2)[255] = '\0';
          }
          Group *group_to_delete = find_group((char *)arg2, sys_state);
          if (group_to_delete == NULL) {
            puts("no such group found");
          } else {
            remove_group(group_to_delete, sys_state);
          }
          break;
        case 2: // "file"
          if (strcmp(arg2, "") == 0) {
            puts("Filename required");
          } else {
            Node *file_node = find_file_node_by_name(arg2, *(Node **)((char *)sys_state + 4));
            if (file_node == NULL) {
              puts("No such file");
            } else {
              delete_node(file_node, sys_state);
            }
          }
          break;
        case 3: // "directory"
          if (strcmp(arg2, "") == 0) {
            puts("Directory name required");
          } else {
            Node *dir_node = find_directory_by_name(arg2, *(Node **)((char *)sys_state + 4));
            if (dir_node == NULL) {
              puts("No such directory");
            } else {
              delete_node(dir_node, sys_state);
            }
          }
          break;
        case 4: // "usertogroup"
          if (strcmp(arg2, "") == 0) {
            puts("User name required");
          } else if (strcmp(arg3, "") == 0) {
            puts("group name required");
          } else {
            User *user_ptr = find_user((char *)arg2, sys_state);
            if (user_ptr == NULL) {
              puts("No such user");
            } else {
              Group *group_ptr = find_group((char *)arg3, sys_state);
              if (group_ptr == NULL) {
                puts("No such group");
              } else if (!is_user_in_group(user_ptr, group_ptr)) {
                puts("User is not in group");
              } else {
                remove_user_from_group(user_ptr, group_ptr);
              }
            }
          }
          break;
        case 5: // "perm"
          if (strcmp(arg3, "") == 0) {
            puts("User or group name required");
          } else if (strcmp(arg2, "") == 0) {
            puts("file name required");
          } else {
            Node *node_ptr = find_node_by_name(arg2, *(Node **)(*(Node **)((char *)sys_state + 4) + 0x24));
            if (node_ptr == NULL) {
              puts("No such file");
            } else {
              Permission *perm_to_delete = find_perm_by_name(arg3, node_ptr, sys_state);
              if (perm_to_delete == NULL) {
                puts("No such user/group permission on file");
              } else {
                delete_perms(node_ptr, perm_to_delete);
              }
            }
          }
          break;
        case 6: // "filebytes"
          {
            int bytes_to_delete = strict_atoi((char *)arg3);
            if (strcmp(arg2, "") == 0) {
              puts("Filename required");
            } else if (bytes_to_delete == 0) {
              puts("zero bytes deleted");
            } else if (bytes_to_delete < 0) {
              puts("Cannot delete negative bytes");
            } else {
              Node *file_node = find_file_node_by_name(arg2, *(Node **)((char *)sys_state + 4));
              if (file_node == NULL) {
                puts("No such file");
                printf("%s\n", arg2);
              } else {
                FileDataChunk *file_data_head = *(FileDataChunk **)((char *)file_node + 0x1c);
                unsigned int current_file_size = get_file_size(file_data_head);
                if (current_file_size < bytes_to_delete) {
                  puts("Too many bytes to delete");
                } else {
                  delete_file_bytes(file_data_head, current_file_size - bytes_to_delete);
                }
              }
            }
          }
          break;
        case 7: // "help"
          print_help(delete_subcommands, sizeof(delete_subcommands) / sizeof(delete_subcommands[0]));
          break;
        default:
          printf("Invalid command: %s\n", arg1);
        }
      }
      break;

    case 4: // "exit"
      puts("exiting");
      return;

    case 5: // "help"
      print_help(main_commands, sizeof(main_commands) / sizeof(main_commands[0]));
      break;

    case 6: // "ls"
      print_working_dir(sys_state);
      break;

    case 7: // "cat"
      if (strcmp(arg1, "") == 0) {
        puts("Filename required");
      } else {
        FileDataChunk *file_data_head = find_file_by_name(arg1, *(Node **)((char *)sys_state + 4));
        if (file_data_head != NULL) {
          puts("-----Begin File-----");
          for (FileDataChunk *current_chunk = file_data_head; current_chunk != NULL;
               current_chunk = *(FileDataChunk **)((char *)current_chunk + 0xc)) {
            void *chunk_buffer = *(void **)((char *)current_chunk + 4);
            unsigned int chunk_size = *(unsigned int *)((char *)current_chunk + 8);
            ssize_t bytes_written = write(STDOUT_FILENO, chunk_buffer, chunk_size);
            if (bytes_written != chunk_size) {
              puts("file write failed");
            }
          }
          puts("-----END File-----");
        } else {
          puts("No such file in working directory.");
        }
      }
      break;

    case 8: // "cd"
      if (strcmp(arg1, "") == 0) {
        puts("directory required");
      } else if (strcmp(arg1, "..") == 0) {
        Node *current_dir = *(Node **)((char *)sys_state + 4);
        Node *parent_dir = *(Node **)((char *)current_dir + 0x20);
        if (parent_dir != NULL) {
          *(Node **)((char *)sys_state + 4) = parent_dir;
        }
      } else {
        Node *target_dir = find_directory_by_name(arg1, *(Node **)((char *)sys_state + 4));
        if (target_dir == NULL) {
          puts("No such directory in working directory");
        } else {
          *(Node **)((char *)sys_state + 4) = target_dir;
        }
      }
      break;

    default:
      printf("Invalid command %s\n", parsed_args[0]);
      print_help(main_commands, sizeof(main_commands) / sizeof(main_commands[0]));
    }
  }
}

// Function: start_UI
void start_UI(SystemState *sys_state) {
  puts("Welcome to SPIFFS (Somewhat Poorly Implemented Fast File System)");
  *(unsigned int *)((char *)sys_state + 0x18) = get_time();
  main_loop(sys_state);
}