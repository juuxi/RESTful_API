#include "json.hpp" 
#include <fstream>
#include "postgreconn.hpp"

class DataBase {
    PGConnection pg;
public:
    DataBase();
    int write(nlohmann::json);
    nlohmann::json read(nlohmann::json);
    int update(nlohmann::json);
    int remove(nlohmann::json);
};