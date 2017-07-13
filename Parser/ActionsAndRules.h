#ifndef ACTIONSANDRULES_H
#define ACTIONSANDRULES_H
/////////////////////////////////////////////////////////////////////
//  ActionsAndRules.h - declares new parsing rules and actions     //
//  ver 2.2                                                        //
//  Language:      Visual C++ 2008, SP1                            //
//  Platform:      Dell Precision T7400, Vista Ultimate SP1        //
//  Application:   Prototype for CSE687 Pr1, Sp09                  //
//  Author:		   Kangli Chu
//  Source:        Jim Fawcett, CST 4-187, Syracuse University     //
//                 (315) 443-3948, jfawcett@twcny.rr.com           //
/////////////////////////////////////////////////////////////////////
/*
  Module Operations: 
  ==================
  This module defines several action classes.  Its classes provide 
  specialized services needed for specific applications.  The modules
  Parser, SemiExpression, and Tokenizer, are intended to be reusable
  without change.  This module provides a place to put extensions of
  these facilities and is not expected to be reusable. 

  Public Interface:
  =================
  Toker t(someFile);              // create tokenizer instance
  SemiExp se(&t);                 // create a SemiExp attached to tokenizer
  Parser parser(se);              // now we have a parser
  Rule1 r1;                       // create instance of a derived Rule class
  Action1 a1;                     // create a derived action
  r1.addAction(&a1);              // register action with the rule
  parser.addRule(&r1);            // register rule with parser
  while(se.getSemiExp())          // get semi-expression
    parser.parse();               //   and parse it

  Build Process:
  ==============
  Required files
    - Parser.h, Parser.cpp, ScopeStack.h, ScopeStack.cpp,
      ActionsAndRules.h, ActionsAndRules.cpp, ConfigureParser.cpp,
      ItokCollection.h, SemiExpression.h, SemiExpression.cpp, tokenizer.h, tokenizer.cpp
  Build commands (either one)
    - devenv CodeAnalysis.sln
    - cl /EHsc /DTEST_PARSER parser.cpp ActionsAndRules.cpp \
         semiexpression.cpp tokenizer.cpp /link setargv.obj

  Maintenance History:
  ====================
  ver 2.3 : 05 April 16
  - add enum definition and action
  -
  ver 2.2 : 18 Mar 16
  - add rules and actions
  - add AST tree.
  ver 2.1 : 15 Feb 16
  - small functional change to a few of the actions changes display strategy
  - preface the (new) Toker and SemiExp with Scanner namespace
  ver 2.0 : 01 Jun 11
  - added processing on way to building strong code analyzer
  ver 1.1 : 17 Jan 09
  - changed to accept a pointer to interfaced ITokCollection instead
    of a SemiExpression
  ver 1.0 : 12 Jan 06
  - first release

*/
//
#include <queue>
#include <string>
#include <sstream>
#include "Parser.h"
#include "../SemiExp/itokcollection.h"
#include "../ScopeStack/ScopeStack.h"
#include "../Tokenizer/Tokenizer.h"
#include "../SemiExp/SemiExp.h"
#include "../TypeAnalyze/TypeAnalyze.h"
#include "MTree.h"

///////////////////////////////////////////////////////////////
// ScopeStack element is application specific
/* ToDo:
 * - chanage lineCount to two fields: lineCountStart and lineCountEnd
 */
struct element
{
  std::string type = "unknown";
  std::string name = "anonymous";
  std::string filename = " ";
  size_t lineCount;
  size_t lineStart;
  size_t lineEnd;
  size_t complexity = 1;
  std::string show()
  {
    std::ostringstream temp;
    temp << "( Type: " << type << ", Name: " << name;
	if (type == "function")
		temp << ", complexity: " << complexity;
    temp << ", start at "<<lineStart<< ", end at "<<lineEnd << ", \n	"<< "		file: " << filename<< " )\n";
    return temp.str();
  }
};

///////////////////////////////////////////////////////////////
// Repository instance is used to share resources
// among all actions.
/*
 * ToDo:
 * - add AST Node class
 * - provide field to hold root of AST
 * - provide interface to access AST
 * - provide helper functions to build AST, perhaps in Tree class
 */	
using AST_MNode = MNode < element > ;
using sPtr = std::shared_ptr < AST_MNode >;
class Repository  // application specific
{
  ScopeStack<sPtr> nodeStack;
  ScopeStack<element> stack;//stack is a List<element>
  Scanner::Toker* p_Toker;
public:
	
