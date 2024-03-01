#pragma once
/*实现连接池功能模块*/
#include<string>
#include<queue>
#include"connection.h"
#include<mutex>
#include<condition_variable>
#include<atomic>
#include<memory>
#include<functional>
using namespace std;
class ConnectionPool
{
public:
	static ConnectionPool* getconnectionpool();
	shared_ptr<Connection> getconnection();
private:
	ConnectionPool();

	bool loadconfigfile();
	//运行在独立线程，抓门负责生产新连接
	void produceconnectiontask();

	void scannerconnectiontask();

	string _ip;
	unsigned short _port;
	string _username;
	string _password;
	string _dbname;
	int _initsize;
	int _maxsize;
	int _maxidletime;
	int _maxconnectiontimeout;
	
	queue<Connection*> _connectionque;
	mutex _queuemutex;
	atomic_int _connectioncnt;
	condition_variable cv;
};