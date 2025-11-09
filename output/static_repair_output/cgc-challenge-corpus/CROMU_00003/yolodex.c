#include <stdio.h>   // For puts, snprintf
#include <stdlib.h>  // For calloc, free, exit, atoi
#include <string.h>  // For strlen, strncpy, memset, strcmp
#include <unistd.h>  // For write, read, close
#include <fcntl.h>   // For open (O_RDONLY)

// Define maximum lengths for contact fields
#define NAME_MAX_LEN 32
#define PHONE_MAX_LEN 16 // E.g., "(nnn)nnn-nnnn" is 13 chars + null
#define OFFICE_MAX_LEN 8 // E.g., "65535" is 5 digits + null

// Structure to represent a contact
typedef struct Contact {
    char firstName[NAME_MAX_LEN];
    char lastName[NAME_MAX_LEN];
    char phoneNumber[PHONE_MAX_LEN];
    short officeNumber;
    char gender;
    char isHacker;
    unsigned int integrityCookie; // Used for integrity check
    unsigned int unknown1;        // Placeholder for original unknown field
    struct Contact *prev;
    struct Contact *next;
} Contact;

// Global variables
Contact *head = NULL;
unsigned int cookie = 0;

// Custom termination function
void _terminate(int status) {
    exit(status);
}

// Function: put
// Writes a string to standard output.
void put(char *str) {
  write(1, str, strlen(str));
}

// Function: validateName
// Validates if a name starts with an uppercase letter (A-Z).
int validateName(char *name) {
  if (name == NULL || *name == '\0') {
      return 0; // Empty name is invalid
  }
  if ((*name < 'A') || (*name > 'Z')) {
    return 0;
  }
  return 1;
}

// Function: validatePhone
// Validates if a phone number matches the format (nnn)nnn-nnnn.
int validatePhone(char *phoneNumber) {
  char *ptr = phoneNumber;

  if (*ptr != '(') {
    return 0;
  }
  ptr++; // Move past '('

  for (int i = 0; i < 3; i++) {
    if (*ptr < '0' || *ptr > '9') {
      return 0;
    }
    ptr++;
  }

  if (*ptr != ')') {
    return 0;
  }
  ptr++; // Move past ')'

  for (int i = 0; i < 3; i++) {
    if (*ptr < '0' || *ptr > '9') {
      return 0;
    }
    ptr++;
  }

  if (*ptr != '-') {
    return 0;
  }
  ptr++; // Move past '-'

  for (int i = 0; i < 4; i++) {
    if (*ptr < '0' || *ptr > '9') {
      return 0;
    }
    ptr++;
  }
  
  // Ensure no extra characters after the valid phone number format
  if (*ptr != '\0') {
      return 0;
  }

  return 1;
}

// Function: createContact
// Allocates and initializes a new Contact structure.
Contact *createContact(char *firstName, char *lastName, char *phoneNumber,
                       short officeNumber, char gender, char isHacker) {
  Contact *newContact = (Contact *)calloc(1, sizeof(Contact));

  if (newContact == NULL) {
    puts("Couldn't allocate contact. Something's wrong.");
    _terminate(EXIT_FAILURE);
  }

  // Validate and copy first name
  if (validateName(firstName) == 0) {
    puts("Invalid first name. Must begin with a capital letter.");
    free(newContact);
    return NULL;
  }
  strncpy(newContact->firstName, firstName, NAME_MAX_LEN - 1);
  newContact->firstName[NAME_MAX_LEN - 1] = '\0'; // Ensure null termination

  // Validate and copy last name
  if (validateName(lastName) == 0) {
    puts("Invalid last name. Must begin with a capital letter.");
    free(newContact);
    return NULL;
  }
  strncpy(newContact->lastName, lastName, NAME_MAX_LEN - 1);
  newContact->lastName[NAME_MAX_LEN - 1] = '\0'; // Ensure null termination

  // Validate and copy phone number
  if (validatePhone(phoneNumber) == 0) {
    puts("Invalid phone number...must look like (nnn)nnn-nnnn");
    free(newContact);
    return NULL;
  }
  strncpy(newContact->phoneNumber, phoneNumber, PHONE_MAX_LEN - 1);
  newContact->phoneNumber[PHONE_MAX_LEN - 1] = '\0'; // Ensure null termination

  // Assign other fields
  newContact->officeNumber = officeNumber;
  newContact->gender = gender;
  newContact->isHacker = isHacker;
  newContact->integrityCookie = cookie; // Assign global cookie
  newContact->unknown1 = 0; 
  newContact->prev = NULL;
  newContact->next = NULL;

  return newContact;
}

// Function: insert
// Inserts a new contact into the linked list.
void insert(Contact *newContact) {
  if (head == NULL) {
    head = newContact;
  } else {
    Contact *current = head;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = newContact;
    newContact->prev = current;
  }
}

