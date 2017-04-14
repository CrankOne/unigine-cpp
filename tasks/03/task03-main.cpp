
# include "trie.hpp"

# include <fstream>

int
main(int argc, const char * argv[]) {
    if( 3 != argc ) {
        std::cerr << "Error: wrong cmd-line arguments number." << std::endl
                  << "Usage:" << std::endl
                  << "  $ " << argv[0] << " <in-filename> <out-filename>"
                  << std::endl;
        return EXIT_FAILURE;
    }

    Node trie;
    {
        std::ifstream iFile( argv[1] );
        for(std::string line; std::getline(iFile, line, '\n');) {
            bool hasTok = false;
            auto prevTok = line.begin();
            for( auto c = line.begin(); line.end() != c; ++c ) {
                if( !! (*c = encode(*c) ) ) {
                    if( !hasTok ) {
                        prevTok = c;
                    }
                    hasTok = true;
                } else if( hasTok ) {
                    hasTok = false;
                    // Used to check tokenization with trivial encoder:
                    //printf( "\"%s\"\n", prevTok );
                    trie.consider_token( &(*prevTok) );
                }
            }
        }
    }
    trie.dump_recursively( "" );

    return EXIT_SUCCESS;
}
