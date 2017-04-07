# ifndef H_QCRYPT_AVL_TREE_H
# define H_QCRYPT_AVL_TREE_H

# include <cstdlib>
# include <inttypes.h>
# include <stdexcept>

# if __cplusplus <= 199711L
# define nullptr_C11 NULL
# else
# define nullptr_C11 nullptr
# endif

namespace advanced_containers {

class AVLTreeException : public std::exception {
    // ...
};

template<typename FirstT, typename SecondT>
struct Pair {
    FirstT first;
    SecondT second;
    Pair( const FirstT & f, const SecondT & s ) : first(f), second(s) {}
};  // class Pair

template<typename KeyT>
struct UsualKeyTraits {
    typedef KeyT Type;

    typedef Type & Reference;
    typedef const Type & ConstReference;

    typedef Type * Pointer;
    typedef const Type * ConstPointer;
};

template<typename KeyT,
         template<class> class TraitsT=UsualKeyTraits >
struct KeyCompare {
    static bool less( typename TraitsT<KeyT>::ConstReference l,
                      typename TraitsT<KeyT>::ConstReference r ) {
        return l < r;
    }
    static bool equal( typename TraitsT<KeyT>::ConstReference l,
                       typename TraitsT<KeyT>::ConstReference r ) {
        return l == r;
    }
};

template<typename T> using StdKeyCompare = KeyCompare<T>;

template<typename KeyT,
         typename ValueT,
         template<class> class KeyCompareT=StdKeyCompare,
         typename NodeHeaderT=int8_t>
class AVLTree {
public:
    typedef KeyT Key;
    typedef ValueT Value;

    typedef Key * KeyPtr;
    typedef Key & KeyRef;
    typedef const KeyRef ConstKeyRef;
    typedef Value * ValuePtr;
    typedef Value & ValueRef;
    typedef KeyCompareT<Key> KeyCompare;

    typedef NodeHeaderT NodeHeader;

    typedef AVLTree<Key, Value, KeyCompareT, NodeHeader> Self;

    struct Node : public Pair<Key, Value> {
        typedef Pair<Key, Value> Parent;

        NodeHeader hdr;
        
        Node * left,
             * right,
             * parent
             ;
        Node() : hdr(0),
                 left(nullptr_C11),
                 right(nullptr_C11),
                 parent(nullptr_C11) {}
        Node( ConstKeyRef k, const Value & v ) :
                 Parent(k, v),
                 hdr(0),
                 left(nullptr_C11),
                 right(nullptr_C11),
                 parent(nullptr_C11) {}
        ~Node() {
            if( left ) delete left;
            if( right ) delete right;
        }

        NodeHeader balance() const { return hdr; }
        void balance(NodeHeader nb) { hdr = nb; }
        bool is_leaf() const;
        bool is_left_heavy() const { return 0 > balance(); }
        bool is_right_heavy() const { return 0 < balance(); }
        bool is_balanced() const { return !balance(); }
        void inc_balance() { hdr--; }
        void dec_balance() { hdr++; }
    };

    template<typename NodeT>
    class TIterator {
    private:
        NodeT * _c;
    protected:
        TIterator( NodeT * c ) : _c(c) {}
        friend Self;
    public:
        TIterator<NodeT> & operator++() {
            if( _c->right ) {
                _c = _c->right;
            } else if( _c->parent ) {
                _c = _c->parent;
            }
            return *this;
        };

        bool operator!=( TIterator<NodeT> & it ) const {
            return it._c != this->_c;
        }

        bool operator==( TIterator<NodeT> & it ) const {
            return ! ( *this != it );
        }

        const Pair<Key, Value> * operator->(){
            return _c;
        }
    };

    typedef TIterator<Node> Iterator;
    typedef TIterator<const Node> ConstIterator;
public:
    AVLTree() : _root(nullptr) {}

    Pair<bool, ConstIterator> insert( ConstKeyRef, const ValueRef );
    ConstIterator find( ConstKeyRef ) const;
    void erase( ConstIterator );

