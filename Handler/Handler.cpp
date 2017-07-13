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
#include "Handler.h"

using Show = StaticLogger<1>;
using namespace Utilities;

void ReceiverHandler::Get_Client(HttpMessage& msg, Socket& socket) {
	std::string filename = msg.findValue("file");
	if (filename != ""){
		size_t contentSize;
		std::string sizeString = msg.findValue("content-length");
		if (sizeString != "")
			contentSize = Converter<size_t>::toValue(sizeString);
		else
			return;
		readFile(filename, contentSize, socket, 1);
	}
	if (filename != ""){// construct message body
		msg.removeAttribute("content-length");
		std::string bodyString = "<file>" + filename + "</file>";
		std::string sizeString = Converter<size_t>::toString(bodyString.size());
		msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
		msg.addBody(bodyString);
	}
	else{// read message body
		size_t numBytes = 0;
		size_t pos = msg.findAttribute("content-length");
		if (pos < msg.attributes().size()){
			numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
			Socket::byte* buffer = new Socket::byte[numBytes + 1];
			socket.recv(numBytes, buffer);
			buffer[numBytes] = '\0';
			std::string msgBody(buffer);
			msg.addBody(msgBody);
			delete[] buffer;
		}
	}
}

void ReceiverHandler::Post_Server(HttpMessage& msg, Socket& socket) {
	std::string filename = msg.findValue("file");
	if (filename != ""){
		size_t contentSize;
		std::string sizeString = msg.findValue("content-length");
		if (sizeString != "")
			contentSize = Converter<size_t>::toValue(sizeString);
		else
			return;
		readFile(filename, contentSize, socket, 0);
	}
	if (filename != ""){// construct message body
		msg.removeAttribute("content-length");
		std::string bodyString = "<file>" + filename + "</file>";
		std::string sizeString = Converter<size_t>::toString(bodyString.size());
		msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
		msg.addBody(bodyString);
	}
	else{// read message body
		size_t numBytes = 0;
		size_t pos = msg.findAttribute("content-length");
		if (pos < msg.attributes().size()){
			numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
			Socket::byte* buffer = new Socket::byte[numBytes + 1];
			socket.recv(numBytes, buffer);
			buffer[numBytes] = '\0';
			std::string msgBody(buffer);
			msg.addBody(msgBody);
			delete[] buffer;
		}
	}
}

HttpMessage ReceiverHandler::readMessage(Socket& socket)
{
	connectionClosed_ = false;
	HttpMessage msg;
	// read message attributes
	while (true)
	{
		std::string attribString = socket.recvString('\n');
		if (attribString.size() > 1)
		{
			HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
			msg.addAttribute(attrib);
		}
		else
			break;
	}
	// If client is done, connection breaks and recvString returns empty string
	if (msg.attributes().size() == 0)
	{
		connectionClosed_ = true;
		return msg;
	}
	if (msg.attributes()[0].first == "POST")// read body if POST - server get data from client
		Post_Server(msg, socket);
	else if (msg.attributes()[0].first == "GET") // read body if GET - client get data from server
		Get_Client(msg, socket);
	return msg;
}
//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ReceiverHandler::readFile(const std::string& filename, size_t fileSize, Socket& socket, size_t ser_or_cli)
{
	static size_t serverFlag = 0, clientFlag = 1;
	std::string fqname; //= "../TestFiles/" + filename + ".snt";
	if (ser_or_cli == serverFlag)
		fqname = "../Repository_server/" + filename;
	else
		fqname = "../Repository_client/" + filename;
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood())
	{
		/*
		* This error handling is incomplete.  The client will continue
		* to send bytes, but if the file can't be opened, then the server
		* doesn't gracefully collect and dump them as it should.  That's
		* an exercise left for students.
		*/
		Show::write("\n\n  can't open file " + fqname);
		return false;
	}
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	size_t bytesToRead;
	while (true)
	{
		if (fileSize > BlockSize)
			bytesToRead = BlockSize;
		else
			bytesToRead = fileSize;

		socket.recv(bytesToRead, buffer);

		FileSystem::Block blk;
		for (size_t i = 0; i < bytesToRead; ++i)
			blk.push_back(buffer[i]);

		file.putBlock(blk);
		if (fileSize < BlockSize)
			break;
		fileSize -= BlockSize;
	}
	file.close();
	return true;
}
//----< receiver functionality is defined by this function >---------

void ReceiverHandler::operator()(Socket socket)
{
	/*
	* There is a potential race condition due to the use of connectionClosed_.
	* If two clients are sending files at the same time they may make changes
	* to this member in ways that are incompatible with one another.  This
	* race is relatively benign in that it simply causes the readMessage to
	* be called one extra time.
	*
	* The race is easy to fix by changing the socket listener to pass in a
	* copy of the clienthandler to the clienthandling thread it created.
	* I've briefly tested this and it seems to work.  However, I did not want
	* to change the socket classes this late in your project cycle so I didn't
	* attempt to fix this.
	*/
	while (true)
	{
		HttpMessage msg = readMessage(socket);
		if (connectionClosed_ || msg.bodyString() == "quit")
		{
			Show::write("\n\n  clienthandler thread is terminating");
			break;
		}
		msgQ_.enQ(msg);
	}
}


#ifdef HANDLER_DEBUG
int main() {
	std::cout << "test Handler\n";
	return 1;
}
#endif
