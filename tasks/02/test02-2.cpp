#include <stdio.h>
#include <assert.h>

#include <string>
using namespace std;

///////////////////////////////////////////////////////////////////////////

#if 0

#define _TESTING_STD
#include <unordered_map>
#define myhash unordered_map

#else
# include "rdus.hpp"
#endif

///////////////////////////////////////////////////////////////////////////

void
main( int argc, const char * argv[] ) {
	/////////////
	// hash test
	/////////////

	{
		myhash<std::string,int> h;
		h["abc"] = 123;
		h["def"] = 456;
		assert(h.find("abc") != h.end());
		assert(h.find("def") != h.end());
		assert(h["abc"] == 123);
		assert(h["def"] == 456);
		h["abc"]++;
		assert(h["abc"] == 124);
		h.erase("abc");
		assert(h.find("abc") == h.end());
		h["abc"] = 789;
		assert(h["abc"] == 789);

		const myhash<std::string,int> & ch = h;
		assert(ch.find("abc") != ch.end());
//		assert(ch["abc"] == 789); // should NOT compile
		assert(ch.size() == 2);

		int r1 = 0, r2 = 0;
		for (auto it : h)
		{
			r1 += it.first.length();
			r2 += it.second;
		}
		assert(r1 == 6);
		assert(r2 == 789 + 456);

		for (auto it = h.begin(); it != h.end(); it++)
		{
			r2 -= it->second;
//			it->first = "fck"; // must NOT compile
			it->second = 0;
		}
		assert(r2 == 0);

		r2 = 0;
		for (auto & it : ch)
			r2 += it.second;
		assert(r2 == 0);

#ifdef _TESTING_STD
#else
		r2 = 0;
		for (auto it = h.find_first(); h.is_valid(it); it = h.find_next(it))
			r2 += it->key.length() + it->value;
#endif

	}
	printf("passed\n");
}
