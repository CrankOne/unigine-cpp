# include <iostream>
# include <stdexcept>
# include <climits>
# include <cstdlib>
# include <cstring>
# include <cassert>
# include <vector>

# if __cplusplus <= 199711L
# define nullptr_C11 NULL
# else
# define nullptr_C11 nullptr
# endif

template<typename T>
struct StdTraits;

template<typename T>
struct DefaultAllocator12;

template<typename T,
         typename TraitsT=StdTraits<T>,
         typename AllocatorT=DefaultAllocator12<T> > class myvector {
public:
    typedef myvector<T, TraitsT, AllocatorT> Self;
    typedef T * iterator;
private:
    T * _data,
      * _end,
      * _reservedEnd;
public:
    // Required interface:
	myvector() : _data(nullptr_C11),
                 _end(nullptr_C11),
                 _reservedEnd(nullptr_C11) {}
	~myvector() { clear(); }

	int capacity() { return _reservedEnd - _data; }
	int size() { return _end - _data; }

	void add(const T & value); // because we can
	T & add(); // easy handmade emplace_back()
	void erase(int index); // easy handmade erase()

	void push_back(const T & value); // for test compatibility
	void erase(const T * item); // for test compatibility

	T & operator[](int index) {
            return at(index); }
	const T & operator[](int index) const {
            return at(index); }

	T * begin() { return _data; }
	T * end() { return _end; }

	void clear();
	void resize(int new_size);
	void reserve(int min_capacity);
    // ^^^ end of required interface
public:
    // sugar/helpers/aux...
    const T & at( int index ) const;
    T & at( int index ) {
        const Self * cSelf = this;
        return const_cast<T&>( cSelf->at(index) ); }
};  // myvector

template<typename T, typename TraitsT, typename AllocatorT> void
myvector<T, TraitsT, AllocatorT>::add(const T & value) {
    if( ! (_end < _reservedEnd) ) {
        reserve( size() + 1 );
    }
    *_end = value;
    //std::cout << "Writing " << value << " to #" << (_end - _data) << std::endl;
    ++_end;
}

template<typename T, typename TraitsT, typename AllocatorT> T &
myvector<T, TraitsT, AllocatorT>::add() {
    T n;
    add( n );
    return n;
}

template<typename T, typename TraitsT, typename AllocatorT> void
myvector<T, TraitsT, AllocatorT>::erase( int index ) {
    erase( &(at(index)) );
}

template<typename T, typename TraitsT, typename AllocatorT> void
myvector<T, TraitsT, AllocatorT>::push_back(const T & value) {
    add( value );
}

// Here comes rockin':

template<typename T, typename TraitsT, typename AllocatorT> const T &
myvector<T, TraitsT, AllocatorT>::at( int index ) const {
    assert( _data );
    if(!(_data + index < _end)) {
        throw std::out_of_range( "Custom vector index overflow." );
    }
    return *(_data + index);
}

template<typename T, typename TraitsT, typename AllocatorT> void
myvector<T, TraitsT, AllocatorT>::erase(const T * item) {
    if( item > _end || item < _data ) {
        throw std::out_of_range( "Invalid element ptr to be erased." );
    }
    AllocatorT::erase_one( _data, const_cast<T*>(item), _end );
}

template<typename T, typename TraitsT, typename AllocatorT> void
myvector<T, TraitsT, AllocatorT>::clear() {
    AllocatorT::clear( _data );
    _data = _end = _reservedEnd = nullptr_C11;
}

template<typename T, typename TraitsT, typename AllocatorT> void
myvector<T, TraitsT, AllocatorT>::resize(int new_size) {
    AllocatorT::reallocate( _data, _end, _reservedEnd, new_size );
}

template<typename T, typename TraitsT, typename AllocatorT> void
myvector<T, TraitsT, AllocatorT>::reserve(int min_capacity) {
    AllocatorT::reallocate( _data, _end, _reservedEnd, min_capacity );
}


//
// Dumb allocation strategy with increasing factor 1.2

template<typename T>
struct DefaultAllocator12 {
    static void erase_one( T *& bgn, T * item, T *& dEnd ) {
        // We have to delete a particular element inside a block of memory
        // keeping then the integrity of the remained sequence.
        //memcpy( item, item + 1, sizeof(T)*(dEnd - (item + 1)) );  // good only for POD
        T * cc = item;
        for( T * c = item + 1; c != dEnd; ++c, ++cc ) {
            *cc = *c;
        }
        --dEnd;
    }
    
    static void clear( T * dat ) {
        delete [] dat;
    }

    static void reallocate( T *& bgn, T *& dEnd, T *& rEnd, int n ) {
        if(!(rEnd - bgn < n)) return;
        int nn;
        T * newdat = new T [nn = fine_block_size(n, rEnd - bgn)];
        if( bgn ) {
            //memcpy( newdat, bgn, sizeof(T)*(dEnd - bgn) );  // good only for POD
            T * cc = newdat;
            for( T * c = bgn; c != dEnd; ++c, ++cc ) {
                *cc = *c;
            }
            // FIXME: according to STL, new objects at reserved space has to
            // be _inserted_ apparently meaning that for POD type they have to be
            // initially set to zero.
            clear( bgn );
        }
        bgn = newdat;
        rEnd = bgn + nn;
        dEnd = bgn + n - 1;
    }

    static size_t fine_block_size( int oldSize, size_t newSize ) {
        if( newSize >= INT_MAX ) {
            throw std::bad_alloc();  // Bad reallocation block size requested.
        }
        // FIXME: Say hello to our precious magic numbers:
        if( newSize < 32 ) {
            return 32;
        } else {
            return newSize*1.2;
        }
    }
};


//
// Testing suite:

# ifndef __GNUG__

int
main(int argc, const char * argv[]) {
    fprintf( stderr,
        "Dummy entry point, sorry. App built without GNU extensions support\n" );
    return 0;
}

# else  // requires GNU extensions

template<template<typename T> class ContainerT, typename T> int
test_suite( const size_t N ) {
    volatile int res = 0;
    for (int i = 0; i < 1000000; i++) res += i*i;
    //fprintf(stderr, "res %d\n", res);

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
    return res;
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

    if( 'a' == argv[1][0] ) {
        for( size_t i = 0; i < 5e3; ++i ) {
            test_suite<stlvec, char>( 1e2 );
        }
    } else if( 'A' == argv[1][0] ) {
        for( size_t i = 0; i < 5e3; ++i ) {
            test_suite<myvec, char>( 1e2 );
        }
    } else if( 'b' == argv[1][0] ) {
        test_suite<stlvec, int>( INT_MAX/3e3 );
    } else if( 'B' == argv[1][0] ) {
        test_suite<myvec, int>( INT_MAX/3e3 );
    } else if( 'c' == argv[1][0] ) {
        test_suite<stlvec, ImNotAPOD>( 1e5 );
    } else if( 'C' == argv[1][0] ) {
        test_suite<myvec, ImNotAPOD>( 1e5 );
    }

    return EXIT_SUCCESS;
}

# endif

