#include <stdio.h>
#include <stdlib.h> // For calloc, free, exit, rand, srand
#include <string.h> // For memcpy, strcmp
#include <stdint.h> // For fixed-width integers
#include <time.h>   // For time (to seed srand)

// Decompiler-generated types mapped to standard C types
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

// --- Inferred Structures ---

// CGCMBDataBlock: Represents a block of data within a CGCMBMessage
typedef struct {
    uint16 size;
    uint16 current_offset; // Used by ReadFromData for tracking read progress
    void *data_ptr;
} CGCMBDataBlock;

// CGCMBMessageHeader: The initial header part of a CGCMBMessage
typedef struct {
    uint32 magic;
    uint8 type;
    uint8 field_5;
    uint8 field_6;
    uint8 field_7[8];
} CGCMBMessageHeader;

// CGCMBMessage: The main message structure
typedef struct {
    CGCMBMessageHeader *header;
    CGCMBDataBlock *data_block_1; // Primary data block (e.g., request/response specific data)
    CGCMBDataBlock *data_block_2; // Secondary data block (e.g., larger payload)
    void *parsed_data; // Pointer to a dynamically allocated structure based on message type
} CGCMBMessage;

// TransportMessage: Structure for messages sent over the transport layer
typedef struct {
    uint32 field_0;
    uint32 total_size;
    void *data_ptr;
} TransportMessage;

// Connection structure
typedef struct {
    uint32 id;
    uint32 status_field;
    void *tree_ptr; // Pointer to a Tree structure or identifier
    uint32 files[16]; // Array of pointers to File objects
} Connection;

// File structure
typedef struct {
    uint16 id;
    // ... other file-related fields
} File;

// Parsed data structures for CGCMBMessage->parsed_data
typedef struct {
    uint32 count;
    uint16 *data;
} CGCMBMessageParsedData_Type1;

typedef struct {
    uint32 client_id;
    uint16 field_1;
    uint32 field_2;
    char *username;
    char *password;
} CGCMBMessageParsedData_Type2;

typedef struct {
    uint32 client_id;
    uint16 user_id;
    char *path;
    char *filename;
    char *field_4;
} CGCMBMessageParsedData_Type3;

typedef struct {
    uint32 client_id;
    uint16 user_id;
    uint32 connection_id;
} CGCMBMessageParsedData_Type4;

typedef struct {
    uint32 client_id;
    uint16 user_id;
    uint32 connection_id;
    uint32 access_flags;
    char *filename;
} CGCMBMessageParsedData_Type5;

typedef struct {
    uint32 client_id;
    uint16 user_id;
    uint32 connection_id;
    uint16 file_id;
} CGCMBMessageParsedData_Type6;

typedef struct {
    uint32 client_id;
    uint16 user_id;
    uint32 connection_id;
    uint16 file_id;
    uint16 read_size;
    uint32 offset;
} CGCMBMessageParsedData_Type7;

typedef struct {
    uint32 client_id;
    uint16 user_id;
    uint32 connection_id;
    uint16 file_id;
    uint16 write_size;
    uint8 field_5;
    uint32 offset;
    void *write_data;
} CGCMBMessageParsedData_Type8;

typedef struct {
    uint32 client_id;
    uint16 user_id;
    uint32 connection_id;
    uint16 file_id;
} CGCMBMessageParsedData_Type10;

// Global server state structure
typedef struct {
    uint32 status;
    uint16 type_field; // e.g., protocol type
    uint32 random_id;
    uint16 user_id;
    Connection *connections[32]; // Max 32 connections
} ServerState;
static ServerState mbServerState = {0}; // Initialize all to zero

// Error codes, inferred from HandleCGCMBMessage
#define ERROR_NONE 0
#define ERROR_INVALID_MESSAGE 0xE1
#define ERROR_MESSAGE_TOO_LARGE 0xE2
#define ERROR_INVALID_STATE 0xE3
#define ERROR_TREE_NOT_FOUND 0xE4
#define ERROR_INVALID_CLIENT_ID 0xE5
#define ERROR_UNKNOWN_MESSAGE_TYPE 0xE6
#define ERROR_INVALID_USER_ID 0xE7
#define ERROR_NO_FREE_CONNECTIONS 0xE8
#define ERROR_CONNECTION_OR_FILE_NOT_FOUND 0xE9
#define ERROR_FILE_OPERATION_FAILED 0xEA
#define ERROR_RPC_UNSUPPORTED 0xEB

// Forward declarations for functions defined later
CGCMBMessage* CreateBlankCGCMBMessage(void);
void DestroyCGCMBMessage(CGCMBMessage **msg_ptr);
int VerifyCGCMBMessage(CGCMBMessage *msg);
void TransmitCGCMBMessage(CGCMBMessage *msg);
void SendErrorResponse(uint8 error_code);
int ReadFromTransportMessage(int transport_fd, void *buffer, size_t size);
uint16 LookupUser(void *username, void *password);
uint32 FindTreeByPath(uint16 user_id, void *path, void *filename);
uint32 CreateFile(void *tree_ptr, void *filename, uint16 user_id, uint32 access_flags);
void CloseFile(uint32 file_ptr);
int ReadFile(void *buffer, uint32 file_ptr, uint32 offset, uint16 size);
int WriteFile(uint32 file_ptr, void *buffer, uint32 offset, uint16 size);
uint8 HandleCGCRPCMessage(uint32 rpc_data_ptr, uint16 rpc_data_size, void **response_data_ptr, int *response_size_ptr);
Connection* FindConnectionByID(uint32 connection_id);
File* FindFileByID(Connection *conn, uint32 file_id);

// Mock implementations for compilation
int ReadFromTransportMessage(int transport_fd, void *buffer, size_t size) {
    if (transport_fd == 0) return -1;
    return 0; // Always succeed for mock
}

