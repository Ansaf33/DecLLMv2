#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Type definitions for clarity and Linux compatibility
typedef uint8_t byte;
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;
typedef uint16_t ushort;

// External functions (mocked for compilation, actual implementation would be linked)
// These functions are assumed to exist in the environment this code will run in.
// Their parameters are derived from the usage in the provided snippet.
extern int link_send(undefined4 type, undefined4 length, void *data);
extern int link_recv(int *type, size_t *length, void *buffer);

// Structure for PSM (Protocol/Service Multiplexer) entries
// Based on usage in session_find_psm and session_register_psm
typedef struct psm_entry {
    struct psm_entry *next;        // 0x0: Pointer to the next PSM entry in the linked list
    undefined4 psm_id;             // 0x4: The PSM ID
    undefined4 callback_fn;        // 0x8: Function pointer for PSM events (e.g., connection request)
} psm_entry_t;

// Structure for Channel entries
// Based on usage in session_new_channel and other session_* functions
typedef struct channel_entry {
    undefined state;                // 0x0: Channel state (e.g., 0=disconnected, 1=connected, 2=configured)
    undefined unknown1;             // 0x1: An additional flag or sub-state
    ushort local_cid;               // 0x2: Local Channel ID
    ushort remote_cid;              // 0x4: Remote Channel ID
    undefined4 psm_id;              // 0x6: PSM ID associated with this channel (guessed, not explicitly set in snippet)
    undefined4 callback_fn;         // 0x8: Function pointer for channel events (e.g., data reception, disconnection)
    undefined4 user_data;           // 0xc: User-defined data associated with the channel
    undefined4 local_mtu;           // 0x10: Local MTU (Maximum Transmission Unit)
    undefined4 remote_mtu;          // 0x14: Remote MTU
} channel_entry_t; // Total size 0x18 (24 bytes)

// Global variables
static psm_entry_t *g_psm_head = NULL;
static int g_dynamic_id = 0x100; // Starting value for dynamic channel IDs
static channel_entry_t *g_channels[256] = {0}; // Array of pointers to channel entries, max 256 channels
static undefined g_config_req_id = 0; // ID for configuration requests

// Global buffer for sending L2CAP packets
// Max chunk size 0x153 + 4 bytes L2CAP header. Assuming total max packet fits.
static uint8_t g_send_pkt_buffer[0x1000];
// Macros to access specific fields within the send buffer
#define G_SEND_PKT_LEN (*(uint16_t*)g_send_pkt_buffer) // L2CAP payload length
#define G_SEND_PKT_CMD_OR_CID (*(uint16_t*)(g_send_pkt_buffer + 2)) // L2CAP Command Code or Channel ID
#define G_SEND_PKT_DATA (g_send_pkt_buffer + 4) // Start of L2CAP payload data

// Global buffer for receiving L2CAP packets
// Size 0x10004 based on `memset(&g_current_packet,0,0x10003)` and `local_10 < 0x10004` in session_loop
static uint8_t g_recv_pkt_buffer[0x10004];
// Macros to access specific fields within the receive buffer
#define G_RECV_TOTAL_L2CAP_PAYLOAD_LEN (*(uint16_t*)g_recv_pkt_buffer) // L2CAP payload length from header
#define G_RECV_L2CAP_CID_OR_TYPE (*(uint16_t*)(g_recv_pkt_buffer + 2)) // L2CAP Channel ID or Command Type
#define G_RECV_L2CAP_PAYLOAD_START (g_recv_pkt_buffer + 4) // Start of L2CAP payload (contains commands or data)

