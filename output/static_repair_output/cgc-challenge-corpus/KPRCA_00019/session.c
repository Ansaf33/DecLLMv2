#include <stddef.h> // For size_t
#include <stdlib.h> // For malloc, free
#include <string.h> // For memcpy, memset

// --- Type Definitions (mapping from decompiler output) ---
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

// Decompiler's 'undefined' types
typedef unsigned char undefined;
typedef unsigned short undefined2;
typedef unsigned int undefined4;

// Forward declarations for external functions
// Assuming link_send takes (type, length, data_ptr)
void link_send(undefined4 type, size_t length, const void *data);
// Assuming link_recv takes (type_out, length_out, buffer_out)
int link_recv(int *type_out, size_t *length_out, void *buffer_out);

// --- Global Variables (inferred from usage) ---

// For session_find_psm, session_register_psm: g_psm_head
typedef struct PsmEntry {
    struct PsmEntry *next;
    int psm_id;
    void (*callback)(void *); // Callback for PSM connection requests, receives Channel*
} PsmEntry;
static PsmEntry *g_psm_head = NULL;

// For session_new_channel, session_configuration_request, etc.: g_dynamic_id, g_channels
// A channel structure is 0x18 bytes.
typedef struct Channel {
    unsigned char status;     // Offset 0: 0=inactive, 1=connecting, 2=open, 3=disconnecting
    unsigned char config_status; // Offset 1: 0=no config pending, 1=config request sent
    unsigned short local_cid; // Offset 2
    unsigned short remote_cid; // Offset 4
    void (*callback)(struct Channel *, undefined4, undefined4); // Offset 8: Data/event callback
    undefined4 userdata;      // Offset 0xc: User-defined data
    unsigned int mtu;         // Offset 0x10: Maximum Transmission Unit
    unsigned int config_val2; // Offset 0x14: Flow control/retransmission mode, etc.
} Channel;
#define MAX_L2CAP_CHANNELS 256 // Assuming channel IDs are 1-255, 0 is special.
static Channel *g_channels[MAX_L2CAP_CHANNELS] = {NULL};
static ushort g_dynamic_id = 0x40; // Common dynamic CID range start (0x40 for L2CAP)

// For session_send_config: g_config_req_id
static unsigned char g_config_req_id = 0; // Counter for config requests identifier

// Global buffer for outgoing L2CAP packets (_session_send)
#define MAX_L2CAP_PAYLOAD_SIZE 0xFFFF // Max ushort value
#define L2CAP_HEADER_SIZE 4 // Length (2 bytes) + CID (2 bytes)
static unsigned char g_outgoing_l2cap_buffer[L2CAP_HEADER_SIZE + MAX_L2CAP_PAYLOAD_SIZE];

// Global buffer for incoming L2CAP packets (session_loop, session_handle_packet, session_handle_control)
#define MAX_LINK_RECV_FRAGMENT_SIZE 339 // From local_163 size in session_loop
#define MAX_L2CAP_PACKET_TOTAL_SIZE (L2CAP_HEADER_SIZE + MAX_L2CAP_PAYLOAD_SIZE)
static unsigned char g_incoming_l2cap_buffer[MAX_L2CAP_PACKET_TOTAL_SIZE];
static size_t g_incoming_l2cap_current_len = 0; // How much of the current L2CAP packet has been received

// Macros for accessing fields within the incoming L2CAP packet buffer
#define IN_L2CAP_LEN  (*(ushort*)&g_incoming_l2cap_buffer[0]) // L2CAP Payload Length
#define IN_L2CAP_CID  (*(ushort*)&g_incoming_l2cap_buffer[2]) // L2CAP Channel ID
#define IN_SDU_START  (&g_incoming_l2cap_buffer[4]) // Start of L2CAP Payload (SDU)

// Function prototypes (needed for mutual recursion or calls before definition)
void session_send_reject(undefined identifier, undefined2 reason);
void session_send_config(Channel *channel);

// --- Function Implementations ---

