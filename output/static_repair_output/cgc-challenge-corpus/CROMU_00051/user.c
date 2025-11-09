#include <stdio.h>   // For FILE, fopen, fclose, fgets, printf, puts, fwrite
#include <string.h>  // For strcpy, strcmp, strtok, strlen, strcat, strcspn
#include <stdlib.h>  // For strdup, free, memset
#include <stddef.h>  // For size_t

// Define global variables for current user and group
// Assuming a reasonable buffer size, e.g., 32 characters for username/group
char current_user[32];
char current_group[32];

// Placeholder for ReadUntilNewline function
// This function reads from stdin until a newline or buffer limit.
int ReadUntilNewline(int fd, char *buffer, int max_len) {
    if (buffer == NULL || max_len <= 0) return 0;
    int i = 0;
    int c;
    // Assuming fd 0 means stdin based on usage, but actual fd is ignored here
    (void)fd; // Suppress unused parameter warning
    while ((i < max_len - 1) && ((c = getchar()) != EOF) && (c != '\n') && (c != '\r')) {
        buffer[i++] = (char)c;
    }
    buffer[i] = '\0';
    // Consume remaining characters on the line if any, up to a newline
    if (c != '\n' && c != '\r' && c != EOF) {
        while ((c = getchar()) != EOF && c != '\n' && c != '\r');
    }
    return i; // Return number of characters read (excluding null terminator)
}

// Function: Login
void Login(void) {
  strcpy(current_user,"crs");
  strcpy(current_group,"crs");
}

// Function: ChUser
// Original signature was `undefined4 ChUser(int *param_1)`.
// Interpreted as `int ChUser(int argc, char *argv[])` for standard C compilation and safety.
int ChUser(int argc, char *argv[]) {
  char password_input[32];
  char line_buffer[128];
  char *group_name;
  char *user_name_from_file;
  char *stored_password_hash;
  FILE *passwd_file;
  
  if (argc != 2) { // Expecting command + username
    puts("Input error");
    puts("Usage: chuser <username>");
    return 0;
  }
  
  char *target_username = argv[1];

  passwd_file = fopen("passwd","r");
  if (passwd_file == NULL) {
    puts("Unable to open passwd file");
    return 0;
  }
  
  int user_found = 0;
  while (fgets(line_buffer, sizeof(line_buffer), passwd_file) != NULL) {
      line_buffer[strcspn(line_buffer, "\n\r")] = '\0'; // Remove trailing newline/carriage return
      
      if (line_buffer[0] == '\0') continue; // Skip empty lines

      // strtok modifies the buffer, but line_buffer is not reused after parsing for this specific function.
      stored_password_hash = strtok(line_buffer,":");
      if (stored_password_hash == NULL) {
        fclose(passwd_file);
        puts("Passwd file is corrupted");
        return 0;
      }
      user_name_from_file = strtok(NULL,":");
      if (user_name_from_file == NULL) {
        fclose(passwd_file);
        puts("Passwd file is corrupted");
        return 0;
      }
      
      if (strcmp(user_name_from_file, target_username) == 0) {
          user_found = 1;
          group_name = strtok(NULL,":");
          if (group_name == NULL) {
              fclose(passwd_file);
              puts("Passwd file is corrupted");
              return 0;
          }

          if (strcmp(current_user, "root") == 0) {
              strcpy(current_user, user_name_from_file);
              strcpy(current_group, group_name);
              printf("Authentication success for user '%s'\n\r", current_user);
              fclose(passwd_file);
              return 1;
          } else {
              printf("Password: ");
              if (ReadUntilNewline(0, password_input, sizeof(password_input)) == 0) {
                  printf("\b\b  \b\b\n\r"); // Erase password prompt
                  puts("Password input failed or empty.");
                  fclose(passwd_file);
                  return 0;
              }
              printf("\b\b  \b\b\n\r");
              
              if (strcmp(stored_password_hash, password_input) == 0) {
                  strcpy(current_user, user_name_from_file);
                  strcpy(current_group, group_name);
                  printf("Authentication success for user '%s'\n\r", current_user);
                  fclose(passwd_file);
                  return 1;
              } else {
                  puts("Authentication failure");
                  fclose(passwd_file);
                  return 0;
              }
          }
      }
  }

  fclose(passwd_file);
  if (!user_found) {
    printf("No passwd entry for user '%s'\n\r", target_username);
  }
  return 0; // Authentication failed or user not found
}

