/////////////////////////////////////////////////////////////////////////////
// MockChannel.cpp - Demo for CSE687 Project #4, Spring 2015               //
// - build as DLL to show how C++\CLI client can use native code channel   //
// - MockChannel reads from sendQ and writes to recvQ                      //
// ver 1.1
// - add method to send command to client.
// - add method to start client.
// ver 1.0
// Author: Kangli Chu                                                      //
// Source: Jim Fawcett, CSE687 - Object Oriented Design, Spring 2015       //
/////////////////////////////////////////////////////////////////////////////

#define IN_DLL
#include "MockChannel.h"
#include "../BlockingQueue/Cpp11-BlockingQueue.h"
#include"../MsgClient/MsgClient.h"
#include"../globalQ/globalQ.h"
#include <string>
#include <thread>
#include <iostream>

using BQueue = BlockingQueue < Message >;

/////////////////////////////////////////////////////////////////////////////
// Sendr class
// - accepts messages from client for consumption by MockChannel
//
class Sendr : public ISendr
{
public:
  void postMessage(const Message& msg);
  BQueue& queue();
private:
  BQueue sendQ_;
};

void Sendr::postMessage(const Message& msg)
{
  sendQ_.enQ(msg);
}

BQueue& Sendr::queue() { return sendQ_; }

/////////////////////////////////////////////////////////////////////////////
// Recvr class
// - accepts messages from MockChanel for consumption by client
//
class Recvr : public IRecvr
{
public:
  Message getMessage();
  BQueue& queue();
private:
  BQueue recvQ_;
};

Message Recvr::getMessage()
{
  return recvQ_.deQ();
}

BQueue& Recvr::queue()
{
  return recvQ_;
}
/////////////////////////////////////////////////////////////////////////////
// MockChannel class
// - reads messages from Sendr and writes messages to Recvr
//
class MockChannel : public IMockChannel
{
public:
  MockChannel(ISendr* pSendr, IRecvr* pRecvr);
  void start(std::string addr_ser, int port_ser, std::string addr_cli, int port_cli);
  void stop();
  void runClient();
  void save(std::string msg, IRecvr* pRecvr);
  void upload(std::string msg, IRecvr* pRecvr);
  void connectHandler(std::string msg, IRecvr* pRecvr); //connection or disconnection
  void getlist(std::string msg, IRecvr* pRecvr);
  void getdep(std::string msg, IRecvr* pRecvr);
private:
  std::thread thread_;
  ISendr* pISendr_;
  IRecvr* pIRecvr_;
  bool stop_ = false;
  bool connectionStatus;
  MsgClient* client;
};

//----< pass pointers to Sender and Receiver >-------------------------------

MockChannel::MockChannel(ISendr* pSendr, IRecvr* pRecvr) : pISendr_(pSendr), pIRecvr_(pRecvr) { connectionStatus = false; }

void MockChannel::save(std::string msg, IRecvr* pRecvr) {
	BlockingQueue<std::string> &mockandClient = globalQ<std::string, 1>().get();
	client->getCommand(msg);
	msg = mockandClient.deQ();
	//"DEPENDLIST " + FIRST ITEM + " " + SECOND ITEM + " " + THIRD ITEM + " "...

	BQueue& recvQ = ((Recvr*)pRecvr)->queue();
	recvQ.enQ(msg);
}
void MockChannel::upload(std::string msg, IRecvr* pRecvr) {
	std::cout << msg;
	BlockingQueue<std::string> &mockandClient = globalQ<std::string, 1>().get();
	client->getCommand(msg);
	msg = mockandClient.deQ();
	//"UPLOADED" + "SUCCEED"/"FAILED"

	BQueue& recvQ = ((Recvr*)pRecvr)->queue();
	recvQ.enQ(msg);
}
void MockChannel::connectHandler(std::string msg, IRecvr* pRecvr) {
	BlockingQueue<std::string> &mockandClient = globalQ<std::string, 1>().get();
	client->getCommand(msg);
	msg = mockandClient.deQ();
	//"CONNECTION " + "FAILED"/"SUCCEED"/"DISCONNECTED"
	std::string command = msg.substr(0, msg.find(" "));
	if (command == "CONNECTION" && msg.substr(msg.find(" ") + 1) == "FAILED") 
		connectionStatus = false;
	BQueue& recvQ = ((Recvr*)pRecvr)->queue();
	recvQ.enQ(msg);
}
void MockChannel::getlist(std::string msg, IRecvr* pRecvr) {
	BlockingQueue<std::string> &mockandClient = globalQ<std::string, 1>().get();
	client->getCommand(msg);
	msg = mockandClient.deQ();
	//"FILELIST" + FIRST ITEM + " " + SECOND ITEM + " " + THIRD ITEM + " "... 

	BQueue& recvQ = ((Recvr*)pRecvr)->queue();
	recvQ.enQ(msg);
}
void MockChannel::getdep(std::string msg, IRecvr* pRecvr) {
	BlockingQueue<std::string> &mockandClient = globalQ<std::string, 1>().get();
	client->getCommand(msg);
	msg = mockandClient.deQ();
	//"GETDEP " + FILENAME + " " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT

	BQueue& recvQ = ((Recvr*)pRecvr)->queue();
	recvQ.enQ(msg);
}

