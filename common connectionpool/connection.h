#pragma once
/*实现mysql增删改查数据库的操作*/
#include<string>
#include<mysql.h>
#include<ctime>
using namespace std;
class Connection
{
public:
	Connection();
	~Connection();
	bool update(string sql);
	bool connection(string ip,
		unsigned short port,
		string user,
		string password,
		string dbname);
	MYSQL_RES* query(string sql);
	void refreshalivetime()
	{
		_alivetime = clock();
	}
	clock_t getalivetime() const
	{
		return clock() - _alivetime;
	}
private:
	MYSQL* _conn;
	clock_t _alivetime;
};