// ICOPTestClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Client.h"

int main()
{
	CClient client;
	string serverAddr;
	int port,numClients;

	cout << "服务器地址：";
	cin >> serverAddr;
	cout << "端口：";
	cin >> port;
	cout << "客户端个数：";
	cin >> numClients;
	client.SetIP(serverAddr);
	client.SetnPort(port);
	client.SetClients(numClients);
	client.Start();
	while (1);
    return 0;
}

