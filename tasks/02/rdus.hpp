# ifndef H_RDUS_MYHASH_H
# define H_RDUS_MYHASH_H

# include <cassert>
# include <stdexcept>
# include <cstdint>

# if __cplusplus <= 199711L
# define nullptr_C11 NULL
# else
# define nullptr_C11 nullptr
# endif

# ifdef MYHASH_LOG_EVAL
#   define printout(...) printf( __VA_ARGS__ )
# else
#   define printout(...)
# endif

template<typename T> uint32_t myhash_hash_spec( const T & );
template<typename T> bool myhash_equals( const T & l, const T & r );

template<typename KEY, typename VALUE>
class myhash
{
public:
    typedef uint32_t Size;
    typedef uint32_t HashValue;
    typedef KEY Key;
    typedef VALUE Value;
    typedef HashValue (*HashFunction)( const uint8_t *, Size );
    typedef myhash<Key, Value> Self;
    struct HashEntry {
        Key first, & key;  // ref for spec compat
        Value second, & value;  // ref for spec compat
        /// Two last bits are reserved for tombstone flag and occupancy flag
        /// correspondingly.
        HashValue hashValue;

        HashEntry() : hashValue(0), key(first), value(second) {}

        /// true, if is not occupied and wasn't occupied.
        bool is_vacant() const { return !((hashValue & 0x1) || (hashValue & 0x2)); }
        bool is_occupied() const { return hashValue & 0x1; }
        void release() { hashValue |= 0x2; hashValue &= ~0x1; }
        bool is_released() { return hashValue & 0x2; }
        void set( const Key & k, const Value & v, HashValue hv ) {
            first = k;
            second = v;
            hashValue = (hv << 2) | 0x1;
        }
        bool is_valid() const {
            return (0x1 & hashValue) && !( 0x2 & hashValue );
        }
    };

    //
    // Required specification {{{ ---------------------------------------------
public:
    // From additional specification (31/03/017) ------------------------------
	// suddenly, our custom extensions!
	// we will benchmark them against standard approach, and check if we can get better performance
	const VALUE & get(const KEY & k, const VALUE & default_value) const;

	// you are allowed (encouraged really) to tweak the iterator definition (and/or extend it as needed)
	// see usage example below in the text code; basically you will need to make ->key and ->value work
	struct myiterator
	{
    //protected:
        HashEntry * entry;
    //public:
		typedef HashEntry mypair;

        myiterator( HashEntry * e ) :
                    entry(e) {}

        myiterator & operator++() {
            (*this)++;
            return *this;
        }
        myiterator operator++(int) {
            //for( ++entry; entry->is_vacant(); ){ entry += 1; }
            do { ++entry; } while( (!entry->is_valid()) && (entry->hashValue != 0x3) );
            return *this;
        }
        HashEntry * operator->() { return entry; }
        HashEntry & operator*() { return *entry; }

        friend bool operator!= (const myiterator & l,
                                const myiterator & r) { return l.entry != r.entry; }

        friend bool operator== (const myiterator & l,
                                const myiterator & r) { return ! (l != r); }
	};

	myiterator find_first() { return begin(); }
	myiterator find_next(myiterator & it) { return it++; }
	bool is_valid(myiterator & it) { return it != end(); }
    // From original specification (29/03/017) --------------------------------
	// you are allowed to tweak the iterator definition (and extend it as needed)
	//struct iterator;  // TODO?
    typedef myiterator iterator;

    # if 1
    typedef iterator const_iterator;
    # else  // *sigh* Useful, but too ponderous to implement
    //operator const_iterator() const { return const_iterator(*this); }
    struct const_iterator {
        const Key & first;
        const Value & second;
        const HashEntry * entry;

        const_iterator( const HashEntry * e ) :
                    first( e->first ),
                    second( e->second ),
                    entry(e) {}
        const_iterator( const iterator & it ) :
                    first( it.first ),
                    second( it.second ),
                    entry( it.entry ) {}

        const_iterator & operator=( const iterator & );

        // TODO: optimize
        //friend bool operator== (const_iterator & l, const_iterator & r);
        //friend bool operator!= (const_iterator & l, const_iterator & r);
    };
    # endif
public:
	myhash();
	~myhash();

	VALUE & operator[](const KEY & k) { return at(k); }
	const VALUE & operator[](const KEY & k) const { return at(k); }

	iterator begin();
	iterator end() { return iterator(_table + table_size()); }
	const iterator begin() const;
	const iterator end() const { return iterator(_table + table_size()); }

