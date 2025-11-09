#include <stdio.h>   // For printf
#include <string.h>  // For strlen, strchr, strncpy, strcmp
#include <stdlib.h>  // For malloc, free, exit, perror

// Error codes
#define ERR_NULL_NODES_HEAD      -5
#define ERR_INVALID_ROUTE_FORMAT -2
#define ERR_CODE_NOT_FOUND       -4
#define ERR_ROUTE_NOT_FOUND      -7
#define SUCCESS                   0

// Define the Node and Connection structures based on memory offsets
// Node:
//   code[4] (0x0) - 3 characters + null terminator
//   connections (0x8) - pointer to Connection list head
//   next (0xc) - pointer to next Node in main list
typedef struct Connection {
    char code[4]; // Code of the connected node (e.g., "BBB")
    int val1;     // Value 1 for this connection
    int val2;     // Value 2 for this connection
    struct Connection *next; // Next connection from the same source node
} Connection;

typedef struct Node {
    char code[4]; // Code of this node (e.g., "AAA")
    Connection *connections; // Head of the list of connections originating from this node
    struct Node *next;       // Next node in the main list of all nodes
} Node;

// Helper function to create a new Connection node
Connection *create_connection(const char *code, int val1, int val2) {
    Connection *new_conn = (Connection *)malloc(sizeof(Connection));
    if (new_conn == NULL) {
        perror("Failed to allocate memory for connection");
        exit(EXIT_FAILURE);
    }
    strncpy(new_conn->code, code, 3);
    new_conn->code[3] = '\0'; // Ensure null termination
    new_conn->val1 = val1;
    new_conn->val2 = val2;
    new_conn->next = NULL;
    return new_conn;
}

// Helper function to create a new Node
Node *create_node(const char *code) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for node");
        exit(EXIT_FAILURE);
    }
    strncpy(new_node->code, code, 3);
    new_node->code[3] = '\0'; // Ensure null termination
    new_node->connections = NULL;
    new_node->next = NULL;
    return new_node;
}

// Helper function to add a connection to a node
void add_connection(Node *node, const char *target_code, int val1, int val2) {
    Connection *new_conn = create_connection(target_code, val1, val2);
    new_conn->next = node->connections;
    node->connections = new_conn;
}

// Helper function to find a Node by its code in the main list
Node *find_node(Node *nodes_head, const char *code) {
    Node *current_node = nodes_head;
    while (current_node != NULL) {
        if (strcmp(current_node->code, code) == 0) {
            return current_node;
        }
        current_node = current_node->next;
    }
    return NULL;
}

// Placeholder for check4Code
// Checks if a node with the given code exists in the main list of nodes.
int check4Code(Node *nodes_head, const char *code) {
    return find_node(nodes_head, code) != NULL;
}

