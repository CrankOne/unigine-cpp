
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
    l->nOccurs = 0;
    return l;
}

void
token_free( struct TokensList * l ) {
    free( l );
}

void
qs( struct TokensList * hd,
    struct TokensList * tl,
    struct TokensList ** rtn) {
    /* See original at:
     * http://stackoverflow.com/questions/14805936/optimal-quicksort-for-single-linked-list
     */
    int nlo, nhi;
    struct TokensList *lo, *hi, *q, *p;
    /* Invariant:  Return head sorted with `tl' appended. */
    while (hd != NULL) {
        nlo = nhi = 0;
        lo = hi = NULL;
        q = hd;
        p = hd->next;
        /* Start optimization for O(n) behavior on sorted and reverse-of-sorted lists */
        while (p != NULL && strcmp(p->str, hd->str) < 0 ) {
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
        while (p != NULL) {
            q = p->next;
            if (strcmp(p->str, hd->str) < 0) {
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
        if (nlo < nhi) {
            qs(lo, hd, rtn);
            rtn = &hd->next;
            hd = hi;        /* Eliminated tail-recursive call. */
        } else {
            qs(hi, tl, &hd->next);
            tl = hd;
            hd = lo;        /* Eliminated tail-recursive call. */
        }
    }
    /* Base case of recurrence. Invariant is easy here. */
    *rtn = tl;
}

struct TokensList *
tokenize_text( char * text ) {
    struct TokensList * head = NULL,
                      * last = NULL;
    const char * tokBgn = NULL;
    for( char * c = text; '\0' != *c; ++c ) {
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
    qs( head, NULL, &head );
    return head;
}

int
main( int argc, const char * argv[] ) {
    char inp[] = "\"The time has come,\" the Walrus said,\n"
                 "To talk of many things:";

    struct TokensList * l = tokenize_text( inp );

    for( struct TokensList * cl = l; cl; cl = cl->next ) {
        printf( "%p -> \"%s\"\n", cl, cl->str );
    }
    return EXIT_SUCCESS;
}

