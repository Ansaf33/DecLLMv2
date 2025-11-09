#include <stdio.h>    // For FILE, fprintf, stderr
#include <stdlib.h>   // For malloc, free, calloc
#include <string.h>   // For memcpy, memset, memcmp
#include <stdint.h>   // For uint32_t, uint8_t, uintptr_t
#include <stddef.h>   // For size_t

// --- Type Aliases ---
// Based on typical decompiler output and common C types
typedef uint32_t undefined4;
typedef uint8_t byte;
typedef unsigned int uint; // Assuming uint is unsigned int

// --- Dummy Declarations for External/Custom Functions ---
// These functions are not provided in the snippet and need to be declared
// to make the code compilable. Their actual implementations are unknown.
// The parameter types and return types are inferred from their usage.

// xcalloc is used like calloc but sometimes with extra arguments.
// For simplicity, we'll define a wrapper that just calls standard calloc,
// ignoring extra arguments if present.
void* xcalloc(size_t nmemb, size_t size, ...) {
    return calloc(nmemb, size);
}

// Trie related
void FreeTrie(void *trie_root) { (void)trie_root; /* Dummy Implementation */ }
void AllocateAndInitializeTrieRoot(void **trie_root_ptr) { *trie_root_ptr = xcalloc(1, 1); /* Dummy Implementation */ }
// InsertIntoTrie's last argument's purpose is inferred as a value associated with the data.
void InsertIntoTrie(void *trie_root, const char *data, size_t len, uintptr_t value) { (void)trie_root; (void)data; (void)len; (void)value; /* Dummy Implementation */ }

// Search Machine related
void FreeSearchMachine(void *search_machine) { (void)search_machine; /* Dummy Implementation */ }
void InitializeSearchMachine(void *search_machine_ptr, void *trie_root) { (void)search_machine_ptr; (void)trie_root; /* Dummy Implementation */ }

// Queue related
void InitializeQueue(void *queue) { (void)queue; /* Dummy Implementation */ }
void* Peek(void *queue) { (void)queue; return NULL; /* Dummy Implementation */ }
void* Dequeue(void *queue) { (void)queue; return NULL; /* Dummy Implementation */ }
void Enqueue(void *queue, void *item) { (void)queue; (void)item; /* Dummy Implementation */ }
void FreeQueue(void *queue) { (void)queue; /* Dummy Implementation */ }

// List related (for search results)
void* UniqExtendList(void *list, void *item) { (void)list; (void)item; return NULL; /* Dummy Implementation */ }
void AllocateAndInitializeListHead(void **list_head, void *item) { *list_head = xcalloc(1, 1); (void)item; /* Dummy Implementation */ }
void UniqAppendToList(void *list_head, void *item) { (void)list_head; (void)item; /* Dummy Implementation */ }

// FindMatches: returns a pointer to an array of MatchResult structures.
// The structure returned by FindMatches is complex, likely an array of structs
// or a linked list. The original code accesses `local_1c + local_10 * 8 + 4`
// and then iterates a list `local_14 = *(undefined4 **)(...)`.
// This implies each match result entry is 8 bytes, and the second 4-byte field
// (at offset +4) points to a list of matched patterns.
typedef struct __attribute__((packed)) MatchResultEntry {
    uint32_t offset;          // Placeholder for the first 4 bytes
    void *match_list_head;    // Pointer to the list of matched patterns (second 4 bytes on 32-bit, 8 on 64-bit)
} MatchResultEntry;

// FindMatches' return value is a pointer to an array of MatchResultEntry.
void* FindMatches(void *search_machine, const char *text, size_t text_len, uint32_t *num_matches) {
    (void)search_machine; (void)text; (void)text_len;
    *num_matches = 0;
    return NULL; /* Dummy Implementation */
}

