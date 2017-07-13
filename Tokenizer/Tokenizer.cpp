/////////////////////////////////////////////////////////////////////
// Tokenizer.cpp - read words from a std::stream                   //
// ver 3.5                                                         //
// Language:    C++, Visual Studio 2015                            //
// Application: Parser component, CSE687 - Object Oriented Design  //
// Author:		Kangli Chu										   //
// Resource:    Jim Fawcett, Syracuse University, CST 4-187        //
//              jfawcett@twcny.rr.com                              //
/////////////////////////////////////////////////////////////////////
/*
  Helper code that does not attempt to handle all tokenizing
  special cases like escaped characters.
*/
#include "Tokenizer.h"
#include <iostream>
#include <cctype>
#include <string>
#include <vector>
namespace Scanner
{
  class ConsumeState
  {
  public:
    ConsumeState();
    ConsumeState(const ConsumeState&) = delete;
	static void reset() { firstConstructor = true; }
    ConsumeState& operator=(const ConsumeState&) = delete;
    virtual ~ConsumeState();
    void attach(std::istream* pIn) { _pIn = pIn; }
    virtual void eatChars() = 0;
    void consumeChars() {
      _pState->eatChars();
      _pState = nextState();
    }
    bool canRead() { return _pIn->good(); }
    std::string getTok() { return token; }
    bool hasTok() { return token.size() > 0; }
    ConsumeState* nextState();
	static std::vector<std::string> sscLib; 
	static std::vector<std::string> scpLib;
	static bool showComment;
	static size_t lineCount;
	static std::istream* _pIn;
  protected:
	bool isOneCharToken(std::string tok);
	bool isTwoCharToken(std::string tok);
	static bool firstConstructor;
	static bool firstDestructor;
    static std::string token;
    static int prevChar;
    static int currChar;
    static ConsumeState* _pState;
    static ConsumeState* _pEatCppComment;
    static ConsumeState* _pEatCComment;
    static ConsumeState* _pEatWhitespace;
    static ConsumeState* _pEatPunctuator;
    static ConsumeState* _pEatAlphanum;
    static ConsumeState* _pEatNewline;
	static ConsumeState* _pEatQuotes;
  };
}

using namespace Scanner;
std::vector<std::string> ConsumeState::sscLib = {
	"<", ">", "[","]", "(",")", "{","}",
	":", "=", "+", "-", "*", "\n", ".", "_", ","
};
std::vector<std::string> ConsumeState::scpLib = {
	"<<", ">>", "::", "++", "--", "==",
	"+=", "-=", "*=", "/="
};
std::string ConsumeState::token;
std::istream* ConsumeState::_pIn = nullptr;
int ConsumeState::prevChar;
int ConsumeState::currChar;
size_t ConsumeState::lineCount;
bool ConsumeState::showComment = true;
bool ConsumeState::firstConstructor = true;
bool ConsumeState::firstDestructor = true;
ConsumeState* ConsumeState::_pState = nullptr;
ConsumeState* ConsumeState::_pEatCppComment = nullptr;
ConsumeState* ConsumeState::_pEatCComment = nullptr;
ConsumeState* ConsumeState::_pEatWhitespace = nullptr;
ConsumeState* ConsumeState::_pEatPunctuator = nullptr;
ConsumeState* ConsumeState::_pEatAlphanum = nullptr;
ConsumeState* ConsumeState::_pEatNewline;
ConsumeState* ConsumeState::_pEatQuotes = nullptr;

void testLog(const std::string& msg);

ConsumeState* ConsumeState::nextState()
{
  if (!(_pIn->good()))
    return nullptr;
  int chNext = _pIn->peek();
  if (chNext == EOF)
    _pIn->clear();
  if (std::isspace(currChar) && currChar != '\n')
    return _pEatWhitespace;
  if (currChar == '/' && chNext == '/')
    return _pEatCppComment;
  if (currChar == '/' && chNext == '*')
    return _pEatCComment;
  if (currChar == '\n')
  {
	lineCount++;
    return _pEatNewline;
  }
  if (std::isalnum(currChar))
    return _pEatAlphanum;
  if (currChar == '\'' || currChar == '\"') 
	  return _pEatQuotes;
  if (ispunct(currChar))
    return _pEatPunctuator;
 
  if (!_pIn->good())
    return _pEatWhitespace;
  throw(std::logic_error("invalid type"));
}

