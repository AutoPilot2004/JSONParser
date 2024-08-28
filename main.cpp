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
	if (token.type == JSONTokenType::KEYWORD_V) std::cout << "KEYWORD";
	if (token.type == JSONTokenType::STRING_V)  std::cout << "STRING";
	if (token.type == JSONTokenType::NUMBER_V) {
		std::cout << "NUMBER";
		std::cout << "(d form: " << std::stod(token.lexeme) << ')';
	}
}

#define v(c) "\c"

int main()
{
	std::ifstream file("hello.txt");

	std::cout << "\n\n\n\n\n\n";

	char c[] = { '\\', 'n', '\0'};

	std::string s;
	s += 'b';
	s += c;
	s += 'n';

	std::cout << (std::string)s;

	std::cout << "\n\n\n\n\n\n";

	
		auto d = tokenize(file);

		for (const auto& t : d) {
			std::cout << t.type;
			print(t);
			if (is_value(t)) std::cout << ": " << t.lexeme;
			std::cout << std::endl;
		}
	

	return 0;
}