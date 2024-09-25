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

	bool root_node_type_is_valid(const std::vector<JSONToken>& tokens)
	{
		return (tokens.front().type == JSONTokenType::LEFT_BRACK && tokens.back().type == JSONTokenType::RIGHT_BRACK) ||
			   (tokens.front().type == JSONTokenType::LEFT_BRACE && tokens.back().type == JSONTokenType::RIGHT_BRACE);
	}

	std::unique_ptr<AST::Value::Node> parse_value(VectorWithPointer<JSONToken>& tokens);

	std::unique_ptr<AST::Value::Array> parse_array(VectorWithPointer<JSONToken>& tokens)
	{
		EXPECT_TOKEN_AND_PROGRESS(JSONTokenType::LEFT_BRACK);

		if (tokens.get().type == JSONTokenType::RIGHT_BRACK) return {};

		auto arr = std::make_unique<AST::Value::Array>();

		while (tokens) {
			arr->values.emplace_back(parse_value(tokens));

			if (tokens.get().type != JSONTokenType::COMMA) break;

			PROGRESS_TOKEN();
		}

		EXPECT_TOKEN(JSONTokenType::RIGHT_BRACK);

		return arr;
	}

	std::unique_ptr<AST::Value::Object> parse_object(VectorWithPointer<JSONToken>& tokens)
	{
		EXPECT_TOKEN_AND_PROGRESS(JSONTokenType::LEFT_BRACE);

		if (tokens.get().type == JSONTokenType::RIGHT_BRACE) return {};

		auto obj = std::make_unique<AST::Value::Object>();

		while (tokens) {
			EXPECT_TOKEN(JSONTokenType::STRING_V);
			std::string name = tokens.get().lexeme;

			PROGRESS_TOKEN();

			EXPECT_TOKEN_AND_PROGRESS(JSONTokenType::COLON);

			if (obj->pairs.find(name) != obj->pairs.end()) throw std::runtime_error("Multiple object pairs with the same key");

			obj->pairs.emplace(name, parse_value(tokens));

			if (tokens.get().type != JSONTokenType::COMMA) break;
			
			PROGRESS_TOKEN();
		}

		EXPECT_TOKEN(JSONTokenType::RIGHT_BRACE);

		return obj;
	}

	template<typename T, typename... Args>
	requires(std::is_base_of_v<AST::Value::Node, T>)
	std::unique_ptr<T> parse_term_value(Args&&... args)
	{
		return std::make_unique<T>(T({}, std::forward<Args>(args)...));
	}

	std::unique_ptr<AST::Value::Node> parse_value(VectorWithPointer<JSONToken>& tokens)
	{
		const auto& token = tokens.get();

		std::unique_ptr<AST::Value::Node> ret;

		switch (token.type) {
		case JSONTokenType::STRING_V:   ret = parse_term_value<AST::Value::String>(token.lexeme);            break;
		case JSONTokenType::NUMBER_V:   ret = parse_term_value<AST::Value::Number>(std::stod(token.lexeme)); break;
		case JSONTokenType::KEYWORD_V:  ret = parse_term_value<AST::Value::Bool>(token.lexeme == "true");    break;
			//TODO: IMPL NULL, KEYWORD IS ONLY FOR BOOL FOR NOW
		case JSONTokenType::LEFT_BRACE: ret = parse_object(tokens); break;
		case JSONTokenType::LEFT_BRACK: ret = parse_array(tokens);  break;
		default: throw std::runtime_error("Invalid value type");    break;
		}

		PROGRESS_TOKEN();

		return ret;
	}
}

std::unique_ptr<AST::Value::Node> parse(std::ifstream& f)
{
	VectorWithPointer<JSONToken> tokens(tokenize(f));
	if (!tokens) return {};
	if (!root_node_type_is_valid(tokens.get_vec())) throw std::runtime_error("Invalid root node type");

	std::unique_ptr<AST::Value::Node> ret = parse_value(tokens);

	if (tokens) throw std::runtime_error("Multiple roots");

	return ret;
}