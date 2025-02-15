#include "../include/simple_json.h"

#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;
using namespace simple_json;

TEST (simple_json_library, creating_a_default_json_object)
{
  Json json;
  ASSERT_EQ (std::get<std::nullptr_t> (json.get_json_value_as_variant ()),
             nullptr);
  const auto json_value_optional{ json.get_json_value_as_null () };
  ASSERT_TRUE (json_value_optional.has_value ()
               && json_value_optional.value () == nullptr);
}

TEST (simple_json_library, creating_a_json_object_with_nullptr_value)
{
  Json json{ nullptr };
  auto json_value_as_variant = json.get_json_value_as_variant ();
  ASSERT_TRUE (std::get<std::nullptr_t> (json_value_as_variant) == nullptr);
  auto json_value_optional{ json.get_json_value_as_null () };
  ASSERT_TRUE (json_value_optional.has_value ()
               && json_value_optional.value () == nullptr);
}

TEST (simple_json_library, creating_a_json_object_with_boolean_value)
{
  Json json1{ true };
  auto json1_value_as_variant = json1.get_json_value_as_variant ();
  ASSERT_TRUE (std::get<bool> (json1_value_as_variant));
  auto json1_value_as_optional = json1.get_json_value_as_bool ();
  ASSERT_TRUE (json1_value_as_optional.has_value ()
               && json1_value_as_optional.value ());

  Json json2{ false };
  auto json2_value_as_variant = json2.get_json_value_as_variant ();
  ASSERT_TRUE (!std::get<bool> (json2_value_as_variant));
  auto json2_value_as_optional = json2.get_json_value_as_bool ();
  ASSERT_TRUE (json2_value_as_optional.has_value ()
               && !json2_value_as_optional.value ());
}

TEST (simple_json_library, creating_a_json_object_with_number_value)
{
  static constexpr double epsilon{ 0.000001 };
  Json json{ 5.32 };
  ASSERT_TRUE (
      std::abs (std::get<double> (json.get_json_value_as_variant ()) - 5.32)
      < epsilon);
  auto json_element_value = json.get_json_value_as_number ();
  ASSERT_TRUE (json_element_value.has_value ()
               && std::abs (json_element_value.value () - 5.32) < epsilon);
}

TEST (simple_json_library, creating_a_json_object_with_string)
{
  static constexpr std::string apple{ "apple" };
  Json json1{ apple };
  ASSERT_EQ (std::get<std::string> (json1.get_json_value_as_variant ()),
             apple);
  auto json_element_value1 = json1.get_json_value_as_string ();
  ASSERT_TRUE (json_element_value1.has_value ()
               && (json_element_value1.value ().get () == apple));

  static constexpr std::string car_key{ "car key" };
  Json json2{ car_key };
  ASSERT_EQ (std::get<std::string> (json2.get_json_value_as_variant ()),
             car_key);
  auto json_element_value2 = json2.get_json_value_as_string ();
  ASSERT_TRUE (json_element_value2.has_value ()
               && (json_element_value2.value ().get () == car_key));
}

TEST (simple_json_library, creating_a_json_object_with_array)
{
  static constexpr double epsilon{ 0.000001 };
  std::array<Json, 5> array_elements{ Json{ 1.1 }, Json{ 2.2 },
                                                Json{ 3.3 }, Json{ 4.4 },
                                                Json{ 5.5 } };
  std::vector<Json> arr (std::cbegin (array_elements),
                         std::cend (array_elements));
  Json json1{ arr };
  ASSERT_TRUE (json1.get_json_value_as_array ().has_value ());
  ASSERT_EQ (json1.get_json_value_as_array ()->get ().size (),
             array_elements.size ());
  const std::vector<Json> &json_arr1{ std::get<std::vector<Json> > (
      json1.get_json_value_as_variant ()) };
  ASSERT_EQ (json_arr1.size (), arr.size ());
  for (size_t i{}; i < json_arr1.size (); ++i)
    {
      ASSERT_TRUE (std::abs (json_arr1[i].to_number () - arr[i].to_number ())
                   < epsilon);
    }

  Json json2{ std::move (arr) };
  ASSERT_TRUE (json2.get_json_value_as_array ().has_value ());
  ASSERT_EQ (json2.get_json_value_as_array ()->get ().size (),
             array_elements.size ());
  const std::vector<Json> &json_arr2{ std::get<std::vector<Json> > (
      json2.get_json_value_as_variant ()) };
  ASSERT_EQ (json_arr2.size (), array_elements.size ());
  for (size_t i{}; i < json_arr2.size (); ++i)
    {
      ASSERT_TRUE (
          std::abs (json_arr2[i].to_number () - array_elements[i].to_number ())
          < epsilon);
    }
}