void SendTransportMessage(TransportMessage *msg) {
    // Mock implementation: Simulate sending data.
}

void DestroyTransportMessage(TransportMessage **msg_ptr) {
    if (msg_ptr && *msg_ptr) {
        free((*msg_ptr)->data_ptr);
        free(*msg_ptr);
        *msg_ptr = NULL;
    }
}

uint16 LookupUser(void *username, void *password) {
    return 1001; // Dummy user ID
}

uint32 FindTreeByPath(uint16 user_id, void *path, void *filename) {
    return 0xDEADBEEF; // Dummy tree pointer
}

uint32 CreateFile(void *tree_ptr, void *filename, uint16 user_id, uint32 access_flags) {
    return 0xBEEFCAFE; // Dummy file pointer
}

void CloseFile(uint32 file_ptr) {
    // Mock implementation: Do nothing
}

int ReadFile(void *buffer, uint32 file_ptr, uint32 offset, uint16 size) {
    memset(buffer, 0xAA, size); // Fill with dummy data
    return 0; // Success
}

int WriteFile(uint32 file_ptr, void *buffer, uint32 offset, uint16 size) {
    return 0; // Success
}

uint8 HandleCGCRPCMessage(uint32 rpc_data_ptr, uint16 rpc_data_size, void **response_data_ptr, int *response_size_ptr) {
    *response_size_ptr = 10;
    *response_data_ptr = calloc(1, *response_size_ptr);
    if (*response_data_ptr) {
        memcpy(*response_data_ptr, "RPC_RESP", *response_size_ptr);
        return 0; // Success
    }
    return 1; // Failure
}

// Initialize random number generator once
__attribute__((constructor))
static void init_rand_seed() {
    srand(time(NULL));
}

// Function: AddToData
void AddToData(CGCMBDataBlock *data_block, void *source_data, size_t data_size) {
    if (!data_block) {
        return;
    }

    if (data_block->size == 0) {
        data_block->data_ptr = calloc(data_size, 1);
        if (!data_block->data_ptr) return; // Handle allocation failure
        data_block->size = (uint16)data_size;
        memcpy(data_block->data_ptr, source_data, data_size);
    } else {
        void *new_data_ptr = calloc(data_block->size + data_size, 1);
        if (!new_data_ptr) return; // Handle allocation failure
        memcpy(new_data_ptr, data_block->data_ptr, data_block->size);
        memcpy((uint8*)new_data_ptr + data_block->size, source_data, data_size);
        data_block->size = (uint16)(data_block->size + data_size);
        free(data_block->data_ptr);
        data_block->data_ptr = new_data_ptr;
    }
}

// Function: ReceiveCGCMBMessage
int ReceiveCGCMBMessage(int transport_fd, CGCMBMessage **msg_out) {
    if (transport_fd == 0) {
        return -1;
    }

    CGCMBMessage *msg = CreateBlankCGCMBMessage();
    if (!msg) {
        return -1; // Failed to create message
    }

    int status = 0;

    // Read header
    if (status == 0) status = ReadFromTransportMessage(transport_fd, &msg->header->magic, sizeof(msg->header->magic));
    if (status == 0) status = ReadFromTransportMessage(transport_fd, &msg->header->type, sizeof(msg->header->type));
    if (status == 0) status = ReadFromTransportMessage(transport_fd, &msg->header->field_5, sizeof(msg->header->field_5));
    if (status == 0) status = ReadFromTransportMessage(transport_fd, &msg->header->field_6, sizeof(msg->header->field_6));
    if (status == 0) status = ReadFromTransportMessage(transport_fd, msg->header->field_7, sizeof(msg->header->field_7));

    // Read data_block_1
    if (status == 0) status = ReadFromTransportMessage(transport_fd, &msg->data_block_1->size, sizeof(msg->data_block_1->size));
    if (status == 0 && msg->data_block_1->size != 0) {
        msg->data_block_1->data_ptr = calloc(msg->data_block_1->size, 1);
        if (!msg->data_block_1->data_ptr) status = -1; // Allocation failed
        else status = ReadFromTransportMessage(transport_fd, msg->data_block_1->data_ptr, msg->data_block_1->size);
    }

    // Read data_block_2
    if (status == 0) status = ReadFromTransportMessage(transport_fd, &msg->data_block_2->size, sizeof(msg->data_block_2->size));
    if (status == 0 && msg->data_block_2->size != 0) {
        msg->data_block_2->data_ptr = calloc(msg->data_block_2->size, 1);
        if (!msg->data_block_2->data_ptr) status = -1; // Allocation failed
        else status = ReadFromTransportMessage(transport_fd, msg->data_block_2->data_ptr, msg->data_block_2->size);
    }

    if (status != 0) {
        DestroyCGCMBMessage(&msg);
        return -1;
    }

    *msg_out = msg;
    return 0;
}

// Function: SendErrorResponse
void SendErrorResponse(uint8 error_code) {
    CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
    if (!response_msg) {
        // Handle critical error: cannot even send an error response
        return;
    }

    response_msg->header->type = 0xff; // Error message type
    uint32 error_code_val = error_code; // Error code to be added to data
    AddToData(response_msg->data_block_2, &error_code_val, sizeof(uint32));
    TransmitCGCMBMessage(response_msg);
    DestroyCGCMBMessage(&response_msg);
}

// Function: ReadFromData
int ReadFromData(void *destination, CGCMBDataBlock *data_block, size_t read_size) {
    if (!destination || !data_block || !data_block->data_ptr) {
        return -1;
    }
    if (read_size == 0) {
        return 0;
    }
    if ((data_block->current_offset + read_size) > data_block->size) {
        return -1; // Not enough data
    }

    memcpy(destination, (uint8*)data_block->data_ptr + data_block->current_offset, read_size);
    data_block->current_offset = (uint16)(data_block->current_offset + read_size);
    return 0;
}

