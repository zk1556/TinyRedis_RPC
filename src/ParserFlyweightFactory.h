#ifndef PARSER_FLYWEIGHT_FACTORY
#define PARSER_FLYWEIGHT_FACTORY
#include"CommandParser.h"
#include<unordered_map>
//享元模式工厂
/*
    这个类用来返回解析器的
*/
class ParserFlyweightFactory{
private:
    std::unordered_map<std::string,std::shared_ptr<CommandParser>> parserMaps; //解析器映射
    std::shared_ptr<CommandParser> createCommandParser(std::string& command); //创建解析器
public:
    std::shared_ptr<CommandParser> getParser(std::string& command); //获取解析器
};

#endif