// Function: findRoutes
// param_1 (nodes_head): Head of the linked list of nodes in the graph.
// param_2 (route_str): The route string in "CODE1/CODE2/CODE3" format.
int findRoutes(Node *nodes_head, const char *route_str) {
    if (nodes_head == NULL) {
        return ERR_NULL_NODES_HEAD;
    }

    size_t route_len = strlen(route_str);
    if (route_len == 0) {
        return ERR_INVALID_ROUTE_FORMAT;
    }

    // Find the first slash
    const char *first_slash_ptr = strchr(route_str, '/');
    if (first_slash_ptr == NULL) {
        return ERR_INVALID_ROUTE_FORMAT; // No slashes found
    }

    // Extract CODE1 (before the first slash)
    size_t len1 = first_slash_ptr - route_str;
    if (len1 == 0 || len1 > 3) { // Code must be 1-3 characters
        return ERR_INVALID_ROUTE_FORMAT;
    }
    char code1_str[4];
    strncpy(code1_str, route_str, len1);
    code1_str[len1] = '\0';

    // Find the second slash to determine the length of CODE2
    const char *second_slash_ptr = strchr(first_slash_ptr + 1, '/');
    if (second_slash_ptr == NULL) {
        // The original code returns -2 if no second slash (i.e., "A/B" format),
        // implying it only supports "A/B/C" format.
        return ERR_INVALID_ROUTE_FORMAT;
    }

    // Extract CODE2 (between first and second slash)
    size_t len2 = second_slash_ptr - (first_slash_ptr + 1);
    if (len2 == 0 || len2 > 3) { // Code must be 1-3 characters
        return ERR_INVALID_ROUTE_FORMAT;
    }
    char code2_str[4];
    strncpy(code2_str, first_slash_ptr + 1, len2);
    code2_str[len2] = '\0';

    // Check if CODE2 exists as a valid node in the graph
    if (!check4Code(nodes_head, code2_str)) {
        return ERR_CODE_NOT_FOUND;
    }

    // Extract CODE3 (after the second slash)
    const char *ptr_after_second_slash = second_slash_ptr + 1;
    size_t len3 = strlen(ptr_after_second_slash);
    if (len3 == 0 || len3 > 3) { // Code must be 1-3 characters
        return ERR_INVALID_ROUTE_FORMAT;
    }
    char code3_str[4];
    strncpy(code3_str, ptr_after_second_slash, len3);
    code3_str[len3] = '\0';

    // Check if CODE3 exists as a valid node in the graph
    if (!check4Code(nodes_head, code3_str)) {
        return ERR_CODE_NOT_FOUND;
    }

    // Find the starting node for traversal.
    // Based on the original snippet's logic, the traversal starts from the node
    // corresponding to CODE2 (local_38 in original).
    Node *node_start = find_node(nodes_head, code2_str);
    if (node_start == NULL) {
        // This case should ideally be caught by check4Code earlier, but as a safeguard.
        return ERR_CODE_NOT_FOUND;
    }

    int found_any_route = 0;

    // Iterate through connections originating from node_start (CODE2)
    for (Connection *conn_from_node_start = node_start->connections;
         conn_from_node_start != NULL;
         conn_from_node_start = conn_from_node_start->next) {

        int val1_conn1 = conn_from_node_start->val1;
        int val2_conn1 = conn_from_node_start->val2;

        if (strcmp(conn_from_node_start->code, code3_str) == 0) {
            // Found a direct route: CODE2 -> CODE3
            found_any_route = 1;
            // Original printf: "@s - @s: (@d, @d)\n",local_38,local_3c,local_30,local_34
            printf("%s - %s: (%d, %d)\n", code2_str, code3_str, val1_conn1, val2_conn1);
        } else {
            // Search for an indirect route: CODE2 -> Intermediate -> CODE3
            // Find the intermediate node in the main list of nodes
            Node *intermediate_node = find_node(nodes_head, conn_from_node_start->code);
            if (intermediate_node == NULL) {
                // If the intermediate connection points to a non-existent node,
                // the original code returns ERR_CODE_NOT_FOUND here.
                return ERR_CODE_NOT_FOUND;
            }

            // Iterate through connections originating from the intermediate node
            for (Connection *conn_from_intermediate = intermediate_node->connections;
                 conn_from_intermediate != NULL;
                 conn_from_intermediate = conn_from_intermediate->next) {

                if (strcmp(conn_from_intermediate->code, code3_str) == 0) {
                    // Found an indirect route: CODE2 -> Intermediate -> CODE3
                    found_any_route = 1;
                    // Original printf: "@s - @s - @s: (@d, @d)\n",local_38,local_20,local_3c, ...
                    printf("%s - %s - %s: (%d, %d)\n", code2_str, conn_from_node_start->code, code3_str,
                           val1_conn1 + conn_from_intermediate->val1,
                           val2_conn1 + conn_from_intermediate->val2);
                    break; // Original code breaks after finding the first 2-hop route
                }
            }
        }
    }

    printf("\n"); // Original code prints a newline at the end

    if (!found_any_route) {
        return ERR_ROUTE_NOT_FOUND;
    }

    return SUCCESS;
}

// Function to free all allocated memory for the graph
void free_graph(Node *nodes_head) {
    Node *current_node = nodes_head;
    while (current_node != NULL) {
        Node *next_node = current_node->next;
        Connection *current_conn = current_node->connections;
        while (current_conn != NULL) {
            Connection *next_conn = current_conn->next;
            free(current_conn);
            current_conn = next_conn;
        }
        free(current_node);
        current_node = next_node;
    }
}

