#include <stdlib.h> // For calloc, free, exit
#include <string.h> // For memcpy, memcmp
#include <stdio.h>  // For NULL, general C operations
#include <stdbool.h> // For bool
#include <limits.h> // For UINT_MAX

// --- Type definitions (from Ghidra, adapted to standard C) ---
typedef unsigned char byte;
typedef unsigned short undefined2;
typedef unsigned int undefined4;

// Standard types for clarity
typedef unsigned int uint;
typedef unsigned short ushort;

// --- Ghidra-like macro definitions for bitwise operations ---
// CONCAT4 combines 4 bytes into a 32-bit unsigned integer.
#define CONCAT4(b3, b2, b1, b0) (((unsigned int)(b3) << 24) | ((unsigned int)(b2) << 16) | ((unsigned int)(b1) << 8) | (unsigned int)(b0))

// CARRY4(a, b) checks for unsigned integer overflow when adding a and b.
#define CARRY4(a, b) ((unsigned int)(a) > (unsigned int)(UINT_MAX - (b)))

// _terminate() is a placeholder for exiting the program on critical errors.
#define _terminate() exit(1)

// Forward declarations for functions used before their definition
extern void *tenfourd_ingest(int param_1, undefined4 param_2);
extern undefined4 tenfourd_validate(void *param_1, void *param_2);
extern void tenfourd_append(int *param_1, int param_2);
extern short *taxpayer_get_tenfourd_by_taxyear(int param_1, short param_2);


// Function: taxpayer_append
void taxpayer_append(int *head_ptr, int new_node) {
  if (*head_ptr == 0) {
    *head_ptr = new_node;
  } else {
    int current = *head_ptr;
    int last_node = *head_ptr; // Tracks the last non-null node
    while (current != 0) {
      last_node = current;
      current = *(int *)(last_node + 0x2a4); // Assuming 0x2a4 is the offset for the 'next' pointer
    }
    *(int *)(last_node + 0x2a4) = new_node;
  }
}

// Function: taxpayer_new
// param_1: source data for memcpy (e.g., template taxpayer struct)
// param_2: destination for memcpy (e.g., specific field in another struct)
// taxpayer_list_head_ptr: pointer to the head of the taxpayer list (int *), where the new taxpayer struct will be appended
void taxpayer_new(int param_1, int param_2, int *taxpayer_list_head_ptr) {
  void *new_taxpayer_struct; // Represents local_18
  int magic_number = 0x4347c000; // Represents local_14

  // Allocate 0x2a8 bytes for the new taxpayer structure
  new_taxpayer_struct = calloc(1, 0x2a8);
  if (new_taxpayer_struct == NULL) {
    _terminate(); // Exit on allocation failure
  }

  // First memcpy: copy 0x288 bytes from param_1 + 0x2e to new_taxpayer_struct
  memcpy(new_taxpayer_struct, (void *)(param_1 + 0x2e), 0x288);

  // Second memcpy: copy 0xc bytes from param_1 + 0xc to new_taxpayer_struct + 0x288
  memcpy((char *)new_taxpayer_struct + 0x288, (void *)(param_1 + 0xc), 0xc);

  // XOR encryption/hashing loop
  for (byte i = 0; i < 0xc; i++) {
    *(byte *)((char *)new_taxpayer_struct + i + 0x294) =
        *(byte *)(param_1 + i) ^ *(byte *)(magic_number + *(byte *)(param_1 + i));
  }

  // Third memcpy: copy 0xc bytes from new_taxpayer_struct + 0x294 to param_2 + 0xc
  memcpy((char *)param_2 + 0xc, (char *)new_taxpayer_struct + 0x294, 0xc);

  // Append the newly created taxpayer struct to the list
  taxpayer_append(taxpayer_list_head_ptr, (int)new_taxpayer_struct);
}

// Function: taxpayer_get_by_username
int taxpayer_get_by_username(int head_node, int username_info_ptr) {
  int current_node = head_node;
  while (current_node != 0) {
    // Compare 0xc bytes from username_info_ptr + 0xc with current_node + 0x288
    if (memcmp((void *)(username_info_ptr + 0xc), (void *)(current_node + 0x288), 0xc) == 0) {
      return current_node;
    }
    current_node = *(int *)(current_node + 0x2a4); // Next node
  }
  return 0; // Not found
}

