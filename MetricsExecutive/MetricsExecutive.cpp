/////////////////////////////////////////////////////////////////////
// MetricsExecutive.cpp - test hub and program entrance            //
//										                           //
// ver 1.0                                                         //
//  Author:		   Kangli Chu
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include "../Utilities/Utilities.h"
#include "../Tokenizer/Tokenizer.h"
#include "../SemiExp/SemiExp.h"
#include "../Parser/Parser.h"
#include "../Parser/ActionsAndRules.h"
#include "../Parser/ConfigureParser.h"
#include "../Parser/MTree.h"
#include "../DataStore/DataStore.h"
#include "../FileMgr/FileMgr.h"
#include "../FileMgr/FileSystem.h"
#include "MetricsExecutive.h"
using namespace Scanner;
using namespace Utilities;


	bool MetricsExe::parseFile(std::string filep) {
	ConfigParseToConsole configure;
	configure.configRepos(pRepos, filep);
	Parser* pParser = configure.Build();
	try
	{
		if (pParser)
		{
			if (!configure.Attach(filep))// attach file to pToker (Toker)
			{
				std::cout << "\n  could not open file " << filep << std::endl;
				return false;
			}
		}
		else
		{
			std::cout << "\n\n  Parser not built\n\n";
			return false;
		}
		while (pParser->next())
			pParser->parse();
		std::cout << "\n";
		return true;
	}
	catch (std::exception& ex)
	{
		std::cout << "\n\n    " << ex.what() << "\n\n";
	}
	return true;
}
void MetricsExe::start() {
	
	std::function<void()> monitorQ = [&]() {
		while (true) {
			std::string container = fileQ->deQ();
			WorkItem<WorkResult>* pThr = new WorkItem<WorkResult>([&, container]() {
				parseFile(container);
				return " ";
			});
			if (container != "\n") {
				pProc->doWork(pThr);
			}
			else {
				pProc->doWork(nullptr);
				return;
			}
		}
	};
	_pThread =  new std::thread(monitorQ);
}

void MetricsExe::wait() {
	_pThread->join();
}
std::string MetricsExe::getInputFromUser() {
	std::string input;
	std::cout << "===============================================\n";
	std::cout << "Repository: ../Repository_server\n\n";
	//std::cout << "Please input your project location: \n";
	//std::cout << "Example1: " << "D:\\Work\\Courses\\CSE687 OOD\\Project1NonStatic -- "<<"Parse files in Prject1NonStatic folder."<<"\n\n";
	//std::cout << "Example2:" << ".. -- " << "Parse files in this project folder." << "\n\n";
	//getline(std::cin,input);
	//std::cout << "Your input: " << input << "\n";
	//return input;
	return "../Repository_server";
}

#ifdef TEST_MetricsExe
#include <queue>
#define Util StringHelper

int main()
{
	/*************************/
	DataStore ds;
	DataStore filePath;
	BlockingQueue<WorkResult> fileQueue;
	ProcessWorkItem<WorkResult> processor;
	processor.start();
	static Repository* pRepos= new Repository();
	MetricsExe exe(pRepos);
	exe.config(&fileQueue, &processor);
	exe.start();
	FileMgr fm(exe.getInputFromUser(), ds, filePath);
	fm.config(&fileQueue);
	fm.addPattern("*.h");
	fm.addPattern("*.c");
	fm.addPattern("*.cpp");
	fm.search();
	exe.wait();
	processor.wait();
	/**************************/
	Util::Title("Testing Project #3: Parallel Dependency Analysis", '=');
	putline();
	std::cout << "==================================================\n\n";
	pRepos->createAST();
	//pRepos->showAST();
	pRepos->showDepend();
	return 1;
}
#endif
