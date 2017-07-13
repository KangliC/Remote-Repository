/////////////////////////////////////////////////////////////////////////
// MsgClient.cpp - Demonstrates simple one-way HTTP messaging          //
//                                                                     //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016           //
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
 * This package implements a client that sends HTTP style messages and
 * files to a server that simply displays messages and stores files.
 *
 * It's purpose is to provide a very simple illustration of how to use
 * the Socket Package provided for Project #4.
 */
/*
 * ToDo:
 * - pull the sending parts into a new Sender class
 * - You should create a Sender like this:
 *     Sender sndr(endPoint);  // sender's EndPoint
 *     sndr.PostMessage(msg);
 *   HttpMessage msg has the sending adddress, e.g., localhost:8080.
 */
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include"../Handler/Handler.h"
#include "MsgClient.h"
#include <string>
#include <iostream>
#include <thread>


using Show = StaticLogger<1>;
using namespace Utilities;
using Utils = StringHelper;

/////////////////////////////////////////////////////////////////////
// ClientCounter creates a sequential number for each client
//
class ClientCounter
{
public:
  ClientCounter() { ++clientCount; }
  size_t count() { return clientCount; }
private:
  static size_t clientCount;
};

size_t ClientCounter::clientCount = 0;

//----< factory for creating messages >------------------------------
/*
 * This function only creates one type of message for this demo.
 * - To do that the first argument is 1, e.g., index for the type of message to create.
 * - The body may be an empty string.
 * - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
 *   expects the receiver EndPoint for the toAddr attribute.
 */
HttpMessage MsgClient::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
  HttpMessage msg;
  HttpMessage::Attribute attrib;
  EndPoint myEndPoint = addr_client;
  myEndPoint += ":";
  myEndPoint+=port_client; //example: "localhost:8081";
  switch (n)
  {
  case 1: {
	  msg.clear();
	  msg.addAttribute(HttpMessage::attribute("POST", "Message"));
	  msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
	  msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
	  msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));
	  msg.addBody(body);
	  if (body.size() > 0)
	  {
		  attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
		  msg.addAttribute(attrib);
	  }
	  break; }
  case 2: {
	  std::string filename = body.substr(body.rfind("\\"));
	  FileSystem::FileInfo fi(body);
	  size_t fileSize = fi.size();
	  std::string sizeString = Converter<size_t>::toString(fileSize);
	  msg.clear();
	  msg.addAttribute(HttpMessage::attribute("POST", "Message"));
	  msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
	  msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
	  msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));
	  msg.addBody("");
	  msg.addAttribute(HttpMessage::Attribute("file", filename));
	  msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
	  break; }
  default: {
	  msg.clear();
	  msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	  break; }
  }
  return msg;
}
//----< send message using socket >----------------------------------

void MsgClient::sendMessage(HttpMessage& msg, Socket& socket)
{
  std::string msgString = msg.toString();
  socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}
std::string MsgClient::makeFilepath(std::string filepath) {
	size_t pos = filepath.find('\\');
	std::string str = filepath;
	while (pos!=std::string::npos) {
		str.insert(pos, "\\");
		pos = str.find('\\', pos + 2);
	}
	return str;
}
//----< send file using socket >-------------------------------------
/*
 * - Sends a message to tell receiver a file is coming.
 * - Then sends a stream of bytes until the entire file
 *   has been sent.
 * - Sends in binary mode which works for either text or binary.
 */
bool MsgClient::sendFile(const std::string& fqname, Socket& socket)
{
  // assumes that socket is connected
  FileSystem::File file(fqname);
  file.open(FileSystem::File::in, FileSystem::File::binary);
  if (!file.isGood())
    return false;
  HttpMessage msg = makeMessage(2, fqname, fullServerPath);
  sendMessage(msg, socket);
  const size_t BlockSize = 2048;
  Socket::byte buffer[BlockSize];
  while (true)
  {
    FileSystem::Block blk = file.getBlock(BlockSize);
    if (blk.size() == 0)
      break;
    for (size_t i = 0; i < blk.size(); ++i)
      buffer[i] = blk[i];
    socket.send(blk.size(), buffer);
    if (!file.isGood())
      break;
  }
  file.close();
  return true;
}
//enqueue command message to senderQ
void MsgClient::getCommand(std::string commandMsg) {
	senderQ.enQ(commandMsg);
}
//receiver messsage from server
void MsgClient::receiver() {
	try {
		SocketSystem ss;
		SocketListener sl(port_client, Socket::IP6);
		BlockingQueue<HttpMessage> &msgQ = globalQ<HttpMessage, 3>().get();
		BlockingQueue<std::string> &mockandClient = globalQ<std::string, 1>().get();//communicate with mockchannel
		ReceiverHandler rh(msgQ);
		sl.start(rh);
		while (true){
			HttpMessage msg = msgQ.deQ();
			mockandClient.enQ(msg.bodyString());
			Show::write("\n\n  client recvd message contents:\n" + msg.bodyString());
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  receiver() Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}
//send message/file to server
void MsgClient::sender() {
	Show::attach(&std::cout);
	Show::start();
	Show::title("Starting HttpMessage client, on thread: " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id()));
	try{
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect(addr_server, port_server)){
			Show::write("\n client waiting to connect");
			::Sleep(100);
		}
		while (true) {
			std::string cmdMsg = senderQ.deQ();
			std::string CMD;
			std::istringstream cmdstream(cmdMsg);
			cmdstream >> CMD;
			if (CMD == "UPLOAD") {
				std::string subfile = cmdMsg.substr(cmdMsg.find("<") + 1);
				std::string file = subfile.substr(0, subfile.find(">"));
				sendFile(makeFilepath(file), si);
			}else
				sendMessage(makeMessage(1, cmdMsg, fullServerPath), si);//"localhost:8080"
		}
	}
	catch (std::exception& exc){
		Show::write("\n  sender() Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}
void MsgClient::execute() {
	std::thread Receiver(&MsgClient::receiver, this);
	std::thread Sender(&MsgClient::sender, this);
	Sender.detach();
	Receiver.detach();
}

//----< entry point - runs two clients each on its own thread >------
#ifdef DEBUG_MSGCLIENT
int main()
{
  ::SetConsoleTitle(L"Clients Running on Threads");

  Show::title("Demonstrating two HttpMessage Clients each running on a child thread");

  //MsgClient c1;
  //std::thread t1(
  //  [&]() { c1.execute(100, 20); } // 20 messages 100 millisec apart
  //);

  //MsgClient c2;
  //std::thread t2(
  //  [&]() { c2.execute(120, 20); } // 20 messages 120 millisec apart
  //);
  //t1.join();
  //t2.join();
}
#endif
