#include <stdio.h>   // For NULL
#include <stdlib.h>  // For calloc, free, exit
#include <stddef.h>  // For size_t

// Custom type definitions based on original snippet
typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned char undefined;
typedef unsigned int undefined4; // Represents a 4-byte unsigned integer

// External function and variable declarations
// These functions are assumed to be defined elsewhere in the system.
// Their signatures are inferred from the usage in the provided snippet.
extern int receive(int fd, void *buf, size_t len, int *bytes_received);
extern int transmit(int fd, const void *buf, size_t len, int *bytes_sent);

// Graph-related functions
extern void DestroyNodes(void);
extern void DestroyEdges(void);

// Node related functions
// FindNode: Returns a node identifier (e.g., node_id or handle) if found, 0 if not.
extern unsigned int FindNode(unsigned int node_id);
// AddNode: Returns non-zero on success, 0 on failure. Takes a pointer to a node structure.
extern int AddNode(void *node_struct);

// Edge related functions
// FindEdge: Returns a pointer to the existing edge structure if found, NULL (0) if not.
// The arguments are node identifiers/handles previously returned by FindNode.
extern void *FindEdge(unsigned int source_node_id, unsigned int dest_node_id);
// AddEdge: Returns non-zero on success, 0 on failure. Takes a pointer to an edge structure.
extern int AddEdge(void *edge_struct);

// Shortest Path Tree (SPT) related function
// FindSpt: Returns a pointer to the SPT result data (e.g., a buffer or a struct containing data and length), or NULL on error.
extern void *FindSpt(void);

// Global variables (inferred from usage)
extern int NumEdges;
extern int NumNodes;
extern char *rand_page; // Assuming this is a pointer to a char array/page used for random values.

// Structure definitions inferred from usage:
// Node structure (assuming 20 bytes total, with ID at offset 0xc and distance at 0x10)
// The fields are guessed based on memory accesses like `((int)local_28 + 0xc)`
struct Node {
    char unknown_fields_0_11[12]; // Placeholder for unknown fields
    unsigned int id;              // Access at offset 0xc
    unsigned int distance;        // Access at offset 0x10
    // ... possibly more fields up to 20 bytes total
};

// Edge structure (assuming 20 bytes total, with source at 0x8, dest at 0xc, weight at 0x10)
// The fields are guessed based on memory accesses like `((int)local_1c + 8)`
struct Edge {
    char unknown_fields_0_7[8];   // Placeholder for unknown fields
    unsigned int source_node_id;  // Access at offset 0x8
    unsigned int dest_node_id;    // Access at offset 0xc
    unsigned int weight;          // Access at offset 0x10
    // ... possibly more fields up to 20 bytes total
};

// SPT Result structure (guessed for FindSpt return type)
// Assumes FindSpt returns a dynamically allocated struct containing length and data.
struct SptResult {
    unsigned int length_in_uints; // The number of unsigned int elements in the data array
    unsigned int data[];          // Flexible array member for the actual SPT data
};


// Function: ReadBytes
unsigned int ReadBytes(void *buffer, unsigned int bytes_to_read) {
    unsigned int total_read = 0;
    int bytes_received_this_call;
    int receive_status;

    while (total_read < bytes_to_read) {
        receive_status = receive(0, (char*)buffer + total_read, bytes_to_read - total_read, &bytes_received_this_call);

        if (receive_status != 0) { // Error from receive function
            break;
        }

        if (bytes_received_this_call == 0) { // No bytes received, possibly EOF or error
            return 0;
        }
        total_read += bytes_received_this_call;
    }

    if (total_read != bytes_to_read) {
        return 0; // Did not read all requested bytes
    }
    return total_read;
}

// Function: ReadNull
unsigned int ReadNull(size_t bytes_to_read) {
    if (bytes_to_read == 0) {
        return 0;
    }

    void *buffer = calloc(bytes_to_read, 1);
    if (buffer == NULL) {
        exit(1); // Replaced _terminate() with exit for Linux compilation
    }

    unsigned int total_read = 0;
    int bytes_received_this_call;
    int receive_status;

    while (total_read < bytes_to_read) {
        receive_status = receive(0, (char*)buffer + total_read, bytes_to_read - total_read, &bytes_received_this_call);

        if (receive_status != 0) { // Error from receive
            break;
        }

        if (bytes_received_this_call == 0) { // No bytes received, possibly EOF
            free(buffer);
            return 0;
        }
        total_read += bytes_received_this_call;
    }

    free(buffer); // Free the buffer after consuming bytes

    if (total_read == bytes_to_read) {
        return total_read; // Successfully read all bytes
    } else {
        return 0; // Did not read all requested bytes
    }
}

