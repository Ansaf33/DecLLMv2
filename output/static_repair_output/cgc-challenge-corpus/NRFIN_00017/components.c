#include <stdlib.h> // For malloc, exit
#include <stdio.h>  // For NULL, stderr, fprintf

// Custom type definitions based on usage in the snippet
typedef unsigned char byte;
typedef unsigned char undefined;
typedef unsigned int undefined4; // Used for pointers and integers
typedef unsigned int uint;

// Global variables (simulated from original snippet)
static int next_receptacle_id = 0;
static int next_outlet_id = 0;
static int next_n_way_splitter_id = 0;
static int next_light_string_id = 0;

static long double DAT_0001401c = 15.0L; // Example value, could be from a configuration
static int DAT_00014020 = 50;  // Example wattage for 5 lights
static int DAT_00014024 = 60;  // Example wattage for 6 lights
static int DAT_00014028 = 70;  // Example wattage for 7 lights
static int DAT_0001402c = 90;  // Example wattage for 9 lights

// Helper function for memory allocation and error handling
static void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        // Original code called _terminate(5)
        fprintf(stderr, "Memory allocation failed, terminating.\n");
        exit(5);
    }
    return ptr;
}

// Dummy for list_create_dup
static unsigned int list_create_dup(void) {
    // Placeholder, returns a non-zero value to simulate success
    // In a real system, this would return a list ID or pointer
    return 1;
}

// Struct definitions (derived from analysis of memory access patterns)
typedef struct Receptacle {
    int id;
    int type; // always 2 in create_receptacle
    int state; // always 0 in create_receptacle
    unsigned char param_val; // param_1 from create_receptacle
    unsigned char padding[3]; // To ensure 16-byte alignment/size
} Receptacle; // Size: 16 bytes

typedef struct Breaker {
    unsigned int id_flags;
    unsigned char param_val; // param_1 from create_breaker
    unsigned char padding[3]; // To ensure 12-byte alignment/size
    unsigned int list_handle;
} Breaker; // Size: 12 bytes

typedef struct LoadCenter {
    unsigned char num_breakers;
    unsigned char unknown_1;
    unsigned short padding; // To ensure 8-byte alignment/size
    unsigned int max_amps; // param_1 from create_load_center
    // Breakers would follow here as a flexible array member if C99,
    // but original code treats it as raw memory.
} LoadCenter; // Base Size: 8 bytes

typedef struct Outlet {
    int id;
    Receptacle receptacle1;
    Receptacle receptacle2;
    unsigned char param_val;
    unsigned char padding[3]; // To ensure 40-byte alignment/size
} Outlet; // Size: 40 bytes

typedef struct NWaySplitter {
    int id;
    unsigned char type1; // 0xf
    unsigned char type2; // 0xf
    unsigned char count; // param_1 from create_n_way_splitter
    unsigned char padding; // To ensure 8-byte alignment/size
    Receptacle receptacles[]; // Flexible array member (C99)
} NWaySplitter; // Base Size: 8 bytes

typedef struct LightString {
    int id;
    int count; // param_1 from create_light_string
    int wattage; // param_2 from create_light_string
    Receptacle receptacle;
} LightString; // Size: 28 bytes


// Function: create_load_center
// Returns a pointer to LoadCenter, cast to undefined4 for compatibility with original return type.
undefined4 create_load_center(unsigned int param_1, byte param_2) {
    LoadCenter *lc = (LoadCenter *)xmalloc(sizeof(LoadCenter) + (size_t)param_2 * sizeof(Breaker));
    
    lc->num_breakers = param_2;
    lc->unknown_1 = 0;
    lc->max_amps = param_1;
    
    return (undefined4)lc;
}

// Function: create_breaker
void create_breaker(unsigned char param_1, unsigned int *param_2, unsigned int param_3) {
    Breaker *breaker = (Breaker *)param_2; // Cast to Breaker*
    
    breaker->id_flags = param_3 & 0xff;
    breaker->param_val = param_1;
    breaker->list_handle = list_create_dup();
    
    if (breaker->list_handle == 0) {
        exit(5); // Original _terminate(5)
    }
}

// Function: create_receptacle
void create_receptacle(unsigned char param_1, int *param_2) {
    Receptacle *rec = (Receptacle *)param_2; // Cast to Receptacle*
    
    rec->id = next_receptacle_id++;
    rec->type = 2;
    rec->state = 0;
    rec->param_val = param_1;
}

