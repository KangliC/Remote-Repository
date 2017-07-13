////////////////////////////////////////////////////////////////////////////////////////////
// globalQ - for CSE687 Project #4, Spring 2015											  //
// - create a static queue works like a global var, with ID parameter to identify its work//
// - get() return a static queue                                                          //
// ver 1.0
// Author: Kangli Chu,  CSE687 - Object Oriented Design, Spring 2015                      //
////////////////////////////////////////////////////////////////////////////////////////////
#ifndef GLOBALQ_H
#define GLOBALQ_H

#include"../BlockingQueue/Cpp11-BlockingQueue.h"

template <typename T, int ID>
class globalQ
{
public:
	BlockingQueue<T>& get() { return static_q; }
private:
	static BlockingQueue<T> static_q;
};
template <typename T, int ID>
BlockingQueue<T> globalQ<T, ID>::static_q;

#endif // !GLOBALQ_H

