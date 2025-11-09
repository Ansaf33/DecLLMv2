#include <stdlib.h> // For malloc, exit, size_t
#include <string.h> // For memset, strlen, memcpy
#include <stdio.h>  // For printf, for _terminate replacement

// Decompiler-generated types, replaced with standard C types
// 'undefined4' is typically an unsigned 4-byte integer.
// 'undefined' as a pointer type is replaced with 'void*'.
typedef unsigned int undefined4;

// Placeholder for external functions and data
// Replaces _terminate() with a standard C exit function.
void _terminate() {
    fprintf(stderr, "Error: Memory allocation failed.\n");
    exit(EXIT_FAILURE);
}

// Assuming onsale_fn is a function pointer with no arguments and void return
void onsale_fn(void) {
    // Placeholder implementation
    // printf("onsale_fn called!\n");
}

// Assuming list_insert_at_end takes an item pointer and a list head pointer
void list_insert_at_end(void* item, void* list_head) {
    // Placeholder implementation
    (void)item;      // Suppress unused parameter warning
    (void)list_head; // Suppress unused parameter warning
    // printf("Item inserted into list.\n");
}

// External data references (assuming unsigned int based on usage)
unsigned int DAT_000196b8 = 0xDEADBEEF; // Placeholder value, replace with actual if known
unsigned int DAT_000196bc = 0xDEADBEEF;
unsigned int DAT_000196c0 = 0xDEADBEEF;
unsigned int DAT_000196c4 = 0xDEADBEEF;
unsigned int DAT_000196c8 = 0xDEADBEEF;
unsigned int DAT_000196cc = 0xDEADBEEF;
unsigned int DAT_000196d0 = 0xDEADBEEF;
unsigned int DAT_000196d4 = 0xDEADBEEF;
unsigned int DAT_000196d8 = 0xDEADBEEF;
unsigned int DAT_000196dc = 0xDEADBEEF;
unsigned int DAT_000196e0 = 0xDEADBEEF;
unsigned int DAT_000196e4 = 0xDEADBEEF;
unsigned int DAT_000196e8 = 0xDEADBEEF;
unsigned int DAT_000196ec = 0xDEADBEEF;
unsigned int DAT_000196f0 = 0xDEADBEEF;
unsigned int DAT_000196f4 = 0xDEADBEEF;
unsigned int DAT_000196f8 = 0xDEADBEEF;
unsigned int DAT_000196fc = 0xDEADBEEF;
unsigned int DAT_00019700 = 0xDEADBEEF;
unsigned int DAT_00019704 = 0xDEADBEEF;
unsigned int DAT_00019708 = 0xDEADBEEF;
unsigned int DAT_0001970c = 0xDEADBEEF;
unsigned int DAT_00019710 = 0xDEADBEEF;
unsigned int DAT_00019714 = 0xDEADBEEF;
unsigned int DAT_00019718 = 0xDEADBEEF;
unsigned int DAT_0001971c = 0xDEADBEEF;
unsigned int DAT_00019720 = 0xDEADBEEF;
unsigned int DAT_00019724 = 0xDEADBEEF;
unsigned int DAT_00019728 = 0xDEADBEEF;
unsigned int DAT_0001972c = 0xDEADBEEF;
unsigned int DAT_00019730 = 0xDEADBEEF;
unsigned int DAT_00019734 = 0xDEADBEEF;
unsigned int DAT_00019738 = 0xDEADBEEF;
unsigned int DAT_0001973c = 0xDEADBEEF;
unsigned int DAT_00019740 = 0xDEADBEEF;
unsigned int DAT_00019744 = 0xDEADBEEF;
unsigned int DAT_00019748 = 0xDEADBEEF;
unsigned int DAT_0001974c = 0xDEADBEEF;
unsigned int DAT_00019750 = 0xDEADBEEF;
unsigned int DAT_00019754 = 0xDEADBEEF;
unsigned int DAT_00019758 = 0xDEADBEEF;
unsigned int DAT_0001975c = 0xDEADBEEF;
unsigned int DAT_00019760 = 0xDEADBEEF;
unsigned int DAT_00019764 = 0xDEADBEEF;
unsigned int DAT_00019768 = 0xDEADBEEF;
unsigned int DAT_0001976c = 0xDEADBEEF;
unsigned int DAT_00019770 = 0xDEADBEEF;
unsigned int DAT_00019774 = 0xDEADBEEF;
unsigned int DAT_00019778 = 0xDEADBEEF;
unsigned int DAT_0001977c = 0xDEADBEEF;
unsigned int DAT_00019780 = 0xDEADBEEF;
unsigned int DAT_00019784 = 0xDEADBEEF;
unsigned int DAT_00019788 = 0xDEADBEEF;
unsigned int DAT_0001978c = 0xDEADBEEF;
unsigned int DAT_00019790 = 0xDEADBEEF;
unsigned int DAT_00019794 = 0xDEADBEEF;
unsigned int DAT_00019798 = 0xDEADBEEF;
unsigned int DAT_0001979c = 0xDEADBEEF;
unsigned int DAT_000197a0 = 0xDEADBEEF;
unsigned int DAT_000197a4 = 0xDEADBEEF;
unsigned int DAT_000197a8 = 0xDEADBEEF;
unsigned int DAT_000197ac = 0xDEADBEEF;
unsigned int DAT_000197b0 = 0xDEADBEEF;