// Function: SendBytes
unsigned int SendBytes(const void *buffer, unsigned int bytes_to_send) {
    unsigned int total_sent = 0;
    int bytes_sent_this_call;
    int transmit_status;

    while (total_sent < bytes_to_send) {
        transmit_status = transmit(1, (const char*)buffer + total_sent, bytes_to_send - total_sent, &bytes_sent_this_call);

        if (transmit_status != 0) { // Error from transmit function
            break;
        }

        if (bytes_sent_this_call == 0) { // No bytes sent, possibly error
            return 0;
        }
        total_sent += bytes_sent_this_call;
    }

    if (total_sent != bytes_to_send) {
        return 0; // Did not send all requested bytes
    }
    return total_sent;
}

// Function: SendResponse
// status_byte: status/command byte for the response
// data_length_in_uints: number of 4-byte units in the data_buffer
// data_buffer: pointer to the data to be sent (can be NULL if data_length_in_uints is 0)
unsigned int SendResponse(unsigned char status_byte, unsigned char data_length_in_uints, const void *data_buffer) {
    struct {
        unsigned char status;
        unsigned char length_units;
    } header;
    header.status = status_byte;
    header.length_units = data_length_in_uints;

    // Send the 2-byte header
    if (SendBytes(&header, sizeof(header)) != sizeof(header)) {
        return 0; // Failed to send header
    }

    // If there's data, send it
    if (data_length_in_uints != 0) {
        unsigned int data_bytes_to_send = (unsigned int)data_length_in_uints * sizeof(unsigned int);
        if (SendBytes(data_buffer, data_bytes_to_send) != data_bytes_to_send) {
            return 0; // Failed to send data
        }
    }

    return 1; // Success
}

// Function: SendErrorResponse
// error_code: The specific error code to send
void SendErrorResponse(unsigned char error_code) {
    SendResponse(error_code, 0, NULL);
}