// --- Struct Definitions ---
// The original code's pointer arithmetic and array indexing (`param_1[N]`, `((int)param_1 + offset)`)
// strongly suggest a 32-bit compilation target where pointers and `size_t` are 4 bytes.
// To ensure correct byte-level access on a 64-bit system, we use `__attribute__((packed))`
// and store addresses/lengths as `uint32_t` where they were 4-byte fields in the original.
// This might lead to issues if actual addresses or lengths exceed 2^32-1.
typedef struct __attribute__((packed)) Signature {
    int severity;              // Offset 0 (4 bytes)
    uint32_t id_data_addr;     // Offset 4 (4 bytes, represents char*)
    uint32_t id_len;           // Offset 8 (4 bytes)
    uint32_t signature_data_addr; // Offset 12 (4 bytes, represents char*)
    uint32_t signature_data_len; // Offset 16 (4 bytes, represents size_t)
} Signature;

#define MAX_SIGNATURES 0x1000 // 4096 signatures

typedef struct SignatureDatabase {
    uint32_t count;
    Signature *signatures[MAX_SIGNATURES]; // Array of pointers to Signature
    void *trie_root;
    void *search_machine;
} SignatureDatabase;

// Define a minimal struct for SearchMachine internal access in BuildSignatureDatabaseSearchMachine
// This is a guess based on array access patterns (puVar1[1], puVar1[2], puVar1[3])
// and the assumption of 32-bit pointers for internal structures.
typedef struct __attribute__((packed)) SearchMachineInternal {
    void *root_node_ptr;       // puVar1[0] (guessed)
    uint32_t output_lists_addr; // puVar1[1] as address of void*[]
    uint32_t failure_links_addr; // puVar1[2] as address of TrieNode*[]
    uint32_t transitions_addr;   // puVar1[3] as address of TrieNode*** (or similar)
} SearchMachineInternal;

// A minimal TrieNode struct for `node_id` access at offset 0x408
// (This is a placeholder, actual TrieNode structure is unknown)
typedef struct __attribute__((packed)) TrieNode {
    char data[0x408]; // Placeholder for data before node_id
    int node_id;      // Accessed via `*(int *)(iVar6 + 0x408)`
} TrieNode;


// Function: InitializeSignature
undefined4 InitializeSignature(Signature *sig, int severity, void *sig_data, size_t sig_data_len, void *id_data, uint id_len) {
    if (!sig || !sig_data || !id_data) {
        return 0xffffffff;
    }
    if (id_len >= 0x40) { // 0x40 is 64
        return 0xffffffff;
    }
    if (severity < 1 || 5 < severity) {
        return 0xffffffff;
    }

    sig->severity = severity;
    
    // Allocate and copy signature data
    char *allocated_sig_data = xcalloc(sig_data_len + 1, 1);
    if (!allocated_sig_data) return 0xffffffff;
    memcpy(allocated_sig_data, sig_data, sig_data_len);
    sig->signature_data_addr = (uint32_t)(uintptr_t)allocated_sig_data;
    sig->signature_data_len = (uint32_t)sig_data_len;
    fprintf(stderr, "(%s)\n", (char*)(uintptr_t)sig->signature_data_addr);

    // Allocate and copy ID data
    char *allocated_id_data = xcalloc(id_len + 1, 1);
    if (!allocated_id_data) {
        free(allocated_sig_data); // Clean up already allocated memory
        return 0xffffffff;
    }
    memcpy(allocated_id_data, id_data, id_len);
    sig->id_data_addr = (uint32_t)(uintptr_t)allocated_id_data;
    sig->id_len = id_len;

    return 0;
}

// Function: FreeSignature
void FreeSignature(void *param_1) {
    if (param_1) {
        Signature *sig = (Signature *)param_1;
        // Accessing members by explicit cast to uintptr_t to match decompiler's byte offsets
        // This assumes the packed struct layout where id_data_addr is at offset 4 and signature_data_addr is at offset 12.
        if (sig->signature_data_addr != 0) {
            free((void *)(uintptr_t)sig->signature_data_addr);
            sig->signature_data_addr = 0;
        }
        if (sig->id_data_addr != 0) {
            free((void *)(uintptr_t)sig->id_data_addr);
            sig->id_data_addr = 0;
        }
        free(sig);
    }
}

