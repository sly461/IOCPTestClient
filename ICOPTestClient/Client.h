/* 
压力测试客户端 客户端使用的是最简单的多线程阻塞式Socket
*/

#pragma once
#include <iostream>
#include <string>
#include <winsock2.h>
#include <string.h>
using namespace std;
#pragma comment(lib,"ws2_32.lib")



#define MAX_BUFLEN 8192                       //缓冲区长度
#define DEFAULT_PORT 9990                     //默认端口
#define DEFAULT_IP "127.0.0.1"                //默认IP地址
#define DEFAULT_CLIENTS 100                   //默认线程数量
#define DEFAULT_MESSAGE "hello!"              //默认信息

#define RELEASE_HANDLE(x) {if((x)!=NULL&&(x)!=INVALID_HANDLE_VALUE){CloseHandle(x); (x)=NULL;}}
#define RELEASE(x) {if((x)!=NULL){delete (x); (x)=NULL;}}

class CClient;


//工作者线程  接收发送信息的工作者线程的参数
typedef struct _WORKER_THREAD_PARAM
{
	CClient *m_client;                        //方便调用类的函数
	SOCKET m_socket;                          //socket
	int m_nThreadNo;                          //线程编号
	char m_buffer[MAX_BUFLEN];                //缓冲区

}WORKER_THREAD_PARAM,*PWORKER_THREAD_PARAM;

//负责socket连接的线程的参数
typedef struct _CONNECTION_THREAD_PARAM
{
	CClient *m_client;                        //方便调用类的函数
}CONNECTION_THREAD_PARAM,* PCONNECTION_THREAD_PARAM;




class CClient
{
public:
	CClient();
	~CClient();
	void SetIP(const string& ip);             //设置ip地址和端口
	void SetnPort(const int &port);
	void SetClients(const int &num);          //设置客户端数量
	void setMessage(const string &str);       //设置发送的消息

	bool Start();                             //设置客户端开始与停止      
	void Stop(); 

private:
	int m_numClients;                         //客户端数量
	int m_nPort;                              //端口
	string m_serverAddr;                      //服务器地址
	string m_message;                         //发送信息
	
	HANDLE m_hConnectionThread;               //连接的线程
	HANDLE *m_phWorkerThread;                 //工作者线程句柄
	PWORKER_THREAD_PARAM m_pWorkerThreadParam;//工作者线程参数
	HANDLE m_hQuitEvent;                      //退出事件


	bool LoadSocketLib();                     //加载socket库
	void UnloadSocketLib() { WSACleanup(); }  //卸载socket库
	bool EstablishConnect();                  //将所有客户端与服务器建立连接
	bool ConnectToServer(SOCKET *s);                   
	void CleanUp();                           //清除资源
											  //连接线程与工作者线程
	static DWORD WINAPI ConnectionThread(LPVOID lpParam);
	static DWORD WINAPI WorkThread(LPVOID lpParam);
};

