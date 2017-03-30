#include <stdio.h>
#include <assert.h>

#include <string>
using namespace std;

///////////////////////////////////////////////////////////////////////////

#if 1

#include <unordered_map>
#define myhash unordered_map

#else

// WARNING
// the example class declaration below will be replaced by #include "xxxx.h"

template<typename KEY, typename VALUE>
class myhash
{
public:
	// you are allowed to tweak the iterator definition (and extend it as needed)
	struct iterator
	{
		const KEY & first;
		VALUE & second;
		// ...
	};

public:
	myhash();
	~myhash();

	VALUE & operator[](const KEY & k);
	const VALUE & operator[](const KEY & k) const;

	iterator * begin();
	iterator * end();
	const iterator * begin() const;
	const iterator * end() const;

	void erase(const KEY & k) {};
	iterator * find(const KEY & k);

	int size() const;
};
#endif

///////////////////////////////////////////////////////////////////////////

void main()
{
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
		assert(h.find("abc") != h.end());
		assert(h["abc"] == 789);
		assert(h.size() == 2);

		int r1 = 0, r2 = 0;
		for (const auto & it : h)
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
	}
	printf("passed\n");
}