    ConstIterator begin() const {
        if( !_root ) return end();
        const Node * n;
        for( n = _root; n->left; n = n->left ){}
        return ConstIterator(n);
    }
    ConstIterator end() const {
        return ConstIterator( (Node *) 0x1 );  // TODO: better ideas?
    }
private:
    Node * _root;
protected:
    // rotations API
    static Node * _rotate_left( Node * );
    static Node * _rotate_left_big( Node * );
    static Node * _rotate_right( Node * );
    static Node * _rotate_right_big( Node * );
    // Basic internal API
    static ConstIterator _find_recursively( const Node *, ConstKeyRef );
    static ConstIterator _lower_bound( const Node *, ConstKeyRef, bool wentRight );
    static Pair<bool, ConstIterator> _insert( Node *, ConstKeyRef, const ValueRef );
    static void _rebalance( Node * );
};  // class AVLTree


// Standard AVL tree rotation routines

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rotate_left( Node * a ) {
    Node * b = a->right;
    a->right = b->left;
    if( a->right ) { a->right->parent = a; }
    b->left = a;
    //
    b->parent = a->parent;
    a->parent = b;
    return b;
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rotate_left_big( Node * n ) {
    n->right = _rotate_right( n->right );
    return _rotate_left( n );
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rotate_right( Node * a ) {
    Node * b = a->left;
    a->left = b->right;
    if( a->left ) { a->left->parent = a; }
    b->right = a;
    //
    b->parent = a->parent;
    a->parent = b;
    return b;
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rotate_right_big( Node * n ) {
    n->left = _rotate_left( n->left );
    return _rotate_right( n );
}

// Internal API

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::ConstIterator
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_find_recursively( const Node * n, ConstKeyRef k ) {
    if( key_less(n->first, k) ) {
        if( n->left ) {
            return _find_recursively( n->left, k );
        }
        return end();
    } else if( KeyCompare::equal( n->first, k ) ) {
        return ConstIterator( n );
    } else if( n->right ) {
        return _find_recursively( n->right, k );
    }
    // Key greater, but current subtree has no right node.
    return end();
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::ConstIterator
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_lower_bound( const Node * n, ConstKeyRef k, bool wentRight ) {
    // TODO: check!
    if( key_less( k, n->first ) ) {
        if( n->left ) {
            return _lower_bound( n->left, k, wentRight ); // go left
        }
        if( wentRight ) {
            return ConstIterator( n );
        }
        return end();  // all keys lies before given
    } else if( KeyCompare::equal( k, n->first ) ) {
        return ConstIterator( n );
    } else {
        if( n->right ) {
            return _lower_bound( n->right, k, true );  // go right
        }
        return ConstIterator( n );
    }
}

// General public API

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::ConstIterator
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::find( ConstKeyRef k ) const {
    if( !_root ) { return end(); }
    return _find_recursively( _root, k );
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
Pair<bool, typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::ConstIterator>
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::insert( ConstKeyRef k, const ValueRef v ) {
    if( !_root ) {
        _root = new Node( k, v );
        return Pair<bool, ConstIterator>( true, ConstIterator(_root) );
    }
    return _insert( _root, k, v );
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
Pair<bool, typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::ConstIterator>
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_insert( Node * n, ConstKeyRef k, const ValueRef v ) {
    if( KeyCompare::equal(k, n->first) ) {
        // insertion failure --- element exists:
        return Pair<bool, ConstIterator>( false, ConstIterator(n) );
    }
    if( KeyCompare::less( k, n->first ) ) {
        if( n->left ) {
            return _insert( n->left, k, v );
        } else {
            n->dec_balance();
            n->left = new Node( k, v );  // TODO: alloc
            n->left->parent = n;
            _rebalance(n);
            return Pair<bool, ConstIterator>( true, ConstIterator(n->left) );
        }
    } else {
        if( n->right ) {
            return _insert( n->right, k, v );
        } else {
            n->inc_balance();
            n->right = new Node( k, v );  // TODO: alloc
            n->right->parent = n;
            _rebalance(n);
            return Pair<bool, ConstIterator>( true, ConstIterator(n->right) );
        }
    }
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT> void
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rebalance( Node * n ) {
    // Given node has a valid balance --- has to re-trace its ancestors.
    while( n->parent ) {
        n = n->parent;

        NodeHeader bDiff = (n->right ? abs(n->right->balance()) : 0 )
                         - (n->left  ? abs(n->left->balance())  : 0 )
                         ;

        if( -2 == bDiff ) {
            // height(n->left->left) < height(n->left->right) => _rotate_right_big(n)
            // else rotate_right(n)
            if( n->left->balance() > 0 ) {
                _rotate_right_big( n );
            } else {
                _rotate_right( n );
            }
        } else if( 2 == bDiff ) {
            // height(n->right->right) < height(n->right->left) < _rotate_left_big(n)
            // else rotate_left(n)
            if( n->right->balance() > 0 ) {
                _rotate_left_big( n );
            } else {
                _rotate_left( n );
            }
        }
    }
}

}  // advanced_containers

# endif  // H_QCRYPT_AVL_TREE_H

