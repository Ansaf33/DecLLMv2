#include <stdio.h>    // For printf, fprintf
#include <stdlib.h>   // For atoi, malloc, free, exit
#include <string.h>   // For memcpy, bzero (or memset)
#include <stdint.h>   // For uint8_t, uint16_t, uint32_t
#include <stdarg.h>   // For va_list in vTransmitFormattedBytes

// --- Type Definitions ---
typedef int STATUS; // For functions returning 0 on success, -1 or -2 on failure

// --- Global Data (approximated from usage) ---
// DAT_000140f5 is used as a single byte. It's likely a newline character.
static const char DAT_000140f5 = '\n';

// Placeholder arrays for names/strings
static const char *authority_names[] = {
    "Local PD", "State Patrol", "FBI", "CIA", "NSA", "Unknown Agency"
};
static const char *physical_type_names[] = {
    "Ethernet", "Wireless", "Fiber", "Unknown Physical Type"
};
static const char *physical_wire_speeds[] = {
    "10 Mbps", "100 Mbps", "1 Gbps", "10 Gbps", "Unknown Speed"
};
static const char *debug_type_names[] = {
    "Ping", "Traceroute", "Route Update", "Error", "Status", "Custom", "Unknown Debug Type"
};

// --- Helper Function Prototypes (approximated from usage) ---
// vTransmitFormattedBytes seems to be a printf-like function taking a format string and va_list.
// DisplayFormatted is the public interface, taking variable arguments.
void vTransmitFormattedBytes(const char *format, va_list args);
void TransmitFormattedBytes(const char *format, ...);
void TransmitBytes(const void *data, size_t len);
int count_strings(const char *s);
const char *next_string(const char *s);
void HexDump(const void *data, size_t len);
void *allocate(size_t size);
void deallocate(void *ptr);
void _terminate(void);

// --- Struct Definitions (inferred from memory accesses) ---

// StatsData: Holds statistics for packet processing
typedef struct StatsData {
    uint32_t total_packets;       // +0
    uint32_t start_time;          // +4
    uint32_t end_time;            // +8
    uint32_t largest_packet;      // +0xc
    uint32_t smallest_packet;     // +0x10
    uint32_t filter_option;       // +0x14 (used in AnalyzePacket)
    uint32_t some_field_18;       // +0x18
    uint32_t packets_shown;       // +0x1c
    uint32_t malformed_packets;   // +0x20
    void*    option_headers_list; // +0x28 -> points to first OptionHeader (void* or OptionHeader*)
} StatsData;

// OptionHeader: Linked list of option headers
typedef struct OptionHeader {
    uint8_t  type;       // +0
    uint32_t next_ptr_val; // +4 (pointer to next OptionHeader, stored as uint32_t)
    void*    data_ptr;   // +8 (pointer to option-specific data)
} OptionHeader;

// Filter: Used for packet filtering
typedef struct {
    uint16_t size;      // +0 (length of mask/value in bytes)
    uint8_t  type;      // +2 (1 byte, used by ProcessAllFilters, 1 for accept, others for reject)
    uint8_t  _padding;  // +3 (padding to align next pointer if 4-byte aligned)
    uint8_t *mask_ptr;  // +4 (pointer to mask bytes for FilterMatch)
    uint8_t *value_ptr; // +8 (pointer to value bytes for FilterMatch)
} Filter;

// PacketContext: Holds global state/flags for packet processing,
// and also filter list information as per ProcessAllFilters usage.
typedef struct {
    uint32_t some_field_0;    // param_1 + 0
    uint16_t display_flags;   // param_1 + 2 (used by DisplayFormatted)
    uint8_t  num_filters;     // param_1 + 4 (used by ProcessAllFilters)
    Filter **filters_array;   // param_1 + 8 (used by ProcessAllFilters)
    uint32_t some_field_c;    // param_1 + c
    uint32_t some_field_10;   // param_1 + 10
    // ... potentially other fields up to 0x48
    StatsData *stats_ptr;     // param_1 + 0x48 -> points to StatsData
} PacketContext;

// PacketState: Represents the current packet data being analyzed,
// with pointers and lengths that are consumed by layer analysis functions.
typedef struct {
    uint16_t remaining_length;      // param_2 + 4
    uint8_t *current_data_ptr;      // param_2 + 8
    uint8_t *original_data_ptr;     // The very beginning of the packet data
    uint32_t original_total_length; // Derived from original_data_ptr + 8
} PacketState;