  Repository(Scanner::Toker* pToker)
  {
    p_Toker = pToker;
	root_element.name = "Global";
	root_element.type = "Namespace";
	root_element.lineCount = 0;
	pRoot = sPtr(new AST_MNode(root_element));
	pRoot->id() = "ROOT_NODE";
	pCurr = pRoot;
	nodeStack.push(pRoot);//initial ASTStack
	typeAnalyzer = new TypeAnalyze<element>();
  }
  Repository()
  {
	  root_element.name = "Global";
	  root_element.type = "Namespace";
	  root_element.lineCount = 0;
	  pRoot = sPtr(new AST_MNode(root_element));
	  pRoot->id() = "ROOT_NODE";
	  pCurr = pRoot;
	  nodeStack.push(pRoot);//initial ASTStack
	  typeAnalyzer = new TypeAnalyze<element>();
  }
  void setFile(std::string file) {
	  fs=file;
	  typeAnalyzer->addFile(fs);
  }
  void showDepend() {
	  typeAnalyzer->analyzeDependeny(pRoot);
	  //typeAnalyzer->showDep();
  }
  std::vector<typeTable*> getDepend() {
	  return typeAnalyzer->gettable();
  }
  void createAST() {
	  MTree<element> ASTtmp(pRoot);
	  AST = ASTtmp;
  }
  void showAST() {
	  std::cout << "Create and Show content in AST tree:\n\n";
	  AST.show();
  }
  void initialTokForRepos(Scanner::Toker* pToker) {
	  p_Toker = pToker;
  }
  ScopeStack<element>& scopeStack()
  {
    return stack;
  }
  ScopeStack<sPtr>& ASTStack()
  {
	  return nodeStack;
  }
  Scanner::Toker* Toker()
  {
    return p_Toker;
  }
  size_t lineCount()
  {
    return (size_t)(p_Toker->currentLineCount());
  }
  
  void addChildTo(std::string parentId, sPtr pChild) {
	  sPtr pTarget = pRoot->findById(parentId);
	  if (pTarget != nullptr)
		  pTarget->addChild(pChild);
	  else
		  pRoot->addChild(pChild);//add to root node
  }
  sPtr addChildTo(sPtr parentNode, sPtr pChild) {
	  parentNode->addChild(pChild);
	  return pChild;
  }
  void reset_pCurr(sPtr pRef) {
	  pCurr = pRoot->findById(pRef->id());
	  if (pCurr != nullptr && pCurr->value().type == "Namespace")
		  return;
	  else
		  pCurr = pRoot;
  }
  void addNamespace(sPtr& child) {
	  if (pRoot->findById(child->id()) == nullptr)
		  pRoot->addChild(child);
  }
  sPtr set_pCurrToId(std::string Id, sPtr& parent) {
	  if (parent == nullptr)
		  parent = pRoot;
	  if (Id == "")
		  pCurr = parent;
	  else {
		  pCurr = parent->findById(Id);
		  if (pCurr == nullptr) {
			  pCurr = sPtr(new AST_MNode(element()));
			  pCurr->id() = Id;
			  //pCurr->value().
			  parent->addChild(pCurr);
			  return pCurr;
		  }
	  }
	  return pCurr;
  }
  void addChildToCurr(sPtr pChild) {
	  sPtr child = pCurr->findById(pChild->id());
	  if (child != nullptr)
	  {
		  child->value() = pChild->value();
		  return;
	  }
	  pCurr->addChild(pChild);
  }
  void addChildToRoot(sPtr pChild) {
	  pRoot->addChild(pChild);
  }
  sPtr findById(std::string Id) {
	  return pRoot->findById(Id);
  }
  std::string fs;
  TypeAnalyze<element>* typeAnalyzer;
private:
	element root_element;
	MTree<element> AST;
	sPtr pRoot = nullptr;
	sPtr pCurr = nullptr;
};

///////////////////////////////////////////////////////////////
// rule to detect beginning of anonymous scope

