#ifndef METRICSEXECUTIVE_H
#define METRICSEXECUTIVE_H
/////////////////////////////////////////////////////////////////////
// MetricsExecutive.h - parse file                                 //
// ver 1.0                                                         //
// Kangli Chu, Spring 2016                                         //
/////////////////////////////////////////////////////////////////////
#include "../Parser/Parser.h"
#include "../Parser/ActionsAndRules.h"
#include "../Parser/ConfigureParser.h"
#include "../Parser/MTree.h"
#include "../QueuedWorkItems/QueuedWorkItems.h"
#include "../BlockingQueue/Cpp11-BlockingQueue.h"
//#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
using WorkResult = std::string;
class MetricsExe {
public:
	using WorkResult = std::string;
	MetricsExe(Repository* pRepos) { 
		this->pRepos = pRepos; 
	}
	void config(BlockingQueue<WorkResult>* pQueue, ProcessWorkItem<WorkResult>* process) {
		fileQ = pQueue;
		pProc = process;
	}
	~MetricsExe() {}
	void start();
	void wait();
	std::string getInputFromUser();
	//virtual bool setStopSign() { ; }
private:
	ProcessWorkItem<WorkResult>* pProc;
	bool parseFile(std::string filep);
	Repository* pRepos;
	BlockingQueue<WorkResult>* fileQ;
	std::thread* _pThread;
};


#endif
