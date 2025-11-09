#include <stdio.h>    // For printf
#include <stdlib.h>   // For malloc, free, exit
#include <string.h>   // For memcpy
#include <stdbool.h>  // For bool

// Custom types based on original snippet's inferred types
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

// Function pointer type for packet handlers
typedef void (*PacketHandlerFunc)(byte *, uint);

// Forward declarations for handlers
void HandleBroadcastPacket(byte *param_1, uint param_2);
void HandleChannelPacket(byte *param_1, uint param_2);
void HandlePrivatePacket(byte *param_1, uint param_2);
void HandleConnectPacket(byte *param_1, int param_2); // Note: param_2 is int in original snippet
void HandleDisconnectPacket(byte *param_1, int param_2); // Note: param_2 is int in original snippet

// Structure for packet handlers (derived from init_packet_handler)
// The original code implies an 8-byte structure per handler entry:
// 1 byte for packet_type, 3 bytes padding, 4 bytes for handler_func (assuming 32-bit pointers).
// This structure is critical for matching the memory layout of the original code.
struct PacketHandlerEntry {
    byte packet_type;
    char _padding[3]; // Padding for 32-bit alignment of handler_func and 8-byte total size
    PacketHandlerFunc handler_func;
};

// Structure for received packet data (derived from add_new_packet)
// Total size 0x40 (64 bytes).
// Data is copied to the beginning (offset 0).
// Metadata is stored at the end of the allocated block.
struct PacketDataNode {
    byte data[48]; // Buffer for packet data
    uint data_len;
    struct PacketDataNode *next;
    PacketHandlerFunc handler_func;
    byte packet_type;
};

// Global variables
// Initialized to 0/NULL by default for static storage duration
uint g_packetHandlerCount;
struct PacketHandlerEntry *g_packetHandlers = NULL;
struct PacketDataNode *g_packetData = NULL;
uint g_receivedPacketCount = 0;
uint g_totalBytesReceived = 0;
uint g_invalidPacketCount = 0;

// Function: init_packet_handler
void init_packet_handler(void) {
    g_packetHandlerCount = 5;
    g_packetHandlers = (struct PacketHandlerEntry *)malloc(g_packetHandlerCount * sizeof(struct PacketHandlerEntry));
    if (g_packetHandlers == NULL) {
        perror("Failed to allocate memory for packet handlers");
        exit(EXIT_FAILURE);
    }

    g_packetHandlers[0].packet_type = 0;
    g_packetHandlers[0].handler_func = HandleBroadcastPacket;

    g_packetHandlers[1].packet_type = 1;
    g_packetHandlers[1].handler_func = HandleChannelPacket;

    g_packetHandlers[2].packet_type = 2;
    g_packetHandlers[2].handler_func = HandlePrivatePacket;

    g_packetHandlers[3].packet_type = 3;
    g_packetHandlers[3].handler_func = HandleConnectPacket;

    g_packetHandlers[4].packet_type = 4;
    g_packetHandlers[4].handler_func = HandleDisconnectPacket;
}

// Function: destroy_packet_handler
void destroy_packet_handler(void) {
    if (g_packetHandlers != NULL) {
        free(g_packetHandlers);
        g_packetHandlers = NULL;
    }

    struct PacketDataNode *current_packet = g_packetData;
    while (current_packet != NULL) {
        struct PacketDataNode *next_packet = current_packet->next;
        free(current_packet);
        current_packet = next_packet;
    }
    g_packetData = NULL;
}

// Function: simple_checksum16
short simple_checksum16(byte *param_1, short param_2) {
    short checksum = -0x4053; // Initial checksum value (0xBFAD)
    short length = param_2;

    while (length != 0) {
        checksum += (ushort)*param_1; // Add unsigned byte value
        param_1++;
        length--;
    }
    return checksum;
}

// Function: add_new_packet
void add_new_packet(byte param_1, PacketHandlerFunc param_2, void *param_3, uint param_4) {
    struct PacketDataNode *newNode = (struct PacketDataNode *)malloc(sizeof(struct PacketDataNode));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new packet data node");
        exit(EXIT_FAILURE);
    }

    newNode->packet_type = param_1;
    newNode->handler_func = param_2;
    newNode->next = NULL;

    // The original code used `param_4 & 0xff` for data_len, implying length is a byte.
    // Since param_4 comes from `param_2 - 1` (where param_2 is byte), it will be <= 255.
    // So, `& 0xff` is redundant here for `uint` assignment, using `param_4` directly.
    // `memcpy` also uses `param_4` as length.
    newNode->data_len = param_4;
    memcpy(newNode->data, param_3, param_4);

    // Link new node to the front of the list
    if (g_packetData != NULL) {
        newNode->next = g_packetData;
    }
    g_packetData = newNode;
}