// Function: removeContact (renamed from 'remove' to avoid conflict with standard library)
// Removes a contact from the linked list and returns it.
Contact *removeContact(Contact *contactToRemove) {
  if (contactToRemove == NULL) {
      return NULL;
  }

  // Adjust head if the contact to remove is the head
  if (contactToRemove->prev == NULL) {
    head = contactToRemove->next;
    if (head != NULL) {
      head->prev = NULL;
    }
  } else { // Adjust previous contact's next pointer
    contactToRemove->prev->next = contactToRemove->next;
  }

  // Adjust next contact's prev pointer
  if (contactToRemove->next != NULL) {
    contactToRemove->next->prev = contactToRemove->prev;
  }

  contactToRemove->next = NULL; // Clear pointers of the removed contact
  contactToRemove->prev = NULL;
  
  return contactToRemove;
}

// Function: printGender
// Prints a descriptive string for the given gender character.
void printGender(char gender) {
  if (gender == 'T') {
    puts("Trans-Gender");
  } else if (gender == 'F') {
    puts("Female");
  } else if (gender == 'M') {
    puts("Male");
  } else {
    puts("Other");
  }
}

// Function: printContact
// Prints the details of a single contact.
void printContact(Contact *contact) {
  put("First Name: \t");
  puts(contact->firstName);
  put("Last Name: \t");
  puts(contact->lastName);
  put("Phone Number: \t");
  puts(contact->phoneNumber);
}

// Function: printList
// Prints all contacts in the yolodex.
void printList(void) {
  if (head == NULL) {
    puts("Empty yolodex.");
    return;
  }

  Contact *current = head;
  for (; current != NULL; current = current->next) {
    puts("========================================");
    printContact(current);
  }
  puts("========================================");
}

// Function: sortYolodex
// Sorts the yolodex by last name using selection sort.
void sortYolodex(void) {
  Contact *sortedListHead = NULL;
  Contact *sortedListTail = NULL;

  while (head != NULL) {
    Contact *minContact = head;
    Contact *current = head->next;

    while (current != NULL) {
      // Compare last names
      int cmpResult = strcmp(current->lastName, minContact->lastName);

      if (cmpResult < 0) {
        minContact = current;
      }
      current = current->next;
    }

    // Remove minContact from the original list
    Contact *removedContact = removeContact(minContact);

    // Add removedContact to the sorted list
    if (sortedListHead == NULL) {
      sortedListHead = removedContact;
      sortedListTail = removedContact;
    } else {
      sortedListTail->next = removedContact;
      removedContact->prev = sortedListTail;
      sortedListTail = removedContact;
    }
  }
  head = sortedListHead; // Update global head to the new sorted list
}

// Function: initYolodex
// Initializes the yolodex with some default contacts.
void initYolodex(void) {
  Contact *newContact;

  newContact = createContact("Robert", "Morris", "(617)253-5982", 0x80cc, 'M', 'y');
  insert(newContact);
  newContact = createContact("Chareth", "Cutestory", "(123)456-7890", 0x1092, 'M', 'n');
  insert(newContact);
  newContact = createContact("Tim", "Berners-Lee", "(638)844-2074", 0x194, 'M', 'y');
  insert(newContact);
  newContact = createContact("Pwnies", "McPwn", "(222)223-1337", 0x7a69, 'F', 'y');
  insert(newContact);
  newContact = createContact("Jenny", "Fakename", "(555)867-5309", 0x7a69, 'F', 'n');
  insert(newContact);
}

// Function: recvUntil
// Reads characters into buffer, up to max_len-1 bytes, until delimiter is encountered.
// Returns number of bytes read (excluding delimiter). Buffer is always null-terminated.
int recvUntil(char *buffer, int max_len, char delimiter) {
  int i = 0;
  ssize_t bytes_read;
  char c;

  // Leave space for null terminator
  while (i < max_len - 1) {
    bytes_read = read(0, &c, 1); // Read one byte from stdin

    if (bytes_read == 0) { // EOF
        buffer[i] = '\0'; // Null-terminate
        return i;
    }
    if (bytes_read == -1) { // Error
      puts("Error reading input.");
      _terminate(EXIT_FAILURE);
    }

    if (c == delimiter) {
      buffer[i] = '\0'; // Replace delimiter with null terminator
      return i;
    }
    buffer[i] = c;
    i++;
  }
  
  buffer[i] = '\0'; // Null-terminate the buffer if max_len-1 was reached
  return i;
}

