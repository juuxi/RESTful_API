#include <thread>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <queue>
#include <string>
#include <iostream>
#include "json.hpp" 
#include "db.hpp"

class HttpServer {
    void receive(void* arg);
    void process(void* arg);
    std::queue<std::string> requst_queue;
public:
    void waiter();
};

int flag_rcv = 0;
int flag_process = 0;
int flag_wait = 0;
int client_sock;
int listen_sock;
struct sockaddr_in addr;
struct sockaddr_in addr2;
pthread_mutex_t mutex;
std::thread *t1 = nullptr, *t2 = nullptr;

void HttpServer::receive(void* arg) {
    int client_fd = *((int*)arg);
    free(((int*)arg));

    printf("поток приема начал работу\n");
    while (flag_rcv == 0) {
        char rcv_msg[256];
        int rv = recv(client_fd, rcv_msg, sizeof(rcv_msg), 0);
        if (rv == -1) {
            perror("receive");
            sleep(1);
        } else if (rv == 0) {
            shutdown(client_fd, 0);
            break;
        } else {
            pthread_mutex_lock(&mutex);
            requst_queue.push(std::string(rcv_msg, rv));
            pthread_mutex_unlock(&mutex);
        }
    }
    close(client_fd);
    printf("поток приема закончил работу\n");
}

void HttpServer::process(void* arg) {
    printf("поток обработки начал работу\n");
    int client_fd = *((int*)arg);
    free(((int*)arg));

    while (flag_process == 0) {
        pthread_mutex_lock(&mutex);
        if (!requst_queue.empty()) {
            std::string first_ent = requst_queue.front();
            requst_queue.pop();
            pthread_mutex_unlock(&mutex);

            std::cout << "Сообщение\n" << first_ent << "Принято" << std::endl;
            std::string http_method = first_ent.substr(0, first_ent.find(' '));
            std::string endpoint = first_ent.substr(first_ent.find('/') + 1, first_ent.find("HTTP") - first_ent.find('/') - 2);
            std::string entry_body = first_ent.substr(first_ent.find('{'));
            nlohmann::json data;
            if (!entry_body.empty()) {
                data = nlohmann::json::parse(entry_body);
            }
            
            char send_msg[256];
            std::string body;
            std::string status_code;
            DataBase db;

            if (endpoint == "borough") {
                if (http_method == "HEAD") {
                    status_code = "200 OK";
                    body = "";
                }
                if (http_method == "GET") {
                    nlohmann::json j = db.read(data);
                    std::string result = j["result"];
                    if (result != "No connection to db") {
                        body = j.dump();
                        std::cout << body << std::endl << std::endl;
                        status_code = "200 OK";
                    }

                    else {
                        std::cout << std::endl << "No connection to db" << std::endl << std::endl;
                        status_code = "503 Service unavailable";
                        body = "";
                    }

                }
                if (http_method == "POST") { 
                    int res = db.write(data);
                    if (res == 0) {
                        body = "";
                        status_code = "200 OK";
                    }
                    else {
                        body = "";
                        status_code = "500 Internal Server Error";
                    }
                }

                if (http_method == "PATCH") {
                    int res = db.update(data);

                    if (res == 0) {
                        body = "";
                        status_code = "200 OK";
                    }
                    else {
                        body = "";
                        status_code = "500 Internal Server Error";
                    }
                }

                if (http_method == "DELETE") {
                    int res = db.remove(data);

                    if (res == 0) {
                        body = "";
                        status_code = "200 OK";
                    }
                    else {
                        body = "";
                        status_code = "500 Internal Server Error";
                    }
                }
            }

            else if (endpoint == "ping") {
                if (http_method == "HEAD") {
                    body = "";
                    status_code = "200 OK";
                }
                if (http_method == "GET") {
                    body = "Hello from server\n";
                    status_code = "200 OK";
                }
                if (http_method == "PATCH") {
                    body = "You're not allowed to watch this\n";
                    status_code = "403 Forbidden";
                }
                if (http_method == "POST") {
                    body = "You're not allowed to touch this\n";
                    status_code = "403 Forbidden";
                }

                if (http_method == "POST") {
                    body = "You're not allowed to touch this\n";
                    status_code = "403 Forbidden";
                }
            }

            else { //неизвестный endpoint
                body = "";
                status_code = "400 Bad Request";
            }

            sprintf(send_msg, //создание ответного HTTP-запроса
                "HTTP/1.1 %s\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s", status_code.c_str(), int(body.size()), body.c_str());

            int rv = send(client_fd, send_msg, strlen(send_msg), 0);
            if (rv == -1) {
                perror("send");
            }
        } else {
            pthread_mutex_unlock(&mutex);
            usleep(100000);
        }
    }
    close(client_fd);
    printf("поток обработки закончил работу\n");
}


void HttpServer::waiter() {
    printf("поток ожидания соединений начал работу\n");
    while (flag_wait == 0) {
        socklen_t len = sizeof(addr);
        int client_fd = accept(listen_sock, (struct sockaddr*)&addr, &len);
        if (client_fd == -1) {
            sleep(1);
        } else {
            int* client_fd1 = new int;
            int* client_fd2 = new int;
            *client_fd1 = client_fd;
            *client_fd2 = client_fd;

            t1 = new std::thread(&HttpServer::receive, this, client_fd1);
            t2 = new std::thread(&HttpServer::process, this, client_fd2);
        }
    }
    printf("поток ожидания соединений закончил работу\n");
}

void sig_handler(int) {
    close(client_sock);
    close(listen_sock);
    exit(0);
}

int main() {
    printf("программа сервера начала работу\n");
    signal(SIGINT, sig_handler);
    HttpServer server;

    pthread_mutex_init(&mutex, NULL);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(listen_sock, F_SETFL, O_NONBLOCK);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int rv = bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr));
    if (rv == -1) {
        perror("bind");
    }

    int optval = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR,
    &optval, sizeof(optval));

    listen(listen_sock, 100);


    std::thread t3(&HttpServer::waiter, server);
    printf("программа ждет нажатия клавиши\n");
    getchar();
    printf("клавиша нажата\n");
    flag_rcv = 1;
    flag_process = 1;
    flag_wait = 1;

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

    t3.join();

    close(client_sock);
    close(listen_sock);

    printf("программа сервера закончила работу\n");
}