// Function: taxpayer_compare_creds
undefined4 taxpayer_compare_creds(int taxpayer_struct_ptr, int creds_info_ptr) {
  // Compare username (0x288 offset)
  if (memcmp((void *)(taxpayer_struct_ptr + 0x288), (void *)(creds_info_ptr + 0xc), 0xc) == 0) {
    // If username matches, compare password (0x294 offset)
    if (memcmp((void *)(taxpayer_struct_ptr + 0x294), (void *)(creds_info_ptr + 0x18), 0xc) == 0) {
      return 0; // Match
    }
  }
  return 0xffffffff; // No match
}

// Function: taxpayer_add_tenfourdee
undefined4 taxpayer_add_tenfourdee(int taxpayer_struct_ptr, undefined4 param_2, undefined4 param_3) {
  void *new_tenfourd_struct = tenfourd_ingest(param_2, param_3);
  if (new_tenfourd_struct == NULL) {
    return 0xffffffff; // Ingest failed
  }

  // Validate the new 1040-D form against the taxpayer
  if (tenfourd_validate(new_tenfourd_struct, (void *)taxpayer_struct_ptr) == 0) {
    // If valid, append to the taxpayer's list of 1040-D forms
    tenfourd_append((int *)(taxpayer_struct_ptr + 0x2a0), (int)new_tenfourd_struct);
    return 0; // Success
  } else {
    free(new_tenfourd_struct); // Free if validation fails
    return 0xffffffff;         // Validation failed
  }
}

// Function: taxpayer_get_tenfourd_by_taxyear
short *taxpayer_get_tenfourd_by_taxyear(int taxpayer_struct_ptr, short tax_year) {
  short *current_tenfourd = *(short **)(taxpayer_struct_ptr + 0x2a0); // Head of 1040-D list
  while (current_tenfourd != NULL) {
    if (tax_year == *current_tenfourd) { // Assuming tax_year is the first field (offset 0)
      return current_tenfourd;
    }
    current_tenfourd = *(short **)((char *)current_tenfourd + 0x2ef); // Next 1040-D form
  }
  return NULL; // Not found
}

// Function: taxpayer_sum_taxes_due
// taxpayer_struct_ptr: taxpayer struct
// tax_year_range_ptr: struct containing tax year range (ushort[2] at offset 0x2e)
// total_taxes_due: pointer to an int where the sum will be stored
undefined4 taxpayer_sum_taxes_due(int taxpayer_struct_ptr, int tax_year_range_ptr, int *total_taxes_due) {
  // Assuming (tax_year_range_ptr + 0x2e) gives a pointer to a ushort pointer
  ushort *year_range_data = *(ushort **)(tax_year_range_ptr + 0x2e);

  if (year_range_data[1] < year_range_data[0]) { // Check if end year is before start year
    return 0xffffffff;
  }

  // Iterate through the taxpayer's 1040-D forms
  for (ushort *current_tenfourd = *(ushort **)(taxpayer_struct_ptr + 0x2a0);
       current_tenfourd != NULL;
       current_tenfourd = *(ushort **)((char *)current_tenfourd + 0x2ef)) {

    // Check if the 1040-D's tax year falls within the specified range
    if ((year_range_data[0] <= *current_tenfourd) && (*current_tenfourd <= year_range_data[1])) {
      // Access fields using byte offsets from current_tenfourd (char*)
      if (*(int *)((char *)current_tenfourd + 0x2e3) == 0) { // Check a specific flag/field
        *total_taxes_due += (*(int *)((char *)current_tenfourd + 0x2eb) - *(int *)((char *)current_tenfourd + 0x2e7));
      } else {
        *total_taxes_due += (*(int *)((char *)current_tenfourd + 0x2e3) - *(int *)((char *)current_tenfourd + 0x2eb));
      }
    }
  }
  return 0; // Success
}

