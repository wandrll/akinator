#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define DEBUG_ON
#ifdef DEBUG_ON
    #define IF_DEBUG_ON(code) code
#elif
    #define IF_DEBUG_ON(code)
#endif

const int max_vertex_line = 60;
struct Node{
    Node* parent;
    Node* left;
    Node* right;
    size_t height;
    bool isleaf;
    char* line;
};

    void node_constructor(Node* node, const char* node_name, size_t size);

struct Buffer{
    char** buffer;
    size_t* curr_sizes;
    size_t* capacity;
    size_t count;
};

    void buffer_constructor(Buffer* buff, char* str, size_t size, size_t count);

    char* buffer_insert(Buffer* buff, const char* str);

    void buffer_destructor(Buffer* buff);

struct Description{
    bool* status;
    const char** traits;
    const char* name;
    size_t traits_count;
};

    void description_constructor(Description* desc, size_t max_traits);

    void description_destructor(Description* desc);





struct Tree{
    Node* root;
    Buffer* buff;
};
    enum tree_codes{WRONG_FILE, TREE_OK, TREE_EMPTY};

    tree_codes tree_constructor(Tree* tree);

    tree_codes tree_load_tree(Tree* tree, const char* file);

    tree_codes tree_save_tree(Tree* tree, const char* file);

    tree_codes dump_tree(Tree* tree, const char* system);

    void interact(Tree* tree);

    tree_codes tree_destructor(Tree* tree);

    enum actions {EXIT, REQUEST, LOAD, SAVE, NEW, VISUALISE, DESCRIPTION, COMPARE} ;

