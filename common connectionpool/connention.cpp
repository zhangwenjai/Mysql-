#include<mysql.h>
#include<string>
#include"public.h"
#include"connection.h"
#include<iostream>

using namespace std;

Connection::Connection()
{
	_conn = mysql_init(nullptr);
	if (_conn == nullptr)
	{
		cout << "init fail" << endl;
	}
}

Connection::~Connection()
{
	if (_conn != nullptr)
		mysql_close(_conn);
}
bool Connection::connection(
	string ip,
	unsigned short port,
	string user,
	string password,
	string dbname
)
{
 	MYSQL* p = mysql_real_connect(_conn, ip.c_str(), user.c_str(),
		password.c_str(), dbname.c_str(), port, nullptr, 0);
	if (p == nullptr)
	{
		cout << "connection fail!" << endl;
		cout << mysql_error(_conn) << endl;
		return false;
	}
	return true;

}
bool Connection::update(string sql)
{
	if (mysql_query(_conn, sql.c_str()))
	{
		LOG("¸üÐÂÊ§°Ü:" + sql);
		cout << mysql_error(_conn) << endl;
		return false;
	}
	return true;

}
MYSQL_RES* Connection::query(string sql)
{
	if (mysql_query(_conn, sql.c_str()))
	{
		LOG("²éÑ¯Ê§°Ü:" + sql);
		return nullptr;
	}
	return mysql_use_result(_conn);
}