// Function: promptAdd
// Prompts the user for contact details and adds a new contact.
void promptAdd(void) {
  char firstName_buffer[NAME_MAX_LEN];
  char lastName_buffer[NAME_MAX_LEN];
  char phone_buffer[PHONE_MAX_LEN];
  char office_buffer[OFFICE_MAX_LEN];
  char gender_char;
  char is_hacker_char;
  short office_number;
  Contact *newContact = NULL;

  while (newContact == NULL) { // Loop until a valid contact is created
    put("Enter first name: ");
    recvUntil(firstName_buffer, NAME_MAX_LEN, '\n');

    put("Enter last name: ");
    recvUntil(lastName_buffer, NAME_MAX_LEN, '\n');

    put("Enter phone number (e.g., (nnn)nnn-nnnn): ");
    recvUntil(phone_buffer, PHONE_MAX_LEN, '\n');

    put("Enter office number: ");
    memset(office_buffer, 0, OFFICE_MAX_LEN); // Clear buffer before reading
    recvUntil(office_buffer, OFFICE_MAX_LEN, '\n');
    office_number = (short)atoi(office_buffer);

    put("Enter gender (M/F/T): ");
    recvUntil(&gender_char, 2, '\n'); // Read one char + newline. Max_len=2 for char + null

    is_hacker_char = '?'; // Sentinel value to ensure valid input
    while (is_hacker_char == '?') {
      put("Is the user a hacker? (y/n): ");
      recvUntil(&is_hacker_char, 2, '\n');
      if (is_hacker_char != 'n' && is_hacker_char != 'y') {
        puts("Please enter y or n for hacker.");
        is_hacker_char = '?'; // Reset to loop again
      }
    }
    
    newContact = createContact(firstName_buffer, lastName_buffer, phone_buffer,
                               office_number, gender_char, is_hacker_char);
  }

  if (newContact->integrityCookie != cookie) {
    puts("Contact corruption detected.");
    _terminate(EXIT_FAILURE);
  }
  
  insert(newContact);
  puts("Contact added.");
}

// Function: findContact
// Prompts for first and last name and searches for a matching contact.
Contact *findContact(void) {
  char firstName_search[NAME_MAX_LEN];
  char lastName_search[NAME_MAX_LEN];
  
  put("First: ");
  recvUntil(firstName_search, NAME_MAX_LEN, '\n');
  put("Last: ");
  recvUntil(lastName_search, NAME_MAX_LEN, '\n');

  Contact *current = head;
  while (current != NULL) {
    if (strcmp(firstName_search, current->firstName) == 0 &&
        strcmp(lastName_search, current->lastName) == 0) {
      return current; // Found it
    }
    current = current->next;
  }
  return NULL; // Not found
}

// Function: promptDel
// Prompts to find a contact and then deletes it.
void promptDel(void) {
  Contact *contactToDelete = findContact();
  if (contactToDelete == NULL) {
    puts("No such contact found.");
  } else {
    Contact *removedContact = removeContact(contactToDelete);
    if (removedContact != NULL) {
        free(removedContact);
        puts("Contact removed.");
    } else {
        puts("Error removing contact.");
    }
  }
}

// Function: editUser
// Allows editing fields of an existing contact.
void editUser(Contact *contact) {
  if (contact == NULL) {
      return; // No contact to edit
  }

  puts("Updating fields. Send just a newline to keep old data.");

  char input_buffer[NAME_MAX_LEN]; // For first and last names
  char phone_buffer[PHONE_MAX_LEN]; // For phone number
  char office_num_str[OFFICE_MAX_LEN]; // For office number
  short new_office_number;
  char new_gender;
  char new_is_hacker;

  // First name
  put("New first name: ");
  recvUntil(input_buffer, NAME_MAX_LEN, '\n');
  if (input_buffer[0] != '\0') {
    if (validateName(input_buffer) == 0) {
      puts("Not a legal first name. Stopping edit.");
      return;
    }
    strncpy(contact->firstName, input_buffer, NAME_MAX_LEN - 1);
    contact->firstName[NAME_MAX_LEN - 1] = '\0';
  }

  // Last name
  put("New last name: ");
  recvUntil(input_buffer, NAME_MAX_LEN, '\n');
  if (input_buffer[0] != '\0') {
    if (validateName(input_buffer) == 0) {
      puts("Not a legal last name. Stopping edit.");
      return;
    }
    strncpy(contact->lastName, input_buffer, NAME_MAX_LEN - 1);
    contact->lastName[NAME_MAX_LEN - 1] = '\0';
  }

  // Phone number
  put("New phone number: ");
  recvUntil(phone_buffer, PHONE_MAX_LEN, '\n');
  if (phone_buffer[0] != '\0') {
    if (validatePhone(phone_buffer) == 0) {
      puts("Not a legal phone number. Stopping edit.");
      return;
    }
    strncpy(contact->phoneNumber, phone_buffer, PHONE_MAX_LEN - 1);
    contact->phoneNumber[PHONE_MAX_LEN - 1] = '\0';
  }

  // Office number
  put("New office number: ");
  recvUntil(office_num_str, OFFICE_MAX_LEN, '\n');
  if (office_num_str[0] != '\0') {
    new_office_number = (short)atoi(office_num_str);
    contact->officeNumber = new_office_number;
  }

  // Gender
  put("New gender (M/F/T): ");
  recvUntil(&new_gender, 2, '\n'); // Read one char + null
  if (new_gender != '\0') {
    contact->gender = new_gender;
  }

  // Hacker status
  put("Is the user a hacker? (y/n): ");
  recvUntil(&new_is_hacker, 2, '\n'); // Read one char + null
  if (new_is_hacker != '\0') {
    if (new_is_hacker != 'y' && new_is_hacker != 'n') {
        puts("Invalid input for hacker status. Keeping old value.");
    } else {
        contact->isHacker = new_is_hacker;
    }
  }

  if (contact->integrityCookie != cookie) {
    puts("Contact corruption detected.");
    _terminate(EXIT_FAILURE);
  }
  puts("Contact updated.");
}

