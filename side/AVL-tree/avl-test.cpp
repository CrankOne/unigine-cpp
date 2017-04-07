# include "avl-tree.tcc"

# include <cstdlib>
# include <iostream>

int
main( int argc, const char * argv[] ) {

    advanced_containers::AVLTree<int, int> tree;

    for( int i = 7; i < 14; ++i ) {
        tree.insert( i, i );
    }

    for( auto it = tree.begin(); tree.end() != it; ++it ) {
        std::cout << it->first << ", ";
    }
    std::cout << std::endl;

    // TODO: tree.lower_bound(10  )->first == 10
    // TODO: tree.lower_bound(6   )->first == 7
    // TODO: tree.lower_bound(14  )->first == tree.end()
    // TODO: tree.lower_bound(10.5)->first == 11

    return EXIT_SUCCESS;
}

