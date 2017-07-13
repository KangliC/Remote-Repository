#ifndef MSGCLIENT_H
#define MSGCLIENT_H
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include "../globalQ/globalQ.h"

class MsgClient
{
public:
	//MsgClient() { }
	MsgClient(std::string addr_ser, int port_ser, std::string addr_cli, int port_cli){
		fullServerPath = addr_server = addr_ser;
		fullClientPath = addr_client = addr_cli;
		port_server = port_ser;
		port_client = port_cli;
		fullClientPath += ":";
		fullClientPath += port_client;
		fullServerPath += ":";
		fullServerPath += port_server;
	}
	MsgClient(){}
	using EndPoint = std::string;
	void execute();
	void sender();
	void receiver();
	void getCommand(std::string commandMsg);//get command from mockchannel.
	BlockingQueue<std::string> &senderQ = globalQ<std::string, 2>().get();
private:
	HttpMessage makeMessage(size_t n, const std::string& msgBody, const EndPoint& ep);
	std::string makeFilepath(std::string filepath);
	void sendMessage(HttpMessage& msg, Socket& socket);
	bool sendFile(const std::string& fqname, Socket& socket);
	std::string addr_server;
	std::string addr_client;
	int port_server;
	int port_client;
	std::string fullServerPath;
	std::string fullClientPath;
};

#endif // !MSGCLIENT_H