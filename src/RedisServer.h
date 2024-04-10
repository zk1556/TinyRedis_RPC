#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <future>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <unistd.h>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <signal.h>
#include<fcntl.h>
#include <cstring> 
#include "ParserFlyweightFactory.h"
#include <queue>
#include <string>
using namespace std;
class RedisServer {
private:
    std::unique_ptr<ParserFlyweightFactory> flyweightFactory; // 解析器工厂
    int port;
    std::atomic<bool> stop{false};
    pid_t pid;
    std::string logoFilePath;
    bool startMulti = false;
    bool fallback = false;
    std::queue<std::string>commandsQueue;//事物指令队列

private:
    RedisServer(int port = 5555, const std::string& logoFilePath = MY_PROJECT_DIR_LOGO);
    static void signalHandler(int sig);
    void printLogo();
    void printStartMessage();
    void replaceText(std::string &text, const std::string &toReplaceText, const std::string &replaceText);
    std::string getDate();
    string executeTransaction(std::queue<std::string>&commandsQueue);
public:
string handleClient(string receivedData);
   static RedisServer* getInstance();
    void start();
};

#endif 
