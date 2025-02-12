//
// Created by atib1980 on 2/11/2025.
//

#include "../include/simple_json.h"
#include <stack>

namespace simple_json
{

inline static constexpr char DASH_CHAR{ '-' };
inline static constexpr const char *TRUE_STRING{ "true" };
inline static constexpr const char *FALSE_STRING{ "false" };
inline static constexpr const char *NULL_STRING{ "null" };
inline static constexpr size_t TRUE_STRING_LEN{ len (TRUE_STRING) };
inline static constexpr size_t FALSE_STRING_LEN{ len (FALSE_STRING) };
inline static constexpr size_t NULL_STRING_LEN{ len (NULL_STRING) };

std::ostream &
operator<< (std::ostream &os, const Json &json)
{
  os << json.to_string (2); // default indentation: 2 spaces
  return os;
}

std::istream &
operator>> (std::istream &is, Json &json)
{
  static constexpr const char *delimiters = "{}[]";

  std::stack<char> open_curly_braces;
  std::stack<char> open_brackets;
  bool is_found_open_curly_brace{};

  std::string buffer;
  for (std::string line; std::getline (is, line);)
    {
      size_t pos{};
      skip_whitespace (line, pos);
      if (pos == line.size ())
        continue;
      if ('{' == line[pos])
        {
          open_curly_braces.push ('{');
          is_found_open_curly_brace = true;
          buffer = std::move (line);
          break;
        }
    }

  if (!is_found_open_curly_brace)
    throw std::invalid_argument (
        "Invalid JSON syntax: missing first open curly brace!");

  for (std::string line; std::getline (is, line);)
    {
      size_t pos{};
      skip_whitespace (line, pos);
      if (pos == line.size ())
        continue;
      buffer += line;

      for (; (pos = line.find_first_of (delimiters, pos)) != std::string::npos;
           ++pos)
        {
          if ('{' == line[pos])
            {
              open_curly_braces.push ('{');
            }
          else if ('[' == line[pos])
            {
              open_brackets.push ('[');
            }
          else if ('}' == line[pos])
            {
              if (open_curly_braces.empty ())
                throw std::invalid_argument (
                    "Invalid JSON syntax: too many close curly braces!");
              open_curly_braces.pop ();
              if (open_curly_braces.empty ())
                break;
            }
          else if (']' == line[pos])
            {
              if (open_brackets.empty ())
                throw std::invalid_argument (
                    "Invalid JSON syntax: too many close brackets!");
              open_brackets.pop ();
            }
        }
    }

  if (!open_curly_braces.empty () || !open_brackets.empty ())
    throw std::invalid_argument (
        "Invalid JSON syntax: too many open curly braces or brackets!");

  auto [result_json, result_status, result_message] = parse (buffer);
  if (result_status == status::fail || !result_json.has_value ())
    throw std::invalid_argument{ result_message };
  json = std::move (result_json.value ());
  return is;
}

result_type
parse (const std::string &input)
{
  size_t pos{};
  return parseValue (input, pos);
}

result_type
parseValue (const std::string &str, size_t &pos)
{
  skip_whitespace (str, pos);

  if (pos >= str.size ())
    {
      return result_type{ {}, status::fail, "Unexpected end of json data!" };
    }

  if (str[pos] == '{')
    return parse_json_object (str, pos);
  if (str[pos] == '[')
    return parse_json_array (str, pos);
  if (str[pos] == '"')
    return parse_json_string (str, pos);
  if (std::isdigit (str[pos]) || str[pos] == DASH_CHAR)
    return parse_json_number (str, pos);

  if (str.compare (pos, TRUE_STRING_LEN, TRUE_STRING) == 0)
    {
      pos += TRUE_STRING_LEN;
      return result_type{ std::make_optional<Json> (Json{ true }),
                          status::success };
    }
  if (str.compare (pos, FALSE_STRING_LEN, FALSE_STRING) == 0)
    {
      pos += FALSE_STRING_LEN;
      return result_type{ std::make_optional<Json> (Json{ false }),
                          status::success };
    }

  if (str.compare (pos, NULL_STRING_LEN, NULL_STRING) == 0)
    {
      pos += NULL_STRING_LEN;
      return result_type{ std::make_optional<Json> (Json{ nullptr }),
                          status::success };
    }

  return result_type{ std::nullopt, status::fail, "Invalid json value!" };
}

result_type
parse_json_object (const std::string &str, size_t &pos)
{
  std::unordered_map<std::string, Json> json_object;
  ++pos;
  skip_whitespace (str, pos);
  while (pos < str.size () && str[pos] != '}')
    {
      if (auto [json_value, success1, error_msg1]
          = parse_json_string (str, pos);
          status::success == success1 && json_value.has_value ()
          && json_value->is_json_string ())
        {
          std::string key{ std::get<std::string> (
              json_value.value ().get_json_value_as_variant ()) };

          skip_whitespace (str, pos);
          if (str[pos] != ':')
            return result_type{ std::nullopt, status::fail,
                                "Expected ':' in JSON object!" };
          ++pos;
          auto [temp_value, success2, error_msg2] = parseValue (str, pos);
          if (success2 != status::success || !temp_value.has_value ())
            {
              throw std::invalid_argument{ error_msg2 };
            }
          json_object[std::move (key)] = temp_value.value ();

          skip_whitespace (str, pos);
          if (str[pos] == ',')
            ++pos;
          skip_whitespace (str, pos);
        }
    }
  if (pos >= str.size () || str[pos] != '}')
    return result_type{ std::nullopt, status::fail,
                        "Expected '}' in JSON object!" };
  ++pos;
  return result_type{ std::make_optional<Json> (std::move (json_object)),
                      status::success };
}

result_type
parse_json_array (const std::string &str, size_t &pos)
{
  std::vector<Json> json_array;
  ++pos;
  skip_whitespace (str, pos);
  while (pos < str.size () && str[pos] != ']')
    {
      auto [json_value, success, error_msg] = parseValue (str, pos);
      if (success == status::fail)
        throw std::invalid_argument{ error_msg };
      json_array.push_back (std::move (json_value.value_or (Json (nullptr))));
      skip_whitespace (str, pos);
      if (str[pos] == ',')
        ++pos;
      skip_whitespace (str, pos);
    }
  if (pos >= str.size () || str[pos] != ']')
    return result_type{ std::nullopt, status::fail,
                        "Expected ']' in JSON array!" };
  ++pos;
  return result_type{ std::make_optional<Json> (json_array), status::success };
}

result_type
parse_json_string (const std::string &str, size_t &pos)
{
  if (str[pos] != '"')
    return result_type{ std::nullopt, status::fail,
                        "Expected '\"' for json string data!" };
  ++pos;
  std::string result;
  while (pos < str.size () && str[pos] != '"')
    {
      result.push_back (str[pos]);
      ++pos;
    }
  if (pos >= str.size () || str[pos] != '"')
    return result_type{ std::nullopt, status::fail,
                        "Unterminated json string data!" };
  ++pos;
  return result_type{ std::make_optional<Json> (Json{ result }),
                      status::success };
}

result_type
parse_json_number (const std::string &str, size_t &pos)
{
  size_t start{ pos };
  if (str[pos] == '-')
    ++pos;
  while (pos < str.size () && (std::isdigit (str[pos]) || str[pos] == '.'))
    ++pos;
  return result_type{ std::make_optional<Json> (
                          Json{ std::stod (str.substr (start, pos - start)) }),
                      status::success };
}

void
skip_whitespace (const std::string &str, size_t &pos)
{
  while (pos < str.size () && is_whitespace (str[pos]))
    ++pos;
}

bool
is_whitespace (char ch)
{
  static constexpr const std::array<bool, 1 << (sizeof (char) * 8u)>
      whitespaces = [] () consteval {
        std::array<bool, 1 << (sizeof (char) * 8u)> chars{};
        chars[static_cast<unsigned> (' ')] = true;
        chars[static_cast<unsigned> ('\t')] = true;
        chars[static_cast<unsigned> ('\n')] = true;
        chars[static_cast<unsigned> ('\r')] = true;
        chars[static_cast<unsigned> ('\f')] = true;
        chars[static_cast<unsigned> ('\v')] = true;
        return chars;
      }();
  return whitespaces[static_cast<unsigned char> (ch)];
}

void
print_value (const JSONValue &val, std::ostream &os, int indent, int level)
{
  std::visit (
      [&] (const auto &variant_value) {
        print_helper (variant_value, os, indent, level);
      },
      val);
}

void
print_helper (std::nullptr_t, std::ostream &os, int, int)
{
  os << "null";
}
void
print_helper (bool b, std::ostream &os, int, int)
{
  os << (b ? "true" : "false");
}
void
print_helper (double d, std::ostream &os, int, int)
{
  os << d;
}
void
print_helper (const std::string &s, std::ostream &os, int, int)
{
  os << s;
}

void
print_helper (const std::vector<Json> &json_array, std::ostream &os,
              int indent, int level)
{
  os << '[' << '\n';
  for (const auto &el : json_array)
    {
      os << std::string ((level + 1) * indent, ' ');
      print_value (el.get_json_value_as_variant (), os, indent, level + 1);
      os << ',' << '\n';
    }
  os << std::string (level * indent, ' ') << ']';
}

void
print_helper (const std::unordered_map<std::string, Json> &json_object,
              std::ostream &os, int indent, int level)
{
  os << '{' << '\n';
  for (const auto &[json_key, json_value] : json_object)
    {
      os << std::string ((level + 1) * indent, ' ') << '"' << json_key << '"'
         << ": ";
      print_value (json_value.get_json_value_as_variant (), os, indent,
                   level + 1);
      os << ",\n";
    }
  os << std::string (level * indent, ' ') << '}';
}
}
