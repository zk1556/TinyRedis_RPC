#pragma once
#include <string>
#include <map>
#include <string>
#include <sstream>
#include <functional>
#include <zmq.hpp> //这个是zeroMQ的头文件
#include "Serializer.hpp"//这个是序列化和反序列化的头文件


//模板的别名，需要一个外敷类
// type_xx<int>::type a = 10;
template <typename T>
struct type_xx {
    typedef T type;
};

//特例化的模板，当T为void时，type为int8_t
template<>
struct type_xx<void> {
    typedef int8_t type;
};


class buttonrpc
{

public:
    enum rpc_role{ 
		RPC_CLIENT,
		RPC_SERVER
	};

    enum rpc_err_code { 
		RPC_ERR_SUCCESS = 0, //成功
		RPC_ERR_FUNCTIION_NOT_BIND,//函数未绑定
		RPC_ERR_RECV_TIMEOUT //接收超时
	};

    //返回值
    template<typename T>
    class value_t{
       public:
         typedef typename type_xx<T>::type type; //通过typename告诉编译器type_xx<T>::type 是一个类型
         typedef std::string msg_type;
         typedef uint16_t code_type;

        value_t() { code_ = 0; msg_.clear(); } 
        bool valid() { return (code_ == 0 ? true : false); } //判断是否有效
        int error_code() { return code_; } //返回错误码
        std::string error_msg() { return msg_; } 
        type val() { return val_; }	//返回值


        void set_val(const type& val) { val_ = val; }
        void set_code(code_type code) { code_ = code; }    
        void set_msg(msg_type msg) { msg_ = msg; }

        friend Serializer& operator >> (Serializer& in, value_t<T>& d) { //定义友元函数
            in >> d.code_ >> d.msg_; 
			if (d.code_ == 0) {
				in >> d.val_;
			}
			return in;
        }
        friend Serializer& operator << (Serializer& out, value_t<T> d) {
			out << d.code_ << d.msg_ << d.val_; //重载运算符<< 
			return out;
		}

       private:
        code_type code_;
		msg_type msg_;
		type val_;

    };

    buttonrpc();
	~buttonrpc();

    // network
    void as_client(std::string ip, int port); //客户端
	void as_server(int port); //服务器
	void send(zmq::message_t& data); //发送数据
	void recv(zmq::message_t& data);//接收数据
	void set_timeout(uint32_t ms);//设置超时时间
	void run();

public:
    // server
    template<typename F>
	void bind(std::string name, F func);

    template<typename F, typename S>
	void bind(std::string name, F func, S* s); //类成员函数

   
    // client
    template<typename R>
	value_t<R> call(std::string name); //无参

    template<typename R, typename P1>
	value_t<R> call(std::string name, P1); //一个参数

    template<typename R, typename P1, typename P2>
	value_t<R> call(std::string name, P1, P2);//两个参数

    template<typename R, typename P1, typename P2, typename P3>
	value_t<R> call(std::string name, P1, P2, P3);//三个参数

    template<typename R, typename P1, typename P2, typename P3, typename P4>
	value_t<R> call(std::string name, P1, P2, P3, P4); //四个参数

    template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	value_t<R> call(std::string name, P1, P2, P3, P4, P5); //五个参数

private:
    Serializer* call_(std::string name, const char* data, int len);

    template<typename R>
	value_t<R> net_call(Serializer& ds);

    template<typename F>
	void callproxy(F fun, Serializer* pr, const char* data, int len);

    template<typename F, typename S>
	void callproxy(F fun, S* s, Serializer* pr, const char* data, int len); //类成员函数

