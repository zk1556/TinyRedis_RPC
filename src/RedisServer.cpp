#include"RedisServer.h"


RedisServer* RedisServer::getInstance()
{
	static RedisServer redis;
	return &redis;
}

void RedisServer::printLogo() {
    std::ifstream ifs(logoFilePath);
    if(!ifs.is_open()){
        std::cout<<"logoFilePath不存在"<<std::endl;
    }
    std::string line = "";
    while (std::getline(ifs, line)) {
        replaceText(line, "PORT", std::to_string(port));
        replaceText(line, "PTHREAD_ID", std::to_string(pid));
        std::cout << line << std::endl;
    }
}

void RedisServer::printStartMessage() {
    std::string startMessage = "[PID] DATE # Server started.";
    std::string initMessage = "[PID] DATE * The server is now ready to accept connections on port PORT";
    
    replaceText(startMessage, "PID", std::to_string(pid));
    replaceText(startMessage, "DATE", getDate());
    replaceText(initMessage, "PORT", std::to_string(port));
    replaceText(initMessage, "PID", std::to_string(pid));
    replaceText(initMessage, "DATE", getDate());
    
    std::cout << startMessage << std::endl;
    std::cout << initMessage << std::endl;
}

void RedisServer::start() {
    signal(SIGINT, signalHandler);  
    printLogo();
    printStartMessage();
    // string s ;
    // while (!stop) {
    //     getline(cin,s);
    //     cout << s <<endl;
    //     cout << RedisServer::handleClient(s) << endl;;                            
    // }
}


string RedisServer::executeTransaction(std::queue<std::string>&commandsQueue){
    //存储所有的执行结果
    std::vector<std::string>responseMessagesList; 
    while(!commandsQueue.empty()){
        std::string receivedData = std::move(commandsQueue.front());
        commandsQueue.pop();
        std::istringstream iss(receivedData);
        std::string command;
        std::vector<std::string> tokens;
        std::string responseMessage;

        
        while (iss >> command) {
            tokens.push_back(command);
        }
        command = tokens.front();
        if (!tokens.empty()) {
            command = tokens.front();
            std::string responseMessage;
            if(command=="quit"||command=="exit"){
                responseMessage="stop";
                return responseMessage;
            }else if(command=="multi"){
                responseMessage="Open the transaction repeatedly!";
                responseMessagesList.emplace_back(responseMessage);
                continue;
            }else if(command == "exec"){
                //处理未打开事物就执行的操作
                responseMessage="No transaction is opened!";
                 responseMessagesList.emplace_back(responseMessage);
                continue;
            }else{
                //处理常规指令
                
                std::shared_ptr<CommandParser> commandParser = flyweightFactory->getParser(command); //获取解析器
                
                try {
                    responseMessage = commandParser->parse(tokens);
                } catch (const std::exception& e) {
                    responseMessage = "Error processing command '" + command + "': " + e.what();
                }   
                responseMessagesList.emplace_back(responseMessage);
            }
                    
        }
                
    }
    string res = "";
    for(int i=0;i<responseMessagesList.size();i++){      
        std::string responseMessage = std::to_string(i+1)+")"+responseMessagesList[i];
        res += responseMessage;
        if(i!=responseMessagesList.size()-1){
            res+="\n";
        } 
    }

    return res;
}
    

string RedisServer::handleClient(string receivedData) {
    
   size_t bytesRead = receivedData.length();
     if (bytesRead > 0) {
         std::istringstream iss(receivedData);
         std::string command;
         std::vector<std::string> tokens;
         while (iss >> command) { //以字符串分割
             tokens.push_back(command);
         }

         if (!tokens.empty()) {
             command = tokens.front();
             std::string responseMessage;
             if (command == "quit" || command == "exit") {
                 responseMessage = "stop";
                 
                 return responseMessage;
             }
             else if (command == "multi") {
                 if (startMulti) {
                     responseMessage = "Open the transaction repeatedly!";
                     
                     return responseMessage;
                 }
                 startMulti = true;
                 std::queue<std::string> empty;
                 std::swap(empty, commandsQueue);
                 responseMessage = "OK";
                 return responseMessage;
             }
             else if (command == "exec") {
                 if (startMulti == false) {
                     //处理未打开事物就执行的操作
                     responseMessage = "No transaction is opened!";
                     return responseMessage;
                 }
                 startMulti = false;
                 if (!fallback) {
                     //执行事物
                     responseMessage =  executeTransaction(commandsQueue);

                     return responseMessage;
                 }
                 else {
                     fallback = false;
                     responseMessage = "(error) EXECABORT Transaction discarded because of previous errors.";
                     return responseMessage;
                 }
             }
             else if (command == "discard") {
                 startMulti = false;
                 fallback = false;
                 responseMessage = "OK";
                 return responseMessage;
             }
             else {
                 //处理常规指令
                 if (!startMulti) {
                    std::shared_ptr<CommandParser> commandParser = flyweightFactory->getParser(command);
                     if (commandParser == nullptr) {
                         responseMessage = "Error: Command '" + command + "' not recognized.";
                     }
                     else {
                         try {
                             responseMessage = commandParser->parse(tokens);
                         }
                         catch (const std::exception& e) {
                             responseMessage = "Error processing command '" + command + "': " + e.what();
                         }
                     }

                     // 发送响应消息回客户端
                     return responseMessage;
                 }
                 else {
                     //添加到事物队列中
                     std::shared_ptr<CommandParser> commandParser = flyweightFactory->getParser(command);
                     if (commandParser == nullptr) {
                        //编译错误,需要回退，后续增加回退功能
                        fallback = true;
                        responseMessage = "Error: Command '" + command + "' not recognized.";
                        return responseMessage;
                     }
                     else {
                        //加入到队列
                        commandsQueue.emplace(receivedData);
                        responseMessage = "QUEUED";
                        return responseMessage;
                     }

                 }

             }

         }
     }
     else {
         // 在非阻塞模式下，没有数据可读时继续循环
         return "nil";
     }
    
     return "error";
}



 //替换字符串中的指定字符
void RedisServer::replaceText(std::string &text, const std::string &toReplaceText, const std::string &newText) {
    size_t start_pos = text.find(toReplaceText); 
    while (start_pos != std::string::npos) {
        text.replace(start_pos, toReplaceText.length(), newText);
        start_pos = text.find(toReplaceText, start_pos + newText.length());
    }
}
//获取当前时间
std::string RedisServer::getDate() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm;
    localtime_r(&now_c, &local_tm); 

    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
 //信号处理函数
void RedisServer::signalHandler(int sig) {
    if (sig == SIGINT) {
        CommandParser::getRedisHelper()->flush();
        exit(0);
    }
}


RedisServer::RedisServer(int port, const std::string& logoFilePath) 
: port(port), logoFilePath(logoFilePath),
flyweightFactory(new ParserFlyweightFactory()){
    pid = getpid();
}
