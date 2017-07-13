#ifndef MSGSERVER_H
#define MSGSERVER_H
#include "../TypeAnalyze/TypeAnalyze.h"
#include"../globalQ/globalQ.h"

class MsgServer{
	public:
		MsgServer(std::string addr_ser, int port_ser) {
		fullServerPath = addr_server = addr_ser;
		//fullClientPath = addr_client = addr_cli;
		port_server = port_ser;
		//port_client = port_cli;
		//fullClientPath += ":";
		//fullClientPath += port_client;
		fullServerPath += ":";
		fullServerPath += port_server;
	}
	using EndPoint = std::string;
	std::string parseMsg(HttpMessage &Msg);
	void execute();
	void sender();
	void receiver();
	void pushCommand(std::string commandMsg);//get command from mockchannel.
	BlockingQueue<std::string> &senderQ = globalQ<std::string, 5>().get();
private:
	bool connectionStatus = false;
	HttpMessage makeMessage(size_t n, const std::string& msgBody, const EndPoint& ep);
	std::string makeFilepath(std::string filepath);
	void sendMessage(HttpMessage& msg, Socket& socket);
	bool sendFile(const std::string& fqname, Socket& socket);
	std::string connectParse(std::istringstream &cmdstream);
	std::string disconnectParse(std::istringstream &cmdstream);
	std::string uploadParse(std::istringstream &cmdstream);
	std::string saveParse(std::string bodystring);
	std::string getlistParse(std::istringstream &cmdstream);
	std::string getdepParse(std::string bodystring);
	std::string addr_server;
	std::string addr_client;
	int port_server;
	int port_client;
	std::string fullServerPath;
	std::string fullClientPath;
	std::vector<std::string> filelistContainer;
	std::vector<typeTable*> listDepend;
};

#endif // !MSGSERVER_H