// InitialPacketHeader: The structure at the very beginning of the packet data
// (pointed to by PacketState->original_data_ptr)
typedef struct {
    uint32_t preamble_0; // Not clearly used
    uint32_t preamble_4; // Not clearly used
    uint32_t total_length;  // +8 (iVar1 in AnalyzePhysicalLayer)
    uint16_t physical_type; // +0xc
    uint16_t physical_subtype_or_wirespeed; // +0xe
    uint32_t physical_checksum_or_src; // +0x10
    // ... more fields follow depending on physical_type
} InitialPacketHeader;


// --- Stub Implementations for Helper Functions ---

void vTransmitFormattedBytes(const char *format, va_list args) {
    vprintf(format, args);
}

void TransmitFormattedBytes(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vTransmitFormattedBytes(format, args);
    va_end(args);
}

void TransmitBytes(const void *data, size_t len) {
    fwrite(data, 1, len, stdout);
}

int count_strings(const char *s) {
    if (!s || *s == '\0') return 0;
    int count = 0;
    const char *current = s;
    while (*current) {
        count++;
        current += strlen(current) + 1;
        if (*current == '\0') break; // End of list marker (double null)
    }
    return count;
}

const char *next_string(const char *s) {
    if (!s || *s == '\0') return NULL;
    s += strlen(s) + 1;
    if (*s == '\0') return NULL; // No more strings
    return s;
}

void HexDump(const void *data, size_t len) {
    const uint8_t *bytes = (const uint8_t *)data;
    size_t i, j;
    TransmitFormattedBytes("Hex Dump (%zu bytes):\n", len);
    for (i = 0; i < len; i += 16) {
        TransmitFormattedBytes("  %08zx: ", i);
        for (j = 0; j < 16; j++) {
            if (i + j < len) {
                TransmitFormattedBytes("%02x ", bytes[i + j]);
            } else {
                TransmitFormattedBytes("   ");
            }
        }
        TransmitFormattedBytes(" ");
        for (j = 0; j < 16; j++) {
            if (i + j < len) {
                char c = bytes[i + j];
                TransmitBytes((c >= ' ' && c <= '~') ? &c : ".", 1);
            } else {
                TransmitBytes(" ", 1);
            }
        }
        TransmitBytes("\n", 1);
    }
}

void *allocate(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed for %zu bytes!\n", size);
        _terminate();
    }
    return ptr;
}

void deallocate(void *ptr) {
    free(ptr);
}

void _terminate(void) {
    fprintf(stderr, "Program terminated due to an error.\n");
    exit(1);
}

// --- Original Functions Refactored ---

// Function: DisplayFormatted
// param_1: PacketContext *ctx
// param_2: uint16_t display_flag
// param_3: const char *format (followed by variadic arguments)
void DisplayFormatted(PacketContext *ctx, uint16_t display_flag, const char *format, ...) {
    if ((ctx->display_flags & display_flag) != 0) {
        va_list args;
        va_start(args, format);
        vTransmitFormattedBytes(format, args);
        va_end(args);
    }
}

// Function: TransmitOptionHeader
void TransmitOptionHeader(OptionHeader *opt) {
    switch (opt->type) {
        case 1:
            TransmitFormattedBytes("Option 1 Data: 0x%x\n", (uint32_t)(uintptr_t)opt->data_ptr);
            TransmitBytes(&DAT_000140f5, 1);
            break;
        case 2: {
            const char *str_data = (const char *)opt->data_ptr;
            int num_strings = count_strings(str_data);
            if (num_strings == 2) {
                const char *lat = str_data;
                const char *lon = next_string(str_data);
                TransmitFormattedBytes("GPS Coordinates: %s Latitude, %s Longitude\n", lat, lon);
            } else {
                TransmitFormattedBytes("Address: %s\n", str_data);
            }
            break;
        }
        case 3: {
            int authority_id = atoi((const char *)opt->data_ptr);
            if (authority_id >= 0 && authority_id < sizeof(authority_names) / sizeof(authority_names[0])) {
                TransmitFormattedBytes("Capturing Authority: %s\n", authority_names[authority_id]);
            } else {
                TransmitFormattedBytes("Capturing Authority Unknown\n");
            }
            break;
        }
        case 4:
            TransmitFormattedBytes("Capture Date: %s\n", (const char *)opt->data_ptr);
            break;
        case 5: {
            int sharing_allowed = atoi((const char *)opt->data_ptr);
            TransmitFormattedBytes("Sharing Allowed: %s\n", (sharing_allowed == 1) ? "True" : "False");
            break;
        }
        case 6: {
            int modified = atoi((const char *)opt->data_ptr);
            TransmitFormattedBytes("This content has %sbeen modified from the original\n", (modified == 1) ? "" : "not ");
            break;
        }
        case 7:
            TransmitFormattedBytes("Capturing Device: %s\n", (const char *)opt->data_ptr);
            break;
        default:
            TransmitFormattedBytes("Unknown Option Type: %d\n", opt->type);
            break;
    }
}

