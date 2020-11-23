#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "akinator.h"
#include <assert.h>
#include <sys/stat.h>



void node_constructor(Node* node, char* node_name){
    node->height = 1;
    node->line = node_name;
}


void buffer_constructor(Buffer* buff, char* str, size_t size, size_t count){
    buff->buffer = (char**)calloc(count + 1, sizeof(char*));
    buff->curr_sizes = (size_t*)calloc(count + 1, sizeof(size_t));
    buff->capacity = (size_t*)calloc(count + 1, sizeof(size_t));
    
    buff->buffer[0] = str;
    buff->buffer[1] = (char*)calloc(size+1, sizeof(char));
    buff->capacity[1] = size;
    buff->count = 1;
}

char* buffer_insert(Buffer* buff, const char* str){
    size_t size = strlen(str);

    for(int i = 1; i <= buff->count; i++){
        if(buff->curr_sizes[i] + size <= buff->capacity[i]){
            fflush(stdout);
            memcpy(buff->buffer[i]+buff->curr_sizes[i], str, size);
            buff->curr_sizes[i] += (size+1);
            return buff->buffer[i]+buff->curr_sizes[i] - size - 1;
        }
    }
    
    buff->count++;
    buff->buffer[buff->count] = (char*)calloc(buff->capacity[buff->count-1]*2 + 1, sizeof(char));

    memcpy(buff->buffer[buff->count], str, size);
    buff->capacity[buff->count] = buff->capacity[buff->count-1]*2;
    buff->curr_sizes[buff->count] = (size+1);

    return buff->buffer[buff->count];
}

void buffer_destructor(Buffer* buff){
    if(buff->buffer != NULL){
        for(int i = 0; i <= buff->count; i++){
            free(buff->buffer[i]);
        }
        free(buff->buffer);
        free(buff->capacity);
        free(buff->curr_sizes);
    }
}



tree_codes tree_constructor(Tree* tree){
    tree->root = NULL;
    tree->buff = (Buffer*)calloc(1, sizeof(Buffer));
    return TREE_OK;
}

static void recursive_delete(Node* node){
    if(node->left != NULL){
        recursive_delete(node->left);
    }
    if(node->right != NULL){
        recursive_delete(node->right);
    }
    free(node);
}


