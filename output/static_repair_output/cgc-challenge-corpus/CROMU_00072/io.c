#include <stdlib.h> // For calloc, free
#include <string.h> // For memcpy
#include <stdint.h> // For uintptr_t

// External function declarations (assuming custom I/O and graph operations)
// These functions' actual implementations are not provided, so their signatures are inferred.

// Custom receive function:
//   sockfd: Socket descriptor (or a custom identifier, here '0' is used).
//   buf: Pointer to the buffer to read data into.
//   len: Maximum number of bytes to read.
//   bytes_read_out: Pointer to an int where the number of bytes actually read will be stored.
// Returns 0 on success, non-zero on error.
extern int receive_custom(int sockfd, void *buf, unsigned int len, int *bytes_read_out);

// Custom transmit function:
//   sockfd: Socket descriptor (or a custom identifier, here '1' is used).
//   buf: Pointer to the buffer containing data to send.
//   len: Number of bytes to send.
//   bytes_sent_out: Pointer to an int where the number of bytes actually sent will be stored.
// Returns 0 on success, non-zero on error.
extern int transmit_custom(int sockfd, const void *buf, unsigned int len, int *bytes_sent_out);

// Terminates the program, usually called on critical errors like memory allocation failure.
extern void _terminate(void);

// Graph management functions
extern void DestroyNodes(void);
extern void DestroyEdges(void);

// Find a node by its ID. Returns a pointer to the node structure if found, NULL otherwise.
// The actual structure of the node is opaque here, referred to by `void*`.
extern void *FindNode(unsigned int node_id);

// Add a new node to the graph. Returns 1 on success, 0 on failure.
// `node` is a pointer to the pre-allocated node structure.
extern int AddNode(void *node);

// Find an edge between two node pointers. Returns a pointer to the edge structure if found, NULL otherwise.
// The actual structure of the edge is opaque here, referred to by `void*`.
extern void *FindEdge(void *src_node_ptr, void *dest_node_ptr);

// Add a new edge to the graph. Returns 1 on success, 0 on failure.
// `edge` is a pointer to the pre-allocated edge structure.
extern int AddEdge(void *edge);

// Structure to hold Shortest Path Tree (SPT) results.
// Assuming FindSpt returns a pointer to such a structure.
struct SptResult {
    unsigned char length_in_4byte_units; // Length of data in 4-byte units
    void *data;                          // Pointer to SPT data
};
extern struct SptResult *FindSpt(void);

// Global variables (inferred from original snippet)
extern unsigned int NumEdges;
extern unsigned int NumNodes_count; // Renamed from NumNodes to avoid ambiguity
extern char *rand_page_ptr;         // Renamed from rand_page, assuming it's a pointer

// Function: ReadBytes
// Reads a specified number of bytes into a buffer from a custom input source (socket 0).
// buf: Pointer to the buffer to fill.
// len: Total number of bytes to read.
// Returns the number of bytes read on success (which should be 'len'), or 0 on failure.
unsigned int ReadBytes(char *buf, unsigned int len) {
    unsigned int total_read = 0;
    int current_read_count;
    int ret_val;

    while (total_read < len) {
        ret_val = receive_custom(0, buf + total_read, len - total_read, &current_read_count);
        if (ret_val != 0) { // Custom receive function failed
            return 0;
        }
        if (current_read_count == 0) { // Read 0 bytes (connection closed or EOF)
            return 0;
        }
        total_read += current_read_count;
    }
    return total_read; // On success, total_read should be equal to len
}

// Function: ReadNull
// Reads a specified number of bytes into a temporary buffer and discards them.
// size_to_read: Total number of bytes to read and discard.
// Returns the number of bytes read (which should be 'size_to_read') on success, or 0 on failure.
unsigned int ReadNull(unsigned int size_to_read) {
    if (size_to_read == 0) {
        return 0;
    }

    void *buffer = calloc(size_to_read, 1);
    if (buffer == NULL) {
        _terminate(); // Critical error: memory allocation failed
    }

    unsigned int total_read = 0;
    int current_read_count;
    int ret_val;

    while (total_read < size_to_read) {
        ret_val = receive_custom(0, (char *)buffer + total_read, size_to_read - total_read, &current_read_count);
        if (ret_val != 0 || current_read_count == 0) {
            free(buffer);
            return 0;
        }
        total_read += current_read_count;
    }

    free(buffer); // Discard the buffer
    return size_to_read; // On success, total_read should be equal to size_to_read
}

