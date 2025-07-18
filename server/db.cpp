#include "db.hpp"

DataBase::DataBase() {
    std::fstream db_info("../server/db_info.txt"); //подключение к БД, данные берутся из скрытого для посторонних глаз текстового файла
    std::string db_name;
    std::string db_user;
    std::string db_pass;
    std::getline(db_info, db_name);
    std::getline(db_info, db_user);
    std::getline(db_info, db_pass);
    pg.set_dbname(db_name);
    pg.set_dbuser(db_user);
    pg.set_dbpass(db_pass);
    pg.reconnect();
    db_info.close();
}

void DataBase::write(nlohmann::json) {

}

nlohmann::json DataBase::read(nlohmann::json data) {
    std::string what, where;
    nlohmann::json j;
    if (data.find("what") != data.end()) { //какие данные нужно вернуть
        what = data["what"];
    }
    if (data.find("where") != data.end()) { //условие
        where = data["where"];
    }
    if (pg.res) {
        char query[256];
        sprintf(query, "SELECT %s FROM one \
            WHERE %s", what.c_str(), where.c_str());
        PGresult* res = PQexec(pg.res, query);
        //std::cout << PQgetvalue(res, 0, 0) << std::endl; //вывести первую ячейку в возвращемой таблице
        j["result"] = PQgetvalue(res, 0, 0);
        return j;
    }

    else {
        j["result"] = "No connection to db";
        return j;
    }
}