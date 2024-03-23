#ifndef PARSE_H
#define PARSE_H

#include<iostream>
#include"RedisValueType.h"
#include"RedisValue.h"
class RedisValueParser final {
public:
    const std::string &str;  // 要解析的JSON字符串
    size_t i;                // 当前解析位置的索引
    std::string &err;        // 用于存储解析过程中的错误信息
    bool failed;             // 解析是否失败的标志

    // 处理失败情况，设置错误信息并返回错误的Json对象
    RedisValue fail(std::string &&msg);

    // 通用的失败处理函数，设置错误信息并返回指定的错误返回值
    template <typename T>
    T fail(std::string &&msg, const T err_ret);

    // 跳过字符串中的空白字符
    void consumeWhitespace();

    // 跳过注释内容，支持解析策略中包含注释的情况
    bool consumeComment();

    // 跳过无效的字符序列
    void consumeGarbage();

    // 获取下一个有效的JSON标记（token），并移动解析位置
    char getNextToken();

    // 将Unicode码点转换为UTF-8编码，并追加到输出字符串中
    void encodeUTF8(long pt, std::string & out);

    // 解析JSON字符串，处理转义序列，并返回解析后的字符串
    std::string parseString();
   
    // 预期读取特定字符串，并在匹配时返回给定的Json结果
    RedisValue expect(const std::string &expected, RedisValue res);

    // 解析JSON文本，根据当前位置解析对应的Json对象或数组等，并处理嵌套情况
    RedisValue parseRedisValue(int depth);
};

#endif