///////////////////////////////////////////////////////////////////////////
// Window.cpp - C++\CLI implementation of WPF Application                //
//          - Demo for CSE 687 Project #4                                //
// ver 2.0                                                               //
// Author Kangli Chu, CSE687 - Object Oriented Design, Spring 2015             //
// Source: Fawcett Jim
///////////////////////////////////////////////////////////////////////////
/*
*  To run as a Windows Application:
*  - Set Project Properties > Linker > System > Subsystem to Windows
*  - Comment out int main(...) at bottom
*  - Uncomment int _stdcall WinMain() at bottom
*  To run as a Console Application:
*  - Set Project Properties > Linker > System > Subsytem to Console
*  - Uncomment int main(...) at bottom
*  - Comment out int _stdcall WinMain() at bottom
*/
#include "Window.h"
using namespace CppCliWindows;

WPFCppCliDemo::WPFCppCliDemo()
{// set up channel
  ObjectFactory* pObjFact = new ObjectFactory;
  pSendr_ = pObjFact->createSendr();
  pRecvr_ = pObjFact->createRecvr();
  pChann_ = pObjFact->createMockChannel(pSendr_, pRecvr_);
  delete pObjFact;
  recvThread = gcnew Thread(gcnew ThreadStart(this, &WPFCppCliDemo::getMessage));
  recvThread->Start();
  this->Loaded += gcnew System::Windows::RoutedEventHandler(this, &WPFCppCliDemo::OnLoaded);
  this->Closing += gcnew CancelEventHandler(this, &WPFCppCliDemo::Unloading);
  setActionMsg();
  setActionMain();
  setActionCon();
  //set control and view
  setCtrlView();
}
void WPFCppCliDemo::setCtrlView() {
	// set Window properties
	this->Title = "Client GUI";
	this->Width = 800;
	this->Height = 800;
	// attach dock panel to Window
	this->Content = hDockPanel;
	hDockPanel->Children->Add(hStatusBar);
	hDockPanel->SetDock(hStatusBar, Dock::Bottom);
	hDockPanel->Children->Add(hGrid);
	// setup Window controls and views
	setUpTabControl();
	setUpStatusBar();
	setUpSendMessageView();
	setUpFileListView();
	setUpConnectionView();
}
void WPFCppCliDemo::setActionMsg() {
	hSendButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::sendMessage);
	hClearButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::clear);
}
void WPFCppCliDemo::setActionMain() {
	hRefreshButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::refreshList);
	hSaveButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::saveFiles);
	hFolderBrowseButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::browseForFolder);
	hUploadButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::uploadFiles);
	hGetDepButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::getdependList);
}
void WPFCppCliDemo::setActionCon() {
	hConnectButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::connectServer);
	hDisconnectButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::disconnectServer);
	hClearStatusButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::clearConnectStatus);
}

WPFCppCliDemo::~WPFCppCliDemo()
{
  delete pChann_;
  delete pSendr_;
  delete pRecvr_;
}

void WPFCppCliDemo::setUpStatusBar()
{
  hStatusBar->Items->Add(hStatusBarItem);
  hStatus->Text = "very important messages will appear here";
  //status->FontWeight = FontWeights::Bold;
  hStatusBarItem->Content = hStatus;
  hStatusBar->Padding = Thickness(10, 2, 10, 2);
}

void WPFCppCliDemo::setUpTabControl()
{
  hGrid->Children->Add(hTabControl);
  hSendMessageTab->Header = "Send Message";
  hFileListTab->Header = "File List";
  hConnectTab->Header = "Connect";
  hTestTab->Header = "Test Only";
  hTabControl->Items->Add(hSendMessageTab);
  hTabControl->Items->Add(hFileListTab);
  hTabControl->Items->Add(hConnectTab);
 // hTabControl->Items->Add(hTestTab);
}