// Function: ParseCGCMBMessage
int ParseCGCMBMessage(CGCMBMessage *msg) {
    if (!msg || !msg->header) {
        return -1;
    }

    int status = 0;
    msg->parsed_data = NULL; // Initialize parsed data pointer

    switch (msg->header->type) {
        case 1: {
            CGCMBMessageParsedData_Type1 *data = calloc(1, sizeof(CGCMBMessageParsedData_Type1));
            if (!data) return -1;
            msg->parsed_data = data;

            data->count = (uint32)(msg->data_block_2->size / 2);
            if (data->count != 0) {
                data->data = calloc(data->count, sizeof(uint16));
                if (!data->data) { status = -1; break; }
                status = ReadFromData(data->data, msg->data_block_2, data->count * sizeof(uint16));
            }
            break;
        }
        case 2: {
            CGCMBMessageParsedData_Type2 *data = calloc(1, sizeof(CGCMBMessageParsedData_Type2));
            if (!data) return -1;
            msg->parsed_data = data;

            if (status == 0) status = ReadFromData(&data->client_id, msg->data_block_1, sizeof(data->client_id));
            if (status == 0) status = ReadFromData(&data->field_1, msg->data_block_1, sizeof(data->field_1)); // local_32
            if (status == 0) status = ReadFromData(&data->field_2, msg->data_block_1, sizeof(data->field_2));

            if (status == 0 && data->field_1 != 0 && data->field_1 < 0x41) {
                data->username = calloc(data->field_1 + 1, 1);
                if (!data->username) { status = -1; break; }
                status = ReadFromData(data->username, msg->data_block_2, data->field_1);
            }
            uint16 local_34 = 0; // Original local_34
            if (status == 0) status = ReadFromData(&local_34, msg->data_block_2, sizeof(uint16));
            if (status == 0 && local_34 != 0 && local_34 < 0x81) {
                data->password = calloc(local_34 + 1, 1);
                if (!data->password) { status = -1; break; }
                status = ReadFromData(data->password, msg->data_block_2, local_34);
            }
            break;
        }
        case 3: {
            CGCMBMessageParsedData_Type3 *data = calloc(1, sizeof(CGCMBMessageParsedData_Type3));
            if (!data) return -1;
            msg->parsed_data = data;

            if (status == 0) status = ReadFromData(&data->client_id, msg->data_block_1, sizeof(data->client_id));
            if (status == 0) status = ReadFromData(&data->user_id, msg->data_block_1, sizeof(data->user_id));

            uint16 local_36 = 0; // Original local_36
            if (status == 0) status = ReadFromData(&local_36, msg->data_block_2, sizeof(uint16));
            if (status == 0 && local_36 != 0 && local_36 < 0x41) {
                data->path = calloc(local_36 + 1, 1);
                if (!data->path) { status = -1; break; }
                status = ReadFromData(data->path, msg->data_block_2, local_36);
            }
            if (status == 0) status = ReadFromData(&local_36, msg->data_block_2, sizeof(uint16));
            if (status == 0 && local_36 != 0 && local_36 < 0x81) {
                data->filename = calloc(local_36 + 1, 1);
                if (!data->filename) { status = -1; break; }
                status = ReadFromData(data->filename, msg->data_block_2, local_36);
            }
            if (status == 0) status = ReadFromData(&local_36, msg->data_block_2, sizeof(uint16));
            if (status == 0 && local_36 != 0 && local_36 < 0x81) {
                data->field_4 = calloc(local_36 + 1, 1);
                if (!data->field_4) { status = -1; break; }
                status = ReadFromData(data->field_4, msg->data_block_2, local_36);
            }
            break;
        }
        case 4: {
            CGCMBMessageParsedData_Type4 *data = calloc(1, sizeof(CGCMBMessageParsedData_Type4));
            if (!data) return -1;
            msg->parsed_data = data;

            if (status == 0) status = ReadFromData(&data->client_id, msg->data_block_1, sizeof(data->client_id));
            if (status == 0) status = ReadFromData(&data->user_id, msg->data_block_1, sizeof(data->user_id));
            if (status == 0) status = ReadFromData(&data->connection_id, msg->data_block_1, sizeof(data->connection_id));
            break;
        }
        case 5: {
            CGCMBMessageParsedData_Type5 *data = calloc(1, sizeof(CGCMBMessageParsedData_Type5));
            if (!data) return -1;
            msg->parsed_data = data;

            if (status == 0) status = ReadFromData(&data->client_id, msg->data_block_1, sizeof(data->client_id));
            if (status == 0) status = ReadFromData(&data->user_id, msg->data_block_1, sizeof(data->user_id));
            if (status == 0) status = ReadFromData(&data->connection_id, msg->data_block_1, sizeof(data->connection_id));
            if (status == 0) status = ReadFromData(&data->access_flags, msg->data_block_1, sizeof(data->access_flags));

            uint16 local_38 = 0; // Original local_38
            if (status == 0) status = ReadFromData(&local_38, msg->data_block_2, sizeof(uint16));
            if (status == 0 && local_38 != 0 && local_38 < 0x81) {
                data->filename = calloc(local_38 + 1, 1);
                if (!data->filename) { status = -1; break; }
                status = ReadFromData(data->filename, msg->data_block_2, local_38);
            }
            break;
        }
        case 6: {
            CGCMBMessageParsedData_Type6 *data = calloc(1, sizeof(CGCMBMessageParsedData_Type6));
            if (!data) return -1;
            msg->parsed_data = data;

            if (status == 0) status = ReadFromData(&data->client_id, msg->data_block_1, sizeof(data->client_id));
            if (status == 0) status = ReadFromData(&data->user_id, msg->data_block_1, sizeof(data->user_id));
            if (status == 0) status = ReadFromData(&data->connection_id, msg->data_block_1, sizeof(data->connection_id));
            if (status == 0) status = ReadFromData(&data->file_id, msg->data_block_1, sizeof(data->file_id));
            break;
        }
        case 7: {
            CGCMBMessageParsedData_Type7 *data = calloc(1, sizeof(CGCMBMessageParsedData_Type7));
            if (!data) return -1;
            msg->parsed_data = data;

            if (status == 0) status = ReadFromData(&data->client_id, msg->data_block_1, sizeof(data->client_id));
            if (status == 0) status = ReadFromData(&data->user_id, msg->data_block_1, sizeof(data->user_id));
            if (status == 0) status = ReadFromData(&data->connection_id, msg->data_block_1, sizeof(data->connection_id));
            if (status == 0) status = ReadFromData(&data->file_id, msg->data_block_1, sizeof(data->file_id));
            if (status == 0) status = ReadFromData(&data->read_size, msg->data_block_2, sizeof(data->read_size));
            if (status == 0) status = ReadFromData(&data->offset, msg->data_block_2, sizeof(data->offset));
            break;
        }
        case 8: {
            CGCMBMessageParsedData_Type8 *data = calloc(1, sizeof(CGCMBMessageParsedData_Type8));
            if (!data) return -1;
            msg->parsed_data = data;

            if (status == 0) status = ReadFromData(&data->client_id, msg->data_block_1, sizeof(data->client_id));
            if (status == 0) status = ReadFromData(&data->user_id, msg->data_block_1, sizeof(data->user_id));
            if (status == 0) status = ReadFromData(&data->connection_id, msg->data_block_1, sizeof(data->connection_id));
            if (status == 0) status = ReadFromData(&data->file_id, msg->data_block_1, sizeof(data->file_id));
            if (status == 0) status = ReadFromData(&data->field_5, msg->data_block_1, sizeof(data->field_5));
            if (status == 0) status = ReadFromData(&data->write_size, msg->data_block_2, sizeof(data->write_size));
            if (status == 0) status = ReadFromData(&data->offset, msg->data_block_2, sizeof(data->offset));

            if (status == 0 && data->write_size != 0 && data->write_size < 0x801) {
                data->write_data = calloc(data->write_size, 1);
                if (!data->write_data) { status = -1; break; }
                status = ReadFromData(data->write_data, msg->data_block_2, data->write_size);
            }
            break;
        }
        case 10: {
            CGCMBMessageParsedData_Type10 *data = calloc(1, sizeof(CGCMBMessageParsedData_Type10));
            if (!data) return -1;
            msg->parsed_data = data;

            if (status == 0) status = ReadFromData(&data->client_id, msg->data_block_1, sizeof(data->client_id));
            if (status == 0) status = ReadFromData(&data->user_id, msg->data_block_1, sizeof(data->user_id));
            if (status == 0) status = ReadFromData(&data->connection_id, msg->data_block_1, sizeof(data->connection_id));
            if (status == 0) status = ReadFromData(&data->file_id, msg->data_block_1, sizeof(data->file_id));
            break;
        }
        case 0xb:
        case 0xc:
            // No specific parsed data for these types
            break;
        default:
            return -1; // Unknown message type
    }
    
    if (status != 0) {
        // Free parsed data on error
        DestroyCGCMBMessage(&msg); // This will handle freeing parsed_data
        return -1;
    }
    return 0;
}

