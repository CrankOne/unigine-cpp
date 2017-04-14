# include "trie.hpp"

# include <cstdlib>
# include <cstdio>

void
letters_encoding_test() {
    char str[] = "The quick brown fox jumps over the lazy dog. BRIGHT VIXENS JUMP; DOZY FOWL QUACK",
        *strEnd = str + sizeof(str),
        encStr[sizeof(str)]
        ;
    char * ec = encStr;
    for( char * c = str; c != strEnd; ++c, ++ec ) {
        *ec = encode(*c);
        putc(*c, stdout);
    }
    putc('\n', stdout);
    strEnd = encStr + sizeof(str);
    for( char *c = encStr; c != strEnd; ++c ) {
        char decoded = decode( *c );
        if( decoded ) {
            putc(decoded, stdout);
        } else {
            putc(' ', stdout);
        }
    }
    putc('\n', stdout);
}

void
test_dump() {
    Node trie;

    char str[] = "Car cap track trick treat two two twelve alone one zip zap zipzap",
        *strEnd = str + sizeof(str),
        encStr[sizeof(str)]
        ;
    char * ec = encStr,
         * prevTok = ec;
    bool hasTok = false;
    for( char * c = str; c != strEnd; ++c, ++ec ) {
        if( !! (*ec = encode(*c) ) ) {
            if( !hasTok ) {
                prevTok = ec;
            }
            hasTok = true;
        } else if( hasTok ) {
            hasTok = false;
            // Used to check tokenization with trivial encoder:
            //printf( "\"%s\"\n", prevTok );
            trie.consider_token( prevTok );
        }
    }
    trie.dump_recursively( "" );
}

int
main( int argc, const char * argv[] ) {
    //letters_encoding_test();
    test_dump();

    return EXIT_SUCCESS;
}

