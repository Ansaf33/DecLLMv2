#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // For uint16_t, uint32_t

// --- Placeholder Types for Decompiler 'undefined' types ---
// Assuming file handles, resource IDs, shared library handles are 32-bit integers
typedef uint32_t file_handle_t;
typedef uint32_t resource_id_t;
typedef int32_t  shared_lib_t;
typedef int      file_type_t; // Assuming file types are int

// --- Custom String Handle Structure ---
// Based on usage like `*(char**)(local_50 + 4)`, it's likely a struct where
// the actual string pointer is at offset 4.
typedef struct {
    uint32_t handle_id; // Placeholder for any internal handle ID or length
    char* string_data;  // Pointer to the actual C-string
} CStringHandle_t;

// --- Forward Declarations for Custom Functions ---
// These functions are assumed to be external and their implementations are not provided.
// Their return types and parameters are inferred from the original snippet.
file_handle_t GetFileID(file_handle_t file_handle);
file_handle_t GetParent(file_handle_t file_handle);
file_type_t GetFileType(file_handle_t file_handle);
int GetFileSize(file_handle_t file_handle); // Changed from undefined4 to int
const char* GetFileName(file_handle_t file_handle);
const char* GetFilePath(file_handle_t file_handle);
file_handle_t FindFile(const char* filename, file_handle_t current_dir);
file_handle_t FindFileAbsolute(const char* filename);
shared_lib_t LoadSharedLibrary(file_handle_t file_handle);
resource_id_t LookupResource(shared_lib_t library_handle, uint16_t resource_id);
int CreateFile(const char* filename, file_type_t type, uint16_t param3, uint32_t param4, file_handle_t current_dir); // param3, param4 inferred
int DeleteFile(const char* filename, file_handle_t current_dir);
void InitializeOSFiles(void);
file_handle_t InitializeFileSystem(void);
CStringHandle_t* ReceiveCString(int max_len); // Returns pointer to CStringHandle_t
void DestroyCString(char* str_handle_ptr); // Takes the base pointer returned by ReceiveCString
int ReadFile(file_handle_t file_handle); // Returns a pointer to data (e.g., char* cast to int)
void ReceiveBytes(void* buffer, size_t size);
int receive(void); // External input function, assumed to return character or error code

// Function: ListFilesRecurse
void ListFilesRecurse(file_handle_t file_handle, int depth) {
    if (file_handle != 0) {
        for (int i = 0; i < depth; ++i) {
            printf("    ");
        }
        // Assuming GetFileName returns the name associated with the handle.
        // And GetFileID returns a numerical ID.
        printf("%s (%u)\n", GetFileName(file_handle), GetFileID(file_handle));

        // These offsets (0x58 for child, 0x50 for sibling) suggest a specific
        // structure for file_handle objects.
        // Casting to char* allows byte-offset arithmetic.
        ListFilesRecurse(*(file_handle_t*)((char*)file_handle + 0x58), depth + 1); // Child handle
        ListFilesRecurse(*(file_handle_t*)((char*)file_handle + 0x50), depth);     // Sibling handle
    }
}

// Function: ListFilesLong
void ListFilesLong(file_handle_t current_dir) {
    // Reduce intermediate variable `local_14` by inlining.
    shared_lib_t flavor_library_base = LoadSharedLibrary(FindFileAbsolute("System%Special%Flavor.sl"));
    
    // Iterate through child files first, then siblings.
    // The starting point is *(file_handle_t*)((char*)current_dir + 0x58) (first child).
    // The iteration uses *(file_handle_t*)((char*)file_entry + 0x50) (next sibling).
    for (file_handle_t file_entry = *(file_handle_t*)((char*)current_dir + 0x58);
         file_entry != 0;
         file_entry = *(file_handle_t*)((char*)file_entry + 0x50)) {
        
        // Reduce intermediate variables `uVar2`, `cVar1`.
        file_type_t file_type = GetFileType(file_entry);
        uint32_t current_res_id = (file_type & 0xff) - 0xf1; // ID or type info for LookupResource

        shared_lib_t current_res_lib = flavor_library_base; // Start with base library, might be overridden by link extraction

        if (file_type == -0xc) { // Assuming -0xc is a specific file type, perhaps a link
            // The source of the link target and ID is at offset 0x5c from file_entry
            ExtractLinkTargetAndID(*(uint32_t*)((char*)file_entry + 0x5c), current_dir, &current_res_lib, (int*)&current_res_id);
        }
        
        // Reduce intermediate variables `uVar3`, `uVar4`. Fix printf format string.
        const char* resource_name = (const char*)LookupResource(current_res_lib, (uint16_t)(current_res_id & 0xffff));
        printf("%s \t%d\t%s\n", GetFileName(file_entry), GetFileSize(file_entry), resource_name);
    }
}

