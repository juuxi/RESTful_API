#include <thread>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include "json.hpp"

int flag_connect = 0;
int flag_send = 0;
int flag_receive = 0;
int server_sock;
struct sockaddr_in addr;
std::thread *t1 = nullptr, *t2 = nullptr;
std::string http_method;
int response_printed = 1;

int menu() {
    int res;
    std::cout << "1. Получить информацию" << std::endl;
    std::cout << "2. Добавить информацию" << std::endl;
    std::cout << "3. Изменить информацию" << std::endl;
    std::cout << "4. Удалить информацию" << std::endl;
    std::cout << "5. Выход" << std::endl;
    std::cout << "> ";
    std::cin >> res;
    std::cin.clear();
    return res;
}

int db_options_menu() {
    int res;
    std::cout << "1. Название" << std::endl;
    std::cout << "2. Население" << std::endl;
    std::cout << "3. Площадь" << std::endl;
    std::cout << "> ";
    std::cin >> res;
    std::cin.clear();
    return res;
}

void func1() {
    char send_msg[256];
    while(flag_send == 0) {
        if (!http_method.empty()) {
            std::string endpoint = "borough";
            std::string body;

            std::string what, how, where, name, area, population;
            if (http_method == "GET") {
                std::cout << "Какую информацию вы хотите получить?" << std::endl;
                switch(db_options_menu()) {
                    case 1: what = "name"; break;
                    case 2: what = "population"; break;
                    case 3: what = "area"; break;
                }
                std::cout << "При каком условии?" << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::getline(std::cin, where);
                body = "{ \"what\": \"";
                body += what;
                body += "\", \"where\": \"";
                body += where;
                body += "\" }";
                //body = "R({ "what": "name", "where": "area=50" })";
            }
            else if (http_method == "PATCH") {
                std::cout << "Какую информацию вы хотите изменить?" << std::endl;
                switch(db_options_menu()) {
                    case 1: what = "name"; break;
                    case 2: what = "population"; break;
                    case 3: what = "area"; break;
                }
                std::cout << "На что вы хотите заменить информацию?" << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::getline(std::cin, how);
                std::cout << "При каком условии?" << std::endl;
                std::getline(std::cin, where);
                body = "{ \"what\": \"";
                body += what;
                body += "\", \"how\" : \"";
                body += how;
                body += "\", \"where\": \"";
                body += where;
                body += "\" }";
                //body = R"({ "what" : "name", "how" : "Staten Island", "where" : "area=50" })";
            }
            else if (http_method == "POST") {
                body = R"({ "name": "Bronx", "area": "75"})"; 
            }
            else if (http_method == "DELETE") {
                std::cout << "При каком условии вы хотите удалить информацию?" << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::getline(std::cin, where);
                body = "{ \"where\": \"";
                body += where;
                body += "\" }";
                //body = R"({ "where" : "area=75" })";
            }

            sprintf(send_msg, 
                "%s /%s HTTP/1.1\r\n"
                "Host: localhost:8080\r\n"
                "User-Agent: curl/8.5.0\r\n"
                "Accept: */*\r\n"
                "\r\n"
                "%s", http_method.c_str(), endpoint.c_str(), body.c_str());

            int rv = send(server_sock, send_msg, strlen(send_msg), 0);
            if (rv == -1) {
                perror("send");
                sleep(1);
            }
            else {
                http_method = "";
            }
        }
    }
}

void func2() {
    while (flag_receive == 0) {
        char rcv_msg[256];
        socklen_t slen = sizeof(addr);
        int rv = recv(server_sock, rcv_msg, sizeof(rcv_msg), 0);
        if (rv == -1) {
            sleep(2);
        }
        else if (rv == 0) {
            shutdown(server_sock, 0);
            sleep(1);
        }
        else {
            std::string curr(rcv_msg, rv); 
            
            while (!curr.empty()) {
                int content_length_start = curr.find("Content-Length: ") + 16, content_length_end = curr.find("\r\n", curr.find("Content-Length: "));
                int length = std::stoi(curr.substr(content_length_start, content_length_end - content_length_start));

                if (length && curr.find('{') != curr.npos) {
                    std::string entry_body = curr.substr(curr.find('{'), length);
                    nlohmann::json data;
                    if (!entry_body.empty()) {
                        data = nlohmann::json::parse(entry_body);
                    } 
                    std::string result;
                    if (data.find("result") != data.end()) {
                        result = data["result"];
                    }
                    
                    printf("%s\n", result.c_str());
                    response_printed = 1;
                }
                else {
                    int status_code = std::stoi(curr.substr(curr.find(' ') + 1, 3));
                    if (status_code == 200) {
                        std::cout << "POST|PATCH|DELETE good" << std::endl;
                        response_printed = 1;
                    }
                    else {
                        std::cout << curr.substr(curr.find(' ') + 1, curr.find('\r') - curr.find(' ')) << std::endl;
                        response_printed = 1;
                    }
                }
                curr.erase(0, curr.find("HTTP/1.1", 9));
            }
        }
       sleep(1);
    }
}

void func3() {
    while (flag_connect == 0) {
        int result = connect(server_sock, (struct sockaddr*)&addr, sizeof(addr));
        if (result == -1) {
            perror("connect");
            sleep(1);
        }
        else {  
            t1 = new std::thread(func1);
            t2 = new std::thread(func2);
            break;
        }
    }
}

int main() {
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(server_sock, F_SETFL, O_NONBLOCK);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int rv = bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    if (rv == -1) {
        perror("bind");
    }

    int optval = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,
    &optval, sizeof(optval));

    std::thread t3(func3);
    
    bool end_flag = false;
    while (!end_flag) {
        while (!response_printed) {

        }
        switch(menu()) {
            case 1: http_method = "GET"; break;
            case 2: http_method = "POST"; break;
            case 3: http_method = "PATCH"; break;
            case 4: http_method = "DELETE"; break;
            case 5: end_flag = true; break;
            default: http_method = "INVALID"; break;
        }
        response_printed = 0;
    }
    
    flag_send = 1;
    flag_receive = 1;
    flag_connect = 1;
    t3.join();
    if (t1 && t1->joinable()) {
        t1->join();
        delete t1;
        t1 = nullptr;
    }
    if (t2 && t2->joinable()) {
        t2->join();
        delete t2;
        t2 = nullptr;
    }

    close(server_sock);
}