// Function: receive_packet
void receive_packet(char *param_1, byte param_2, short param_3) {
    if (param_1 != NULL && param_2 != 0 && simple_checksum16((byte *)param_1, param_2) == param_3) {
        bool handler_found = false;
        byte packet_type = (byte)*param_1;

        for (uint i = 0; i < g_packetHandlerCount; i++) {
            if (packet_type == g_packetHandlers[i].packet_type) {
                handler_found = true;
                add_new_packet(packet_type, g_packetHandlers[i].handler_func, param_1 + 1, param_2 - 1);
                g_receivedPacketCount++;
                g_totalBytesReceived += param_2;
                break; // Found handler, no need to check others
            }
        }
        if (!handler_found) {
            g_invalidPacketCount++;
        }
    }
}

// Function: display_packets
void display_packets(void) {
    printf("Total %u bytes received and %u invalid packets.\n", g_totalBytesReceived, g_invalidPacketCount);
    printf("Displaying %u received packets:\n", g_receivedPacketCount);

    uint packet_idx = 0;
    struct PacketDataNode *current_node = g_packetData;

    while (current_node != NULL) {
        printf("Displaying packet %u type %u:\n", packet_idx, (uint)current_node->packet_type);
        current_node->handler_func(current_node->data, current_node->data_len);
        current_node = current_node->next;
        packet_idx++;
    }
}

// Function: HandleBroadcastPacket
void HandleBroadcastPacket(byte *param_1, uint param_2) {
    char username[9];  // Max 8 chars + null terminator
    char message[256]; // Max 255 chars + null terminator

    if (param_1 == NULL) {
        printf("[BROADCAST]No data\n");
    } else if (param_2 == 0) {
        printf("[BROADCAST]Missing length\n");
    } else {
        byte username_len = *param_1; // First byte is username length
        if (username_len >= sizeof(username)) { // Check against buffer capacity (8 actual chars)
            printf("[BROADCAST]Username length was too large\n");
        } else if (param_2 < username_len + 2) { // Need at least username_len + 1 (msg_len) + 1 (username_start_byte)
            printf("[BROADCAST]Invalid message length\n");
        } else {
            byte message_len = param_1[username_len + 1]; // Message length is after username length and username data
            if (param_2 != (uint)message_len + (uint)username_len + 2) {
                printf("[BROADCAST]Message length did not match packet length\n");
            } else {
                memcpy(username, param_1 + 1, username_len);
                username[username_len] = '\0';
                memcpy(message, param_1 + username_len + 2, message_len);
                message[message_len] = '\0';
                printf("[BROADCAST]From %s::%s\n", username, message);
            }
        }
    }
}

// Function: HandleChannelPacket
void HandleChannelPacket(byte *param_1, uint param_2) {
    char username[9];
    char message[256];

    if (param_1 == NULL) {
        printf("[CHANNEL]No data\n");
    } else if (param_2 < 2) { // Need at least username_len + channel_id
        printf("[CHANNEL]Invalid length\n");
    } else {
        byte username_len = *param_1;
        if (username_len >= sizeof(username)) {
            printf("[CHANNEL]Username length was too large\n");
        } else if (param_2 < username_len + 3) { // Need username_len + channel_id + message_len + 1 (username_start)
            printf("[CHANNEL]Invalid message length\n");
        } else {
            byte channel_id = param_1[username_len + 1];
            byte message_len = param_1[username_len + 2];
            if (param_2 != (uint)message_len + (uint)username_len + 3) {
                printf("[CHANNEL]Message length did not match packet length\n");
            } else {
                memcpy(username, param_1 + 1, username_len);
                username[username_len] = '\0';
                memcpy(message, param_1 + username_len + 3, message_len);
                message[message_len] = '\0';
                printf("[CHANNEL %u]Message from %s::%s\n", (uint)channel_id, username, message);
            }
        }
    }
}

// Function: HandlePrivatePacket
void HandlePrivatePacket(byte *param_1, uint param_2) {
    char sender_username[9];
    char receiver_username[9];
    char message[256];

    if (param_1 == NULL) {
        printf("[PRIVATE MESSAGE]No data\n");
    } else if (param_2 < 2) { // Need at least sender_len + receiver_len
        printf("[PRIVATE MESSAGE]Invalid length\n");
    } else {
        byte sender_username_len = *param_1;
        if (sender_username_len >= sizeof(sender_username)) {
            printf("[PRIVATE MESSAGE]Username length was too large\n");
        } else if (param_2 < sender_username_len + 3) { // Need sender_len + receiver_len + message_len + 1 (sender_start)
            printf("[PRIVATE MESSAGE]Message length did not match packet length\n");
        } else {
            byte receiver_username_len = param_1[sender_username_len + 1];
            if (receiver_username_len >= sizeof(receiver_username)) {
                printf("[PRIVATE MESSAGE]Username length was too large\n");
            } else if (param_2 < (uint)receiver_username_len + (uint)sender_username_len + 3) {
                printf("[PRIVATE MESSAGE]Message length did not match packet length\n");
            } else {
                byte message_len = param_1[(uint)sender_username_len + (uint)receiver_username_len + 2];
                if (param_2 != (uint)message_len + (uint)sender_username_len + (uint)receiver_username_len + 3) {
                    printf("[PRIVATE MESSAGE]Message length did not match packet length\n");
                } else {
                    memcpy(sender_username, param_1 + 1, sender_username_len);
                    sender_username[sender_username_len] = '\0';
                    memcpy(receiver_username, param_1 + sender_username_len + 2, receiver_username_len);
                    receiver_username[receiver_username_len] = '\0';
                    memcpy(message, param_1 + sender_username_len + receiver_username_len + 3, message_len);
                    message[message_len] = '\0';
                    printf("[PRIVATE MESSAGE]%s to %s::%s\n", sender_username, receiver_username, message);
                }
            }
        }
    }
}