// Strings referenced as &DAT_0001908f, etc.
// These are likely string literals stored in read-only data section.
const char DAT_0001908f[] = "xJbYjXbZt";
const char DAT_0001909c[] = "yZJtBwP";
const char DAT_000190ef[] = "DkXnJtKq";
const char DAT_000191c8[] = "lKjXgIdtQ";
const char DAT_0001937d[] = "ZmLKqXf";
const char DAT_0001945e[] = "gVfLgRzL";
const char DAT_000194eb[] = "jWwYqU";
const char DAT_0001969b[] = "rKndjYf";
const char DAT_000196a9[] = "ZzYlOq";
const char DAT_000196b6[] = "nZpXf";

// Structure definition for an inventory item based on offsets and total size (0x98)
typedef struct InventoryItem {
    char id[8];          // ID string, max 8 chars (e.g., "41916155")
    char name[128];      // Name string, takes up 0x88 - 8 = 128 bytes. Longest is 31 chars.
    undefined4 field_88; // Value at offset 0x88
    undefined4 field_8c; // Value at offset 0x8c
    void (*func_ptr)(void); // Function pointer at offset 0x90
    undefined4 field_94; // Value at offset 0x94, always 0
} InventoryItem; // Total size: 8 + 128 + 4 + 4 + 4 + 4 = 152 bytes (0x98)

// Helper function to create and initialize an InventoryItem
InventoryItem* create_inventory_item(const char* id_str, const char* name_str, undefined4 val_88, undefined4 val_8c) {
    InventoryItem* item = (InventoryItem*)malloc(sizeof(InventoryItem));
    if (item == NULL) {
        _terminate();
    }

    // Initialize all members to zero (including string buffers for null termination)
    memset(item, 0, sizeof(InventoryItem));

    // Copy ID string, ensuring null termination and preventing buffer overflow
    size_t id_len = strlen(id_str);
    if (id_len >= sizeof(item->id)) {
        id_len = sizeof(item->id) - 1;
    }
    memcpy(item->id, id_str, id_len);
    item->id[id_len] = '\0';

    // Copy Name string, ensuring null termination and preventing buffer overflow
    size_t name_len = strlen(name_str);
    if (name_len >= sizeof(item->name)) {
        name_len = sizeof(item->name) - 1;
    }
    memcpy(item->name, name_str, name_len);
    item->name[name_len] = '\0';

    // Assign other fields
    item->field_88 = val_88;
    item->field_8c = val_8c;
    item->func_ptr = onsale_fn;
    item->field_94 = 0; // This field is consistently set to 0 in the original code

    return item;
}

