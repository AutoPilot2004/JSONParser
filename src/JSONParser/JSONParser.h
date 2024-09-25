#pragma once

#include <vector>
#include <unordered_map>
#include <fstream>

namespace AST
{
	namespace Value
	{
		struct Node
		{};

		struct String : Node
		{
			std::string str;
		};

		struct Number : Node
		{
			double num;
		};

		struct Bool : Node
		{
			bool bln;
		};

		struct Object : Node
		{
			std::unordered_map<std::string, std::unique_ptr<Node>> pairs;
		};

		struct Array : Node
		{
			std::vector<std::unique_ptr<Node>> values;
		};
	}
}

[[nodiscard]] std::unique_ptr<AST::Value::Node> parse(std::ifstream& f);