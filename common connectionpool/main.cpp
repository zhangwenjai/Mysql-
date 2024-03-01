#include<iostream>
#include"connection.h"
#include"common c_p.h"
using namespace std;

int main() {
	


	clock_t begin = clock();
	ConnectionPool* cp = ConnectionPool::getconnectionpool();

	thread t1([]() {
		ConnectionPool* cp = ConnectionPool::getconnectionpool();
		for (int i = 0; i < 1250; i++)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getconnection();
			sp->update(sql);
		}
		});
	thread t2([]() {
		ConnectionPool* cp = ConnectionPool::getconnectionpool();
		for (int i = 0; i < 1250; i++)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getconnection();
			sp->update(sql);
		}
		});
	thread t3([]() {
		ConnectionPool* cp = ConnectionPool::getconnectionpool();
		for (int i = 0; i < 1250; i++)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getconnection();
			sp->update(sql);
		}
		});
	thread t4([]() {
		ConnectionPool* cp = ConnectionPool::getconnectionpool();
		for (int i = 0; i < 1250; i++)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getconnection();
			sp->update(sql);
		}
		});

	t1.join();
	t2.join();
	t3.join();
	t4.join();
#if 0
	for (int i = 0; i < 1000; i++)
	{  
		/*Connection conn; 
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('zhang san',20,'male');");
		conn.connection("127.0.0.1", 3306 , "root", "0508", "chat");
		conn.update(sql);*/

		shared_ptr<Connection> sp = cp->getconnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
		sp->update(sql);
	}
#endif

	clock_t end = clock();
	cout << (end - begin) << "ms" << endl;
	return 0;
}