// Function: _session_send
// param_1: L2CAP Channel ID (CID)
// param_2: L2CAP Payload Length
// param_3: Pointer to L2CAP Payload data
void _session_send(undefined2 param_1, ushort param_2, void *param_3) {
    // Construct the L2CAP packet in the global buffer
    *(undefined2 *)&g_outgoing_l2cap_buffer[0] = param_1; // CID
    *(ushort *)&g_outgoing_l2cap_buffer[2] = param_2;     // Length
    memcpy(&g_outgoing_l2cap_buffer[4], param_3, param_2); // Payload

    size_t total_packet_len = param_2 + L2CAP_HEADER_SIZE;
    size_t current_offset = 0;

    while (current_offset < total_packet_len) {
        size_t fragment_len = total_packet_len - current_offset;
        if (fragment_len > MAX_LINK_RECV_FRAGMENT_SIZE) { // Max fragment size for link_send
            fragment_len = MAX_LINK_RECV_FRAGMENT_SIZE;
        }

        undefined4 send_type;
        if (current_offset == 0) {
            send_type = 2; // Start of packet
        } else {
            send_type = 1; // Continuation
        }
        link_send(send_type, fragment_len, &g_outgoing_l2cap_buffer[current_offset]);
        current_offset += fragment_len;
    }
}

