#include "Header.h"
#include "pch.h"
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


string build_get_query(string get)
{
	stringstream streamget(get);
	string useragent;
	int useragentpos = get.find("User-Agent: ");
	string res = get.substr(0, useragentpos + 12);
	streamget.seekg(useragentpos + 12);
	getline(streamget, useragent, '\r');
	res = res + useragent +"\r\nConnection: close"+ "\r\n\r\n";

	return res;
}
string getHostFromRequest(string g)
{
	string host;
	stringstream getstream(g);
	int pos = g.find("Host: ");
	getstream.seekg(pos + 6);
	getstream >> host;

	return host;
}
bool isHTTPrequest(string request)
{
	string host = getHostFromRequest(request);
	stringstream shost(host);
	int pos = host.find_last_of(":");
	if (pos < 0) return true;
	else shost.seekg(pos + 1);
	int port;
	shost >> port;
	if (port != 443)
		return true;
	return false;
}
bool isEndMessage(char* buf)
{
	string temp(buf);
	if (temp.find_first_of("\r\n\r\n") != -1)
	{
		return true;
	}
	return false;
}
void requestFix(string & R)
{
	//stringstream sRequest(R);
	if (R.empty()) return;
	int httpPos = R.find_first_of("http://");
	string host = getHostFromRequest(R);
	string erase = "http://" + host;
	R.erase(httpPos, erase.length());
	R = build_get_query(R);
}
bool isEndMessage(string message)
{
	int len = message.length();
	if (message.find("\r\n\r\n") != message.npos)
	{
		return true;
	}
	return false;
}
string devideIntoResponse(string & response)
{
	int endPos = response.find("\r\n\r\n") + 4;
	string ret;
	ret = response.substr(endPos, response.length() - 1);
	response = response.substr(0, endPos);
	return ret;
}
wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}
//get header from first part of message (not FULL)
string getHeader(vector<char> message)
{
	string header;
	int i = 0;
	while (header.find("\r\n\r\n")==header.npos)
	{
		header.push_back(message[i]);
		i++;
	}
		return header;
}
int getContentLength(string header)
{
	stringstream s(header);
	int ret;
	s.seekg(header.find("Content-Length: ") + 16);
	s >> ret;
	return ret;
}
string getTransferEncoding(string header)
{
	stringstream s(header);
	string TE;
	s.seekg(header.find("Transfer-Encoding: ") + 19);
	getline(s, TE,'\r');
	return TE;
}
bool ischunkedBodyEnd(string body)
{
	return body.find("0\r\n\r\n") != body.npos;
}
int utf8len(char* buf)
{
	int len = 0;
	for (int i = 0; i < strlen(buf); i++)
	{
		if (buf >= 0) len += 1;
		else len += 2;
	}
	return len;
}
string getContentType(string header)
{
	stringstream s(header);
	string contenttype;
	s.seekg(header.find("Content-Type: ") + 14);
	getline(s, contenttype, '\r');
	return contenttype;
}
bool isGetRequest(string req)
{
	return (req.find("GET") == 0);
}
bool isPostRequest(string req)
{
	return (req.find("POST") == 0);
}
void vectorchar_append(vector<char>& v, char* c, int len)
{
	for (int i = 0; i < len; i++)
	{
		v.push_back(c[i]);
	}
}
void vectorchar_append(vector<char>& v, string s)
{
	int len = s.length();
	for (int i = 0; i < len; i++)
	{
		v.push_back(s[i]);
	}
}
//Web_server da dc create va gui get query
vector<char> receiveResponse(CSocket* web_server)
{

	vector<char> mess_vector;
	char* message;
	string temp;
	char buf[BUFSIZ + 1];
	int tmpres;
	string header;
	string body;
	char* header_c;
	//get the header first;
	memset(buf, 0, sizeof(buf));
	while ((tmpres = web_server->Receive(buf, BUFSIZ, 0)) > 0)
	{
		if (buf) {
			header += buf;
		}
		if (tmpres == 0) break;
		if (isEndMessage(header)) break;
		memset(buf, 0, tmpres);
	}
	body = devideIntoResponse(header);
	//Receive the body
	string TransferEnc = getTransferEncoding(header);
	memset(buf, 0, sizeof(buf));
	if (TransferEnc.find("chunked") != TransferEnc.npos)
	{
		while ((tmpres = web_server->Receive(buf, BUFSIZ, 0)) >= 0)
		{
			if (buf) {
				body.append(buf);
			}
			if (buf == "") break;
			memset(buf, 0, tmpres);
			if (tmpres < 0)
			{
				perror("Error receiving data");
			}
			if (ischunkedBodyEnd(body)) break;
		}
		//prepair the message;
		vectorchar_append(mess_vector, header);
		vectorchar_append(mess_vector, body);
	}
	else
	{
		//!!!!!! Error in content length and how many bytes to read
		int bodylen = body.length();
		int contentLenght = getContentLength(header);
		int bytes = contentLenght - bodylen;
		int received = 0;
		vectorchar_append(mess_vector, header);
		vectorchar_append(mess_vector, body);
		char* bodybuffer = new char[bytes];

		memset(bodybuffer, 0, bytes);
		web_server->Receive(bodybuffer, bytes, 0);
		vectorchar_append(mess_vector, bodybuffer, bytes);
		free(bodybuffer);
	}

	return mess_vector;
}
