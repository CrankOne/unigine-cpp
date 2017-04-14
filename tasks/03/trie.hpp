# ifndef H_TRIE_H
# define H_TRIE_H

# include <cstdint>
# include <cassert>

# include <iostream>  // XXX: for dumps
# include <algorithm>  // XXX: for binary search

//
// aux

// The interesting thing is that it sometimes really does making things a
// little faster.
// To test, just switch the macro below:
# if 1
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
# else
// Returns code for given letter (ASCII)
char encode( char c ) {
    if( c < 'A' ) {
        return 0;
    } else if( c <= 'Z' ) {
        return c;
    } else if( c >= 'a' && c <= 'z' ) {
        return c;
    }
    return 0;
}

// Returns letter (ASCII) for given code
char decode( char b ) {
    if( b ) {
        return b;
    }
    return 0;
}
# endif

class Node {
private:
    uint32_t _counter;
    std::vector<char> _codes;
    std::vector<Node *> _childs;
public:
    # if 0
    struct Iterator {
    private:
        Node * _root;
        char * _c;
        std::vector<Node *>::iterator _nIt;
        /*mutable*/ std::stack<Node *> _ascendants;
    protected:
        Iterator( Node * root ) {
            _root = root;
            _c = _root->_codes.data();
            _nIt = _root->_childs.begin();
        }
    public:
        Iterator() : _root( nullptr ),
                     _c( nullptr ) {}

        Node * operator->() {
            return *_nIt;
        }

        Node & operator*() {
            return **_nIt;
        }

        Iterator & operator++() {
            (*this)++;
            return *this;
        }

        friend bool operator!= (const myiterator & l,
                                const myiterator & r) { return l._nIt != r._nIt; }

        friend bool operator== (const myiterator & l,
                                const myiterator & r) { return ! (l != r); }

        Iterator operator++(int) {
            //...
        }

        friend class Node;
    };  // struct iterator
    # endif
public:
    Node() : _counter(0) {}
    ~Node() {
        for( auto nodePtr : _childs ) {
            delete nodePtr;
        }
    }
    
    void inc_counter() { ++_counter; }
    size_t counter() const { return _counter; }

    /// Performs look-up for node indexed with given code. If fails, iserts a
    /// new one.
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

    // XXX: dev
    //void dump_recursively( const std::string & prfx ) {
    //    char *c = _codes.data();
    //    for( auto nIt = _childs.begin(); nIt != _childs.end(); ++nIt, ++c ) {
    //        //std::cout << prfx << decode(*c);
    //        (*nIt)->dump_recursively( prfx + decode(*c) );
    //        if( (*nIt)->counter() ) {
    //            std::cout << prfx + decode(*c)
    //                      << " (" << (*nIt)->counter() << ")"
    //                      << std::endl;
    //        }
    //    }
    //}
};


# endif  // H_TRIE_H