// Function: HandleConnectPacket
void HandleConnectPacket(byte *param_1, int param_2) { // param_2 is int here
    char username[8]; // Max 7 chars + null terminator

    if (param_1 == NULL) {
        printf("[CONNECT MESSAGE]No data\n");
    } else if (param_2 == 0) {
        printf("[CONNECT MESSAGE]Invalid length\n");
    } else {
        byte username_len = *param_1;
        if (username_len >= sizeof(username)) { // Check against buffer size (7 actual chars)
            printf("[CONNECT MESSAGE]Username length was too large\n");
        } else if (param_2 != username_len + 1) { // Total length should be username_len + 1 (for length byte itself)
            printf("[CONNECT MESSAGE]Message length did not match packet length\n");
        } else {
            memcpy(username, param_1 + 1, username_len);
            username[username_len] = '\0';
            printf("[CONNECT MESSAGE]%s connected\n", username);
        }
    }
}

// Function: HandleDisconnectPacket
void HandleDisconnectPacket(byte *param_1, int param_2) { // param_2 is int here
    char username[8]; // Max 7 chars + null terminator

    if (param_1 == NULL) {
        printf("[DISCONNECT MESSAGE]No data\n");
    } else if (param_2 == 0) {
        printf("[DISCONNECT MESSAGE]Invalid length\n");
    } else {
        byte username_len = *param_1;
        if (username_len >= sizeof(username)) { // Check against buffer size (7 actual chars)
            printf("[DISCONNECT MESSAGE]Username length was too large\n");
        } else if (param_2 != username_len + 1) { // Total length should be username_len + 1 (for length byte itself)
            printf("[DISCONNECT MESSAGE]Message length did not match packet length\n");
        } else {
            memcpy(username, param_1 + 1, username_len);
            username[username_len] = '\0';
            printf("[DISCONNECT MESSAGE]%s disconnected\n", username);
        }
    }
}

// Main function for compilation and basic testing
int main() {
    init_packet_handler();

    // Simulate receiving packets
    byte broadcast_packet[] = {0x04, 'U', 's', 'e', 'r', 0x07, 'H', 'e', 'l', 'l', 'o', '!', '!'}; // Type 0, User: Hello!!
    short broadcast_checksum = simple_checksum16(broadcast_packet, sizeof(broadcast_packet));
    receive_packet((char*)broadcast_packet, sizeof(broadcast_packet), broadcast_checksum);

    byte channel_packet[] = {0x04, 'A', 'l', 'i', 'c', 0x01, 0x05, 'W', 'o', 'r', 'l', 'd'}; // Type 1, Alice, channel 1: World
    short channel_checksum = simple_checksum16(channel_packet, sizeof(channel_packet));
    receive_packet((char*)channel_packet, sizeof(channel_packet), channel_checksum);

    byte private_packet[] = {0x04, 'B', 'o', 'b', 0x05, 'A', 'l', 'i', 'c', 'e', 0x06, 'H', 'i', 'A', 'l', 'i', 'c', 'e'}; // Type 2, Bob to Alice: HiAlice
    short private_checksum = simple_checksum16(private_packet, sizeof(private_packet));
    receive_packet((char*)private_packet, sizeof(private_packet), private_checksum);

    byte connect_packet[] = {0x03, 'J', 'o', 'e'}; // Type 3, Joe connected
    short connect_checksum = simple_checksum16(connect_packet, sizeof(connect_packet));
    receive_packet((char*)connect_packet, sizeof(connect_packet), connect_checksum);

    byte disconnect_packet[] = {0x03, 'J', 'o', 'e'}; // Type 4, Joe disconnected
    short disconnect_checksum = simple_checksum16(disconnect_packet, sizeof(disconnect_packet));
    receive_packet((char*)disconnect_packet, sizeof(disconnect_packet), disconnect_checksum);

    // Simulate an invalid packet (wrong checksum)
    byte invalid_packet[] = {0x05, 'I', 'n', 'v', 'a', 'l', 0x01, 'X'};
    receive_packet((char*)invalid_packet, sizeof(invalid_packet), 0); // Wrong checksum

    // Simulate an unknown packet type (type 5)
    byte unknown_type_packet[] = {0x05, 'U', 'n', 'k', 'n', 'o', 'w', 0x01, 'Y'}; // Type 5, unknown handler
    short unknown_checksum = simple_checksum16(unknown_type_packet, sizeof(unknown_type_packet));
    receive_packet((char*)unknown_type_packet, sizeof(unknown_type_packet), unknown_checksum);

    display_packets();

    destroy_packet_handler();
    return 0;
}