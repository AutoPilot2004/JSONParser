#pragma once

class JSON;

template<typename T>
concept has_from_json = requires(const JSON& j, T& val)
{
	from_json(j, val);
};

template<typename T>
concept has_to_json = requires(JSON& j, const T& val)
{
	to_json(j, val);
};