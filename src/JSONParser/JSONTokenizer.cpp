#include "JSONTokenizer.h"

#include <fstream>
#include <iostream>
#include <type_traits>
#include <cassert>

namespace
{

	bool read_char(std::ifstream& f, char& c)
	{
		c = f.get();

		return !f.eof();
	}

	void push_ctrl_char_to_string(char c, auto& str)
	{
		switch (c) {
		case '"':  str += '\"'; return;
		case '\\': str += '\\'; return;
		case '/':  str += '\/'; return;
		case 'b':  str += '\b'; return;
		case 'f':  str += '\f'; return;
		case 'n':  str += '\n'; return;
		case 'r':  str += '\r'; return;
		case 't':  str += '\t'; return;
			//TODO: case for unicode
		}

		throw std::runtime_error("Invalid control character");
	}

	JSONToken parse_string(std::ifstream& f, char c)
	{
		assert(c == '"');

		std::string str;

		bool not_eof;

		while (not_eof = read_char(f, c)) {
			//esc seq
			if (c == '\\') {
				if (!read_char(f, c)) throw std::runtime_error("Control character expected after '\\'");

				push_ctrl_char_to_string(c, str);
			}
			else {
				if (c == '"') break;
			}

			str += c;
		}

		if (!not_eof) throw std::runtime_error("Missing '\"'");

		return { JSONTokenType::STRING_V, str };
	}

	//0.324
	//532.32e5

	JSONToken parse_number(std::ifstream& f, char c)
	{
		assert(isdigit(c) || c == '-');

		std::string str;
		str += c;

		bool dec      = false;
		bool expo     = false;
		bool digitReq = false;

		if (c == '-' && read_char(f, c)) str += c;
		if (c == '0') {
			if (read_char(f, c)) {
				if (isdigit(c)) throw std::runtime_error("Invalid number");
				f.unget();
			}
		}

		while (read_char(f, c) && !isspace(c)) {
			str += c;

			if (isdigit(c)) {
				digitReq = expo;
			}
			else if (digitReq) break;
			else if (c == '.' && !dec) {
				digitReq = dec = true;
			}
			else if (tolower(c) == 'e') {
				digitReq = expo = true;

				if (!read_char(f, c) || (!isdigit(c) && c != '+' && c != '-')) break;
				str += c;
			}
			else {
				break;
			}
		}

		if (!isdigit(str.back())) throw std::runtime_error("Invalid number");

		return { JSONTokenType::NUMBER_V, str };
	}

	JSONToken parse_keyword(std::ifstream& f, char c)
	{
		std::string str;
		str += c;

		size_t i = 0;
		while (read_char(f, c) && !isspace(c) && ++i < 6) str += c;

		if (!(str == "false" || str == "true" || str == "null")) throw std::runtime_error("Invalid keyword");

		return { JSONTokenType::KEYWORD_V, str };
	}

	JSONToken parse_symbol(char c)
	{
		switch (c) {
		case '{': return { JSONTokenType::LEFT_BRACE };
		case '}': return { JSONTokenType::RIGHT_BRACE };
		case ',': return { JSONTokenType::COMMA };
		case ':': return { JSONTokenType::COLON };
		case '[': return { JSONTokenType::LEFT_BRACK };
		case ']': return { JSONTokenType::RIGHT_BRACK };
		}

		throw std::runtime_error("Invalid symbol");
	}
}

std::vector<JSONToken> tokenize(std::ifstream& f)
{
	std::vector<JSONToken> tokens;

	char c;

	while (read_char(f, c)) {
		if (isspace(c)) continue;

		JSONToken token;

		//string
		if (c == '"') {
			token = parse_string(f, c);
		}
		//number
		else if (isdigit(c) || c == '-') {
			token = parse_number(f, c);
		}
		//null/true/false
		else if (isalpha(c)) {
			token = parse_keyword(f, c);
		}
		//symbol
		else {
			token = parse_symbol(c);
		}

		tokens.push_back(token);
	}

	return tokens;
}

bool is_value(JSONToken token)
{
	return !token.lexeme.empty();
}