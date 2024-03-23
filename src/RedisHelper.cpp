#include"RedisHelper.h"
#include"FileCreator.h"


void RedisHelper::flush(){
    // 打开文件并覆盖写入
    std::string filePath=getFilePath();
    std::ofstream outputFile(filePath);
    // 检查文件是否成功打开
    if (!outputFile) {
        std::cout<<"文件："<<filePath<<"打开失败"<<std::endl;
        return ;
    }
    auto currentNode=redisDataBase->getHead();
    while(currentNode!=nullptr){
        std::string key=currentNode->key;
        RedisValue value=currentNode->value;
        if(!key.empty())
            outputFile<<key<<":"<<value.dump()<<std::endl;
        currentNode=currentNode->forward[0];
    }
    // 关闭文件
    outputFile.close();
}

std::string RedisHelper::getFilePath(){
    std::string folder = DEFAULT_DB_FOLDER; //文件夹名
    std::string fileName = DATABASE_FILE_NAME; //文件名
    std::string filePath=folder+"/"+fileName+dataBaseIndex; //文件路径
    return filePath;
}

//从文件中加载
void RedisHelper::loadData(std::string loadPath){
    redisDataBase->loadFile(loadPath);
}

//选择数据库
std::string RedisHelper::select(int index){
    if(index<0||index>DATABASE_FILE_NUMBER-1){
        return "database index out of range.";
    }
    flush(); //选择数据库之前先写入一下
    redisDataBase=std::make_shared<SkipList<std::string, RedisValue>>();
    dataBaseIndex=std::to_string(index);
    std::string filePath=getFilePath(); //根据选择的数据库，修改文件路径，然后加载

    loadData(filePath);
    return "OK";
}
// key操作命令
// 获取所有键
// 语法：keys pattern
// 127.0.0.1:6379> keys *
// 1) "javastack"
// *表示通配符，表示任意字符，会遍历所有键显示所有的键列表，时间复杂度O(n)，在生产环境不建议使用。
std::string RedisHelper::keys(const std::string pattern){
    std::string res="";
    auto node=redisDataBase->getHead()->forward[0];
    int count=0;
    while(node!=nullptr){
        res+=std::to_string(++count)+") "+"\""+node->key+"\""+"\n";
        node=node->forward[0];
    }
    if(!res.empty())
        res.pop_back();
    else{
        res="this database is empty!";
    }
    return res;
}
// 获取键总数
// 语法：dbsize
// 127.0.0.1:6379> dbsize
// (integer) 6
// 获取键总数时不会遍历所有的键，直接获取内部变量，时间复杂度O(1)。
std::string RedisHelper::dbsize()const{
    std::string res="(integer) " +std::to_string(redisDataBase->size());
    return res;
}
// 查询键是否存在
// 语法：exists key [key ...]
// 127.0.0.1:6379> exists javastack java
// (integer) 2
// 查询查询多个，返回存在的个数。
std::string RedisHelper::exists(const std::vector<std::string>&keys){
    int count=0;
    for(auto& key:keys){
        if(redisDataBase->searchItem(key)!=nullptr){
            count++;
        }
    }
    std::string res="(integer) " +std::to_string(count);
    return res;
}
// 删除键
// 语法：del key [key ...]
// 127.0.0.1:6379> del java javastack
// (integer) 1
// 可以删除多个，返回删除成功的个数。
std::string RedisHelper::del(const std::vector<std::string>&keys){
    int count=0;
    for(auto& key:keys){
        if(redisDataBase->deleteItem(key)){
            count++;
        }
    }
        std::string res="(integer) " +std::to_string(count);
    return res;
}

// 更改键名称
// 语法：rename key newkey
// 127.0.0.1:6379[2]> rename javastack javastack123
// OK
std::string RedisHelper::rename(const std::string&oldName,const std::string&newName){
    auto currentNode=redisDataBase->searchItem(oldName);
    std::string resMessage="";
    if(currentNode==nullptr){
        resMessage+=oldName+" does not exist!";
        return resMessage;
    }
    currentNode->key=newName;
    resMessage="OK";
    return resMessage;
}

