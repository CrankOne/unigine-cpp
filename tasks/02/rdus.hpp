# ifndef H_RDUS_MYHASH_H
# define H_RDUS_MYHASH_H

template<typename T> myhash_hash_spec( const T & );

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
        Key key;
        HashValue hashValue;
        Value value;
    };
    static HashFunction demoFunctions[8];  // whatever...

    //
    // Required specification {{{ ---------------------------------------------
public:
	// you are allowed to tweak the iterator definition (and extend it as needed)
	struct iterator
	{
		const KEY & first;
		VALUE & second;
		HashEntry * _entry;
	};

public:
	myhash();
	~myhash();

	VALUE & operator[](const KEY & k) { return at(k); }
	const VALUE & operator[](const KEY & k) const { return at(k); }

	iterator begin();
	iterator end();
	const iterator begin() const;
	const iterator end() const;

	void erase(const KEY & k) {};
	iterator * find(const KEY & k);

	int size() const;
    // }}} Required specification ---------------------------------------------
    //
private:
    HashFunction _hash_f;
    Entry * _table;
    Size _tableSize,
         _fillmentThreshold
         ;
protected:
    Size placement_hint( const Key & k ) {
        return myhash_hash_spec<Key>(k)%table_size();
    }

    iterator _insert_element( const Key & k, const Value & v) {
        Size place = placement_hint(),
             searchDepth = 0;
        // linear probing strategy:
        while( !_table[place].is_vacant() ) {
            ++searchDepth;
            assert( searchDepth < _tableSize );
            place = (place+1)%table_size();
        }
        _table[place]
        if( searchDepth > _fillmentThreshold ) {
            _grow();
        }
    }

    void _grow();
public:
    Value & at( const KEY & k );
    const Value & find( const KEY & k ) const;

    const_iterator & find( const KEY & k ) const {
        HashValue hv = myhash_hash_spec<Key>(k);
        Size place = hv%table_size(),
             searchDepth = 0;
        while( searchDepth < table_size() ) {
            place = (place+1)%table_size();
        }
        return end();
    }
    Size table_size() const { return _tableSize; }

    // XXX: for benchmarking purposes
    static void set_function( HashFunction );    
    static HashValue hash_byteseq( const uint8_t * b, Size l )
                                                    { return _hash_f(b, l); }
};

# endif  // H_RDUS_MYHASH_H

