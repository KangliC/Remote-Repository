#ifndef WINDOW_H
#define WINDOW_H
///////////////////////////////////////////////////////////////////////////
// Window.h - C++\CLI implementation of WPF Application                  //
//          - Demo for CSE 687 Project #4                                //
// ver 2.1                                                               //
// Kangli Chu, CSE687 - Object Oriented Design, Spring 2015             //
///////////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  This package demonstrates how to build a C++\CLI WPF application.  It 
*  provides one class, WPFCppCliDemo, derived from System::Windows::Window
*  that is compiled with the /clr option to run in the Common Language
*  Runtime, and another class MockChannel written in native C++ and compiled
*  as a DLL with no Common Language Runtime support.
*
*  The window class hosts, in its window, a tab control with three views, two
*  of which are provided with functionality that you may need for Project #4.
*  It loads the DLL holding MockChannel.  MockChannel hosts a send queue, a
*  receive queue, and a C++11 thread that reads from the send queue and writes
*  the deQ'd message to the receive queue.
*
*  The Client can post a message to the MockChannel's send queue.  It hosts
*  a receive thread that reads the receive queue and dispatches any message
*  read to a ListBox in the Client's FileList tab.  So this Demo simulates
*  operations you will need to execute for Project #4.
*
*  Required Files:
*  ---------------
*  Window.h, Window.cpp, MochChannel.h, MochChannel.cpp,
*  Cpp11-BlockingQueue.h, Cpp11-BlockingQueue.cpp
*
*  Build Command:
*  --------------
*  devenv CppCli-WPF-App.sln
*  - this builds C++\CLI client application and native mock channel DLL
*
*  Maintenance History:
*  --------------------
*  ver 2.0 : 15 Apr 2015
*  - completed message passing demo with moch channel
*  - added FileBrowserDialog to show files in a selected directory
*  ver 1.0 : 13 Apr 2015
*  - incomplete demo with GUI but not connected to mock channel
*/
/*
* Create C++/CLI Console Application
* Right-click project > Properties > Common Language Runtime Support > /clr
* Right-click project > Add > References
*   add references to :
*     System
*     System.Windows.Presentation
*     WindowsBase
*     PresentatioCore
*     PresentationFramework
*/
using namespace System;
using namespace System::Text;
using namespace System::Windows;
using namespace System::Windows::Input;
using namespace System::Windows::Markup;
using namespace System::Windows::Media;                   // TextBlock formatting
using namespace System::Windows::Controls;                // TabControl
using namespace System::Windows::Controls::Primitives;    // StatusBar
using namespace System::Threading;
using namespace System::Threading::Tasks;
using namespace System::Windows::Threading;
using namespace System::ComponentModel;

//#include "../HttpMessage/HttpMessage.h"
//#include "../Sockets/Sockets.h"
//#include "../FileSystem/FileSystem.h"
//#include "../Logger/Logger.h"
//#include "../Utilities/Utilities.h"
//#include "../MsgClient/MsgClient.h"
//#include <string>
//#include <thread>
#include "../MockChannel/MockChannel.h"
#include <iostream>
#include <string>
#include <sstream>