// Function: FindConnectionByID
Connection* FindConnectionByID(uint32 connection_id) {
    for (int i = 0; i < 32; ++i) {
        Connection *conn = mbServerState.connections[i];
        if (conn && conn->id == connection_id) {
            return conn;
        }
    }
    return NULL;
}

// Function: FindFileByID
File* FindFileByID(Connection *conn, uint32 file_id) {
    if (!conn) return NULL;

    for (int i = 0; i < 16; ++i) {
        File *file = (File*)conn->files[i];
        if (file && file->id == file_id) {
            return file;
        }
    }
    return NULL;
}

// Function: HandleCGCMBMessage
int HandleCGCMBMessage(CGCMBMessage *msg) {
    if (!msg) {
        return -1;
    }

    if (VerifyCGCMBMessage(msg) != 0) {
        SendErrorResponse(ERROR_INVALID_MESSAGE);
        return 0; // Return 0 for processed, even if error
    }

    int status = 0;

    switch (msg->header->type) {
        case 1: { // ClientHello
            if (mbServerState.status != 0) {
                SendErrorResponse(ERROR_INVALID_STATE);
                return -1;
            }
            CGCMBMessageParsedData_Type1 *parsed_data = (CGCMBMessageParsedData_Type1 *)msg->parsed_data;
            if (!parsed_data || !parsed_data->data) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                return -1;
            }

            CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
            if (!response_msg) { SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; } // Allocation failure

            response_msg->header->type = 0xf1;
            mbServerState.status = 1; // Set state to awaiting authentication

            uint16 selected_type = 0;
            for (uint32 i = 0; i < parsed_data->count; ++i) {
                if (parsed_data->data[i] == 0x313e || parsed_data->data[i] == 0x4155 || parsed_data->data[i] == 0x2671) {
                    selected_type = parsed_data->data[i];
                    break;
                }
            }
            mbServerState.type_field = selected_type;
            mbServerState.random_id = rand();

            AddToData(response_msg->data_block_2, &mbServerState.random_id, sizeof(mbServerState.random_id));
            AddToData(response_msg->data_block_2, &mbServerState.type_field, sizeof(mbServerState.type_field));

            if (mbServerState.type_field == 0) {
                mbServerState.status = 3; // Negotiation failed
            }
            TransmitCGCMBMessage(response_msg);
            DestroyCGCMBMessage(&response_msg);
            break;
        }
        case 2: { // Authenticate
            if (mbServerState.status != 1) {
                SendErrorResponse(ERROR_INVALID_STATE);
                return -1;
            }
            CGCMBMessageParsedData_Type2 *parsed_data = (CGCMBMessageParsedData_Type2 *)msg->parsed_data;
            if (!parsed_data || !parsed_data->username || !parsed_data->password) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                return -1;
            }
            if (mbServerState.random_id != parsed_data->client_id) {
                SendErrorResponse(ERROR_INVALID_CLIENT_ID);
                return -1;
            }

            mbServerState.status = 2; // Set state to authenticated
            mbServerState.user_id = LookupUser(parsed_data->username, parsed_data->password);

            CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
            if (!response_msg) { SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; }

            response_msg->header->type = 0xf2;
            AddToData(response_msg->data_block_2, &mbServerState.user_id, sizeof(mbServerState.user_id));
            uint8 auth_success = (mbServerState.user_id != (uint16)-1) ? 0 : 1;
            AddToData(response_msg->data_block_2, &auth_success, sizeof(auth_success));
            TransmitCGCMBMessage(response_msg);
            DestroyCGCMBMessage(&response_msg);
            break;
        }
        case 3: { // OpenConnection
            if (mbServerState.status != 2) {
                SendErrorResponse(ERROR_INVALID_STATE);
                return -1;
            }
            CGCMBMessageParsedData_Type3 *parsed_data = (CGCMBMessageParsedData_Type3 *)msg->parsed_data;
            if (!parsed_data || !parsed_data->path || !parsed_data->filename) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                return -1;
            }
            if (mbServerState.random_id != parsed_data->client_id) {
                SendErrorResponse(ERROR_INVALID_CLIENT_ID);
                return -1;
            }
            if (mbServerState.user_id != parsed_data->user_id) {
                SendErrorResponse(ERROR_INVALID_USER_ID);
                return -1;
            }

            uint32 tree_ptr = FindTreeByPath(parsed_data->user_id, parsed_data->path, parsed_data->filename);
            if (tree_ptr == 0) {
                SendErrorResponse(ERROR_TREE_NOT_FOUND);
                return -1;
            }

            Connection *new_conn = NULL;
            int conn_idx = -1;
            for (int i = 0; i < 32; ++i) {
                if (mbServerState.connections[i] == NULL) {
                    new_conn = calloc(1, sizeof(Connection));
                    if (!new_conn) break; // Allocation failed
                    mbServerState.connections[i] = new_conn;
                    conn_idx = i;
                    break;
                }
            }

            if (!new_conn) {
                SendErrorResponse(ERROR_NO_FREE_CONNECTIONS);
                return -1;
            }

            new_conn->id = rand();
            new_conn->status_field = 0; // Initialize
            new_conn->tree_ptr = (void*)tree_ptr;

            CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
            if (!response_msg) { free(new_conn); mbServerState.connections[conn_idx] = NULL; SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; }

            response_msg->header->type = 0xf3;
            AddToData(response_msg->data_block_1, &parsed_data->client_id, sizeof(parsed_data->client_id));
            AddToData(response_msg->data_block_1, &parsed_data->user_id, sizeof(parsed_data->user_id));
            AddToData(response_msg->data_block_2, &new_conn->id, sizeof(new_conn->id));
            TransmitCGCMBMessage(response_msg);
            DestroyCGCMBMessage(&response_msg);
            break;
        }
        case 4: { // CloseConnection
            if (mbServerState.status != 2) {
                SendErrorResponse(ERROR_INVALID_STATE);
                return -1;
            }
            CGCMBMessageParsedData_Type4 *parsed_data = (CGCMBMessageParsedData_Type4 *)msg->parsed_data;
            if (!parsed_data) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                return -1;
            }
            if (mbServerState.random_id != parsed_data->client_id) {
                SendErrorResponse(ERROR_INVALID_CLIENT_ID);
                return -1;
            }
            if (mbServerState.user_id != parsed_data->user_id) {
                SendErrorResponse(ERROR_INVALID_USER_ID);
                return -1;
            }

            for (int i = 0; i < 32; ++i) {
                Connection *conn = mbServerState.connections[i];
                if (conn && conn->id == parsed_data->connection_id) {
                    free(conn);
                    mbServerState.connections[i] = NULL;

                    CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
                    if (!response_msg) { SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; }

                    response_msg->header->type = 0xf4;
                    AddToData(response_msg->data_block_1, &parsed_data->client_id, sizeof(parsed_data->client_id));
                    AddToData(response_msg->data_block_1, &parsed_data->user_id, sizeof(parsed_data->user_id));
                    AddToData(response_msg->data_block_1, &parsed_data->connection_id, sizeof(parsed_data->connection_id));
                    TransmitCGCMBMessage(response_msg);
                    DestroyCGCMBMessage(&response_msg);
                    return 0;
                }
            }
            SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
            break;
        }
        case 5: { // OpenFile
            if (mbServerState.status != 2) {
                SendErrorResponse(ERROR_INVALID_STATE);
                return -1;
            }
            CGCMBMessageParsedData_Type5 *parsed_data = (CGCMBMessageParsedData_Type5 *)msg->parsed_data;
            if (!parsed_data || !parsed_data->filename) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                return -1;
            }
            if (mbServerState.random_id != parsed_data->client_id) {
                SendErrorResponse(ERROR_INVALID_CLIENT_ID);
                return -1;
            }
            if (mbServerState.user_id != parsed_data->user_id) {
                SendErrorResponse(ERROR_INVALID_USER_ID);
                return -1;
            }

            Connection *conn = FindConnectionByID(parsed_data->connection_id);
            if (!conn) {
                SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                break;
            }

            for (int i = 0; i < 16; ++i) {
                if (conn->files[i] == 0) {
                    uint32 file_ptr = CreateFile(conn->tree_ptr, parsed_data->filename, parsed_data->user_id, parsed_data->access_flags);
                    if (file_ptr != 0) {
                        conn->files[i] = file_ptr;

                        CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
                        if (!response_msg) { SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; }

                        response_msg->header->type = 0xf5;
                        AddToData(response_msg->data_block_1, &parsed_data->client_id, sizeof(parsed_data->client_id));
                        AddToData(response_msg->data_block_1, &parsed_data->user_id, sizeof(parsed_data->user_id));
                        AddToData(response_msg->data_block_1, &parsed_data->connection_id, sizeof(parsed_data->connection_id));
                        AddToData(response_msg->data_block_1, &((File*)file_ptr)->id, sizeof(((File*)file_ptr)->id));
                        TransmitCGCMBMessage(response_msg);
                        DestroyCGCMBMessage(&response_msg);
                        return 0;
                    }
                    SendErrorResponse(ERROR_FILE_OPERATION_FAILED);
                    break;
                }
            }
            SendErrorResponse(ERROR_NO_FREE_CONNECTIONS);
            break;
        }
        case 6: { // CloseFile
            if (mbServerState.status != 2) {
                SendErrorResponse(ERROR_INVALID_STATE);
                return -1;
            }
            CGCMBMessageParsedData_Type6 *parsed_data = (CGCMBMessageParsedData_Type6 *)msg->parsed_data;
            if (!parsed_data) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                return -1;
            }
            if (mbServerState.random_id != parsed_data->client_id) {
                SendErrorResponse(ERROR_INVALID_CLIENT_ID);
                return -1;
            }
            if (mbServerState.user_id != parsed_data->user_id) {
                SendErrorResponse(ERROR_INVALID_USER_ID);
                return -1;
            }

            Connection *conn = FindConnectionByID(parsed_data->connection_id);
            if (!conn) {
                SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                break;
            }
            File *file = FindFileByID(conn, parsed_data->file_id);
            if (!file) {
                SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                break;
            }
            CloseFile((uint32)file); // Pass the pointer value

            CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
            if (!response_msg) { SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; }

            response_msg->header->type = 0xf6;
            AddToData(response_msg->data_block_1, &parsed_data->client_id, sizeof(parsed_data->client_id));
            AddToData(response_msg->data_block_1, &parsed_data->user_id, sizeof(parsed_data->user_id));
            AddToData(response_msg->data_block_1, &parsed_data->connection_id, sizeof(parsed_data->connection_id));
            AddToData(response_msg->data_block_1, &parsed_data->file_id, sizeof(parsed_data->file_id));
            TransmitCGCMBMessage(response_msg);
            DestroyCGCMBMessage(&response_msg);
            break;
        }
        case 7: { // ReadFile
            if (mbServerState.status != 2) {
                SendErrorResponse(ERROR_INVALID_STATE);
                return -1;
            }
            CGCMBMessageParsedData_Type7 *parsed_data = (CGCMBMessageParsedData_Type7 *)msg->parsed_data;
            if (!parsed_data) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                return -1;
            }
            if (mbServerState.random_id != parsed_data->client_id) {
                SendErrorResponse(ERROR_INVALID_CLIENT_ID);
                return -1;
            }
            if (mbServerState.user_id != parsed_data->user_id) {
                SendErrorResponse(ERROR_INVALID_USER_ID);
                return -1;
            }

            Connection *conn = FindConnectionByID(parsed_data->connection_id);
            if (!conn) {
                SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                break;
            }
            File *file = FindFileByID(conn, parsed_data->file_id);
            if (!file) {
                SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                break;
            }
            if (parsed_data->read_size == 0 || parsed_data->read_size > 0x800) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                break;
            }

            void *read_buffer = calloc(parsed_data->read_size, 1);
            if (!read_buffer) { SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; }

            status = ReadFile(read_buffer, (uint32)file, parsed_data->offset, parsed_data->read_size);
            if (status == 0) {
                CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
                if (!response_msg) { free(read_buffer); SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; }

                response_msg->header->type = 0xf7;
                AddToData(response_msg->data_block_1, &parsed_data->client_id, sizeof(parsed_data->client_id));
                AddToData(response_msg->data_block_1, &parsed_data->user_id, sizeof(parsed_data->user_id));
                AddToData(response_msg->data_block_1, &parsed_data->connection_id, sizeof(parsed_data->connection_id));
                AddToData(response_msg->data_block_1, &parsed_data->read_size, sizeof(parsed_data->read_size));
                AddToData(response_msg->data_block_2, read_buffer, parsed_data->read_size);
                TransmitCGCMBMessage(response_msg);
                DestroyCGCMBMessage(&response_msg);
            } else {
                SendErrorResponse(ERROR_FILE_OPERATION_FAILED);
            }
            free(read_buffer);
            break;
        }
        case 8: { // WriteFile
            if (mbServerState.status != 2) {
                SendErrorResponse(ERROR_INVALID_STATE);
                return -1;
            }
            CGCMBMessageParsedData_Type8 *parsed_data = (CGCMBMessageParsedData_Type8 *)msg->parsed_data;
            if (!parsed_data || !parsed_data->write_data) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                return -1;
            }
            if (mbServerState.random_id != parsed_data->client_id) {
                SendErrorResponse(ERROR_INVALID_CLIENT_ID);
                return -1;
            }
            if (mbServerState.user_id != parsed_data->user_id) {
                SendErrorResponse(ERROR_INVALID_USER_ID);
                return -1;
            }

            Connection *conn = FindConnectionByID(parsed_data->connection_id);
            if (!conn) {
                SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                break;
            }
            File *file = FindFileByID(conn, parsed_data->file_id);
            if (!file) {
                SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                break;
            }
            if (parsed_data->write_size == 0 || parsed_data->write_size > 0x800) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                break;
            }

            status = WriteFile((uint32)file, parsed_data->write_data, parsed_data->offset, parsed_data->write_size);
            if (status == 0) {
                CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
                if (!response_msg) { SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; }

                response_msg->header->type = 0xf8;
                AddToData(response_msg->data_block_1, &parsed_data->client_id, sizeof(parsed_data->client_id));
                AddToData(response_msg->data_block_1, &parsed_data->user_id, sizeof(parsed_data->user_id));
                AddToData(response_msg->data_block_1, &parsed_data->connection_id, sizeof(parsed_data->connection_id));
                AddToData(response_msg->data_block_1, &parsed_data->file_id, sizeof(parsed_data->file_id));
                TransmitCGCMBMessage(response_msg);
                DestroyCGCMBMessage(&response_msg);
            } else {
                SendErrorResponse(ERROR_FILE_OPERATION_FAILED);
            }
            break;
        }
        case 10: { // RPCMessage
            if (mbServerState.status != 2) {
                SendErrorResponse(ERROR_INVALID_STATE);
                return -1;
            }
            CGCMBMessageParsedData_Type10 *parsed_data = (CGCMBMessageParsedData_Type10 *)msg->parsed_data;
            if (!parsed_data) {
                SendErrorResponse(ERROR_INVALID_MESSAGE);
                return -1;
            }
            if (mbServerState.random_id != parsed_data->client_id) {
                SendErrorResponse(ERROR_INVALID_CLIENT_ID);
                return -1;
            }
            if (mbServerState.user_id != parsed_data->user_id) {
                SendErrorResponse(ERROR_INVALID_USER_ID);
                return -1;
            }

            Connection *conn = FindConnectionByID(parsed_data->connection_id);
            if (!conn) {
                SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                break;
            }
            if (!conn->tree_ptr) { // Assuming tree_ptr contains the RPC context
                SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                break;
            }

            // This strcmp is highly specific to the original context. Assuming it checks for a magic string in the tree_ptr structure.
            if (strcmp((char*)conn->tree_ptr + 0x84, "EYEPSEE") == 0) {
                File *file = FindFileByID(conn, parsed_data->file_id);
                if (!file) {
                    SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                    break;
                }
                // This condition `*file == **(short **)(conn->tree_ptr + 0xc4)` suggests `conn->tree_ptr` also points to some RPC-related info.
                // Assuming `*(uint16*)conn->tree_ptr_plus_0xc4` is a specific file ID for RPC.
                if (file->id == *(uint16*)((uint8*)conn->tree_ptr + 0xc4)) {
                    void *rpc_response_data = NULL;
                    int rpc_response_size = 0;
                    uint8 rpc_status = HandleCGCRPCMessage((uint32)msg->data_block_2->data_ptr, msg->data_block_2->size, &rpc_response_data, &rpc_response_size);

                    CGCMBMessage *response_msg = CreateBlankCGCMBMessage();
                    if (!response_msg) { free(rpc_response_data); SendErrorResponse(ERROR_FILE_OPERATION_FAILED); return -1; }

                    response_msg->header->type = 0xfa;
                    AddToData(response_msg->data_block_1, &parsed_data->client_id, sizeof(parsed_data->client_id));
                    AddToData(response_msg->data_block_1, &parsed_data->user_id, sizeof(parsed_data->user_id));
                    AddToData(response_msg->data_block_1, &parsed_data->connection_id, sizeof(parsed_data->connection_id));
                    AddToData(response_msg->data_block_1, &rpc_status, sizeof(rpc_status));

                    if (rpc_response_size != 0 && rpc_response_data) {
                        AddToData(response_msg->data_block_2, rpc_response_data, rpc_response_size);
                        free(rpc_response_data);
                    }
                    TransmitCGCMBMessage(response_msg);
                    DestroyCGCMBMessage(&response_msg);
                } else {
                    SendErrorResponse(ERROR_CONNECTION_OR_FILE_NOT_FOUND);
                }
            } else {
                SendErrorResponse(ERROR_RPC_UNSUPPORTED);
            }
            break;
        }
        case 0xb: { // Ping
            msg->header->type = 0xfb; // Pong
            TransmitCGCMBMessage(msg);
            break;
        }
        case 0xc: { // Disconnect
            mbServerState.status = 3; // Set state to disconnected
            msg->header->type = 0xfc; // Disconnect confirmation
            TransmitCGCMBMessage(msg);
            return 1; // Special return for disconnect
        }
    }
    return 0;
}