bool ConsumeState::isOneCharToken(std::string tok) {
	for (size_t i = 0; i < sscLib.size(); i++) {
		if (sscLib[i] == tok)
			return true;
	}
	return false;
}
bool ConsumeState::isTwoCharToken(std::string tok) {
	for (size_t i = 0; i < scpLib.size(); i++) {
		if (scpLib[i] == tok)
			return true;
	}
	return false;
}

class EatWhitespace : public ConsumeState
{
public:
  virtual void eatChars()
  {
    token.clear();
    //std::cout << "\n  eating whitespace";
    do {
      if (!_pIn->good())  // end of stream
        return;
      currChar = _pIn->get();
    } while (std::isspace(currChar) && currChar != '\n');
  }
};

class EatCppComment : public ConsumeState
{
public:
  virtual void eatChars()
  {
    token.clear();
    //std::cout << "\n  eating C++ comment";
    do {
      if (!_pIn->good())  // end of stream
        return;
	  if(showComment)
		token += currChar;
      currChar = _pIn->get();
    } while (currChar != '\n');
  }
};

class EatCComment : public ConsumeState
{
public:
  virtual void eatChars()
  {
    token.clear();
    //std::cout << "\n  eating C comment";
    do {
      if (!_pIn->good())  // end of stream
        return;
	  if(showComment)
		token += currChar;
      currChar = _pIn->get();
	  
    } while (currChar != '*' || _pIn->peek() != '/');
	if(showComment)
		token += currChar;
	currChar = _pIn->get();
	if(showComment)
		token += currChar;
    currChar = _pIn->get();
  }
};

class EatQuotes : public ConsumeState {
public:
	virtual void eatChars() {
		token.clear();
		int caseSel = currChar;
		int prevChar = 0;
		int preprevChar = 0;
		do {
			switch (caseSel)
			{
			case '\'':
				if (prevChar == '\\')
					token += currChar;
				preprevChar = prevChar;
				prevChar = currChar;
				currChar = _pIn->get();
				break;
			case '\"':
				if (prevChar != 0 && (prevChar == '\\' || currChar!='\\'))
					token += currChar;
				preprevChar = prevChar;
				prevChar = currChar;
				currChar = _pIn->get();
				break;
			}
			if((preprevChar=='\\'||prevChar != '\\')&& currChar == caseSel){
				currChar = _pIn->get();
				break;
			}
		} while (true);
	}
};
class EatPunctuator : public ConsumeState
{
public:
  virtual void eatChars()
  {
    token.clear();
	std::string tempOneToken, tempTwoToken;
	tempOneToken.clear();
	tempTwoToken.clear();

	do {
		tempOneToken = currChar;
		tempTwoToken = currChar;
		tempTwoToken += _pIn->peek();
		if (isTwoCharToken(tempTwoToken))
		{
			token += tempTwoToken;
			currChar = _pIn->get();
			currChar = _pIn->get();
			return;
		}
		else if(isOneCharToken(tempOneToken))
		{
			token += tempOneToken;
			currChar = _pIn->get();
			return;
		}
		token += currChar;
		if (!_pIn->good())  // end of stream
			return;
		currChar = _pIn->get();
	} while (ispunct(currChar));
  }
};

class EatAlphanum : public ConsumeState
{
public:
  virtual void eatChars()
  {
    token.clear();
    //std::cout << "\n  eating alphanum";
    do {
      token += currChar;
      if (!_pIn->good())  // end of stream
        return;
      currChar = _pIn->get();
    } while (isalnum(currChar));
  }
};

class EatNewline : public ConsumeState
{
public:
  virtual void eatChars()
  {
    token.clear();
    //std::cout << "\n  eating alphanum";
    token += currChar;
    if (!_pIn->good())  // end of stream
      return;
    currChar = _pIn->get();
  }
};

	//static int test = 0;
	//test++;
	//std::cout << test;