// Function: promptEdit
// Prompts to find a contact and then allows editing it.
void promptEdit(void) {
  Contact *contactToEdit = findContact();
  if (contactToEdit == NULL) {
    puts("No such contact.");
  } else {
    editUser(contactToEdit);
  }
}

// Function: promptShow
// Prompts to find a contact and then displays its full details.
void promptShow(void) {
  Contact *contactToShow = findContact();
  if (contactToShow == NULL) {
    puts("No such contact.");
  } else {
    char command_char = 's'; // 's' for show, 'q' to quit
    char office_num_str[OFFICE_MAX_LEN]; // Buffer for snprintf

    while (command_char != 'q') {
      puts("****************************************");
      put("First name:\t");
      puts(contactToShow->firstName);
      put("Last name:\t");
      puts(contactToShow->lastName);
      put("Phone num:\t");
      puts(contactToShow->phoneNumber);
      
      // Convert short officeNumber to string
      snprintf(office_num_str, OFFICE_MAX_LEN, "%hd", contactToShow->officeNumber);
      put("Office Number:\t");
      puts(office_num_str);
      
      put("Gender:\t\t");
      printGender(contactToShow->gender);
      
      if (contactToShow->isHacker == 'y') {
        puts("[\x1b[31m!\x1b[0m]This user known to be a hacker[\x1b[31m!\x1b[0m]");
      }
      puts("****************************************");
      
      put("Press 'q' to quit showing this contact: ");
      recvUntil(&command_char, 2, '\n'); // Read one char + null
    }
  }
}

// Function: initCookies
// Initializes a global cookie value using /dev/urandom.
void initCookies(void) {
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd == -1) {
    puts("Error opening /dev/urandom. Panic!");
    _terminate(EXIT_FAILURE);
  }

  ssize_t bytes_read = read(fd, &cookie, sizeof(cookie));
  close(fd);

  if (bytes_read != sizeof(cookie)) {
    puts("Couldn't get 4 random bytes. Panic!");
    _terminate(EXIT_FAILURE);
  }
  
  // Original code has `cookie = cookie | 0x10204080;` -- preserving original intent.
  // This ensures some bits are set, possibly for distinguishing from all-zero or small values.
  cookie |= 0x10204080;
}

// Function: inputLoop
// Main loop for user interaction with the yolodex menu.
void inputLoop(void) {
  char command_buffer[6]; // Buffer to read command + newline + null terminator

  while (1) {
    put("\n--- Yolodex Menu ---\n");
    put("@: Add Contact\n");
    put("A: Delete Contact\n");
    put("B: Edit Contact\n");
    put("C: Show Contact Details\n");
    put("D: Print All Contacts\n");
    put("E: Sort Contacts (by last name)\n");
    put("F: Exit\n");
    put("Enter command: ");

    memset(command_buffer, 0, sizeof(command_buffer)); // Clear buffer
    recvUntil(command_buffer, sizeof(command_buffer), '\n'); // Read command

    switch (command_buffer[0]) {
      case '@':
        promptAdd();
        break;
      case 'A':
        promptDel();
        break;
      case 'B':
        promptEdit();
        break;
      case 'C':
        promptShow();
        break;
      case 'D':
        printList();
        break;
      case 'E':
        sortYolodex();
        puts("Yolodex sorted by last name.");
        break;
      case 'F':
        puts("Thank you for using the yolodex.");
        _terminate(EXIT_SUCCESS);
      default:
        puts("Unrecognized Command.");
        break;
    }
  }
}

// Function: main
// Entry point of the program.
int main(void) {
  initCookies();
  puts("Welcome to the yolodex.");
  initYolodex();
  inputLoop();
  
  // This part is typically unreachable as inputLoop contains an infinite loop
  // and exits via _terminate().
  return 0;
}