// Function: _session_send
// Sends an L2CAP packet in chunks.
void _session_send(undefined2 type_or_cid, ushort payload_len, void *data) {
    G_SEND_PKT_LEN = payload_len;
    G_SEND_PKT_CMD_OR_CID = type_or_cid;
    memcpy(G_SEND_PKT_DATA, data, payload_len);

    size_t total_packet_len = payload_len + 4; // L2CAP payload + 4 bytes L2CAP header
    for (size_t sent_bytes = 0; sent_bytes < total_packet_len; ) {
        size_t chunk_len = total_packet_len - sent_bytes;
        if (0x153 < chunk_len) { // Max chunk size (0x153 bytes)
            chunk_len = 0x153;
        }
        // Packet type: 2 for first chunk, 1 for subsequent chunks
        uint32_t link_pkt_type = (sent_bytes == 0) ? 2 : 1;
        link_send(link_pkt_type, chunk_len, g_send_pkt_buffer + sent_bytes);
        sent_bytes += chunk_len;
    }
}

// Function: session_find_psm
// Finds a PSM entry in the g_psm_head linked list.
psm_entry_t *session_find_psm(int psm_id) {
    psm_entry_t *current_psm = g_psm_head;
    while (current_psm != NULL) {
        if (psm_id == current_psm->psm_id) {
            break;
        }
        current_psm = current_psm->next;
    }
    return current_psm;
}

// Function: session_new_channel
// Allocates and initializes a new channel entry.
channel_entry_t *session_new_channel(ushort param_1_cid) {
    ushort channel_id = param_1_cid;
    if (channel_id == 0) { // If param_1_cid is 0, assign a dynamic ID
        channel_id = (ushort)g_dynamic_id++;
    }

    channel_entry_t *new_channel = (channel_entry_t *)malloc(sizeof(channel_entry_t));
    if (new_channel == NULL) {
        return NULL;
    }

    memset(new_channel, 0, sizeof(channel_entry_t)); // Initialize all fields to 0
    new_channel->state = 0;
    new_channel->local_cid = channel_id;
    new_channel->local_mtu = 0x2a0; // Default MTU
    new_channel->remote_mtu = 0x2a0; // Default MTU

    if (channel_id < sizeof(g_channels) / sizeof(g_channels[0])) { // Check for array bounds
        g_channels[channel_id] = new_channel;
    } else {
        free(new_channel); // Channel ID out of bounds, free memory
        return NULL;
    }
    return new_channel;
}

// Function: session_send_reject
// Sends an L2CAP Command Reject response.
void session_send_reject(undefined packet_id, undefined2 result_code) {
    // L2CAP Command Reject packet structure:
    // Code (1) | ID (1) | Length (2) | Reason (2) | Data (variable, 0-2)
    // Here, it seems to be Code (1) | ID (1) | Length (2) | Reason (2) | Result (2)
    struct {
        undefined code;
        undefined id;
        undefined2 length;
        undefined2 reason;
        undefined2 result_data; // Not always used, but implied by original length
    } reject_pkt;

    reject_pkt.code = 1; // Command Reject
    reject_pkt.id = packet_id;
    reject_pkt.length = 2; // Length of the reason field
    reject_pkt.reason = result_code; // The actual reject reason (e.g., 0=command not understood)
    // The original code implies a 6-byte payload (code,id,len,reason,result_data)
    // So if result_data is used, length should be 4.
    // Given `_session_send(1,6,&local_12)`, it implies a total payload of 6 bytes.
    // If reject_pkt.length is 2, then total payload is 1 (code) + 1 (id) + 2 (length) + 2 (reason) = 6.
    // Let's explicitly set the length to 2 for just the reason code.
    // If the original `local_10` was for `length` and `local_e` was for `result`,
    // then it's `length = 2` and `result = param_2`.
    // The struct has `length` and `result_data`. Let's use `result_code` for `result_data`.
    reject_pkt.length = 2; // Length of the `result_code` field
    reject_pkt.result_data = result_code; // Assuming `param_2` is the reason code

    // Correcting interpretation of original code: local_12=1(code), local_11=param_1(id), local_10=2(length), local_e=param_2(data)
    // So the payload is [1, param_1, 2, param_2] (6 bytes total)
    // This implies `reject_pkt.code = 1; reject_pkt.id = packet_id; reject_pkt.length = 2; reject_pkt.result_data = result_code;`
    // And `_session_send(1, 6, &reject_pkt.code)`
    // To match the original `_session_send(1,6,&local_12)`, `local_12` being the first byte of the payload.
    // So, `reject_pkt` is correct, and the _session_send call uses `sizeof(reject_pkt)`.
    _session_send(1, sizeof(reject_pkt), &reject_pkt);
}