// Function: ChPw
// Original signature was `undefined4 ChPw(int *param_1)`.
// Interpreted as `int ChPw(int argc, char *argv[])` for standard C compilation and safety.
int ChPw(int argc, char *argv[]) {
  char new_entry_buffer[128];
  char all_entries_buffer[4096];
  char target_username[32];
  char new_password_input[32];
  char line_buffer[128];
  char *group_name;
  char *user_name_from_file;
  char *stored_password_hash;
  FILE *passwd_file;
  
  if (strcmp(current_user, "root") == 0) {
    if (argc != 2) {
      puts("Input error");
      puts("Usage: chpw <username>");
      return 0;
    }
    strcpy(target_username, argv[1]);
  } else {
    if (argc != 1) { // Current user changes their own password, no username arg
      puts("Input error");
      puts("Usage: chpw");
      return 0;
    }
    strcpy(target_username, current_user);
  }

  passwd_file = fopen("passwd","r");
  if (passwd_file == NULL) {
    puts("Unable to open passwd file");
    return 0;
  }
  
  memset(all_entries_buffer, 0, sizeof(all_entries_buffer));
  int user_found_and_processed = 0;
  
  while (fgets(line_buffer, sizeof(line_buffer), passwd_file) != NULL) {
    line_buffer[strcspn(line_buffer, "\n\r")] = '\0';
    if (line_buffer[0] == '\0') {
        strcat(all_entries_buffer, "\n"); // Preserve empty lines in output
        continue;
    }

    char *temp_line_for_strtok = strdup(line_buffer); // Duplicate to preserve original line_buffer
    if (temp_line_for_strtok == NULL) {
        fclose(passwd_file);
        puts("Memory allocation failed");
        return 0;
    }

    stored_password_hash = strtok(temp_line_for_strtok, ":");
    if (stored_password_hash == NULL) {
      free(temp_line_for_strtok);
      fclose(passwd_file);
      puts("Passwd file is corrupted");
      return 0;
    }
    user_name_from_file = strtok(NULL, ":");
    if (user_name_from_file == NULL) {
      free(temp_line_for_strtok);
      fclose(passwd_file);
      puts("Passwd file is corrupted");
      return 0;
    }

    if (strcmp(user_name_from_file, target_username) == 0) {
        user_found_and_processed = 1;
        group_name = strtok(NULL, ":");
        if (group_name == NULL) {
            free(temp_line_for_strtok);
            fclose(passwd_file);
            puts("Passwd file is corrupted");
            return 0;
        }

        printf("New password: ");
        if (ReadUntilNewline(0, new_password_input, sizeof(new_password_input)) == 0) {
            printf("\b\b  \b\b\n\r");
            puts("Password not changed (empty input)");
            free(temp_line_for_strtok);
            fclose(passwd_file);
            return 0;
        }
        printf("\b\b  \b\b\n\r");

        sprintf(new_entry_buffer, "%s:%s:%s\n", new_password_input, user_name_from_file, group_name);
        
        if (strlen(all_entries_buffer) + strlen(new_entry_buffer) >= sizeof(all_entries_buffer)) {
            puts("Password file is too large");
            free(temp_line_for_strtok);
            fclose(passwd_file);
            return 0;
        }
        strcat(all_entries_buffer, new_entry_buffer);
    } else {
        // User does not match, append the original line to the buffer
        if (strlen(all_entries_buffer) + strlen(line_buffer) + 1 >= sizeof(all_entries_buffer)) { // +1 for newline
            puts("Password file is too large");
            free(temp_line_for_strtok);
            fclose(passwd_file);
            return 0;
        }
        strcat(all_entries_buffer, line_buffer);
        strcat(all_entries_buffer, "\n");
    }
    free(temp_line_for_strtok);
  }
  fclose(passwd_file);

  if (!user_found_and_processed) {
      puts("User not found in passwd file.");
      return 0;
  }

  if (strlen(all_entries_buffer) != 0) {
    passwd_file = fopen("passwd","w");
    if (passwd_file == NULL) {
      puts("Unable to open passwd file for writing");
      return 0;
    }
    size_t bytes_to_write = strlen(all_entries_buffer);
    size_t bytes_written = fwrite(all_entries_buffer, 1, bytes_to_write, passwd_file);
    if (bytes_written != bytes_to_write) {
      puts("Password file update failed\n\r");
      fclose(passwd_file);
      return 0;
    }
    fclose(passwd_file);
  }
  return 1;
}