// Function: ReadCmd
unsigned int ReadCmd(void) {
    // Structure for the 13-byte command header, inferred from local_35 and local_34 usage
    struct CommandHeader {
        char unknown_field_0_6[7];         // Placeholder for unknown bytes
        unsigned char command_type;        // Corresponds to local_35 at offset 7
        unsigned int data_length;          // Corresponds to local_34 at offset 8
        char unknown_field_12;             // Placeholder for the last byte to make it 13 bytes
    } cmd_header;

    // Read the 13-byte command header
    if (ReadBytes(&cmd_header, sizeof(cmd_header)) != sizeof(cmd_header)) {
        return 0; // Failed to read full header
    }

    unsigned char command_type = cmd_header.command_type;
    unsigned int data_length = cmd_header.data_length; // Number of nodes/edges or other data units

    // Command Type 1: Add Nodes
    if (command_type == 0x01) {
        // Check for maximum nodes (0xff implies 255 nodes based on original code's check)
        if (0xff < data_length) {
            ReadNull(data_length * sizeof(unsigned int)); // Consume remaining input for too large request
            SendErrorResponse(1); // Assuming error code 1 for invalid length
            return 0;
        }

        // Allocate buffer for node IDs (each ID is 4 bytes)
        unsigned int *node_ids_buffer = (unsigned int *)calloc(data_length, sizeof(unsigned int));
        if (node_ids_buffer == NULL) {
            DestroyNodes();
            DestroyEdges();
            exit(1); // Fatal error: memory allocation failed
        }

        // Read node IDs into the buffer
        if (ReadBytes(node_ids_buffer, data_length * sizeof(unsigned int)) != data_length * sizeof(unsigned int)) {
            free(node_ids_buffer);
            return 0; // Failed to read all node IDs
        }

        // Check if any node already exists
        for (unsigned int i = 0; i < data_length; ++i) {
            if (FindNode(node_ids_buffer[i]) != 0) { // If node found (non-zero return)
                free(node_ids_buffer);
                SendErrorResponse(2); // Assuming error code 2 for node already exists
                return 0;
            }
        }

        // Add new nodes
        for (unsigned int i = 0; i < data_length; ++i) {
            struct Node *new_node = (struct Node *)calloc(1, 20); // Allocate 20 bytes for a new node
            if (new_node == NULL) {
                free(node_ids_buffer);
                DestroyNodes();
                DestroyEdges();
                exit(1); // Fatal error: memory allocation failed
            }
            new_node->id = node_ids_buffer[i];
            new_node->distance = 0xffffffff; // Initialize distance to infinity

            if (AddNode(new_node) == 0) { // If AddNode fails (returns 0)
                free(new_node); // AddNode failed, so we free the allocated node structure
                free(node_ids_buffer);
                DestroyNodes();
                DestroyEdges();
                exit(1); // Fatal error: AddNode failed
            }
        }
        free(node_ids_buffer); // Free the buffer of node IDs
    }
    // Command Type 2: Add Edges
    else if (command_type == 0x02) {
        // Check for total edges limit (2000)
        if (2000 < NumEdges + data_length) {
            ReadNull(data_length * 12); // Consume remaining input for too many edges
            SendErrorResponse(3); // Assuming error code 3 for too many edges
            return 0;
        }

        // Allocate buffer for edge data (each edge entry is 12 bytes: src_id, dest_id, weight)
        unsigned int *edge_data_buffer = (unsigned int *)calloc(data_length, 12); // 3 unsigned ints per edge
        if (edge_data_buffer == NULL) {
            DestroyNodes();
            DestroyEdges();
            exit(1); // Fatal error: memory allocation failed
        }

        // Read edge data into the buffer
        if (ReadBytes(edge_data_buffer, data_length * 12) != data_length * 12) {
            free(edge_data_buffer);
            return 0; // Failed to read all edge data
        }

        for (unsigned int i = 0; i < data_length; ++i) {
            struct Edge *new_edge = (struct Edge *)calloc(1, 20); // Allocate 20 bytes for a new edge
            if (new_edge == NULL) {
                free(edge_data_buffer);
                DestroyNodes();
                DestroyEdges();
                exit(1); // Fatal error: memory allocation failed
            }

            // Extract source node ID from buffer and find node handle
            unsigned int source_id_from_buffer = edge_data_buffer[i * 3]; // Assuming 3 unsigned ints per edge
            unsigned int source_node_handle = FindNode(source_id_from_buffer);
            new_edge->source_node_id = source_node_handle;

            if (source_node_handle == 0) { // If source node not found
                SendErrorResponse(4); // Error code 4 for node not found
                free(new_edge);
                free(edge_data_buffer);
                DestroyNodes();
                DestroyEdges();
                exit(1); // Fatal error: source node not found
            }

            // Extract destination node ID from buffer and find node handle
            unsigned int dest_id_from_buffer = edge_data_buffer[i * 3 + 1];
            unsigned int dest_node_handle = FindNode(dest_id_from_buffer);
            new_edge->dest_node_id = dest_node_handle;

            if (dest_node_handle == 0) { // If destination node not found
                SendErrorResponse(4); // Error code 4 for node not found
                free(new_edge);
                free(edge_data_buffer);
                DestroyNodes();
                DestroyEdges();
                exit(1); // Fatal error: destination node not found
            }

            // Calculate edge weight
            unsigned int weight_from_buffer = edge_data_buffer[i * 3 + 2];
            // Interpretation of `(uint)*(byte *)(NumNodes + rand_page)`:
            // Assumed `rand_page` is a char array/pointer and `NumNodes` is an index into it.
            new_edge->weight = weight_from_buffer + (unsigned int)(unsigned char)rand_page[NumNodes];

            // Check if edge already exists
            // FindEdge returns a pointer to the existing edge struct if found, NULL otherwise.
            struct Edge *existing_edge = (struct Edge *)FindEdge(new_edge->source_node_id, new_edge->dest_node_id);

            if (existing_edge == NULL) { // Edge not found, add new one
                if (AddEdge(new_edge) == 0) { // If AddEdge fails
                    free(new_edge); // AddEdge failed, so we free it
                    free(edge_data_buffer);
                    DestroyNodes();
                    DestroyEdges();
                    exit(1); // Fatal error: AddEdge failed
                }
            } else { // Edge found, update weight if new one is smaller
                if (new_edge->weight < existing_edge->weight) {
                    existing_edge->weight = new_edge->weight;
                }
                free(new_edge); // Free the newly created edge struct as we updated the existing one
            }
        }
        free(edge_data_buffer); // Free the buffer of edge data
    }
    // Command Type 3: Find Shortest Path Tree (SPT)
    else if (command_type == 0x03) {
        struct SptResult *spt_result = (struct SptResult *)FindSpt();
        if (spt_result == NULL) {
            SendErrorResponse(5); // Assuming error code 5 for SPT calculation failure
            return 0;
        }
        // Send the SPT result. Assuming 0 for success status.
        if (SendResponse(0, spt_result->length_in_uints, spt_result->data) == 0) {
            free(spt_result); // Free result even if sending failed
            return 0; // Failed to send response
        }
        free(spt_result); // Free the SPT result buffer
    }
    // Unknown Command Type
    else {
        SendErrorResponse(6); // Assuming error code 6 for unknown command
        return 0;
    }

    return 1; // Command processed successfully
}