ConsumeState::ConsumeState()
{

  if (firstConstructor)
  {
	  lineCount = 1;
	 firstConstructor = false;
    _pEatAlphanum = new EatAlphanum();
    _pEatCComment = new EatCComment();
    _pEatCppComment = new EatCppComment();
    _pEatPunctuator = new EatPunctuator();
    _pEatWhitespace = new EatWhitespace();
    _pEatNewline = new EatNewline();
	_pEatQuotes = new EatQuotes();
    _pState = _pEatWhitespace;
  }
}

ConsumeState::~ConsumeState()
{

  if (firstDestructor)
  {
	firstDestructor = false;
    delete _pEatAlphanum;
    delete _pEatCComment;
    delete _pEatCppComment;
    delete _pEatPunctuator;
    delete _pEatWhitespace;
    delete _pEatNewline;
	delete _pEatQuotes;
  }
}

Toker::Toker() : pConsumer(new EatWhitespace()) { reset(); }

Toker::~Toker() { delete pConsumer; }

bool Toker::attach(std::istream* pIn)
{
  if (pIn != nullptr && pIn->good())
  {
    pConsumer->attach(pIn);
    return true;
  }
  return false;
}
size_t Toker::currentLineCount() {
	return pConsumer->lineCount;
}
void Toker::reset()
{ ConsumeState::reset(); }

bool Toker::setSpecialSingleChars(std::string ssc) {
	for (size_t i = 0; i < pConsumer->sscLib.size(); i++) {
		if (pConsumer->sscLib[i] == ssc)
			return false;
	}
	pConsumer->sscLib.push_back(ssc);
	return true;
}

bool Toker::setSpecialCharPairs(std::string scp) {
	for (size_t i = 0; i < pConsumer->scpLib.size(); i++) {
		if (pConsumer->scpLib[i] == scp)
			return false;
	}
	pConsumer->scpLib.push_back(scp);
	return true;
}
bool Toker::rmvSpecialSingleChars(std::string ssc) {
	for (size_t i = 0; i < pConsumer->sscLib.size(); i++) {
		if (pConsumer->sscLib[i] == ssc)
		{
			pConsumer->sscLib.erase(pConsumer->sscLib.begin() + i);
			return true;
		}
	}
	return false;
}
bool Toker::rmvSpecialCharPairs(std::string scp) {
	for (size_t i = 0; i < pConsumer->scpLib.size(); i++) {
		if (pConsumer->scpLib[i] == scp)
		{
			pConsumer->scpLib.erase(pConsumer->scpLib.begin() + i);
			return true;
		}
	}
	return false;
}
void Toker::showComm(bool show) {
	pConsumer->showComment = show;
}

int Toker::getNextChar() {
	return pConsumer->_pIn->peek();
}

std::string Toker::getTok()
{
  while(true) 
  {
    if (!pConsumer->canRead())
      return "";
    pConsumer->consumeChars();
    if (pConsumer->hasTok())
      break;
  }
  return pConsumer->getTok();
}

bool Toker::canRead() { return pConsumer->canRead(); }

void testLog(const std::string& msg)
{
#ifdef TEST_LOG
  std::cout << "\n  " << msg;
#endif
}

//----< test stub >--------------------------------------------------

#ifdef TEST_TOKENIZER

#include <fstream>

int main()
{

  std::string fileSpec = "../Test adding special pairs case.txt";
  //std::string fileSpec = "../case test.txt";

  std::ifstream in(fileSpec);
  if (!in.good())
  {
    std::cout << "\n  can't open " << fileSpec << "\n\n";
    return 1;
  }
  Toker toker;
  toker.attach(&in);

  //add new special single char here
//	toker.setSpecialSingleChars("singlestring");
  //add new special pair string here
//	toker.setSpecialCharPairs("pairtring");
  toker.setSpecialCharPairs("->");
  do
  {
    std::string tok = toker.getTok();
    if (tok == "\n")
      tok = "newline";
    std::cout << "\n -- " << tok;
  } while (in.good());

  std::cout << "\n\n";
  return 0;
}
#endif
