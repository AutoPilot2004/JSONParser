#pragma once

#include <string>
#include <optional>
#include <vector>

enum class JSONTokenType
{
	LEFT_BRACE,
	RIGHT_BRACE,
	COMMA,
	COLON,
	LEFT_BRACK,
	RIGHT_BRACK,
	VALUE
};

struct JSONToken
{
	JSONTokenType type;
	std::string lexeme;
};

std::vector<JSONToken> tokenize(std::ifstream& f);

bool is_string(const JSONToken& token);
bool is_number(const JSONToken& token);
bool is_bool(const JSONToken& token);
bool is_null(const JSONToken& token);