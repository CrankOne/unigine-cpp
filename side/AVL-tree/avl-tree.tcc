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
class Pair {
    FirstT first;
    SecondT second;
};  // class Pair

template<typename KeyT,
         template<class> class TraitsT > bool
key_less( const typename TraitsT<KeyT>::Reference l,
          const typename TraitsT<KeyT>::Reference r );

template<typename KeyT,
         typename ValueT,
         typename NodeHeaderT=uint8_t>
class AVLTree {
public:
    typedef KeyT Key;
    typedef ValueT Value;

    typedef Key * KeyPtr;
    typedef Key & KeyRef;
    typedef Value * ValuePtr;
    typedef Value & ValueRef;

    typedef NodeHeaderT NodeHeader;

    struct Node;

    struct Node {
        Key key;
        Value value;
        NodeHeader hdr;
        
        Node * left,
             * right,
             * parent
             ;
        Node() : left(nullptr_C11),
                 right(nullptr_C11),
                 parent(nullptr_C11) {}
        ~Node() {
            if( left ) delete left;
            if( right ) delete right;
        }

        NodeHeader balance() const { return hdr; }
        void balance(NodeHeader nb) { hdr = nb; }
        bool is_leaf() const;
    };

    template<typename NodeT>
    class TIterator {
    private:
        NodeT * _c;
    protected:
        TIterator( NodeT * c ) : _c(c) {}
    };

    typedef TIterator<Node> Iterator;
    typedef TIterator<const Node> ConstIterator;
public:
    Pair<bool, ConstIterator> insert( const KeyRef, const ValueRef );
    ConstIterator find( const KeyRef ) const;
    void erase( ConstIterator );

    ConstIterator begin() const;  // TODO
    ConstIterator end() const;  // TODO
private:
    Node * _root;
protected:
    // rotations API
    static Node * _rotate_left( Node * );
    static Node * _rotate_left_big( Node * );
    static Node * _rotate_right( Node * );
    static Node * _rotate_right_big( Node * );
    // Basic internal API
    static ConstIterator _find_recursively( const Node *, const KeyRef );
    static ConstIterator _lower_bound( const Node *, const KeyRef );
};  // class AVLTree


// Standard AVL tree rotation routines

template<typename KeyT, typename ValueT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, NodeHeaderT>::_rotate_left( Node * a ) {
    Node * b = a->right;
    a->right = b->left;
    if( a->right ) { a->right->parent = a; }
    b->left = a;
    //
    b->parent = a->parent;
    a->parent = b;
    return b;
}

template<typename KeyT, typename ValueT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, NodeHeaderT>::_rotate_left_big( Node * n ) {
    n->right = _rotate_right( n->right );
    return _rotate_left( n );
}

template<typename KeyT, typename ValueT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, NodeHeaderT>::_rotate_right( Node * a ) {
    Node * b = a->left;
    a->left = b->right;
    if( a->left ) { a->left->parent = a; }
    b->right = a;
    //
    b->parent = a->parent;
    a->parent = b;
    return b;
}

template<typename KeyT, typename ValueT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, NodeHeaderT>::Node *
AVLTree<KeyT, ValueT, NodeHeaderT>::_rotate_right_big( Node * n ) {
    n->left = _rotate_left( n->left );
    return _rotate_right( n );
}

// Internal API

template<typename KeyT, typename ValueT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, NodeHeaderT>::ConstIterator
AVLTree<KeyT, ValueT, NodeHeaderT>::_find_recursively( const Node * n, const KeyRef k ) {
    if( key_less(n->key, k) ) {
        if( n->left ) {
            return _find_recursively( n->left, k );
        }
        return end();
    } else if( key_equal( n->key, k ) ) {
        return ConstIterator( n );
    } else if( n->right ) {
        return _find_recursively( n->right, k );
    }
    // Key greater, but current subtree has no right node.
    return end();
}

template<typename KeyT, typename ValueT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, NodeHeaderT>::ConstIterator
AVLTree<KeyT, ValueT, NodeHeaderT>::_lower_bound( const Node * n, const KeyRef k) {
    if( key_less( n->key, k ) ) {
        // v < current, so try to compare it with left, if possible:
        if( n->left ) {
            return _lower_bound( n->left, k );
        }
        // if impossible, return parent, since current < parent
        return 
    }
}

// General public API

template<typename KeyT, typename ValueT, typename NodeHeaderT>
typename AVLTree<KeyT, ValueT, NodeHeaderT>::ConstIterator
AVLTree<KeyT, ValueT, NodeHeaderT>::find( const KeyRef k ) const {
    if( !_root ) { return end(); }
    return _find_recursively( _root, k );
}

template<typename KeyT, typename ValueT, typename NodeHeaderT>
Pair<bool, typename AVLTree<KeyT, ValueT, NodeHeaderT>::ConstIterator>
AVLTree<KeyT, ValueT, NodeHeaderT>::insert( const KeyRef k, const ValueRef ) {

}

}  // advanced_containers

# endif  // H_QCRYPT_AVL_TREE_H