// Function: taxpayer_list_submitted_tax_years
// taxpayer_struct_ptr: taxpayer struct
// tax_year_range_ptr: struct containing tax year range (ushort[2] at offset 0x2e)
// output_years_array: ushort array to store the list of years
// output_size_bytes: pointer to an int where the total size (in bytes) of the list will be stored
undefined4 taxpayer_list_submitted_tax_years(int taxpayer_struct_ptr, int tax_year_range_ptr, ushort *output_years_array, int *output_size_bytes) {
  ushort *year_range_data = *(ushort **)(tax_year_range_ptr + 0x2e);

  if (year_range_data[1] < year_range_data[0]) {
    return 0xffffffff;
  }

  ushort *current_output_ptr = output_years_array;
  int years_count_bytes = 0;

  for (ushort *current_tenfourd = *(ushort **)(taxpayer_struct_ptr + 0x2a0);
       current_tenfourd != NULL;
       current_tenfourd = *(ushort **)((char *)current_tenfourd + 0x2ef)) {
    if ((year_range_data[0] <= *current_tenfourd) && (*current_tenfourd <= year_range_data[1])) {
      *current_output_ptr = *current_tenfourd;
      years_count_bytes += sizeof(ushort); // Each year is a ushort (2 bytes)
      current_output_ptr++;
    }
  }

  *output_size_bytes = years_count_bytes;
  return 0;
}

// Function: taxpayer_count_submitted_tax_years
int taxpayer_count_submitted_tax_years(int taxpayer_struct_ptr) {
  int count = 0;
  for (int current_tenfourd = *(int *)(taxpayer_struct_ptr + 0x2a0);
       current_tenfourd != 0;
       current_tenfourd = *(int *)(current_tenfourd + 0x2ef)) {
    count++;
  }
  return count;
}

// Function: taxpayer_get_refund
// taxpayer_struct_ptr: taxpayer struct
// unused_param_2: undefined4 (unused in body)
// refund_info_array: int array to store refund info (first element total refund, subsequent elements tax years)
// output_array_size_ptr: pointer to int, input max size of refund_info_array in bytes, output actual size used
undefined4 taxpayer_get_refund(int taxpayer_struct_ptr, undefined4 unused_param_2, int *refund_info_array, int *output_array_size_ptr) {
  undefined2 *current_tenfourd = *(undefined2 **)(taxpayer_struct_ptr + 0x2a0);
  uint current_output_size_bytes = 0; // Tracks bytes written to output_array for years
  int total_refund_amount = 0;
  // Start writing years after the total refund amount (which takes sizeof(int) bytes)
  int *current_output_year_ptr = refund_info_array + 1;

  uint max_output_size_bytes = *output_array_size_ptr;

  // Loop while there are 1040-D forms and space in the output array
  // (max_output_size_bytes - sizeof(int)) accounts for the initial total_refund_amount
  while ((current_tenfourd != NULL) && (current_output_size_bytes < max_output_size_bytes - sizeof(int))) {
    // Check if there's a refund due for this form
    if ((*(int *)((char *)current_tenfourd + 0x2e7) != 0) &&
        (*(uint *)((char *)current_tenfourd + 0x2eb) < *(uint *)((char *)current_tenfourd + 0x2e7))) {
      int refund_amount_for_form = *(int *)((char *)current_tenfourd + 0x2e7) - *(int *)((char *)current_tenfourd + 0x2eb);
      *(int *)((char *)current_tenfourd + 0x2eb) += refund_amount_for_form; // Mark as refunded
      total_refund_amount += refund_amount_for_form;
      *(undefined2 *)current_output_year_ptr = *current_tenfourd; // Store tax year
      current_output_size_bytes += sizeof(undefined2); // Size of ushort (2 bytes)
      current_output_year_ptr = (int *)((char *)current_output_year_ptr + sizeof(undefined2)); // Advance pointer
    }
    current_tenfourd = *(undefined2 **)((char *)current_tenfourd + 0x2ef); // Next form
  }

  if (total_refund_amount == 0) {
    return 0xffffffff; // No refund
  } else {
    *refund_info_array = total_refund_amount; // Store total refund in the first element
    *output_array_size_ptr = current_output_size_bytes + sizeof(int); // Total bytes used: total refund (4 bytes) + years
    return 0; // Success
  }
}

