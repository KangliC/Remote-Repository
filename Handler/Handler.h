////////////////////////////////////////////////////////////////////////////////////////////
// ReceiverHandler class - for CSE687 Project #4, Spring 2015	
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - Low level code, connect message from server/client to socket.
// ver 1.0
// Author: Kangli Chu,  CSE687 - Object Oriented Design, Spring 2015                      //
////////////////////////////////////////////////////////////////////////////////////////////
#ifndef HANDLER_H
#define HANDLER_H

#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include "../BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>

class ReceiverHandler
{
public:
	ReceiverHandler(BlockingQueue<HttpMessage>& msgQ) : msgQ_(msgQ) {}
	void operator()(Socket socket);
private:
	void Get_Client(HttpMessage& msg, Socket& socket);
	void Post_Server(HttpMessage& msg, Socket& socket);
	bool connectionClosed_;
	HttpMessage readMessage(Socket& socket);
	bool readFile(const std::string& filename, size_t fileSize, Socket& socket, size_t ser_or_cli);
	BlockingQueue<HttpMessage> &msgQ_;
	std::string addr_server;
	std::string addr_client;
	int port_server;
	int port_client;
	std::string fullServerPath;
	std::string fullClientPath;
};
#endif // !HANDLER_H