    // PROXY FUNCTION POINT
    template<typename R>
	void callproxy_(R(*func)(), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R()>(func), pr, data, len); //无参函数
	}
 
    template<typename R, typename P1>
	void callproxy_(R(*func)(P1), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1)>(func), pr, data, len);//一个参数
	}

    template<typename R, typename P1, typename P2>
	void callproxy_(R(*func)(P1, P2), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2)>(func), pr, data, len);
	}

	template<typename R, typename P1, typename P2, typename P3>
	void callproxy_(R(*func)(P1, P2, P3), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3)>(func), pr, data, len);
	}

    template<typename R, typename P1, typename P2, typename P3, typename P4>
	void callproxy_(R(*func)(P1, P2, P3, P4), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3, P4)>(func), pr, data, len);
	}

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	void callproxy_(R(*func)(P1, P2, P3, P4, P5), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3, P4, P5)>(func), pr, data, len);
	}

    // PROXY CLASS MEMBER，function不能包装类成员变量或函数，需要配合Bind,传入函数地址和类对象地址
    template<typename R, typename C, typename S>
	void callproxy_(R(C::* func)(), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R()>(std::bind(func, s)), pr, data, len);
	}

    template<typename R, typename C, typename S, typename P1>
	void callproxy_(R(C::* func)(P1), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1)>(std::bind(func, s, std::placeholders::_1)), pr, data, len);
	}

    template<typename R, typename C, typename S, typename P1, typename P2>
	void callproxy_(R(C::* func)(P1, P2), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2)>(std::bind(func, s, std::placeholders::_1, std::placeholders::_2)), pr, data, len);
	}

	template<typename R, typename C, typename S, typename P1, typename P2, typename P3>
	void callproxy_(R(C::* func)(P1, P2, P3), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3)>(std::bind(func, s, 
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), pr, data, len);
	}

	template<typename R, typename C, typename S, typename P1, typename P2, typename P3, typename P4>
	void callproxy_(R(C::* func)(P1, P2, P3, P4), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3, P4)>(std::bind(func, s,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)), pr, data, len);
	}

	template<typename R, typename C, typename S, typename P1, typename P2, typename P3, typename P4, typename P5>
	void callproxy_(R(C::* func)(P1, P2, P3, P4, P5), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3, P4, P5)>(std::bind(func, s,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)), pr, data, len);
	}

    // PORXY FUNCTIONAL
    template<typename R>
	void callproxy_(std::function<R()>, Serializer* pr, const char* data, int len);

    template<typename R, typename P1>
	void callproxy_(std::function<R(P1)>, Serializer* pr, const char* data, int len);

    template<typename R, typename P1, typename P2>
	void callproxy_(std::function<R(P1, P2)>, Serializer* pr, const char* data, int len);

	template<typename R, typename P1, typename P2, typename P3>
	void callproxy_(std::function<R(P1, P2, P3)>, Serializer* pr, const char* data, int len);

	template<typename R, typename P1, typename P2, typename P3, typename P4>
	void callproxy_(std::function<R(P1, P2, P3, P4)>, Serializer* pr, const char* data, int len);

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	void callproxy_(std::function<R(P1, P2, P3, P4, P5)>, Serializer* pr, const char* data, int len);

private:
    std::map<std::string, std::function<void(Serializer*, const char*, int)>> m_handlers; //函数映射表

    zmq::context_t m_context; //上下文
    zmq::socket_t* m_socket; //套接字

    rpc_err_code m_error_code; //错误码
    int m_role; //角色
};  

buttonrpc::buttonrpc() : m_context(1){ 
	m_error_code = RPC_ERR_SUCCESS; 
}

buttonrpc::~buttonrpc(){ 
	m_socket->close(); //关闭套接字
	delete m_socket;
	m_context.close(); //关闭上下文
}

// network
void buttonrpc::as_client( std::string ip, int port )
{
    m_role = RPC_CLIENT;
    m_socket = new zmq::socket_t(m_context, ZMQ_REQ); //创建一个套接字 参数为上下文和套接字类型	 //ZMQ_REQ 用于请求-应答模式
    ostringstream os;//创建一个字符串流
    os << "tcp://" << ip << ":" << port;
    m_socket->connect (os.str()); //连接到指定的地址
}

void buttonrpc::as_server( int port )
{
	m_role = RPC_SERVER; //设置角色为服务器
	m_socket = new zmq::socket_t(m_context, ZMQ_REP); //创建一个套接字 参数为上下文和套接字类型	 //ZMQ_REP 用于请求-应答模式
	ostringstream os;
	os << "tcp://*:" << port;
	m_socket->bind (os.str()); //绑定到指定的地址
}

void buttonrpc::send( zmq::message_t& data )
{
	m_socket->send(data);  //发送数据
}

void buttonrpc::recv( zmq::message_t& data )
{
	m_socket->recv(&data); //接收数据
}