// Function: load_inventory
void load_inventory(void* param_1) {
    InventoryItem* item;

    // Each block of the original code follows a repetitive pattern:
    // 1. Allocate memory for an item (0x98 bytes).
    // 2. Initialize memory to 0.
    // 3. Set item ID (string).
    // 4. Set item Name (string, at offset +8).
    // 5. Set field_88 (undefined4 at offset +0x88).
    // 6. Set field_8c (undefined4 at offset +0x8c, referencing DAT_00019xxx).
    // 7. Set func_ptr (function pointer at offset +0x90, always onsale_fn).
    // 8. Set field_94 (undefined4 at offset +0x94, always 0).
    // 9. Insert the item into the list using list_insert_at_end.

    // Using the helper function to reduce code duplication and intermediate variables.

    item = create_inventory_item("41916155", "CBoXdTVlxY", 0xe9f1, DAT_000196b8);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("12099137", "BURcRQvrJKLoWsuIu", 0x468, DAT_000196bc);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("86848322", "OuYFqLLPzpStQmLGNfrdmNBtqESEZPD", 0xfa65, DAT_000196c0);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("87849215", "osjXtGjM", 0xb3d3, DAT_000196c4);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("50361802", "WaALSqUGTUcRRQbrpD", 0x3251, DAT_000196c8);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("28433101", DAT_0001908f, 0x7bdc, DAT_000196cc);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("12220539", DAT_0001909c, 0x5a9e, DAT_000196d0);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("47111925", "MxLyPsRDfZNyZCZyPiFcO", 0x7afe, DAT_000196d4);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("00664810", "WBsFTZeoBiL", 0x6aa4, DAT_000196d8);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("94282841", "CiyXLti", 0x2986, DAT_000196dc);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("72291601", DAT_000190ef, 0xc435, DAT_000196e0);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("49840365", "LrmPNlODBcMuDdejSrklcnEJbCdJOA", 0xb121, DAT_000196e4);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("50717007", "OcYEZmPsrGyvcbxflxxamqLlLJ", 0xea6b, DAT_000196e8);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("99964712", "NcVIOJwRguXBwKdEwzx", 0x74, DAT_000196ec);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("13706532", "SmMpCSxZTsPtsDwZprYJIBYtCczZ", 0x9b7e, DAT_000196f0);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("44381534", "mfuSPgiZIaaPTtfMKuYWPqW", 0x75a8, DAT_000196f4);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("66900952", "dLuOlZFcVURjFo", 0x3a4f, DAT_000196f8);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("74926350", DAT_000191c8, 0x9a8, DAT_000196fc);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("31951871", "gZOISWZizsGWxfIV", 0x6432, DAT_00019700);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("20585072", "OVNnctOCWjcIXOvxXwZdeYk", 0x82a5, DAT_00019704);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("29987528", "bQemYYbkbgRlHzYhddLTXBV", 0xf96a, DAT_00019708);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("44567831", "bVxVzqJoMAbbABpuK", 0x2821, DAT_0001970c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("50044195", "TIwQNAxBYeidFJgiuiceKyyZOo", 0xfb54, DAT_00019710);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("80320569", "ZbOKiKsPtSLZUXGSgVuVvEg", 0xc9a, DAT_00019714);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("12555125", "OOBHIukxWBXTujZouCmy", 0xbcbc, DAT_00019718);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("77475676", "TkFFmsuzuD", 0x326e, DAT_0001971c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("68500061", "xXKivnPyFeKjDSsLIcHJkcfDZr", 0x62ed, DAT_00019720);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("28801547", "zPzfWrYgkzusXM", 0xd73a, DAT_00019724);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("27286316", "iaOSmHodjClBbsmNMxMUjFEEBw", 0xb091, DAT_00019728);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("06744775", "RZaRMCR", 0x98f9, DAT_0001972c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("14976063", "lXkSGfmjnIcJrqjzuwaLpNVsxCoeZla", 0xe440, DAT_00019730);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("93921599", "qJGFxRH", 0xcaca, DAT_00019734);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("78486525", "lrWSt", 0x145f, DAT_00019738);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("75836131", DAT_0001937d, 0x8940, DAT_0001973c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("95486291", "jJIhbikVwbdVIhCAZGHqjLY", 0x775d, DAT_00019740);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("19044824", "mfoLvDmaKMrQNvWLWUiYCiXNnZOMU", 0x4768, DAT_00019744);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("30172465", "CKFuyRMTABZMIBdsK", 0xae40, DAT_00019748);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("83497266", "XyEYbfHmiWVtxO", 0x5f0, DAT_0001974c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("46693171", "YmUHUvQoJ", 0xb007, DAT_00019750);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("60531381", "QiMrgINCfFjkdvozTnpjtYrELDjgmK", 0xc4bd, DAT_00019754);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("80111827", "EGCyAcNvAoGTznBIInHD", 0xc3f3, DAT_00019758);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("46108933", DAT_0001945e, 0x4a35, DAT_0001975c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("71344234", "GZxglAxzKwgOEwRVCgPdf", 0x9ed6, DAT_00019760);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("20102117", "RijMwfgJPEmoMjLCeepSY", 0x7cfe, DAT_00019764);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("79267208", "XvnPoMSNWZVhfUMmpFKpphmfbwSTVo", 0x85fd, DAT_00019768);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("13001388", "ngllfhKdGIPHvstPd", 0xd487, DAT_0001976c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("17573103", DAT_000194eb, 0x9d3, DAT_00019770);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("22995054", "GoiSSYhLBSSyDMzpZEfEN", 0x5a20, DAT_00019774);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("58885039", "LvJBSMasVdFOjklc", 0xb020, DAT_00019778);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("36970139", "emCCzWhvvveEoEViRKwIsyWcMA", 0xb83f, DAT_0001977c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("15048891", "QPuFuYcsUqextKPzFHBFhWWxiwL", 0x9bea, DAT_00019780);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("52820574", "VweQMYhAYdTCE", 0x86f5, DAT_00019784);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("05558432", "YrsbMdytqSNuDPUN", 0xa80d, DAT_00019788);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("45266556", "SxagJhzOzEhqAhpAdpuREDJnM", 0xd1e7, DAT_0001978c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("55671691", "JvqXCPHjJyQ", 0xed0b, DAT_00019790);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("30246117", "DaEzVgFxbPzzIF", 0x2eca, DAT_00019794);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("32349600", "vDzqEgRHizanDmHWYmtKjy", 0x7fb6, DAT_00019798);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("27002400", "TAAwfhYZBDTbayzgoqEhIiYYZLPBss", 0x6e3d, DAT_0001979c);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("23728750", "KMLqLPUgFrTS", 0x7088, DAT_000197a0);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("27227405", "xbIkRMPWFwVechYvOEnobB", 0x1b5e, DAT_000197a4);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("88925705", "aMZLCrgwzBKDLiauIPwEHaX", 0x17c2, DAT_000197a8);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("60234543", DAT_0001969b, 0x5a1d, DAT_00019738);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("26550548", DAT_000196a9, 0xdee2, DAT_000197ac);
    list_insert_at_end(item, param_1);

    item = create_inventory_item("80824106", DAT_000196b6, 0x929d, DAT_000197b0);
    list_insert_at_end(item, param_1);

    return;
}