// Function: session_send_config
// Sends an L2CAP Configuration Request or Response.
void session_send_config(channel_entry_t *channel) {
    // L2CAP Configuration Request/Response packet structure:
    // Code (1) | ID (1) | Length (2) | Local_CID (2) | Options (variable)
    // The snippet sends 8 bytes total, which implies:
    // Code (1) | ID (1) | Length (2) | Local_CID (2) | Flags/Result (2)
    struct {
        undefined code;
        undefined id;
        undefined2 length;
        undefined2 remote_cid; // Corresponds to *(param_1 + 4)
        undefined2 flags; // Corresponds to local_e = 0
    } config_pkt;

    config_pkt.code = 4; // Configuration Request
    config_pkt.id = g_config_req_id; // Use global ID
    config_pkt.length = 4; // Length of CID + Flags
    config_pkt.remote_cid = channel->remote_cid; // Use remote CID for the request
    config_pkt.flags = 0; // Default flags

    _session_send(1, sizeof(config_pkt), &config_pkt);
    channel->unknown1 = 1; // Mark channel as having sent a config request
}

// Function: session_connection_request
// Handles an incoming L2CAP Connection Request.
void session_connection_request(const undefined *param_1_cmd_data) {
    // Incoming L2CAP Connection Request packet structure (from param_1_cmd_data):
    // Code (0) | ID (1) | Length (2,3) | PSM (4,5) | Remote_CID (6,7)
    const uint16_t payload_len = *(const uint16_t *)(param_1_cmd_data + 2);
    const uint16_t psm_id = *(const uint16_t *)(param_1_cmd_data + 4);
    const uint16_t remote_cid_in_pkt = *(const uint16_t *)(param_1_cmd_data + 6);
    const undefined packet_id = param_1_cmd_data[1];

    if (payload_len == 4) { // Expected payload length for PSM + Remote_CID
        psm_entry_t *psm_entry = session_find_psm(psm_id);

        struct {
            undefined code;
            undefined id;
            undefined2 length;
            undefined2 local_cid;
            undefined2 remote_cid;
            undefined2 result;
            undefined2 status; // Often 0 for success
        } conn_resp_pkt;

        conn_resp_pkt.code = 3; // Connection Response
        conn_resp_pkt.id = packet_id;
        conn_resp_pkt.length = 8; // Length of data fields (local_cid, remote_cid, result, status)
        conn_resp_pkt.status = 0; // Default status (success)

        if (psm_entry == NULL) {
            // PSM not found, reject connection
            conn_resp_pkt.local_cid = 0;
            conn_resp_pkt.remote_cid = remote_cid_in_pkt;
            conn_resp_pkt.result = 2; // Result: PSM not supported
        } else {
            channel_entry_t *new_channel = session_new_channel(0); // Get a dynamic channel ID
            if (new_channel == NULL) {
                session_send_reject(packet_id, 0); // Out of memory
                return;
            }
            new_channel->remote_cid = remote_cid_in_pkt;
            new_channel->state = 1; // Mark channel as connected
            new_channel->unknown1 = 0; // Reset any flags

            if (psm_entry->callback_fn != 0) {
                // Call the PSM's connection callback
                // Assuming callback_fn is of type void (*)(channel_entry_t*)
                ((void (*)(channel_entry_t*))psm_entry->callback_fn)(new_channel);
            }
            conn_resp_pkt.local_cid = new_channel->local_cid;
            conn_resp_pkt.remote_cid = new_channel->remote_cid;
            conn_resp_pkt.result = 0; // Result: success
        }
        _session_send(1, sizeof(conn_resp_pkt), &conn_resp_pkt);
    } else {
        session_send_reject(packet_id, 0); // Invalid length for Connection Request
    }
}

