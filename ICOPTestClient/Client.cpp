#include "stdafx.h"
#include "Client.h"


CClient::CClient()
	:m_nPort(DEFAULT_PORT),
	 m_serverAddr(DEFAULT_IP),
	 m_numClients(DEFAULT_CLIENTS),
	 m_hConnectionThread(NULL),
	 m_phWorkerThread(nullptr),
	 m_pWorkerThreadParam(nullptr),
	 m_message(DEFAULT_MESSAGE)
{
	LoadSocketLib();
}


CClient::~CClient()
{
	Stop();
}

void CClient::SetIP(const string & ip)
{
	m_serverAddr = ip;
}

void CClient::SetnPort(const int & port)
{
	m_nPort = port;
}

void CClient::SetClients(const int & num)
{
	m_numClients = num;
}



void CClient::setMessage(const string & str)
{
	m_message = str;
}

bool CClient::Start()
{
	//创建退出事件
	m_hQuitEvent = CreateEvent(NULL, true, false, NULL);
	//创建连接线程
	DWORD nThreadNo;
	PCONNECTION_THREAD_PARAM pConnectionThreadParam = new CONNECTION_THREAD_PARAM;
	pConnectionThreadParam->m_client = this;
	m_hConnectionThread = CreateThread(NULL, 0, ConnectionThread,
		(LPVOID)pConnectionThreadParam, 0, &nThreadNo);

	return true;
}

void CClient::Stop()
{
	if (m_hQuitEvent == NULL)return;
	//激活退出事件
	SetEvent(m_hQuitEvent);
	//等待连接线程退出
	WaitForSingleObject(m_hConnectionThread, INFINITE);
	//关闭套接字
	for (int i = 0; i < m_numClients; i++)
	{
		closesocket(m_pWorkerThreadParam[i].m_socket);
	}
	//等待工作者线程退出
	WaitForMultipleObjects(m_numClients,m_phWorkerThread,true,INFINITE);
	//清除资源
	CleanUp();
	printf("测试停止\n");
}

bool CClient::LoadSocketLib()
{
	WSADATA wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printf("初始化winsocket2.2失败！\n");
		return false;
	}
	return true;
}

bool CClient::EstablishConnect()
{
	//线程号
	DWORD nThreadNo;
	
	m_phWorkerThread = new HANDLE[m_numClients];
	m_pWorkerThreadParam = new WORKER_THREAD_PARAM[m_numClients];

	for (int i = 0; i < m_numClients; i++)
	{
		//停止客户端连接
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hQuitEvent, 0))
		{
			printf("接收到用户停止命令！\n");
			return true;
		}
		if (false == ConnectToServer(&m_pWorkerThreadParam[i].m_socket))
		{
			printf("连接服务器失败！\n");
			CleanUp();
			return false;
		}
		nThreadNo = i + 1;
		m_pWorkerThreadParam[i].m_client = this;
		m_pWorkerThreadParam[i].m_nThreadNo = nThreadNo;

		sprintf(m_pWorkerThreadParam[i].m_buffer, "%d号线程 发送数据 %s", i + 1, m_message.c_str());

		Sleep(10);


		m_phWorkerThread[i] = CreateThread(NULL,0,WorkThread,
			(LPVOID)(&m_pWorkerThreadParam[i]),0,&nThreadNo);

	}
	return true;
}

bool CClient::ConnectToServer(SOCKET * s)
{
	//服务器地址
	sockaddr_in serverAddr;
	serverAddr.sin_addr.S_un.S_addr = inet_addr(m_serverAddr.c_str());
	serverAddr.sin_port = htons(m_nPort);
	serverAddr.sin_family = AF_INET;

	//建立socket
	*s = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == *s)
	{
		printf("错误：初始化Socket失败，错误信息：%d\n", WSAGetLastError());
		return false;
	}


	//连接服务器
	if (SOCKET_ERROR == connect(*s, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("错误：连接至服务器失败！\n");
		closesocket(*s);
		return false;
	}

	return true;
}

void CClient::CleanUp()
{
	if (m_hQuitEvent == NULL)return;
	delete[] m_phWorkerThread;
	delete[] m_pWorkerThreadParam;
	RELEASE_HANDLE(m_hQuitEvent);
	RELEASE_HANDLE(m_hConnectionThread);
}

DWORD WINAPI CClient::ConnectionThread(LPVOID lpParam)
{
	PCONNECTION_THREAD_PARAM param = (PCONNECTION_THREAD_PARAM)lpParam;
	CClient *client = (CClient *)param->m_client;
	printf("连接服务器中......\n");
	client->EstablishConnect();
	printf("所有服务器已经建立连接！\n");
	RELEASE(lpParam);
	return 0;
}

DWORD WINAPI CClient::WorkThread(LPVOID lpParam)
{
	PWORKER_THREAD_PARAM param = (PWORKER_THREAD_PARAM)lpParam;
	CClient* pClient = (CClient*)param->m_client;

	char szSent[MAX_BUFLEN];
	char szRecv[MAX_BUFLEN];

	int nBytesSent = 0;
	int nBytesRecv = 0;

	int index = 1;


	while (1)
	{
		sprintf(param->m_buffer, "%d号线程 发送数据 %s", param->m_nThreadNo, pClient->m_message.c_str());
		memset(szSent, 0, sizeof(szSent));
		sprintf(szSent, ("第%d条信息：%s\n"), index, param->m_buffer);
		nBytesSent = send(param->m_socket, szSent, strlen(szSent), 0);
		if (SOCKET_ERROR == nBytesSent)
		{
			printf("错误：发送信息失败，错误代码：%ld\n", WSAGetLastError());
			return 1;
		}
		index++;
		Sleep(1000);

		memset(szRecv, 0, sizeof(szRecv));
		nBytesRecv = recv(param->m_socket, szRecv, sizeof(szRecv), 0);
		if (nBytesRecv == 0)
		{
			printf("服务器已关闭！\n");
			return 1;
		}
		else if (SOCKET_ERROR==nBytesRecv)
		{
			printf("错误：发送信息失败，错误代码：%ld\n", WSAGetLastError());
			return 1;
		}
		printf("回传信息：%s\n", szRecv);
		Sleep(1000);
	}

	
	/*sprintf(param->m_buffer, "%d号线程 发送数据 %s", param->m_nThreadNo, pClient->m_message.c_str());
	memset(szSent, 0, sizeof(szSent));
	sprintf(szSent, ("第%d条信息：%s\n"), index, param->m_buffer);
	nBytesSent = send(param->m_socket, szSent, strlen(szSent), 0);
	if (SOCKET_ERROR == nBytesSent)
	{
		printf("错误：发送信息失败，错误代码：%ld\n", WSAGetLastError());
		return 1;
	}
	index++;
	Sleep(3000);

	sprintf(param->m_buffer, "%d号线程 发送数据 %s", param->m_nThreadNo, pClient->m_message.c_str());
	memset(szSent, 0, sizeof(szSent));
	sprintf(szSent, ("第%d条信息：%s\n"), index, param->m_buffer);
	nBytesSent = send(param->m_socket, szSent, strlen(szSent), 0);
	if (SOCKET_ERROR == nBytesSent)
	{
		printf("错误：发送信息失败，错误代码：%ld\n", WSAGetLastError());
		return 1;
	}
	index++;*/
	return 0;
}

