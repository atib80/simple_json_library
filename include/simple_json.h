//
// Created by atib1980 on 2/2/2025.
//

#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#include <array>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <vector>
#include <string>
#include <cctype>
#include <stack>
#include <stdexcept>
#include <optional>
#include <iostream>
#include <ostream>
#include <sstream>
#include <type_traits>

namespace simple_json {

	template <typename T>
	concept char_type = std::is_same_v<T, char> || std::is_same_v<T, wchar_t>;

	template <typename CharType>
	struct delimiters_t {
		static constexpr const CharType* value = "{}[]";
	};

	template <>
	struct delimiters_t<wchar_t> {
		static constexpr const wchar_t* value = L"{}[]";
	};

	template <typename CharType>
	static constexpr const CharType* delimiters_v = delimiters_t<CharType>::value;

	template <char_type CharType>
	constexpr const size_t len(const CharType* src) noexcept {
		size_t i{};

		while (src[i])
			++i;

		return i;		
	}

	template <char_type CharType, size_t N>
	constexpr const size_t len(const CharType (&src)[N]) noexcept {
		size_t i{};

		while (src[i] && i < N)
			++i;

		return i;
	}

	enum class json_type {null_t, boolean_t, number_t, string_t, array_t, object_t};

	template<char_type CharType>
	class Json {
	public:
		using JSONValue = std::variant<std::nullptr_t, bool, double, std::basic_string<CharType>,
			std::vector<Json>,
			std::unordered_map<std::basic_string<CharType>, Json>>;

		Json() = default;
		Json(std::nullptr_t) : value{ nullptr } {}
		Json(const bool b) : value{ b } {}
		Json(const double d) : value{ d } {}
		Json(const std::basic_string<CharType>& s) : value{ s } {}
		Json(const CharType* s) : value{ std::basic_string<CharType>(s) } {}
		Json(const std::vector<Json>& v) : value{ v } {}
		Json(const std::unordered_map<std::basic_string<CharType>, Json>& o) : value{ o } {}

		static Json parse(const std::basic_string<CharType>& input) {
			size_t pos{};
			return parseValue(input, pos);
		}

		std::basic_string<CharType> to_string(int indent = 0) const {
			std::basic_ostringstream<CharType> oss;
			print_value(value, oss, indent, 0);
			return oss.str();
		}

		friend std::basic_ostream<CharType>& operator<<(std::basic_ostream<CharType>& os, const Json& json) {
			os << json.to_string(2); // default indentation: 2 spaces
			return os;
		}

		friend std::basic_istream<CharType>& operator>>(std::basic_istream<CharType>& is, Json<CharType>& json) {

			std::stack<CharType> open_curly_braces;
			std::stack<CharType> open_brackets;
			bool is_found_open_curly_brace{};

			std::basic_string<CharType> buffer;
			for (std::basic_string<CharType> line; std::getline(is, line); ) {
				size_t pos{};
				skip_whitespace(line, pos);
				if (pos == line.size()) continue;
				if (static_cast<CharType>('{') == line[pos]) {
					open_curly_braces.push(static_cast<CharType>('{'));
					is_found_open_curly_brace = true;
					buffer = std::move(line);
					break;
				}
			}

			if (!is_found_open_curly_brace) throw std::invalid_argument("Invalid JSON syntax: missing first open curly brace!");

			for (std::basic_string<CharType> line; std::getline(is, line); ) {
				size_t pos{};
				skip_whitespace(line, pos);
				if (pos == line.size()) continue;
				buffer += line;

				for (; (pos = line.find_first_of(delimiters_v<CharType>, pos)) != std::string::npos; ++pos) {
					if (static_cast<CharType>('{') == line[pos]) {
						open_curly_braces.push(static_cast<CharType>('{'));
					}
					else if (static_cast<CharType>('[') == line[pos]) {
						open_brackets.push(static_cast<CharType>('['));
					}
					else if (static_cast<CharType>('}') == line[pos]) {
						if (open_curly_braces.empty())
							throw std::invalid_argument("Invalid JSON syntax: too many close curly braces!");
						open_curly_braces.pop();
						if (open_curly_braces.empty()) break;
					}
					else if (static_cast<CharType>(']') == line[pos]) {
						if (open_brackets.empty())
							throw std::invalid_argument("Invalid JSON syntax: too many close brackets!");
						open_brackets.pop();
					}
				}
			}

			if (!open_curly_braces.empty() || !open_brackets.empty()) throw std::invalid_argument("Invalid JSON syntax too many open curly braces or brackets!");

			json = parse(buffer);

			return is;
		}