class BeginningOfScope : public IRule
{
public:
  bool doTest(ITokCollection*& pTc)
  {
    if(pTc->find("{") < pTc->length())
    {
      doActions(pTc);
      return true;
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to handle scope stack at end of scope

class HandlePush : public IAction
{
  Repository* p_Repos;
public:
  HandlePush(Repository* pRepos)
  {
    p_Repos = pRepos;
  }
  void doAction(ITokCollection*& pTc)
  {
    element elem;
    elem.lineCount = p_Repos->lineCount();
	elem.lineStart = elem.lineCount;
	elem.filename = p_Repos->fs;
    p_Repos->scopeStack().push(elem);
	sPtr pNode = sPtr(new AST_MNode(elem));
	p_Repos->ASTStack().push(pNode);
  }
};

///////////////////////////////////////////////////////////////
// rule to detect end of scope

class EndOfScope : public IRule
{
public:
  bool doTest(ITokCollection*& pTc)
  {
    if(pTc->find("}") < pTc->length())
    {
      doActions(pTc);
      return true;
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to handle scope stack at end of scope

class HandlePop : public IAction
{
  Repository* p_Repos;
public:
  HandlePop(Repository* pRepos)
  {
    p_Repos = pRepos;
  }
  void doAction(ITokCollection*& pTc)
  {
    if(p_Repos->scopeStack().size() == 0)
      return;
    element elem = p_Repos->scopeStack().pop();
	elem.lineEnd = p_Repos->lineCount();
	elem.lineCount = p_Repos->lineCount() - elem.lineCount + 1;
	sPtr pNode = p_Repos->ASTStack().pop();
	pNode->value().lineEnd= elem.lineEnd;
	pNode->value().lineCount=elem.lineCount;
	pNode->value().filename = p_Repos->fs;
	if (elem.type == "function") {
		pNode->value().complexity = calComplexity(pNode);
	}
  }
private:
	size_t calComplexity(sPtr node) {
		size_t count = 1;
		std::vector<sPtr> children = node->nodeChildren();
		for (size_t i = 0; i < children.size(); i++) {
			count += calComplexity(children[i]);
		}
		return count;
	}
};

///////////////////////////////////////////////////////////////
// rule to detect preprocessor statements
class PreprocStatement : public IRule
{
public:
  bool doTest(ITokCollection*& pTc)
  {
    if(pTc->find("#") < pTc->length())
    {
      doActions(pTc);
      return true;
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to print preprocessor statement to console

class PrintPreproc : public IAction
{
public:
  void doAction(ITokCollection*& pTc)
  {
    //std::cout << "\n\n  Preproc Stmt: " << pTc->show().c_str();
  }
};

///////////////////////////////////////////////////////////////
// rule to detect keywords: try, do, else
class SpecialKeyWordsDefinition2 : public IRule
{
public:
	bool isSpecialKeyWord2(ITokCollection*& pTc)
	{
		const static std::string keys[]
			= { "do", "else", "try" };
		for (int i = 0; i<3; ++i)
			if (pTc->find(keys[i])<pTc->length())
				return true;
		return false;
	}
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			size_t len = tc.find("(");
			if (len == tc.length() && isSpecialKeyWord2(pTc))
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};
///////////////////////////////////////////////////////////////
// action to add specialkeyword node to AST leaves

class PushSpecialKeyWord2 : public IAction
{
	Repository* p_Repos;
public:
	std::string getSpecialKeyWord2(ITokCollection*& pTc)
	{
		const static std::string keys[]
			= { "do", "else", "try" };
		for (int i = 0; i<3; ++i)
			if (pTc->find(keys[i])<pTc->length())
				return keys[i];
		return " ";
	}
	PushSpecialKeyWord2(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		size_t len = tc.find("{");
		element elem = p_Repos->scopeStack().pop();
		sPtr node = p_Repos->ASTStack().pop();
		sPtr nodeP = p_Repos->ASTStack().pop();
		elem.type = "SpecialKeyWord";
		elem.name = getSpecialKeyWord2(pTc);
		elem.lineCount = p_Repos->lineCount();
		//no id for keywords. because they are not unique
		node->value().type = "SpecialKeyWord";
		node->value().name = elem.name;
		node->value().lineCount = elem.lineCount;
		node->value().filename = elem.filename;
		p_Repos->addChildTo(nodeP, node);
		p_Repos->scopeStack().push(elem);
		p_Repos->ASTStack().push(nodeP);
		p_Repos->ASTStack().push(node);
	}
};

///////////////////////////////////////////////////////////////
// rule to detect keywords: for while switch if catch
class SpecialKeyWordsDefinition : public IRule
{
public:
	bool isSpecialKeyWord(const std::string& tok)
	{
		const static std::string keys[]
			= { "for", "while", "switch", "if", "catch" };
		for (int i = 0; i<5; ++i)
			if (tok == keys[i])
				return true;
		return false;
	}
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			size_t len = tc.find("(");
			if (len < tc.length() && isSpecialKeyWord(tc[len - 1]))
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};

///////////////////////////////////////////////////////////////
// action to add specialkeyword node to AST leaves

class PushSpecialKeyWord : public IAction
{
	Repository* p_Repos;
public:
	PushSpecialKeyWord(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		p_Repos->typeAnalyzer->getType_Keywords(pTc);
		ITokCollection& tc = *pTc;
		size_t len = tc.find("(");
		element elem = p_Repos->scopeStack().pop();
		sPtr node = p_Repos->ASTStack().pop();
		sPtr nodeP = p_Repos->ASTStack().pop();
		elem.type = "SpecialKeyWord";
		elem.name = tc[len-1];
		elem.lineCount = p_Repos->lineCount();
//no id for keywords. because they are not unique
		node->value().type = "SpecialKeyWord";
		node->value().name = elem.name;
		node->value().lineCount = elem.lineCount;
		node->value().filename = elem.filename;
		p_Repos->addChildTo(nodeP, node);
		p_Repos->scopeStack().push(elem);
		p_Repos->ASTStack().push(nodeP);
		p_Repos->ASTStack().push(node);
	}
};

///////////////////////////////////////////////////////////////
// rule to detect keywords: for while switch if catch
class ClassDefinition : public IRule
{
public:
	bool isClassStrutKeyWord(ITokCollection*& pTc)
	{
		if (pTc->find("class") < pTc->length() || pTc->find("struct") < pTc->length() || pTc->find("enum") < pTc->length())
			return true;
		return false;
	}
	void condenseTemplateTypes(ITokCollection& tc)
	{
		if (tc.find("template") == tc.length())
			return;
		size_t start = tc.find("<");
		size_t end = tc.find(">");
		if (start >= end || start == 0)
			return;
		std::string tok = tc[start - 1];
		for (size_t i = start; i < end + 1; ++i)
			tok += tc[i];//tok = template<...>
		for (size_t i = start; i < end + 1; ++i)
			tc.remove(start);//remove template<...>
		tc[start - 1] = tok; //set the 1st tok as template<...>
	}
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			condenseTemplateTypes(*pTc);
			if (isClassStrutKeyWord(pTc))
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};

///////////////////////////////////////////////////////////////
// action to add class/struct node to AST leaves
class PushClassStruct : public IAction
{
	Repository* p_Repos;
public:
	PushClassStruct(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc){
		p_Repos->typeAnalyzer->getType_ClassStruct(pTc);
		element elem = p_Repos->scopeStack().pop();
		sPtr node = p_Repos->ASTStack().pop();
		sPtr nodeP = p_Repos->ASTStack().pop();
		std::string type = "class";
		size_t pos = pTc->find("class");
		if (pos == pTc->length()) {
			pos = pTc->find("struct");
			if (pos == pTc->length())
			{
				type = "enum";
				pos = pTc->find("enum");
			}
			else
				type = "struct";
		}
		std::string name = (*pTc)[pos + 1];
		elem.type = type;
		elem.name = name;
		elem.lineCount = p_Repos->lineCount();
		node->id() = name;
		node->value().type = type;
		node->value().name = name;
		node->value().lineCount = elem.lineCount;
		node->value().filename = elem.filename;
		p_Repos->scopeStack().push(elem);
		p_Repos->reset_pCurr(nodeP);
		p_Repos->addChildToCurr(node);
		p_Repos->ASTStack().push(nodeP);
		p_Repos->ASTStack().push(node);
	}
};
//////////////////////////////////////////////////////////
//rule to detect namespace
class NamespaceDefinition : public IRule {
public:
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		size_t par = tc.length() - 1;
		if (tc[par] == "{")
		{
			size_t len = tc.find("namespace");
			if (len!=tc.length())
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};
//////////////////////////////////////////////////////////
//action to push namespace in stack and add node
class PushNamespace : public IAction {
	Repository* p_Repos;
public:
	PushNamespace(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc) {
		p_Repos->typeAnalyzer->getType_Namespace(pTc);
		element elem = p_Repos->scopeStack().pop();
		sPtr node = p_Repos->ASTStack().pop();
		elem.name = (*pTc)[pTc->find("namespace")+1];
		elem.type = "Namespace";
		elem.lineCount = p_Repos->lineCount();
		node->id() = elem.name;
		node->value().type = "Namespace";
		node->value().name = elem.name;
		node->value().lineCount = elem.lineCount;
		node->value().filename = elem.filename;
		p_Repos->addNamespace(node);
		p_Repos->scopeStack().push(elem);
		p_Repos->ASTStack().push(node);
	}
};

///////////////////////////////////////////////////////////////
//rule to detect value list definition
class ValuelistDefinition : public IRule 
{
public:
	bool doTest(ITokCollection*& pTc) {
		size_t cnt = pTc->find("{");
		if (((*pTc)[cnt - 1] == "=") && (cnt!=pTc->length())) {
			doActions(pTc);
			return true;
		}
		return true;
	}
};

class PushValuelist : public IAction
{
	Repository* p_Repos;
	std::string classname;
public:
	PushValuelist(Repository* pRepos) {
		p_Repos = pRepos;
		classname = " ";
	}
	std::string composition(ITokCollection*& pTc, size_t s, size_t e) {
		std::string str;
		for (size_t i = s + 1; i < e; i++) {
			str += (*pTc)[i];
		}
		return str;
	}
	std::string getName(ITokCollection*& pTc){
		size_t equal = pTc->findFromBack("=");
		size_t comm = pTc->findFromBack("::");
		size_t angl = pTc->findFromBack(">");
		size_t end = pTc->length();
		if (comm == end) {
			if (angl == end)
				return composition(pTc, 0, equal);
			else
				return composition(pTc, angl, equal);
		}
		else {
			if (angl == end)
				return composition(pTc, comm, equal);
			else if (comm > angl) {
				classname = composition(pTc, angl, comm);
				return composition(pTc, comm, equal);
			}
			else
				return composition(pTc, angl, equal);
		}
	}
	void doAction(ITokCollection*& pTc) {
		element elem = p_Repos->scopeStack().pop();
		sPtr node = p_Repos->ASTStack().pop();
		elem.name = getName(pTc);
		elem.type = "Valuelist";
		elem.lineCount = p_Repos->lineCount();
		node->id() = elem.name;
		node->value().type = "Valuelist";
		node->value().name = elem.name;
		node->value().lineCount = elem.lineCount;
		node->value().filename = elem.filename;
		if (classname != " ") {
			p_Repos->typeAnalyzer->addType(classname);
			sPtr nodeP = p_Repos->ASTStack().pop();
			p_Repos->set_pCurrToId(classname, nodeP);
			p_Repos->ASTStack().push(nodeP);			
		}else
			p_Repos->typeAnalyzer->addType(elem.name);
		p_Repos->addChildToCurr(node);
		p_Repos->scopeStack().push(elem);
		p_Repos->ASTStack().push(node);
	}
};

///////////////////////////////////////////////////////////////
// rule to detect function definitions
class FunctionDefinition : public IRule
{
public:
	bool isClassStrutKeyWord(ITokCollection*& pTc)
	{
		if (pTc->find("class") < pTc->length() || pTc->find("struct") < pTc->length())
			return true;
		return false;
	}
  bool isSpecialKeyWord(const std::string& tok)
  {
    const static std::string keys[]
      = { "for", "while", "switch", "if", "catch" };
    for(int i=0; i<5; ++i)
      if(tok == keys[i])
        return true;
    return false;
  }
  bool doTest(ITokCollection*& pTc)
  {
    ITokCollection& tc = *pTc;
    if(tc[tc.length()-1] == "{")
    {
      size_t len = tc.find("(");
	  size_t len2 = tc.find(":");
	  if (len2==tc.length() && len < tc.length() && !isSpecialKeyWord(tc[len - 1]))// && !isClassStrutKeyWord(pTc)
	  {
		  doActions(pTc);
		  return true;
	  }
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to push function name onto ScopeStack

class PushFunction : public IAction
{
  Repository* p_Repos;
  std::string trueFuncname;
public:
  PushFunction(Repository* pRepos)
  {//Repository* p_Repos;
    p_Repos = pRepos;
  }
  void removeModifier(ITokCollection& tc) {
	  const static std::string keys[] = {
		  "const", "extern", "static","inline",
		  "public", "protected", "private"
	  };
	  size_t rec;
	  for (size_t i = 0; i < 7; i++) {
		  rec = tc.find(keys[i]);
		  if (rec = !tc.length())
			  tc.remove(rec);
	  }
	  while (true) {
		  rec = tc.find("std");
		  if (rec < tc.find("(")) {
			  tc.remove(rec);
			  tc.remove(tc.find("::"));
		  }
		  else return;
	  }
  }

  void condenseTemplateTypes(ITokCollection& tc)
  {
	  if (tc.find("template") == tc.length())
		  return;
	  size_t start = tc.find("<");
	  size_t end = tc.find(">");
	  if (start >= end || start == 0)
		  return;
	  std::string tok = tc[start - 1];
	  for (size_t i = start; i < end + 1; ++i)
		  tok += tc[i];//tok = template<...>
	  for (size_t i = start-1; i < end + 1; ++i)
		  tc.remove(start-1);//remove template<...>
	//  tc[start - 1] = tok; //set the 1st tok as template<...>
  }
  //get name of class
  std::string getClassName(ITokCollection& tc) {
	  std::string classname="";
	  size_t par = tc.find("(");
	  size_t col = tc.find("::");
	  if (col < par && tc[col - 1] != ">")
		  classname = tc[col - 1];
	  else if (col < par && tc[col - 1] == ">") {
		  size_t cnt = 1;
		  for (size_t i = col - 2; i > 0; i--) {
			  if (tc[i] == ">")
				  cnt++;
			  if (tc[i] == "<")
				  cnt--;
			  if (cnt == 0) {
				  classname = tc[i - 1];
				  break;
			  }
		  }
	  }
	  return classname;
  }
  //get name of function, include content in ()
  std::string getfunc(ITokCollection& tc) {
	  size_t len = tc.find("operator");
	  size_t start = tc.find("(");
	  size_t end = tc.find(")");
	  std::string op = tc[start - 1];
	  trueFuncname = op;
	  if (len == tc.length()) {
		  for (size_t i = start; i < end+1; i++)
			  op += tc[i];
		  if (tc[start - 2] == "~")
			  op = "~" + op;
	  }
	  else {
		  op = "operator";
		  for (size_t i = len + 1; i < end + 1; i++)
			  op += tc[i];
	  }
	  std::string type = tc[0];
	  size_t checkPos1 = tc.find("::");
	  size_t checkPos2 = tc.find("<");
	  if (checkPos1 == 1 || checkPos2 == 1)
		  return op;
	  return type + " " + op;
  }
  void doAction(ITokCollection*& pTc)
  {
	  ITokCollection& in = *pTc;
	  Scanner::SemiExp tc;
	  for (size_t i = 0; i<in.length(); ++i)
		  tc.push_back(in[i]);
	  condenseTemplateTypes(tc);
	  removeModifier(tc);
	  //use class name as the id of node
	  std::string classId = getClassName(tc);
	  p_Repos->typeAnalyzer->addType(classId);
	  p_Repos->typeAnalyzer->getType_Function(&tc);
	element elem = p_Repos->scopeStack().pop();
	std::string name = getfunc(tc);//include (...)
    elem.type = "function";
    elem.name = name;
    elem.lineCount = p_Repos->lineCount();
    p_Repos->scopeStack().push(elem);
	sPtr node = p_Repos->ASTStack().pop();
	sPtr nodeP = p_Repos->ASTStack().pop();
	std::string globalflag="func: ";
	std::string nodePtype = nodeP->value().type;
	if (classId == "" && nodePtype != "class" && nodePtype != "struct" && nodePtype != "enum")
		globalflag = "globalfunc: ";
	node->value().type = "function";
	node->value().name = name;
	node->value().lineCount = elem.lineCount;
	node->value().filename = elem.filename;
	if (pTc->find("main") < pTc->length()) //here we do not set id for main function because we have a lot of main functions
		p_Repos->addChildToRoot(node);
	else {
		node->id() = globalflag+ trueFuncname;
		p_Repos->set_pCurrToId(classId, nodeP);
		p_Repos->addChildToCurr(node);
	}
	p_Repos->ASTStack().push(nodeP);
	p_Repos->ASTStack().push(node);
  }
};

///////////////////////////////////////////////////////////////
// action to send semi-expression that starts a function def
// to console

class PrintFunction : public IAction
{
  Repository* p_Repos;
public:
  PrintFunction(Repository* pRepos)
  {
    p_Repos = pRepos;
  }
  void doAction(ITokCollection*& pTc)
  {
	  ;// std::cout << "\n  FuncDef: " << pTc->show().c_str();
  }
};

///////////////////////////////////////////////////////////////
// action to send signature of a function def to console

class PrettyPrintFunction : public IAction
{
public:
  void doAction(ITokCollection*& pTc)
  {
	  ;
    //pTc->remove("public");
    //pTc->remove(":");
    //pTc->trimFront();
    //size_t len = pTc->find(")");
    //std::cout << "\n\n  Pretty Stmt:    ";
    //for(size_t i=0; i<len+1; ++i)
    //  std::cout << (*pTc)[i] << " ";
    //std::cout << "\n";
  }
};

///////////////////////////////////////////////////////////////
// rule to detect declaration

class Declaration : public IRule          
{   
	Repository* p_Repos;					
public:
	Declaration(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
  bool isModifier(const std::string& tok) 
  {                                       
    const size_t numKeys = 20;           
    const static std::string keys[numKeys] = {
      "const", "extern", "friend", "mutable", "signed", "static",
      "typedef", "typename", "unsigned", "volatile", "&", "*", "std", "::",
      "public", "protected", "private", ":", "++", "--"
    };
    for (int i = 0; i<numKeys; ++i)
    if (tok == keys[i])
      return true;
    return false;
  }
  void condenseTemplateTypes(ITokCollection& tc)
  {
    size_t start = tc.find("<");
    size_t end = tc.find(">");
    if (start >= end || start == 0)
      return;
    else
    {
      if (end == tc.length())
        end = tc.find(">::");
      if (end == tc.length())
        return;
    }
    std::string save = tc[end];
    std::string tok = tc[start - 1];
    for (size_t i = start; i < end + 1; ++i)
      tok += tc[i];
    for (size_t i = start; i < end + 1; ++i)
      tc.remove(start);//remove template<...>
    if (save == ">::")
    {
      tok += tc[start + 1];
      tc.remove(start);//?
    }
    tc[start - 1] = tok;
    //std::cout << "\n  -- " << tc.show();
  }
  bool isSpecialKeyWord(const std::string& tok)
  {
    const static std::string keys[]
      = { "for", "while", "switch", "if", "catch" };
    for (int i = 0; i<5; ++i)
    if (tok == keys[i])
      return true;
    return false;
  }
  void removeInvocationParens(ITokCollection& tc)
  {
    size_t start = tc.find("(");
    size_t end = tc.find(")");
    if (start >= end || end == tc.length() || start == 0)
      return;
    if (isSpecialKeyWord(tc[start - 1]))
      return;
    for (size_t i = start; i < end + 1; ++i)
      tc.remove(start);
    //std::cout << "\n  -- " << tc.show();
  }
  void condenseOperatorType(ITokCollection& tc) {
	  size_t start = tc.find("operator");
	  size_t end = tc.find("(");
	  if (start == tc.length())
		  return;
	  std::string tok = "operator";
	  for (size_t i = start + 1; i < end; ++i)
		  tok += tc[i];
	  for (size_t i = start+1; i < end; ++i)
		  tc.remove(start+1);//remove template<...>
	  tc[start] = tok;
  }
  void condensesizet(ITokCollection& tc) {
	  size_t pos = tc.find("size");
	  if (pos != tc.length()) {
		  if (tc[pos + 1] == "_"&&tc[pos + 2] == "t") {
			  tc.remove(pos + 1);
			  tc.remove(pos + 1);
			  tc[pos] = "size_t";
		  }
	  }
  }
  bool doTest(ITokCollection*& pTc)
  {
    ITokCollection& in = *pTc;
    Scanner::SemiExp tc;
    for (size_t i = 0; i<in.length(); ++i)
      tc.push_back(in[i]);
    if (tc[tc.length() - 1] == ";" && tc.length() > 2)
    {
	  condenseOperatorType(tc);
      removeInvocationParens(tc);
      condenseTemplateTypes(tc);
	  condensesizet(tc);
      Scanner::SemiExp se;
      for (size_t i = 0; i < tc.length(); ++i)
      {
        if (isModifier(tc[i]))
          continue;
        if (se.isComment(tc[i]) || tc[i] == "\n" || tc[i] == "return")
          continue;
        if (tc[i] == "=" || tc[i] == ";")
          break;
        else
          se.push_back(tc[i]);
      }
      if (se.length() == 2)  // type & name, so declaration
      {
		if (se.find("using") != 2)
			p_Repos->typeAnalyzer->addType(se[1]);
		else
			p_Repos->typeAnalyzer->getType_Declaration(&tc);
        doActions(pTc);
        return true;
      }
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to show declaration
class ShowDeclaration : public IAction
{
public:
  void doAction(ITokCollection*& pTc)
  {
    ITokCollection& tc = *pTc;
    // remove comments
    Scanner::SemiExp se;
    for (size_t i = 0; i<tc.length(); ++i)
      if (!se.isComment(tc[i]))
        se.push_back(tc[i]);
    // show cleaned semiExp
   // std::cout << "\n  Declaration: " << se.show();
  }
};

///////////////////////////////////////////////////////////////
// rule to detect executable

class Executable : public IRule           // declar ends in semicolon
{                                         // has type, name, modifiers &
public:                                   // initializers.  So eliminate
  bool isModifier(const std::string& tok) // modifiers and initializers.
  {                                       // If you have two things left
    const size_t numKeys = 20;            // its declar else executable.
    const static std::string keys[numKeys] = {
      "const", "extern", "friend", "mutable", "signed", "static",
      "typedef", "typename", "unsigned", "volatile", "&", "*", "std", "::",
      "public", "protected", "private", ":", "++", "--"
    };
    for (int i = 0; i<numKeys; ++i)
    if (tok == keys[i])
      return true;
    return false;
  }
  void condenseTemplateTypes(ITokCollection& tc)
  {
    size_t start = tc.find("<");
    size_t end = tc.find(">");
    if (start >= end || start == 0)
      return;
    else
    {
      if (end == tc.length())
        end = tc.find(">::");
      if (end == tc.length())
        return;
    }
    std::string save = tc[end];
    std::string tok = tc[start - 1];
    for (size_t i = start; i < end+1; ++i)
      tok += tc[i];
    for (size_t i = start; i < end+1; ++i)
      tc.remove(start);
    if (save == ">::")
    {
      tok += tc[start + 1];
      tc.remove(start);
    }
    tc[start - 1] = tok;
    //std::cout << "\n  -- " << tc.show();
  }
  void condenseOperatorType(ITokCollection& tc) {
	  size_t start = tc.find("operator");
	  size_t end = tc.find("(");
	  if (start == tc.length())
		  return;
	  std::string tok = "operator";
	  for (size_t i = start + 1; i < end; ++i)
		  tok += tc[i];
	  for (size_t i = start + 1; i < end; ++i)
		  tc.remove(start + 1);//remove template<...>
	  tc[start] = tok;
  }
  bool isSpecialKeyWord(const std::string& tok)
  {
    const static std::string keys[]
      = { "for", "while", "switch", "if", "catch" };
    for (int i = 0; i<5; ++i)
    if (tok == keys[i])
      return true;
    return false;
  }
  void removeInvocationParens(ITokCollection& tc)
  {
    size_t start = tc.find("(");
    size_t end = tc.find(")");
    if (start >= end || end == tc.length() || start == 0)
      return;
    if (isSpecialKeyWord(tc[start - 1]))
      return;
    for (size_t i = start; i < end + 1; ++i)
      tc.remove(start);
    //std::cout << "\n  -- " << tc.show();
  }
  void condensesizet(ITokCollection& tc) {
	  size_t pos = tc.find("size");
	  if (pos != tc.length()) {
		  if (tc[pos + 1] == "_"&&tc[pos + 2] == "t") {
			  tc.remove(pos + 1);
			  tc.remove(pos + 1);
			  tc[pos] = "size_t";
		  }
	  }
  }
  bool doTest(ITokCollection*& pTc)
  {
    ITokCollection& in = *pTc;
    Scanner::SemiExp tc;
    for (size_t i = 0; i<in.length(); ++i)
      tc.push_back(in[i]);
    if (tc[tc.length() - 1] == ";" && tc.length() > 2){
	  condenseOperatorType(tc);
      removeInvocationParens(tc);
      condenseTemplateTypes(tc);
	  condensesizet(tc);
      Scanner::SemiExp se;
      for (size_t i = 0; i < tc.length(); ++i){
        if (isModifier(tc[i]))
          continue;
        if (se.isComment(tc[i]) || tc[i] == "\n" || tc[i] == "return")
          continue;
        if (tc[i] == "=" || tc[i] == ";")
          break;
        else
          se.push_back(tc[i]);
      }
      if (se.length() != 2) {
        doActions(pTc);
        return true;
      }
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to show executable

class ShowExecutable : public IAction
{
	Repository* p_Repos;
public:
	ShowExecutable(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	bool checkKeywords(ITokCollection*& pTc) {
		const static std::string keys[]
			= { "for", "while", "switch", "if", "catch" };
		for each (std::string key in keys)
		{
			if (pTc->find(key) != pTc->length())
				return true;
		}
		return false;
	}
  void doAction(ITokCollection*& pTc)
  {
	  if (!checkKeywords(pTc))
		  p_Repos->typeAnalyzer->getType_Executable(pTc);
	  else
		  p_Repos->typeAnalyzer->getType_Keywords(pTc);
    ITokCollection& tc = *pTc;
    // remove comments
    Scanner::SemiExp se;
    for (size_t i = 0; i < tc.length(); ++i)
    {
      if (!se.isComment(tc[i]))
        se.push_back(tc[i]);
    }
    // show cleaned semiExp
	//std::cout << "\n  Executable: "<<se.show();
  }
};

#endif