// Function: SendBytes
// Sends a specified number of bytes from a buffer using a custom output source (socket 1).
// buf: Pointer to the buffer containing data to send.
// len: Total number of bytes to send.
// Returns the number of bytes sent on success (which should be 'len'), or 0 on failure.
unsigned int SendBytes(const char *buf, unsigned int len) {
    unsigned int total_sent = 0;
    int current_sent_count;
    int ret_val;

    while (total_sent < len) {
        ret_val = transmit_custom(1, buf + total_sent, len - total_sent, &current_sent_count);
        if (ret_val != 0) { // Custom transmit function failed
            return 0;
        }
        if (current_sent_count == 0) { // Sent 0 bytes (connection closed or error)
            return 0;
        }
        total_sent += current_sent_count;
    }
    return total_sent; // On success, total_sent should be equal to len
}

// Function: SendResponse
// Sends a 2-byte header (type and length) followed by optional data.
// type: Response type.
// length: Length of the data in 4-byte units (actual bytes = length * 4).
// data_ptr: Pointer to the data to send.
// Returns 1 on success, 0 on failure.
unsigned int SendResponse(char type, char length, uintptr_t data_ptr) {
    char header[2];
    header[0] = type;
    header[1] = length;

    if (SendBytes(header, 2) != 2) {
        return 0; // Failed to send header
    }

    if (length != 0) {
        // Send the data block
        if (SendBytes((const char *)data_ptr, (unsigned int)length << 2) == 0) {
            return 0; // Failed to send data
        }
    }
    return 1; // Success
}

// Function: SendErrorResponse
// Sends an error response with a specific error code.
// error_code: The error type to send.
void SendErrorResponse(char error_code) {
    SendResponse(error_code, 0, 0); // Error responses typically have no data
}