TEST (simple_json_library, creating_a_json_object_with_map)
{

  std::unordered_map<std::string, Json> map_elements{
    { "name", Json ("Alice") },
    { "age", Json (25.) },
    { "is_student", Json (true) },
    { "scores",
      Json (std::vector<Json>{ Json (88.5), Json (97.), Json (78.) }) },
    { "address",
      Json (std::unordered_map<std::string, Json>{
          { "city", Json ("Los Angeles") }, { "zip", Json (90001) } }) }
  };

  Json json1{ map_elements };
  ASSERT_TRUE (json1.get_json_value_as_object ().has_value ());
  ASSERT_EQ (json1.get_json_value_as_object ()->get ().size (),
             map_elements.size ());
  const std::unordered_map<std::string, Json> &json_map1{
    std::get<std::unordered_map<std::string, Json> > (
        json1.get_json_value_as_variant ())
  };
  ASSERT_EQ (json_map1.size (), map_elements.size ());

  Json json2{ std::move (map_elements) };
  ASSERT_TRUE (json2.get_json_value_as_object ().has_value ());
  ASSERT_EQ (json2.get_json_value_as_object ()->get ().size (),
             json1.get_json_value_as_object ()->get ().size ());
  const std::unordered_map<std::string, Json> &json_map2{
    std::get<std::unordered_map<std::string, Json> > (
        json2.get_json_value_as_variant ())
  };
  ASSERT_EQ (json_map2.size (), json_map1.size ());
}

TEST (simple_json_library, creating_a_json_object_by_parsing_input_string)
{
  const std::string json_input_string{
    R"({
        "name": "Alice",
        "age": 25,
        "is_student": true,
        "scores": [88.5, 92, 79],
        "address": {
            "city": "Los Angeles",
            "zip": "90001"
        }
    })"
  };

  auto [json_object, is_success, error_string] = parse (json_input_string);
  ASSERT_TRUE (status::success == is_success);
  ASSERT_TRUE (status::fail != is_success);
  ASSERT_TRUE (error_string.empty ());

  auto json_name_value{ json_object->get_child_as_json_string ("name") };
  ASSERT_TRUE (json_name_value.has_value ());
  ASSERT_EQ (json_name_value.value ().get(), "Alice");

  auto json_age_value{ json_object->get_child_as_json_number ("age") };
  ASSERT_TRUE (json_age_value.has_value ());
  ASSERT_EQ (json_age_value.value (), 25);

  auto json_is_student_value{ json_object->get_child_as_json_boolean (
      "is_student") };
  ASSERT_TRUE (json_is_student_value.has_value ());
  ASSERT_TRUE (json_is_student_value.value ());

  static constexpr double epsilon{ 0.000001 };
  const std::vector<double> scores{ 88.5, 92, 79 };
  auto json_scores_array_optional{ json_object->get_child_as_json_array (
      "scores") };
  ASSERT_TRUE (json_scores_array_optional.has_value ());
  const auto &json_scores_array = json_scores_array_optional.value ().get ();
  ASSERT_EQ (json_scores_array.size (), 3);
  for (size_t i{}; i < std::min (json_scores_array.size (), scores.size ());
       ++i)
    {
      ASSERT_TRUE (std::abs (json_scores_array[i].to_number () - scores[i])
                   < epsilon);
    }

  const std::unordered_map<std::string, Json> address_information{
    { "city", Json{ "Los Angeles" } }, { "zip", Json{ "90001" } }
  };
  auto json_address_object_optional{ json_object->get_child_as_json_object (
      "address") };
  ASSERT_TRUE (json_address_object_optional.has_value ());
  const auto &json_address_object
      = json_address_object_optional.value ().get ();
  ASSERT_EQ (json_address_object.size (), 2);
  ASSERT_TRUE (json_address_object.contains ("city"));
  const auto json_city_value{ json_address_object.at ("city").to_string () };
  ASSERT_TRUE (!json_city_value.empty ());
  ASSERT_EQ (json_city_value, "Los Angeles");

  ASSERT_TRUE (json_address_object.contains ("zip"));
  const auto json_zip_value{ json_address_object.at ("zip").to_string () };
  ASSERT_TRUE (!json_zip_value.empty ());
  ASSERT_EQ (json_zip_value, "90001");
}