// Function: taxpayer_pay_taxes
// taxpayer_struct_ptr: taxpayer struct
// payment_info_ptr: struct containing payment amount (uint at offset 0x2e, address of pointer)
// output_years_paid_array: undefined2 array to store the list of tax years paid
// output_array_ushort_count_ptr: pointer to uint, input max count of ushorts for output, output actual count of ushorts written
undefined4 taxpayer_pay_taxes(int taxpayer_struct_ptr, int payment_info_ptr, undefined2 *output_years_paid_array, uint *output_array_ushort_count_ptr) {
  // Assuming payment_info_ptr + 0x2e points to a pointer, which then points to the uint payment amount
  uint remaining_payment_amount = *(uint *)*(void **)(payment_info_ptr + 0x2e);

  if (remaining_payment_amount == 0) {
    return 0xffffffff; // No payment amount
  }

  undefined2 *current_tenfourd = *(undefined2 **)(taxpayer_struct_ptr + 0x2a0);
  undefined2 *current_output_ptr = output_years_paid_array;
  uint max_output_ushorts = *output_array_ushort_count_ptr;
  uint ushorts_written_count = 0;
  uint amount_paid_this_iteration = 0; // Tracks if any payment was made in this call

  // Loop while there are 1040-D forms, payment remaining, and space in the output array
  while ((current_tenfourd != NULL) && (remaining_payment_amount != 0) && (ushorts_written_count < max_output_ushorts)) {
    // Check if taxes are due for this form
    if ((*(int *)((char *)current_tenfourd + 0x2e3) != 0) &&
        (*(uint *)((char *)current_tenfourd + 0x2eb) < *(uint *)((char *)current_tenfourd + 0x2e3))) {

      uint amount_due_for_form = *(uint *)((char *)current_tenfourd + 0x2e3) - *(uint *)((char *)current_tenfourd + 0x2eb);
      amount_paid_this_iteration = amount_due_for_form;

      if (remaining_payment_amount < amount_paid_this_iteration) {
        amount_paid_this_iteration = remaining_payment_amount; // Pay what we can
      }

      *(uint *)((char *)current_tenfourd + 0x2eb) += amount_paid_this_iteration; // Update paid amount
      remaining_payment_amount -= amount_paid_this_iteration;

      *current_output_ptr = *current_tenfourd; // Store tax year
      ushorts_written_count++;
      current_output_ptr++;
    }
    current_tenfourd = *(undefined2 **)((char *)current_tenfourd + 0x2ef); // Next form
  }

  if (amount_paid_this_iteration == 0) { // If no taxes were paid in this run
    return 0xffffffff;
  } else {
    *output_array_ushort_count_ptr = ushorts_written_count;
    return 0; // Success
  }
}

// Function: tenfourd_append
void tenfourd_append(int *head_ptr, int new_node) {
  if (*head_ptr == 0) {
    *head_ptr = new_node;
  } else {
    int current = *head_ptr;
    int last_node = *head_ptr;
    while (current != 0) {
      last_node = current;
      current = *(int *)(last_node + 0x2ef); // Offset for next pointer in 1040-D struct
    }
    *(int *)(last_node + 0x2ef) = new_node;
  }
}

// Function: tenfourd_get_last_three_from_list
// head_node: head of 1040-D list
// total_count: total count of elements in the list
// output_array_ptr: destination address for the output array (stores ushorts)
void tenfourd_get_last_three_from_list(undefined2 *head_node, uint total_count, int output_array_ptr) {
  undefined2 *current_node = head_node;
  uint elements_to_skip = total_count;

  // Skip elements until we are at most 3 elements from the end
  for (; elements_to_skip > 3; elements_to_skip--) {
    current_node = *(undefined2 **)((char *)current_node + 0x2ef);
  }

  // Copy the last `elements_to_skip` (which will be 1, 2, or 3) elements to the output array
  for (uint i = 0; i < elements_to_skip; i++) {
    *(undefined2 *)((char *)output_array_ptr + (i * sizeof(undefined2))) = *current_node;
    current_node = *(undefined2 **)((char *)current_node + 0x2ef);
  }
}

