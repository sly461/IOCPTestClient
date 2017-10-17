#include "stdafx.h"
#include "Client.h"


CClient::CClient()
	:m_nPort(DEFAULT_PORT),
	 m_serverAddr(DEFAULT_IP),
	 m_numClients(DEFAULT_CLIENTS),
	 m_hConnectionThread(NULL),
	 m_phWorkerThread(nullptr),
	 m_phWorkerThreadParam(nullptr)
{
}


CClient::~CClient()
{
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

bool CClient::Start()
{
	return true;
}

void CClient::Stop()
{
}

bool CClient::LoadSocketLib()
{
	WSADATA wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printf("≥ı ºªØwinsocket2.2 ß∞‹£°\n");
		return false;
	}
	return true;
}

