# MySQLConnectionPool-连接池
# 关键技术点
MySQL数据库编程、单例模式、C++11多线程编程、基于CAS的原子整形、智能指针shared_ptr、函数绑定器技术、lambda表达式、生产者-消费者模型。
# 项目背景
为了提高MySQL数据库的访问瓶颈，除了在服务器端增加缓存服务器缓存常用的数据之外（例如redis），还可以增加连接池，来提高MySQL Server的访问效率，**在高并发情况下，大量的TCP三次握手、MySQL Server连接认证、MySQL Server关闭连接回收资源和TCP四次挥手所耗费的性能时间也是很明显的**，增加连接池就是为了减少这一部分的性能损耗。 

在市场上比较流行的连接池包括阿里的druid，c3p0以及apache dbcp连接池，它们对于短时间内大量的数据库增删改查操作性能的提升是很明显的，但是它们有一个共同点就是，全部由Java实现的。

那么本项目就是为了在C/C++项目中，提供MySQL Server的访问效率，实现基于C++代码的数据库连接池模块。
# 连接池功能特点介绍
连接池一般包含了数据库连接所用的ip地址、port端口号、用户名和密码以及其它的性能参数，例如初始连接量，最大连接量，最大空闲时间、连接超时时间等，该项目是基于C++语言实现的连接池，主要也是实现以上几个所有连接池都支持的通用基础功能。

**初始连接量（initSize）：**表示连接池事先会和MySQL Server创建initSize个数的connection连接，当应用发起MySQL访问时，不用再创建和MySQL Server新的连接，直接从连接池中获取一个可用的连接就可以，使用完成后，并不去释放connection，而是把当前connection再归还到连接池当中。

**最大连接量（maxSize）：**当并发访问MySQL Server的请求增多时，初始连接量已经不够使用了，此时会根据新的请求数量去创建更多的连接给应用去使用，但是新创建的连接数量上限是maxSize，不能无限制的创建连接，因为每一个连接都会占用一个socket资源，一般连接池和服务器程序是部署在一台主机上的，如果连接池占用过多的socket资源，那么服务器就不能接收太多的客户端请求了。当这些连接使用完成后，再次归还到连接池当中来维护。

**最大空闲时间（maxIdleTime）：**当访问MySQL的并发请求多了以后，连接池里面的连接数量会动态增加，上限是maxSize个，当这些连接用完再次归还到连接池当中。如果在指定的maxIdleTime里面，这些新增加的连接都没有被再次使用过，那么新增加的这些连接资源就要被回收掉，只需要保持初始连接量initSize个连接就可以了。

**连接超时时间（connectionTimeout）：**当MySQL的并发请求量过大，连接池中的连接数量已经到达maxSize了，而此时没有空闲的连接可供使用，那么此时应用从连接池获取连接无法成功，它通过阻塞的方式获取连接的时间如果超过connectionTimeout时间，那么获取连接失败，无法访问数据库。该项目主要实现上述的连接池四大功能，其余连接池更多的扩展功能，可以自行实现。
# 功能实现设计
common c\_pool.cpp和common c\_p.h：连接池代码实现

connection.cpp和connection.h：数据库操作代码、增删改查代码实现

**连接池主要包括以下功能特点：**

1.连接池只需要一个实例，所以ConnectionPool以单例模式进行设计

2.从ConnectionPool中可以获取和MySQL的连接Connection

3.空闲连接Connection全部维护在一个线程安全的Connection队列中，使用线程互斥锁保证队列的线程安全

4.如果Connection队列为空，还需要再获取连接，此时需要动态创建连接，上限数量是maxSize

5.队列中空闲连接时间超过maxIdleTime的就要被释放掉，只保留初始的initSize个连接就可以了，这个功能点肯定需要放在独立的线程中去做

6.如果Connection队列为空，而此时连接的数量已达上限maxSize，那么等待connectionTimeout时间如果
还获取不到空闲的连接，那么获取连接失败，此处从Connection队列获取空闲连接，可以使用带超时时间的mutex互斥锁来实现连接超时时间

7.用户获取的连接用shared_ptr智能指针来管理，用lambda表达式定制连接释放的功能（不真正释放连接，而是把连接归还到连接池中）

8.连接的生产和连接的消费采用生产者-消费者线程模型来设计，使用了线程间的同步通信机制条件变量和互斥锁

# 开发平台以及对应版本

有关MySQL数据库编程、多线程编程、线程互斥和同步通信操作、智能指针、设计模式、容器等等这些技术在C++语言层面都可以直接实现，因此该项目选择直接在windows平台上进行开发，当然放在Linux平台下用g++也可以直接编译运行。

