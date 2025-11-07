#include <stdlib.h> // For malloc, free, calloc, atoi
#include <string.h> // For strlen, strncpy, strcmp
#include <stdio.h>  // For NULL, but stddef.h is also an option
#include <stdint.h> // For uint32_t or similar if needed, using unsigned int for undefined4

// Type aliases for clarity, assuming 32-bit pointer sizes based on offsets
// A "Subscription" object is a char** pointing to an array of 4 char*
// [0]: Subscription Name (char*)
// [1]: Delivery Type (char*)
// [2]: Data/Payload (char*)
// [3]: Pointer to next Subscription in a list (char**)
typedef char** Subscription;

// A "Channel" object is a char** pointing to an array of 6 char*
// [0]: Channel Name (char*)
// [1]: Head of a Subscription list (Subscription) or counter
// [2]: Some Data (char*) or counter
// [3]: Pointer to next Channel in a *subscription-related* list (Channel) or Message list head
// [4]: Head of another Subscription list (Subscription) or Message list head
// [5]: Pointer to next Channel in the main Channel list (Channel)
typedef char** Channel;

// Function: setDeliveryType - forward declaration needed for newSubscription
int setDeliveryType(char ***subscription_ref, char *delivery_type_str);

// Function: newSubscription
char **newSubscription(char *name_param, char *data2_param, char *delivery_type_str_param) {
  Subscription subscription = (Subscription)calloc(4, sizeof(char *)); // 0x10 bytes for 4 char*
  if (!subscription) {
    return NULL;
  }

  if (setDeliveryType(&subscription, delivery_type_str_param) != 0) {
    free(subscription);
    return NULL;
  }

  size_t name_len = strlen(name_param);
  subscription[0] = (char *)malloc(name_len + 1);
  if (!subscription[0]) {
    free(subscription[1]); // This was allocated by setDeliveryType
    free(subscription);
    return NULL;
  }
  strncpy(subscription[0], name_param, name_len);
  subscription[0][name_len] = '\0'; // Ensure null termination

  subscription[2] = data2_param; // Direct assignment, not a copy
  // subscription[3] is already NULL from calloc

  return subscription;
}

// Function: getChannel
char **getChannel(Channel head, char *name) {
  for (Channel current = head; current != NULL; current = (Channel)current[5]) {
    if (strcmp(current[0], name) == 0) {
      return current;
    }
  }
  return NULL;
}

// Function: getSubscription
char **getSubscription(Subscription head, char *name) {
  for (Subscription current = head; current != NULL; current = (Subscription)current[3]) {
    if (strcmp(current[0], name) == 0) {
      return current;
    }
  }
  return NULL;
}

// Function: setDeliveryType
int setDeliveryType(char ***subscription_ref, char *delivery_type_str) {
  Subscription subscription = *subscription_ref;

  // Original logic checked for predefined strings OR an integer >= 1.
  // Both branches performed the same string allocation and copy.
  // Consolidating the string copy logic.
  if (!(strcmp("guaranteed", delivery_type_str) == 0 ||
        strcmp("latest", delivery_type_str) == 0 ||
        strcmp("high", delivery_type_str) == 0 ||
        strcmp("medium", delivery_type_str) == 0 ||
        strcmp("low", delivery_type_str) == 0)) {
    // If not a predefined string, check if it's a valid positive integer
    if (atoi(delivery_type_str) < 1) {
      return 1; // Invalid delivery type string
    }
  }

  size_t type_len = strlen(delivery_type_str);
  subscription[1] = (char *)malloc(type_len + 1);
  if (!subscription[1]) {
    return 1; // Allocation failed
  }
  strncpy(subscription[1], delivery_type_str, type_len);
  subscription[1][type_len] = '\0'; // Ensure null termination

  return 0; // Success
}

// Function: getMessageById
int *getMessageById(int *head, int id) {
  for (int *current = head; current != NULL; current = (int *)current[4]) {
    if (id == current[0]) {
      return current;
    }
  }
  return NULL;
}

