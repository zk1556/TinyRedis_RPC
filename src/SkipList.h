#ifndef SKIPLIST_H
#define SKIPLIST_H
#include<iostream>
#include<vector>
#include<memory>
#include<random>
#include<cstring>
#include<string>
#include<fstream>
#include<mutex>
#include"global.h"
#include"RedisValue/RedisValue.h"
#define MAX_SKIP_LIST_LEVEL 32
#define  PROBABILITY_FACTOR 0.25
#define  DELIMITER ":"
#define SAVE_PATH "data_file"
//定义跳表节点，包含key，value和指向当前层下一个节点的指针数组
/*

*/
template<typename Key,typename Value>
class SkipListNode{
public:
    Key key;
    Value value;
    std::vector<std::shared_ptr<SkipListNode<Key,Value>>>forward; // 指向下一个节点的指针数组
    SkipListNode(Key key,Value value,int maxLevel=MAX_SKIP_LIST_LEVEL):
    key(key),value(value),forward(maxLevel,nullptr){}
    
};

template<typename Key, typename Value>
class SkipList{
private:
    int currentLevel; //当前跳表的最大层数
    std::shared_ptr<SkipListNode<Key,Value>>head; //头节点
    std::mt19937 generator{ std::random_device{}()}; //随机数生成器
    std::uniform_real_distribution<double> distribution; //随机数分布
    int elementNumber=0; 
    std::ofstream writeFile; //写文件
    std::ifstream readFile; //读文件
    std::mutex mutex;
private:
    //随机生成新节点的层数
    int randomLevel();
    bool parseString(const std::string&line,std::string&key,std::string&value);
    bool isVaildString(const std::string&line);
public:
    SkipList();
    ~SkipList();
    bool addItem(const Key& key, const Value& value); //添加节点
    bool modifyItem(const Key& key, const Value& value); //修改节点
    std::shared_ptr<SkipListNode<Key,Value>> searchItem(const Key& key); //查找节点
    bool deleteItem(const Key& key); //删除节点
    void printList(); //打印跳表
    void dumpFile(std::string save_path); //保存跳表到文件
    void loadFile(std::string load_path); //从文件加载跳表
    int size(); //返回跳表元素个数
public:
    int getCurrentLevel(){return currentLevel;} //返回当前跳表的最大层数
        std::shared_ptr<SkipListNode<Key,Value>> getHead(){return head;} //返回头节点
};

/*--------------函数定义---------------------*/

template<typename Key,typename Value>
bool SkipList<Key,Value>::addItem(const Key& key,const Value& value){
    mutex.lock();
    auto currentNode=this->head; //从头节点开始查找
    std::vector<std::shared_ptr<SkipListNode<Key,Value>>>update(MAX_SKIP_LIST_LEVEL,head); //记录每层需要更新的节点
    //找到小于目标键值的最大节点
    for(int i=currentLevel-1;i>=0;i--){
        while(currentNode->forward[i]&&currentNode->forward[i]->key<key){
            currentNode=currentNode->forward[i];
        }
        update[i]=currentNode;
    }
    
    int newLevel=this->randomLevel(); //生成新节点的层数
    currentLevel=std::max(newLevel,currentLevel); //更新当前跳表的最大层数
    std::shared_ptr<SkipListNode<Key,Value>> newNode=std::make_shared<SkipListNode<Key,Value>>(key,value,newLevel); //只分配节点的层高
    for(int i=0;i<newLevel;i++){
        newNode->forward[i]=update[i]->forward[i];
        update[i]->forward[i]=newNode;
    }
    elementNumber++;
    mutex.unlock();
    return true;
}

template<typename Key,typename Value>
bool SkipList<Key,Value>::modifyItem(const Key&key, const Value& value){

    std::shared_ptr<SkipListNode<Key,Value>> targetNode=this->searchItem(key);
    mutex.lock();
    if(targetNode==nullptr){
        mutex.unlock();
        return false;
    }
    targetNode->value=value;
    mutex.unlock();
    return true;

}

//查找节点
template<typename Key,typename Value>
std::shared_ptr<SkipListNode<Key,Value>> SkipList<Key,Value>::searchItem(const Key& key){
    mutex.lock();
    std::shared_ptr<SkipListNode<Key,Value>> currentNode=this->head;
    if(!currentNode){
        mutex.unlock();
        return nullptr;
    }
    for(int i=currentLevel-1;i>=0;i--){
        while(currentNode->forward[i]!=nullptr&&currentNode->forward[i]->key<key){
            currentNode=currentNode->forward[i];
        }
    }
    currentNode=currentNode->forward[0];
    if(currentNode&&currentNode->key==key){
        mutex.unlock();
        return currentNode;
    }
    mutex.unlock();
    return nullptr;
}

