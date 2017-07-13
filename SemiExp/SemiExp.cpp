///////////////////////////////////////////////////////////////////////
// SemiExpression.cpp - collect tokens for analysis                  //
// ver 3.2                                                           //
// Language:    C++, Visual Studio 2015                              //
// Application: Parser component, CSE687 - Object Oriented Design    //
// Author:		Kangli Chu											 //
// Resource:    Jim Fawcett, Syracuse University, CST 4-187          //
//              jfawcett@twcny.rr.com                                //
///////////////////////////////////////////////////////////////////////
/**********************************************************************
* ver 3.2: 
*	1. implement all method in ItokCollection
*	2. add special cases
**********************************************************************/

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <exception>
#include <locale>
#include "SemiExp.h"
#include "../Tokenizer/Tokenizer.h"

using namespace Scanner;

SemiExp::SemiExp(Toker* pToker) : _pToker(pToker) {}

//true by default
bool SemiExp::get(bool clear)
{
	if (_pToker == nullptr)
		throw(std::logic_error("no Toker reference"));
	if(clear)
		_tokens.clear();
	
	specialDelRule();//delete '\n' and add ";" in the special string vector
	while (true)
	{
		std::string token = _pToker->getTok();
		if (token == "")
			break;
		if(!isComment(token))//if (token.find("/*") != 0 && token.find("//") != 0)
		{
			_tokens.push_back(token);
			if (specialAddRule(token, _tokens))
				return true;
		}
	}
	return false;
}
bool SemiExp::isComment(const std::string& tok) {
	if (tok.find("/*") == 0 || tok.find("//") == 0)
	{
		return true;
	}
	return false;
}
//size_t leftb = 0;
//size_t rightb = 0;
//for (size_t i = 0; i < tokens.size(); i++) {
//	if (tokens[i] == "(")
//		leftb++;
//	else if (tokens[i] == ")")
//		rightb++;
//}
//if ((leftb == rightb) && (leftb != 0) && (token != ")")) {
//	if (_pToker->getNextChar() != '{')
//		return true;
//	else
//		return false;
//}
//Add cases in the method: 
//"for(;;)"
//"#" for newline
//":" for public private protected
bool SemiExp::specialAddRule(std::string token, std::vector<Token> &tokens)
{
	if (token == "#") {// add for the first time 
		_specialStr.push_back("\n");
		return false;
	}

	if (token == "for") {// add for the first time 
		for (size_t i = 0; i < _specialStr.size(); i++) {
			if (_specialStr[i] == ";")
				_specialStr.erase(_specialStr.begin() + i);
		}
		return false;
	}
	//for public, private, protected keywords
	for (size_t i = 0; i < _specialStr.size(); i++) {
		if (token == _specialStr[i])
			if(token != ":")
				return true;
			else {
				std::string tempStr = tokens[tokens.size() - 2];
				if (tempStr == "public"|| tempStr == "private" || tempStr == "protected") {
					return true;
				}
				return false;
			}
	}
	return false;
}

//delete rule for special case
void SemiExp::specialDelRule() {
	for (size_t i = 0; i < _specialStr.size(); i++) {
		if (_specialStr[i] == "\n")
			_specialStr.erase(_specialStr.begin() + i);
	}

	for (size_t i = 0; i < _specialStr.size(); i++) {
		if (_specialStr[i] == ";")
			return;
	}
	_specialStr.push_back(";");
}

Token& SemiExp::operator[](size_t n)
{
  if (n < 0 || n >= _tokens.size())
    throw(std::invalid_argument("index out of range 1, SemiExp.cpp"));
  return _tokens[n];
}

Token SemiExp::operator[](size_t n)const {
	if (n < 0 || n >= _tokens.size())
		throw(std::invalid_argument("index out of range 2, SemiExp.cpp"));
	return _tokens[n];
}

size_t SemiExp::length()
{
  return _tokens.size();
}

bool SemiExp::merge(const std::string& firstTok, const std::string& secondTok) { return true; }

size_t SemiExp::find(const std::string& tok) {
	for (size_t i = 0; i < _tokens.size(); i++) {
		if (_tokens[i] == tok)
			return i;
	}
	return length();
}

size_t SemiExp::findFrom(size_t pos, const std::string& tok) {
	for (size_t i = pos; i < _tokens.size(); i++) {
		if (_tokens[i] == tok)
			return i;
	}
	return length();
}

size_t SemiExp::findFromBack(const std::string& tok) {
	int i = (int)(_tokens.size() - 1);
	while (i >= 0) {
		if (_tokens[(size_t)i] == tok)
			return (size_t)i;
		i--;
	}
	return length();
}

void SemiExp::push_back(const std::string& tok) {
	_tokens.push_back(tok);
}

bool SemiExp::remove(const std::string& tok) {
	if (_tokens.size() == 0)
		return false;
	for (size_t i = 0; i < _tokens.size(); i++) {
		if (_tokens[i] == tok)
		{
			_tokens.erase(_tokens.begin() + i);
			return true;
		}
	}
	//cannot find tok in the semi-expression
	return false;
}

//remove token of position i
bool SemiExp::remove(size_t i) {
	if (_tokens.size() <= i)
		return false;
	_tokens.erase(_tokens.begin() + i);
	return true;
}

void SemiExp::toLower() {
	if (_tokens.size() == 0)
		return;
	std::locale loc;
	std::string lowerStr;
	for (size_t i = 0; i < _tokens.size(); i++) {
		for (size_t p = 0; p < _tokens[i].size();p++)
			lowerStr += std::tolower(_tokens[i][p], loc);
		_tokens[i] = lowerStr;
		lowerStr.clear();
	}
}

void SemiExp::trimFront() {
	if (_tokens.size() == 0)
		return;
	for (size_t i = 0; i < _tokens.size(); i++) {
		if (_tokens[i] == "\n") {
			_tokens.erase(_tokens.begin()+i);
			i--;
		}
	}
}

void SemiExp::clear() {
	_tokens.clear();
}

//showNewLines : false by default
Token SemiExp::show(bool showNewLines)
{
	std::ostringstream out;
	out << "\n  ";
	if (!showNewLines) { 
		for (auto token : _tokens)
			if (token != "\n")
			{
				out << token<<" ";
			}
		out << "\n";
		return out.str();
	}
  for (auto token : _tokens)
	  out << token<< " ";
  out << "\n";
  return out.str();
}
#ifdef TEST_SemiExp
int main()
{
  Toker toker;
  //std::string fileSpec = "../Tokenizer/Tokenizer.cpp";
  std::string fileSpec = "../case test.txt";
  std::fstream in(fileSpec);
  if (!in.good())
  {
    std::cout << "\n  can't open file " << fileSpec << "\n\n";
    return 1;
  }
  toker.attach(&in);

  SemiExp semi(&toker);
  while(semi.get())
  {
    std::cout << "\n  -- semiExpression -- ";
	semi.toLower();
	semi.trimFront();
    semi.show();
  }
  /*
     May have collected tokens, but reached end of stream
     before finding SemiExp terminator.
   */
  if (semi.length() > 0)
  {
    std::cout << "\n  -- semiExpression -- ";
	semi.trimFront();
    semi.show();
    std::cout << "\n\n";
  }
  return 0;
}
#endif
