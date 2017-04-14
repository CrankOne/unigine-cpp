# include <cstdint>
# include <cassert>
# include <iostream>
# include <algorithm>
# include <fstream>
# include <cstring>

// Returns code for given letter (ASCII)
char encode( char c ) {
    if( c < 'A' ) {
        return 0;
    } else if( c <= 'Z' ) {
        return c - 'A' + 1;
    } else if( c >= 'a' && c <= 'z' ) {
        return c - 'a' + 1;
    }
    return 0;
}

// Returns letter (ASCII) for given code
char decode( char b ) {
    if( b ) {
        return b + 'a' - 1;
    }
    return 0;
}

struct Node {
//private:
    uint32_t _counter;
    std::vector<char> _codes;
    std::vector<Node *> _childs;
//public:
    Node() : _counter(0) {}
    ~Node() {
        for( auto nodePtr : _childs ) {
            delete nodePtr;
        }
    }
    
    void inc_counter() { ++_counter; }
    size_t counter() const { return _counter; }

    /// Performs look-up for node indexed with given code. If lookup fails,
    /// inserts a new node.
    Node * node_by( char c ) {
        auto it = std::lower_bound( _codes.begin(), _codes.end(), c );
        if( _codes.end() != it && c == *it ) {
            return _childs[ it - _codes.begin() ];
        } else {
            _codes.insert( it, c );
            it = std::lower_bound( _codes.begin(), _codes.end(), c );
            assert( _codes.end() != it );
            size_t newPos = it - _codes.begin();
            _childs.reserve( newPos + 1 );
            return *_childs.insert( _childs.begin() + newPos, new Node() );
        }
    }

    void consider_token( char * tok ) {
        Node * n = this;
        for( char * c = tok; *c; ++c ) {
            n = n->node_by( *c );
        }
        n->inc_counter();
    }
};

struct Entry {
    uint32_t first;
    std::string second;

    Entry( uint32_t n, const std::string & str ) : first(n), second(str) {
    }

    friend bool operator<( const Entry & l, const Entry & r ) {
        if( l.first != r.first ) {
            return l.first < r.first;
        }
        return strcmp( l.second.c_str(), r.second.c_str() ) > 0;
    }
};

static void
fill_tmp_strg( std::vector<Entry> & dest,
               const std::string & prfx,
               Node & trie ) {
    char *c = trie._codes.data() + trie._codes.size()-1;
    for( auto nIt = trie._childs.rbegin();
             nIt != trie._childs.rend(); ++nIt, --c ) {
        fill_tmp_strg( dest, prfx + decode(*c), **nIt );
        if( (*nIt)->counter() ) {
            dest.push_back( Entry( (*nIt)->counter(), prfx + decode(*c) ) );
        }
    }
}

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
                    trie.consider_token( &(*prevTok) );
                }
            }
            if( !line.empty() && hasTok ) {
                trie.consider_token( &(*prevTok) );
            }
        }
    }

    std::vector<Entry> tokens;
    fill_tmp_strg( tokens, "", trie );

    std::sort( tokens.rbegin(), tokens.rend() );

    {
        std::ofstream oFile (argv[2]);
        for( auto p : tokens ) {
            oFile << p.first << " "
                  << p.second << std::endl;
        }
    }

    return EXIT_SUCCESS;
}