// Function: getLastMessage
void *getLastMessage(void *head_ptr) {
  if (!head_ptr) {
    return NULL;
  }
  void *current = head_ptr;
  // Assuming 32-bit environment where 0x10 is the offset for the 4th int/pointer field (index 4)
  while (*(void **)((char *)current + 0x10) != NULL) {
    current = *(void **)((char *)current + 0x10);
  }
  return current;
}

// Function: cleanupChannel
void cleanupChannel(void *channel_ptr_as_void) {
  // Cast the generic pointer to a Channel type for consistent access
  Channel channel = (Channel)channel_ptr_as_void;

  // Assuming 32-bit environment (sizeof(char*) == 4)
  // channel[2] is 0x8 offset
  unsigned int min_message_id = *(unsigned int *)(channel + 2);

  // channel[4] is 0x10 offset, interpreted as head of a message list
  for (void *msg_ptr_as_void = channel[4];
       msg_ptr_as_void != NULL;
       // msg_ptr[3] is 0xc offset, interpreted as next message pointer
       msg_ptr_as_void = *(void **)((char *)msg_ptr_as_void + 0xc)) {
    // msg_ptr[2] is 0x8 offset, interpreted as message ID
    if (*(unsigned int *)((char *)msg_ptr_as_void + 8) < min_message_id) {
      min_message_id = *(unsigned int *)((char *)msg_ptr_as_void + 8);
    }
  }

  // channel[1] is 0x4 offset, interpreted as a counter/ID
  while (*(unsigned int *)(channel + 1) < min_message_id) {
    // channel[3] is 0xc offset, interpreted as another message list head
    // channel[3] = channel[3][4] (where channel[3] is a message pointer, and [4] is its next field)
    *(void **)(channel + 3) = *(void **)(*(char **)(channel + 3) + 0x10);
    // channel[1]++
    *(unsigned int *)(channel + 1) = *(unsigned int *)(channel + 1) + 1;
  }
}

// Function: newChannel
char **newChannel(char *name_param) {
  Channel channel = (Channel)calloc(6, sizeof(char *)); // 0x18 bytes for 6 char*
  if (!channel) {
    return NULL;
  }

  size_t name_len = strlen(name_param);
  channel[0] = (char *)malloc(name_len + 1);
  if (!channel[0]) {
    free(channel);
    return NULL;
  }
  strncpy(channel[0], name_param, name_len);
  channel[0][name_len] = '\0'; // Ensure null termination

  // Fields channel[1] through channel[5] are already NULL due to calloc

  return channel;
}

// Function: addSubscriptions
void addSubscriptions(Channel **channel_list_head_ref, Subscription **subscription_list_head_ref,
                      char *new_subscription_name, char *target_channel_name) {
  Channel current_channel = getChannel(*channel_list_head_ref, target_channel_name);
  if (!current_channel) {
    current_channel = newChannel(target_channel_name);
    if (!current_channel) {
      return;
    }
    current_channel[5] = (char *)*channel_list_head_ref; // Link new channel to existing list
    *channel_list_head_ref = current_channel;             // Update head of channel list
  }

  // Check if a subscription exists for this channel's name in the main subscription list
  Subscription existing_subscription =
      getSubscription(*subscription_list_head_ref, current_channel[0]);
  if (!existing_subscription) {
    // Create the first new subscription, using the channel's name and data2
    Subscription new_sub_1 = newSubscription(current_channel[0], current_channel[2], "latest");
    if (!new_sub_1) {
      return;
    }
    new_sub_1[3] = (char *)*subscription_list_head_ref; // Link to existing subscription list
    *subscription_list_head_ref = new_sub_1;             // Update head of global subscription list

    // Create a second new subscription, using the provided new_subscription_name
    Subscription new_sub_2 = newSubscription(new_subscription_name, current_channel[2], "latest");
    if (!new_sub_2) {
      // If new_sub_2 fails, new_sub_1 has already been linked.
      // A more robust error handling would unwind new_sub_1, but original code just returns.
      return;
    }
    new_sub_2[3] = current_channel[4];          // Link to channel's alt subscription list
    current_channel[4] = (char *)new_sub_2; // Update head of channel's alt subscription list
  } else {
    // If subscription already exists, update its data2 field
    existing_subscription[2] = current_channel[2];
  }
}