		std::optional<std::reference_wrapper<JSONValue>> get_json_element(std::basic_string<CharType> key) const {
			if (!is_json_object(value)) return std::nullopt;
			try {
				const auto& element = value.get<std::to_underlying(json_type::object_t)>();
				if (element.contains(key)) return std::make_optional<std::reference_wrapper<JSONValue>>(std::cref(element.at(key)));
			}
			catch (const std::exception& ex) {
				std::cerr << "Exception: " << ex.what() << '\n';
			}

			return std::nullopt;
		}

		const json_type& get_json_element_type(const JSONValue& json_element) const noexcept {

			if (std::get_if<std::unordered_map<std::basic_string<CharType>, Json>>(&json_element))
				return json_type::object_t;

			if (std::get_if<std::vector<Json>>(&json_element))
				return json_type::array_t;

			if (std::get_if<std::basic_string<CharType>>(&json_element))
				return json_type::string_t;

			if (std::get_if<std::get_if<double>>(&json_element))
				return json_type::number_t;

			if (std::get_if<std::get_if<bool>>(&json_element))
				return json_type::boolean_t;

			return json_type::null_t;
		}

		// boolean predicate member functions for testing if specified json element is of certain JSON enum type (json_type)
		bool is_json_object(const JSONValue& json_element) const noexcept {
			const auto* value_ptr = std::get_if<std::unordered_map<std::basic_string<CharType>, Json>>(&json_element);
			return value_ptr != nullptr;
		}

		bool is_json_array(const JSONValue& json_element) const noexcept {
			const auto* value_ptr = std::get_if<std::vector<Json>>(&json_element);
			return value_ptr != nullptr;
		}

		bool is_json_string(const JSONValue& json_element) const noexcept {
			const auto* value_ptr = std::get_if<std::basic_string<CharType>>(&json_element);
			return value_ptr != nullptr;
		}

		bool is_json_number(const JSONValue& json_element) const noexcept {
			const auto* value_ptr = std::get_if<double>(&json_element);
			return value_ptr != nullptr;
		}

		bool is_json_boolean(const JSONValue& json_element) const noexcept {
			const auto* value_ptr = std::get_if<bool>(&json_element);
			return value_ptr != nullptr;
		}

		bool is_json_null(const JSONValue& json_element) const noexcept {
			const auto* value_ptr = std::get_if<std::nullptr_t>(&json_element);
			return value_ptr != nullptr;
		}

	private:
		JSONValue value;

		static Json parseValue(const std::basic_string<CharType>& str, size_t& pos) {
			
			skip_whitespace(str, pos);

			if (pos >= str.size()) throw std::runtime_error("Unexpected end of json data!");

			if (str[pos] == static_cast<CharType>('{')) return parse_json_object(str, pos);
			if (str[pos] == static_cast<CharType>('[')) return parse_json_array(str, pos);
			if (str[pos] == static_cast<CharType>('"')) return parse_json_string(str, pos);
			if (std::isdigit(str[pos]) || str[pos] == DASH_CHAR) return parse_json_number(str, pos);
			
			if (str.compare(pos, TRUE_STRING_LEN, TRUE_STRING) == 0) { 
				pos += TRUE_STRING_LEN;
				return Json(true); 
			}
			if (str.compare(pos, FALSE_STRING_LEN, FALSE_STRING) == 0) { 
				pos += FALSE_STRING_LEN; 
				return Json(false); 
			}
			
			if (str.compare(pos, NULL_STRING_LEN, NULL_STRING) == 0) { 
				pos += NULL_STRING_LEN; 
				return Json(nullptr); 
			}

			throw std::runtime_error("Invalid json value!");
		}