void WPFCppCliDemo::setTextBlockProperties()
{
  RowDefinition^ hRow1Def = gcnew RowDefinition();
  hSendMessageGrid->RowDefinitions->Add(hRow1Def);
  Border^ hBorder1 = gcnew Border();
  hBorder1->BorderThickness = Thickness(1);
  hBorder1->BorderBrush = Brushes::Black;
  hBorder1->Child = hTextBlock1;
  hTextBlock1->Padding = Thickness(15);
  hTextBlock1->Text = "";
  hTextBlock1->FontFamily = gcnew Windows::Media::FontFamily("Tahoma");
  hTextBlock1->FontWeight = FontWeights::Bold;
  hTextBlock1->FontSize = 16;
  hScrollViewer1->VerticalScrollBarVisibility = ScrollBarVisibility::Auto;
  hScrollViewer1->Content = hBorder1;
  hSendMessageGrid->SetRow(hScrollViewer1, 0);
  hSendMessageGrid->Children->Add(hScrollViewer1);
}

void WPFCppCliDemo::setButtonsProperties()
{
  RowDefinition^ hRow2Def = gcnew RowDefinition();
  hRow2Def->Height = GridLength(75);
  hSendMessageGrid->RowDefinitions->Add(hRow2Def);
  //Button Send Message
  hSendButton->Content = "Send Message";
  Border^ hBorder2 = gcnew Border();
  hBorder2->Width = 120;
  hBorder2->Height = 30;
  hBorder2->BorderThickness = Thickness(1);
  hBorder2->BorderBrush = Brushes::Black;
  hBorder2->Child = hSendButton;
  //Button Clear
  hClearButton->Content = "Clear";
  Border^ hBorder3 = gcnew Border();
  hBorder3->Width = 120;
  hBorder3->Height = 30;
  hBorder3->BorderThickness = Thickness(1);
  hBorder3->BorderBrush = Brushes::Black;
  hBorder3->Child = hClearButton;

  hStackPanel1->Children->Add(hBorder2);
  TextBlock^ hSpacer = gcnew TextBlock();
  hSpacer->Width = 20;
  hStackPanel1->Children->Add(hSpacer);
  hStackPanel1->Children->Add(hBorder3);
  hStackPanel1->Orientation = Orientation::Horizontal;
  hStackPanel1->HorizontalAlignment = System::Windows::HorizontalAlignment::Center;
  hSendMessageGrid->SetRow(hStackPanel1, 1);
  hSendMessageGrid->Children->Add(hStackPanel1);
}

void WPFCppCliDemo::connectServer(Object^ obj, RoutedEventArgs^ args) {
	hServerAddr->IsEnabled = false;
	hServerPort->IsEnabled = false;
	hLocalAddr->IsEnabled = false;
	hLocalPort->IsEnabled = false;
	ServerAddr = hServerAddr->Text;
	ServerPort = hServerPort->Text;
	LocalAddr = hLocalAddr->Text;
	LocalPort = hLocalPort->Text;
	pChann_->start(toStdString(ServerAddr), std::stoi(toStdString(ServerPort)), toStdString(LocalAddr), std::stoi(toStdString(LocalPort)));
	hConnectButton->IsEnabled = false;
	hDisconnectButton->IsEnabled = true;
	//"CONNECT "/"DISCONNECT " + "NULL " + "CLIENT_ADDR " + CLIENT ADDRESS + " " + "CLIENT_PORT " + CLIENT PORT
	String^ req = "CONNECT ";
	req += "NULL ";
	req += "CLIENT_ADDR:";
	req += LocalAddr;
	req += " ";
	req += "CLIENT_PORT:";
	req += LocalPort;
	pSendr_->postMessage(toStdString(req));
	Console::Write("\n  Connect to server...\n");
	hStatus->Text = "Connect to server...";
}
//"CONNECT "/"DISCONNECT " + "NULL " + "CLIENT_ADDR " + CLIENT ADDRESS + " " + "CLIENT_PORT " + CLIENT PORT
void WPFCppCliDemo::disconnectServer(Object^ obj, RoutedEventArgs^ args) {
	//pChann_->stop();
	hServerAddr->IsEnabled = true;
	hServerPort->IsEnabled = true;
	hLocalAddr->IsEnabled = true;
	hLocalPort->IsEnabled = true;
	hConnectButton->IsEnabled = true;
	hDisconnectButton->IsEnabled = false;
	String^ req = "DISCONNECT ";
	req += "NULL ";
	req += "CLIENT_ADDR:";
	req += LocalAddr;
	req += " CLIENT_PORT:";
	req += LocalPort;
	pSendr_->postMessage(toStdString(req));
	Console::Write("\n  Disconnect from server...\n");
	hStatus->Text = "Disconnect from server...";
}
void WPFCppCliDemo::clearConnectStatus(Object^ obj, RoutedEventArgs^ args) {
	hConnStatus->Clear();
}

