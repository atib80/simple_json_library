#include "include/simple_json.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace simple_json;


int main() {
    const string jsonString1 = R"({
        "name": "Alice",
        "age": 25,
        "is_student": true,
        "scores": [88.5, 92, 79],
        "address": {
            "city": "Los Angeles",
            "zip": "90001"
        }
    })";

    try {
        const json parsed_json = json::parse(jsonString1);

        cout << "Formatted JSON output:\n";
        cout << parsed_json << "\n";

        const string json_str{ parsed_json.to_string(2) };
        cout << "\nparsed_json as a string:\n" << json_str << "\n";

    } catch (const exception& e) {
        cerr << "Error parsing json data: " << e.what() << endl;
    }

    try {
        json parsed_json = R"({
        "name": "Alice",
        "age": 25,
        "is_student": true,
        "scores": [88.5, 92, 79],
        "address": {
            "city": "Los Angeles",
            "zip": "90001"
        }
    })"_json;

        cout << "Formatted JSON output:\n";
        cout << parsed_json << "\n";

        const string json_str{ parsed_json.to_string(2) };
        cout << "\nparsed_json as a string:\n" << json_str << "\n";

    } catch (const exception& e) {
        cerr << "Error parsing json data: " << e.what() << endl;
    }

    ifstream input_file{ "sample.json", std::ios::in };
    if (input_file) {

        json parsed_json;
        input_file >> parsed_json;

        cout << "\nFormatted JSON output:\n";
        cout << parsed_json << "\n";

        const string json_str{ parsed_json.to_string(2) };
        cout << "\nparsed_json as a string:\n" << json_str << "\n";
        
    }

    return 0;
}
