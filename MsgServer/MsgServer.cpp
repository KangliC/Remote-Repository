/////////////////////////////////////////////////////////////////////////
// MsgServer.cpp - Demonstrates simple one-way HTTP style messaging    //
//                 and file transfer                                   //
//                                                                     //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016           //
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
* This package implements a server that receives HTTP style messages and
* files from multiple concurrent clients and simply displays the messages
* and stores files.
*
* It's purpose is to provide a very simple illustration of how to use
* the Socket Package provided for Project #4.
*/
/*
* Required Files:
*   MsgClient.cpp, MsgServer.cpp
*   HttpMessage.h, HttpMessage.cpp
*   Cpp11-BlockingQueue.h
*   Sockets.h, Sockets.cpp
*   FileSystem.h, FileSystem.cpp
*   Logger.h, Logger.cpp 
*   Utilities.h, Utilities.cpp
*/
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include "../BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include "../Handler/Handler.h"
#include "MsgServer.h"
#include "../Tokenizer/Tokenizer.h"
#include "../SemiExp/SemiExp.h"
#include "../Parser/Parser.h"
#include "../Parser/ActionsAndRules.h"
#include "../Parser/ConfigureParser.h"
#include "../Parser/MTree.h"
#include "../DataStore/DataStore.h"
#include "../FileMgr/FileMgr.h"
//#include "../FileMgr/FileSystem.h"
#include "../MetricsExecutive/MetricsExecutive.h"
#include <string>
#include <iostream>

using Show = StaticLogger<1>;
using namespace Utilities;