//----< creates thread to read from sendQ and echo back to the recvQ >-------

void MockChannel::start(std::string addr_ser, int port_ser, std::string addr_cli, int port_cli)
{
  std::cout << "\n  MockChannel starting up";
  client = new MsgClient(addr_ser, port_ser, addr_cli, port_cli);
  thread_ = std::thread([this] {
    Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);
    Recvr* pRecvr = dynamic_cast<Recvr*>(pIRecvr_);
	this->runClient();
    if (pSendr == nullptr || pRecvr == nullptr){
      std::cout << "\n  failed to start Mock Channel\n\n";
      return;
	}
	BQueue& sendQ = pSendr->queue(); BQueue& recvQ = pRecvr->queue();
    while (!stop_){
      std::cout << "\n  channel deQing message";
      Message msg = sendQ.deQ();  // will block here so send quit message when stopping
	  if (msg.substr(0, msg.find(" ")) == "CONNECT" && connectionStatus == false) {
		  connectionStatus = true;
		  connectHandler(msg, pRecvr);
	  }else if (msg.substr(0, msg.find(" ")) == "DISCONNECT" && connectionStatus == true) {
		  connectionStatus = false;
		  connectHandler(msg, pRecvr);
	  }else if (msg.substr(0, msg.find(" ")) == "UPLOAD")
		  upload(msg, pRecvr);
	  else if (msg.substr(0, msg.find(" ")) == "SAVE")
		  save(msg, pRecvr);
	  else if (msg.substr(0, msg.find(" ")) == "GETLIST")
		  getlist(msg, pRecvr);
	  else if (msg.substr(0, msg.find(" ")) == "GETDEP")
		  getdep(msg, pRecvr);
      recvQ.enQ(msg);
    }
    std::cout << "\n  Server stopping\n\n";
  });
}
//----< signal server thread to stop >---------------------------------------

void MockChannel::stop() { stop_ = true; }

void MockChannel::runClient() {
	//start the client here. Client for communication with Repository.
	client->execute();
}

//----< factory functions >--------------------------------------------------

ISendr* ObjectFactory::createSendr() { return new Sendr; }

IRecvr* ObjectFactory::createRecvr() { return new Recvr; }

IMockChannel* ObjectFactory::createMockChannel(ISendr* pISendr, IRecvr* pIRecvr) 
{ 
  return new MockChannel(pISendr, pIRecvr); 
}

#ifdef TEST_MOCKCHANNEL

//----< test stub >----------------------------------------------------------

int main()
{
  ObjectFactory objFact;
  ISendr* pSendr = objFact.createSendr();
  IRecvr* pRecvr = objFact.createRecvr();
  IMockChannel* pMockChannel = objFact.createMockChannel(pSendr, pRecvr);
  pMockChannel->start("localhost", 9090, "localhost", 8080);
  pSendr->postMessage("Hello World");
  pSendr->postMessage("CSE687 - Object Oriented Design");
  Message msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  pSendr->postMessage("stopping");
  msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  pMockChannel->stop();
  pSendr->postMessage("quit");
  std::cin.get();
}
#endif
