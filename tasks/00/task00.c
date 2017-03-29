
# include <stdlib.h>
# include <ctype.h>
# include <stdio.h>
# include <string.h>

/** A custom version of standard `isalpha()' since messeur Aksyonoff insisted
 * using it is better.
 * */
char
isalpha_( const char c ) {
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        ? c : 0 ;
}

struct TokensList {
    size_t nOccurs;
    const char * str;
    struct TokensList * next;
};

struct TokensList *
token_new( const char * bgn ) {
    struct TokensList * l =
                    (struct TokensList *) malloc( sizeof(struct TokensList) );
    l->str = bgn;
    l->next = NULL;
    l->nOccurs = 1;
    return l;
}

void
token_free( struct TokensList * l ) {
    free( l );
}

void
token_list_dump( FILE * fd, struct TokensList * l ) {
    struct TokensList * cl;
    for( cl = l; cl; cl = cl->next ) {
        # if 1
        fprintf( fd, "%zu %s\n", cl->nOccurs, cl->str );
        # else
        fprintf( fd, "%p -> %zu \"%s\"\n", cl, cl->nOccurs, cl->str );
        # endif
    }
}

void
qs( struct TokensList * hd,
    struct TokensList * tl,
    int (*leq)(struct TokensList *, struct TokensList *),
    struct TokensList ** rtn) {
    /* See original at:
     * http://stackoverflow.com/questions/14805936/optimal-quicksort-for-single-linked-list
     */
    int nlo, nhi;
    struct TokensList *lo, *hi, *q, *p;
    /* Invariant: return head sorted with `tl' appended. */
    while (hd != NULL) {
        nlo = nhi = 0;
        lo = hi = NULL;
        q = hd;
        p = hd->next;
        /* Start optimization for O(n) behavior on sorted and reverse-of-sorted
         * lists */
        while( p != NULL && leq(p, hd) ) {
            hd->next = hi;
            hi = hd;
            ++nhi;
            hd = p;
            p = p->next;
        }
        /* If entire list was ascending, we're done. */
        if (p == NULL) {
            *rtn = hd;
            hd->next = hi;
            q->next = tl;
            return;
        }
        /* End optimization.  Can be deleted if desired. */
        /* Partition and count sizes. */
        while( p != NULL ) {
            q = p->next;
            if( leq(p, hd) ) {
                p->next = lo;
                lo = p;
                ++nlo;
            } else {
                p->next = hi;
                hi = p;
                ++nhi;
            }
            p = q;
        }
        /* Recur to establish invariant for sublists of hd, 
           choosing shortest list first to limit stack. */
        if( nlo < nhi ) {
            qs(lo, hd, leq, rtn);
            rtn = &hd->next;
            hd = hi;        /* Eliminated tail-recursive call. */
        } else {
            qs(hi, tl, leq, &hd->next);
            tl = hd;
            hd = lo;        /* Eliminated tail-recursive call. */
        }
    }
    /* Base case of recurrence. Invariant is easy here. */
    *rtn = tl;
}

/* This function implies that incoming list is sorted and, like Linux `uniq'
 * command, excludes only duplicates following in a row. */
void
count_freqs( struct TokensList * l ) {
    struct TokensList * interim, * cl;
    for( cl = l; cl->next; cl = cl->next ) {
        while( !strcmp( cl->str, cl->next->str ) ) {
            ++cl->nOccurs;
            interim = cl->next;
            cl->next = cl->next->next;
            token_free( interim );
        }
    }
}

static int
_static_leq_lexical( struct TokensList * l,
                     struct TokensList * r ) {
    return (!(strcmp(l->str, r->str) > 0)) ? -1 : 0;
}

static int
_static_leq_freq( struct TokensList * l,
                  struct TokensList * r ) {
    if( l->nOccurs != r->nOccurs ) {
        return (!(l->nOccurs < r->nOccurs)) ? -1 : 0;
    }
    return _static_leq_lexical(l, r);
}

struct TokensList *
tokenize_text( char * text ) {
    struct TokensList * head = NULL,
                      * last = NULL;
    const char * tokBgn = NULL;
    char * c;
    for( c = text; '\0' != *c; ++c ) {
        if( isalpha_(*c) ) {
            if( !tokBgn ) {
                tokBgn = c;
            }
            *c = tolower(*c);
        } else {
            if( tokBgn ) {
                *c = '\0';
                struct TokensList * t = token_new( tokBgn );
                if( head ) {
                    last->next = t;
                    last = t;
                } else {
                    head = last = t;
                }
                tokBgn = NULL;
            }
        }
    }
    /* This will yield list with repeatative occurencies: */
    qs( head, NULL, _static_leq_lexical, &head );
    /* This will mege repeatative occurencies */
    count_freqs( head );
    /* This will re-sort the list with freqs comparison operator */
    qs( head, NULL, _static_leq_freq, &head );
    return head;
}

int
main( int argc, const char * argv[] ) {
    FILE * infd, * outfd;
    char * inp = NULL;
    struct TokensList * l;
    size_t bfLength;

    if( 3 != argc ) {
        fprintf( stderr, "Error: wrong cmd-line arguments number.\n"
                         "Usage:\n"
                         "  $ %s <in-filename> <out-filename>\n",
                argv[0] );
        return EXIT_FAILURE;
    }

    infd = fopen( argv[1], "rb" );
    fseek( infd, 0, SEEK_END );
    bfLength = ftell( infd );
    if( !! (inp = (char *) malloc( bfLength + 1 )) ) {
        fseek( infd, 0, SEEK_SET );
        fread( inp, 1, bfLength, infd );
        inp[bfLength] = '\0';
    }
    fclose( infd );
    if( !inp ) {
        fprintf( stderr, "Memory allocation error failed to read all file of "
            "size %zu bytes at once.\n", bfLength );
        return EXIT_FAILURE;
    }
    l = tokenize_text( inp );

    if( !!(outfd = fopen( argv[2], "w" ) ) ){
        token_list_dump( outfd, l );
        fclose(outfd);
    }

    if( inp ) {
        free( inp );
    }
    return EXIT_SUCCESS;
}