// Function: session_find_psm
PsmEntry *session_find_psm(int psm_id) {
    PsmEntry *current = g_psm_head;
    while (current != NULL) {
        if (psm_id == current->psm_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function: session_new_channel
Channel *session_new_channel(ushort remote_cid) {
    ushort local_cid = remote_cid;
    if (remote_cid == 0) { // If remote_cid is 0, generate a dynamic local_cid
        local_cid = g_dynamic_id;
        // Find next available dynamic ID
        while (local_cid < MAX_L2CAP_CHANNELS && g_channels[local_cid] != NULL) {
            local_cid++;
        }
        if (local_cid >= MAX_L2CAP_CHANNELS) { // No available dynamic CID
            return NULL;
        }
        g_dynamic_id = local_cid + 1; // Update for next allocation
    } else { // Static CID, check if already in use
        if (local_cid >= MAX_L2CAP_CHANNELS || g_channels[local_cid] != NULL) {
            return NULL; // CID out of bounds or already in use
        }
    }

    Channel *new_channel = (Channel *)malloc(sizeof(Channel));
    if (new_channel == NULL) {
        return NULL;
    }

    memset(new_channel, 0, sizeof(Channel)); // Initialize to zero
    new_channel->status = 0;
    new_channel->local_cid = local_cid;
    new_channel->mtu = 0x2a0; // Default MTU
    new_channel->config_val2 = 0x2a0; // Default config value

    g_channels[local_cid] = new_channel;
    return new_channel;
}

// Function: session_send_reject
// identifier: Identifier from incoming SDU
// reason: Reason code
void session_send_reject(undefined identifier, undefined2 reason) {
    // SDU structure for Command Reject:
    // Code (1 byte) = 0x01
    // Identifier (1 byte)
    // Length (2 bytes) = 2
    // Reason (2 bytes)
    unsigned char reject_sdu[6];
    reject_sdu[0] = 1; // Code: Command Reject
    reject_sdu[1] = identifier; // Identifier from original command
    *(ushort *)&reject_sdu[2] = 2; // Length: 2 bytes for reason
    *(undefined2 *)&reject_sdu[4] = reason; // Reason code

    _session_send(1, sizeof(reject_sdu), reject_sdu); // CID 1 for signaling
}

// Function: session_send_config
void session_send_config(Channel *channel) {
    // SDU structure for Configuration Request:
    // Code (1 byte) = 0x04
    // Identifier (1 byte)
    // Length (2 bytes)
    // Local CID (2 bytes)
    // Remote CID (2 bytes)
    // Config Options (variable)
    unsigned char config_req_sdu[8];
    config_req_sdu[0] = 4; // Code: Configuration Request
    config_req_sdu[1] = g_config_req_id++; // Identifier
    *(ushort *)&config_req_sdu[2] = 4; // Length of parameters (Local CID + Remote CID)
    *(ushort *)&config_req_sdu[4] = channel->local_cid; // Local CID
    *(ushort *)&config_req_sdu[6] = 0; // Remote CID (set to 0 for initial request)

    _session_send(1, sizeof(config_req_sdu), config_req_sdu); // CID 1 for signaling
    channel->config_status = 1; // Mark channel as waiting for config response
}

// Function: session_connection_request
// sdu_data: Pointer to incoming SDU (Code, ID, Len, Params...)
void session_connection_request(unsigned char *sdu_data) {
    // SDU structure for Connection Request:
    // Code (1 byte) = 0x02
    // Identifier (1 byte)
    // Length (2 bytes) = 4
    // PSM (2 bytes)
    // Source CID (2 bytes)
    unsigned char identifier = sdu_data[1];
    ushort sdu_len = *(ushort *)&sdu_data[2];
    ushort psm = *(ushort *)&sdu_data[4];
    ushort remote_cid = *(ushort *)&sdu_data[6];

    if (sdu_len != 4) { // Expected length for PSM + Source CID
        session_send_reject(identifier, 0); // Invalid length
        return;
    }

    // SDU structure for Connection Response:
    // Code (1 byte) = 0x03
    // Identifier (1 byte)
    // Length (2 bytes) = 8
    // Destination CID (2 bytes)
    // Source CID (2 bytes)
    // Result (2 bytes)
    // Status (2 bytes)
    unsigned char conn_resp_sdu[12];
    conn_resp_sdu[0] = 3; // Code: Connection Response
    conn_resp_sdu[1] = identifier; // Identifier
    *(ushort *)&conn_resp_sdu[2] = 8; // Length of parameters

    PsmEntry *psm_entry = session_find_psm(psm);
    if (psm_entry == NULL) {
        // PSM not found
        *(ushort *)&conn_resp_sdu[4] = 0; // Local CID (Destination CID)
        *(ushort *)&conn_resp_sdu[6] = remote_cid; // Remote CID (Source CID)
        *(ushort *)&conn_resp_sdu[8] = 0x0002; // Result: PSM not supported
        *(ushort *)&conn_resp_sdu[10] = 0; // Status: No further information
    } else {
        // PSM found, create a new channel
        Channel *new_channel = session_new_channel(0); // Generate dynamic local CID
        if (new_channel == NULL) {
            // Out of resources
            session_send_reject(identifier, 0); // Can't create channel
            return;
        }

        new_channel->remote_cid = remote_cid; // Store remote CID
        new_channel->status = 1; // Set channel status to connecting (waiting for config)
        new_channel->config_status = 0; // Not yet configured

        // Call PSM callback if registered
        if (psm_entry->callback != NULL) {
            psm_entry->callback(new_channel);
        }

        *(ushort *)&conn_resp_sdu[4] = new_channel->local_cid; // Local CID (Destination CID)
        *(ushort *)&conn_resp_sdu[6] = remote_cid; // Remote CID (Source CID)
        *(ushort *)&conn_resp_sdu[8] = 0x0000; // Result: Connection successful
        *(ushort *)&conn_resp_sdu[10] = 0; // Status: No further information
    }

    _session_send(1, sizeof(conn_resp_sdu), conn_resp_sdu); // CID 1 for signaling
}

// Function: session_configuration_request
// sdu_data: Pointer to incoming SDU (Code, ID, Len, Params...)
void session_configuration_request(unsigned char *sdu_data) {
    // SDU structure for Configuration Request:
    // Code (1 byte) = 0x04
    // Identifier (1 byte)
    // Length (2 bytes)
    // Destination CID (2 bytes)
    // Flags (2 bytes)
    // Config Options (variable)
    unsigned char identifier = sdu_data[1];
    ushort sdu_len = *(ushort *)&sdu_data[2];
    ushort local_cid = *(ushort *)&sdu_data[4];
    ushort flags = *(ushort *)&sdu_data[6];
    unsigned char *options_start = &sdu_data[8];

    if (sdu_len < 4) { // Minimum length for Dest CID + Flags
        session_send_reject(identifier, 0);
        return;
    }

    Channel *channel = (local_cid < MAX_L2CAP_CHANNELS) ? g_channels[local_cid] : NULL;
    if (channel == NULL || channel->status == 0) { // Channel not found or not active
        session_send_reject(identifier, 2); // Invalid CID
        return;
    }
    if (channel->status == 1 && channel->config_status != 0) { // Channel in connecting state, already sent config req
        // This might be a re-transmission or a race condition.
        // For simplicity, reject for now or handle more gracefully.
        session_send_reject(identifier, 2); // Invalid CID (or state)
        return;
    }

    // Allocate buffer for Configuration Response SDU
    // Response SDU: Code, ID, Len, Dest CID, Flags, Result, Config Options
    // Minimum 10 bytes: Code, ID, Len=6, Dest CID, Flags, Result
    // Max options len is sdu_len - 4.
    unsigned char *config_resp_sdu = (unsigned char *)malloc(L2CAP_HEADER_SIZE + sdu_len + 2); // +2 for Result field
    if (config_resp_sdu == NULL) {
        session_send_reject(identifier, 0); // Out of memory
        return;
    }

    config_resp_sdu[0] = 5; // Code: Configuration Response
    config_resp_sdu[1] = identifier; // Identifier
    *(ushort *)&config_resp_sdu[4] = local_cid; // Destination CID
    *(ushort *)&config_resp_sdu[6] = flags; // Flags (echo back)

    ushort response_result = 0; // 0 = Success
    ushort response_options_len = 0;
    unsigned char *resp_options_ptr = &config_resp_sdu[10]; // Start of options in response

    size_t current_option_offset = 0;
    while (current_option_offset < sdu_len - 4) { // Iterate through config options
        byte type = options_start[current_option_offset];
        byte len = options_start[current_option_offset + 1];
        unsigned char *value_ptr = &options_start[current_option_offset + 2];

        // Ensure option length doesn't exceed available SDU data
        if (current_option_offset + 2 + len > sdu_len - 4) {
             response_result = 2; // Invalid parameters (option length exceeds SDU)
             break;
        }

        if ((type & 0x7F) == 1) { // MTU option
            if (len == 2) {
                ushort mtu_val = *(ushort *)value_ptr;
                if (mtu_val < 0x30) { // Reject MTU < 48
                    response_result = 1; // Unacceptable parameters
                    // Add MTU option with min value 0x30 to response
                    resp_options_ptr[response_options_len++] = type;
                    resp_options_ptr[response_options_len++] = 2;
                    *(ushort *)&resp_options_ptr[response_options_len] = 0x30;
                    response_options_len += 2;
                } else {
                    channel->mtu = mtu_val;
                    // Echo back accepted MTU
                    resp_options_ptr[response_options_len++] = type;
                    resp_options_ptr[response_options_len++] = 2;
                    *(ushort *)&resp_options_ptr[response_options_len] = mtu_val;
                    response_options_len += 2;
                }
            } else { // Invalid length for MTU option
                response_result = 2; // Invalid parameters
                break;
            }
        } else if ((char)type < 0) { // Hint bit set (unknown option, but acceptable)
            // Copy the unknown option to the response
            resp_options_ptr[response_options_len++] = type;
            resp_options_ptr[response_options_len++] = len;
            memcpy(&resp_options_ptr[response_options_len], value_ptr, len);
            response_options_len += len;
        } else { // Unknown mandatory option
            response_result = 3; // Unknown option
            // Copy the unknown option to the response with a "reject" bit
            resp_options_ptr[response_options_len++] = type | 0x80; // Set hint bit
            resp_options_ptr[response_options_len++] = len;
            memcpy(&resp_options_ptr[response_options_len], value_ptr, len);
            response_options_len += len;
        }
        current_option_offset += len + 2;
    }

    *(ushort *)&config_resp_sdu[8] = response_result; // Result
    *(ushort *)&config_resp_sdu[2] = 6 + response_options_len; // Total Length of parameters

    _session_send(1, *(ushort *)&config_resp_sdu[2] + L2CAP_HEADER_SIZE, config_resp_sdu);
    free(config_resp_sdu);

    // If configuration was successful and no renegotiation is pending, and channel status is not already open
    if (response_result == 0 && (flags & 1) == 0 && channel->status != 2) {
        session_send_config(channel); // Send own config request to peer
    }
}

// Function: session_configuration_response
// sdu_data: Pointer to incoming SDU (Code, ID, Len, Params...)
void session_configuration_response(unsigned char *sdu_data) {
    // SDU structure for Configuration Response:
    // Code (1 byte) = 0x05
    // Identifier (1 byte)
    // Length (2 bytes)
    // Destination CID (2 bytes)
    // Flags (2 bytes)
    // Result (2 bytes)
    // Config Options (variable)
    ushort local_cid = *(ushort *)&sdu_data[4];
    ushort result = *(ushort *)&sdu_data[8];

    Channel *channel = (local_cid < MAX_L2CAP_CHANNELS) ? g_channels[local_cid] : NULL;
    if (channel != NULL && channel->status == 1 && channel->config_status == 1) {
        if (result == 0) { // Success
            channel->status = 2; // Channel is now open
        }
        channel->config_status = 0; // Clear config request pending
    } else {
        session_send_reject(sdu_data[1], 2); // Invalid CID or state
    }
}

// Function: session_disconnection_request
// sdu_data: Pointer to incoming SDU (Code, ID, Len, Params...)
void session_disconnection_request(unsigned char *sdu_data) {
    // SDU structure for Disconnection Request:
    // Code (1 byte) = 0x06
    // Identifier (1 byte)
    // Length (2 bytes) = 4
    // Destination CID (2 bytes)
    // Source CID (2 bytes)
    unsigned char identifier = sdu_data[1];
    ushort sdu_len = *(ushort *)&sdu_data[2];
    ushort local_cid = *(ushort *)&sdu_data[4];
    ushort remote_cid = *(ushort *)&sdu_data[6];

    if (sdu_len != 4) {
        session_send_reject(identifier, 0); // Invalid length
        return;
    }

    Channel *channel = (local_cid < MAX_L2CAP_CHANNELS) ? g_channels[local_cid] : NULL;
    if (channel == NULL || channel->status == 0) {
        session_send_reject(identifier, 2); // Invalid CID
        return;
    }

    // SDU structure for Disconnection Response:
    // Code (1 byte) = 0x07
    // Identifier (1 byte)
    // Length (2 bytes) = 4
    // Destination CID (2 bytes)
    // Source CID (2 bytes)
    unsigned char disc_resp_sdu[8];
    disc_resp_sdu[0] = 7; // Code: Disconnection Response
    disc_resp_sdu[1] = identifier; // Identifier
    *(ushort *)&disc_resp_sdu[2] = 4; // Length
    *(ushort *)&disc_resp_sdu[4] = local_cid; // Destination CID
    *(ushort *)&disc_resp_sdu[6] = remote_cid; // Source CID

    if (channel->callback != NULL) {
        // Call callback with channel, userdata, and a reason for disconnection
        undefined4 reason_code = 1; // Generic reason
        channel->callback(channel, channel->userdata, reason_code);
    }

    _session_send(1, sizeof(disc_resp_sdu), disc_resp_sdu); // CID 1 for signaling
    channel->status = 0; // Mark channel as inactive/closed
}

// Function: session_disconnection_response
// sdu_data: Pointer to incoming SDU (Code, ID, Len, Params...)
void session_disconnection_response(unsigned char *sdu_data) {
    // SDU structure for Disconnection Response:
    // Code (1 byte) = 0x07
    // Identifier (1 byte)
    // Length (2 bytes) = 4
    // Destination CID (2 bytes)
    // Source CID (2 bytes)
    unsigned char identifier = sdu_data[1];
    ushort sdu_len = *(ushort *)&sdu_data[2];
    ushort local_cid = *(ushort *)&sdu_data[4];
    // ushort remote_cid = *(ushort *)&sdu_data[6]; // Not used in this function

    if (sdu_len != 4) {
        session_send_reject(identifier, 0); // Invalid length
        return;
    }

    Channel *channel = (local_cid < MAX_L2CAP_CHANNELS) ? g_channels[local_cid] : NULL;
    if (channel == NULL || channel->status != 3) { // Expecting status 3 (disconnecting)
        session_send_reject(identifier, 2); // Invalid CID or state
        return;
    }

    channel->status = 0; // Mark channel as inactive/closed
}

// Function: session_echo_request
// sdu_data: Pointer to incoming SDU (Code, ID, Len, Params...)
void session_echo_request(unsigned char *sdu_data) {
    // SDU structure for Echo Request:
    // Code (1 byte) = 0x08
    // Identifier (1 byte)
    // Length (2 bytes)
    // Data (variable)
    // Echo Response has Code = 0x09, same ID, Length, Data
    sdu_data[0] = 9; // Change code to Echo Response
    ushort sdu_len = *(ushort *)&sdu_data[2];
    _session_send(1, sdu_len + L2CAP_HEADER_SIZE, sdu_data); // CID 1 for signaling
}

// Function: session_information_request
// sdu_data: Pointer to incoming SDU (Code, ID, Len, Params...)
void session_information_request(unsigned char *sdu_data) {
    // SDU structure for Information Request:
    // Code (1 byte) = 0x0A
    // Identifier (1 byte)
    // Length (2 bytes) = 2
    // InfoType (2 bytes)
    unsigned char identifier = sdu_data[1];
    ushort sdu_len = *(ushort *)&sdu_data[2];
    // ushort info_type = *(ushort *)&sdu_data[4]; // Not directly used in response content

    if (sdu_len != 2) {
        session_send_reject(identifier, 0); // Invalid length
        return;
    }

    // SDU structure for Information Response:
    // Code (1 byte) = 0x0B
    // Identifier (1 byte)
    // Length (2 bytes) = 4 (for InfoType + Result)
    // InfoType (2 bytes)
    // Result (2 bytes)
    unsigned char info_resp_sdu[8];
    info_resp_sdu[0] = 0x0B; // Code: Information Response
    info_resp_sdu[1] = identifier; // Identifier
    *(ushort *)&info_resp_sdu[2] = 4; // Length (InfoType + Result)
    *(ushort *)&info_resp_sdu[4] = *(ushort *)&sdu_data[4]; // InfoType (echo back)
    *(ushort *)&info_resp_sdu[6] = 1; // Result: Not supported (0x0001)

    _session_send(1, sizeof(info_resp_sdu), info_resp_sdu); // CID 1 for signaling
}

// Function: session_handle_command
// sdu_data: Pointer to incoming SDU (Code, ID, Len, Params...)
void session_handle_command(unsigned char *sdu_data) {
    switch (sdu_data[0]) { // SDU Code
        case 1: // Command Reject
        case 3: // Connection Response
        case 9: // Echo Response
        case 0xB: // Information Response
            // These are responses to commands we sent, or errors.
            // For simplicity, just ignore them here. A real stack would parse them.
            break;
        case 2: // Connection Request
            session_connection_request(sdu_data);
            break;
        case 4: // Configuration Request
            session_configuration_request(sdu_data);
            break;
        case 5: // Configuration Response
            session_configuration_response(sdu_data);
            break;
        case 6: // Disconnection Request
            session_disconnection_request(sdu_data);
            break;
        case 7: // Disconnection Response
            session_disconnection_response(sdu_data);
            break;
        case 8: // Echo Request
            session_echo_request(sdu_data);
            break;
        case 10: // Information Request
            session_information_request(sdu_data);
            break;
        default:
            session_send_reject(sdu_data[1], 0); // Command not understood (Reason 0: Command not understood)
            break;
    }
}

// Function: session_handle_control
// Handles L2CAP Signaling channel (CID 1) commands
undefined4 session_handle_control(void) {
    size_t current_sdu_offset = 0;
    // Iterate through multiple SDUs if present in the L2CAP payload
    while (current_sdu_offset < IN_L2CAP_LEN) {
        // Each SDU has Code (1), Identifier (1), Length (2), Parameters (Length bytes)
        unsigned char *sdu_ptr = IN_SDU_START + current_sdu_offset;
        // Check if there's enough data for SDU header (Code, ID, Len)
        if (current_sdu_offset + 4 > IN_L2CAP_LEN) {
            // Malformed SDU, not enough data for header
            return 0; // Error
        }
        ushort sdu_payload_len = *(ushort *)&sdu_ptr[2];
        // Check if there's enough data for the full SDU payload
        if (current_sdu_offset + 4 + sdu_payload_len > IN_L2CAP_LEN) {
            // Malformed SDU, payload length exceeds L2CAP payload
            return 0; // Error
        }

        session_handle_command(sdu_ptr);
        current_sdu_offset += (4 + sdu_payload_len);
    }
    return 0; // Success
}

// Function: session_handle_packet
// Handles a complete incoming L2CAP packet
undefined4 session_handle_packet(void) {
    // IN_L2CAP_CID refers to the channel ID received in the L2CAP header
    if (IN_L2CAP_CID == 1) { // Signaling channel
        return session_handle_control();
    } else { // Data channel
        Channel *channel = (IN_L2CAP_CID < MAX_L2CAP_CHANNELS) ? g_channels[IN_L2CAP_CID] : NULL;
        if (channel == NULL || channel->status != 2) { // Channel not found or not open
            return 0; // Drop packet
        }

        if (channel->callback != NULL) {
            // Call data callback if registered
            // Assuming callback takes channel, userdata, and data buffer/length
            channel->callback(channel, channel->userdata, (undefined4)IN_SDU_START); // Pass pointer to SDU
        }
        return 0; // Success
    }
}

// Function: session_loop
// Main loop for receiving and processing L2CAP packets
void session_loop(void) {
    int link_packet_type; // 2 for start, 1 for continuation
    size_t link_packet_len; // Length of current fragment
    unsigned char link_packet_buffer[MAX_LINK_RECV_FRAGMENT_SIZE]; // Buffer for received fragment

    while (1) {
        if (link_recv(&link_packet_type, &link_packet_len, link_packet_buffer) != 0) {
            // Error or no data, return from loop
            return;
        }

        if (link_packet_type == 2) { // Start of a new L2CAP packet
            g_incoming_l2cap_current_len = 0;
            // memset(g_incoming_l2cap_buffer, 0, MAX_L2CAP_PACKET_TOTAL_SIZE); // Not strictly necessary if overwritten
        } else if (link_packet_type != 1) { // Unknown type, discard current packet state
            g_incoming_l2cap_current_len = 0; // Reset state
            continue; // Go to next link_recv
        }

        // Check for buffer overflow before copying
        if (g_incoming_l2cap_current_len + link_packet_len > MAX_L2CAP_PACKET_TOTAL_SIZE) {
            g_incoming_l2cap_current_len = 0; // Discard oversized packet
            continue; // Go to next link_recv
        }

        // Copy received fragment into the global L2CAP buffer
        memcpy(&g_incoming_l2cap_buffer[g_incoming_l2cap_current_len], link_packet_buffer, link_packet_len);
        g_incoming_l2cap_current_len += link_packet_len;

        // Check if L2CAP header is complete (at least 4 bytes)
        if (g_incoming_l2cap_current_len >= L2CAP_HEADER_SIZE) {
            // Check if the announced L2CAP payload length is too large for our buffer
            if (IN_L2CAP_LEN + L2CAP_HEADER_SIZE > MAX_L2CAP_PACKET_TOTAL_SIZE) {
                g_incoming_l2cap_current_len = 0; // Discard malformed/oversized packet
                continue; // Go to next link_recv
            }

            // Check if the full L2CAP packet has been received
            // Total expected length is L2CAP_Header_Size (4) + L2CAP_Payload_Length
            if (IN_L2CAP_LEN + L2CAP_HEADER_SIZE == g_incoming_l2cap_current_len) {
                // Full L2CAP packet received
                session_handle_packet();
                g_incoming_l2cap_current_len = 0; // Reset for next packet
            }
        }
    }
}

// Function: session_register_psm
void session_register_psm(int psm_id, void (*callback)(void *)) {
    PsmEntry *existing_psm = session_find_psm(psm_id);
    if (existing_psm == NULL) {
        PsmEntry *new_psm = (PsmEntry *)malloc(sizeof(PsmEntry));
        if (new_psm == NULL) {
            return; // Out of memory
        }
        new_psm->psm_id = psm_id;
        new_psm->next = g_psm_head;
        g_psm_head = new_psm;
        existing_psm = new_psm;
    }
    existing_psm->callback = callback; // Update or set callback
}

// Function: session_register_events
void session_register_events(Channel *channel, void (*callback)(Channel *, undefined4, undefined4)) {
    if (channel != NULL) {
        channel->callback = callback;
    }
}

// Function: session_register_userdata
void session_register_userdata(Channel *channel, undefined4 userdata) {
    if (channel != NULL) {
        channel->userdata = userdata;
    }
}

// Function: session_send
// channel: Pointer to the Channel structure
// payload_len: L2CAP Payload Length
// payload_data_ptr: Pointer to L2CAP Payload data
void session_send(Channel *channel, undefined2 payload_len, undefined4 payload_data_ptr) {
    if (channel == NULL || channel->status != 2) {
        // Channel not open or invalid
        return;
    }
    _session_send(channel->local_cid, payload_len, (void *)payload_data_ptr);
}