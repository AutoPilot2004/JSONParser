#pragma once

#include <string>
#include <optional>
#include <vector>

enum class JSONTokenType
{
	//SYNTAX
	LEFT_BRACE,
	RIGHT_BRACE,
	COMMA,
	COLON,
	LEFT_BRACK,
	RIGHT_BRACK,

	//VALUES
	STRING_V,
	NUMBER_V,
	KEYWORD_V
};

struct JSONToken
{
	JSONTokenType type;
	std::string lexeme;
};

[[nodiscard]] std::vector<JSONToken> tokenize(std::ifstream& f);

bool is_value(JSONToken token);