#include <stdlib.h> // For malloc, free, atoi
#include <string.h> // For strlen, strcmp, memset, strncpy, strcpy
#include <stdio.h>  // For NULL, generally good practice

// Forward declarations to resolve dependencies
// setDeliveryType is used by newSubscription
// getChannel, newChannel, getSubscription, newSubscription are used by addSubscriptions

// Note: The original code uses `undefined4` for both integer values and pointers.
// In C, a pointer typically occupies `sizeof(void*)` bytes, and an `int` typically 4 bytes.
// `undefined4` suggests 4 bytes. If the original code was compiled for a 32-bit system,
// `sizeof(int)` and `sizeof(void*)` would both be 4 bytes.
// For 64-bit Linux, `sizeof(void*)` is 8 bytes, while `sizeof(int)` is 4 bytes.
// This refactored code uses `void*` for generic pointers and `unsigned int` for integer values that were `undefined4` or `uint`.
// It also adjusts `malloc` sizes to use `sizeof(void*)` to be compatible with 64-bit systems.

// Function: setDeliveryType
// param_1: A pointer to a char**, which is the 'subscription' struct.
//          It's expected to be allocated as char *[4] (or similar).
//          Specifically, it modifies subscription[1].
// param_2: The delivery type string.
int setDeliveryType(char ***subscription_ptr_addr, char *delivery_type_str) {
  char **subscription = *subscription_ptr_addr;
  
  // Check if delivery_type_str is one of the predefined strings
  if (!(strcmp("guaranteed", delivery_type_str) == 0 ||
        strcmp("latest", delivery_type_str) == 0 ||
        strcmp("high", delivery_type_str) == 0 ||
        strcmp("medium", delivery_type_str) == 0 ||
        strcmp("low", delivery_type_str) == 0)) {
    // Not a predefined string, try to parse as an integer
    if (atoi(delivery_type_str) < 1) { // Assuming 0 or negative integers are invalid
      return 1; // Invalid numeric delivery type
    }
  }

  // Common allocation and copy logic
  size_t len = strlen(delivery_type_str);
  char *allocated_str = (char *)malloc(len + 1);
  if (allocated_str == NULL) {
    return 1; // Allocation failed
  }
  
  memset(allocated_str, 0, len + 1);
  strcpy(allocated_str, delivery_type_str);
  
  subscription[1] = allocated_str; // Store the allocated string pointer at index 1
  
  return 0; // Success
}

// Function: newSubscription
// Allocates and initializes a new subscription structure (char*[4]).
// param_1: The subscription name string.
// param_2: A pointer to the channel's delivery type string (stored as is).
// param_3: The delivery type string for this specific subscription (copied).
char ** newSubscription(char *subscription_name, char *channel_delivery_type_ptr, char *delivery_type_str) {
  char **subscription = (char **)malloc(sizeof(char *) * 4); // Allocate for 4 char* pointers
  if (subscription == NULL) {
    return NULL;
  }

  // Initialize all pointers to NULL for safety
  for (int i = 0; i < 4; ++i) {
      subscription[i] = NULL;
  }

  if (setDeliveryType(&subscription, delivery_type_str) != 0) {
    // setDeliveryType failed, it didn't allocate subscription[1] or it failed freeing if it did.
    // In this case, subscription[1] is NULL.
    free(subscription);
    return NULL;
  }

  size_t name_len = strlen(subscription_name);
  subscription[0] = (char *)malloc(name_len + 1); // Allocate for subscription name
  if (subscription[0] == NULL) {
    free(subscription[1]); // Free the delivery type string if name allocation fails
    free(subscription);
    return NULL;
  }

  memset(subscription[0], 0, name_len + 1);
  strncpy(subscription[0], subscription_name, name_len);
  subscription[2] = channel_delivery_type_ptr; // Store the channel's delivery type pointer directly
  subscription[3] = NULL;                      // Next subscription in list, initialized to NULL

  return subscription;
}

