#pragma once

#include <memory>
#include <concepts>

#include "Concepts.h"

#define JSON_VALUE_CLASS_IMPL(VType, Type)	struct Type##_t : ValueBaseType						      \
											{													      \
												Type##_t(Type val) : val(std::move(val)) {}		      \
																									  \
												std::unique_ptr<ValueBaseType> clone() const override \
												{													  \
													return std::make_unique<Type##_t>(*this);		  \
												}													  \
																									  \
												ValueType get_type() const override					  \
												{													  \
													return get_static_type();						  \
												}													  \
																									  \
												constexpr static ValueType get_static_type()		  \
												{													  \
													return ValueType::VType;					      \
												}													  \
																									  \
												Type val;											  \
											}

class JSON
{
	enum class ValueType
	{
		STRING,
		NUMBER,
		BOOL,
		ARRAY,
		OBJECT
	};

	struct Value
	{
		virtual std::unique_ptr<Value> clone() const = 0;
		virtual ValueType get_type()           const = 0;
	};

	using ValueBaseType = Value;
	using ValueString = std::string;
	using ValueNumber = double;
	using ValueBool = bool;
	using ValueArray = std::vector<JSON>;
	using ValueObjectKey = std::string;
	using ValueObjectVal = JSON;
	using ValueObjectPair = std::pair<ValueObjectKey, ValueObjectVal>;
	using ValueObject = std::unordered_map<ValueObjectKey, ValueObjectVal>;

	JSON_VALUE_CLASS_IMPL(STRING, ValueString);
	JSON_VALUE_CLASS_IMPL(NUMBER, ValueNumber);
	JSON_VALUE_CLASS_IMPL(BOOL, ValueBool);
	JSON_VALUE_CLASS_IMPL(ARRAY, ValueArray);
	JSON_VALUE_CLASS_IMPL(OBJECT, ValueObject);

public:
	JSON() = default;
	JSON(const JSON& rhs) : m_root(rhs.m_root ? rhs.m_root->clone() : nullptr) {}
	JSON(JSON&& rhs) = default;

	template<typename T>
	requires has_to_json<T>
	JSON(const T& val)
	{
		to_json(*this, val);
	}

	JSON& operator=(const JSON& rhs)
	{
		if (&rhs == this) return *this;
		return *this = JSON(rhs);
	}

	JSON& operator=(JSON&& rhs) = default;

	/*template<typename T>
	requires has_to_json<T>
	JSON(const T& val)
	{
		to_json(*this, val);
	}

	JSON(std::initializer_list<ValueObjectPair> val)
	{
		to_json(*this, ValueObject{val.begin(), val.end()});
	}

	JSON(std::initializer_list<JSON> val)
	{
		to_json(*this, val);
	}*/

	void push_back(JSON val)
	{
		to_json_if_new(ValueArray{});
		get_ptr<ValueArray_t>()->val.push_back(std::move(val));
	}

	void insert_pair(ValueObjectPair pair)
	{
		to_json_if_new(ValueObject{});
		get_ptr<ValueObject_t>()->val.insert(std::move(pair));
	}

	JSON& operator[](const std::string& key) const
	{
		return get_ptr<ValueObject_t>()->val[key];
	}

	JSON& operator[](size_t idx) const
	{
		auto& vec = get_ptr<ValueArray_t>()->val;
		if (idx >= vec.size()) vec.resize(idx + 1);
		return vec.at(idx);
	}

	JSON& operator[](const std::string& key)
	{
		to_json_if_new(ValueObject{});
		return std::as_const(*this)[key];
	}

	JSON& operator[](size_t idx)
	{
		to_json_if_new(ValueArray{});
		return std::as_const(*this)[idx];
	}

	template<typename T, typename RetType = std::remove_cv_t<T>>
	requires has_from_json<RetType> && (!std::is_reference_v<RetType>)
	RetType as() const
	{
		auto ret = RetType();
		from_json(*this, ret);
		return ret;
	}

private:
	template<typename T>
	requires std::is_arithmetic_v<T>
	friend void from_json(const JSON& j, T& val)
	{
		if (j.is_type<ValueType::BOOL>()) {
			ValueBool b;
			from_json(j, b);
			val = b;
			return;
		}

		val = j.get_ptr<ValueNumber_t>()->val;
	}

	friend void from_json(const JSON& j, ValueBool& val)
	{
		val = j.get_ptr<ValueBool_t>()->val;
	}

	template<typename T>
	requires std::is_assignable_v<T, ValueString>
	friend void from_json(const JSON& j, T& val)
	{
		val = j.get_ptr<ValueString_t>()->val;
	}

	template<typename T>
	requires std::is_arithmetic_v<T>
	friend void to_json(JSON& j, const T& val)
	{
		j.assign<ValueNumber_t>(val);
	}

	friend void to_json(JSON& j, ValueBool val)
	{
		j.assign<ValueBool_t>(val);
	}

	template<typename T>
	requires std::is_convertible_v<T, ValueString>
	friend void to_json(JSON& j, T&& val)
	{
		j.assign<ValueString_t>(std::forward<T>(val));
	}

	friend void to_json(JSON& j, ValueObject val)
	{
		j.assign<ValueObject_t>(std::move(val));
	}

	friend void to_json(JSON& j, ValueArray val)
	{
		j.assign<ValueArray_t>(std::move(val));
	}

private:
	template<typename T, typename... Args>
	requires std::derived_from<T, ValueBaseType>
	void assign(Args&&... args)
	{
		m_root = std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	void to_json_if_new(T&& val)
	{
		if (!m_root) to_json(*this, std::forward<T>(val));
	}

	template<ValueType T>
	constexpr bool is_type() const
	{
		if (!m_root) throw std::runtime_error("The current object isn't holding any value!");

		return m_root->get_type() == T;
	}

	template<typename T>
	requires std::derived_from<T, ValueBaseType>
	T* get_ptr() const
	{
		if (!is_type<T::get_static_type()>()) throw std::runtime_error("The requested type and the type held by the object don't match!");

		return static_cast<T*>(m_root.get());
	}

private:
	std::unique_ptr<ValueBaseType> m_root;
};