		static Json parse_json_object(const std::basic_string<CharType>& str, size_t& pos) {

			std::unordered_map<std::basic_string<CharType>, Json> json_object;
			++pos;
			skip_whitespace(str, pos);
			while (pos < str.size() && str[pos] != static_cast<CharType>('}')) {
				std::basic_string<CharType> key{ parse_json_string(str, pos) };
				skip_whitespace(str, pos);
				if (str[pos] != static_cast<CharType>(':')) throw std::runtime_error("Expected ':' in JSON object!");
				++pos;
				Json temp_value = parseValue(str, pos);
				json_object[std::move(key)] = std::move(temp_value);
				skip_whitespace(str, pos);
				if (str[pos] == static_cast<CharType>(',')) ++pos;
				skip_whitespace(str, pos);
			}
			if (pos >= str.size() || str[pos] != static_cast<CharType>('}')) throw std::runtime_error("Expected '}' in JSON object!");
			++pos;
			return json_object;
		}

		static Json parse_json_array(const std::basic_string<CharType>& str, size_t& pos) {
			std::vector<Json> json_array;
			++pos;
			skip_whitespace(str, pos);
			while (pos < str.size() && str[pos] != static_cast<CharType>(']')) {
				json_array.push_back(parseValue(str, pos));
				skip_whitespace(str, pos);
				if (str[pos] == static_cast<CharType>(',')) ++pos;
				skip_whitespace(str, pos);
			}
			if (pos >= str.size() || str[pos] != static_cast<CharType>(']')) throw std::runtime_error("Expected ']' in JSON array!");
			++pos;
			return json_array;
		}

		static std::basic_string<CharType> parse_json_string(const std::basic_string<CharType>& str, size_t& pos) {
			if (str[pos] != static_cast<CharType>('"')) throw std::runtime_error("Expected '\"' for json string data!");
			++pos;
			std::basic_string<CharType> result;
			while (pos < str.size() && str[pos] != static_cast<CharType>('"')) {
				result.push_back(str[pos]);
				++pos;
			}
			if (pos >= str.size() || str[pos] != static_cast<CharType>('"')) throw std::runtime_error("Unterminated json string data!");
			++pos;
			return result;
		}

		static Json parse_json_number(const std::basic_string<CharType>& str, size_t& pos) {
			size_t start{ pos };
			if (str[pos] == static_cast<CharType>('-')) ++pos;
			while (pos < str.size() && (std::isdigit(str[pos]) || str[pos] == static_cast<CharType>('.'))) ++pos;
			return std::stod(str.substr(start, pos - start));
		}

		static void skip_whitespace(const std::basic_string<CharType>& str, size_t& pos) {
			while (pos < str.size() && is_whitespace(str[pos])) ++pos;
		}

		/*template <typename T>
		T get() const {
			return std::get<T>(value);
		}*/

		static bool is_whitespace(CharType ch) {
			static constexpr const std::array<bool, 1 << (sizeof(CharType) * 8u)> whitespaces = []() consteval {
				std::array<bool, 1 << (sizeof(CharType) * 8u)> chars{};
				chars[static_cast<unsigned>(' ')] = true;
				chars[static_cast<unsigned>('\t')] = true;
				chars[static_cast<unsigned>('\n')] = true;
				chars[static_cast<unsigned>('\r')] = true;
				chars[static_cast<unsigned>('\f')] = true;
				chars[static_cast<unsigned>('\v')] = true;
				return chars;
				}();
			return whitespaces[static_cast<unsigned>(ch)];
		}

