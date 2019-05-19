// ProxyServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "ProxyServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// The one and only application object

CWinApp theApp;
int create_tcp_socket();
char* get_ip(char* host);
char* build_get_query(char* host, char* page);
void usage();
string getHostFromRequest(string g);

#define HOST "www.fit.hcmus.edu.vn"
#define PAGE "/"
#define PORT 80
#define USERAGENT "HTMLGET 1.0"	
#define CLIENT_PORT 8888
using namespace std;
//Ref: http://stackoverflow.com/questions/19715144/how-to-convert-char-to-lpcwstr
wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}
/* Ref code:http://coding.debuntu.org/c-linux-socket-programming-tcp-simple-http-client */

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: code your application's behavior here.
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: code your application's behavior here.
			/* Ref code:http://coding.debuntu.org/c-linux-socket-programming-tcp-simple-http-client */
			CSocket socket_web_client;
			CSocket socket_proxy_server;
			int tmpres;
			//char* ip;
			string ip;
			//char* get;
			string get;
			char buf[BUFSIZ + 1];
			string host;
			string page;
			string response;
			string GETrequest;

			//Buoc 1: Khoi dong socket trong Window
			AfxSocketInit(NULL);

			//Buoc 2: Tao socket
			socket_web_client.Create();
			socket_proxy_server.Create(CLIENT_PORT, SOCK_STREAM, NULL);
			CSocket connector;
			if (socket_proxy_server.Accept(connector))
			{
				do
				{
					//Cho ket noi tu Proxy client
					//Nhan ket noi tu proxy client
					memset(buf, 0, sizeof(buf));
					/*int htmlstart = 0;
					char * htmlcontent;*/
					while ((tmpres = socket_proxy_server.Receive(buf, BUFSIZ, 0)) > 0)
					{
						if (buf) {
							GETrequest.append(buf);
						}
						memset(buf, 0, tmpres);
					}
					if (tmpres < 0)
					{
						perror("Error receiving data");
					}
					//Lay host tu request nhan duoc
					host = getHostFromRequest(GETrequest);

					//Buoc 3: Ket noi toi web Server
					ip = get_ip((char*)host.c_str());
					//fprintf(stderr, "IP is %s\n", ip);
					if (socket_web_client.Connect(convertCharArrayToLPCWSTR(ip.c_str()), PORT) < 0)
					{
						perror("Could not connect");
						//exit(1);
					}
					get = GETrequest;
					//strcpy(get, GETrequest.c_str());
					//Send the query to the server
					//int sent = 0;
					tmpres = socket_web_client.Send(get.c_str(), get.length(), 0);
					if (tmpres == -1) {
						perror("Can't send query");
						//exit(1);
						}
					//now it is time to receive the page
					memset(buf, 0, sizeof(buf));
					while ((tmpres = socket_web_client.Receive(buf, BUFSIZ, 0)) > 0)
					{
						if (buf) {
							response.append((const char*)buf);
						}
						memset(buf, 0, tmpres);
					}
					if (tmpres < 0)
					{
						perror("Error receiving data");
					}
					//sent = 0;
					//caching time
		
					//send respond to proxy client

						tmpres = socket_proxy_server.Send(response.c_str(), response.length() , 0);
						if (tmpres == -1) {
							perror("Can't send respond");
						}
	

				} while (1);
				socket_web_client.Close();
			}
			//free(get);
			//free(ip);

        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
void usage()
{
	fprintf(stderr, "USAGE: htmlget host [page]\n\
		\thost: the website hostname. ex: coding.debuntu.org\n\
		\tpage: the page to retrieve. ex: index.html, default: /\n");
}

/*
Get ip from domain name
*/
char* get_ip(char* host)
{
	struct hostent* hent;
	int iplen = 15; //XXX.XXX.XXX.XXX
	char* ip = (char*)malloc(iplen + 1);
	memset(ip, 0, iplen + 1);
	if ((hent = gethostbyname(host)) == NULL)
	{
		perror("Can't get IP");
		exit(1);
	}
	if (inet_ntop(AF_INET, (void*)hent->h_addr_list[0], ip, iplen) == NULL)
	{
		perror("Can't resolve host");
		exit(1);
	}
	return ip;
}


char* build_get_query(char* host, char* page)
{
	char* query;
	char* getpage = page;
	char* tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
	if (getpage[0] == '/') {
		getpage = getpage + 1;
		fprintf(stderr, "Removing leading \"/\", converting %s to %s\n", page, getpage);
	}
	// -5 is to consider the %s %s %s in tpl and the ending \0
	query = (char*)malloc(strlen(host) + strlen(getpage) + strlen(USERAGENT) + strlen(tpl) - 5);
	sprintf_s(query, strlen(query) + 1, tpl, getpage, host, USERAGENT);
	return query;
}
string getHostFromRequest(string g)
{
	string host;
	if (g.find_first_of("GET") == 0)
	{
		stringstream getstream(g);
		int hostpos = g.find_first_of("Host: ");
		getstream.seekg(hostpos + 6);
		getstream >> host;
	}
}