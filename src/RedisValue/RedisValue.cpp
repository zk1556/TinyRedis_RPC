
#include"Global.h"
#include "Parse.h"


/*************构造函数******************/

RedisValue::RedisValue() noexcept : redisValue(statics().null) {}

RedisValue::RedisValue(std::nullptr_t) noexcept : redisValue(statics().null) {}

RedisValue::RedisValue(const std::string& value) : redisValue(std::make_shared<RedisString>(value)) {}

RedisValue::RedisValue(std::string&& value) : redisValue(std::make_shared<RedisString>(std::move(value))) {}

RedisValue::RedisValue(const char* value) : redisValue(std::make_shared<RedisString>(value)) {}

RedisValue::RedisValue(const RedisValue::array& value) : redisValue(std::make_shared<RedisList>(value)) {}

RedisValue::RedisValue(RedisValue::array&& value) : redisValue(std::make_shared<RedisList>(std::move(value))) {}

RedisValue::RedisValue(const RedisValue::object& value) : redisValue(std::make_shared<RedisObject>(value)) {}

RedisValue::RedisValue(RedisValue::object &&value) : redisValue(std::make_shared<RedisObject>(std::move(value))) {}

/************* Member Functions ******************/

RedisValue::Type RedisValue::type() const {
    return redisValue->type();
}

std::string & RedisValue::stringValue() {
    return redisValue->stringValue();
}

std::vector<RedisValue> & RedisValue::arrayItems() {
    return redisValue->arrayItems();
}

std::map<std::string, RedisValue> & RedisValue::objectItems()  {
    return redisValue->objectItems();
}

RedisValue & RedisValue::operator[] (size_t i)  {
    return (*redisValue)[i];
}

RedisValue & RedisValue::operator[] (const std::string& key) {
    return (*redisValue)[key];
}

std::string& RedisValueType::stringValue() {
    return statics().emptyString;
}

std::vector<RedisValue> & RedisValueType::arrayItems() {
    return statics().emptyVector;
}

std::map<std::string, RedisValue> & RedisValueType::objectItems() {
    return statics().emptyMap;
}

RedisValue& RedisValueType::operator[] (size_t) {
    return staticNull();
}

RedisValue& RedisValueType::operator[] (const std::string&) {
    return staticNull();
}

RedisValue& RedisObject::operator[] (const std::string&key) {
    auto it = value.find(key);
    return (it==value.end()) ? staticNull() : it->second;
}

RedisValue& RedisList::operator[](size_t i ) {
    if(i>=value.size()) return staticNull();
    return value[i];
}

/*比较*/

bool RedisValue::operator== (const RedisValue&other) const{
    if (redisValue == other.redisValue)
        return true;
    if (redisValue->type() != other.redisValue->type())
        return false;
    return redisValue->equals(other.redisValue.get());
}

bool RedisValue::operator< (const RedisValue& other) const{
    if (redisValue == other.redisValue)
        return false;
    if (redisValue->type() != other.redisValue->type())
        return redisValue->type() < other.redisValue->type();
    return redisValue->less(other.redisValue.get());
}


// 定义Json类的成员函数dump，用于将Json对象转化为JSON字符串并追加到out中
void RedisValue::dump(std::string &out) const {
    redisValue->dump(out); // 调用JsonImpl类的dump函数将Json对象转化为JSON字符串并追加到out中
}



// 将字符串转化为Json对象
RedisValue RedisValue::parse(const std::string &in, std::string &err) {
    // 初始化一个Json解析器
    RedisValueParser parser { in, 0, err, false};
    // 解析输入字符串以得到Json结果
    RedisValue result = parser.parseRedisValue(0);

    // 检查是否有尾随的垃圾字符
    parser.consumeGarbage();
    if (parser.failed)
        return RedisValue(); // 如果解析失败，返回一个空的Json对象
    if (parser.i != in.size())
        return parser.fail("unexpected trailing " + esc(in[parser.i])); // 如果输入字符串尚有未解析内容，报告错误

    return result; // 返回解析得到的Json对象
}

RedisValue RedisValue::parse(const char* in, std::string& err){
    if (in) {
            return parse(std::string(in), err);
    } else {
        err = "null input";
        return nullptr;
    }
}
// 解析输入字符串中的多个Json对象
std::vector<RedisValue> RedisValue::parseMulti(const std::string &in,
                               std::string::size_type &parser_stop_pos,
                               std::string &err) {
    // 初始化一个Json解析器
    RedisValueParser parser { in, 0, err, false };
    parser_stop_pos = 0;
    std::vector<RedisValue> jsonList; // 存储解析得到的多个Json对象的容器

    // 当输入字符串还有内容并且解析未出错时继续
    while (parser.i != in.size() && !parser.failed) {
        jsonList.push_back(parser.parseRedisValue(0)); // 解析Json对象并添加到容器中
        if (parser.failed)
            break; // 如果解析失败，中断循环

        // 检查是否还有其他对象
        parser.consumeGarbage();
        if (parser.failed)
            break; // 如果发现垃圾字符或有错误，中断循环
        parser_stop_pos = parser.i; // 更新停止位置
    }
    return jsonList; // 返回解析得到的Json对象容器
}

std::vector<RedisValue> RedisValue::parseMulti(
        const std::string & in,
        std::string & err
    )
{
        std::string::size_type parser_stop_pos;
        return parseMulti(in, parser_stop_pos, err);
}

// 检查 JSON 对象是否具有指定的形状
bool RedisValue::hasShape(const shape & types, std::string & err)  {
    // 如果 JSON 不是对象类型，则返回错误
    if (!isObject()) {
        err = "expected JSON object, got " + dump();
        return false;
    }

    // 获取 JSON 对象的所有成员项
    auto obj_items = objectItems();
    
    // 遍历指定的形状
    for (auto & item : types) {
        // 查找 JSON 对象中是否存在形状中指定的项
        const auto it = obj_items.find(item.first);
        
        // 如果找不到项或者项的类型不符合指定的类型，则返回错误
        if (it == obj_items.cend() || it->second.type() != item.second) {
            err = "bad type for " + item.first + " in " + dump();
            return false;
        }
    }

    // 如果所有形状都匹配，则返回 true
    return true;
}