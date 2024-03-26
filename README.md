## TinyReids_RPC

  Linux下C++实现的基于RPC框架的轻量级Redis，主要实现以下功能：
- **RPC框架**：函数映射采用map和function实现，序列化和反序列化采用字节流实现，网路传输采用ZeroMQ。
- **数据持久化**：服务器关闭时，通过捕获信号实现数据自动保存到磁盘，支持选择多个数据库文件。
- **支持事务功能**：支持事务的执行和撤销，提供回滚操作。
- **跳表**：底层采用跳表，实现多种数据类型，包括字符串、列表、哈希表等。
- **命令解析**：命令解析，采用享元模式实现不同指令的解析： select、set、setnx、get、keys、exists、del、incr、incrby、incrbyfloat、decr、decrby、mset、mget、strlen append、multi、exec、discard、lpush、rpush、lpop、rpop、lrange、hset、hget、hdel、hkeys、hvals。

## 运行配置及使用
* zeroMQ库安装
```
sudo apt-get install libzmq3-dev
```
* 生成可执行程序
```
 mkdir build
 cd build
 cmake .. //生成Makefile文件

 make
```

* 运行可执行程序
```
 服务器： ./bin/server
 客户端： ./bin/client
```

## 项目文件介绍

___

​	项目采用模块化设计，主要包括客户端、服务端、命令解析、网路通信、序列化容器等模块。
以下是项目的目录结构及文件说明：
```
src
├── CommandParser.cpp               # 命令解析器实现文件，解析客户端命令。
├── CommandParser.h                 # 命令解析器头文件，定义命令解析相关类和方法。
├── FileCreator.h                   # 数据库文件创建和管理的头文件。
├── ParserFlyweightFactory.cpp      # 命令解析器实现文件
├── ParserFlyweightFactory.h        # 命令解析器享元工厂头文件，定义享元工厂相关类和方法。
├── RedisHelper.cpp                 # 提供数据库操作的辅助函数实现文件。
├── RedisHelper.h                   # 数据库操作辅助函数头文件。
├── RedisServer.cpp                 # Redis服务端主逻辑实现文件，包括连接管理和请求处理。
├── RedisServer.h                   # Redis服务端头文件，定义服务端相关类和方法。
├── RedisValue                      # Redis数据类型对象模块，处理不同类型的Redis数据类型。
│   ├── Dump.h                      # Redis数据导出头文件。
│   ├── Global.h                    # Redis数据类型对象模块的全局定义头文件。
│   ├── Parse.cpp                   # Redis数据类型解析实现文件。
│   ├── Parse.h                     # Redis数据类型解析头文件。
│   ├── RedisValue.cpp              # Redis数据类型对象实现文件。
│   ├── RedisValue.h                # Redis数据类型对象头文件，定义值对象相关类和方法。
│   └── RedisValueType.h            # 定义Redis数据类型类型的头文件。
├── Serializer.hpp                  # 定义RPC框架序列化和反序列化容器
├── SkipList.h                      # 跳表数据结构实现头文件
├── buttonrpc.hpp                   # 定义RPC框架函数调用和通信
├── client.cpp                      # 客户端启动逻辑，处理用户输入并与Redis服务器通信。    
├── global.h                        # 存放全局变量和定义，如支持的命令列表。
└── server.cpp                      # 服务端启动逻辑，初始化服务器并开始接受客户端连接。
```

## 运行效果
___

### 服务端

![服务端](https://github.com/zk1556/TinyRedis_RPC/blob/main/img/%E6%9C%8D%E5%8A%A1%E7%AB%AF.png)

### 客户端
![客户端](https://github.com/zk1556/TinyRedis_RPC/blob/main/img/%E5%AE%A2%E6%88%B7%E7%AB%AF.png)


## 参考
___

1. [myTinyRedis](https://github.com/haolian123/myTinyRedis) ：学习Redis命令解析
2. [力扣 1206. 设计跳表](https://leetcode.cn/problems/design-skiplist/) ：学习跳表的实现。
3.  [buttonrpc](https://github.com/button-chen/buttonrpc) :学习RPC框架使用。

