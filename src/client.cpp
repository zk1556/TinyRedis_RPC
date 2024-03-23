#include <iostream>
#include <string>
#include "buttonrpc.hpp"
using namespace std;
int main() {
    string hostName = "127.0.0.1";
    int port = 5555;

    buttonrpc client;
    client.as_client(hostName, port);
    client.set_timeout(2000);

    string message;
    while(true){
        //发送数据
        std::cout << hostName << ":" << port << "> ";
        std::getline(std::cin, message);
        string res = client.call<string>("redis_command", message).val();
        //添加结束字符 
        if(res.find("stop") != std::string::npos){
            break;
        }
        std::cout << res << std::endl;
    }
    return 0;
}