void WPFCppCliDemo::refreshList(Object^ obj, RoutedEventArgs^ args) {
	//request filelist from server: "GETLIST " + "ALLFILES " + "CLIENT_ADDR " + CLIENT ADDRESS + " " + "CLIENT_PORT " + CLIENT PORT
	String^ req="GETLIST ALLFILES ";
	req += "CLIENT_ADDR:";
	req += LocalAddr;
	req += " ";
	req += "CLIENT_PORT:";
	req += LocalPort;
	pSendr_->postMessage(toStdString(req));
	Console::Write("\n  request file list from server...\n");
	hStatus->Text = "Request server file list...";
}
void WPFCppCliDemo::saveFiles(Object^ obj, RoutedEventArgs^ args) {
	//request to save file to client : "SAVE " + FILENAME + " " + "CLIENT_ADDR " + CLIENT ADDRESS + " " + "CLIENT_PORT " + CLIENT PORT
	String^ req = "SAVE ";
	if (!hServerFileList->SelectedItem)
		return;
	String^ sel = hServerFileList->SelectedItem->ToString();
	req += "<";
	req += sel;
	req += " > ";
	req += "CLIENT_ADDR:";
	req += LocalAddr;
	req += " ";
	req += "CLIENT_PORT:";
	req += LocalPort;
	pSendr_->postMessage(toStdString(req));
	Console::Write("\n  request download file and dependency from server...\n");
	hStatus->Text = "Save file to local...";
}
//get depend list of file in server, for selected file
void WPFCppCliDemo::getdependList(Object^ obj, RoutedEventArgs^ args) {
	String^ req = "GETDEP ";
	if (!hServerFileList->SelectedItem)
		return;
	String^ sel = hServerFileList->SelectedItem->ToString();
	req += "<";
	req += sel;
	req += " > ";
	req += "CLIENT_ADDR:";
	req += LocalAddr;
	req += " ";
	req += "CLIENT_PORT:";
	req += LocalPort;
	pSendr_->postMessage(toStdString(req));
	Console::Write("\n  request dependency list from server...\n");
	hStatus->Text = "Get depend list from server...";
}
void WPFCppCliDemo::uploadFiles(Object^ obj, RoutedEventArgs^ args) {
	//request to upload file to server: "UPLOAD "+ FILENAME + "CLIENT_ADDR " + CLIENT ADDRESS + "CLIENT_PORT " + CLIENT PORT
	String^ req = "UPLOAD ";
	if (!hListBox->SelectedItem)
		return;
	String^ sel = hListBox->SelectedItem->ToString();
	req += "<";
	req += sel;
	req += " > ";
	req += "CLIENT_ADDR:";
	req += LocalAddr;
	req += " ";
	req += "CLIENT_PORT:";
	req += LocalPort;
	pSendr_->postMessage(toStdString(req));
	Console::Write("\n  request download file and dependency from server...\n");
	hStatus->Text = "Save file to local...";
}

void WPFCppCliDemo::setUpSendMessageView()
{
  Console::Write("\n  setting up sendMessage view");
  hSendMessageGrid->Margin = Thickness(20);
  hSendMessageTab->Content = hSendMessageGrid;

  setTextBlockProperties();
  setButtonsProperties();
}

std::string WPFCppCliDemo::toStdString(String^ pStr)
{
  std::string dst;
  for (int i = 0; i < pStr->Length; ++i)
    dst += (char)pStr[i];
  return dst;
}

void WPFCppCliDemo::sendMessage(Object^ obj, RoutedEventArgs^ args)
{
  pSendr_->postMessage(toStdString(msgText));
  Console::Write("\n  sent message");
  hStatus->Text = "Sent message";
}

String^ WPFCppCliDemo::toSystemString(std::string& str)
{
  StringBuilder^ pStr = gcnew StringBuilder();
  for (size_t i = 0; i < str.size(); ++i)
    pStr->Append((Char)str[i]);
  return pStr->ToString();
}

