#include <memory>
#include <string>
#include <stdexcept>
#include <mutex>
#include <libpq-fe.h>

class PGConnection
{
public:
    PGConnection();
    std::shared_ptr<PGconn> connection() const;
    PGconn* res;

private:
    void establish_connection();

    std::string m_dbhost = "localhost";
    int         m_dbport = 5432;
    std::string m_dbname = "";
    std::string m_dbuser = "";
    std::string m_dbpass = "";

    std::shared_ptr<PGconn>  m_connection;

};

PGConnection::PGConnection()
{
    res = PQsetdbLogin(m_dbhost.c_str(), std::to_string(m_dbport).c_str(), nullptr, nullptr, m_dbname.c_str(), m_dbuser.c_str(), m_dbpass.c_str());
}