#include "JSONTokenizer.h"

#include <fstream>
#include <iostream>
#include <type_traits>
#include <cassert>

#define LINE "Line " + std::to_string(line) + ": "

#define RET_FALSE_OR_GET_FIRST_CHAR() if (token.type != JSONTokenType::VALUE) {                                         \
										assert(token.lexeme.empty());                                                   \
										return false;                                                                   \
									  }                                                                                 \
									  assert(!token.lexeme.empty());                                                    \
									  char c = token.lexeme.front()                                                     

namespace
{
	bool eof(const std::ifstream& f)
	{
		return f.eof();
	}

	bool read_char(std::ifstream& f, char& c)
	{
		c = f.get();

		return !eof(f);
	}

	bool read_and_digit(std::ifstream& f, char& c)
	{
		return read_char(f, c) && isdigit(c);
	}

	bool let_reread_char(std::ifstream& f)
	{
		f.unget();
		return !eof(f);
	}

	bool try_push_back_non_value(char c, auto& tokens)
	{
		switch (c) {
		case '{': tokens.emplace_back(JSONTokenType::LEFT_BRACE);  break;
		case '}': tokens.emplace_back(JSONTokenType::RIGHT_BRACE); break;
		case ',': tokens.emplace_back(JSONTokenType::COMMA);       break;
		case ':': tokens.emplace_back(JSONTokenType::COLON);       break;
		case '[': tokens.emplace_back(JSONTokenType::LEFT_BRACK);  break;
		case ']': tokens.emplace_back(JSONTokenType::RIGHT_BRACK); break;
		default:  return false;
		}

		return true;
	}

	bool try_push_back_number(std::ifstream& f, char c, const std::string& str, auto& tokens)
	{
		if (!(eof(f) || isspace(c))) return false;

		let_reread_char(f);
		tokens.emplace_back(JSONTokenType::VALUE, str);
		return true;
	}

	bool try_push_back_ctrl_chars(char c, auto& str)
	{
		switch (c) {
		case '"':  str += '\"'; break;
		case '\\': str += '\\'; break;
		case '/':  str += '\/'; break;
		case 'b':  str += '\b'; break;
		case 'f':  str += '\f'; break;
		case 'n':  str += '\n'; break;
		case 'r':  str += '\r'; break;
		case 't':  str += '\t'; break;
			//TODO: case for unicode
		default: return false;
		}

		return true;
	}
}

std::vector<JSONToken> tokenize(std::ifstream& f)
{
	std::vector<JSONToken> tokens;

	char c;

	size_t line = 1;

	while (read_char(f, c)) {
		if (c == '\n') line++;
		if (isspace(c) || try_push_back_non_value(c, tokens)) continue;

		std::string str;
		str += c;

		//string
		if (c == '"') {
			while (read_char(f, c)) {
				//esc seq
				if (c == '\\') {
					if (!read_char(f, c)) throw std::runtime_error(LINE + "Control character expected after '\\'");

					if (try_push_back_ctrl_chars(c, str)) continue;
					
					throw std::runtime_error(LINE + "Invalid control character after '\\'");
				}

				str += c;

				if (c == '"') break;
			}

			if (eof(f)) throw std::runtime_error(LINE + "Missing '\"'");

			tokens.emplace_back(JSONTokenType::VALUE, str);
		}
		//number
		else if (isdigit(c) || c == '-') {
			if (c == '-') {
				if (!read_and_digit(f, c)) throw std::runtime_error(LINE + "Invalid value");

				str += c;
			}

			if (c == '0') {
				if (read_and_digit(f, c)) throw std::runtime_error(LINE + "Numbers cannot start with 0");

				if (try_push_back_number(f, c, str, tokens)) continue;

				let_reread_char(f);

			}

			while (read_and_digit(f, c)) str += c;
			if (try_push_back_number(f, c, str, tokens)) continue;

			if      (c == '.') str += c;
			else if (tolower(c) == 'e') let_reread_char(f);
			else    throw std::runtime_error(LINE + "Invalid value");

			while (read_and_digit(f, c)) str += c;
			if (try_push_back_number(f, c, str, tokens)) continue;

			if (tolower(c) != 'e') throw std::runtime_error(LINE + "Invalid value");
			str += c;

			if (!read_char(f, c) || (c != '+' && c != '-' && !isdigit(c))) throw std::runtime_error(LINE + "sign or digit expected after 'e'");
			str += c;

			while (read_and_digit(f, c)) str += c;
			if (try_push_back_number(f, c, str, tokens)) continue;

			throw std::runtime_error(LINE + "Invalid value");
		}
		//null/true/false
		else {
			size_t i = 0;
			while (read_char(f, c) && !isspace(c) && ++i < 6) str += c;

			if (!(str == "false" || str == "true" || str == "null")) throw std::runtime_error(LINE + "Invalid value");

			tokens.emplace_back(JSONTokenType::VALUE, str);
		}
	}

	return tokens;
}

bool is_string(const JSONToken& token)
{
	RET_FALSE_OR_GET_FIRST_CHAR();

	return c == '"';
}

bool is_number(const JSONToken& token)
{
	RET_FALSE_OR_GET_FIRST_CHAR();

	return isdigit(c) || c == '-';
}

bool is_bool(const JSONToken& token)
{
	RET_FALSE_OR_GET_FIRST_CHAR();

	return c == 't' || c == 'f';
}

bool is_null(const JSONToken& token)
{
	RET_FALSE_OR_GET_FIRST_CHAR();

	return c == 'n';
}