// Function: BytesToUnsigned
int BytesToUnsigned(uintptr_t param_1, uint param_2) {
    int hash_val = 0x1505;
    for (uint i = 0; i < param_2; ++i) {
        hash_val = ((uint8_t *)(uintptr_t)param_1)[i] + hash_val * 0x21;
    }
    return hash_val;
}

// Function: SeverityString
char * SeverityString(undefined4 severity) {
    switch(severity) {
        case 1:  return "LOW";
        case 2:  return "GUARDED";
        case 3:  return "ELEVATED";
        case 4:  return "HIGH";
        case 5:  return "SEVERE";
        default: return "UNKNOWN";
    }
}

// Function: PrintSignature
void PrintSignature(FILE *output_file, Signature *sig) {
    fprintf(output_file, "%s - %s - %x\n",
            (char*)(uintptr_t)sig->id_data_addr,
            SeverityString(sig->severity),
            BytesToUnsigned(sig->signature_data_addr, sig->signature_data_len));
}

// Function: InitializeSignatureDatabase
undefined4 InitializeSignatureDatabase(SignatureDatabase *db) {
    if (!db) {
        return 0xffffffff;
    }
    db->count = 0;
    memset(db->signatures, 0, sizeof(db->signatures));
    db->trie_root = NULL;
    db->search_machine = xcalloc(1, 0x10); // xcalloc with 1, 0x10, and two more ignored args
    if (!db->search_machine) {
        return 0xffffffff; // Allocation failed
    }
    return 0;
}

// Function: FreeSignatureDatabase
void FreeSignatureDatabase(SignatureDatabase *db) {
    if (db) {
        for (uint32_t i = 0; i < db->count; ++i) {
            if (db->signatures[i]) {
                FreeSignature(db->signatures[i]);
                db->signatures[i] = NULL;
            }
        }
        if (db->trie_root) {
            FreeTrie(db->trie_root);
            db->trie_root = NULL;
        }
        if (db->search_machine) {
            FreeSearchMachine(db->search_machine);
            db->search_machine = NULL; // Added for consistency
        }
        free(db);
    }
}

// Function: AddSignatureToSignatureDatabase
undefined4 AddSignatureToSignatureDatabase(SignatureDatabase *db, Signature *sig) {
    if (db->count == MAX_SIGNATURES) {
        return 0xffffffff;
    }
    db->signatures[db->count] = sig;
    db->count++;
    return 0;
}

// Function: BuildSignatureDatabaseTrie
undefined4 BuildSignatureDatabaseTrie(SignatureDatabase *db) {
    if (!db) {
        return 0xffffffff;
    }
    AllocateAndInitializeTrieRoot(&db->trie_root);
    if (!db->trie_root) {
        return 0xffffffff; // Allocation failed
    }
    for (uint32_t i = 0; i < db->count; ++i) {
        InsertIntoTrie(db->trie_root,
                       (const char*)(uintptr_t)db->signatures[i]->signature_data_addr,
                       db->signatures[i]->signature_data_len,
                       (uintptr_t)db->signatures[i]); // Pass signature pointer as value
    }
    return 0;
}

