#include "JSONParser.h"

#include <iostream>
#include <string>
#include <type_traits>

#include "JSONTokenizer.h"

#define EXPECT_TOKEN(token) if (tokens.get().type != token) throw std::runtime_error("Unexpected token")
#define PROGRESS_TOKEN() tokens.progress()
#define EXPECT_TOKEN_AND_PROGRESS(token) EXPECT_TOKEN(token); PROGRESS_TOKEN()

namespace
{
	template<typename T>
	class VectorWithPointer
	{
	public:
		VectorWithPointer(std::vector<T> vector) : m_vec(std::move(vector)) {}

		const T& get() const
		{
			return m_vec.at(m_idx);
		}

		void progress()
		{
			m_idx++;
		}

		operator bool() const
		{
			return m_idx < m_vec.size();
		}

		const std::vector<T>& get_vec() const
		{
			return m_vec;
		}

	private:
		std::vector<T> m_vec;
		size_t m_idx = 0;
	};

	JSON parse_value(VectorWithPointer<JSONToken>& tokens);

	JSON parse_array(VectorWithPointer<JSONToken>& tokens)
	{
		EXPECT_TOKEN_AND_PROGRESS(JSONTokenType::LEFT_BRACK);

		if (tokens.get().type == JSONTokenType::RIGHT_BRACK) return {};

		JSON arr;

		while (tokens) {
			arr.push_back(parse_value(tokens));

			if (tokens.get().type != JSONTokenType::COMMA) break;

			PROGRESS_TOKEN();
		}

		EXPECT_TOKEN(JSONTokenType::RIGHT_BRACK);

		return arr;
	}

	JSON parse_object(VectorWithPointer<JSONToken>& tokens)
	{
		EXPECT_TOKEN_AND_PROGRESS(JSONTokenType::LEFT_BRACE);

		if (tokens.get().type == JSONTokenType::RIGHT_BRACE) return {};

		JSON obj;

		while (tokens) {
			EXPECT_TOKEN(JSONTokenType::STRING_V);
			std::string name = tokens.get().lexeme;

			PROGRESS_TOKEN();

			EXPECT_TOKEN_AND_PROGRESS(JSONTokenType::COLON);

			//TODO: THROW IF MLTIPLE OBJECTS WITH SAME KEY
			//if (obj->pairs.find(name) != obj->pairs.end()) throw std::runtime_error("Multiple object pairs with the same key");

			obj.insert_pair({ name, parse_value(tokens) });

			if (tokens.get().type != JSONTokenType::COMMA) break;
			
			PROGRESS_TOKEN();
		}

		EXPECT_TOKEN(JSONTokenType::RIGHT_BRACE);

		return obj;
	}

	JSON parse_value(VectorWithPointer<JSONToken>& tokens)
	{
		const auto& token = tokens.get();

		JSON ret;

		switch (token.type) {
		case JSONTokenType::STRING_V:   ret = token.lexeme;            break;
		case JSONTokenType::NUMBER_V:   ret = std::stod(token.lexeme); break;
		case JSONTokenType::KEYWORD_V:  ret = token.lexeme == "true";  break;
			//TODO: IMPL NULL, KEYWORD IS ONLY FOR BOOL FOR NOW
		case JSONTokenType::LEFT_BRACE: ret = parse_object(tokens);    break;
		case JSONTokenType::LEFT_BRACK: ret = parse_array(tokens);     break;
		default: throw std::runtime_error("Invalid value type");
		}

		PROGRESS_TOKEN();

		return ret;
	}
}

JSON parse(std::ifstream& f)
{
	VectorWithPointer<JSONToken> tokens(tokenize(f));
	if (!tokens) return {};

	auto ret = parse_value(tokens);

	if (tokens) throw std::runtime_error("Multiple roots");

	return ret;
}