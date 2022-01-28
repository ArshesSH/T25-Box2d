#pragma once

#include <unordered_map>
#include <functional>


template <typename T>
void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher( v ) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

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

	template<> struct hash<std::pair<Color, Color>>
	{
		size_t operator()( const std::pair<Color, Color> cs ) const
		{
			auto seed = std::hash<int>{}(cs.first.dword);
			hash_combine( seed, cs.second.dword );
		}
	};
}

template<typename T>
class TemplateSwitch
{
public:
	std::function<void()>& Case( T key )
	{
		return map[key];
	}
	std::function<void()>& Default()
	{
		return def;
	}
	void operator[]( const T& key ) const
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
	std::unordered_map<T, std::function<void()>> map;
	std::function<void()> def = []() {};
};

typedef TemplateSwitch<Color> ColorSwitch;