tree_codes tree_destructor(Tree* tree){
    if(tree->root != NULL){
        recursive_delete(tree->root);
        tree->root = NULL;
    }
    if(tree->buff != NULL){
        // printf("%p ", tree->buff);
        buffer_destructor(tree->buff);
        free(tree->buff);
        tree->buff = NULL;
    }
    return TREE_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////ERROR PROCESSING//////////////////////////////////////////

void error_processing(tree_codes error){
    switch(error){
        case TREE_EMPTY:{
            printf("\e[91mERROR\e[0m: Tree isn't loaded\n");
            break;
        }
        case WRONG_FILE:{
            printf("\e[91mERROR\e[0m: Wrong file\n");
            break;
        }
       
    }

}



////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////LOAD BLOCK////////////////////////////////////////////////

static void recalculate_height(Node* tree){
    int l = 0;
    int r = 0;
    if(tree->left != NULL){
        l = tree->left->height;
    }
    if(tree->right != NULL){
        r = tree->right->height;
    }
    if(l > r){
        tree->height = l + 1;
    }else{
        tree->height = r + 1;
    }
}



static size_t file_size(const char* file){
    assert(file != NULL);
    struct stat st = {};
    stat(file, &st);
    return st.st_size;
}

bool is_ok(char c){
    if(c == '[' || c == ']'){
        return true;
    }else{
        return false;
    }
}


static char* load_and_prepare_in_buffer(const char* file, size_t* size){
    assert(file != NULL);
    FILE* fp = fopen(file, "r");
    if(fp == NULL){
        return NULL;
    }
    *size = 0;
    size_t fsize = file_size(file);
    char* tmp = (char*)calloc(fsize+1, sizeof(char));
    fread(tmp, sizeof(char), fsize, fp);
    fclose(fp);
    char* left = tmp;
    char* right = tmp;
    bool is_opened = 0;

    while(right - tmp <= fsize){
        if(*right == '"'){
            *left = '"';
            left++;
            (*size)++;
            is_opened = (is_opened+1)%2;
        }else{
            if(is_opened){
                *left = *right;
                left++;
            }else{
                if(is_ok(*right)){
                    *left = *right;
                    left++;
                }
            }
        }
        right++;
    }
    *left = 0;
    (*size) = (*size)/2;
    (*size) += (left - tmp);
    return tmp;
}

static char* find_pointer_on_right_node(char* line){
    char* tmp = line;
    int delta = 1;
    bool is_name = 0;
    tmp++;
    while(delta != 0){

        if(*tmp == '"'){
            is_name = (is_name + 1)%2;
        }else{
            if(!is_name){
                switch(*tmp){
                    case '[':{
                        delta++;
                        break;
                    }
                    case ']':{
                        delta--;
                        break;
                    }
                }
            }
        }
        tmp++;
    }
    return tmp;
}

static Node* do_load_from_buffer(char* line){
    size_t node_name_begin = 1;
    size_t node_name_end = 0;
    char* tmp = line + 1;
    while(*tmp != '"'){
        tmp ++;
    }
    Node* res = (Node*)calloc(1, sizeof(Node));
    *line = 0;
    node_constructor(res, line+1);

    *tmp = 0;
    tmp++;

    if(*tmp == '"' || *tmp == ']'){
        res->isleaf = true;
    }else{
        tmp++;
        res->isleaf = false;
        char* left = tmp;
        tmp++;
        while(*tmp != '"'){
            tmp++;
        }
        tmp++;
        if(*tmp == '['){
            tmp = find_pointer_on_right_node(tmp);
        }
        char* right = tmp;
        fflush(stdout);
       
        res->left = do_load_from_buffer(left);
        res->left->parent = res;
        res->right = do_load_from_buffer(right);
        res->right->parent = res;
        recalculate_height(res);


    }
    
    return res;
}


tree_codes tree_load_tree(Tree* tree, const char* file){
    assert(tree != NULL);
    assert(file != NULL);
    size_t size = 0;
    char* buffer = load_and_prepare_in_buffer(file, &size);
    
    buffer_constructor(tree->buff, buffer, max_vertex_line * 2, 20);

    if(buffer == NULL){
        return WRONG_FILE;
    }
    tree->root = do_load_from_buffer(buffer);

    return TREE_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////VALIDATE BLOCk////////////////////////////////////////////



bool node_validate(Node* node){
    if(node->left == NULL && node->right == NULL){
        if(node->isleaf){
            return true;
        }else{
            return false;
        }
    }else{
        if(node->left != NULL && node->right != NULL){
            if(node->isleaf){
                return false;
            }
            return node_validate(node->left) && node_validate(node->right);
        }else{
            return false;
        }
    }
}


void tree_validate(Tree* tree){
    if(tree->root == NULL){
        return;
    }else{
        if(!node_validate(tree->root)){
            printf("Tree has been corrupted. Check error.pdf\n");
            dump_tree(tree, "error.pdf");
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////SAVE BLOCK////////////////////////////////////////////////


void set_spaces(FILE* fp, int count){
    for(int i = 0; i < count; i++){
        fputc(' ', fp);
    }
}

void save_node(Node* node, FILE* fp, int level){
    assert(node != NULL);
    fprintf(fp, "\"%s\"", node->line);
    if(node->left != NULL && node->left != NULL){
        fprintf(fp, "\n");
        set_spaces(fp, level*4);
        fprintf(fp, "[");

        save_node(node->left, fp, level + 1);
        set_spaces(fp, level*4);

        save_node(node->right, fp, level + 1);
        set_spaces(fp, level*4);

        fprintf(fp, "]");
        set_spaces(fp, level*4);

        fprintf(fp, "\n");


    }
     fprintf(fp, "\n");

}

tree_codes tree_save_tree(Tree* tree, const char* file){
    assert(tree != NULL);
    assert(file != NULL);
    if(tree->root == NULL){
        return TREE_EMPTY;
    }
    FILE* fp = fopen(file, "w");
    save_node(tree->root, fp, 0);
    fflush(fp);
    fclose(fp);
    return TREE_OK;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////DUMP BLOCK////////////////////////////////////////////////


void do_dump_tree(Node* node, FILE* fp){
    fprintf(fp, "%ld[shape=record label = \"line = %s\\n height = %ld|{left = %p|right = %p|addr = %p| parent = %p}| is leaf = %d\"]\n", (size_t)node, node->line, node->height, node->left, node->right,  node, node->parent, node->isleaf);
 
    if(node->left != NULL){

        fprintf(fp, "%ld->%ld\n", (size_t)node, (size_t)node->left);        
        do_dump_tree(node->left, fp);
    }
    
    if(node->right != NULL){
        fprintf(fp, "%ld->%ld\n", (size_t)node, (size_t)node->right);        
        do_dump_tree(node->right, fp);
    }

}


tree_codes dump_tree(Tree* tree, const char* file){
    assert(tree != NULL);
    if(tree->root == NULL){
        return TREE_EMPTY;
    }
    FILE* fp = fopen("res.gv", "w");
    fprintf(fp,"digraph G{\n");
    if(tree->root != NULL){
            do_dump_tree(tree->root, fp);
        }
    fprintf(fp,"}\n");
    fclose(fp);


    char* str = (char*)calloc(23+strlen(file), sizeof(char));
    strcat(str, "dot -Tpdf res.gv -o");
    strcat(str, file);
    system(str);
    free(str);
    remove("res.gv");
    return TREE_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////DESCRIPTION BLOCK/////////////////////////////////////////

void description_constructor(Description* desc, size_t max_traits){
    desc->traits_count = 0;
    desc->traits = (const char**)calloc(max_traits, sizeof(char*));
    desc->status = (bool*)calloc(max_traits, sizeof(bool));
}

void description_destructor(Description* desc){
    free(desc->traits);
    free(desc->status);
    desc->traits = NULL;
    desc->name = NULL;
    desc->traits_count = 0;
}

void description_add_trait(Description* desc, char* trait, bool status){
    desc->traits[desc->traits_count] = trait;
    desc->status[desc->traits_count] = status;
    desc->traits_count++;
}

void description_add_name(Description* desc, char* name){
    desc->name = name;
}

static void my_gets(char* dest, size_t size){
    size_t curr = 0;
    char c = getchar();
    while(curr < size && c != '\n'){
        *dest = c;
        dest++;
        curr++;
        c = getchar();
    }
    *dest = 0;

}

static Node* find_node_and_fill_description(Node* node, const char* key, Description* desc){
    if(node->left == NULL && node->right == NULL){
        if(strcmp(key, node->line) == 0){
            description_add_name(desc, node->line);
            return node;
        }else{
            return NULL;
        }
    }else{
        Node* left = NULL;
        Node* right = NULL;
        left = find_node_and_fill_description(node->left, key, desc);
        right = find_node_and_fill_description(node->right, key, desc);
        if(left == NULL && right == NULL){
            return NULL;
        }else{
            if(left == NULL){
                description_add_trait(desc, node->line, false);
                return right;
            }else{
                description_add_trait(desc, node->line, true);
                return left;
            }
        }
    }
}

void print_description(Description* descr){
    printf("This is \e[4m\e[93m%s \e[0mdescription:\n", descr->name);
    if(descr->traits_count == 0){
        printf("Sorry, i can't say anything about it.\n");
    }else{
        for(int i = descr->traits_count - 1; i >= 0; i--){
            if(descr->status[i] == true){
                printf(":\e[92m%s\e[0m\n", descr->traits[i]);
            }else{
                printf(":\e[91mnot %s\e[0m\n",  descr->traits[i]);
            }
        }
    }
}

void print_cmp_description(Description* descr1, Description* descr2){
    printf("This is similarities and differences beetwen \e[4m\e[93m%s\e[0m and \e[4m\e[93m%s\e[0m:\n", descr1->name, descr2->name);
    int curr1 = descr1->traits_count-1;
    int curr2 = descr2->traits_count-1;
    printf("\e[4m\e[93mSimilarities\e[0m:\n");
    while(curr1 >= 0 && curr2 >= 0 && descr1->status[curr1] == descr2->status[curr2]){
        if(descr1->status[curr1] == true){
            printf(":\e[92m%s\e[0m\n", descr1->traits[curr1]);
        }else{
            printf(":\e[91mnot %s\e[0m\n",  descr1->traits[curr1]);
        }
        curr1--;
        curr2--;
    }
    if(curr1 == -1 && curr2 == -1){
        printf("\e[4m\e[93mThis is the same objects!!!\e[0m:\n");
        return ;
    }
    printf("-----------------\n");
    printf("\e[4m\e[93mDifferences\e[0m:\n");
    printf("%s:\n", descr1->name);
    while(curr1 >= 0){
        if(descr1->status[curr1] == true){
            printf(":\e[92m%s\e[0m\n", descr1->traits[curr1]);
        }else{
            printf(":\e[91mnot %s\e[0m\n",  descr1->traits[curr1]);
        }
        curr1--;
    }    
    printf("-----------------\n");
    printf("%s:\n", descr2->name);
    while(curr2 >= 0){
        if(descr2->status[curr2] == true){
            printf(":\e[92m%s\e[0m\n", descr2->traits[curr2]);
        }else{
            printf(":\e[91mnot %s\e[0m\n",  descr2->traits[curr2]);
        }
        curr2--;
    }  

}


void tree_description(Tree* tree, char* buffer){
     if(tree->root == NULL){
        error_processing(TREE_EMPTY);
        return;
    }
    getchar();
    my_gets(buffer, max_vertex_line);
   
    Description descr = {};
    description_constructor(&descr, tree->root->height);

    Node* res = find_node_and_fill_description(tree->root, buffer, &descr);
    
    if(res == NULL){
        printf("I'm sorry, i cant find it. Try another request\n");
    }else{
        print_description(&descr);
    }
    description_destructor(&descr);
}


void tree_compare(Tree* tree, char* buffer){
    if(tree->root == NULL){
        error_processing(TREE_EMPTY);
        return;
    }
    getchar();
    my_gets(buffer, max_vertex_line);
   
    Description descr1 = {};
    description_constructor(&descr1, tree->root->height);
    Node* res1 = find_node_and_fill_description(tree->root, buffer, &descr1);

    my_gets(buffer, max_vertex_line);

    Description descr2 = {};
    description_constructor(&descr2, tree->root->height);
    Node* res2 = find_node_and_fill_description(tree->root, buffer, &descr2);

    if(res1 == NULL && res2 == NULL){
        printf("I'm sorry, i cant find these objects. Try another request\n");
        return;
    }
    if(res1 == NULL){
        printf("I'm sorry, i cant find first object. Try another request\n");
        return;
    }
    if(res2 == NULL){
        printf("I'm sorry, i cant find second object. Try another request\n");
        return;
    }

    print_cmp_description(&descr1, &descr2);
    

    description_destructor(&descr1);
    description_destructor(&descr2);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////ADD OBJECT BLOCK//////////////////////////////////////////




static Node* update_node(Node* node, Buffer* buff, char* tmp){

    getchar();

    printf("I don't know what is it. Please tell me what is it?\n");

   

    Node* right = (Node*)calloc(1, sizeof(Node));    
    my_gets(tmp, max_vertex_line);
    right->line = buffer_insert(buff, tmp);


    
    printf("Also, please tell me which trait %s has, but %s hasn't\n", node->line, tmp);
    

    Node* res = (Node*)calloc(1, sizeof(Node));    
    my_gets(tmp, max_vertex_line);
    res->line = buffer_insert(buff, tmp);

    printf("I remembered this!\n");


    res->left = node;
    res->right = right;
    right->isleaf = true;
    res->right->parent = res;
    res->left->parent = res;

    return res;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////REQUESTs BLOCK////////////////////////////////////////////



static Node* request(Node* node, Buffer* buff, char* tmp){
    if(node->isleaf){
        printf("Is it %s?\n\e[92mType 1 if yes\n", node->line);
        printf("\e[91mType 0 if no\n\e[0m");

        int act = -1;
        while(act != 1 && act != 0){
            scanf("%d", &act);
        }
        if(act == 1){
            printf("Awesome!\n");
           
            return node;
        }else{
            Node* tmp1 = update_node(node, buff, tmp);
            recalculate_height(tmp1); 
            return tmp1;
        }
    }else{
        printf("Does it %s?\n \e[92mType 1 if yes\n \e[91mType 0 if no\n\e[0m", node->line);
        int act = -1;
        while(act != 1 && act != 0){
            scanf("%d", &act);
        }
        if(act == 1){
            node->left = request(node->left, buff, tmp);
            node->left->parent = node;
        }else{
            node->right = request(node->right, buff, tmp);
            node->right->parent = node;

        }
        recalculate_height(node);
        return node;
    }


}

void tree_first_vertex(Tree* tree, char* buffer){
    Node* res = (Node*)calloc(1, sizeof(Node));
    getchar();
    printf("Enter first object in future database\n");
    // char* first = (char*)calloc(max_vertex_line + 1, sizeof(char));
    my_gets(buffer, max_vertex_line);
    buffer_constructor(tree->buff, NULL, max_vertex_line * 2, 20);
    node_constructor(res, buffer_insert(tree->buff, buffer));
    res->isleaf = true;
    tree->root = res;
    IF_DEBUG_ON(tree_validate(tree);)

}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////INTERACTIONS BLOCK////////////////////////////////////////



void tree_Request(Tree* tree, char* tmp){
    if(tree->root == NULL){
        error_processing(TREE_EMPTY);
        return;
    }
    IF_DEBUG_ON(tree_validate(tree);)

    
    tree->root = request(tree->root, tree->buff, tmp);
    
    
    IF_DEBUG_ON(tree_validate(tree);)
}

void tree_Load(Tree* tree, const char* buffer){

    IF_DEBUG_ON(tree_validate(tree);)

    tree_destructor(tree);
    tree_constructor(tree);
    scanf("%20s", buffer);

    tree_codes res = tree_load_tree(tree, buffer);  

    if(res != TREE_OK){
        error_processing(res);
    }
    IF_DEBUG_ON(tree_validate(tree);)

}


void tree_Save(Tree* tree, const char* buffer){

    IF_DEBUG_ON(tree_validate(tree);)
    
    scanf("%20s", buffer);
    tree_codes res = tree_save_tree(tree, buffer);  
    if(res != TREE_OK){
        error_processing(res);
    }
    IF_DEBUG_ON(tree_validate(tree);)

}

void tree_visualaise(Tree* tree, const char* buffer){
    scanf("%20s", buffer);
    tree_codes res = dump_tree(tree, buffer);
    if(res != TREE_OK){
        error_processing(res);
    }
}


void interact(Tree* tree){
    printf("\e[95mHello user.\e[0m \n It is user interface of huge database.\n I can try to guess whatevere you want\n");
    actions act = REQUEST;
    printf("\e[93mType \e[0m\e[4m0\e[0m if you want exit programm.\n");
    printf("\e[93mType \e[0m\e[4m1\e[0m if you want make a request for database\n");
    printf("\e[93mType \e[0m\e[4m2\e[0m and name of file (less then %d symbols) if you want load tree from file.\n", max_vertex_line);
    printf("\e[93mType \e[0m\e[4m3\e[0m and name of file (less then %d symbols) if you want save tree in file.\n", max_vertex_line);
    printf("\e[93mType \e[0m\e[4m4\e[0m if you want create new own tree.\n");
    printf("\e[93mType \e[0m\e[4m5\e[0m and name of pdf file(less then %d symbols) if you want visualise tree in pdf file.\n", max_vertex_line);
    printf("\e[93mType \e[0m\e[4m6\e[0m and object (less then %d symbols) if you want see description of this element.\n", max_vertex_line);
    printf("\e[93mType \e[0m\e[4m7\e[0m and 2 objects (less then %d symbols) if you want compare them.\n", max_vertex_line);

    scanf("%d", &act);
    char* buffer = (char*)calloc(max_vertex_line+ 1, sizeof(char));
    while(act != EXIT){

        switch(act){
            case REQUEST:{
                tree_Request(tree, buffer);
                break;
            }

            case LOAD:{
                tree_Load(tree, buffer);
                break;
            }


            case SAVE:{
                tree_Save(tree, buffer);
                break;
            }

            case NEW:{
                tree_first_vertex(tree, buffer);
                break;
            }

            case VISUALISE:{
                tree_visualaise(tree, buffer);
                break;
            }


            case DESCRIPTION:{
                tree_description(tree, buffer);
                break;
            }

            case COMPARE:{
                tree_compare(tree, buffer);
                break;
            }

            default:{
                printf("Wrong command, try again\n");
            }

        }

        scanf("%d", &act);

    }
    free(buffer);
}