// Function: session_configuration_request
// Handles an incoming L2CAP Configuration Request.
void session_configuration_request(const undefined *param_1_cmd_data) {
    // Incoming L2CAP Configuration Request packet structure (from param_1_cmd_data):
    // Code (0) | ID (1) | Length (2,3) | Local_CID (4,5) | Flags (6,7) | Options (...)
    const uint16_t payload_len = *(const uint16_t *)(param_1_cmd_data + 2);
    const uint16_t local_cid_in_pkt = *(const uint16_t *)(param_1_cmd_data + 4);
    const undefined packet_id = param_1_cmd_data[1];

    if (payload_len < 4) { // Minimum payload: Local_CID (2) + Flags (2)
        session_send_reject(packet_id, 0); // Invalid length
        return;
    }

    channel_entry_t *channel = NULL;
    if (local_cid_in_pkt < sizeof(g_channels) / sizeof(g_channels[0])) {
        channel = g_channels[local_cid_in_pkt];
    }

    if (channel == NULL || channel->state == 0) {
        session_send_reject(packet_id, 2); // Channel not found or disconnected
        return;
    }
    if (channel->state == 1 && channel->unknown1 != 0) {
        session_send_reject(packet_id, 2); // Already in config process or mis-state
        return;
    }

    // Allocate response packet buffer (header + options from request)
    undefined *response_pkt_buffer = (undefined *)malloc(payload_len + 10); // 10 bytes for config response header
    if (response_pkt_buffer == NULL) {
        session_send_reject(packet_id, 0); // Out of memory
        return;
    }

    // Initialize response packet header
    response_pkt_buffer[0] = 5; // Configuration Response
    response_pkt_buffer[1] = packet_id;
    // Length (bytes 2,3) will be filled later
    *(uint16_t*)(response_pkt_buffer + 4) = channel->remote_cid; // Remote CID
    response_pkt_buffer[6] = *(byte *)(param_1_cmd_data + 6) & 1; // Copy flags from request
    response_pkt_buffer[7] = 0; // Reserved
    *(uint16_t*)(response_pkt_buffer + 8) = 0; // Result code (0=success)

    // Process configuration options from the incoming packet
    // Options start at param_1_cmd_data + 8
    size_t response_option_offset = 0; // Offset for options in the response buffer (after 10 bytes header)
    size_t options_data_len = payload_len - 4; // Total length of options data (excluding Local_CID and Flags)

    for (size_t option_offset = 0; option_offset < options_data_len; ) {
        byte option_type = *(byte *)(param_1_cmd_data + 8 + option_offset);
        byte option_len = *(byte *)(param_1_cmd_data + 9 + option_offset);

        // Check if option_len would exceed remaining options data
        if (option_offset + option_len + 2 > options_data_len) {
            *(uint16_t*)(response_pkt_buffer + 8) = 2; // Result: Bad parameter
            break; // Malformed packet
        }

        if ((option_type & 0x7f) == 1) { // Option type 1: MTU
            if (option_len == 2) {
                ushort mtu_val = *(ushort *)(param_1_cmd_data + 10 + option_offset);
                if (mtu_val < 0x30) { // MTU too small
                    *(uint16_t*)(response_pkt_buffer + 8) = 1; // Result: Unacceptable parameter
                    // Add option to response indicating unacceptable MTU
                    response_pkt_buffer[response_option_offset + 10] = option_type;
                    response_pkt_buffer[response_option_offset + 11] = 2;
                    *(uint16_t*)(response_pkt_buffer + response_option_offset + 12) = 0x30; // Report min MTU
                    response_option_offset += 4;
                } else {
                    channel->local_mtu = (uint32_t)mtu_val; // Update channel's MTU
                    // Add option to response indicating acceptance (same as request)
                    response_pkt_buffer[response_option_offset + 10] = option_type;
                    response_pkt_buffer[response_option_offset + 11] = 2;
                    *(uint16_t*)(response_pkt_buffer + response_option_offset + 12) = mtu_val;
                    response_option_offset += 4;
                }
            } else { // Invalid length for MTU option
                *(uint16_t*)(response_pkt_buffer + 8) = 2; // Result: Bad parameter
            }
        } else if ((int8_t)option_type >= 0) { // Unknown mandatory option (bit 7 not set)
            *(uint16_t*)(response_pkt_buffer + 8) = 3; // Result: Unknown option
            // Copy unknown option to response (as is)
            response_pkt_buffer[response_option_offset + 10] = option_type;
            response_pkt_buffer[response_option_offset + 11] = option_len;
            memcpy(response_pkt_buffer + response_option_offset + 12,
                   (void *)(param_1_cmd_data + 10 + option_offset), option_len);
            response_option_offset += (2 + option_len);
        }
        // If (char)option_type < 0, it's an optional parameter, ignore if unknown.
        option_offset += (2 + option_len); // Move to next option
    }

    // Fill in total length of response packet (header + processed options)
    uint16_t response_payload_total_len = (uint16_t)(response_option_offset + 6); // 6 bytes for fixed header (CID, flags, result)
    *(uint16_t*)(response_pkt_buffer + 2) = response_payload_total_len;

    _session_send(1, response_payload_total_len + 4, response_pkt_buffer); // +4 for L2CAP header
    free(response_pkt_buffer);

    // If configuration was successful and channel state is not already 'configured'
    if ((*(uint16_t*)(response_pkt_buffer + 8) == 0) && // Result is success
        ((*(uint16_t*)(response_pkt_buffer + 6) & 1) == 0) && // Configuration flags check
        (channel->state != 2)) {
        session_send_config(channel); // Send a config request back (if needed for negotiation)
    }
}