inline void buttonrpc::set_timeout(uint32_t ms)
{
	// only client can set
	// if (m_role == RPC_CLIENT) {
	// 	m_socket->setsockopt(ZMQ_RCVTIMEO, ms); //设置接收超时时间
	// }
}

void buttonrpc::run()
{
    if (m_role != RPC_SERVER) { //如果不是服务器
		return;
	}
	while (1){
		zmq::message_t data;  //创建一个消息
		recv(data); //接收数据 没消息就阻塞
		StreamBuffer iodev((char*)data.data(), data.size());//创建一个流缓冲区
		Serializer ds(iodev); //创建一个序列化器

		std::string funname;
		ds >> funname; //读取函数名
		Serializer* r = call_(funname, ds.current(), ds.size()- funname.size()); //调用函数

		zmq::message_t retmsg (r->size()); //创建一个消息
		memcpy (retmsg.data (), r->data(), r->size()); //拷贝数据
		send(retmsg); //发送数据
		delete r;
	}

}

// 处理函数相关
Serializer* buttonrpc::call_(std::string name, const char* data, int len)
{
    Serializer* ds = new Serializer(); //创建一个序列化器
    if (m_handlers.find(name) == m_handlers.end()) { //如果没有找到函数
		(*ds) << value_t<int>::code_type(RPC_ERR_FUNCTIION_NOT_BIND); //设置错误码
		(*ds) << value_t<int>::msg_type("function not bind: " + name); //设置错误信息
		return ds;
	}

    auto fun = m_handlers[name]; //获取函数
    fun(ds, data, len);  //调用函数
    ds->reset(); //重置序列号容器
    return ds;
}