	void erase(const KEY & k) { erase( find(k) ); }
	iterator find(const KEY & k);

	int size() const { return (int) _nOccupiedEntries; }
    // }}} Required specification ---------------------------------------------
    //
private:
    static HashFunction _hash_f;
    HashEntry * _table;
    mutable Size _latestSearchDepth;
    Size _tableSize,
         _fillmentThreshold,
         _nOccupiedEntries
         ;
protected:
    /// Inserts new element and returns iterator to newly inserted element.
    iterator _insert_element( const Key & k, const Value & v);
    /// Frees hash table, if was allocated.
    void _free();
    /// For open addressing: deletes hash table and re-inserts all the stuff.
    void _grow();
public:
    Value & at( const KEY & k );
    const Value & at( const KEY & k ) const;
    const_iterator find( const KEY & k ) const;
    Size table_size() const { return _tableSize; }

    void erase(const const_iterator & it);

    // XXX: for benchmarking purposes
    static void set_function( HashFunction f ) {
        _hash_f = f;
    }
    static HashValue hash_byteseq( const uint8_t * b, Size l )
                                                    { return _hash_f(b, l); }
};  // class myhash

// Implementation
////////////////

template<typename KEY, typename VALUE>
myhash<KEY, VALUE>::myhash() :
                _table( nullptr_C11 ),
                _latestSearchDepth( 0 ),
                _tableSize( 1 ),
                _fillmentThreshold( 0 ),
                _nOccupiedEntries( 0 ) { _grow(); }

template<typename KEY, typename VALUE>
myhash<KEY, VALUE>::~myhash() {
    if( !! _nOccupiedEntries ) {
        _free();
    }
}

template<typename KEY, typename VALUE> typename myhash<KEY, VALUE>::iterator
myhash<KEY, VALUE>::begin() {
    iterator it(_table);
    if( !it.entry->is_valid() ) { ++it; }
    return it;
}

template<typename KEY, typename VALUE> const typename myhash<KEY, VALUE>::iterator
myhash<KEY, VALUE>::begin() const {
    const_iterator it(_table);
    if( !it.entry->is_valid() ) { ++it; }
    return it;
}

template<typename KEY, typename VALUE> typename myhash<KEY, VALUE>::iterator
myhash<KEY, VALUE>::find(const KEY & k) {
    const Self * this_ = this;
    return iterator( const_cast<HashEntry *>(this_->find(k).entry) );
}

template<typename KEY, typename VALUE> typename myhash<KEY, VALUE>::iterator
myhash<KEY, VALUE>::_insert_element( const Key & k, const Value & v) {
    if( _nOccupiedEntries >= _fillmentThreshold ) {
        _grow();
    }
    HashValue hv = myhash_hash_spec<Key>(k);
    Size place = hv%table_size();
    // linear probing strategy:
    _latestSearchDepth = 0;
    while( !_table[place].is_vacant() ) {
        assert( _latestSearchDepth < table_size() );
        ++_latestSearchDepth;
        place = (place+1)%table_size();
        # ifndef NDEBUG
        if( _latestSearchDepth > table_size() ) {
            throw std::runtime_error( "Hash table seems busy, bot growth "
                "condition failed." );  // must never happen
        }
        # endif
    }
    _table[place].set( k, v, hv );
    printout( "> inserted %s->%d at %d w hash=%d\n", k.c_str(), v, place, hv );  // XXX
    ++_nOccupiedEntries;
    return iterator( _table + place );
}

template<typename KEY, typename VALUE> void
myhash<KEY, VALUE>::_free() {
    if( _table ) {
        delete [] _table;
    }
}

template<typename KEY, typename VALUE> void
myhash<KEY, VALUE>::_grow() {
    HashEntry * oldTable = _table,
              * oldTableEnd = _table + _tableSize;
    _nOccupiedEntries = 0;
    _tableSize <<= 2;
    _table = new HashEntry [_tableSize + 1];
    if( oldTable ) {
        for( const HashEntry * c = oldTable; oldTableEnd != c; ++c ) {
            if( !c->is_valid() ) {
                continue;
            }
            _insert_element( c->first, c->second );
        }
        delete [] oldTable;
    }
    _latestSearchDepth = 0;
    _fillmentThreshold = 0.7*_tableSize;
    _table[_tableSize].hashValue = 0x3;  // end marker
    printout( "> grown to %d, end=%p, %p\n",
                _tableSize, _table + _tableSize,
                iterator(_table + _tableSize).entry );  // XXX
}