int main() {
    Node *nodes_head = NULL;

    // Create nodes
    Node *nodeA = create_node("AAA");
    Node *nodeB = create_node("BBB");
    Node *nodeC = create_node("CCC");
    Node *nodeD = create_node("DDD");
    Node *nodeE = create_node("EEE"); // E exists but has no connections

    // Link nodes into the main list (order doesn't matter for find_node)
    nodes_head = nodeA;
    nodeA->next = nodeB;
    nodeB->next = nodeC;
    nodeC->next = nodeD;
    nodeD->next = nodeE;

    // Add connections to nodes
    // Connections from nodeA
    add_connection(nodeA, "BBB", 10, 20);
    add_connection(nodeA, "CCC", 5, 15);

    // Connections from nodeB
    add_connection(nodeB, "CCC", 30, 40);
    add_connection(nodeB, "DDD", 1, 2);

    // Connections from nodeC
    add_connection(nodeC, "DDD", 50, 60);

    // Node D and E have no outgoing connections

    printf("--- Testing findRoutes ---\n");
    int result;

    printf("\nTest 1: Route XXX/AAA/CCC (Direct A->C)\n");
    // Start node for traversal is "AAA" (code2_str), target is "CCC" (code3_str)
    result = findRoutes(nodes_head, "XXX/AAA/CCC");
    printf("Result: %d (Expected: AAA - CCC: (5, 15), %d)\n", result, SUCCESS);

    printf("\nTest 2: Route XXX/AAA/DDD (A->B->D and A->C->D)\n");
    // Start node for traversal is "AAA", target is "DDD"
    result = findRoutes(nodes_head, "XXX/AAA/DDD");
    printf("Result: %d (Expected: AAA - BBB - DDD: (11, 22), AAA - CCC - DDD: (55, 75), %d)\n", result, SUCCESS);

    printf("\nTest 3: Route XXX/BBB/DDD (Direct B->D)\n");
    // Start node for traversal is "BBB", target is "DDD"
    result = findRoutes(nodes_head, "XXX/BBB/DDD");
    printf("Result: %d (Expected: BBB - DDD: (1, 2), %d)\n", result, SUCCESS);

    printf("\nTest 4: Route XXX/CCC/EEE (C->D->E - E exists but D has no connection to E)\n");
    // Start node for traversal is "CCC", target is "EEE"
    result = findRoutes(nodes_head, "XXX/CCC/EEE");
    printf("Result: %d (Expected: %d)\n", result, ERR_ROUTE_NOT_FOUND);

    printf("\nTest 5: Route XXX/AAA/EEE (A->C->?->E, no path)\n");
    // Start node for traversal is "AAA", target is "EEE"
    result = findRoutes(nodes_head, "XXX/AAA/EEE");
    printf("Result: %d (Expected: %d)\n", result, ERR_ROUTE_NOT_FOUND);

    printf("\nTest 6: Invalid format - missing third code (X/B/C)\n");
    result = findRoutes(nodes_head, "X/B/C");
    printf("Result: %d (Expected: %d)\n", result, ERR_INVALID_ROUTE_FORMAT);

    printf("\nTest 7: Invalid format - only 1 slash (X/Y)\n");
    result = findRoutes(nodes_head, "X/Y");
    printf("Result: %d (Expected: %d)\n", result, ERR_INVALID_ROUTE_FORMAT);

    printf("\nTest 8: Invalid format - no slashes (ABC)\n");
    result = findRoutes(nodes_head, "ABC");
    printf("Result: %d (Expected: %d)\n", result, ERR_INVALID_ROUTE_FORMAT);

    printf("\nTest 9: Invalid node - code2_str 'XXX' not found\n");
    result = findRoutes(nodes_head, "XXX/XXX/CCC");
    printf("Result: %d (Expected: %d)\n", result, ERR_CODE_NOT_FOUND);

    printf("\nTest 10: Invalid node - code3_str 'XXX' not found\n");
    result = findRoutes(nodes_head, "AAA/BBB/XXX");
    printf("Result: %d (Expected: %d)\n", result, ERR_CODE_NOT_FOUND);

    printf("\nTest 11: Short codes A/B/C (A->B->C)\n");
    Node *node_sA = create_node("A");
    Node *node_sB = create_node("B");
    Node *node_sC = create_node("C");
    Node *nodes_head_short = node_sA;
    node_sA->next = node_sB;
    node_sB->next = node_sC;
    add_connection(node_sA, "B", 1, 1);
    add_connection(node_sB, "C", 2, 2);
    result = findRoutes(nodes_head_short, "X/A/C"); // Start node for traversal is "A"
    printf("Result: %d (Expected: A - B - C: (3, 3), %d)\n", result, SUCCESS);
    free_graph(nodes_head_short);

    printf("\nTest 12: NULL nodes_head\n");
    result = findRoutes(NULL, "XXX/AAA/BBB");
    printf("Result: %d (Expected: %d)\n", result, ERR_NULL_NODES_HEAD);

    // Free all allocated memory
    free_graph(nodes_head);

    return 0;
}