// Function: getChannel
// Iterates a linked list of channel structures (char**), comparing channel names.
// param_1: Head of the channel list.
// param_2: Name of the channel to find.
char ** getChannel(char **head_channel, char *channel_name) {
  for (char **current_channel = head_channel; current_channel != NULL; current_channel = (char **)current_channel[5]) {
    if (strcmp(current_channel[0], channel_name) == 0) { // current_channel[0] is the channel name
      return current_channel;
    }
  }
  return NULL;
}

// Function: getSubscription
// Iterates a linked list of subscription structures (char**), comparing subscription names.
// param_1: Head of the subscription list.
// param_2: Name of the subscription to find.
char ** getSubscription(char **head_subscription, char *subscription_name) {
  for (char **current_subscription = head_subscription; current_subscription != NULL; current_subscription = (char **)current_subscription[3]) {
    if (strcmp(current_subscription[0], subscription_name) == 0) { // current_subscription[0] is the subscription name
      return current_subscription;
    }
  }
  return NULL;
}

// Function: getMessageById
// Iterates a linked list of message structures (void*), comparing message IDs.
// Assumes the message ID is at offset 0 and the next pointer is at offset 4*sizeof(int).
// param_1: Head of the message list.
// param_2: ID of the message to find.
void * getMessageById(void *head_message, int message_id) {
  for (void *current_message = head_message; current_message != NULL; current_message = *(void **)((char *)current_message + sizeof(int) * 4)) {
    if (message_id == *(int *)current_message) { // Assume message ID is at offset 0
      return current_message;
    }
  }
  return NULL;
}

// Function: getLastMessage
// Iterates a linked list of message structures (void*) to find the last message.
// The 'next' pointer is assumed to be at offset 0x10 (16 bytes) within the message structure.
// param_1: Head of the message list.
void * getLastMessage(void *head_message) {
  if (head_message == NULL) {
    return NULL;
  }
  
  void *current_message = head_message;
  while (*(void **)((char *)current_message + 0x10) != NULL) { // Next pointer at offset 0x10
    current_message = *(void **)((char *)current_message + 0x10);
  }
  return current_message;
}

// Function: cleanupChannel
// Cleans up messages in a channel based on a minimum ID threshold.
// param_1: Pointer to the channel structure (void*).
void cleanupChannel(void *channel_ptr) {
  if (channel_ptr == NULL) return;

  // Assume channel_ptr points to a structure where:
  // Offset 0x08: unsigned int min_message_id_to_keep_threshold
  // Offset 0x10: void* head_of_message_list_A (used for finding actual min ID)
  // Offset 0x0c: void* head_of_message_list_B (updated by removing elements)
  // Offset 0x04: unsigned int current_processed_message_id
  // Message struct:
  // Offset 0x08: unsigned int message_id (used for comparison)
  // Offset 0x0c: void* next_message_in_list_A
  // Offset 0x10: void* next_message_in_list_B

  unsigned int min_id_threshold = *(unsigned int *)((char *)channel_ptr + 8);
  unsigned int actual_min_id_in_list = min_id_threshold;

  // First loop: Find the true minimum message ID among messages in list A (at offset 0x10 from channel_ptr)
  // This loop iterates a list where the next pointer is at offset 0xc in the message struct itself.
  for (void *msg_in_list_A = *(void **)((char *)channel_ptr + 0x10);
       msg_in_list_A != NULL;
       msg_in_list_A = *(void **)((char *)msg_in_list_A + 0xc)) {
    if (*(unsigned int *)((char *)msg_in_list_A + 8) < actual_min_id_in_list) {
      actual_min_id_in_list = *(unsigned int *)((char *)msg_in_list_A + 8);
    }
  }

  // Second loop: Remove messages from the head of list B (at offset 0x0c from channel_ptr)
  // until the channel's current_processed_message_id (at offset 0x04 from channel_ptr) reaches `actual_min_id_in_list`.
  while (*(unsigned int *)((char *)channel_ptr + 4) < actual_min_id_in_list) {
    void **head_of_list_B_ptr = (void **)((char *)channel_ptr + 0xc);
    void *current_head_message_B = *head_of_list_B_ptr;

    if (current_head_message_B == NULL) {
        break; // List B is empty
    }

    // Advance head of list B. The next pointer for this list is at offset 0x10 in the message struct.
    *head_of_list_B_ptr = *(void **)((char *)current_head_message_B + 0x10);
    
    // Increment the processed message ID counter for the channel.
    *(unsigned int *)((char *)channel_ptr + 4) = *(unsigned int *)((char *)channel_ptr + 4) + 1;
    // NOTE: The original code does not free the removed messages, leading to a memory leak.
  }
}