// Function: session_configuration_response
// Handles an incoming L2CAP Configuration Response.
void session_configuration_response(const undefined *param_1_cmd_data) {
    // Incoming L2CAP Configuration Response packet structure:
    // Code (0) | ID (1) | Length (2,3) | Local_CID (4,5) | Flags (6,7) | Result (8,9) | Options (...)
    const uint16_t local_cid_in_pkt = *(const uint16_t*)(param_1_cmd_data + 4);
    // const undefined packet_id = param_1_cmd_data[1]; // Not used

    channel_entry_t *channel = NULL;
    if (local_cid_in_pkt < sizeof(g_channels) / sizeof(g_channels[0])) {
        channel = g_channels[local_cid_in_pkt];
    }

    // Check if channel exists, is in 'connected' state (1), and a config request was sent (unknown1 == 1)
    if (channel != NULL && channel->state == 1 && channel->unknown1 == 1) {
        channel->state = 2; // Move channel to 'configured' state
    }
    // No explicit reject for invalid response, just ignores it if conditions not met.
}

// Function: session_disconnection_request
// Handles an incoming L2CAP Disconnection Request.
void session_disconnection_request(const undefined *param_1_cmd_data) {
    // Incoming L2CAP Disconnection Request packet structure:
    // Code (0) | ID (1) | Length (2,3) | Local_CID (4,5) | Remote_CID (6,7)
    const uint16_t payload_len = *(const uint16_t *)(param_1_cmd_data + 2);
    const uint16_t local_cid_in_pkt = *(const uint16_t *)(param_1_cmd_data + 4);
    const uint16_t remote_cid_in_pkt = *(const uint16_t *)(param_1_cmd_data + 6);
    const undefined packet_id = param_1_cmd_data[1];

    if (payload_len == 4) { // Expected payload length for CIDs
        channel_entry_t *channel = NULL;
        if (local_cid_in_pkt < sizeof(g_channels) / sizeof(g_channels[0])) {
            channel = g_channels[local_cid_in_pkt];
        }

        if (channel == NULL || channel->state == 0) {
            session_send_reject(packet_id, 2); // Invalid CID or channel already disconnected
        } else {
            // Call disconnection callback if registered
            if (channel->callback_fn != 0) {
                uint32_t reason = 1; // Disconnect reason (e.g., remote initiated)
                // Assuming callback_fn is of type void (*)(channel_entry_t*, undefined4, uint32_t*)
                ((void (*)(channel_entry_t*, undefined4, uint32_t*))channel->callback_fn)
                    (channel, channel->user_data, &reason);
            }

            // Send L2CAP Disconnection Response
            struct {
                undefined code;
                undefined id;
                undefined2 length;
                undefined2 local_cid;
                undefined2 remote_cid;
            } disc_resp_pkt;

            disc_resp_pkt.code = 7; // Disconnection Response
            disc_resp_pkt.id = packet_id;
            disc_resp_pkt.length = 4; // Length of CIDs
            disc_resp_pkt.local_cid = local_cid_in_pkt;
            disc_resp_pkt.remote_cid = remote_cid_in_pkt;

            _session_send(1, sizeof(disc_resp_pkt), &disc_resp_pkt);

            channel->state = 0; // Mark channel as disconnected
            // The actual channel_entry_t object is not freed here,
            // assuming it might be reused or freed by a higher layer.
        }
    } else {
        session_send_reject(packet_id, 0); // Invalid length
    }
}

