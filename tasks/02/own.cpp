# include <unordered_map>

# include <cstdlib>
# include <cstdio>
# include <cassert>
# include <cstring>
# include <random>

# include "rdus.hpp"

template<template<typename KeyT, typename ValueT> class ContainerT, typename KeyT, typename ValueT> void
test_suite() {
    // Fill hash with random info.
    
}

template<typename KeyT, typename ValueT> using stlhasht = std::unordered_map<KeyT, ValueT>;
template<typename KeyT, typename ValueT> using myhasht = myhash<KeyT, ValueT>;

static void
read_random_token( FILE * rndF, char data[], size_t length ) {
    // I'm not sure that rand() call will always elapse a fixed amount of time
    // for generating random numbers and tried to read random tokens from
    // /dev/urandom. Despite this approach is not better this way so far,
    // I've decided to leave it as is just because I could.
    fread( data, 1, length, rndF );
    char * c = data,
         * dataEnd = c + length;
    for( ; dataEnd > c; ++c ) {
        if( *c & 0x10 ) {
            *c = ((unsigned) (*c & 0xe7)%26) + 97;
        } else {
            *c = ((unsigned) (*c & 0xe7)%26) + 65;
        }
    }
    *c = '\0';
}

int
integrity_test( size_t nEntries ) {
    stlhasht<std::string, int> hash1;
    myhash<std::string, int> hash2;
    FILE * rndF = fopen( "/dev/urandom", "r" );
    char tok[16];
    for( size_t n = 0; n < nEntries; ++n ) {
        read_random_token( rndF, tok, 15 );
        //printf( "! %s\n", tok );
        int value = rand();
        hash1[tok] = value;
        hash2[tok] = value;
    }
    fclose(rndF);

    printout( "*** INTEGRITY CHECK ***\n" );

    int n = 0;
    for( const auto & p : hash1 ) {
        if( p.second != hash2[p.first] ) {
            fprintf( stderr, "(%d, %s) %d != %d\n", n, p.first.c_str(), hash2[p.first], p.second );
            return 1;
        }
        n++;
    }

    for( size_t n = 0; n < nEntries/3; ++n ) {
        auto random_it = std::next(std::begin(hash1), rand()%hash1.size());
        hash2.erase(random_it->first);
        hash1.erase(random_it);
    }

    for( const auto & p : hash1 ) {
        if( hash1[p.first] != p.second ) {
            return 2;
        }
    }

    rndF = fopen( "/dev/urandom", "r" );
    for( size_t n = 0; n < nEntries/3; ++n ) {
        read_random_token( rndF, tok, 15 );
        //printf( "! %s\n", tok );
        int value = rand();
        hash1[tok] = value;
        hash2[tok] = value;
    }
    fclose(rndF);

    for( const auto & p : hash1 ) {
        if( hash1[p.first] != p.second ) {
            return 3;
        }
    }

    return 0;
}

int
main( int argc, const char * argv[] ) {

    if( integrity_test( 1024 ) ) {
        fprintf(stderr, "Error: integrity test failed.\n");
        return EXIT_FAILURE;
    }
    printf( "Integrity test passed.\n" );

    return EXIT_SUCCESS;
}