template<typename F>
void buttonrpc::bind( std::string name, F func ) //普通函数
{
	m_handlers[name] = std::bind(&buttonrpc::callproxy<F>, this, func, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

template<typename F, typename S>
inline void buttonrpc::bind(std::string name, F func, S* s) //类函数
{
	m_handlers[name] = std::bind(&buttonrpc::callproxy<F, S>, this, func, s, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

template<typename F>
void buttonrpc::callproxy( F fun, Serializer* pr, const char* data, int len )//代理普通函数
{
	callproxy_(fun, pr, data, len);
}

template<typename F, typename S>
inline void buttonrpc::callproxy(F fun, S * s, Serializer * pr, const char * data, int len)//代理类函数
{
	callproxy_(fun, s, pr, data, len);
}

#pragma region 区分返回值
// help call return value type is void function ,c++11的模板参数类型约束
template<typename R, typename F>
typename std::enable_if<std::is_same<R, void>::value, typename type_xx<R>::type >::type call_helper(F f) {
	f();
	return 0;
}
template<typename R, typename F>
typename std::enable_if<!std::is_same<R, void>::value, typename type_xx<R>::type >::type call_helper(F f) {
	return f();
}
#pragma endregion



template<typename R>
void buttonrpc::callproxy_(std::function<R()> func, Serializer* pr, const char* data, int len)
{
    /*
    typename关键字用于指定一个依赖类型,依赖类型是指在模板参数中定义的类型，其具体类型直到模板实例化时才能确定。
    ype_xx<R>::type是一个依赖类型，因为它依赖于模板参数R。在这种情况下，你需要使用typename关键字来告诉编译器type_xx<R>::type是一个类型。
    如果不使用typename，编译器可能会将type_xx<R>::type解析为一个静态成员
    */
	typename type_xx<R>::type r = call_helper<R>(std::bind(func));

	value_t<R> val;
	val.set_code(RPC_ERR_SUCCESS);
	val.set_val(r);
	(*pr) << val;
}


template<typename R, typename P1>
void buttonrpc::callproxy_(std::function<R(P1)> func, Serializer* pr, const char* data, int len)
{
	Serializer ds(StreamBuffer(data, len));
	P1 p1;
	ds >> p1;
	typename type_xx<R>::type r = call_helper<R>(std::bind(func, p1));

	value_t<R> val;
	val.set_code(RPC_ERR_SUCCESS);
	val.set_val(r);
	(*pr) << val;
}


template<typename R, typename P1, typename P2>
void buttonrpc::callproxy_(std::function<R(P1, P2)> func, Serializer* pr, const char* data, int len )
{
	Serializer ds(StreamBuffer(data, len));
	P1 p1; P2 p2;
	ds >> p1 >> p2;
	typename type_xx<R>::type r = call_helper<R>(std::bind(func, p1, p2));
	
	value_t<R> val;
	val.set_code(RPC_ERR_SUCCESS);
	val.set_val(r);
	(*pr) << val;
}

template<typename R, typename P1, typename P2, typename P3>
void buttonrpc::callproxy_(std::function<R(P1, P2, P3)> func, Serializer* pr, const char* data, int len)
{
	Serializer ds(StreamBuffer(data, len));
	P1 p1; P2 p2; P3 p3;
	ds >> p1 >> p2 >> p3;
	typename type_xx<R>::type r = call_helper<R>(std::bind(func, p1, p2, p3));
	value_t<R> val;
	val.set_code(RPC_ERR_SUCCESS);
	val.set_val(r);
	(*pr) << val;
}

template<typename R, typename P1, typename P2, typename P3, typename P4>
void buttonrpc::callproxy_(std::function<R(P1, P2, P3, P4)> func, Serializer* pr, const char* data, int len)
{
	Serializer ds(StreamBuffer(data, len));
	P1 p1; P2 p2; P3 p3; P4 p4;
	ds >> p1 >> p2 >> p3 >> p4;
	typename type_xx<R>::type r = call_helper<R>(std::bind(func, p1, p2, p3, p4));
	value_t<R> val;
	val.set_code(RPC_ERR_SUCCESS);
	val.set_val(r);
	(*pr) << val;
}

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
void buttonrpc::callproxy_(std::function<R(P1, P2, P3, P4, P5)> func, Serializer* pr, const char* data, int len)
{
	Serializer ds(StreamBuffer(data, len));
	P1 p1; P2 p2; P3 p3; P4 p4; P5 p5;
	ds >> p1 >> p2 >> p3 >> p4 >> p5;
	typename type_xx<R>::type r = call_helper<R>(std::bind(func, p1, p2, p3, p4, p5));
	value_t<R> val;
	val.set_code(RPC_ERR_SUCCESS);
	val.set_val(r);
	(*pr) << val;
}



template<typename R>
inline buttonrpc::value_t<R> buttonrpc::net_call(Serializer& ds)
{
	zmq::message_t request(ds.size() + 1);
	memcpy(request.data(), ds.data(), ds.size());
	if (m_error_code != RPC_ERR_RECV_TIMEOUT) {
		send(request);
	}
	zmq::message_t reply;
	recv(reply);
	value_t<R> val;
	if (reply.size() == 0) {
		// timeout
		m_error_code = RPC_ERR_RECV_TIMEOUT;
		val.set_code(RPC_ERR_RECV_TIMEOUT);
		val.set_msg("recv timeout");
		return val;
	}
	m_error_code = RPC_ERR_SUCCESS;
	ds.clear();
	ds.write_raw_data((char*)reply.data(), reply.size());
	ds.reset();

	ds >> val;
	return val;
}


template<typename R>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name)
{
	Serializer ds;
	ds << name;
	return net_call<R>(ds);
}

template<typename R, typename P1>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name, P1 p1)
{
	Serializer ds;
	ds << name << p1;
	return net_call<R>(ds);
}

template<typename R, typename P1, typename P2>
inline buttonrpc::value_t<R> buttonrpc::call( std::string name, P1 p1, P2 p2 )
{
	Serializer ds;
	ds << name << p1 << p2;
	return net_call<R>(ds);
}

template<typename R, typename P1, typename P2, typename P3>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name, P1 p1, P2 p2, P3 p3)
{
	Serializer ds;
	ds << name << p1 << p2 << p3;
	return net_call<R>(ds);
}

template<typename R, typename P1, typename P2, typename P3, typename P4>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name, P1 p1, P2 p2, P3 p3, P4 p4)
{
	Serializer ds;
	ds << name << p1 << p2 << p3 << p4;
	return net_call<R>(ds);
}

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
	Serializer ds;
	ds << name << p1 << p2 << p3 << p4 << p5;
	return net_call<R>(ds);
}