// Function: session_disconnection_response
// Handles an incoming L2CAP Disconnection Response.
void session_disconnection_response(const undefined *param_1_cmd_data) {
    // Incoming L2CAP Disconnection Response packet structure:
    // Code (0) | ID (1) | Length (2,3) | Local_CID (4,5) | Remote_CID (6,7)
    const uint16_t payload_len = *(const uint16_t *)(param_1_cmd_data + 2);
    const uint16_t local_cid_in_pkt = *(const uint16_t*)(param_1_cmd_data + 4);
    const undefined packet_id = param_1_cmd_data[1];

    if (payload_len == 4) { // Expected payload length for CIDs
        channel_entry_t *channel = NULL;
        if (local_cid_in_pkt < sizeof(g_channels) / sizeof(g_channels[0])) {
            channel = g_channels[local_cid_in_pkt];
        }

        // If channel exists and is in 'disconnecting' state (3, as implied by original `*pcVar1 != '\x03'`)
        if (channel != NULL && channel->state == 3) {
            channel->state = 0; // Mark channel as disconnected
        } else {
            session_send_reject(packet_id, 2); // Channel not in expected state or not found
        }
    } else {
        session_send_reject(packet_id, 0); // Invalid length
    }
}

// Function: session_echo_request
// Handles an incoming L2CAP Echo Request by sending an Echo Response.
void session_echo_request(undefined *param_1_cmd_data) {
    // Incoming L2CAP Echo Request packet structure:
    // Code (0) | ID (1) | Length (2,3) | Data (...)
    // Change command code from 8 (Echo Request) to 9 (Echo Response)
    param_1_cmd_data[0] = 9;
    // The total length for _session_send is the L2CAP command payload length + 4 bytes for L2CAP Command Header
    _session_send(1, *(uint16_t*)(param_1_cmd_data + 2) + 4, param_1_cmd_data);
}

// Function: session_information_request
// Handles an incoming L2CAP Information Request.
void session_information_request(const undefined *param_1_cmd_data) {
    // Incoming L2CAP Information Request packet structure:
    // Code (0) | ID (1) | Length (2,3) | Info_Type (4,5)
    const uint16_t payload_len = *(const uint16_t *)(param_1_cmd_data + 2);
    const uint16_t info_type_in_pkt = *(const uint16_t*)(param_1_cmd_data + 4);
    const undefined packet_id = param_1_cmd_data[1];

    if (payload_len == 2) { // Expected payload length for Info_Type
        struct {
            undefined code;
            undefined id;
            undefined2 length;
            undefined2 info_type;
            undefined2 result;
        } info_resp_pkt;

        info_resp_pkt.code = 0xb; // Information Response
        info_resp_pkt.id = packet_id;
        info_resp_pkt.length = 4; // Length of info_type + result
        info_resp_pkt.info_type = info_type_in_pkt;
        info_resp_pkt.result = 1; // Result: success/supported

        _session_send(1, sizeof(info_resp_pkt), &info_resp_pkt);
    } else {
        session_send_reject(packet_id, 0); // Invalid length
    }
}

