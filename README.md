# Remote-Repository
Remote repository developed with C++

README:
Test: 
1. connect first,
2. request file list
3. get dependency
4. save
5. upload


protocol config:

globalQ<T, ID> create a static queue works as a global queue and use ID to identify one specific BlockingQueue.
BlockingQueue<T> globalQ<T, 1>().get() for Client return message to MockChannel.
BlockingQueue<T> globalQ<T, 2>().get() for Client send message to Server.
BlockingQueue<T> globalQ<T, 3>().get() for Client get message from Server.
BlockingQueue<T> globalQ<T, 4>().get() for Server get message from Client.
BlockingQueue<T> globalQ<T, 5>().get() for Server send message to Client.

MockChannel -> Client
request to connect/disconnect: "CONNECT "/"DISCONNECT " + "NULL " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT
request to upload file to server: "UPLOAD "+ FILENAME + " " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT
request to save file list of FILENAME to client : "SAVE " + FILENAME + " " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT
request filelist from server: "GETLIST " + "ALLFILES " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT
request dependency list of FILENAME from server: "GETDEP " + FILENAME + " " + "CLIENT_ADDR:" + CLIENT ADDRESS + " " + "CLIENT_PORT:" + CLIENT PORT

MockChannel <- Client || Client <- Server
"CONNECTION " + "SUCCEED"/"FAILED"/"DISCONNECTED"
"DEPENDLIST " + FIRST ITEM + " " + SECOND ITEM + " " + THIRD ITEM + " "...
"FILELIST" + FIRST ITEM + " " + SECOND ITEM + " " + THIRD ITEM + " "... 
"UPLOADED" + "SUCCEED"/"FAILED"
"FILES " + "SAVED " + <file1 > + <file2 > + ...

TEST INSTRUCTION:
The project will read repository_server directory as the root folder for server.
And client will download files from server to repository_client directory.
