#include "pch.h"
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

//#define USE_STD_ALLOCATOR

#ifdef USE_STD_ALLOCATOR

using String = std::string;
template<typename K, typename V>
using Map = std::map<K, V>;
template<typename V>
using Vector = std::vector<V>;

#else

#include "inblock_allocator.hpp"
struct holder {
	static inblock_allocator_heap heap;
};

inblock_allocator_heap holder::heap;

using String = std::
basic_string<char, std::char_traits<char>, inblock_allocator<char, holder>>;

template<typename K, typename V>
using Map =
std::map<K, V, std::less<K>, inblock_allocator<std::pair<K, V>, holder>>;

template<typename V>
using Vector = std::vector<V, inblock_allocator<V, holder>>;
#endif

class html {
public:
	bool pair;
	String name;
	Map<String, String> params;
	String content;

	inline html& in(const html& x)
	{
		content += x.code();
		return *this;
	}

	inline html& text(const String& txt)
	{
		content += txt;
		return *this;
	}

	inline html& p(const String& n, const String& v)
	{
		params[n] = v;
		return *this;
	}

	inline html& cls(const String& n) { return p("class", n); }

	inline html& id(const String& n) { return p("id", n); }

	html(const String& name = "", bool pair = true)
	{
		this->name = name;
		this->pair = pair;
		params.clear();
		content.clear();
	}

	inline String& operator[] (const String& x) { return params[x]; }

	String code() const;
};

inline html ahref(const String& l, const String& v = "")
{
	return html("a", true).p("href", l).text(v);
}

inline html div()
{
	return html("div");
}

inline String htmlheader(const String& ctype = "text/html; charset=utf-8")
{
	return "Content-type: " + ctype + "\n\n";
}

String html::code() const
{
	String r;

	if (!name.length()) return content;

	r = "<" + name;
	for (Map<String, String>::const_iterator i = params.begin(),
		e = params.end();
		i != e;
		++i)
		r += " " + i->first + "=\"" + i->second + "\"";
	if (pair)
		r += ">" + content + "</" + name + ">";
	else
		r += " />";

	return r;
}

int main()
{
#ifndef USE_STD_ALLOCATOR
	std::vector<uint8_t> memory;
	memory.resize(100 * 1024 * 1024);

	holder::heap(memory.data(), 100 * 1024 * 1024);
#endif

	Vector<html> htmls;
	for (int i = 0; i < 16; ++i) {
		for (size_t j = 1; j < htmls.size(); ++j)
			htmls[j].in(htmls[j - 1]);
		htmls.insert(
			htmls.begin(),
			div().in(ahref("http://www.mff.cz/",
				"Velmi zahnizdeny link na matfyz.")));
	}

	auto x = div();
	for (auto& h : htmls) x.in(h);

	std::cout << x.code() << std::endl;

	return 0;
}
