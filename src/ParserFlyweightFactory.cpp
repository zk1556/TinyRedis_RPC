#include"ParserFlyweightFactory.h"

std::shared_ptr<CommandParser> ParserFlyweightFactory::getParser(std::string& command){
    if(parserMaps.find(command)!=parserMaps.end()){
        return parserMaps[command];
    }
    return createCommandParser(command);
}



std::shared_ptr<CommandParser> ParserFlyweightFactory::createCommandParser(std::string& command){
    Command op;
    if(commandMaps.find(command)==commandMaps.end()){ //命令不存在
        op=INVALID_COMMAND;
    }else{
        op=commandMaps[command]; //获取命令
    }
    
    switch(op){
        case SET:{
            parserMaps[command]=std::make_shared<SetParser>();
            break;
        }
        case SETNX:{
            parserMaps[command]=std::make_shared<SetnxParser>();
            break;
        }
        case SETEX:{
            parserMaps[command]=std::make_shared<SetexParser>();
            break;
        }
        case DBSIZE:{
            parserMaps[command]=std::make_shared<DBSizeParser>();
            break;
        }
        case GET:{
            parserMaps[command]=std::make_shared<GetParser>();
            break;
        }
        case KEYS:{
            parserMaps[command]=std::make_shared<KeysParser>();
            break;
        }
        case EXISTS:{
            parserMaps[command]=std::make_shared<ExistsParser>();
            break;
        }
        case DEL:{
            parserMaps[command]=std::make_shared<DelParser>();
            break;
        }
        case RENAME:{
            parserMaps[command]=std::make_shared<RenameParser>();
            break;
        }
        case INCR:{
            parserMaps[command]=std::make_shared<IncrParser>();
            break;
        }
        case INCRBY:{
            parserMaps[command]=std::make_shared<IncrbyParser>();
            break;
        }
        case INCRBYFLOAT:{
            parserMaps[command]=std::make_shared<IncrbyfloatParser>();
            break;
        }
        case DECR:{
            parserMaps[command]=std::make_shared<DecrParser>();
            break;
        }
        case DECRBY:{
            parserMaps[command]=std::make_shared<DecrbyParser>();
            break;
        }
        case MSET:{
            parserMaps[command]=std::make_shared<MSetParser>();
            break;
        }
        case MGET:{
            parserMaps[command]=std::make_shared<MGetParser>();
            break;
        }
        case STRLEN:{
            parserMaps[command]=std::make_shared<StrlenParser>();
            break;
        }
        case APPEND:{
            parserMaps[command]=std::make_shared<AppendParser>();
            break;
        }
        case SELECT:{
            parserMaps[command]=std::make_shared<SelectParser>();
            break;
        }
        case LPUSH:{
            parserMaps[command]=std::make_shared<LPushParser>();
            break;
        }
        case RPUSH:{
            parserMaps[command]=std::make_shared<RPushParser>();
            break;
        }
        case LPOP:{
            parserMaps[command]=std::make_shared<LPopParser>();
            break;
        }
        case RPOP:{
            parserMaps[command]=std::make_shared<RPopParser>();
            break;
        }
        case LRANGE:{
            parserMaps[command]=std::make_shared<LRangeParser>();
            break;
        }
        case HSET:{
            parserMaps[command]=std::make_shared<HSetParser>();
            break;
        }
        case HGET:{
            parserMaps[command]=std::make_shared<HGetParser>();
            break;
        }
        case HDEL:{
            parserMaps[command]=std::make_shared<HDelParser>();
            break;
        }
        case HKEYS:{
            parserMaps[command]=std::make_shared<HKeysParser>();
            break;
        }
        case HVALS:{
            parserMaps[command]=std::make_shared<HValsParser>();
            break;
        }
        default:{
            return nullptr;
        }
    }
    return parserMaps[command];
}