// Function: ID
// Original signature was `undefined4 ID(int *param_1)`.
// Interpreted as `int ID(int argc, char *argv[])` for standard C compilation and safety.
int ID(int argc, char *argv[]) {
  if (argc != 1) { // Expecting only command
    puts("Input error");
    puts("Usage: id");
    return 0;
  }
  printf("uid=%s gid=%s\n\r", current_user, current_group);
  return 1;
}

// Function: NewUser
// Original signature was `undefined4 NewUser(int *param_1)`.
// Interpreted as `int NewUser(int argc, char *argv[])` for standard C compilation and safety.
int NewUser(int argc, char *argv[]) {
  char new_entry_buffer[128];
  char all_entries_buffer[4096];
  char new_password_input[32];
  char line_buffer[128];
  char *user_name_from_file;
  char *stored_password_hash;
  FILE *passwd_file;
  
  if (strcmp(current_user, "root") != 0) {
    puts("Must be root");
    return 0;
  }

  if (argc != 3) { // Expecting command + username + group
    puts("Input error");
    puts("Usage: newuser <username> <group>");
    return 0;
  }
  
  char *new_username = argv[1];
  char *new_group = argv[2];

  passwd_file = fopen("passwd","r");
  if (passwd_file == NULL) {
    puts("Unable to open passwd file");
    return 0;
  }
  
  memset(all_entries_buffer, 0, sizeof(all_entries_buffer));
  
  while (fgets(line_buffer, sizeof(line_buffer), passwd_file) != NULL) {
    line_buffer[strcspn(line_buffer, "\n\r")] = '\0';
    if (line_buffer[0] == '\0') {
        strcat(all_entries_buffer, "\n");
        continue;
    }

    char *temp_line_for_strtok = strdup(line_buffer);
    if (temp_line_for_strtok == NULL) {
        fclose(passwd_file);
        puts("Memory allocation failed");
        return 0;
    }

    stored_password_hash = strtok(temp_line_for_strtok, ":");
    if (stored_password_hash == NULL) {
      free(temp_line_for_strtok);
      fclose(passwd_file);
      puts("Passwd file is corrupted");
      return 0;
    }
    user_name_from_file = strtok(NULL, ":");
    if (user_name_from_file == NULL) {
      free(temp_line_for_strtok);
      fclose(passwd_file);
      puts("Passwd file is corrupted");
      return 0;
    }
    
    if (strcmp(user_name_from_file, new_username) == 0) {
        printf("User '%s' already exists\n\r", new_username);
        free(temp_line_for_strtok);
        fclose(passwd_file);
        return 0;
    }
    
    // Append existing line to buffer
    if (strlen(all_entries_buffer) + strlen(line_buffer) + 1 >= sizeof(all_entries_buffer)) {
        puts("Password file is too large");
        free(temp_line_for_strtok);
        fclose(passwd_file);
        return 0;
    }
    strcat(all_entries_buffer, line_buffer);
    strcat(all_entries_buffer, "\n");
    free(temp_line_for_strtok);
  }
  fclose(passwd_file);
  
  printf("New user password: ");
  if (ReadUntilNewline(0, new_password_input, sizeof(new_password_input)) == 0) {
    printf("\b\b  \b\b\n\r");
    puts("Password input failed or empty.");
    return 0;
  }
  printf("\b\b  \b\b\n\r");

  sprintf(new_entry_buffer, "%s:%s:%s\n", new_password_input, new_username, new_group);
  
  if (strlen(all_entries_buffer) + strlen(new_entry_buffer) >= sizeof(all_entries_buffer)) {
    puts("Password file is too large");
    return 0;
  }
  strcat(all_entries_buffer, new_entry_buffer);
  
  if (strlen(all_entries_buffer) != 0) {
    passwd_file = fopen("passwd","w");
    if (passwd_file == NULL) {
      puts("Unable to open passwd file");
      return 0;
    }
    size_t bytes_to_write = strlen(all_entries_buffer);
    size_t bytes_written = fwrite(all_entries_buffer, 1, bytes_to_write, passwd_file);
    if (bytes_written != bytes_to_write) {
      puts("Password file update failed\n\r");
      fclose(passwd_file);
      return 0;
    }
    fclose(passwd_file);
  }
  printf("User '%s' added\n\r", new_username);
  return 1;
}