// Function: DestroyCGCMBMessage
void DestroyCGCMBMessage(CGCMBMessage **msg_ptr) {
    if (!msg_ptr || !*msg_ptr) {
        return;
    }

    CGCMBMessage *msg = *msg_ptr;

    if (msg->parsed_data) {
        // Free specific members of parsed_data based on message type
        switch (msg->header->type) {
            case 1: {
                CGCMBMessageParsedData_Type1 *data = (CGCMBMessageParsedData_Type1 *)msg->parsed_data;
                free(data->data);
                break;
            }
            case 2: {
                CGCMBMessageParsedData_Type2 *data = (CGCMBMessageParsedData_Type2 *)msg->parsed_data;
                free(data->username);
                free(data->password);
                break;
            }
            case 3: {
                CGCMBMessageParsedData_Type3 *data = (CGCMBMessageParsedData_Type3 *)msg->parsed_data;
                free(data->path);
                free(data->filename);
                free(data->field_4);
                break;
            }
            case 5: {
                CGCMBMessageParsedData_Type5 *data = (CGCMBMessageParsedData_Type5 *)msg->parsed_data;
                free(data->filename);
                break;
            }
            case 8: {
                CGCMBMessageParsedData_Type8 *data = (CGCMBMessageParsedData_Type8 *)msg->parsed_data;
                free(data->write_data);
                break;
            }
            // Other types don't have dynamically allocated members within parsed_data
            case 4:
            case 6:
            case 7:
            case 10:
            case 0xb:
            case 0xc:
            default:
                break;
        }
        free(msg->parsed_data);
        msg->parsed_data = NULL;
    }

    if (msg->data_block_2) {
        free(msg->data_block_2->data_ptr);
        free(msg->data_block_2);
        msg->data_block_2 = NULL;
    }
    if (msg->data_block_1) {
        free(msg->data_block_1->data_ptr);
        free(msg->data_block_1);
        msg->data_block_1 = NULL;
    }
    if (msg->header) {
        free(msg->header);
        msg->header = NULL;
    }
    free(msg);
    *msg_ptr = NULL;
}