// Function: ExtractLinkTargetAndID
void ExtractLinkTargetAndID(uint32_t *link_data, file_handle_t current_dir, shared_lib_t *out_library_handle, uint32_t *out_resource_id) {
    // `local_14` (undefined4 local_14 = 0;) was unused, removed.
    int resource_id = -1; 
    
    // Check for "LINK" magic (0x4c494e4b) and size constraint
    if ((*link_data < 0x801) && (link_data[1] == 0x4c494e4b)) { 
        char* link_target_base_str = (char *)(link_data + 2); // String data starts after header

        // Check for "System%Special%" prefix
        if (strncmp(link_target_base_str, "System%Special%", 0xf) == 0) {
            // Assuming 0x48 for 32-bit aligned read of an integer ID. Original was 0x49.
            resource_id = *(uint32_t*)((char*)link_data + 0x48);
        }

        // `local_18` (strchr result) is reduced by direct usage.
        char *plus_char_in_base = strchr(link_target_base_str, '+');
        
        if (plus_char_in_base == NULL) { // If '+' not found in the original string
            char formatted_link_buffer[77]; // `local_6d` was 77 bytes
            // Fix sprintf format string.
            snprintf(formatted_link_buffer, sizeof(formatted_link_buffer), "%s+%d", link_target_base_str, resource_id);

            // `local_1c` (calloc result) is reduced by direct usage.
            // Fix calloc arguments: `calloc(1, size)` instead of `calloc(size, (size_t)char_ptr)`.
            char *allocated_link_str = (char *)calloc(1, 0x43); // Allocate 67 bytes (0x43)
            if (allocated_link_str == NULL) {
                return; // Allocation failed
            }
            strncpy(allocated_link_str, formatted_link_buffer, 0x42); // Copy up to 66 bytes (0x42)
            allocated_link_str[0x42] = '\0'; // Ensure null termination

            // `local_20` (strchr result) is reduced by direct usage.
            char *plus_in_allocated = strchr(allocated_link_str, '+');
            if (plus_in_allocated != NULL) {
                int parsed_id = atoi(plus_in_allocated + 1);
                if (parsed_id >= 0) { // Original: -1 < parsed_id
                    resource_id = parsed_id; 
                    shared_lib_t loaded_lib = VerifyAndLoadFlavorFile(link_target_base_str, current_dir); // Uses original string
                    if (loaded_lib != 0) {
                        *out_resource_id = resource_id;
                        *out_library_handle = loaded_lib;
                    }
                }
            }
            free(allocated_link_str); // Free the dynamically allocated string
        }
        // If '+' was found in the original string, the function's original logic
        // did not proceed to load a library or update the output parameters.
        // This specific behavior is preserved.
    }
}

// Function: VerifyAndLoadFlavorFile
shared_lib_t VerifyAndLoadFlavorFile(const char *filename_base, file_handle_t current_dir) {
    // `sVar1`, `sVar2` (strlen results) are reduced by direct usage.
    char processed_filename_buffer[65]; // `local_59` was 65 bytes (0x41)
    memset(processed_filename_buffer, 0, sizeof(processed_filename_buffer));
    strncpy(processed_filename_buffer, filename_base, sizeof(processed_filename_buffer) - 1);
    processed_filename_buffer[sizeof(processed_filename_buffer) - 1] = '\0'; // Ensure null termination

    // `local_10` (strchr result) is kept as `underscore_char_ptr` for string modification logic.
    char *underscore_char_ptr = strchr(processed_filename_buffer, '_');
    char *append_point;

    if (underscore_char_ptr == NULL) {
        append_point = processed_filename_buffer + strlen(processed_filename_buffer);
    } else {
        *underscore_char_ptr = '\0'; // Null-terminate at '_'
        append_point = underscore_char_ptr;
    }

    const char *suffix = ".listofiles";
    // Check if there's enough space in `processed_filename_buffer` to append ".listofiles"
    if ((sizeof(processed_filename_buffer) - 1) - strlen(processed_filename_buffer) < strlen(suffix)) {
        return 0; // Not enough space
    } else {
        strcpy(append_point, suffix); // Append ".listofiles"

        // `local_18` (FindFile result) was unused, removed.
        // The original code uses `param_1` (filename_base) for FindFile, not the modified `local_59`.
        file_handle_t file_handle = FindFile(filename_base, current_dir); 
        if (file_handle == 0) {
            file_handle = FindFileAbsolute(filename_base); 
        }
        return LoadSharedLibrary(file_handle);
    }
}

// Function: ReceiveCommand
// This function's original decompiler output contained significant stack manipulation
// and ambiguous `receive()` calls. This version interprets the *logical flow*
// to return a command character for `main`'s switch statement.
// It assumes `receive()` reads a single character and returns it (as an int),
// or a negative value for error/EOF.
char ReceiveCommand(void) {
    // The original code had two initial `receive()` calls with `if (iVar1 != 0) _terminate();`
    // These might be for handshake or status.
    if (receive() != 0) { // First control/status read
        exit(1); // _terminate()
    }
    
    // The actual command character is likely read here.
    // `local_e` from the original code would hold this character.
    int res = receive(); // Second control/status or actual command read
    if (res < 0) { // Assuming negative is error/EOF
        exit(1); // _terminate()
    }
    char actual_command_char = (char)res; // This is `local_e` from the original.

    // The original logic `if (local_e != '\n') { local_d[0] = 0x78; }`
    // means if the received character is NOT newline, the command becomes 'x'.
    // `local_d[0]` was the final return value.
    if (actual_command_char != '\n') {
        return 'x'; // 0x78 is 'x'
    }
    return actual_command_char;
}