// Function: session_handle_command
// Dispatches L2CAP control commands to their respective handlers.
void session_handle_command(undefined *param_1_cmd_data) {
    switch (param_1_cmd_data[0]) { // Command Code is the first byte
        case 1: // Command Reject
        case 3: // Connection Response
        case 9: // Echo Response
        case 0xb: // Information Response
            // These are responses, typically processed by the sender of the request.
            // Original code does nothing for these cases, implying they are handled elsewhere
            // or simply ignored if received by this general command handler.
            break;
        case 2: // Connection Request
            session_connection_request(param_1_cmd_data);
            break;
        case 4: // Configuration Request
            session_configuration_request(param_1_cmd_data);
            break;
        case 5: // Configuration Response
            session_configuration_response(param_1_cmd_data);
            break;
        case 6: // Disconnection Request
            session_disconnection_request(param_1_cmd_data);
            break;
        case 7: // Disconnection Response
            session_disconnection_response(param_1_cmd_data);
            break;
        case 8: // Echo Request
            session_echo_request(param_1_cmd_data);
            break;
        case 10: // Information Request
            session_information_request(param_1_cmd_data);
            break;
        default:
            session_send_reject(param_1_cmd_data[1], 0); // Unknown command
            break;
    }
}

// Function: session_handle_control
// Parses and handles multiple L2CAP control commands within a single L2CAP control packet.
undefined4 session_handle_control(void) {
    size_t offset_in_payload = 0; // Current offset within the L2CAP control packet payload

    // Loop through commands in the L2CAP payload
    while (G_RECV_TOTAL_L2CAP_PAYLOAD_LEN >= offset_in_payload + 4) { // Ensure at least a command header (Code, ID, Length) is available
        undefined *current_cmd_data = G_RECV_L2CAP_PAYLOAD_START + offset_in_payload;
        uint16_t command_payload_len = *(uint16_t*)(current_cmd_data + 2); // Length field of the current command

        // Check for malformed packet: reported total payload length is less than expected for current command
        if (G_RECV_TOTAL_L2CAP_PAYLOAD_LEN < offset_in_payload + 4 + command_payload_len) {
            return 0; // Or handle error appropriately (e.g., log, reject, discard)
        }

        session_handle_command(current_cmd_data);
        offset_in_payload += (4 + command_payload_len); // Move to the next command (4 bytes for command header)
    }
    return 0;
}

// Function: session_handle_packet
// Dispatches received L2CAP packets to either the control channel handler or a data channel handler.
undefined4 session_handle_packet(void) {
    // G_RECV_L2CAP_CID_OR_TYPE identifies the L2CAP channel or command type (1 for control channel)
    if (G_RECV_L2CAP_CID_OR_TYPE == 1) { // L2CAP Control Channel
        return session_handle_control();
    } else { // L2CAP Data Channel
        channel_entry_t *channel = NULL;
        if (G_RECV_L2CAP_CID_OR_TYPE < sizeof(g_channels) / sizeof(g_channels[0])) {
            channel = g_channels[G_RECV_L2CAP_CID_OR_TYPE];
        }

        // Check if channel exists and is in 'configured' state (2)
        if (channel == NULL || channel->state != 2) {
            return 0; // Or indicate error (e.g., drop packet, send reject)
        } else {
            if (channel->callback_fn != 0) {
                // Call the channel's data reception callback
                // Assuming callback_fn is of type void (*)(channel_entry_t*, undefined4, undefined*, uint32_t)
                ((void (*)(channel_entry_t*, undefined4, undefined*, uint32_t))channel->callback_fn)
                    (channel, channel->user_data, G_RECV_L2CAP_PAYLOAD_START, G_RECV_TOTAL_L2CAP_PAYLOAD_LEN);
            }
            return 0;
        }
    }
}

