//
// Created by atib1980 on 2/2/2025.
//

#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#include <array>
#include <cctype>
#include <format>
#include <optional>
#include <ostream>
#include <sstream>

#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace simple_json
{

template <typename T>
concept char_type
    = std::is_same_v<T, char> || std::is_same_v<T, wchar_t>
      || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>
      || std::is_same_v<T, char8_t>;

template <char_type CharType>
constexpr size_t
len (const CharType *src) noexcept
{
  size_t i{};

  while (src[i])
    ++i;

  return i;
}

template <char_type CharType, size_t N>
constexpr size_t
len (const CharType (&src)[N]) noexcept
{
  size_t i{};

  while (src[i] && i < N)
    ++i;

  return i;
}

bool is_whitespace (char ch);
void skip_whitespace (const std::string &str, size_t &pos);

class Json;

using JSONValue
    = std::variant<std::nullptr_t, bool, double, std::string,
                   std::vector<Json>, std::unordered_map<std::string, Json> >;

struct result_type;

result_type parse (const std::string &input);
std::ostream &operator<< (std::ostream &os, const Json &json);
std::istream &operator>> (std::istream &is, Json &json);

result_type parseValue (const std::string &str, size_t &pos);
result_type parse_json_object (const std::string &str, size_t &pos);
result_type parse_json_array (const std::string &str, size_t &pos);
result_type parse_json_string (const std::string &str, size_t &pos);
result_type parse_json_number (const std::string &str, size_t &pos);
void print_value (const JSONValue &val, std::ostream &os, int indent,
                  int level);
void print_helper (std::nullptr_t, std::ostream &os, int, int);
void print_helper (bool b, std::ostream &os, int, int);
void print_helper (double d, std::ostream &os, int, int);
void print_helper (const std::string &s, std::ostream &os, int, int);
void print_helper (const std::vector<Json> &json_array, std::ostream &os,
                   int indent, int level);
void print_helper (const std::unordered_map<std::string, Json> &json_object,
                   std::ostream &os, int indent, int level);

enum class status
{
  success,
  fail
};

enum class json_type : unsigned
{
  null_t,
  boolean_t,
  number_t,
  string_t,
  array_t,
  object_t
};

class Json
{
public:
  constexpr explicit Json () : value{ nullptr } {}
  constexpr explicit Json (std::nullptr_t) : value{ nullptr } {}
  constexpr explicit Json (const bool b) : value{ b } {}
  constexpr explicit Json (const int n) : Json(static_cast<double>(n)) {}
  constexpr explicit Json (const double d) : value{ d } {}
  constexpr explicit Json (const char* s) : value{ std::string{s} } {}
  constexpr explicit Json (const std::string &s) : value{ s } {}
  constexpr explicit Json (const std::vector<Json> &values) : value{ values } {}
  constexpr explicit Json (std::vector<Json> &&values) : value{ std::move (values) } {}
  // constexpr Json (const std::initializer_list<Json> values)
  //     : value{ std::vector<Json> (values) } {}
  explicit Json (const std::unordered_map<std::string, Json> &o) : value{ o } {}
  explicit Json (std::unordered_map<std::string, Json> &&o) : value{ std::move (o) } {}

  std::optional<std::nullptr_t>
  get_json_value_as_null () const
  {
    if (is_json_null ())
      return std::make_optional (std::get<std::nullptr_t> (value));
    return std::nullopt;
  }

  std::optional<bool>
  get_json_value_as_bool () const
  {
    if (is_json_boolean ())
      return std::make_optional (std::get<bool> (value));
    return std::nullopt;
  }

  std::optional<double>
  get_json_value_as_number () const
  {
    if (is_json_number ())
      return std::make_optional (std::get<double> (value));
    return std::nullopt;
  }

  std::optional<std::reference_wrapper<std::string> >
  get_json_value_as_string ()
  {
    if (is_json_string ())
      return std::make_optional<std::reference_wrapper<std::string> > (
          std::ref (std::get<std::string> (value)));
    return std::nullopt;
  }

  std::optional<std::reference_wrapper<const std::string> >
  get_json_value_as_string () const
  {
    if (is_json_string ())
      return std::make_optional<std::reference_wrapper<const std::string> > (
          std::cref (std::get<std::string> (value)));
    return std::nullopt;
  }

  std::optional<std::reference_wrapper<std::vector<Json> > >
  get_json_value_as_array ()
  {
    if (is_json_array ())
      return std::make_optional<std::reference_wrapper<std::vector<Json> > > (
          std::ref (std::get<std::vector<Json> > (value)));
    return std::nullopt;
  }

  std::optional<std::reference_wrapper<const std::vector<Json> > >
  get_json_value_as_array () const
  {
    if (is_json_array ())
      return std::make_optional<
          std::reference_wrapper<const std::vector<Json> > > (
          std::cref (std::get<std::vector<Json> > (value)));
    return std::nullopt;
  }

  std::optional<
      std::reference_wrapper<std::unordered_map<std::string, Json> > >
  get_json_value_as_object ()
  {
    if (is_json_object ())
      return std::make_optional<
          std::reference_wrapper<std::unordered_map<std::string, Json> > > (
          std::ref (std::get<std::unordered_map<std::string, Json> > (value)));
    return std::nullopt;
  }

  std::optional<
      std::reference_wrapper<const std::unordered_map<std::string, Json> > >
  get_json_value_as_object () const
  {
    if (is_json_object ())
      return std::make_optional<std::reference_wrapper<
          const std::unordered_map<std::string, Json> > > (
          std::cref (
              std::get<std::unordered_map<std::string, Json> > (value)));
    return std::nullopt;
  }

  const JSONValue&
  get_json_value_as_variant () const noexcept
  {
    return value;
  }

  // template <typename T>
  // const T &
  // get () const
  // {
  //   return std::get<T> (value);
  // }

  std::string
  to_string (int indent = 0) const
  {
    std::ostringstream oss;
    print_value (value, oss, indent, 0);
    return oss.str ();
  }

  constexpr double
  to_number () const noexcept
  {
    if (is_json_number ())
      return std::get<double> (value);
    return std::numeric_limits<double>::quiet_NaN ();
  }

  constexpr bool
  to_bool () const noexcept
  {
    return is_json_boolean () ? std::get<bool> (value) : false;
  }

  Json &
  get_json_element_if_exists (const std::string &key)
  {
    if (!is_json_object ())
      throw std::invalid_argument ("JSON element is not a JSON object!");
    auto &parent_element
        = std::get<std::unordered_map<std::string, Json> > (value);
    if (!parent_element.contains (key))
      throw std::out_of_range{ std::format (
          "JSON element with key {} is not found!", key) };
    return parent_element.at (key);
  }

  const Json &
  get_json_element_if_exists (const std::string &key) const
  {

    if (!is_json_object ())
      throw std::invalid_argument ("JSON element is not a JSON object!");
    const auto &parent_element
        = std::get<std::unordered_map<std::string, Json> > (value);
    if (!parent_element.contains (key))
      throw std::out_of_range{ std::format (
          "JSON element with key {} is not found!", key) };
    return parent_element.at (key);
  }

  Json &
  get_json_element (const std::string &key)
  {
    static Json null_json{ nullptr };
    if (!is_json_object ())
      return null_json;
    auto &parent_element
        = std::get<std::unordered_map<std::string, Json> > (value);
    if (!parent_element.contains (key))
      parent_element[key] = null_json;
    return parent_element[key];
  }

  const Json &
  get_json_element (const std::string &key) const
  {
    static const Json null_json{ nullptr };
    if (!is_json_object ())
      return null_json;
    const auto &parent_element
        = std::get<std::unordered_map<std::string, Json> > (value);
    if (parent_element.contains (key))
      return parent_element.at (key);
    return null_json;
  }

  std::optional<
      std::reference_wrapper<const std::unordered_map<std::string, Json> > >
  get_json_element_as_json_object (const std::string &key,
                                   std::ostream *error_stream = nullptr) const
  {

    if (!is_json_object ())
      {
        if (key.empty () && is_json_object ())
          return std::make_optional (std::cref (
              std::get<std::unordered_map<std::string, Json> > (value)));
        return std::nullopt;
      }
    try
      {
        const auto &parent_element
            = std::get<std::unordered_map<std::string, Json> > (value);

        if (parent_element.contains (key))
          {
            const auto &child_element = parent_element.at (key);
            return std::make_optional (
                std::cref (std::get<std::unordered_map<std::string, Json> > (
                    child_element.value)));
          }
      }
    catch (const std::exception &ex)
      {
        if (error_stream)
          {
            *error_stream << ex.what ();
          }
      }

    return {};
  }

  std::optional<std::reference_wrapper<const std::vector<Json> > >
  get_json_element_as_json_array (const std::string &key,
                                  std::ostream *error_stream = nullptr) const
  {
    if (!is_json_object ())
      {
        if (key.empty () && is_json_array ())
          return std::make_optional (
              std::cref (std::get<std::vector<Json> > (value)));
        return std::nullopt;
      }
    try
      {
        const auto &parent_element
            = std::get<std::unordered_map<std::string, Json> > (value);

        if (parent_element.contains (key))
          {
            const auto &child_element = parent_element.at (key);
            return std::make_optional (std::cref (
                std::get<std::vector<Json> > (child_element.value)));
          }
      }
    catch (const std::exception &ex)
      {
        if (error_stream)
          {
            *error_stream << ex.what ();
          }
      }

    return {};
  }

  std::optional<std::string>
  get_json_element_as_json_string (const std::string &key,
                                   std::ostream *error_stream = nullptr) const
  {
    static constexpr std::nullptr_t null_value{};
    if (!is_json_object ())
      {
        if (key.empty () && is_json_string ())
          return std::make_optional<std::string> (
              std::get<std::string> (value));
        return std::nullopt;
      }
    try
      {
        const auto &parent_element
            = std::get<std::unordered_map<std::string, Json> > (value);

        if (parent_element.contains (key))
          {
            const auto &child_element = parent_element.at (key);
            return std::make_optional (
                std::get<std::string> (child_element.value));
          }
      }
    catch (const std::exception &ex)
      {
        if (error_stream)
          {
            *error_stream << ex.what ();
          }
      }

    return null_value;
  }

  std::optional<double>
  get_json_element_as_json_number (const std::string &key,
                                   std::ostream *error_stream = nullptr) const
  {

    if (!is_json_object ())
      {
        if (key.empty () && is_json_number ())
          return std::make_optional<double> (std::get<double> (value));
        return std::nullopt;
      }
    try
      {
        const auto &parent_element
            = std::get<std::unordered_map<std::string, Json> > (value);

        if (parent_element.contains (key))
          {
            const auto &child_element = parent_element.at (key);
            return std::make_optional<double> (
                std::get<double> (child_element.value));
          }
      }
    catch (const std::exception &ex)
      {
        if (error_stream)
          {
            *error_stream << ex.what ();
          }
      }

    return std::nullopt;
  }

  std::optional<bool>
  get_json_element_as_json_boolean (const std::string &key,
                                    std::ostream *error_stream = nullptr) const
  {
    if (!is_json_object ())
      {
        if (key.empty () && is_json_boolean ())
          return std::make_optional<bool> (std::get<bool> (value));
        return std::nullopt;
      }
    try
      {
        const auto &parent_element
            = std::get<std::unordered_map<std::string, Json> > (value);

        if (parent_element.contains (key))
          {
            const auto &child_element = parent_element.at (key);
            return std::make_optional<bool> (
                std::get<bool> (child_element.value));
          }
      }
    catch (const std::exception &ex)
      {
        if (error_stream)
          {
            *error_stream << ex.what ();
          }
      }

    return std::nullopt;
  }

  std::optional<std::nullptr_t>
  get_json_element_as_json_null (const std::string &key,
                                 std::ostream *error_stream = nullptr) const
  {
    if (!is_json_object ())
      {
        if (key.empty () && is_json_null ())
          return std::make_optional<std::nullptr_t> (nullptr);
        return std::nullopt;
      }
    try
      {
        const auto &parent_element
            = std::get<std::unordered_map<std::string, Json> > (value);

        if (parent_element.contains (key))
          {
            const auto &child_element = parent_element.at (key);
            return std::make_optional<std::nullptr_t> (
                std::get<std::nullptr_t> (child_element.value));
          }
      }
    catch (const std::exception &ex)
      {
        if (error_stream)
          {
            *error_stream << ex.what ();
          }
      }

    return std::nullopt;
  }

  constexpr json_type
  get_json_element_type () const noexcept
  {
    if (get_if<std::unordered_map<std::string, Json> > (&value))
      return json_type::object_t;

    if (std::get_if<std::vector<Json> > (&value))
      return json_type::array_t;

    if (std::get_if<std::string> (&value))
      return json_type::string_t;

    if (std::get_if<double> (&value))
      return json_type::number_t;

    if (std::get_if<bool> (&value))
      return json_type::boolean_t;

    return json_type::null_t;
  }

  constexpr bool
  is_json_object () const noexcept
  {
    return std::get_if<std::unordered_map<std::string, Json> > (&value)
           != nullptr;
  }

  constexpr bool
  is_json_array () const noexcept
  {
    return std::get_if<std::vector<Json> > (&value) != nullptr;
  }

  constexpr bool
  is_json_string () const noexcept
  {
    return std::get_if<std::string> (&value) != nullptr;
  }

  constexpr bool
  is_json_number () const noexcept
  {
    return std::get_if<double> (&value) != nullptr;
  }

  constexpr bool
  is_json_boolean () const noexcept
  {
    return std::get_if<bool> (&value) != nullptr;
  }

  constexpr bool
  is_json_null () const noexcept
  {
    return std::get_if<std::nullptr_t> (&value) != nullptr;
  }

  // accessor method Json::at(key)
  Json &
  at (const std::string &key)
  {
    return get_json_element_if_exists (key);
  }

  const Json &
  at (const std::string &key) const
  {
    return get_json_element_if_exists (key);
  }

  // Json& operator[]
  Json &
  operator[] (const std::string &key)
  {
    return get_json_element (key);
  }

  // const Json& operator[]
  const Json &
  operator[] (const std::string &key) const
  {
    return get_json_element (key);
  }

private:
  JSONValue value;
};

struct result_type
{
  std::optional<Json> result_value;
  status result_status;
  std::string result_string;
  explicit result_type (std::optional<Json> res_value,
                        status res_status = status::success,
                        std::string res_string = {})
      : result_value{ std::move (res_value) }, result_status (res_status),
        result_string{ std::move (res_string) }
  {
  }
};

inline result_type
operator"" _json (const char *json_string, const size_t)
{
  return parse (json_string);
}

} // namespace simple_json

#endif // SIMPLE_JSON_H
