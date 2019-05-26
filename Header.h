#pragma once
#include "pch.h"
// The one and only application object
bool isHTTPrequest(string request);
bool isEndMessage(char* buf);
bool isEndMessage(string message);
string devideIntoResponse(string& response);
//int create_tcp_socket();
char* get_ip(char* host);
string build_get_query(string get);
void usage();
string getHostFromRequest(string g);
void requestFix(string& R);

#define HOST "www.fit.hcmus.edu.vn"
#define PAGE "/"
#define PORT 80
#define CLIENT_PORT 8888
const string error_403 = "HTTP/1.1 403 Forbidden\r\nCache-Control: no-cache\r\nConnection: close";
using namespace std;
//Ref: http://stackoverflow.com/questions/19715144/how-to-convert-char-to-lpcwstr
string getHeader(vector<char> message);
int getContentLength(string header);
wchar_t* convertCharArrayToLPCWSTR(const char* charArray);
string getTransferEncoding(string header);
bool ischunkedBodyEnd(string body);
DWORD WINAPI response_send(LPWORD lpParameter);
DWORD WINAPI request_send(LPWORD lpParameter);
string getContentType(string header);
bool isGetRequest(string req);
bool isPostRequest(string req);
vector<char> receiveResponse(CSocket* web_server);
void vectorchar_append(vector<char>& v, char* c, int len);
void vectorchar_append(vector<char>& v, string s);
//POST:
//get the header of the request, read the content-lenght, read the body