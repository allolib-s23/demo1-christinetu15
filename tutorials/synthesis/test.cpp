// #include "json.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;
using namespace std;

// #include <iostream>
// #include <jsoncpp/json/value.h>
// #include <jsoncpp/json/json.h>
// #include <fstream>
// #include <string>

using namespace std;


int main() {
    ifstream f("guns_for_hire.json");
    cout << "FILE" << endl;
    cout << f.rdbuf();
    json music = json::parse(f);
    
    json notes = music["tracks"]["notes"];

    // ifstream file("guns_for_hire.json");
    // Json::Value actualJson;
    // Json::Reader reader;

    // reader.parse(file, actualJson);

    // Json::Value notes = actualJson["tracks"]["notes"];

    for(auto note: notes) {
        cout << note["name"] << endl;
        // playNote(freq_of(note["midi"]), note["time"], note["duration"]);
    }
}