// 字符串操作命令
// 存放键值
// 语法：set key value [EX seconds] [PX milliseconds] [NX|XX]
// nx：如果key不存在则建立，xx：如果key存在则修改其值，也可以直接使用setnx/setex命令。
std::string RedisHelper::set(const std::string& key, const RedisValue& value,const SET_MODEL model){
    
    if(model==XX){
        return setex(key,value);
    }else if(model==NX){
        return setnx(key,value);
    }else{
        auto currentNode=redisDataBase->searchItem(key);
        if(currentNode==nullptr){
            setnx(key,value);
        }else{
            setex(key,value);
        }
    }
    
    return "OK";
}

std::string RedisHelper::setnx(const std::string& key, const RedisValue& value){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode!=nullptr){
        return "key: "+ key +"  exists!";
    }else{
        redisDataBase->addItem(key,value);
        
    }
    return "OK";
}
std::string RedisHelper::setex(const std::string& key, const RedisValue& value){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr){
        return "key: "+ key +" does not exist!";
    }else{
        currentNode->value=value;
    }
    return "OK";
}
// 127.0.0.1:6379> set javastack 666
// OK
// 获取键值
// 语法：get key
// 127.0.0.1:6379[2]> get javastack
// "666"
std::string RedisHelper::get(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr){
        return "key: "+ key +" does not exist!";
    }
    return currentNode->value.dump();

}
// 值递增/递减
// 如果字符串中的值是数字类型的，可以使用incr命令每次递增，不是数字类型则报错。

// 语法：incr key
// 127.0.0.1:6379[2]> incr javastack
// (integer) 667
// 一次想递增N用incrby命令，如果是浮点型数据可以用incrbyfloat命令递增。
std::string RedisHelper::incr(const std::string& key){
    return incrby(key,1);
}
std::string RedisHelper::incrby(const std::string& key,int increment){
    auto currentNode=redisDataBase->searchItem(key);
    std::string value="";
    if(currentNode==nullptr){
        value=std::to_string(increment);
        redisDataBase->addItem(key,value);
        return "(integer) "+value;
    }
    value=currentNode->value.dump();
    //去掉双引号
    value.erase(0, 1); 
    value.erase(value.size()-1);
    for(char ch:value){
        if(!isdigit(ch)){
            std::string res="The value of "+key +" is not a numeric type";
            return res;
        }
    }
    int curValue=std::stoi(value)+increment;
    value=std::to_string(curValue);
    currentNode->value=value;
    std::string res="(integer) "+value;
    return res;
}
std::string RedisHelper::incrbyfloat(const std::string&key,double increment){
    auto currentNode=redisDataBase->searchItem(key);
    std::string value="";
    if(currentNode==nullptr){
        value=std::to_string(increment);
        redisDataBase->addItem(key,value);
        return "(float) "+value;
    }
    value=currentNode->value.dump();
    value.erase(0, 1); 
    value.erase(value.size()-1);
    double curValue=0.0;
    try {
        curValue = std::stod(value)+increment;
    } catch (std::invalid_argument const &e) {
        return "The value of "+key +" is not a numeric type";
    } 
    value=std::to_string(curValue);
    currentNode->value=value;
    std::string res="(float) "+value;
    return res;
}
// 同样，递减使用decr、decrby命令。
std::string RedisHelper::decr(const std::string&key){
    return incrby(key,-1);
}
std::string RedisHelper::decrby(const std::string&key,int increment){
    return incrby(key,-increment);
}
// 批量存放键值
// 语法：mset key value [key value ...]
// 127.0.0.1:6379[2]> mset java1 1 java2 2 java3 3
// OK

