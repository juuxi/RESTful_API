#include <unordered_map>
#include <string>

namespace json {
    class JSON {
    public:
        std::unordered_map<std::string, std::string> parse();
    };

    std::unordered_map<std::string, std::string> JSON::parse() {

    }
}