// Function: newChannel
// Allocates and initializes a new channel structure (char*[6]).
// param_1: The channel name string.
char ** newChannel(char *channel_name) {
  char **channel = (char **)malloc(sizeof(char *) * 6); // Allocate for 6 char* pointers
  if (channel == NULL) {
    return NULL;
  }

  // Initialize all pointers to NULL for safety
  for (int i = 0; i < 6; ++i) {
      channel[i] = NULL;
  }

  size_t name_len = strlen(channel_name);
  channel[0] = (char *)malloc(name_len + 1); // Allocate for channel name
  if (channel[0] == NULL) {
    free(channel);
    return NULL;
  }
  
  memset(channel[0], 0, name_len + 1);
  strcpy(channel[0], channel_name);
  
  // Other pointers (channel[1] to channel[5]) are already NULL from initialization loop.
  // channel[1] : (unused in newChannel)
  // channel[2] : delivery type string (set in newSubscription)
  // channel[3] : message list head
  // channel[4] : subscription list head
  // channel[5] : next channel in global list

  return channel;
}

// Function: addSubscriptions
// Manages adding subscriptions to a channel or creating a new channel/subscription.
// param_1: Pointer to the head of the global channel linked list (char***).
// param_2: Pointer to the head of a specific subscription linked list (char***).
// param_3: The name of the new subscription to potentially add (char*).
// param_4: The name of the channel to operate on (char*).
void addSubscriptions(char ***head_channel_list_ptr, char ***head_subscription_list_ptr, char *new_subscription_name, char *channel_name) {
  char **channel = getChannel(*head_channel_list_ptr, channel_name);
  if (channel == NULL) {
    channel = newChannel(channel_name);
    if (channel == NULL) {
      return; // Failed to create new channel
    }
    channel[5] = *head_channel_list_ptr; // Link new channel to the head of the list
    *head_channel_list_ptr = channel;    // Update head of the channel list
  }

  // Now 'channel' points to the desired channel structure.
  // Check if a subscription with the channel's name already exists in the list pointed to by head_subscription_list_ptr.
  char **found_subscription = getSubscription(*head_subscription_list_ptr, channel[0]); // channel[0] is the channel name

  if (found_subscription == NULL) {
    // Subscription does not exist, create two new subscriptions.

    // 1. Create a subscription based on the channel's name and delivery type.
    char **new_sub_for_channel = newSubscription(channel[0], channel[2], "latest"); // param_1: channel name, param_2: channel delivery type ptr, param_3: "latest"
    if (new_sub_for_channel == NULL) {
        return; // Failed to create subscription
    }
    // Link this new subscription to the list pointed to by head_subscription_list_ptr.
    new_sub_for_channel[3] = *head_subscription_list_ptr; // new_sub_for_channel[3] is the 'next' pointer
    *head_subscription_list_ptr = new_sub_for_channel;    // Update head of the list

    // 2. Create another subscription using `new_subscription_name` and link it to the channel's internal subscription list (channel[4]).
    char **new_sub_for_param3 = newSubscription(new_subscription_name, channel[2], "latest"); // param_1: new_subscription_name, param_2: channel delivery type ptr, param_3: "latest"
    if (new_sub_for_param3 == NULL) {
        // NOTE: new_sub_for_channel was allocated but not freed here if this fails. Potential leak.
        // Original code does not free it either.
        return;
    }
    // Link this new subscription to the list starting at channel[4].
    new_sub_for_param3[3] = channel[4]; // new_sub_for_param3[3] is the 'next' pointer
    channel[4] = new_sub_for_param3;     // Update channel[4] to point to this new subscription
  } else {
    // Subscription already exists, update its associated channel delivery type.
    // found_subscription[2] is the pointer to the channel's delivery type string.
    found_subscription[2] = channel[2];
  }
}