# include "rdus01.h"

# include <iostream>
# include <cstring>
# include <cassert>
# include <cstdio>
# include <vector>

# ifdef _ENABLE_TIMING
#include <stdio.h>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#pragma comment(linker, "/defaultlib:psapi.lib")
#pragma message("Automatically linking with psapi.lib")
#else
#include <sys/time.h>
#endif

typedef long long llong;
typedef unsigned long long ullong;

llong microtimer() {
#ifdef _MSC_VER
	// Windows time query
	static llong iBase = 0;
	static llong iStart = 0;
	static llong iFreq = 0;

	LARGE_INTEGER iLarge;
	if (!iBase)
	{
		// get start QPC value
		QueryPerformanceFrequency(&iLarge); iFreq = iLarge.QuadPart;
		QueryPerformanceCounter(&iLarge); iStart = iLarge.QuadPart;

		// get start UTC timestamp
		// assuming it's still approximately the same moment as iStart, give or take a msec or three
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		iBase = (llong(ft.dwHighDateTime)<<32) + llong(ft.dwLowDateTime);
		iBase = (iBase - 116444736000000000ULL) / 10; // rebase from 01 Jan 1601 to 01 Jan 1970, and rescale to 1 usec from 100 ns
	}

	// we can't easily drag iBase into parens because iBase*iFreq/1000000 overflows 64bit int!
	QueryPerformanceCounter(&iLarge);
	return iBase + (iLarge.QuadPart - iStart) * 1000000 / iFreq;

#else
	// UNIX time query
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return llong(tv.tv_sec) * llong(1000000) + llong(tv.tv_usec);
#endif
}