std::string RedisHelper::mset(std::vector<std::string>&items){
    if(items.size()%2!=0){
        return "wrong number of arguments for MSET.";
    }
    for(int i=0;i<items.size();i+=2){
        std::string key=items[i];
        std::string value=items[i+1];
        set(key,value);
    }
    return "OK";
}
// 获取获取键值
// 语法：mget key [key ...]
// 127.0.0.1:6379[2]> mget java1 java2
// 1) "1"
// 2) "2"
// Redis接收的是UTF-8的编码，如果是中文一个汉字将占3位返回。
std::string RedisHelper::mget(std::vector<std::string>&keys){
    if(keys.size()==0){
        return "wrong number of arguments for MGET.";
    }
    std::vector<std::string>values;
    std::string res="";
    for(int i=0;i<keys.size();i++){
        std::string& key=keys[i];
        std::string value="";
        auto currentNode=redisDataBase->searchItem(key);
        if(currentNode==nullptr){
            value="(nil)";
            res+=std::to_string(i+1)+") "+value+"\n";
        }else{
            value=currentNode->value.dump();
            res+=std::to_string(i+1)+") "+value+"\n";
        }
        
    }
    res.pop_back();
    return res;
}
// 获取值长度
// 语法：strlen key
// 127.0.0.1:6379[2]> strlen javastack (integer) 3
std::string RedisHelper::strlen(const std::string& key){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr){
        return "(integer) 0";
    }
    return "(integer) "+std::to_string(currentNode->value.dump().size());
}
// 追加内容
// 语法：append key value
// 127.0.0.1:6379[2]> append javastack hi
// (integer) 5
// 向键值尾部添加，如上命令执行后由666变成666hi
std::string RedisHelper::append(const std::string&key,const std::string &value){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr){
        redisDataBase->addItem(key,value);
        return "(integer) "+std::to_string(value.size());
    }
    currentNode->value=currentNode->value.dump()+value;
    return "(integer) "+std::to_string(currentNode->value.dump().size());
}


RedisHelper::RedisHelper(){
    FileCreator::createFolderAndFiles(DEFAULT_DB_FOLDER,DATABASE_FILE_NAME,DATABASE_FILE_NUMBER);
    std::string filePath=getFilePath();
    loadData(filePath);
}
RedisHelper::~RedisHelper(){flush();}


//列表操作
// LPUSH key value：将一个值插入到列表头部。
// RPUSH key value：将一个值插入到列表尾部。
// LPOP key：移出并获取列表的第一个元素。
// RPOP key：移出并获取列表的最后一个元素。
// LRANGE key start stop：获取列表指定范围内的元素。
std::string RedisHelper::lpush(const std::string&key,const std::string &value){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr){
        std::vector<RedisValue>data;
        RedisValue redisList(data) ;
        RedisValue::array& valueList = redisList.arrayItems();
        valueList.insert(valueList.begin(),value);
        redisDataBase->addItem(key,redisList);
        size = 1;
    }else{
        if(currentNode->value.type()!=RedisValue::ARRAY){
            resMessage="The key:" +key+" "+"already exists and the value is not a list!";
            return resMessage;
        }else{
            RedisValue::array& valueList = currentNode->value.arrayItems();
            valueList.insert(valueList.begin(),value);
            size = valueList.size();
        }
    }

    resMessage="(integer) "+std::to_string(size);
    return resMessage;
}
std::string RedisHelper::rpush(const std::string&key,const std::string &value){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr){
        std::vector<RedisValue>data;
        RedisValue redisList(data) ;
        RedisValue::array& valueList = redisList.arrayItems();
        valueList.push_back(value);
        redisDataBase->addItem(key,redisList);
        size = 1;
    }else{
        if(currentNode->value.type()!=RedisValue::ARRAY){
            resMessage="The key:" +key+" "+"already exists and the value is not a list!";
            return resMessage;
        }else{
            RedisValue::array& valueList = currentNode->value.arrayItems();
            valueList.push_back(value);
            size = valueList.size();
        }
    }

    resMessage="(integer) "+std::to_string(size);
    return resMessage;
}
std::string RedisHelper::lpop(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::ARRAY){
        resMessage="(nil)";
    }else{
        
        RedisValue::array& valueList = currentNode->value.arrayItems();
        resMessage = (*valueList.begin()).dump();
        valueList.erase(valueList.begin());
        resMessage.erase(0,1);
        resMessage.erase(resMessage.size()-1);
    }
    return resMessage;
}
std::string RedisHelper::rpop(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::ARRAY){
        resMessage="(nil)";
    }else{
        RedisValue::array& valueList = currentNode->value.arrayItems();
        resMessage = (valueList.back()).dump();
        valueList.pop_back();
        resMessage.erase(0,1);
        resMessage.erase(resMessage.size()-1);
    }
    return resMessage;
}
std::string RedisHelper::lrange(const std::string&key,const std::string &start,const std::string&end){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::ARRAY){
        resMessage="(nil)";
    }else{
        
        RedisValue::array& valueList = currentNode->value.arrayItems();
        int left = std::stoi(start);
        int right = std::stoi(end);
        left = std::max(left,0);
        right = std::min(right,int(valueList.size()-1));
        if(right<left||left>=valueList.size()){
            resMessage="(empty list or set)";
        }
        for(int i=left;i<=right;i++){
            auto item = valueList[i];
            resMessage+=std::to_string(i+1)+") "+item.dump();
            if(i!=right){
                resMessage+="\n";
            }
        }
    }
    return resMessage;
}

