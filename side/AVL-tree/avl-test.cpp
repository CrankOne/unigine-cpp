# include <iostream>  // XXX

# include "avl-tree.tcc"

# include <cstdlib>
# include <iostream>
# include <set>

int
main( int argc, const char * argv[] ) {

    advanced_containers::AVLTree<int, int> tree;

    # if 0
    std::set<int> has;
    for( int i = 0; i < 5; ++i ) {
        int newVal;
        for( newVal = rand()%15; has.find(newVal) != has.end(); newVal = rand()%15) {}
        tree.insert( newVal, i );
        has.insert(newVal);
    }
    # else
    # if 1
    for( int i = 14; i > 7; --i ) {
        tree.insert( i, i );
        tree.dump_self();
        std::cout << "--- --- ---" << std::endl;
    }
    # else
    for( int i = 7; i < 14; ++i ) {
        tree.insert( i, i );
        tree.dump_self();
        std::cout << "--- --- ---" << std::endl;
    }
    # endif
    # endif

    //std::cout << "--- --- ---" << std::endl;
    tree.dump_self();

    # if 0
    int sense = 10;
    for( auto it = tree.begin(); tree.end() != it; ++it, --sense ) {
        std::cout << it->first << ", ";
        if( !sense ) {
            std::cout << "...(infin.)";
            break;
        }
    }
    std::cout << std::endl;
    # endif

    // TODO: tree.lower_bound(10  )->first == 10
    // TODO: tree.lower_bound(6   )->first == 7
    // TODO: tree.lower_bound(14  )->first == tree.end()
    // TODO: tree.lower_bound(10.5)->first == 11

    return EXIT_SUCCESS;
}