void onexit() {
#ifdef _MSC_VER
	PROCESS_MEMORY_COUNTERS pmc;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	if (hProcess && GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		printf ( "--- peak-wss=%d, peak-pagefile=%d\n", (int)pmc.PeakWorkingSetSize, (int)pmc.PeakPagefileUsage);
#endif
}
# endif


//
// Testing suite:

# ifndef __GNUG__

int
main(int argc, const char * argv[]) {
    # ifdef _TESTING_STD
    # define myvector std::vector
    # endif
    //fprintf( stderr,
    //    "Dummy entry point, sorry --- app was built without GNU "
    //    "extensions support.\n" );
	{   // Shodan's killa test:
        int r = 0;
		myvector<int> t;
		r += t.capacity();
		r += t.size();
#ifndef _TESTING_STD
		t.add(1);       // 1
		t.add(2);       // 1, 2
		t.add() = 3;    // 1, 2, 3
		t.add() = 4;    // 1, 2, 3, 4
		t.erase(1);     // 1, 3, 4
#else
		t.emplace_back(1);
		t.emplace_back(2);
		t.push_back(3);
		t.push_back(4);
		t.erase(t.begin() + 1);
#endif
		t.push_back(5); // 1, 3, 4, 5
		t.erase(t.begin() + 1); // 1, 4, 5
		//r += t[3];  // shall fail here.
		const myvector<int> & rt = t;
		r += rt[2];
		r += (t.begin() != t.end());
		t.clear();
		t.resize(12);
		t.reserve(34);

        for( auto it = t.begin(); t.end() != it; ++it ) {
            printf( "%d\n", *it );
        }
	}
    printf( "Passed.\n" );
    return 0;
}

# else  // requires GNU extensions

template<template<typename T> class ContainerT, typename T> int
test_suite( const size_t N ) {
    ContainerT<T> v;
    
    for( size_t i = 0; i < N; ++i ) {
        v.push_back(T(N-i));
    }
    {
        int i = N;
        for( typename ContainerT<T>::iterator it = v.begin(); v.end() != it; ++it, --i ) {
            if( *it - i ) {
                std::cerr << "Integrity check failure: "
                          << *it << " != " << i
                          << std::endl;
                return -1;
            }
        }
    }

    v.clear();

    for( size_t i = 0; i < N; ++i ) {
        v.push_back(T(N-i));
        if( i && !(i%10) ) {
            v.erase( v.begin() + i/2 );
        }
    }

    //std::cout << "#0" << ":" << v[0] << std::endl;
    //std::cout << "#" << N/2 << ":" << v[N/2] << std::endl;
    //std::cout << "#" << N-1 << ":" << v[N-1] << std::endl;
    return 0;
}

template<typename T> using stlvec = std::vector<T>;
template<typename T> using myvec = myvector<T>;

class ImNotAPOD {
private:
    static const char strTok[128];
    void * _someSophisticatedData;
    int _n;
protected:
    void _allocate_data() {
        _someSophisticatedData = strdup( strTok );
    }
    void _free_data() {
        if( _someSophisticatedData ) {
            free( _someSophisticatedData );
            _someSophisticatedData = nullptr_C11;
        }
    }
    void _make_authentic_copy( const char * origStr ) {
        _free_data();
        if( !origStr ) return;
        char * mutatedData = strdup( origStr );
        int r1 = strlen(strTok)*(double(rand())/RAND_MAX),
            r2 = 'a' + ('z' - 'a')*double(rand())/RAND_MAX
            ;
        mutatedData[r1] = r2;
        _someSophisticatedData = mutatedData;
    }
public:
    ImNotAPOD() : _someSophisticatedData(nullptr_C11), _n(0) {
        _allocate_data();
    }
    ImNotAPOD( int n ) : _someSophisticatedData(nullptr_C11), _n(n) {
        _allocate_data();
    }
    ImNotAPOD( const ImNotAPOD & orig ) : _someSophisticatedData(nullptr_C11),
                                          _n(orig._n) {
            _make_authentic_copy( (const char *) orig._someSophisticatedData );
        }
    ~ImNotAPOD() {
        _free_data();
        _n = 0;
    }
    int n() const { return _n; }
    int operator-(int i) {
        return _n - i;
    }
    ImNotAPOD & operator=( const ImNotAPOD & orig ) {
        _make_authentic_copy( (const char *) orig._someSophisticatedData );
        _n = orig._n;
        return *this;
    }
    friend std::ostream & operator<<( std::ostream &, const ImNotAPOD & );
};

std::ostream &
operator<<( std::ostream & os, const ImNotAPOD & inst ) {
    os << inst.n();
    return os;
}

const char
ImNotAPOD::strTok[128] = "All work and no play made Jack a dull boy.";

int
main( int argc, char * const argv[] ) {
    if(argc != 2) {
        std::cerr << "Usage:" << std::endl
                  << "    $ " << argv[0] << "[a|A|b|B|b|B|b]" << std::endl
                  ;
        return EXIT_FAILURE;
    }
    # ifdef _ENABLE_TIMING
    // Warm things up
    volatile int res = 0;
    for (int i = 0; i < 1000000; i++) res += i*i;
    fprintf(stderr, "res %d\n", res);
    llong started = microtimer();
    # endif
    if( 'a' == argv[1][0] ) {
        for( size_t i = 0; i < 8e4; ++i ) {
            test_suite<stlvec, char>( 1e2 );
        }
    } else if( 'A' == argv[1][0] ) {
        for( size_t i = 0; i < 8e4; ++i ) {
            test_suite<myvec, char>( 1e2 );
        }
    } else if( 'b' == argv[1][0] ) {
        test_suite<stlvec, int>( INT_MAX/4e3 );
    } else if( 'B' == argv[1][0] ) {
        test_suite<myvec, int>( INT_MAX/4e3 );
    } else if( 'c' == argv[1][0] ) {
        test_suite<stlvec, ImNotAPOD>( 1e4 );
    } else if( 'C' == argv[1][0] ) {
        test_suite<myvec, ImNotAPOD>( 1e4 );
    }
    # ifdef _ENABLE_TIMING
    llong ended = microtimer();
    std::cout << ended - started << std::endl;
    # endif

    return EXIT_SUCCESS;
}

# endif

