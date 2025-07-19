#include "json.hpp" 
#include <fstream>
#include "postgreconn.hpp"

class DataBase {
    PGConnection pg;
public:
    DataBase();
    void write(nlohmann::json);
    nlohmann::json read(nlohmann::json);
    void update(nlohmann::json);
};