template<typename Key,typename Value>
bool SkipList<Key,Value>::deleteItem(const Key& key){
    mutex.lock();
    std::shared_ptr<SkipListNode<Key,Value>> currentNode=this->head;
    std::vector<std::shared_ptr<SkipListNode<Key,Value>>>update(MAX_SKIP_LIST_LEVEL,head);
    for(int i=currentLevel-1;i>=0;i--){
        while(currentNode->forward[i]&&currentNode->forward[i]->key<key){
            currentNode=currentNode->forward[i];
        }
        update[i]=currentNode;
    }
    currentNode=currentNode->forward[0];
    if(!currentNode||currentNode->key!=key){
        mutex.unlock();
        return false;
    }
    for(int i=0;i<currentLevel;i++){
        if(update[i]->forward[i]!=currentNode){
            break;
        }
        update[i]->forward[i]=currentNode->forward[i];
    }
    currentNode.reset();
    while(currentLevel>1&&head->forward[currentLevel-1]==nullptr){
        currentLevel--;
    }
    elementNumber--;
    mutex.unlock();
    return true;
}

//打印跳表
template<typename Key,typename Value>
void SkipList<Key,Value>::printList(){
    mutex.lock();
    for(int i=currentLevel;i>=0;i--){
        auto node=this->head->forward[i];
        std::cout<<"Level"<<i+1<<":";
        while(node!=nullptr){
            std::cout<<node->key<<DELIMITER<<node->value<<"; ";
            node=node->forward[i];
        }
        std::cout<<std::endl;
    }
    mutex.unlock();
}

template<typename Key,typename Value>
void SkipList<Key,Value>::dumpFile( std::string save_path){
    mutex.lock();
    writeFile.open(save_path); //打开文件
    auto node=this->head->forward[0];  //从第一层开始遍历
    while(node!=nullptr){
        writeFile<<node->key<<DELIMITER<<node->value.dump()<<"\n"; //写入文件 dump()函数将value转换为字符串 整数 1->"1" 字符串 "hello"->"hello" 二进制数据 0x01 0x02 0x03->"0x01 0x02 0x03"
        node=node->forward[0];
    }
    writeFile.flush(); //刷新缓冲区 写入文件 直接写入文件 不用等到文件关闭 
    // 众所周与,你所要输出的内容会先存入缓冲区,而flush()的作用正是强行将缓冲区的数据清空
    writeFile.close(); //关闭文件
    mutex.unlock();
}



template<typename Key,typename Value>
void SkipList<Key,Value>::loadFile(std::string load_path){

    readFile.open(load_path); //打开文件
    if(!readFile.is_open()){ 
        mutex.unlock();
        return;
    }
    std::string line;
    std::string key;
    std::string value;
    std::string err;
    while(std::getline(readFile,line)){ //读取文件
        if(parseString(line,key,value)){
            addItem(key,RedisValue::parse(value,err)); //将字符串转换为RedisValue对象
        }
    }
    readFile.close();

}


template<typename Key,typename Value>
bool SkipList<Key,Value>::isVaildString(const std::string&line){
    if(line.empty()){
        return false;
    }
    if(line.find(DELIMITER)==std::string::npos){
        return false;
    }
    return true;
}

//解析字符串
template<typename Key,typename Value>
bool SkipList<Key,Value>::parseString(const std::string&line,std::string&key,std::string&value){
    if(!isVaildString(line)){
        return false;
    }
    int index=line.find(DELIMITER); //找到分隔符的位置 返回的是分隔符的位置
    key=line.substr(0,index);
    value=line.substr(index+1);
    return true;

} 

template<typename Key,typename Value>
int SkipList<Key,Value>::size(){
    mutex.lock();
    int ret=this->elementNumber;
    mutex.unlock();
    return ret;
}


template<typename Key,typename Value>
SkipList<Key,Value>::SkipList()
    :currentLevel(0),distribution(0, 1)
{
    Key key;
    Value value;
    this->head=std::make_shared<SkipListNode<Key,Value>>(key,value); //初始化头节点,层数为最大层数
}

//随机生成新节点的层数
template<typename Key, typename Value>
int SkipList<Key,Value>::randomLevel()
{
    int level=1;
    while(distribution(generator)< PROBABILITY_FACTOR
        && level<MAX_SKIP_LIST_LEVEL){
        level++;
    }
    return level;
}

template<typename Key,typename Value>
SkipList<Key,Value>::~SkipList(){
    if(this->readFile){
        readFile.close();
    }
    if(this->writeFile){
        writeFile.close();
    }
}
#endif