// Function: BuildSignatureDatabaseSearchMachine
undefined4 BuildSignatureDatabaseSearchMachine(SignatureDatabase *db) {
    if (!db) {
        return 0xffffffff;
    }

    if (!db->trie_root && BuildSignatureDatabaseTrie(db) != 0) {
        return 0xffffffff;
    }

    InitializeSearchMachine(db->search_machine, db->trie_root);
    SearchMachineInternal *sm_internal = (SearchMachineInternal *)db->search_machine;

    void *queue = xcalloc(1, 0x10);
    if (!queue) return 0xffffffff; // Allocation failed
    InitializeQueue(queue);

    // Assuming transitions, failure_links, output_lists are arrays of 32-bit addresses
    uint32_t *transitions = (uint32_t *)(uintptr_t)sm_internal->transitions_addr;
    uint32_t *failure_links = (uint32_t *)(uintptr_t)sm_internal->failure_links_addr;
    uint32_t *output_lists = (uint32_t *)(uintptr_t)sm_internal->output_lists_addr;

    for (uint32_t c = 0; c < 0x100; ++c) {
        TrieNode *next_node = (TrieNode *)(uintptr_t)transitions[c]; // Assuming transitions[c] is a TrieNode*
        if (next_node && ((TrieNode*)(uintptr_t)failure_links[next_node->node_id]) != NULL) { // Check failure_links[node_id]
            failure_links[next_node->node_id] = (uint32_t)(uintptr_t)sm_internal->root_node_ptr; // Assign root node
            Enqueue(queue, next_node);
        }
    }

    void *current_node_ptr;
    while ((current_node_ptr = Peek(queue)) != NULL) {
        Dequeue(queue);
        TrieNode *current_node = (TrieNode *)current_node_ptr;

        for (uint32_t c = 0; c < 0x100; ++c) {
            TrieNode *child_node = (TrieNode *)(uintptr_t)transitions[current_node->node_id * 0x100 + c]; // Access transition table for current_node
            if (child_node) {
                Enqueue(queue, child_node);
                
                TrieNode *fail_node = (TrieNode *)(uintptr_t)failure_links[current_node->node_id];
                while (((TrieNode *)(uintptr_t)transitions[fail_node->node_id * 0x100 + c]) == NULL) {
                    fail_node = (TrieNode *)(uintptr_t)failure_links[fail_node->node_id];
                }
                failure_links[child_node->node_id] = transitions[fail_node->node_id * 0x100 + c]; // Set failure link
                
                output_lists[child_node->node_id] = (uint32_t)(uintptr_t)UniqExtendList(
                    (void*)(uintptr_t)output_lists[child_node->node_id],
                    (void*)(uintptr_t)output_lists[((TrieNode*)(uintptr_t)failure_links[child_node->node_id])->node_id]
                );
            }
        }
    }

    FreeQueue(queue);
    return 0;
}

// Function: SearchSignatureDatabase
int SearchSignatureDatabase(SignatureDatabase *db, const char *text, size_t text_len) {
    if (!db || !text || !text_len) {
        return 0;
    }
    if (!db->search_machine) {
        return 0;
    }

    uint32_t num_matches = 0;
    void *match_results_list = FindMatches(db->search_machine, text, text_len, &num_matches);

    int result_list_head = 0; // Initialize as NULL for list head
    for (uint32_t i = 0; i < num_matches; ++i) {
        MatchResultEntry *current_match_entry = &((MatchResultEntry*)match_results_list)[i];
        for (void *pattern_list_node = current_match_entry->match_list_head; pattern_list_node != NULL; pattern_list_node = *(void**)pattern_list_node) {
            // The original code iterates over all signatures in the database for each matched pattern
            // and compares the signature data. This implies pattern_list_node contains information
            // about the matched pattern itself, not necessarily the signature in the database.
            // Assuming pattern_list_node[1] holds the data pointer for comparison.
            // This is very ambiguous in the original code. Let's assume pattern_list_node points to a struct
            // with a data pointer at offset 4 (as indicated by `local_14[1]`).
            char *matched_pattern_data = ((char**)pattern_list_node)[1]; // Assuming pattern_list_node is a pointer to an array of pointers, and the second element is the data.

            for (uint32_t j = 0; j < db->count; ++j) {
                if (memcmp((void *)(uintptr_t)db->signatures[j]->signature_data_addr, matched_pattern_data, db->signatures[j]->signature_data_len) == 0) {
                    if (result_list_head == 0) {
                        AllocateAndInitializeListHead((void**)&result_list_head, db->signatures[j]);
                    } else {
                        UniqAppendToList((void*)(uintptr_t)result_list_head, db->signatures[j]);
                    }
                }
            }
        }
    }
    // Free the match_results_list obtained from FindMatches if it's dynamically allocated.
    // The snippet doesn't show it being freed, but it should be.
    // For this task, we omit adding free calls for dummy functions.
    return result_list_head;
}