namespace CppCliWindows
{
  ref class WPFCppCliDemo : Window
  {
    // MockChannel references

    ISendr* pSendr_;
    IRecvr* pRecvr_;
    IMockChannel* pChann_;

    // Controls for Window
    DockPanel^ hDockPanel = gcnew DockPanel();      // support docking statusbar at bottom
    Grid^ hGrid = gcnew Grid();                    
    TabControl^ hTabControl = gcnew TabControl();
    TabItem^ hSendMessageTab = gcnew TabItem();
    TabItem^ hFileListTab = gcnew TabItem();
    TabItem^ hConnectTab = gcnew TabItem();
	TabItem^ hTestTab = gcnew TabItem();
    StatusBar^ hStatusBar = gcnew StatusBar();
    StatusBarItem^ hStatusBarItem = gcnew StatusBarItem();
    TextBlock^ hStatus = gcnew TextBlock();
	
    // Controls for SendMessage View

    Grid^ hSendMessageGrid = gcnew Grid();
    Button^ hSendButton = gcnew Button();
    Button^ hClearButton = gcnew Button();
	Button^ hTestButton = gcnew Button();
	//add start 
	Button^ hRefreshButton = gcnew Button();
	Button^ hSaveButton = gcnew Button();
	Button^ hUploadButton = gcnew Button();
	 
	TextBlock^ hServerDir = gcnew TextBlock();
	TextBlock^ hDependName = gcnew TextBlock();
	TextBlock^ hClientFileSend = gcnew TextBlock();
	 
	ListBox^ hServerFileList = gcnew ListBox();
	ListBox^ hDependList = gcnew ListBox();
	ListBox^ hClientSelFile = gcnew ListBox();//not used
	 
	ScrollViewer^ hScrollViewerServerFileList = gcnew ScrollViewer();
	ScrollViewer^ hScrollViewerDependList = gcnew ScrollViewer();
	ScrollViewer^ hScrollViewerClientSelFile = gcnew ScrollViewer();

	TextBox^ hLocalAddr = gcnew TextBox();
	TextBox^ hLocalPort = gcnew TextBox();
	TextBox^ hServerAddr = gcnew TextBox();
	TextBox^ hServerPort = gcnew TextBox();
	Button^ hConnectButton = gcnew Button();
	Button^ hDisconnectButton = gcnew Button();
	Button^ hClearStatusButton = gcnew Button();
	Button^ hGetDepButton = gcnew Button();
	TextBox^ hConnStatus = gcnew TextBox();

	Grid^ hConnectGrid = gcnew Grid();
	//add end
    TextBlock^ hTextBlock1 = gcnew TextBlock();
    ScrollViewer^ hScrollViewer1 = gcnew ScrollViewer();
    StackPanel^ hStackPanel1 = gcnew StackPanel();

    String^ msgText 
      = "Command:ShowMessage\n"   // command
      + "Sendr:localhost@8080\n"  // send address
      + "Recvr:localhost@8090\n"  // receive address
      + "Content-length:44\n"     // body length attribute
      + "\n"                      // end header
      + "Hello World\nCSE687 - Object Oriented Design";  // message body

    // Controls for FileListView View

    Grid^ hFileListGrid = gcnew Grid();
    Forms::FolderBrowserDialog^ hFolderBrowserDialog = gcnew Forms::FolderBrowserDialog();
    ListBox^ hListBox = gcnew ListBox();
    Button^ hFolderBrowseButton = gcnew Button();
    Grid^ hGrid2 = gcnew Grid();

    // receive thread

    Thread^ recvThread;

  public:
    WPFCppCliDemo();
    ~WPFCppCliDemo();

    void setUpStatusBar();
    void setUpTabControl();
    void setUpSendMessageView();
    void setUpFileListView();
    void setUpConnectionView();

	//add start
	void setActionMsg();
	void setActionMain();
	void setActionCon();

	void setCtrlView();

	void connectServer(Object^ obj, RoutedEventArgs^ args);
	void disconnectServer(Object^ obj, RoutedEventArgs^ args);
	void clearConnectStatus(Object^ obj, RoutedEventArgs^ args);
	void refreshList(Object^ obj, RoutedEventArgs^ args);
	void saveFiles(Object^ obj, RoutedEventArgs^ args);
	void uploadFiles(Object^ obj, RoutedEventArgs^ args);
	void getdependList(Object^ obj, RoutedEventArgs^ args);
	//add end
    void sendMessage(Object^ obj, RoutedEventArgs^ args);
    void addText(String^ msg);
	std::string parseMsg(std::string& msg);
    void getMessage();
    void clear(Object^ sender, RoutedEventArgs^ args);
    void browseForFolder(Object^ sender, RoutedEventArgs^ args);
    void OnLoaded(Object^ sender, RoutedEventArgs^ args);
    void Unloading(Object^ sender, System::ComponentModel::CancelEventArgs^ args);
  private:
    std::string toStdString(String^ pStr);
    String^ toSystemString(std::string& str);
    void setTextBlockProperties();
    void setButtonsProperties();
	String^ ServerAddr;
	String^ ServerPort;
	String^ LocalAddr;
	String^ LocalPort;
  };
}


#endif
