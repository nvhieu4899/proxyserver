// ProxyServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "ProxyServer.h"
#include "Header.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CWinApp theApp;

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
			AfxSocketInit(NULL);

			CSocket socket_proxy_server;
			int tmpres;
			string ip;
			string get;
			char buf[BUFSIZ + 1];
			string host;
			string page;
			string response;
			string GETrequest;
			string lasthost;
			int tmpres2;
			//Buoc 1: Khoi dong socket trong Window

			//Buoc 2: Tao socket
			if (socket_proxy_server.Create(CLIENT_PORT) == 0)
			{
				cout << "Khoi tao server that bai\n";
				cout << socket_proxy_server.GetLastError();
				return FALSE;
			}
			else
			{
				cout << "Khoi tao server thanh cong !!!\n";
			}
			do {
				socket_proxy_server.Listen();
				//Nhan ket noi tu proxy client
				CSocket connector;
				if (socket_proxy_server.Accept(connector))
				{
					cout << "Da co client ket noi toi\n";
					get.clear();
					memset(buf, 0, sizeof(buf));
					do {
						get.clear();
						while ((tmpres = connector.Receive(buf, BUFSIZ, 0)) > 0)
						{
							int socket_s = connector.sends;
							int socket_r = connector.receives;
							if (buf) {
								get.append(buf);
							}

							if (isEndMessage(buf))
							{
								break;
							}
							memset(buf, 0, tmpres);
						}
						if (get.empty()) break;
						//Buoc 3: Ket noi toi web Server
						CSocket socket_web_client;
						if (!isHTTPrequest(get)) break;
						if (isGetRequest(get))
						{
							host = getHostFromRequest(get);
							ip = get_ip((char*)host.c_str());
							socket_web_client.Create();
							if (socket_web_client.Connect(convertCharArrayToLPCWSTR(ip.c_str()), PORT) < 0)
							{
								perror("Could not connect");
								continue;
							}
							//Send the query to the server
							requestFix(get);
							tmpres = socket_web_client.Send(get.c_str(), strlen(get.c_str()), 0);
							if (tmpres == -1) {
								perror("Can't send query");
							}
						}
						if (isPostRequest(get)) break;
						//now it is time to receive the page
						//Receive the header first
						vector<char> message = receiveResponse(&socket_web_client);
						string header = getHeader(message);
						if (getContentType(header)!="text/plain")
						connector.Send(message.data(), message.size(), 0);
						else
						{
							socket_web_client.Close();
							connector.Close();
							break;
						}
						socket_web_client.Close();
						connector.Close();
						//sent = 0;
						//caching time

						//send respond to proxy client

					} while (1);
					}
			} while (1);
			socket_proxy_server.Close();
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