// Function: DisplayStatistics
// param_1: PacketContext *ctx
void DisplayStatistics(PacketContext *ctx) {
    StatsData *stats = ctx->stats_ptr;
    TransmitFormattedBytes("Final Statistics:\n");
    TransmitFormattedBytes("\tTotal Packets: %u\n", stats->total_packets);
    TransmitFormattedBytes("\tStart Time: 0x%x\n", stats->start_time);
    TransmitFormattedBytes("\tEnd Time: 0x%x\n", stats->end_time);
    TransmitFormattedBytes("\tLargest Packet: %u\n", stats->largest_packet);
    TransmitFormattedBytes("\tSmallest Packet: %u\n", stats->smallest_packet);
    TransmitFormattedBytes("\tNumber of malformed packets: %u\n", stats->malformed_packets);
    TransmitFormattedBytes("\tNumber of packets shown: %u\n", stats->packets_shown);
    TransmitFormattedBytes("Option Headers:\n");

    // Assuming option_headers_list is a pointer to the first OptionHeader
    // and OptionHeader->next_ptr_val is a direct pointer value (int-casted address).
    OptionHeader *current_opt_header = (OptionHeader *)stats->option_headers_list;
    while (current_opt_header != NULL) {
        TransmitOptionHeader(current_opt_header);
        current_opt_header = (OptionHeader *)(uintptr_t)current_opt_header->next_ptr_val;
    }
    TransmitBytes(&DAT_000140f5, 1);
}

// Function: FilterMatch
// param_1: Filter *filter
// param_2: PacketState *packet_state
STATUS FilterMatch(Filter *filter, PacketState *packet_state) {
    if (packet_state->remaining_length < filter->size) {
        return 0; // Mismatch: packet data too short for filter
    }
    if (packet_state->current_data_ptr == NULL) {
        return 0; // No data to match
    }

    for (uint16_t i = 0; i < filter->size; i++) {
        // Condition: (data[i] & (mask[i] ^ value[i])) != 0
        // This implies a match occurs if (data[i] & (mask[i] ^ value[i])) == 0 for all bytes.
        // If it's NOT 0 for any byte, it's a mismatch.
        if ((packet_state->current_data_ptr[i] & (filter->mask_ptr[i] ^ filter->value_ptr[i])) != 0) {
            return 0; // Mismatch
        }
    }
    return 1; // Match
}

// Function: ProcessAllFilters
// param_1: PacketState *packet_state
// param_2: PacketContext *filter_context (param_1 from AnalyzePacket)
// param_3: int *reject_count
// param_4: int *accept_count
void ProcessAllFilters(PacketState *packet_state, PacketContext *filter_context, int *reject_count, int *accept_count) {
    int local_accept_count = 0;
    int local_reject_count = 0;

    for (int i = 0; i < filter_context->num_filters; i++) {
        Filter *current_filter = filter_context->filters_array[i];
        if (FilterMatch(current_filter, packet_state) == 1) {
            if (current_filter->type == 1) { // Assuming filter->type == 1 means "accept"
                local_accept_count++;
            } else { // Other types mean "reject"
                local_reject_count++;
            }
        }
    }
    *reject_count = local_reject_count;
    *accept_count = local_accept_count;
}