IDE：vs2019；  MySQL数据库：5.7.17.0  

# 压力测试

验证数据的插入操作所花费的时间，第一次测试使用普通的数据库访问操作，第二次测试使用带连接池的数据库访问操作，对比两次操作同样数据量所花费的时间，性能压力测试结果如下：

+-----------+--------------------------+--------------------------+--+--+
| 数据量\条 | 未使用连接池花费时间\ms  | 使用连接池花费时间\ms    |  |  |
+-----------+--------------------------+--------------------------+--+--+
| 1000      | 单线程:3782 四线程:994   | 单线程:2158 四线程:805   |  |  |
+-----------+--------------------------+--------------------------+--+--+
| 5000      | 单线程:20516 四线程:4230 | 单线程:10615 四线程:3915 |  |  |
+-----------+--------------------------+--------------------------+--+--+
| 10000     | 单线程:39564 四线程:7815 | 单线程:15926 四线程:7061  |  |  |
+-----------+--------------------------+--------------------------+--+--+

# 可能会遇到的问题（mysql\_error(MYSQL\*)
**第一：mysql sever has gone awawy**

此报错主要原因可能有两点：

1.发送的sql语句太长，超过max\_allowed\_packet的大小；

解决方法有两种(数值按需自行修改)，修改配置文件my.cnf（my.ini），在配置文件中添加
>max\_allowed\_packet = 100m

或者在终端修改对应数值
>set global max\_allowed\_packet=1024\*1024\*16;

2.由于挂起时间过长被server强行关闭或被人为kill，解决方法仍然有两种

在配置文件中添加
>wait_timeout=2880000

>interactive_timeout = 2880000

或者在终端修改对应数值
>set global wait_timeout=28800;

>set global interactive_timeout=28800;

>set global net_read_timeout=28800;

>set global net_write_timeout=28800;

完成以上操作之后重启mysql服务即可。
参考资料：

>https://blog.csdn.net/weixin_42365141/article/details/114329751?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522170917012516800226580330%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fall.%2522%257D&request_id=170917012516800226580330&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_ecpm_v1~rank_v31_ecpm-1-114329751-null-null.142^v99^pc_search_result_base9&utm_term=mysql%20sever%20has%20gone%20awawy%20windows&spm=1018.2226.3001.4187

>https://blog.csdn.net/weixin_33132819/article/details/113226302?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522170917012516800226580330%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fall.%2522%257D&request_id=170917012516800226580330&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_ecpm_v1~rank_v31_ecpm-6-113226302-null-null.142^v99^pc_search_result_base9&utm_term=mysql%20sever%20has%20gone%20awawy%20windows&spm=1018.2226.3001.4187

**第二：连接数据库失败**

此问题与问题三基本同源，刚开始解决时发现在浏览器查到的ip与在ipconfig\all中查询到的不一致，浏览器查询到的为公网ip，本地计算机查询到的是局域网ip，但使用两者都无法连接mysql数据库，最后使用本地127.0.0.1地址连接成功。

**第三：Can't connect to MySQL server on 'localhost' (10048)**

这是使用mysql给予的api函数连接数据库时遇到的问题，与上述问题不同的是已经设置ip地址为127.0.0.1还是报错误码10048，大概原因可能是两点，第一，mysql短时间内瞬间增加太多连接数,而tcp连接在短时间内又不释放, 这样就导致不能有新的连接产生,所以提示连接不到mysql数据库. 第二，5000以下端口用完了,而5000以上端口禁止连接。针对以上两种问题来源，对应两种解决方式：

第一、修改tcp连接释放时间,在注册表里修改: tcp连接释放时间默认240,我们可以修改的小一些,一般在30-60之间就可以了. 找到 HKEY\_LOCAL\_MACHINE\SYSTEM\CurrentControlSet\ Services\TCPIP\Parameters 注册表子键 并创建名为 TcpTimedWaitDelay 的新 REG\_DWORD 值 设置此值为十进制 30, 十六进制为 0×0000001e

第二、 修改允许连接最大端口号MaxUserPort设置(增加最大值端口连接): 找到 HKEY\_LOCAL\_MACHINE\SYSTEM\CurrentControlSet\ Services\TCPIP\Parameters 注册表子键 并创建名为 MaxUserPort 的新 REG\_DWORD 值 设置此值为十进制最低 32768

第一、第二、执行完毕后重新启动服务器。
参考资料：

>https://blog.csdn.net/weixin_33207144/article/details/113188112?ops_request_misc=&request_id=&biz_id=102&utm_term=mysql_real_connect%E5%A4%B1%E8%B4%A510048&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-1-113188112.142^v99^pc_search_result_base9&spm=1018.2226.3001.4187
