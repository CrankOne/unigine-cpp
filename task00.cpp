// Unfortunately, I couldn't test this code under MSVC, but both GCC/Clang
// tells everything is ok.

# include <cstdlib>
# include <iostream>
# include <fstream>
# include <map>
# include <list>
# include <algorithm>
# include <set>

typedef std::map<std::string, size_t> WordFreqs;
typedef std::list<std::string> Tokens;
typedef std::pair<std::pair<size_t, std::string>, std::string> SortedEntry;

std::list<std::string>
tokenize_line( const std::string & line ) {
    Tokens tokens;
    std::string::const_iterator cWordBgn = line.end();
    for( std::string::const_iterator it  = line.begin();
                                     it != line.end(); ++it ) {
        if( std::isalnum(*it) ) {
            if(line.end() == cWordBgn) {
                cWordBgn = it;
            }
        } else {
            if(line.end() != cWordBgn) {
                std::string token(cWordBgn, it);
                std::transform(token.begin(), token.end(), token.begin(), ::tolower);
                tokens.push_back( token );
                cWordBgn = line.end();
            }
        }
    }
    if( line.end() != cWordBgn ) {
        std::string token(cWordBgn, line.end());
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        tokens.push_back( token );
    }
    return tokens;
}

void
append_frequencies( WordFreqs & freqs,
                    const std::list<std::string> & tokens ) {
    for( Tokens::const_iterator it  = tokens.begin();
                                it != tokens.end(); ++it ) {
        std::pair<WordFreqs::iterator, bool> wItRes = freqs.insert(
                                                WordFreqs::value_type(*it, 0) );
        wItRes.first->second++;
    }
}

static SortedEntry
transpose_pair( const std::pair<std::string, size_t> & o ) {
    return std::make_pair(
            std::make_pair(o.second, o.first),
            o.first );
}

// Without this comparison functor std::multimap works just fine using
// usual std::less, but order of words/numbers are somehow
// anti-symmetric w.r.t required one.
struct CustomCompare {
    bool operator()( const SortedEntry::first_type & a,
                     const SortedEntry::first_type & b ) {
        if( a.first < b.first ) {
            return true;
        }
        if( a.first > b.first ) {
            return false;
        }
        return 0 < a.second.compare( b.second );
    }
};

int
main(int argc, const char * argv[]) {
    if( 3 != argc ) {
        std::cerr << "Error: wrong cmd-line arguments number." << std::endl
                  << "Usage:" << std::endl
                  << "  $ " << argv[0] << " <in-filename> <out-filename>"
                  << std::endl;
        return EXIT_FAILURE;
    }

    WordFreqs wFreqs;
    {
        std::ifstream iFile( argv[1] );
        for(std::string line; std::getline(iFile, line, '\n');) {
            append_frequencies( wFreqs, tokenize_line(line) );
        }
    }

    {
        std::multimap<SortedEntry::first_type,
                      SortedEntry::second_type,
                      CustomCompare> sortedOut;
        std::transform( wFreqs.begin(), wFreqs.end(),
                       std::inserter( sortedOut, sortedOut.end() ),
                       transpose_pair );
        
        std::ofstream oFile (argv[2]);

        // In C++11 we would just for(auto & it : sortedOut) { ... }
        for( std::multimap<SortedEntry::first_type,
                           SortedEntry::second_type,
                           CustomCompare>::const_reverse_iterator
                        it  = sortedOut.rbegin();
                        it != sortedOut.rend(); ++it ) {
            oFile << it->first.first << " "
                  << it->second << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
