#pragma once

#include <unordered_map>
#include <functional>
#include "Colors.h"

namespace std
{
	template<> struct hash<Color>
	{
		size_t operator()( const Color& c ) const
		{
			std::hash<int> hasher;
			return hasher( c.dword );
		}
	};
}

class ColorSwitch
{
public:
	std::function<void()>& Case( Color key )
	{
		return map[key];
	}
	std::function<void()>& Default()
	{
		return def;
	}
	void operator[]( const Color& key ) const
	{
		auto i = map.find( key );
		if ( i != map.end() )
		{
			i->second();
		}
		else
		{
			def();
		}
	}
private:
	std::unordered_map<Color, std::function<void()>> map;
	std::function<void()> def = []() {};
};