cmake_minimum_required(VERSION 3.10)
project(RedisHelper)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置源代码目录和二进制文件目录
set(SRC_DIR ../src)
set(BIN_DIR ../bin)

# 添加源文件
set(SOURCE_FILES 
    ${SRC_DIR}/RedisHelper.cpp 
    ${SRC_DIR}/CommandParser.cpp 
    ${SRC_DIR}/RedisServer.cpp 
    ${SRC_DIR}/ParserFlyweightFactory.cpp 
    ${SRC_DIR}/RedisValue/Parse.cpp 
    ${SRC_DIR}/RedisValue/RedisValue.cpp
    ${SRC_DIR}/buttonrpc.hpp
    ${SRC_DIR}/Serializer.hpp
)

# 确保二进制文件目录存在
file(MAKE_DIRECTORY ${BIN_DIR})

# 编译server
add_executable(server ${SRC_DIR}/server.cpp ${SOURCE_FILES})
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
target_link_libraries(server zmq)
# 编译client
add_executable(client ${SRC_DIR}/client.cpp)
 set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
target_link_libraries(client zmq)