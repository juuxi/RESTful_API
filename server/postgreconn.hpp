#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <mutex>
#include <libpq-fe.h>

class PGConnection
{
public:
    PGConnection();
    void set_dbname(std::string _dbname) {m_dbname = _dbname;}
    void set_dbuser(std::string _dbuser) {m_dbuser = _dbuser;}
    void set_dbpass(std::string _dbpass) {m_dbpass = _dbpass;}
    void reconnect();
    std::shared_ptr<PGconn> connection() const;
    PGconn* res;
    ~PGConnection() {PQfinish(res);}

private:
    void establish_connection();

    std::string m_dbhost = "localhost";
    int         m_dbport = 5432;
    std::string m_dbname;
    std::string m_dbuser;
    std::string m_dbpass;

    std::shared_ptr<PGconn>  m_connection;

};