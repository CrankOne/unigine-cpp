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
    typedef Value * ValuePtr;
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
        Node( const Key & k, const Value & v ) :
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
        void inc_balance() { ++hdr; }
        void dec_balance() { --hdr; }
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
                for(_c = _c->right; _c->left; _c = _c->left){}
            }
            for( _c = _c->parent; _c && _c->right; _c = _c->right ) {}
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

    Pair<bool, ConstIterator> insert( const Key &, const Value & );
    ConstIterator find( const Key & ) const;
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

    // XXX
    static void XXX_dump( const Node * n, size_t ns ) {
        if( !n ) {
            std::cout << std::string( ns, '\t' );
            std::cout << "-" << std::endl;
            return;
        }
        XXX_dump( n->left,  ns+1 );
        std::cout << std::string( ns, '\t' )
                  << n->first << "(" << (int) n->balance() << ")<" << std::endl;
        XXX_dump( n->right, ns+1 );
    }

    // XXX
    void dump_self() const {
        XXX_dump( _root, 0 );
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
    static ConstIterator _find_recursively( const Node *, const Key & );
    static ConstIterator _lower_bound( const Node *, const Key &, bool wentRight );
    static Pair<bool, ConstIterator> _insert( Node *, const Key &, const Value &, Node *& root );
    static void _rebalance( Node *, Node *& root );
};  // class AVLTree


// Standard AVL tree rotation routines

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rotate_left( Node * a ) {
    // TODO: re-set balances!
    Node * b = a->right;
    a->right = b->left;
    
    if( a->right ) {
        a->right->parent = a;
    }

    b->left = a;
    //
    b->parent = a->parent;
    a->parent = b;

    b->dec_balance();
    a->dec_balance();

    return b;
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rotate_left_big( Node * n ) {
    n->right = _rotate_right( n->right );
    Node * p = _rotate_left( n );
    p->dec_balance();
    n->dec_balance();
    return p;
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rotate_right( Node * a ) {
    // TODO: re-set balances!
    Node * b = a->left;
    a->left = b->right;

    if( a->left ) {
        a->left->parent = a;
    }

    b->right = a;
    //
    b->parent = a->parent;
    a->parent = b;

    a->inc_balance();
    b->inc_balance();

    return b;
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rotate_right_big( Node * n ) {
    n->left = _rotate_left( n->left );
    Node * p = _rotate_right( n );
    p->inc_balance();
    n->inc_balance();
    return p;
}

// Internal API

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::ConstIterator
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_find_recursively( const Node * n, const Key & k ) {
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
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_lower_bound( const Node * n, const Key & k, bool wentRight ) {
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
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::find( const Key & k ) const {
    if( !_root ) { return end(); }
    return _find_recursively( _root, k );
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
Pair<bool, typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::ConstIterator>
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::insert( const Key & k, const Value & v ) {
    if( !_root ) {
        _root = new Node( k, v );
        return Pair<bool, ConstIterator>( true, ConstIterator(_root) );
    }
    return _insert( _root, k, v, _root );
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT>
Pair<bool, typename AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::ConstIterator>
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_insert(
                    Node * n, const Key & k, const Value & v, Node *& rwp ) {
    if( KeyCompare::equal(k, n->first) ) {
        // insertion failure --- element exists:
        return Pair<bool, ConstIterator>( false, ConstIterator(n) );
    }
    if( KeyCompare::less( k, n->first ) ) {
        if( n->left ) {
            auto p = _insert( n->left, k, v, n->left );
            if( p.first ) {
                //n->dec_balance();
                if( -1 == n->balance() ) {
                    if( n->left->balance() > 0 ) {
                        std::cout << ">>> R" << (int) n->first << std::endl;  // XXX
                        //XXX_dump( n,  6 );  // XXX
                        rwp = _rotate_right_big( n );
                    } else {
                        std::cout << ">>> r" << (int) n->first << std::endl;  // XXX
                        //XXX_dump( n,  6 );  // XXX
                        rwp = _rotate_right( n );
                    }
                } else if( n->left->balance() < 0 ) {
                    n->dec_balance();
                }
            }
            return p;
        } else {
            n->left = new Node( k, v );  // TODO: alloc
            n->left->parent = n;
            n->dec_balance();
            //_rebalance(n, root);
            return Pair<bool, ConstIterator>( true, ConstIterator(n->left) );
        }
    } else {
        if( n->right ) {
            auto p = _insert( n->right, k, v, n->right );
            if( p.first ) {
                //n->inc_balance();
                if( 1 == n->balance() ) {
                    if( n->right->balance() < 0 ) {
                        std::cout << ">>> L" << (int) n->first << std::endl;  // XXX
                        rwp = _rotate_left_big( n );
                    } else {
                        std::cout << ">>> l" << (int) n->first << std::endl;  // XXX
                        rwp = _rotate_left( n );
                    }
                } else if( n->right->balance() > 0 ) {
                    n->inc_balance();
                }
            }
            return p;
        } else {
            n->right = new Node( k, v );  // TODO: alloc
            n->right->parent = n;
            n->inc_balance();
            //_rebalance(n, root);
            return Pair<bool, ConstIterator>( true, ConstIterator(n->right) );
        }
    }
}

template<typename KeyT, typename ValueT, template<class> class KeyCompareT, typename NodeHeaderT> void
AVLTree<KeyT, ValueT, KeyCompareT, NodeHeaderT>::_rebalance( Node * n, Node *& root ) {
    // Given node has a valid balance --- has to re-trace its ancestors.
    std::cout << "rebalance(" << n->first << "):" << std::endl;
    XXX_dump( root, 1 );
    while( n->parent ) {
        n = n->parent;

        NodeHeader bDiff = (n->right ? 2*abs(n->right->balance()) : 0 )
                         - (n->left  ? 2*abs(n->left->balance())  : 0 )
                         ;
        //n->balance( bDiff );
        std::cout << "Rec-d bal for (" << n->first << ")=" << (int) bDiff << std::endl;
        XXX_dump( root, 1 );

        if( -2 == bDiff ) {
            // height(n->left->left) < height(n->left->right) => _rotate_right_big(n)
            // else rotate_right(n)
            if( n->left->balance() > 0 ) {
                std::cout << ">>> R" << (int) n->first << std::endl;  // XXX
                _rotate_right_big( n );
            } else {
                std::cout << ">>> r" << (int) n->first << std::endl;  // XXX
                _rotate_right( n );
                XXX_dump( n->parent, 1 );  // XXX
            }
        } else if( 2 == bDiff ) {
            // height(n->right->right) < height(n->right->left) < _rotate_left_big(n)
            // else rotate_left(n)
            if( n->right->balance() < 0 ) {
                std::cout << ">>> L" << (int) n->first << std::endl;  // XXX
                _rotate_left_big( n );
            } else {
                std::cout << ">>> l" << (int) n->first << std::endl;  // XXX
                _rotate_left( n );
            }
        }
    }
    root = n;
    XXX_dump( root, 1 );
}

}  // advanced_containers

# endif  // H_QCRYPT_AVL_TREE_H