TEST (simple_json_library,
      creating_a_json_object_by_parsing_json_string_literal)
{
  auto [json_object, is_success, error_string] =
      R"({
        "name": "Alice",
        "age": 25,
        "is_student": true,
        "scores": [88.5, 92, 79],
        "address": {
            "city": "Los Angeles",
            "zip": "90001"
        }
    })"_json;

  ASSERT_TRUE (status::success == is_success);
  ASSERT_TRUE (status::fail != is_success);
  ASSERT_TRUE (error_string.empty ());

  auto json_name_value{ json_object->get_child_as_json_string ("name") };
  ASSERT_TRUE (json_name_value.has_value ());
  ASSERT_EQ (json_name_value.value ().get(), "Alice");

  auto json_age_value{ json_object->get_child_as_json_number ("age") };
  ASSERT_TRUE (json_age_value.has_value ());
  ASSERT_EQ (json_age_value.value (), 25);

  auto json_is_student_value{ json_object->get_child_as_json_boolean (
      "is_student") };
  ASSERT_TRUE (json_is_student_value.has_value ());
  ASSERT_TRUE (json_is_student_value.value ());

  static constexpr double epsilon{ 0.000001 };
  const std::vector<double> scores{ 88.5, 92, 79 };
  auto json_scores_array_optional{ json_object->get_child_as_json_array (
      "scores") };
  ASSERT_TRUE (json_scores_array_optional.has_value ());
  const auto &json_scores_array = json_scores_array_optional.value ().get ();
  ASSERT_EQ (json_scores_array.size (), 3);
  for (size_t i{}; i < std::min (json_scores_array.size (), scores.size ());
       ++i)
    {
      ASSERT_TRUE (std::abs (json_scores_array[i].to_number () - scores[i])
                   < epsilon);
    }

  const std::unordered_map<std::string, Json> address_information{
    { "city", Json{ "Los Angeles" } }, { "zip", Json{ "90001" } }
  };
  auto json_address_object_optional{ json_object->get_child_as_json_object (
      "address") };
  ASSERT_TRUE (json_address_object_optional.has_value ());
  const auto &json_address_object
      = json_address_object_optional.value ().get ();
  ASSERT_EQ (json_address_object.size (), 2);
  ASSERT_TRUE (json_address_object.contains ("city"));
  const auto json_city_value{ json_address_object.at ("city").to_string () };
  ASSERT_TRUE (!json_city_value.empty ());
  ASSERT_EQ (json_city_value, "Los Angeles");

  ASSERT_TRUE (json_address_object.contains ("zip"));
  const auto json_zip_value{ json_address_object.at ("zip").to_string () };
  ASSERT_TRUE (!json_zip_value.empty ());
  ASSERT_EQ (json_zip_value, "90001");
}

int
main (int argc, char **argv)
{
  // const string jsonString1 = R"({
  //       "name": "Alice",
  //       "age": 25,
  //       "is_student": true,
  //       "scores": [88.5, 92, 79],
  //       "address": {
  //           "city": "Los Angeles",
  //           "zip": "90001"
  //       }
  //   })";

  //
  // try {
  //   const json parsed_json = json::parse(jsonString1);
  //
  //   cout << "Formatted JSON output:\n";
  //   cout << parsed_json << "\n";
  //
  //   const string json_str{parsed_json.to_string(2)};
  //   cout << "\nparsed_json as a string:\n" << json_str << "\n";
  // } catch (const exception& e) {
  //   cerr << "Error parsing json data: " << e.what() << endl;
  // }
  //
  // try {
  //   json parsed_json = R"({
  //       "name": "Alice",
  //       "age": 25,
  //       "is_student": true,
  //       "scores": [88.5, 92, 79],
  //       "address": {
  //           "city": "Los Angeles",
  //           "zip": "90001"
  //       }
  //   })"_json;
  //
  //   cout << "Formatted JSON output:\n";
  //   cout << parsed_json << "\n";
  //
  //   const string json_str{parsed_json.to_string(2)};
  //   cout << "\nparsed_json as a string:\n" << json_str << "\n";
  // } catch (const exception& e) {
  //   cerr << "Error parsing json data: " << e.what() << endl;
  // }
  //
  // ifstream input_file{"sample.json", std::ios::in};
  // if (input_file) {
  //   json parsed_json;
  //   input_file >> parsed_json;
  //
  //   cout << "\nFormatted JSON output:\n";
  //   cout << parsed_json << "\n";
  //
  //   const string json_str{parsed_json.to_string(2)};
  //   cout << "\nparsed_json as a string:\n" << json_str << "\n";
  // }

  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}