// Function: session_loop
// Main event loop for receiving and processing L2CAP packets.
void session_loop(void) {
    size_t current_recv_offset = SIZE_MAX; // Use SIZE_MAX to indicate an invalid/reset state for the current packet

    while (1) { // Infinite loop for continuous packet processing
        int link_pkt_type;
        size_t link_pkt_len;
        uint8_t recv_chunk_buffer[0x153]; // Max chunk size for link_recv

        int ret = link_recv(&link_pkt_type, &link_pkt_len, recv_chunk_buffer);
        if (ret != 0) {
            return; // Error during receive or shutdown signal
        }

        if (link_pkt_type == 2) { // Start of a new L2CAP packet
            current_recv_offset = 0;
            memset(g_recv_pkt_buffer, 0, sizeof(g_recv_pkt_buffer)); // Clear receive buffer for new packet
        } else if (link_pkt_type != 1) { // Not start (2) or continuation (1), invalid packet type
            current_recv_offset = SIZE_MAX; // Reset state, discard current incomplete packet
            continue;
        }

        if (current_recv_offset != SIZE_MAX) { // If we are in the middle of receiving a packet
            // Check for buffer overflow before copying
            if (link_pkt_len + current_recv_offset < sizeof(g_recv_pkt_buffer)) {
                memcpy(g_recv_pkt_buffer + current_recv_offset, recv_chunk_buffer, link_pkt_len);
                current_recv_offset += link_pkt_len;

                // Check if a full L2CAP packet has been received
                // An L2CAP packet consists of 2 bytes Length + 2 bytes CID + L2CAP Payload.
                // G_RECV_TOTAL_L2CAP_PAYLOAD_LEN holds the L2CAP Payload Length.
                // So, the total expected bytes in g_recv_pkt_buffer for a complete L2CAP packet is G_RECV_TOTAL_L2CAP_PAYLOAD_LEN + 4.
                if (current_recv_offset > 3 && (G_RECV_TOTAL_L2CAP_PAYLOAD_LEN + 4 == current_recv_offset)) {
                    current_recv_offset = SIZE_MAX; // Packet complete, reset state
                    if (session_handle_packet() != 0) {
                        return; // Error during packet handling, or shutdown signal
                    }
                }
            } else {
                // Buffer overflow: received packet chunk would exceed buffer size
                current_recv_offset = SIZE_MAX; // Reset state, discard current packet
            }
        }
    }
}

// Function: session_register_psm
// Registers a PSM ID and its associated callback function.
void session_register_psm(undefined4 psm_id, undefined4 callback_fn_ptr) {
    psm_entry_t *psm_entry = session_find_psm(psm_id);
    if (psm_entry == NULL) {
        psm_entry = (psm_entry_t *)malloc(sizeof(psm_entry_t));
        if (psm_entry == NULL) {
            return; // Out of memory
        }
        psm_entry->psm_id = psm_id;
        psm_entry->next = g_psm_head; // Add new entry to the head of the linked list
        g_psm_head = psm_entry;
    }
    psm_entry->callback_fn = callback_fn_ptr; // Update or set callback function
}

// Function: session_register_events
// Registers an event callback function for a specific channel.
void session_register_events(channel_entry_t *channel, undefined4 callback_fn_ptr) {
    if (channel != NULL) {
        channel->callback_fn = callback_fn_ptr;
    }
}

// Function: session_register_userdata
// Registers user-defined data for a specific channel.
void session_register_userdata(channel_entry_t *channel, undefined4 user_data_val) {
    if (channel != NULL) {
        channel->user_data = user_data_val;
    }
}

// Function: session_send
// Sends data over a specific channel.
void session_send(channel_entry_t *channel, undefined2 payload_len, undefined4 data_ptr) {
    if (channel != NULL) {
        _session_send(channel->remote_cid, payload_len, (void *)(uintptr_t)data_ptr);
    }
}