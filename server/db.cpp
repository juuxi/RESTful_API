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

void DataBase::write(nlohmann::json data) {
    std::string name="", population="", area="";
    std::string columns = "", values = "";
    if (data.find("name") != data.end()) {
        name = data["name"];
        columns += "name";
        values += "'";
        values += name;
        values += "'";
    }
    if (data.find("population") != data.end()) {
        population = data["population"];
        if (!columns.empty()) {
            columns += ", population";
            values += ", ";
            values += population;
        }
        else {
            columns += "population";
            values += population;
        }
    }
    if (data.find("area") != data.end()) {
        area = data["area"];
        if (!columns.empty()) {
            columns += ", area";
            values += ", ";
            values += area;
        }
        else {
            columns += "area";
            values += area;
        }
    }

    if (pg.res) {
        char query[256];
        sprintf(query, "INSERT INTO one (%s)\
            VALUES(%s)", columns.c_str(), values.c_str());
        PGresult* res = PQexec(pg.res, query);
    }
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

void DataBase::update(nlohmann::json data) {
    std::string what, how = "", where;
    if (data.find("what") != data.end()) { //что заменить
        what = data["what"];
    }
    if (data.find("how") != data.end()) { //на какие данные заменить
        how += "'";
        how += data["how"];
        how += "'";
    }
    if (data.find("where") != data.end()) { //условие
        where = data["where"];
    }
    if (pg.res) {
        char query[256];
        sprintf(query, "UPDATE one \
            SET %s = %s WHERE %s", what.c_str(), how.c_str(), where.c_str());
        PGresult* res = PQexec(pg.res, query);
    }
}

void DataBase::remove(nlohmann::json data) {
    std::string where;
    if (data.find("where") != data.end()) {
        where = data["where"];
    }

    if (pg.res) {
        char query[256]; //проверять есть ли такие данные, иначе можно поймать ошибку двойного удаления
        sprintf(query, "DELETE FROM one \
            WHERE %s", where.c_str());
        PGresult* res = PQexec(pg.res, query);
    }
}