HttpMessage MsgServer::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = addr_client;
	myEndPoint += ":";
	myEndPoint += port_client; //example: "localhost:8081";
	switch (n)
	{
	case 1: {
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("GET", "Message"));//GET means data return from server
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
		msg.addAttribute(HttpMessage::attribute("GET", "Message"));
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

void MsgServer::sendMessage(HttpMessage& msg, Socket& socket)
{
	std::string msgString = msg.toString();
	socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}
std::string MsgServer::makeFilepath(std::string filepath) {
	size_t pos = filepath.find('\\');
	std::string str = filepath;
	while (pos != std::string::npos) {
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
bool MsgServer::sendFile(const std::string& fqname, Socket& socket)
{
	// assumes that socket is connected
	FileSystem::File file(fqname);
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
		return false;
	HttpMessage msg = makeMessage(2, fqname, fullClientPath);
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
void MsgServer::pushCommand(std::string commandMsg) {
	if (commandMsg.size() == 0)
		return;
	senderQ.enQ(commandMsg);
}
/*
Client -> Server
request to connect/disconnect:						"CONNECT "/"DISCONNECT " + "NULL " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT
request to upload file to server:					"UPLOAD "+ FILENAME + " " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT
request to save file list of FILENAME to client :	"SAVE " + FILENAME + " " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT
request filelist from server:						"GETLIST " + "ALLFILES " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT
request dependency list of FILENAME from server:	"GETDEP " + FILENAME + " " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT

Client <- Server
"CONNECTION " + "SUCCEED"/"FAILED"/"DISCONNECTED"
"DEPENDLIST " + FIRST ITEM + " " + SECOND ITEM + " " + THIRD ITEM + " "...
"FILELIST" + FIRST ITEM + " " + SECOND ITEM + " " + THIRD ITEM + " "...
"UPLOADED" + "SUCCEED"/"FAILED"
*/
std::string MsgServer::parseMsg(HttpMessage &Msg) {
	std::string returnCommand = "";
	std::string CMD;
	std::istringstream cmdstream(Msg.bodyString());
	std::string bodystring = Msg.bodyString();
	cmdstream >> CMD;
	if (CMD == "CONNECT") {
		returnCommand = connectParse(cmdstream);
	}
	else if (CMD == "DISCONNECT") {
		returnCommand = disconnectParse(cmdstream);
	}
	else if (CMD == "UPLOAD") {
		returnCommand = "UPLOADED SUCCEED";
	}
	else if (CMD == "SAVE") {
		returnCommand = saveParse(bodystring);
	}
	else if (CMD == "GETLIST") {
		returnCommand = getlistParse(cmdstream);
	}
	else if (CMD == "GETDEP") {
		returnCommand = getdepParse(bodystring);
	}
	else
		return "";
	return returnCommand;
}
std::string MsgServer::connectParse(std::istringstream &cmdstream) {
	std::string addr, port, ACTION, returnCommand = "";
	cmdstream >> ACTION;
	cmdstream >> addr;
	cmdstream >> port;
	addr_client = addr.substr(addr.find(":") + 1);
	port_client = std::stoi(port.substr(port.find(":") + 1));

	fullClientPath += addr_client;
	fullClientPath += ":";
	fullClientPath += port_client;

	returnCommand += "CONNECTION SUCCEED";
	connectionStatus = true;//todo: not used yet
	std::thread Sender(&MsgServer::sender, this);
	Sender.detach();
	return returnCommand;
}
std::string MsgServer::disconnectParse(std::istringstream &cmdstream) {
	//connectionStatus = false;
	return "CONNECTION DISCONNECTED";
}
//"UPLOAD " + FILENAME + " " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT
//"UPLOADED" + "SUCCEED"/"FAILED"
std::string MsgServer::uploadParse(std::istringstream &cmdstream) {
	return "";
}
//"FILES SAVED " + <file1 > + <file2 > + ...
std::string MsgServer::saveParse(std::string bodystring) {
	std::string returnComm = "FILES SAVED";

	std::string ACTION;
	ACTION = bodystring.substr(bodystring.find("<") + 1);
	ACTION = ACTION.substr(0, ACTION.find(" >") - 1);
	returnComm += " <";
	returnComm += ACTION;
	returnComm += " >";
	for (size_t i = 0; i < listDepend.size(); i++) {
		if (listDepend[i]->filename == ACTION) {
			for (size_t j = 0; j < listDepend[i]->dependency.size(); j++) {
				if (listDepend[i]->dependency[j] == " ")
					continue;
				returnComm += " <";
				returnComm += listDepend[i]->dependency[j];
				returnComm += " >";
			}
			return returnComm;
		}
	}
	return returnComm;
}
//"FILELIST" + FIRST ITEM + " " + SECOND ITEM + " " + THIRD ITEM + " "...
std::string MsgServer::getlistParse(std::istringstream &cmdstream) {
	std::string returnComm = "FILELIST ";
	DataStore ds;
	DataStore filePath;
	BlockingQueue<WorkResult> fileQueue;
	ProcessWorkItem<WorkResult> processor;
	processor.start();
	 Repository* pRepos = new Repository();
	MetricsExe exe(pRepos);
	exe.config(&fileQueue, &processor);
	exe.start();
	FileMgr fm(exe.getInputFromUser(), ds, filePath);
	fm.config(&fileQueue);
	fm.addPattern("*.h");
	fm.addPattern("*.cpp");
	fm.addPattern("*.c");
	fm.addPattern("*.java");
	fm.search();
	exe.wait();
	processor.wait();
	pRepos->showDepend();
	listDepend = pRepos->getDepend();
	for (size_t i = 0; i < listDepend.size(); i++) {
		returnComm += " <";
		returnComm += listDepend[i]->filename;
		returnComm += " >";
	}
	return returnComm;
}
//"DEPENDLIST " + FIRST ITEM + " " + SECOND ITEM + " " + THIRD ITEM + " "...
std::string MsgServer::getdepParse(std::string bodystring) {
	std::string returnComm = "DEPENDLIST ";
	std::string ACTION;
	ACTION = bodystring.substr(bodystring.find("<") + 1);
	ACTION = ACTION.substr(0, ACTION.find(" >")-1);

	for (size_t i = 0; i < listDepend.size(); i++) {
		if (listDepend[i]->filename == ACTION) {
			for (size_t j = 0; j < listDepend[i]->dependency.size(); j++) {
				returnComm += " <";
				returnComm += listDepend[i]->dependency[j];
				returnComm += " >";
			}
			return returnComm;
		}
	}
	return returnComm;
}

//receiver messsage from client
void MsgServer::receiver() {
	try {
		SocketSystem ss;
		SocketListener sl(port_server, Socket::IP6);
		BlockingQueue<HttpMessage> &msgQ = globalQ<HttpMessage, 4>().get();
		ReceiverHandler rh(msgQ);
		sl.start(rh);
		while (true)
		{
			HttpMessage msg = msgQ.deQ();
			pushCommand(parseMsg(msg));
			Show::write("\n\n  server recvd message contents:\n" + msg.bodyString());
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  receiver() Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}
//send message/file to client
void MsgServer::sender() {
	Show::attach(&std::cout);
	Show::start();
	Show::title("Starting HttpMessage client, on thread: " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id()));
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect(addr_client, port_client)){
			Show::write("\n client waiting to connect");
			::Sleep(100);
		}
		while (connectionStatus) {
			std::string cmdMsg = senderQ.deQ();
			std::string CMD, ACTION;
			std::istringstream cmdstream(cmdMsg);
			cmdstream >> CMD;
			if (CMD == "FILES") {
				//for (size_t i = 0;)
				size_t pos=cmdMsg.find("<");
				std::string subfile = cmdMsg, file;
				while (pos != std::string::npos) {
					subfile = subfile.substr(pos + 1);
					file = subfile.substr(0, subfile.find(">"));
					sendFile(makeFilepath(file), si);
					pos = subfile.find("<");
				}
			}
			else
				sendMessage(makeMessage(1, cmdMsg, fullClientPath), si);//"localhost:8080"
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  sender() Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}
//create threads for server: receiver and sender
void MsgServer::execute() {
	std::thread Receiver(&MsgServer::receiver, this);
	Receiver.detach();
	//std::thread Sender(&MsgServer::sender, this);
	//Sender.detach();
}

//----< test stub >--------------------------------------------------

int main()
{
  ::SetConsoleTitle(L"HttpMessage Server - Runs Forever");

  Show::attach(&std::cout);
  Show::start();
  Show::title("\n  HttpMessage Server started");

  MsgServer *server = new MsgServer("localhost", 8080);//listen to localhost:8080
  server->execute();
  while(true){}
#ifdef DEBUG
  BlockingQueue<HttpMessage> msgQ;
  
  try
  {
    SocketSystem ss;
    SocketListener sl(8080, Socket::IP6);
    ReceiverHandler cp(msgQ);
    sl.start(cp);
    /*
     * Since this is a server the loop below never terminates.
     * We could easily change that by sending a distinguished 
     * message for shutdown.
     */
    while (true)
    {
      HttpMessage msg = msgQ.deQ();
      Show::write("\n\n  server recvd message contents:\n" + msg.bodyString());
    }
  }
  catch (std::exception& exc)
  {
    Show::write("\n  Exeception caught: ");
    std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
    Show::write(exMsg);
  }
#endif // DEBUG
}