// Function: CreateBlankCGCMBMessage
CGCMBMessage* CreateBlankCGCMBMessage(void) {
    CGCMBMessage *msg = calloc(1, sizeof(CGCMBMessage));
    if (!msg) return NULL;

    msg->header = calloc(1, sizeof(CGCMBMessageHeader));
    if (!msg->header) { free(msg); return NULL; }

    msg->data_block_1 = calloc(1, sizeof(CGCMBDataBlock));
    if (!msg->data_block_1) { free(msg->header); free(msg); return NULL; }

    msg->data_block_2 = calloc(1, sizeof(CGCMBDataBlock));
    if (!msg->data_block_2) { free(msg->data_block_1); free(msg->header); free(msg); return NULL; }

    msg->header->magic = 0xc47c4d42; // Magic number
    return msg;
}

// Function: VerifyCGCMBMessage
int VerifyCGCMBMessage(CGCMBMessage *msg) {
    if (!msg || !msg->header) {
        return -1;
    }
    if (msg->header->magic != 0xc47c4d42) {
        return -1;
    }

    // Check data_block_2 integrity
    if (msg->data_block_2) {
        if (msg->data_block_2->size != 0 && msg->data_block_2->current_offset != 0) {
            return -1; // Data block 2 has data but is partially read
        }
    }

    // Check data_block_1 integrity
    if (msg->data_block_1) {
        if (msg->data_block_1->size != 0 && msg->data_block_1->current_offset != 0) {
            return -1; // Data block 1 has data but is partially read
        }
    }
    return 0; // Message is valid
}