template<typename KEY, typename VALUE> typename myhash<KEY, VALUE>::Value &
myhash<KEY, VALUE>::at( const KEY & k ) {
    printout( "> mutable at():\n" );  // XXX
    const_iterator it = find(k);
    if( end() == it ) {
        printout( "< mutable at() added new element.\n" );  // XXX
        return _insert_element( k, Value() )->second;
    }
    // The const validity was guaranteed by methods before, so the
    // const_cast<>() here seemd legit.
    printout( "< mutable at() returning existing element.\n" );  // XXX
    return iterator( const_cast<HashEntry *>( it.entry ) )->second;
}

template<typename KEY, typename VALUE> const typename myhash<KEY, VALUE>::Value &
myhash<KEY, VALUE>::at( const KEY & k ) const {
    printout( "> immutable at():\n" );  // XXX
    const_iterator it = find(k);
    if( end() == it ) {
        throw std::out_of_range( "Element not found." );
    }
    printout( "< immutable at().\n" );  // XXX
    return it;
}

template<typename KEY, typename VALUE> typename myhash<KEY, VALUE>::const_iterator
myhash<KEY, VALUE>::find( const KEY & k ) const {
    HashValue hv = myhash_hash_spec<Key>(k);
    Size place = hv%table_size();
    _latestSearchDepth = 0;
    printout( "> initial lookup state: hv=%d, place=%d, lsd=%d\n",
            hv, place, _latestSearchDepth);
    while( _latestSearchDepth < table_size()
        && !_table[place].is_vacant()
        /*&& hv%table_size() == ((_table[place].hashValue) >> 2)%table_size()*/ ) {
        if( myhash_equals<Key>( _table[place].first, k ) ) {
            if( !_table[place].is_released() ) {
                printout( "> have found %s at %d w val %d\n",
                        k.c_str(),
                        place,
                        (_table + place)->second );  // XXX
                return const_iterator( _table + place );
            }
            printout( "> omitting colliding %s at %d w val %d since it was erased\n",
                    k.c_str(),
                    place,
                    (_table + place)->second );  // XXX
        }
        printout( "* %s != %s\n", _table[place].first.c_str(), k.c_str() );
        place=(place+1)%table_size();
        ++_latestSearchDepth;
    }
    printout( "< final lookup state: hv=%d, place=%d, lsd=%d "
              " (latest hv=%d) (entry %s NOT FOUND)\n",
            hv, place, _latestSearchDepth, ((_table[place].hashValue) >> 2),
            k.c_str() );
    return end();
}

template<typename KEY, typename VALUE> void
myhash<KEY, VALUE>::erase(const const_iterator & it) {
    if( it.entry >= _table + table_size() || it.entry < _table ) {
        throw std::out_of_range( "Invalid iterator provided." );
    }
    --_nOccupiedEntries;
    printout( "> erased %s w val %d (%p)\n",
                it.entry->first.c_str(),
                it.entry->second,
                it.entry );  // XXX
    const_cast<HashEntry *>(it.entry)->release();
}

//__ This part has to be put into an implementation file //////////////////////
uint32_t adler32( const uint8_t * b, uint32_t l ) {
     uint32_t s1 = 1, s2 = 0;
     const uint8_t * bEnd = b + l;
     for( ; b < bEnd; ++b ) {
        s1 = (s1 + *b) % 65521;
        s2 = (s2 + s1) % 65521;
     }     
     return (s2 << 16) | s1;
}

uint32_t djb2( const uint8_t * b, uint32_t l ) {
    const uint8_t * bEnd = b + l;
    uint32_t v = 5381;
    for( ; b != bEnd; ++b){
        v = ((v << 5) + v) + *b;
    }
    return v;
}

uint32_t sdbm( const uint8_t * b, uint32_t l ) {
    const uint8_t * bEnd = b + l;
    uint32_t v = 0;
    for( ; b != bEnd; ++b ){
        v = *b + (v << 6) + (v << 16) - v;
    }
    return v;
}


template<typename KEY, typename VALUE>
typename myhash<KEY, VALUE>::HashFunction
myhash<KEY, VALUE>::_hash_f = adler32;

template<> uint32_t
myhash_hash_spec<std::string>( const std::string & v ) {
    return myhash<std::string, int>::hash_byteseq(
        (const uint8_t *) v.c_str(), v.size() );  // TODO
}

template<> bool
myhash_equals<std::string>( const std::string & l, const std::string & r ) {
    return l == r;
}
//^^ This part has to be put into an implementation file //////////////////////

# endif  // H_RDUS_MYHASH_H