		static void print_value(const JSONValue& val, std::basic_ostream<CharType>& os, int indent, int level) {
			std::visit([&](const auto& variant_value) { print_helper(variant_value, os, indent, level); }, val);
		}

		static void print_helper(std::nullptr_t, std::basic_ostream<CharType>& os, int, int) { os << "null"; }
		static void print_helper(bool b, std::basic_ostream<CharType>& os, int, int) { os << (b ? "true" : "false"); }
		static void print_helper(double d, std::basic_ostream<CharType>& os, int, int) { os << d; }
		static void print_helper(const std::basic_string<CharType>& s, std::basic_ostream<CharType>& os, int, int) { os << static_cast<CharType>('"') << s << static_cast<CharType>('"'); }

		static void print_helper(const std::vector<Json>& json_array, std::basic_ostream<CharType>& os, int indent, int level) {
			os << static_cast<CharType>('[') << static_cast<CharType>('\n');
			for (const auto& el : json_array) {
				os << std::basic_string<CharType>((level + 1) * indent, static_cast<CharType>(' '));
				print_value(el.value, os, indent, level + 1);
				os << static_cast<CharType>(',') << static_cast<CharType>('\n');
			}
			os << std::basic_string<CharType>(level * indent, static_cast<CharType>(' ')) << static_cast<CharType>(']');
		}

		static void print_helper(const std::unordered_map<std::basic_string<CharType>, Json>& json_object, std::basic_ostream<CharType>& os, int indent, int level) {
			os << static_cast<CharType>('{') << static_cast<CharType>('\n');
			for (const auto& [json_key, json_value] : json_object) {
				os << std::basic_string<CharType>((level + 1) * indent, static_cast<CharType>(' ')) << static_cast<CharType>('"') << json_key << static_cast<CharType>('"') << static_cast<CharType>(':') << static_cast<CharType>(' ');
				print_value(json_value.value, os, indent, level + 1);
				os << static_cast<CharType>(',') << static_cast<CharType>('\n');
			}
			os << std::basic_string<CharType>(level * indent, static_cast<CharType>(' ')) << static_cast<CharType>('}');
		}

		static constexpr const CharType* get_true_string() {
			if constexpr (std::is_same_v<CharType, char>) {
				return "true";
			}
			else if constexpr (std::is_same_v<CharType, wchar_t>) {
				return L"true";
			}
			else return "true";
		}

		static constexpr const CharType* get_false_string() noexcept {
			if constexpr (std::is_same_v<CharType, char>) {
				return "false";
			}
			else if constexpr (std::is_same_v<CharType, wchar_t>) {
				return L"false";
			}
			else return "false";
		}

		static constexpr const CharType* get_null_string() noexcept {
			if constexpr (std::is_same_v<CharType, char>) {
				return "null";
			}
			else if constexpr (std::is_same_v<CharType, wchar_t>) {
				return L"null";
			}
			else return "null";
		}

		inline static constexpr CharType DASH_CHAR{ static_cast<CharType>('-') };
		inline static constexpr const CharType* TRUE_STRING{ get_true_string() };
		inline static constexpr const CharType* FALSE_STRING{ get_false_string() };
		inline static constexpr const CharType* NULL_STRING{ get_null_string() };
		inline static constexpr const size_t TRUE_STRING_LEN{ len(TRUE_STRING) };
		inline static constexpr const size_t FALSE_STRING_LEN{ len(FALSE_STRING) };
		inline static constexpr const size_t NULL_STRING_LEN{ len(NULL_STRING) };
	};

	using json = Json<char>;
	using wjson = Json<wchar_t>;

	json operator ""_json(const char* json_string, const size_t) {
		return json::parse(json_string);
	}

	wjson operator ""_json(const wchar_t* json_string, const size_t) {
		return wjson::parse(json_string);
	}


}

#endif //SIMPLE_JSON_H
