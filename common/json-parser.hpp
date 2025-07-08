#include <unordered_map>
#include <string>
#include <sstream>

namespace json {
    class JSON {
    public:
        std::unordered_map<std::string, std::string> parse(std::string);
    };

    std::unordered_map<std::string, std::string> JSON::parse(std::string came) {
        std::unordered_map<std::string, std::string> result;
        std::string s;
        std::stringstream input(came);
        std::getline(input, s);
        while (true) {
            std::getline(input, s);
            std::string key = s.substr(s.find(' ') + 1, s.find(":") - s.find(" ") - 1);
            std::string val = s.substr(s.find(':') + 2, s.find(",") - s.find(":"));
        }
    }
}