void WPFCppCliDemo::addText(String^ msg)
{
	std::string Msg = toStdString(msg);
	std::string comMsg = Msg.substr(0, Msg.find(" "));
	std::string subMsg = Msg.substr(Msg.find(" ") + 1);
	if (comMsg == "CONNECTION") {
		hConnStatus->Text += msg + "\n\n";
		return;
	}
	else if (comMsg == "DEPENDLIST") {
		hDependList->Items->Clear();
		std::string item = parseMsg(subMsg);
		while (item != " ") {
			hDependList->Items->Add(toSystemString(item));
			item = parseMsg(subMsg);
		}
		return;
	}
	else if (comMsg == "FILELIST") {
		hServerFileList->Items->Clear();
		std::string item = parseMsg(subMsg);
		while (item != " ") {
			hServerFileList->Items->Add(toSystemString(item));
			item = parseMsg(subMsg);
		}
		return;
	}
	hTextBlock1->Text += msg + "\n\n";
}
std::string WPFCppCliDemo::parseMsg(std::string& msg)
{
	size_t pos1 = msg.find("<");
	size_t pos2 = msg.find(" >");
	if (pos1 == std::string::npos)
		return " ";
	std::string item = msg.substr(pos1+1,pos2-1);
	//item = item.substr(item.find(">"));
	if (msg.find("<") != std::string::npos) {
		msg = msg.substr(msg.find(">")+1);
	}
	else return " ";
	return item;
}


void WPFCppCliDemo::getMessage()
{
  // recvThread runs this function
  while (true)
  {
    std::cout << "\n  receive thread calling getMessage()";
    std::string msg = pRecvr_->getMessage();
    String^ sMsg = toSystemString(msg);
    array<String^>^ args = gcnew array<String^>(1);
    args[0] = sMsg;

    Action<String^>^ act = gcnew Action<String^>(this, &WPFCppCliDemo::addText);
    Dispatcher->Invoke(act, args);  // must call addText on main UI thread
  }
}

void WPFCppCliDemo::clear(Object^ sender, RoutedEventArgs^ args)
{
  Console::Write("\n  cleared message text");
  hStatus->Text = "Cleared message";
  hTextBlock1->Text = "";
}

