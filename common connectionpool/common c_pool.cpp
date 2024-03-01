#include"common c_p.h"
#include<string>
#include"public.h"
#include<iostream>
#include<thread>
#include<functional>
using namespace std;
ConnectionPool* ConnectionPool::getconnectionpool()
{
	static ConnectionPool pool;
	return &pool;
}

bool ConnectionPool::loadconfigfile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exist!");
		return false;
	}

	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1)
		{
			continue;
		}
		int endidx = str.find('\n', idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);

		if (key == "ip")
		{
			_ip = value;
		}
		else if (key == "port") {
			_port = atoi(value.c_str());
		}
		else if (key == "username") {
			_username = value;
		}
		else if (key == "password") {
			_password = value;
		}
		else if (key == "dbname")
		{
			_dbname = value;
		}
		else if (key == "initsize") {
			_initsize = atoi(value.c_str());
		}
		else if (key == "maxsize") {
			_maxsize = atoi(value.c_str());
		}
		else if (key == "maxidletime") {
			_maxidletime = atoi(value.c_str());
		}
		else if (key == "maxconnectiontimeout") {
			_maxconnectiontimeout = atoi(value.c_str());
		}
	}
}

ConnectionPool::ConnectionPool()
{
	if (!loadconfigfile())
	{
		return;
	}

	for (int i = 0;i < _initsize; i++)
	{
		Connection* p = new Connection();
		p->connection(_ip, _port, _username, _password, _dbname);
		p->refreshalivetime();
		_connectionque.push(p);
		_connectioncnt++;
	}

	//启动线程生成生产者
	thread produce(std::bind(&ConnectionPool::produceconnectiontask,this));
	produce.detach();

	thread scanner(std::bind(&ConnectionPool::scannerconnectiontask, this));
	scanner.detach();
}

void ConnectionPool::produceconnectiontask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queuemutex);
		while (!_connectionque.empty())
		{
			cv.wait(lock);
		}

		if (_connectioncnt < _maxsize)
		{
			Connection* p = new Connection();
			p->connection(_ip, _port, _username, _password, _dbname);
			p->refreshalivetime();
			_connectionque.push(p);
			_connectioncnt++;
		}

		cv.notify_all();
	}
}

shared_ptr<Connection> ConnectionPool:: getconnection()
{
	unique_lock<mutex> lock(_queuemutex);
	while (_connectionque.empty())
	{
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_maxconnectiontimeout))) 
		{
			if (_connectionque.empty())
			{
				LOG("获取空闲连接超时。。。获取连接失败！");
				return nullptr;
			}
		}
	}

	shared_ptr<Connection> sp(_connectionque.front(),
		[&](Connection* pcon) {
			unique_lock<mutex> lock(_queuemutex);
			pcon->refreshalivetime();
			_connectionque.push(pcon);
		});
	_connectionque.pop();

	cv.notify_all();
	return sp;
}

void ConnectionPool::scannerconnectiontask()
{
	for (;;)
	{
		this_thread::sleep_for(chrono::seconds(_maxidletime));

		unique_lock<mutex> lock(_queuemutex);
		while (_connectioncnt > _initsize)
		{
			Connection* p = _connectionque.front();
			if (p->getalivetime() >= (_maxidletime*1000))
			{
				_connectionque.pop();
				_connectioncnt--;
				delete p;
			}
			else
			{
				break;
			}
		}
	}
}