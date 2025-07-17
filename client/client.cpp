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

void func1() {
    printf("поток отправки запросов начал работу\n");
    std::string http_method = "GET";
    char send_msg[256];
    while(flag_send == 0) {
        std::string endpoint = "borough";
        std::string body;
        if (http_method == "GET") {
            body = R"({ "what": "name", "where": "area=100"})"; //создание запроса с json-телом
        }
        else {
            body = R"({ "name": "Queens", "area": "50"})"; //создание запроса с json-телом
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
            http_method == "GET" ? http_method = "POST" : http_method = "GET";
            sleep(2);
        }
    }
    printf("поток отправки запросов закончил работу\n");
}

void func2() {
    printf("поток обработки ответов начал работу\n");
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

                if (curr.find('{') != curr.npos) {
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
                }
                else {
                    std::cout << "POST good" << std::endl;
                }
                curr.erase(0, curr.find("HTTP/1.1", 9));
            }
        }
       sleep(1);
    }
    printf("поток обработки ответов закончил работу\n");
}

void func3() {
    printf("поток установления соединения начал работу\n");
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
    printf("поток установления соединения закончил работу\n");
}

int main() {
    printf("программа клиента начала работу\n");

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
    printf("программа ждет нажатия клавиши\n");
    getchar();
    printf("клавиша нажата\n");
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

    printf("программа клиента закончила работу\n");
}