// 哈希表操作
// HSET key field value：向哈希表中添加一个字段及其值。
// HGET key field：获取哈希表中指定字段的值。
// HDEL key field：删除哈希表 key 中的一个或多个指定字段。
// HKEYS key：获取哈希表中的所有字段名。
// HVALS key：获取哈希表中的所有值。


std::string RedisHelper::hset(const std::string&key,const std::vector<std::string>&filed){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr){
        std::map<std::string,RedisValue>data;
        RedisValue redisHash(data) ;
        RedisValue::object& valueMap = redisHash.objectItems();
        for(int i=0;i<filed.size();i+=2){
            std::string hkey=filed[i];
            RedisValue hval=filed[i+1];
            if(!valueMap.count(hkey)){
                valueMap[hkey] = hval;
                count++;
            }
        }
        redisDataBase->addItem(key,valueMap);
    }else{
        if(currentNode->value.type()!=RedisValue::OBJECT){
            resMessage="The key:" +key+" "+"already exists and the value is not a hashtable!";
            return resMessage;
        }else{
            RedisValue::object& valueMap = currentNode->value.objectItems();
            for(int i=0;i<filed.size();i+=2){
                std::string hkey=filed[i];
                RedisValue hval=filed[i+1];
                if(!valueMap.count(hkey)){
                    valueMap[hkey] = hval;
                    count++;
                }
            }
        }
    }

    resMessage="(integer) "+std::to_string(count);
    return resMessage;
}
std::string RedisHelper::hget(const std::string&key,const std::string&filed){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::OBJECT){
        resMessage="(nil)";
    }else{
        RedisValue::object& valueMap = currentNode->value.objectItems();
        if(!valueMap.count(filed)){
            resMessage="(nil)";
        }else{
            resMessage = valueMap[filed].stringValue();
        }
        
    }
    return resMessage;
}
std::string RedisHelper::hdel(const std::string&key,const std::vector<std::string>&filed){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::OBJECT){
        count = 0;
    }else{
        RedisValue::object& valueMap = currentNode->value.objectItems();
        for(auto& hkey:filed){
            if(valueMap.count(hkey)){
                count++;
                valueMap.erase(hkey);
            }
        }
    }
    resMessage="(integer) "+std::to_string(count);
    return resMessage;
}

std::string RedisHelper::hkeys(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr){
        resMessage="The key:" +key+" "+"does not exist!";
        return resMessage;
    }else{
        if(currentNode->value.type()!=RedisValue::OBJECT){
            resMessage="The key:" +key+" "+"already exists and the value is not a hashtable!";
            return resMessage;
        }else{
            RedisValue::object& valueMap = currentNode->value.objectItems();
            int index = 1;
            for(auto hkey:valueMap){
                resMessage+=std::to_string(index)+") "+hkey.first+"\n";
                index++;
            }
            resMessage.erase(resMessage.size()-1);
        }
    }
    return resMessage;
}

std::string RedisHelper::hvals(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr){
        resMessage="The key:" +key+" "+"does not exist!";
        return resMessage;
    }else{
        if(currentNode->value.type()!=RedisValue::OBJECT){
            resMessage="The key:" +key+" "+"already exists and the value is not a hashtable!";
            return resMessage;
        }else{
            RedisValue::object& valueMap = currentNode->value.objectItems();
            int index = 1;
            for(auto hkey:valueMap){
                resMessage+=std::to_string(index)+") "+hkey.second.stringValue()+"\n";
                index++;
            }
            resMessage.erase(resMessage.size()-1);
        }
    }
    return resMessage;
}