// Function: DelUser
// Original signature was `undefined4 DelUser(int *param_1)`.
// Interpreted as `int DelUser(int argc, char *argv[])` for standard C compilation and safety.
int DelUser(int argc, char *argv[]) {
  char all_entries_buffer[4096];
  char line_buffer[128];
  FILE *passwd_file;
  
  if (strcmp(current_user, "root") != 0) {
    puts("Must be root");
    return 0;
  }

  if (argc != 2) { // Expecting command + username
    puts("Input error");
    puts("Usage: deluser <username>");
    return 0;
  }
  
  char *target_username = argv[1];
  
  passwd_file = fopen("passwd","r");
  if (passwd_file == NULL) {
    puts("Unable to open passwd file");
    return 0;
  }
  
  memset(all_entries_buffer, 0, sizeof(all_entries_buffer));
  int user_deleted_flag = 0;
  
  while (fgets(line_buffer, sizeof(line_buffer), passwd_file) != NULL) {
    line_buffer[strcspn(line_buffer, "\n\r")] = '\0';
    if (line_buffer[0] == '\0') {
        strcat(all_entries_buffer, "\n");
        continue;
    }

    char *temp_line_for_strtok = strdup(line_buffer);
    if (temp_line_for_strtok == NULL) {
        fclose(passwd_file);
        puts("Memory allocation failed");
        return 0;
    }

    char *password_field = strtok(temp_line_for_strtok, ":");
    if (password_field == NULL) {
      free(temp_line_for_strtok);
      fclose(passwd_file);
      puts("Passwd file is corrupted");
      return 0;
    }
    char *user_name_from_file = strtok(NULL, ":");
    if (user_name_from_file == NULL) {
      free(temp_line_for_strtok);
      fclose(passwd_file);
      puts("Passwd file is corrupted");
      return 0;
    }
    char *group_name = strtok(NULL, ":"); // This field is parsed but not used in DelUser logic
    if (group_name == NULL) { // Still need to check for corruption
      free(temp_line_for_strtok);
      fclose(passwd_file);
      puts("Passwd file is corrupted");
      return 0;
    }
    
    if (strcmp(user_name_from_file, target_username) == 0) {
        user_deleted_flag = 1;
    } else {
        if (strlen(all_entries_buffer) + strlen(line_buffer) + 1 >= sizeof(all_entries_buffer)) {
            puts("Password file is too large");
            free(temp_line_for_strtok);
            fclose(passwd_file);
            return 0;
        }
        strcat(all_entries_buffer, line_buffer);
        strcat(all_entries_buffer, "\n");
    }
    free(temp_line_for_strtok);
  }
  fclose(passwd_file);

  if (!user_deleted_flag) {
    printf("User '%s' not found\n\r", target_username);
    return 0;
  }
  
  if (strlen(all_entries_buffer) != 0) {
    passwd_file = fopen("passwd","w");
    if (passwd_file == NULL) {
      puts("Unable to open passwd file");
      return 0;
    }
    size_t bytes_to_write = strlen(all_entries_buffer);
    size_t bytes_written = fwrite(all_entries_buffer, 1, bytes_to_write, passwd_file);
    if (bytes_written != bytes_to_write) {
      puts("Password file update failed\n\r");
      fclose(passwd_file);
      return 0;
    }
    fclose(passwd_file);
  }
  printf("User '%s' deleted\n\r", target_username);
  return 1;
}