// Function: CalculateChecksum
int CalculateChecksum(const uint8_t *data, uint16_t length) {
    int checksum = 0;
    for (uint16_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum;
}

// Function: AnalyzePhysicalLayer
// param_1: PacketContext *ctx
// param_2: PacketState *state
STATUS AnalyzePhysicalLayer(PacketContext *ctx, PacketState *state) {
    InitialPacketHeader *initial_header = (InitialPacketHeader *)state->original_data_ptr;
    uint32_t physical_length = initial_header->total_length; // Total length of the physical frame

    if (state->original_total_length < physical_length) {
        DisplayFormatted(ctx, 1, "Invalid Packet Length: 0x%x (expected 0x%x)\n", physical_length, state->original_total_length);
        return -1;
    }
    if (physical_length < 0xe) {
        DisplayFormatted(ctx, 1, "Packet too short: %u bytes\n", physical_length);
        return -1;
    }

    DisplayFormatted(ctx, 1, "Physical Length: %u\n", physical_length);

    uint16_t physical_type_val = initial_header->physical_type;
    uint16_t physical_type_idx = physical_type_val - 1; // 0-indexed

    if (physical_type_idx < sizeof(physical_type_names) / sizeof(physical_type_names[0]) -1) { // -1 for "Unknown Physical Type"
        DisplayFormatted(ctx, 1, "Physical Type: %s\n", physical_type_names[physical_type_idx]);

        if (physical_type_idx == 0) { // Ethernet
            if (physical_length < 0x14) {
                DisplayFormatted(ctx, 1, "Packet too short for Ethernet: %u bytes\n", physical_length);
                return -1;
            }
            uint16_t wire_speed_val = initial_header->physical_subtype_or_wirespeed;
            uint16_t wire_speed_idx = wire_speed_val - 1;
            if (wire_speed_idx >= sizeof(physical_wire_speeds) / sizeof(physical_wire_speeds[0]) - 1) { // -1 for "Unknown Speed"
                 DisplayFormatted(ctx, 1, "Invalid Wire speed: %u\n", wire_speed_val);
                 return -1;
            }
            DisplayFormatted(ctx, 1, "Wire Speed: %s\n", physical_wire_speeds[wire_speed_idx]);
            uint32_t expected_checksum = initial_header->physical_checksum_or_src;

            state->current_data_ptr += 0x14;
            state->remaining_length -= 0x14;

            uint32_t calculated_checksum = CalculateChecksum(state->current_data_ptr, state->remaining_length);
            if (expected_checksum != calculated_checksum) {
                DisplayFormatted(ctx, 1, "Invalid Physical Layer Checksum: got 0x%x, expected 0x%x\n", expected_checksum, calculated_checksum);
                return -1;
            }
        } else if (physical_type_idx == 1) { // Wireless
            if (physical_length < 0x2a) {
                DisplayFormatted(ctx, 1, "Packet too short for Wireless: %u bytes\n", physical_length);
                return -1;
            }
            // Wireless fields start at offset 0xe from the *original* data pointer
            DisplayFormatted(ctx, 1, "Source Address: 0x%x\n", *(uint32_t *)(state->original_data_ptr + 0xe));
            DisplayFormatted(ctx, 1, "Destination Address: 0x%x\n", *(uint32_t *)(state->original_data_ptr + 0x12));
            DisplayFormatted(ctx, 1, "Transmit strength: %u\n", *(uint32_t *)(state->original_data_ptr + 0x16));
            DisplayFormatted(ctx, 1, "Receive strength: %u\n", *(uint32_t *)(state->original_data_ptr + 0x1a));
            DisplayFormatted(ctx, 1, "Frequency: %u\n", *(uint32_t *)(state->original_data_ptr + 0x1e));
            DisplayFormatted(ctx, 1, "SNR: %u\n", *(uint32_t *)(state->original_data_ptr + 0x22));

            state->current_data_ptr += 0x2a;
            state->remaining_length -= 0x2a;
        } else if (physical_type_idx == 2) { // Fiber
            if ((ctx->display_flags & 1) != 0) { // Check if flag 1 is set for hex dump
                HexDump(state->original_data_ptr + 0xe, state->remaining_length - 0xe);
            }
            return -1; // Original returns 0xffffffff here, implying failure/end of processing
        } else {
            DisplayFormatted(ctx, 1, "Invalid Physical Type (subtype): %u\n", physical_type_val);
            return -1;
        }
    } else {
        DisplayFormatted(ctx, 1, "Invalid Physical Type: %u\n", physical_type_val);
        return -1;
    }
    return 0; // Success
}

// Function: AnalyzeTransportLayer
// param_1: PacketContext *ctx
// param_2: PacketState *state
STATUS AnalyzeTransportLayer(PacketContext *ctx, PacketState *state) {
    if (state->remaining_length < 4) {
        DisplayFormatted(ctx, 2, "Transport Layer too short\n");
        return -1;
    }
    uint32_t transport_type = *(uint32_t *)state->current_data_ptr;

    if (transport_type == 1) { // STREAM
        if (state->remaining_length < 0x16) {
            DisplayFormatted(ctx, 2, "Transport Layer too short for STREAM\n");
            return -1;
        }
        DisplayFormatted(ctx, 2, "Transport Type: STREAM\n");
        DisplayFormatted(ctx, 2, "Stream number: %u\n", *(uint32_t *)(state->current_data_ptr + 4));
        DisplayFormatted(ctx, 2, "Sequence number: %u\n", *(uint32_t *)(state->current_data_ptr + 8));
        DisplayFormatted(ctx, 2, "Window: %d\n", *(uint16_t *)(state->current_data_ptr + 0xc));
        DisplayFormatted(ctx, 2, "Acknowledgment: 0x%x\n", *(uint32_t *)(state->current_data_ptr + 0xe));
        DisplayFormatted(ctx, 2, "Length: %u\n", *(uint32_t *)(state->current_data_ptr + 0x12));
        state->current_data_ptr += 0x16;
        state->remaining_length -= 0x16;
    } else if (transport_type == 2) { // MESSAGE
        if (state->remaining_length < 8) {
            DisplayFormatted(ctx, 2, "Transport Layer too short for MESSAGE\n");
            return -1;
        }
        DisplayFormatted(ctx, 2, "Transport Type: MESSAGE\n");
        DisplayFormatted(ctx, 2, "Length: %u\n", *(uint32_t *)(state->current_data_ptr + 4));
        state->current_data_ptr += 8;
        state->remaining_length -= 8;
    } else {
        DisplayFormatted(ctx, 2, "Invalid Transport Type: %u\n", transport_type);
        return -1;
    }
    return 0;
}

// Function: AnalyzeNetworkLayer
// param_1: PacketContext *ctx
// param_2: PacketState *state
STATUS AnalyzeNetworkLayer(PacketContext *ctx, PacketState *state) {
    if (state->remaining_length < 6) {
        DisplayFormatted(ctx, 4, "Network layer too short\n");
        return -1;
    }
    uint16_t network_type = *(uint16_t *)(state->current_data_ptr + 4);

    if (network_type == 1) { // Endpoint
        DisplayFormatted(ctx, 4, "Network type: Endpoint\n");
        if (state->remaining_length < 0xe) {
            DisplayFormatted(ctx, 4, "Network layer too short for Endpoint\n");
            return -1;
        }
        DisplayFormatted(ctx, 4, "Source Address: 0x%x\n", *(uint32_t *)(state->current_data_ptr + 6));
        DisplayFormatted(ctx, 4, "Destination Address: 0x%x\n", *(uint32_t *)(state->current_data_ptr + 10));
        state->current_data_ptr += 0xe;
        state->remaining_length -= 0xe;
    } else if (network_type == 2) { // Broadcast
        DisplayFormatted(ctx, 4, "Network type: Broadcast\n");
        if (state->remaining_length < 10) {
            DisplayFormatted(ctx, 4, "Network layer too short for Broadcast\n");
            return -1;
        }
        DisplayFormatted(ctx, 4, "Source Address: 0x%x\n", *(uint32_t *)(state->current_data_ptr + 6));
        state->current_data_ptr += 10;
        state->remaining_length -= 10;
    } else if (network_type == 3) { // Debug
        DisplayFormatted(ctx, 4, "Network type: Debug\n");
        if (state->remaining_length < 0x16) {
            DisplayFormatted(ctx, 4, "Network layer too short for Debug\n");
            return -1;
        }
        uint16_t debug_type_val = *(uint16_t *)(state->current_data_ptr + 6);
        uint16_t debug_type_idx = debug_type_val - 1;
        if (debug_type_idx >= sizeof(debug_type_names) / sizeof(debug_type_names[0]) -1) { // -1 for "Unknown Debug Type"
            DisplayFormatted(ctx, 4, "Invalid network debug type: %u\n", debug_type_val);
            return -1;
        }
        DisplayFormatted(ctx, 4, "Network debug type: %s\n", debug_type_names[debug_type_idx]);
        DisplayFormatted(ctx, 4, "Source Address: 0x%x\n", *(uint32_t *)(state->current_data_ptr + 8));
        DisplayFormatted(ctx, 4, "Destination Address: 0x%x\n", *(uint32_t *)(state->current_data_ptr + 0xc));
        DisplayFormatted(ctx, 4, "Hop Count: %u\n", *(uint16_t *)(state->current_data_ptr + 0x10));
        DisplayFormatted(ctx, 4, "Timestamp: %u\n", *(uint32_t *)(state->current_data_ptr + 0x12));
        state->current_data_ptr += 0x16;
        state->remaining_length -= 0x16;
    } else {
        DisplayFormatted(ctx, 4, "Invalid network type: %u\n", network_type);
        return -1;
    }
    return 0;
}

// Function: AnalyzeApplicationLayer
// param_1: PacketContext *ctx
// param_2: PacketState *state
STATUS AnalyzeApplicationLayer(PacketContext *ctx, PacketState *state) {
    char temp_buffer[256] = {0}; // For memcpy operations, zero-initialized
    void *allocated_url_ptr = NULL;
    void *allocated_headers_ptr = NULL;

    if (state->remaining_length < 6) {
        DisplayFormatted(ctx, 8, "Application layer too short\n");
        return -1;
    }
    uint32_t app_protocol_id = *(uint32_t *)state->current_data_ptr; // Not explicitly used but read
    uint16_t app_type = *(uint16_t *)(state->current_data_ptr + 4);

    if (app_type == 1) { // Command
        DisplayFormatted(ctx, 8, "Application type: Command\n");
        if (state->remaining_length < 0xc) {
            DisplayFormatted(ctx, 8, "Application layer too short for Command\n");
            return -1;
        }
        DisplayFormatted(ctx, 8, "Command ID: %u\n", *(uint32_t *)(state->current_data_ptr + 6));
        DisplayFormatted(ctx, 8, "Command Data: 0x%x\n", *(uint32_t *)(state->current_data_ptr + 8));
        state->current_data_ptr += 0xc;
        state->remaining_length -= 0xc;
    } else if (app_type == 2) { // Data Transfer
        DisplayFormatted(ctx, 8, "Application type: Data Transfer\n");
        if (state->remaining_length < 0xe) {
            DisplayFormatted(ctx, 8, "Application layer too short for Data Transfer\n");
            return -1;
        }
        DisplayFormatted(ctx, 8, "Transfer ID: %u\n", *(uint32_t *)(state->current_data_ptr + 6));
        DisplayFormatted(ctx, 8, "Block Size: %u\n", *(uint32_t *)(state->current_data_ptr + 8));
        uint16_t num_blocks = *(uint16_t *)(state->current_data_ptr + 0xc);
        DisplayFormatted(ctx, 8, "Number of Blocks: %u\n", num_blocks);
        state->current_data_ptr += 0xe;
        state->remaining_length -= 0xe;

        while (num_blocks > 0 && state->remaining_length > 0) {
            uint8_t block_len = *state->current_data_ptr;
            state->current_data_ptr++;
            state->remaining_length--;

            if (state->remaining_length < block_len) {
                DisplayFormatted(ctx, 8, "Application layer too short for block data (expected %u, got %u)\n", block_len, state->remaining_length);
                return -1;
            }
            memset(temp_buffer, 0, sizeof(temp_buffer));
            memcpy(temp_buffer, state->current_data_ptr, block_len);
            DisplayFormatted(ctx, 8, "Block Data: %s\n", temp_buffer); // Assuming text blocks
            state->current_data_ptr += block_len;
            state->remaining_length -= block_len;
            num_blocks--;
        }
    } else if (app_type == 3) { // Authentication
        DisplayFormatted(ctx, 8, "Application type: Authentication\n");
        if (state->remaining_length < 0x46) {
            DisplayFormatted(ctx, 8, "Application layer too short for Authentication\n");
            return -1;
        }
        memset(temp_buffer, 0, 0x21); // Max 32 chars + null terminator for username
        memcpy(temp_buffer, state->current_data_ptr + 6, 0x20);
        DisplayFormatted(ctx, 8, "Username: %s\n", temp_buffer);

        memset(temp_buffer, 0, 0x21); // Max 32 chars + null terminator for password
        memcpy(temp_buffer, state->current_data_ptr + 0x26, 0x20);
        DisplayFormatted(ctx, 8, "Password: %s\n", temp_buffer); // Insecure, but following original logic
        state->current_data_ptr += 0x46;
        state->remaining_length -= 0x46;
    } else if (app_type == 4) { // File Transfer
        DisplayFormatted(ctx, 8, "Application type: File Transfer\n");
        state->current_data_ptr += 6;
        state->remaining_length -= 6;

        if (state->remaining_length == 0) {
            DisplayFormatted(ctx, 8, "Application layer too short for filename length\n");
            return -1;
        }
        uint8_t filename_len = *state->current_data_ptr;
        state->current_data_ptr++;
        state->remaining_length--;
        if (state->remaining_length < filename_len) {
            DisplayFormatted(ctx, 8, "Application layer too short for filename data (expected %u, got %u)\n", filename_len, state->remaining_length);
            return -1;
        }
        memset(temp_buffer, 0, sizeof(temp_buffer));
        memcpy(temp_buffer, state->current_data_ptr, filename_len);
        DisplayFormatted(ctx, 8, "Filename: %s\n", temp_buffer);
        state->current_data_ptr += filename_len;
        state->remaining_length -= filename_len;

        if (state->remaining_length == 0) {
            DisplayFormatted(ctx, 8, "Application layer too short for filetype length\n");
            return -1;
        }
        uint8_t filetype_len = *state->current_data_ptr;
        state->current_data_ptr++;
        state->remaining_length--;
        if (state->remaining_length < filetype_len) {
            DisplayFormatted(ctx, 8, "Application layer too short for filetype data (expected %u, got %u)\n", filetype_len, state->remaining_length);
            return -1;
        }
        memset(temp_buffer, 0, sizeof(temp_buffer));
        memcpy(temp_buffer, state->current_data_ptr, filetype_len);
        DisplayFormatted(ctx, 8, "File Type: %s\n", temp_buffer);
        state->current_data_ptr += filetype_len;
        state->remaining_length -= filetype_len;

        if (state->remaining_length == 0) {
            DisplayFormatted(ctx, 8, "Application layer too short for file hash length\n");
            return -1;
        }
        uint8_t filehash_len = *state->current_data_ptr;
        state->current_data_ptr++;
        state->remaining_length--;
        if (state->remaining_length < filehash_len) {
            DisplayFormatted(ctx, 8, "Application layer too short for file hash data (expected %u, got %u)\n", filehash_len, state->remaining_length);
            return -1;
        }
        memset(temp_buffer, 0, sizeof(temp_buffer));
        memcpy(temp_buffer, state->current_data_ptr, filehash_len);
        DisplayFormatted(ctx, 8, "File Hash: %s\n", temp_buffer);
        state->current_data_ptr += filehash_len;
        state->remaining_length -= filehash_len;
    } else if (app_type == 5) { // Web Request
        DisplayFormatted(ctx, 8, "Application type: Web Request\n");
        if (state->remaining_length < 8) {
            DisplayFormatted(ctx, 8, "Application layer too short for Web Request header\n");
            return -1;
        }
        state->current_data_ptr += 6; // Skip protocol ID and app type
        state->remaining_length -= 6;

        uint16_t url_len = *(uint16_t *)state->current_data_ptr;
        state->current_data_ptr += 2;
        state->remaining_length -= 2;

        if (url_len > 0x400) { // Max URL length check
            DisplayFormatted(ctx, 8, "Invalid URL length: %u (max 1024)\n", url_len);
            return -1;
        }
        if (state->remaining_length < url_len) {
            DisplayFormatted(ctx, 8, "Application layer too short for URL (expected %u, got %u)\n", url_len, state->remaining_length);
            return -1;
        }
        allocated_url_ptr = allocate(url_len + 1);
        memset(allocated_url_ptr, 0, url_len + 1);
        memcpy(allocated_url_ptr, state->current_data_ptr, url_len);
        DisplayFormatted(ctx, 8, "URL: %s\n", (char *)allocated_url_ptr);
        state->current_data_ptr += url_len;
        state->remaining_length -= url_len;
        deallocate(allocated_url_ptr);
        allocated_url_ptr = NULL; // Clear pointer after use

        if (state->remaining_length < 2) {
            DisplayFormatted(ctx, 8, "Application layer too short for headers length\n");
            return -1;
        }
        uint16_t headers_len = *(uint16_t *)state->current_data_ptr;
        state->current_data_ptr += 2;
        state->remaining_length -= 2;

        if (headers_len > 0x400) { // Max headers length check
            DisplayFormatted(ctx, 8, "Invalid header length: %u (max 1024)\n", headers_len);
            return -1;
        }
        if (state->remaining_length < headers_len) {
            DisplayFormatted(ctx, 8, "Application layer too short for headers (expected %u, got %u)\n", headers_len, state->remaining_length);
            return -1;
        }
        allocated_headers_ptr = allocate(headers_len + 1);
        memset(allocated_headers_ptr, 0, headers_len + 1);
        memcpy(allocated_headers_ptr, state->current_data_ptr, headers_len);
        DisplayFormatted(ctx, 8, "Headers: %s\n", (char *)allocated_headers_ptr);
        state->current_data_ptr += headers_len;
        state->remaining_length -= headers_len;
        deallocate(allocated_headers_ptr);
        allocated_headers_ptr = NULL; // Clear pointer after use

        if (state->remaining_length < 2) {
            DisplayFormatted(ctx, 8, "Application layer too short for webpage length\n");
            return -1;
        }
        uint16_t webpage_len = *(uint16_t *)state->current_data_ptr;
        state->current_data_ptr += 2;
        state->remaining_length -= 2;

        if (webpage_len != state->remaining_length) {
            DisplayFormatted(ctx, 8, "Invalid webpage length: expected %u, got %u\n", webpage_len, state->remaining_length);
            return -1;
        }
        // The webpage content itself is not printed, just its length checked.
        // If it were, it would consume remaining_length bytes.
        state->current_data_ptr += webpage_len;
        state->remaining_length -= webpage_len;

    } else {
        DisplayFormatted(ctx, 8, "Invalid Application Type: %u\n", app_type);
        return -1;
    }
    return 0;
}

// Function: AnalyzePacket
// param_1: PacketContext *ctx
// param_2: PacketState *state (Note: this param_2 is a pointer to a struct,
// which contains remaining_length at offset 4 and current_data_ptr at offset 8)
STATUS AnalyzePacket(PacketContext *ctx, PacketState *state) {
    int reject_count = 0;
    int accept_count = 0;

    // ProcessAllFilters takes PacketState as param_1 and PacketContext as param_2
    // So, `ProcessAllFilters(state, ctx, &reject_count, &accept_count);`
    ProcessAllFilters(state, ctx, &reject_count, &accept_count);

    StatsData *stats = ctx->stats_ptr;

    if (reject_count >= 1) {
        TransmitBytes("skip\n", 5);
        return -1; // 0xffffffff
    } else {
        if (stats->filter_option == 0 || accept_count != 0) {
            stats->packets_shown++;
            STATUS result = AnalyzePhysicalLayer(ctx, state);
            if (result == 0) {
                result = AnalyzeTransportLayer(ctx, state);
            }
            if (result == 0) {
                result = AnalyzeNetworkLayer(ctx, state);
            }
            if (result == 0) {
                result = AnalyzeApplicationLayer(ctx, state);
            }

            if (result == 0) {
                if ((ctx->display_flags & 0x20) != 0) { // Check for HEX_DUMP_FLAG
                    HexDump(state->current_data_ptr, state->remaining_length);
                    TransmitBytes(&DAT_000140f5, 1);
                }
                return 0; // Success
            }
            stats->malformed_packets++; // Increment malformed packets on analysis failure
            return -2; // 0xfffffffe (Malformed packet)
        } else {
            TransmitBytes("skip\n", 5);
            return -1; // 0xffffffff (Filtered out)
        }
    }
}