void WPFCppCliDemo::setUpFileListView()
{
  Console::Write("\n  setting up FileList view");
  hFileListGrid->Margin = Thickness(10);
  hFileListTab->Content = hFileListGrid;
   
  Grid^ childGrid = gcnew Grid();
  Border^ parent = gcnew Border();
  parent->BorderThickness = Thickness(1);
  parent->BorderBrush = Brushes::Black;
  parent->Child = childGrid;

  hServerDir->Text = "Server Directory";
  Border^ hBorder2 = gcnew Border();
  hBorder2->BorderThickness = Thickness(1);
  hBorder2->BorderBrush = Brushes::Black;
  hBorder2->Child = hServerFileList;

  hDependName->Text = "Dependency List";
  Border^ hBorder3 = gcnew Border(); 
  hBorder3->BorderThickness = Thickness(1);
  hBorder3->BorderBrush = Brushes::Black;
  hBorder3->Child = hDependList;

  Grid^ grandChildGrid1 = gcnew Grid();
  Grid^ grandChildGrid2 = gcnew Grid();
  RowDefinition^ hRowG11 = gcnew RowDefinition();
  hRowG11->Height = GridLength(20);
  RowDefinition^ hRowG12 = gcnew RowDefinition();
  RowDefinition^ hRowG21 = gcnew RowDefinition();
  hRowG21->Height = GridLength(20);
  RowDefinition^ hRowG22 = gcnew RowDefinition();
  grandChildGrid1->RowDefinitions->Add(hRowG11);
  grandChildGrid1->RowDefinitions->Add(hRowG12);
  grandChildGrid2->RowDefinitions->Add(hRowG21);
  grandChildGrid2->RowDefinitions->Add(hRowG22);
  grandChildGrid1->SetRow(hServerDir, 0);
  grandChildGrid1->SetRow(hBorder2, 1);
  grandChildGrid1->Children->Add(hServerDir);
  grandChildGrid1->Children->Add(hBorder2);
  grandChildGrid2->SetRow(hDependName, 0);
  grandChildGrid2->SetRow(hBorder3, 1);
  grandChildGrid2->Children->Add(hDependName);
  grandChildGrid2->Children->Add(hBorder3);

  ColumnDefinition^ hCol1 = gcnew ColumnDefinition();
  ColumnDefinition^ hCol2 = gcnew ColumnDefinition();
  childGrid->ColumnDefinitions->Add(hCol1);
  
  childGrid->ColumnDefinitions->Add(hCol2);
  childGrid->SetColumn(grandChildGrid1, 0);//(hStackPanelV1, 0);
  childGrid->Children->Add(grandChildGrid1);//(hStackPanelV1);
  childGrid->SetColumn(grandChildGrid2, 1);//(hStackPanelV2, 1);
  childGrid->Children->Add(grandChildGrid2);//(hStackPanelV2);

  RowDefinition^ hRowDef1 = gcnew RowDefinition(); 
  hFileListGrid->RowDefinitions->Add(hRowDef1);
  hFileListGrid->SetRow(parent, 0);
  hFileListGrid->Children->Add(parent);

  StackPanel^ hStackPanelH = gcnew StackPanel();
  hStackPanelH->Orientation = Orientation::Horizontal;
  hStackPanelH->HorizontalAlignment = System::Windows::HorizontalAlignment::Center;
  RowDefinition^ hRowDef2 = gcnew RowDefinition();
  hRowDef2->Height = GridLength(75);
  hFileListGrid->RowDefinitions->Add(hRowDef2);
  hRefreshButton->Content = "Refresh List";
  hRefreshButton->Height = 30;
  hRefreshButton->Width = 120;
  hRefreshButton->BorderThickness = Thickness(2);
  hRefreshButton->BorderBrush = Brushes::Black;

  hSaveButton->Content = "Save Seleted File&Dependency";
  hSaveButton->Height = 30;
  hSaveButton->Width = 180;
  hSaveButton->BorderThickness = Thickness(2);
  hSaveButton->BorderBrush = Brushes::Black;
  hGetDepButton->Content = "Get Dependency";
  hGetDepButton->Height = 30;
  hGetDepButton->Width = 180;
  hGetDepButton->BorderThickness = Thickness(2);
  hGetDepButton->BorderBrush = Brushes::Black;
  TextBlock^ hSpacer = gcnew TextBlock();
  hSpacer->Width = 20; //hGetDepButton
  TextBlock^ hSpacerb = gcnew TextBlock();
  hSpacerb->Width = 20;
  hStackPanelH->Children->Add(hRefreshButton);
  hStackPanelH->Children->Add(hSpacer);
  hStackPanelH->Children->Add(hGetDepButton);
  hStackPanelH->Children->Add(hSpacerb);
  hStackPanelH->Children->Add(hSaveButton);
  hFileListGrid->SetRow(hStackPanelH, 1);
  hFileListGrid->Children->Add(hStackPanelH);

  //selected files for sending to server
  Grid^ childGrid2 = gcnew Grid();
  Border^ parent2 = gcnew Border();
  parent2->BorderThickness = Thickness(1);
  parent2->BorderBrush = Brushes::Black;
  parent2->Child = childGrid2;

  RowDefinition^ hRow1Def = gcnew RowDefinition();
  hRow1Def->Height = GridLength(20);
  RowDefinition^ hRow2Def = gcnew RowDefinition();
  childGrid2->RowDefinitions->Add(hRow1Def);
  childGrid2->RowDefinitions->Add(hRow2Def);
  TextBlock^ hDesSelect = gcnew TextBlock();
  hDesSelect->Text = "Select file for uploading to Server";
  Border^ hBorder1 = gcnew Border();
  hBorder1->BorderThickness = Thickness(1);
  hBorder1->BorderBrush = Brushes::Black;
  hBorder1->Child = hListBox;

  childGrid2->SetRow(hDesSelect, 0);
  childGrid2->Children->Add(hDesSelect);
  childGrid2->SetRow(hBorder1, 1);
  childGrid2->Children->Add(hBorder1);

  RowDefinition^ hRow3Def = gcnew RowDefinition();
  hFileListGrid->RowDefinitions->Add(hRow3Def);
  hFileListGrid->SetRow(parent2, 2);
  hFileListGrid->Children->Add(parent2);

  StackPanel^ hStackPanelH2 = gcnew StackPanel();
  hStackPanelH2->Orientation = Orientation::Horizontal;
  hStackPanelH2->HorizontalAlignment = System::Windows::HorizontalAlignment::Center;

  RowDefinition^ hRow4Def = gcnew RowDefinition();
  hRow4Def->Height = GridLength(75);
  hFileListGrid->RowDefinitions->Add(hRow4Def);
  hFolderBrowseButton->Content = "Select Directory";
  hFolderBrowseButton->Height = 30;
  hFolderBrowseButton->Width = 120;
  hFolderBrowseButton->BorderThickness = Thickness(2);
  hFolderBrowseButton->BorderBrush = Brushes::Black;
  hUploadButton->Content = "Upload";
  hUploadButton->Height = 30;
  hUploadButton->Width = 120;
  hUploadButton->BorderThickness = Thickness(2);
  hUploadButton->BorderBrush = Brushes::Black;
  TextBlock^ hSpacer2 = gcnew TextBlock();
  hSpacer2->Width = 20;
  hStackPanelH2->Children->Add(hFolderBrowseButton);
  hStackPanelH2->Children->Add(hSpacer2);
  hStackPanelH2->Children->Add(hUploadButton);

  hFileListGrid->SetRow(hStackPanelH2, 3);
  hFileListGrid->Children->Add(hStackPanelH2);

  hFolderBrowserDialog->ShowNewFolderButton = false;
  hFolderBrowserDialog->SelectedPath = System::IO::Directory::GetCurrentDirectory();
}

