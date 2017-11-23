typedef struct chunk_data CHUNK_DATA;
typedef struct freed_chunk_data FREED_CHUNK_DATA;
typedef struct memory_data MEMORY_DATA;
typedef struct recycle_data RECYCLE_DATA;


// A single node in the chunk_list that contains all raw memory.
struct chunk_data {
    CHUNK_DATA *next; // Points to the next chunk in the list.
    int amount; // This is the size of the memory in this chunk.
    int used; // This is the size used of the memory in this chunk.

    char natural;
    void *ptr; // This points to a chunk.
};

// A single node pointing back to the chunks with an amount it points to.
// Latent recycled data will be reinserted into the system using this method.
struct freed_chunk_data {
    FREED_CHUNK_DATA *next; // next freed chunk

    int amount; // Total in this chunk.
    int used; // How much is used out of this chunk
    void *ptr; // This points to a chunk.
};

// A node pointing to some memory.
struct memory_data {
    MEMORY_DATA *next; // The next memory.
    void *ptr; // The memory.
};

struct recycle_data {
    RECYCLE_DATA *next; // points to the next bucket
    int size; // These nodes are of this size.
    MEMORY_DATA *list; // A list of memory of this size.
};


extern CHUNK_DATA *chunk_list;
extern FREED_CHUNK_DATA *freed_chunk_list;
extern MEMORY_DATA *memory_list;
extern RECYCLE_DATA *recycle_list;

