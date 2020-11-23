#include <stdio.h>
#include "akinator.h"

int main(){
    Tree tree = {};
    tree_constructor(&tree);
    // tree_load_tree(&tree, "akinator.db");
    // tree_load_tree(&tree, "tmp1.txt");

    // dump_tree(&tree, "res1.pdf");
    // tree_save_tree(&tree, "tree.txt");
      interact(&tree);
    tree_destructor(&tree);
}