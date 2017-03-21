/* non-stlfreq: the plain-C (almost) implementation of the word-frequency
 * gathering app. Follows the same specification as initial task except for
 * restricted usage of STL templates: only std::sort() was allowed.
 *
 * I've found it extremely boring to invent the wheel, and have trying to
 * entertain mysalf a little by implementing this task on plain C which is
 * always a bit of challenging thing, IYKWIM.
 */

# include <stdlib.h>
# include <string.h>
# include <assert.h>

/** A custom version of standard `isalpha()' since messeur Aksyonoff insisted
 * using it is better.
 * */
char
isalpha_( const char c ) {
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        ? c : 0 ;
}

/** A plain-C representation of token met at the incoming data. */
struct Token {
    size_t nOccurs;
    char * str;
};

/** Internally uses malloc() invokation to construct a list entry corresponding
 * to specific token. Complementary with dtr function `token_free()'. */
struct Token *
token_new( const char * tBgn, const char * tEnd ) {
    struct Token * tokPtr = (struct Token *) malloc( sizeof(struct Token) );
    /* Note: bzero() and strndup() is a POSIX extensions and aren't available
     * under MSVC. */
    /* TODO: tolower */
    bzero( tokPtr, sizeof(struct Token) );
    tokPtr->str = strndup( tBgn, tEnd - tBgn );
}

/** Dtr function cleaning a single token entry. */
void
token_delete( struct Token * tokPtr ) {
    assert( tokPtr );
    assert( tokPtr->str );
    free( tokPtr->str );
    free( tokPtr );
}

/** Token look-up cache.
 *
 * Well, most suitable here would be the red-black tree, but they forbid us
 * to use them. Look-up in a simple binary tree at the worst case is still
 * better than plain comparison. One may at any time implement rebalancing
 * procedure once being concerning about performance anyway...
 * */
struct TokenBinTree {
    struct Token * token;
    struct TokenLookUpCache * left, * right;
};

struct TokenBinTree *
tbtree_new( struct Token * tokPtr ) {
    struct TokenBinTree * r =
                (struct TokenBinTree *) malloc(sizeof(struct TokenBinTree));
    bzero( r, sizeof(struct TokenBinTree) );
    r->token = tokPtr;
    return r;
}

/* The two pfunctions below are used in tight combination with
 * `tokenize_text()' to perform navigation and insertion of acquired tokens
 * with state object.
 */

static void
_tbtree_consider_letter(
                struct TokenBinTree * root,
                struct TokenBinTree ** state,
                const char * tBgn,
                const char * c ) {
    if( !root ) return;  /* Deny navigation at empty tree */
    
}

static void
_tbtree_consider_token(
                struct TokenBinTree ** state,
                const char * tBgn,
                const char * tEnd ) {
    /* TODO */
    /* *state = tbtree_new( token_new( tBgn, tEnd ) ); */
}

struct TokenBinTree *
tokenize_text( const char * text ) {
    /* TODO: state has to become an another structure */
    struct TokenBinTree * root = NULL,
                        * state = NULL;
    const char * tokBgn = NULL;
    for( const char * c = text; '\0' != *c; ++c ) {
        if( isalpha_(*c) ) {
            if( !tokBgn ) {
                tokBgn = c;
            }
            _tbtree_consider_letter( root, &state, tokBgn, c );
        } else {
            if( tokBgn ) {
                _tbtree_consider_token( &state, tokBgn, c );
                if( !root ) {
                    root = state;
                }
                /* Reset state upon insertion of token */
                state = NULL;
            }
        }
    }
    return root;
}