// Function: ReadCmd
// Reads and processes a command from the input stream.
// Returns 1 on successful command processing, 0 on failure.
unsigned int ReadCmd(void) {
    char header_buf[13]; // Buffer for the 13-byte command header
    char cmd_type;
    unsigned int cmd_len_or_count; // Renamed from local_34

    // Read the 13-byte command header
    if (ReadBytes(header_buf, 13) != 13) {
        return 0; // Failed to read full header
    }

    cmd_type = header_buf[0];
    // Assuming cmd_len_or_count is a 4-byte unsigned int starting at offset 1 in the header.
    memcpy(&cmd_len_or_count, header_buf + 1, sizeof(unsigned int));

    if (cmd_type == 0x01) { // Command type 1: Add Nodes
        if (cmd_len_or_count > 0xff) { // Check for max nodes (255)
            ReadNull(cmd_len_or_count * sizeof(unsigned int)); // Consume remaining data for oversized request
            SendErrorResponse(cmd_type); // Placeholder error code
            return 0;
        }

        unsigned int *node_ids = (unsigned int *)calloc(cmd_len_or_count, sizeof(unsigned int));
        if (node_ids == NULL) {
            DestroyNodes();
            DestroyEdges();
            _terminate(); // Critical error: memory allocation failed
        }

        // Read node IDs into the buffer
        if (ReadBytes((char *)node_ids, cmd_len_or_count * sizeof(unsigned int)) != cmd_len_or_count * sizeof(unsigned int)) {
            free(node_ids);
            return 0;
        }

        // Check for duplicate node IDs
        for (unsigned int i = 0; i < cmd_len_or_count; ++i) {
            if (FindNode(node_ids[i]) != NULL) { // Assuming FindNode returns non-NULL if found
                free(node_ids);
                SendErrorResponse(cmd_type); // Placeholder error code for duplicate node
                return 0;
            }
        }

        // Add new nodes to the graph
        for (unsigned int i = 0; i < cmd_len_or_count; ++i) {
            void *new_node = calloc(1, 20); // Allocate space for a new node structure (20 bytes)
            if (new_node == NULL) {
                free(node_ids);
                DestroyNodes();
                DestroyEdges();
                _terminate(); // Critical error: memory allocation failed
            }
            // Assuming node ID is stored at offset 0xc and another field at 0x10
            *(unsigned int *)((char *)new_node + 0xc) = node_ids[i];
            *(unsigned int *)((char *)new_node + 0x10) = 0xffffffff;

            if (AddNode(new_node) == 0) { // Assuming AddNode returns 0 on failure
                free(node_ids);
                free(new_node); // Free the failed node structure
                DestroyNodes();
                DestroyEdges();
                _terminate(); // Critical error: AddNode failed
            }
        }
        free(node_ids); // Free the temporary node ID buffer
    } else if (cmd_type == 0x02) { // Command type 2: Add Edges
        if (2000 < NumEdges + cmd_len_or_count) { // Check for max edges
            ReadNull(cmd_len_or_count * 3 * sizeof(unsigned int)); // Consume remaining data for oversized request
            SendErrorResponse(cmd_type); // Placeholder error code
            return 0;
        }

        // Allocate buffer for edge data (source ID, destination ID, weight)
        unsigned int *edge_data = (unsigned int *)calloc(cmd_len_or_count, 3 * sizeof(unsigned int));
        if (edge_data == NULL) {
            DestroyNodes();
            DestroyEdges();
            _terminate(); // Critical error: memory allocation failed
        }

        // Read edge data into the buffer
        if (ReadBytes((char *)edge_data, cmd_len_or_count * 3 * sizeof(unsigned int)) != cmd_len_or_count * 3 * sizeof(unsigned int)) {
            free(edge_data);
            return 0;
        }

        for (unsigned int i = 0; i < cmd_len_or_count; ++i) {
            unsigned int src_node_id = edge_data[i * 3];
            unsigned int dest_node_id = edge_data[i * 3 + 1];
            unsigned int initial_edge_weight = edge_data[i * 3 + 2];

            void *new_edge = calloc(1, 20); // Allocate space for a new edge structure (20 bytes)
            if (new_edge == NULL) {
                free(edge_data);
                DestroyNodes();
                DestroyEdges();
                _terminate(); // Critical error: memory allocation failed
            }

            void *src_node_ptr = FindNode(src_node_id);
            if (src_node_ptr == NULL) {
                SendErrorResponse(4); // Error code 4 for node not found
                free(new_edge);
                free(edge_data);
                DestroyNodes();
                DestroyEdges();
                _terminate(); // Critical error: source node not found
            }
            // Assuming source node pointer is stored at offset 8
            *(void **)((char *)new_edge + 8) = src_node_ptr;

            void *dest_node_ptr = FindNode(dest_node_id);
            if (dest_node_ptr == NULL) {
                SendErrorResponse(4); // Error code 4 for node not found
                free(new_edge);
                free(edge_data);
                DestroyNodes();
                DestroyEdges();
                _terminate(); // Critical error: destination node not found
            }
            // Assuming destination node pointer is stored at offset 0xc
            *(void **)((char *)new_edge + 0xc) = dest_node_ptr;

            // Calculate final edge weight, adding a value from a memory location
            // Assuming rand_page_ptr points to a char array/page, and NumNodes_count is an offset.
            *(unsigned int *)((char *)new_edge + 0x10) = initial_edge_weight + (unsigned int)*(rand_page_ptr + NumNodes_count);

            void *existing_edge = FindEdge(src_node_ptr, dest_node_ptr);
            if (existing_edge == NULL) {
                if (AddEdge(new_edge) == 0) { // Assuming AddEdge returns 0 on failure
                    free(new_edge);
                    free(edge_data);
                    DestroyNodes();
                    DestroyEdges();
                    _terminate(); // Critical error: AddEdge failed
                }
            } else {
                // Edge already exists, update weight if new one is smaller
                // Assuming edge weight is stored at offset 0x10
                if (*(unsigned int *)((char *)new_edge + 0x10) < *(unsigned int *)((char *)existing_edge + 0x10)) {
                    *(unsigned int *)((char *)existing_edge + 0x10) = *(unsigned int *)((char *)new_edge + 0x10);
                }
                free(new_edge); // Free the temporary new_edge structure as it's not added
            }
        }
        free(edge_data); // Free the temporary edge data buffer
    } else if (cmd_type == 0x03) { // Command type 3: Find Shortest Path Tree (SPT)
        struct SptResult *spt_info = FindSpt();
        if (spt_info == NULL) {
            SendErrorResponse(cmd_type); // Placeholder error code
            return 0;
        }

        if (SendResponse(cmd_type, spt_info->length_in_4byte_units, (uintptr_t)spt_info->data) == 0) {
            free(spt_info->data); // Assuming data is separately allocated
            free(spt_info);       // Free the SptResult structure itself
            return 0;
        }
        free(spt_info->data); // Free SPT data
        free(spt_info);       // Free SptResult structure
    } else { // Unknown command type
        // No specific data to read for unknown command, just send error
        SendErrorResponse(cmd_type);
        return 0;
    }
    return 1; // Command processed successfully
}