#ifndef TYPEANALYZE_H
#define TYPEANALYZE_H
/////////////////////////////////////////////////////////////////////
// TypeAnalyze.h - analyze Semi-tok and create type table          //
// ver 1.0                                                         //
// Kangli Chu, Spring 2016                                         //
/////////////////////////////////////////////////////////////////////

#include <queue>
#include <string>
#include <sstream>
#include "../Tokenizer/Tokenizer.h"
#include "../SemiExp/SemiExp.h"
#include "../Parser/MTree.h"
#include "../Parser/MNode.h"
//#include "../Parser/ActionsAndRules.h"

/***********************************************************************
typeTable includes all info about the types and dependency of this file.
***********************************************************************/
struct typeTable
{
	std::string filename = " ";
	std::vector<std::string> types;
	void addTyp(std::string type) {
		types.push_back(type);
	}
	std::vector<std::string> dependency;
	void addDep(std::string dep) {
		for (size_t i = 0; i < dependency.size(); i++)
			if (dep == dependency[i])
				return;
		dependency.push_back(dep);
	}
	void show() {
		std::cout << "\n\nFile: " << filename<<"\n";
		if (dependency.size() == 0) {
			std::cout << "Dependency: " << "NO DEPENDENCY!" << "\n";
			return;
		}
		for (size_t i = 0; i < dependency.size(); i++) {
			std::cout << "Dependency: " << dependency[i] << "\n";
		}
	}
};

///////////////////////////////
template<typename T>
class TypeAnalyze {
public:
	using AST_MNode = MNode < T >;
	using sPtr = std::shared_ptr < AST_MNode >;

	TypeAnalyze(){}
	//get type from semi-expre
	void getType_Declaration(ITokCollection* pTc);
	void getType_Executable(ITokCollection*& pTc);
	void getType_Function(ITokCollection* pTc);
	void getType_Namespace(ITokCollection*& pTc);
	void getType_ClassStruct(ITokCollection*& pTc);
	void getType_Valuelist(ITokCollection*& pTc);
	void getType_Keywords(ITokCollection*& pTc);
	// add types to currTable.
	void addType(std::string type);
	void addFile(std::string file);
	void analyzeDependeny(sPtr root);
	void showDep();
	std::vector<typeTable*> gettable() {
		return typeTables;
	}
private:
	bool stdTypecheck(std::string sample);
	bool checkExist(std::string type);
	bool checkDependency(std::string depend);
	std::vector<typeTable*> typeTables;
	typeTable* currTable = nullptr;
};
template<typename T>
void TypeAnalyze<T>::showDep() {
	for (size_t i = 0; i < typeTables.size(); i++) {
		typeTables[i]->show();
	}
}

//false: not std type
template<typename T>
bool TypeAnalyze<T>::stdTypecheck(std::string sample) {
	std::vector<std::string> stdTypes = { "std","ostringstream","size_t","size",
		"int","float","double","char", "unsigned","signed",")","void", "vector","list", "string"};
	for each (std::string var in stdTypes)
	{
		if (var == sample)
			return true;
	}
	return false;
}
//
template<typename T>
void TypeAnalyze<T>::addFile(std::string file) {
	typeTable* tmp = new typeTable();
	tmp->filename = file;
	typeTables.push_back(tmp);
	currTable = typeTables.back();
}

//true: already have type in the table
//false: type is not exist in the table
template<typename T>
bool TypeAnalyze<T>::checkExist(std::string type) {
	for each (std::string var in currTable->types)
	{
		if (type == var)
			return true;
	}
	return false;
}

//return value: similar to checkExist
template<typename T>
bool TypeAnalyze<T>::checkDependency(std::string depend) {
	for each (std::string var in currTable->dependency)
	{
		if (depend == var)
			return true;
	}
	return false;
}

//parse pTc to get type (std::string type)
template<typename T>
void TypeAnalyze<T>::getType_Declaration(ITokCollection* pTc) {
	size_t found = (*pTc)[0].find("<");
	if (found == std::string::npos)
		addType((*pTc)[0]);
	else {
		std::string tmp;
		for (size_t i = 0; i < found; i++)
			tmp += (*pTc)[0][i];
		addType(tmp);
		tmp.clear();
		for (size_t i = found+1; i < (*pTc)[0].find(">"); i++)
			tmp += (*pTc)[0][i];
		addType(tmp);
	}
}

template<typename T>
void TypeAnalyze<T>::getType_Executable(ITokCollection*& pTc) {
	size_t pos = pTc->find("(");
	std::string str = (*pTc)[pos - 1];
	if (pos != pTc->length() && str != "," && str != "]")
		addType("globalfunc: "+str);
}

template<typename T>
void TypeAnalyze<T>::getType_Function(ITokCollection* pTc) {
	//add return type
	if (pTc->find("<") != 1 && pTc->find("::") != 1)
		addType((*pTc)[0]);
	//add parameters' type 
	size_t pos = pTc->find("(");
	size_t len = pTc->length();
	while (pos != len)
	{
		if (!stdTypecheck((*pTc)[pos + 1]))
		{
			if ((*pTc)[pos + 1] == "const") {
					addType((*pTc)[pos + 2]);
			}else
				addType((*pTc)[pos + 1]);	
		}
		pos = pTc->findFrom(pos+1, ",");
	}
}

template<typename T>
void TypeAnalyze<T>::getType_Namespace(ITokCollection*& pTc) {
	//addType((*pTc)[pTc->find("namespace") + 1]);
}
template<typename T>
void TypeAnalyze<T>::getType_ClassStruct(ITokCollection*& pTc) {
	size_t pos = pTc->find("class");
	if (pos == pTc->length()) {
		pos = pTc->find("struct");
		if (pos == pTc->length())
			pos = pTc->find("enum");
	}
	std::string name = (*pTc)[pos + 1];
	addType(name);
	//inherit from another class
	pos = pTc->find(":");
	if (pos != pTc->length())
		addType((*pTc)[pTc->length() - 2]);
}
template<typename T>
void TypeAnalyze<T>::getType_Valuelist(ITokCollection*& pTc) {
	return;
}
template<typename T>
void TypeAnalyze<T>::getType_Keywords(ITokCollection*& pTc) {

	size_t pos = pTc->find("(");
	size_t len = pTc->length();
	while (pos != len)
	{
		if (!stdTypecheck((*pTc)[pos + 1]))
		{
			if ((*pTc)[pos + 1] == "const") {
				addType("globalfunc: "+(*pTc)[pos + 2]);
			}
			else
				addType("globalfunc: "+(*pTc)[pos + 1]);

		}
		pos = pTc->findFrom(pos + 1, "(");
	}
}

//get type and add in table
template<typename T>
void TypeAnalyze<T>::addType(std::string type) {
	if (type == " " || type == "")
		return;
	if(!checkExist(type) && !stdTypecheck(type))
		currTable->addTyp(type);
}

//add dependency to table
template<typename T>
void TypeAnalyze<T>::analyzeDependeny(sPtr root) {
	for each (typeTable* var in typeTables)
	{
		for each(std::string id in var->types) {
			sPtr child = root->findById(id);
			if (child != nullptr)
				var->addDep(child->value().filename);
		}
	}
}

#endif