// Function: TransmitCGCMBMessage
void TransmitCGCMBMessage(CGCMBMessage *msg) {
    if (VerifyCGCMBMessage(msg) != 0) {
        SendErrorResponse(ERROR_INVALID_MESSAGE);
        return;
    }

    TransportMessage *t_msg = calloc(1, sizeof(TransportMessage));
    if (!t_msg) {
        SendErrorResponse(ERROR_FILE_OPERATION_FAILED); // Allocation failure
        return;
    }

    // Calculate total size for the transport message payload
    t_msg->total_size = sizeof(CGCMBMessageHeader) +
                        sizeof(msg->data_block_1->size) + msg->data_block_1->size +
                        sizeof(msg->data_block_2->size) + msg->data_block_2->size;

    if (t_msg->total_size > 0x1000) { // Max 4096 bytes
        SendErrorResponse(ERROR_MESSAGE_TOO_LARGE);
        DestroyTransportMessage(&t_msg);
        return;
    }

    t_msg->data_ptr = calloc(t_msg->total_size, 1);
    if (!t_msg->data_ptr) {
        SendErrorResponse(ERROR_FILE_OPERATION_FAILED); // Allocation failure
        DestroyTransportMessage(&t_msg);
        return;
    }

    uint8 *current_ptr = (uint8*)t_msg->data_ptr;

    // Copy header
    memcpy(current_ptr, &msg->header->magic, sizeof(msg->header->magic)); current_ptr += sizeof(msg->header->magic);
    memcpy(current_ptr, &msg->header->type, sizeof(msg->header->type)); current_ptr += sizeof(msg->header->type);
    memcpy(current_ptr, &msg->header->field_5, sizeof(msg->header->field_5)); current_ptr += sizeof(msg->header->field_5);
    memcpy(current_ptr, &msg->header->field_6, sizeof(msg->header->field_6)); current_ptr += sizeof(msg->header->field_6);
    memcpy(current_ptr, msg->header->field_7, sizeof(msg->header->field_7)); current_ptr += sizeof(msg->header->field_7);

    // Copy data_block_1 size and data
    memcpy(current_ptr, &msg->data_block_1->size, sizeof(msg->data_block_1->size)); current_ptr += sizeof(msg->data_block_1->size);
    if (msg->data_block_1->size != 0 && msg->data_block_1->data_ptr) {
        memcpy(current_ptr, msg->data_block_1->data_ptr, msg->data_block_1->size);
        current_ptr += msg->data_block_1->size;
    }

    // Copy data_block_2 size and data
    memcpy(current_ptr, &msg->data_block_2->size, sizeof(msg->data_block_2->size)); current_ptr += sizeof(msg->data_block_2->size);
    if (msg->data_block_2->size != 0 && msg->data_block_2->data_ptr) {
        memcpy(current_ptr, msg->data_block_2->data_ptr, msg->data_block_2->size);
        current_ptr += msg->data_block_2->size;
    }

    SendTransportMessage(t_msg);
    DestroyTransportMessage(&t_msg);
}