// Function: main
int main(void) { // Changed return type from undefined4 to int
    // `local_14 = &stack0x00000004;` was a decompiler artifact, removed.
    file_handle_t current_fs_handle = InitializeFileSystem();
    InitializeOSFiles();
    
    int running = 1; // `local_28` controls the main loop
    while (running != 0) {
        printf(">");
        // `local_29` (command char) is inlined directly into the switch statement.
        char command = ReceiveCommand();
        
        switch(command) {
            case 'c': { // Create file
                // `local_50` is a temporary variable for the CStringHandle_t pointer.
                CStringHandle_t* filename_handle = ReceiveCString(0x40);
                if (CreateFile(filename_handle->string_data, 0xf7, 0, 0, current_fs_handle) != 0) {
                    printf("FAILED\n");
                }
                DestroyCString((char*)filename_handle);
                break;
            }
            case 'd': { // Delete file
                // `local_4c` is a temporary variable for the CStringHandle_t pointer.
                CStringHandle_t* filename_handle = ReceiveCString(0x40);
                if (DeleteFile(filename_handle->string_data, current_fs_handle) != 0) {
                    printf("FAILED\n");
                }
                DestroyCString((char*)filename_handle);
                break;
            }
            case 'g': { // List files long
                ListFilesLong(current_fs_handle);
                break;
            }
            case 'h': { // Change directory
                // `local_44` is a temporary variable for the CStringHandle_t pointer.
                CStringHandle_t* dirname_handle = ReceiveCString(0x40);
                if (strcmp("upone", dirname_handle->string_data) == 0) {
                    current_fs_handle = GetParent(current_fs_handle);
                } else {
                    // `local_48` (FindFile result) is kept as `target_dir_handle` for clarity and conditional use.
                    file_handle_t target_dir_handle = FindFile(dirname_handle->string_data, current_fs_handle);
                    // `cVar1` (GetFileType result) is inlined.
                    if (GetFileType(target_dir_handle) == -9) { // -9 (0xF7) is directory type
                        current_fs_handle = target_dir_handle;
                    } else {
                        printf("File Not Found\n");
                    }
                }
                DestroyCString((char*)dirname_handle);
                break;
            }
            case 'l': { // List files recursively
                ListFilesRecurse(current_fs_handle, 0);
                break;
            }
            case 'r': { // Read file
                // `local_38` is a temporary variable for the CStringHandle_t pointer.
                CStringHandle_t* filename_handle = ReceiveCString(0x40);
                // `local_3c` (FindFile result) is kept for conditional use.
                file_handle_t file_to_read = FindFile(filename_handle->string_data, current_fs_handle);
                if (file_to_read != 0) {
                    // `local_40` (ReadFile result) is kept as `file_content` because it needs to be freed.
                    char* file_content = (char*)ReadFile(file_to_read); // Assuming ReadFile returns char*
                    printf("%s\n", file_content); // Fix printf format string
                    free(file_content);
                }
                DestroyCString((char*)filename_handle);
                break;
            }
            case 'u': { // Upload/Update file (CreateFile with content)
                // `local_30` is a temporary variable for the CStringHandle_t pointer.
                CStringHandle_t* filename_handle = ReceiveCString(0x40);
                char file_type_char; // `local_51` changed from undefined to char
                ReceiveBytes(&file_type_char, 1);

                // `local_34` is a temporary variable for the CStringHandle_t pointer.
                // Assuming ReceiveCString(0x800) returns a CStringHandle_t*
                // where its `string_data` points to a buffer containing a uint16_t and a uint32_t.
                CStringHandle_t* data_handle = ReceiveCString(0x800);
                uint16_t* data_ptr_u16 = (uint16_t*)data_handle->string_data;
                uint32_t* data_ptr_u32 = (uint32_t*)((char*)data_ptr_u16 + 2); // Offset 2 bytes from u16 ptr

                if (CreateFile(filename_handle->string_data, file_type_char, *data_ptr_u16, *data_ptr_u32, current_fs_handle) != 0) {
                    printf("FAILED\n");
                }
                DestroyCString((char*)filename_handle);
                DestroyCString((char*)data_handle);
                break;
            }
            case 'w': { // Print current path
                // `uVar3`, `uVar4` (GetFileName/GetFilePath results) are inlined.
                printf("%s%s\n", GetFilePath(current_fs_handle), GetFileName(current_fs_handle)); // Fix printf format string
                break;
            }
            case 'x': { // Exit
                running = 0;
                break;
            }
            default: {
                printf("Invalid Command\n");
                break;
            }
        }
    }
    printf("\n");
    return 0;
}