#include <fstream>
#include "src/JSONParser/JSONTokenizer.h"
#include <iostream>

std::ostream& operator<<(std::ostream& s, JSONTokenType t)
{
	std::string str;

	switch (t) {
	case JSONTokenType::LEFT_BRACE:  str = "LEFT_BRACE";  break;
	case JSONTokenType::RIGHT_BRACE: str = "RIGHT_BRACE"; break;
	case JSONTokenType::COMMA:       str = "COMMA";       break;
	case JSONTokenType::COLON:       str = "COLON";       break;
	case JSONTokenType::LEFT_BRACK:  str = "LEFT_BRACK";  break;
	case JSONTokenType::RIGHT_BRACK: str = "RIGHT_BRACK"; break;
	}

	s << str;

	return s;
}

void print(const JSONToken& token)
{
	if (is_bool(token))   std::cout << "BOOL";
	if (is_null(token))   std::cout << "NULL";
	if (is_string(token)) std::cout << "STRING";
	if (is_number(token)) {
		std::cout << "NUMBER";
		std::cout << "(d form: " << std::stod(token.lexeme) << ')';
	}
}

int main()
{
	std::ifstream file("hello.txt");

	try {
		auto d = tokenize(file);

		for (const auto& t : d) {
			std::cout << t.type;
			print(t);
			if (t.type == JSONTokenType::VALUE) std::cout << ": " << t.lexeme;
			std::cout << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}