// Function: create_outlet
// Returns a pointer to Outlet, cast to int* for compatibility with original return type.
int * create_outlet(byte param_1) {
    Outlet *outlet = (Outlet *)xmalloc(sizeof(Outlet));
    
    outlet->id = next_outlet_id++;
    
    // create_receptacle calls, passing the address of the Receptacle structs
    create_receptacle(param_1, (int *)&outlet->receptacle1);
    create_receptacle(param_1, (int *)&outlet->receptacle2);
    
    outlet->param_val = param_1;
    
    return (int *)outlet;
}

// Function: create_n_way_splitter
// Returns a pointer to NWaySplitter, cast to int* for compatibility with original return type.
int * create_n_way_splitter(byte param_1) {
    NWaySplitter *splitter = (NWaySplitter *)xmalloc(sizeof(NWaySplitter) + (size_t)param_1 * sizeof(Receptacle));
    
    splitter->id = next_n_way_splitter_id++;
    splitter->type1 = 0xf;
    splitter->type2 = 0xf;
    splitter->count = param_1;
    
    for (int i = 0; i < param_1; ++i) {
        create_receptacle(0xf, (int *)&splitter->receptacles[i]);
    }
    
    return (int *)splitter;
}

// Function: create_light_string
// Returns a pointer to LightString, cast to int* for compatibility with original return type.
int * create_light_string(int param_1, int param_2) {
    LightString *ls = (LightString *)xmalloc(sizeof(LightString));
    
    ls->id = next_light_string_id++;
    ls->count = param_1;
    ls->wattage = param_2;
    
    create_receptacle(0xf, (int *)&ls->receptacle);
    
    return (int *)ls;
}

// Function: get_max_amps_of_light_string
long double get_max_amps_of_light_string(void) {
  return DAT_0001401c;
}

// Function: get_new_load_center_by_model_id
undefined4 get_new_load_center_by_model_id(unsigned int param_1) {
    switch (param_1) {
        case 0x2718: return create_load_center(100, 8);
        case 0x2720: return create_load_center(100, 0x10); // 16
        case 0x2724: return create_load_center(100, 0x14); // 20
        case 0x3aac: return create_load_center(0x96, 0x14); // 150 amps, 20 breakers
        case 0x3ab0: return create_load_center(0x96, 0x18); // 150 amps, 24 breakers
        case 0x3ab6: return create_load_center(0x96, 0x1e); // 150 amps, 30 breakers
        case 0x4e34: return create_load_center(200, 0x14); // 20 breakers
        case 0x4e3e: return create_load_center(200, 0x1e); // 30 breakers
        case 0x4e48: return create_load_center(200, 0x28); // 40 breakers
        case 0x4e4a: return create_load_center(200, 0x2a); // 42 breakers
        case 0x9c54: return create_load_center(400, 0x14); // 20 breakers
        case 0x9c5e: return create_load_center(400, 0x1e); // 30 breakers
        case 0x9c68: return create_load_center(400, 0x28); // 40 breakers
        case 0x9c6a: return create_load_center(400, 0x2a); // 42 breakers
        default: return 0;
    }
}

// Function: get_new_breaker_by_model_id
unsigned char get_new_breaker_by_model_id(int param_1, unsigned int *param_2, unsigned int param_3) {
    if (param_1 == 0xf) {
        create_breaker(0xf, param_2, param_3);
    } else if (param_1 == 0x14) {
        create_breaker(0x14, param_2, param_3);
    } else {
        return 0xff; // Error code
    }
    return 0; // Success code
}

// Function: get_new_outlet_by_model_id
undefined4 get_new_outlet_by_model_id(int param_1) {
    if (param_1 == 0xf) {
        return (undefined4)create_outlet(0xf);
    } else if (param_1 == 0x14) {
        return (undefined4)create_outlet(0x14);
    }
    return 0;
}

// Function: get_new_n_way_splitter_by_model_id
undefined4 get_new_n_way_splitter_by_model_id(unsigned int param_1) {
    switch (param_1) {
        case 3: return (undefined4)create_n_way_splitter(3);
        case 6: return (undefined4)create_n_way_splitter(6);
        case 8: return (undefined4)create_n_way_splitter(8);
        default: return 0;
    }
}

// Function: get_new_light_string_by_model_id
undefined4 get_new_light_string_by_model_id(unsigned int param_1) {
    switch (param_1) {
        case 5: return (undefined4)create_light_string(5, DAT_00014020);
        case 6: return (undefined4)create_light_string(6, DAT_00014024);
        case 7: return (undefined4)create_light_string(7, DAT_00014028);
        case 9: return (undefined4)create_light_string(9, DAT_0001402c);
        default: return 0;
    }
}