void WPFCppCliDemo::browseForFolder(Object^ sender, RoutedEventArgs^ args)
{
  std::cout << "\n  Browsing for folder";
  hListBox->Items->Clear();
  System::Windows::Forms::DialogResult result;
  result = hFolderBrowserDialog->ShowDialog();
  if (result == System::Windows::Forms::DialogResult::OK)
  {
    String^ path = hFolderBrowserDialog->SelectedPath;
    std::cout << "\n  opening folder \"" << toStdString(path) << "\"";
    array<String^>^ files = System::IO::Directory::GetFiles(path, L"*.*");
    for (int i = 0; i < files->Length; ++i)
      hListBox->Items->Add(files[i]);
    array<String^>^ dirs = System::IO::Directory::GetDirectories(path);
    for (int i = 0; i < dirs->Length; ++i)
      hListBox->Items->Add(L"<> " + dirs[i]);
  }
}
void WPFCppCliDemo::setUpConnectionView()
{
  Console::Write("\n  setting up Connection view");
  hConnectGrid->Margin = Thickness(20);
  hConnectTab->Content = hConnectGrid;

  StackPanel^ hStackPanelP = gcnew StackPanel();
  hStackPanelP->Orientation = Orientation::Horizontal;
  hStackPanelP->HorizontalAlignment = System::Windows::HorizontalAlignment::Stretch;
  StackPanel^ hStackPanelV1 = gcnew StackPanel();
  hStackPanelV1->Orientation = Orientation::Vertical;
  hStackPanelV1->HorizontalAlignment = System::Windows::HorizontalAlignment::Left;
  StackPanel^ hStackPanelV2 = gcnew StackPanel();
  hStackPanelV2->Orientation = Orientation::Vertical;
  hStackPanelV2->HorizontalAlignment = System::Windows::HorizontalAlignment::Left;

  TextBlock^ localAddr = gcnew TextBlock();
  TextBlock^ localPort = gcnew TextBlock();
  TextBlock^ serverAddr = gcnew TextBlock();
  TextBlock^ serverPort = gcnew TextBlock();
  TextBlock^ space1 = gcnew TextBlock();
  TextBlock^ space2 = gcnew TextBlock();
  TextBlock^ space3 = gcnew TextBlock();
  TextBlock^ space4 = gcnew TextBlock();
  space1->Height = 30;
  space2->Height = 30;
  space3->Height = 30;
  space4->Height = 30;
  localAddr->Text = "Local Address";
  localPort->Text = "Port";
  serverAddr->Text = "Server Address";
  serverPort->Text = "Port";

  hServerAddr->Width = 250;
  hServerAddr->Text = "localhost";
  hLocalAddr->Width = 250;
  hLocalAddr->Text = "localhost";
  
  hServerPort->Width = 100;
  hServerPort->Text = "8080";
  hLocalPort->Width = 100;
  hLocalPort->Text = "8081";

  hConnectButton->Content = "Connect";
  hDisconnectButton->Content = "Disconnect";
  hConnectButton->Height = 30;
  hConnectButton->Width = 90;
  hConnectButton->BorderThickness = Thickness(2);
  hConnectButton->BorderBrush = Brushes::Black;
  hDisconnectButton->Height = 30;
  hDisconnectButton->Width = 90;
  hDisconnectButton->BorderThickness = Thickness(2);
  hDisconnectButton->BorderBrush = Brushes::Black;

  hStackPanelV1->Children->Add(localAddr);
  hStackPanelV1->Children->Add(hLocalAddr);
  hStackPanelV1->Children->Add(space1);
  hStackPanelV1->Children->Add(serverAddr);
  hStackPanelV1->Children->Add(hServerAddr);
  hStackPanelV1->Children->Add(space3);
  hStackPanelV1->Children->Add(hConnectButton);
  
  hStackPanelV2->Children->Add(localPort);
  hStackPanelV2->Children->Add(hLocalPort);
  hStackPanelV2->Children->Add(space2);
  hStackPanelV2->Children->Add(serverPort);
  hStackPanelV2->Children->Add(hServerPort);
  hStackPanelV2->Children->Add(space4);
  hStackPanelV2->Children->Add(hDisconnectButton);
  hDisconnectButton->IsEnabled = false;

  TextBlock^ spaceP = gcnew TextBlock();
  spaceP->Width = 50;
  hStackPanelP->Children->Add(hStackPanelV1);
  hStackPanelP->Children->Add(spaceP);
  hStackPanelP->Children->Add(hStackPanelV2);

  RowDefinition^ hRowDef0 = gcnew RowDefinition();
  hConnectGrid->RowDefinitions->Add(hRowDef0);
  hConnectGrid->SetRow(hStackPanelP, 0);
  hConnectGrid->Children->Add(hStackPanelP);

  TextBlock^ statusDes = gcnew TextBlock();
  statusDes->Text = "Connection Status";

  RowDefinition^ hRowDef1 = gcnew RowDefinition();
  hRowDef1->Height = GridLength(20);
  hConnectGrid->RowDefinitions->Add(hRowDef1);
  RowDefinition^ hRowDef2 = gcnew RowDefinition();
  hConnectGrid->RowDefinitions->Add(hRowDef2);
  hConnectGrid->SetRow(statusDes, 1);
  hConnectGrid->Children->Add(statusDes);
  hConnectGrid->SetRow(hConnStatus, 2);
  hConnectGrid->Children->Add(hConnStatus);

  hClearStatusButton->Content = "Clear Status";
  hClearStatusButton->Height = 30;
  hClearStatusButton->Width = 120;
  hClearStatusButton->BorderThickness = Thickness(2);
  hClearStatusButton->BorderBrush = Brushes::Black;
  RowDefinition^ hRowDef3 = gcnew RowDefinition();
  hRowDef3->Height = GridLength(80);
  hConnectGrid->RowDefinitions->Add(hRowDef3);
  hConnectGrid->SetRow(hClearStatusButton, 3);
  hConnectGrid->Children->Add(hClearStatusButton);
}

void WPFCppCliDemo::OnLoaded(Object^ sender, RoutedEventArgs^ args)
{
  Console::Write("\n  Window loaded");
}
void WPFCppCliDemo::Unloading(Object^ sender, System::ComponentModel::CancelEventArgs^ args)
{
  Console::Write("\n  Window closing");
}

[STAThread]
//int _stdcall WinMain()
int main(array<System::String^>^ args)
{
  Console::WriteLine(L"\n Starting WPFCppCliDemo");

  Application^ app = gcnew Application();
  app->Run(gcnew WPFCppCliDemo());
  Console::WriteLine(L"\n\n");
}