// Function: tenfourd_ingest
// param_1: source data for memcpy (e.g., template 1040-D struct)
// copy_size: size for memcpy
void *tenfourd_ingest(int param_1, undefined4 copy_size) {
  void *new_tenfourd_struct;

  // Allocate 0x2f3 bytes for the new 1040-D structure
  new_tenfourd_struct = calloc(1, 0x2f3);
  if (new_tenfourd_struct == NULL) {
    _terminate(); // Exit on allocation failure
  }

  // Copy 'copy_size' bytes from param_1 + 0x2e to new_tenfourd_struct
  memcpy(new_tenfourd_struct, (void *)(param_1 + 0x2e), copy_size);
  return new_tenfourd_struct;
}

// Function: tenfourd_validate
// new_tenfourd_form_void: new 1040-D form struct (void*, treated as byte array for offsets)
// taxpayer_struct_ptr: taxpayer struct (void*)
undefined4 tenfourd_validate(void *new_tenfourd_form_void, void *taxpayer_struct_ptr) {
  char *new_tenfourd_form = (char *)new_tenfourd_form_void; // Cast to char* for byte arithmetic
  undefined4 result = 0; // local_20, default success

  // Check if a 1040-D for this tax year already exists for the taxpayer
  if (taxpayer_get_tenfourd_by_taxyear((int)taxpayer_struct_ptr, *(short *)new_tenfourd_form) != NULL) {
    return 0xffffffff; // Already exists
  }

  // Check if the form's identity/metadata matches the taxpayer's
  // All offsets are byte offsets relative to the start of the structure.
  if (memcmp(taxpayer_struct_ptr, new_tenfourd_form + 0x2, 0x80) == 0 &&
      memcmp((char *)taxpayer_struct_ptr + 0x80, new_tenfourd_form + 0x82, 0x80) == 0 &&
      memcmp((char *)taxpayer_struct_ptr + 0x100, new_tenfourd_form + 0x102, 0x80) == 0 &&
      memcmp((char *)taxpayer_struct_ptr + 0x180, new_tenfourd_form + 0x182, 0x80) == 0 &&
      memcmp((char *)taxpayer_struct_ptr + 0x200, new_tenfourd_form + 0x202, 0x80) == 0 &&
      *(int *)((char *)taxpayer_struct_ptr + 0x284) == *(int *)(new_tenfourd_form + 0x282)) {

    // --- Checksum calculations and validation logic ---
    uint uVar1_val = *(uint *)((char *)taxpayer_struct_ptr + 0x284);
    uint local_88_val = uVar1_val; // For byte extraction

    uint local_84_checksum = CONCAT4(
        (*(byte *)(new_tenfourd_form + 0x183) ^ ((local_88_val >> 24) & 0xFF)),
        (*(byte *)(new_tenfourd_form + 0x103) ^ ((local_88_val >> 16) & 0xFF)),
        (*(byte *)(new_tenfourd_form + 0x41) ^ ((local_88_val >> 8) & 0xFF)),
        (*(byte *)(new_tenfourd_form + 0x1) ^ (local_88_val & 0xFF)) // new_tenfourd_form + 1
    );

    // Check flag/field at 0x143 and 0x287
    if (*(char *)(new_tenfourd_form + 0x143) == 'N' &&
        *(int *)(new_tenfourd_form + 0x287) != 0) {
      result = 0xfffffffd;
    } else if (*(char *)(new_tenfourd_form + 0x143) == 'Y' &&
               (*(int *)(new_tenfourd_form + 0x287) == 0 ||
                *(int *)(new_tenfourd_form + 0x28b) == 0 ||
                4 < *(uint *)(new_tenfourd_form + 0x28b))) {
      result = 0xfffffffc;
    } else {
      uint local_34_val = *(uint *)(new_tenfourd_form + 0x287);
      int local_30_val = 0; // Kept as it is used in a calculation, but value is 0.

      // Calculate sums and carries for different sections
      uint sum1_a = *(uint *)(new_tenfourd_form + 0x28f) + *(uint *)(new_tenfourd_form + 0x293);
      uint sum2_a = sum1_a + *(uint *)(new_tenfourd_form + 0x297);
      uint sum3_a = sum2_a + *(uint *)(new_tenfourd_form + 0x29b);
      uint local_90_carry_sum = CARRY4(*(uint *)(new_tenfourd_form + 0x28f), *(uint *)(new_tenfourd_form + 0x293)) +
                                CARRY4(sum1_a, *(uint *)(new_tenfourd_form + 0x297)) +
                                CARRY4(sum2_a, *(uint *)(new_tenfourd_form + 0x29b));
      uint local_94_val = sum3_a; // Used to extract bytes for checksum

      uint local_80_checksum = CONCAT4(
          ((local_94_val >> 24) & 0xFF) ^ *(byte *)(new_tenfourd_form + 0xc2) ^ *(byte *)(new_tenfourd_form + 0x205),
          ((local_94_val >> 16) & 0xFF) ^ *(byte *)(new_tenfourd_form + 0x82) ^ *(byte *)(new_tenfourd_form + 0x102),
          ((local_94_val >> 8) & 0xFF) ^ *(byte *)(new_tenfourd_form + 0x83) ^ *(byte *)(new_tenfourd_form + 0x203),
          (local_94_val & 0xFF) ^ *(byte *)(new_tenfourd_form + 0x3) ^ *(byte *)(new_tenfourd_form + 0x101)
      );

      sum1_a = *(uint *)(new_tenfourd_form + 0x29f) + *(uint *)(new_tenfourd_form + 0x2a3);
      sum2_a = sum1_a + *(uint *)(new_tenfourd_form + 0x2a7);
      uint local_98_carry_sum = CARRY4(*(uint *)(new_tenfourd_form + 0x29f), *(uint *)(new_tenfourd_form + 0x2a3)) +
                                CARRY4(sum1_a, *(uint *)(new_tenfourd_form + 0x2a7));
      uint local_9c_val = sum2_a; // Used to extract bytes for checksum

      uint local_7c_checksum = CONCAT4(
          ((local_9c_val >> 24) & 0xFF),
          ((local_9c_val >> 16) & 0xFF),
          ((local_9c_val >> 8) & 0xFF),
          ((local_9c_val & 0xFF) ^ (local_90_carry_sum & 0xFF)) // XOR with LSB of local_90_carry_sum
      );

      sum1_a = *(uint *)(new_tenfourd_form + 0x2ab) + *(uint *)(new_tenfourd_form + 0x2af);
      sum2_a = sum1_a + *(uint *)(new_tenfourd_form + 0x2b3);
      sum3_a = sum2_a + *(uint *)(new_tenfourd_form + 0x2b7);
      uint local_a0_carry_sum = CARRY4(*(uint *)(new_tenfourd_form + 0x2ab), *(uint *)(new_tenfourd_form + 0x2af)) +
                                CARRY4(sum1_a, *(uint *)(new_tenfourd_form + 0x2b3)) +
                                CARRY4(sum2_a, *(uint *)(new_tenfourd_form + 0x2b7));
      uint local_a4_val = sum3_a; // Used to extract bytes for checksum

      uint local_78_checksum = CONCAT4(
          ((local_a4_val >> 24) & 0xFF),
          ((local_a4_val >> 16) & 0xFF),
          ((local_a4_val >> 8) & 0xFF),
          ((local_a4_val & 0xFF) ^ (local_98_carry_sum & 0xFF)) // XOR with LSB of local_98_carry_sum
      );

      bool bVar10_carry = CARRY4(*(uint *)(new_tenfourd_form + 0x2bb), *(uint *)(new_tenfourd_form + 0x2bf));
      uint sum_last = *(uint *)(new_tenfourd_form + 0x2bb) + *(uint *)(new_tenfourd_form + 0x2bf);
      uint local_a8_carry_sum = bVar10_carry; // The carry itself

      uint local_ac_val = sum_last; // Used to extract bytes for checksum

      uint local_74_checksum = CONCAT4(
          ((local_ac_val >> 24) & 0xFF),
          ((local_ac_val >> 16) & 0xFF),
          ((local_ac_val >> 8) & 0xFF),
          ((local_ac_val & 0xFF) ^ (local_a0_carry_sum & 0xFF)) // XOR with LSB of local_a0_carry_sum
      );

      uint local_70_checksum = CONCAT4(
          *(byte *)(new_tenfourd_form + 0xc4),
          *(byte *)(new_tenfourd_form + 0x107),
          *(byte *)(new_tenfourd_form + 0x43),
          (*(byte *)(new_tenfourd_form + 0x5) ^ bVar10_carry) // new_tenfourd_form + 5
      );

      // Compare calculated checksums with stored checksums in the form
      uint calculated_checksums[6];
      calculated_checksums[0] = local_84_checksum;
      calculated_checksums[1] = local_80_checksum;
      calculated_checksums[2] = local_7c_checksum;
      calculated_checksums[3] = local_78_checksum;
      calculated_checksums[4] = local_74_checksum;
      calculated_checksums[5] = local_70_checksum;

      if (memcmp(&calculated_checksums, new_tenfourd_form + 0x2c3, 0x18) == 0) {
        int local_24_sum = 0; // Sum of 8 bytes from 0x2db
        for (byte i = 0; i < 8; i++) {
          local_24_sum += *(char *)(new_tenfourd_form + 0x2db + i);
        }

        if (local_24_sum == 0) {
          result = 0xfffffffa;
        } else if (*(int *)(new_tenfourd_form + 0x2e3) == 0 ||
                   *(int *)(new_tenfourd_form + 0x2e7) == 0) {

          // Final complex arithmetic check
          uint uVar3_calc = (local_94_val >> 3) | (local_90_carry_sum << 0x1d);
          uint uVar4_calc = (local_9c_val >> 4) | (local_98_carry_sum << 0x1c);
          uint uVar5_calc = (local_a4_val >> 2) | (local_a0_carry_sum << 0x1e);

          uint uVar6_temp = uVar3_calc + local_34_val + 0x3039;
          uint uVar7_temp = uVar6_temp - uVar4_calc;
          uint uVar8_temp = uVar7_temp - uVar5_calc;
          uint uVar9_temp = uVar8_temp - local_ac_val;

          uint final_uVar3_val =
              ((local_90_carry_sum >> 3) + local_30_val + (uint)(0xffffcfc6U < local_34_val) + (uint)CARRY4(uVar3_calc, local_34_val + 0x3039)) -
              (local_98_carry_sum >> 4) - (uint)(uVar6_temp < uVar4_calc) -
              (local_a0_carry_sum >> 2) - (uint)(uVar7_temp < uVar5_calc) -
              local_a8_carry_sum - (uint)(uVar8_temp < local_ac_val);

          // The original Ghidra condition `((SBORROW4(-uVar3,(uint)(uVar9 != 0)) != false) == (int)(-uVar3 - (uint)(uVar9 != 0)) < 0)`
          // simplifies to `((signed int)(-final_uVar3_val) - (signed int)(uVar9_temp != 0) < 0)` as discussed in thoughts.
          if ((( (signed int)(-final_uVar3_val) - (signed int)(uVar9_temp != 0) < 0 ) ||
               ((*(uint *)(new_tenfourd_form + 0x2e3) ^ uVar9_temp | final_uVar3_val) == 0))) {

            if (((signed int)final_uVar3_val < 0) &&
                ((*(uint *)(new_tenfourd_form + 0x2e7) ^ (uint)-uVar9_temp | (uint)-(final_uVar3_val + (uVar9_temp != 0))) != 0)) {
              result = 0xfffffff7;
            } else if (((uVar9_temp | final_uVar3_val) == 0) &&
                       (*(int *)(new_tenfourd_form + 0x2e3) != 0 &&
                        *(int *)(new_tenfourd_form + 0x2e7) != 0)) {
              result = 0xfffffff6;
            } else {
              result = 0; // Final success
            }
          } else {
            result = 0xfffffff8;
          }
        } else {
          result = 0xfffffff9;
        }
      } else {
        result = 0xfffffffb; // Checksum mismatch
      }
    }
  } else {
    result = 0xfffffffe; // Identity/metadata mismatch
  }
  return result;
}