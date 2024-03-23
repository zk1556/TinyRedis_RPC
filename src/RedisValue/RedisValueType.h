#ifndef REDISVALUETYPE_H
#define REDISVALUETYPE_H
#include<map>
#include<vector>
#include<iostream>
#include<cassert>
#include"Parse.h"
#include"RedisValue.h"
#include"Dump.h"

/*
     
*/
class RedisValueType{
protected:
    friend class RedisValue;
    virtual RedisValue::Type type() const = 0;
    virtual bool equals(const RedisValueType*other) const = 0;
    virtual bool less(const RedisValueType*other) const = 0;
    virtual void dump(std::string& out) const = 0;
    virtual std::string &stringValue() ;
    virtual RedisValue::array &arrayItems() ;
    virtual RedisValue::object &objectItems() ;
    virtual RedisValue& operator[] (size_t i) ;
    virtual RedisValue& operator[](const std::string &key) ;
    virtual ~RedisValueType(){}

};

template<RedisValue::Type tag,typename T>
class Value : public RedisValueType{
protected:
    T value;
protected:
    explicit Value(const T& value) : value(value){}
    explicit Value(T&&value) : value(std::move(value)){}
    T getValue(){
        return value;
    }
    RedisValue::Type type() const override{
        return tag;
    }

    bool equals(const RedisValueType* other) const override {
        return value == static_cast<const Value<tag,T>*>(other)->value;
    }

    bool less(const RedisValueType* other) const override{
        return value < static_cast<const Value<tag, T> *>(other)->value;
    }

    void dump(std::string&out) const override{ ::dump(value,out);}
};




class RedisString final:public Value<RedisValue::STRING,std::string>{
    std::string & stringValue()override { return value;}
public:
    explicit RedisString(const std::string& value): Value(value){}
    explicit RedisString(std::string&& value) : Value(std::move(value)) {}
};

class RedisList final : public Value<RedisValue::ARRAY, RedisValue::array>{
    RedisValue::array & arrayItems()  override{ return value;}
    RedisValue & operator[](size_t i)  override;
public:
    explicit RedisList(const RedisValue::array &value): Value(value){}
    explicit RedisList(RedisValue::array &&value) : Value(std::move(value)){}
};

class RedisObject final : public Value<RedisValue::OBJECT,RedisValue::object>{
    RedisValue::object & objectItems()  override{ return value;}
    RedisValue& operator[] (const std::string& key)  override;
public:
    explicit RedisObject(const RedisValue::object &value) : Value(value) {} 
    explicit RedisObject(RedisValue::object &&value)      : Value(std::move(value)) {} 
};

class RedisValueNull final: public Value<RedisValue::NUL,NullStruct>{
public:
    RedisValueNull() : Value({}) {}
};
#endif