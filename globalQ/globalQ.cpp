////////////////////////////////////////////////////////////////////////////////////////////
// globalQ - for CSE687 Project #4, Spring 2015											  //
// - create a static queue works like a global var, with ID parameter to identify its work//
// - get() return a static queue                                                          //
// ver 1.0
// Author: Kangli Chu,  CSE687 - Object Oriented Design, Spring 2015                      //
////////////////////////////////////////////////////////////////////////////////////////////
#include"globalQ.h"
#include<iostream>

#ifdef GLOBALQ_DEBUG
int main() {
	std::cout << "globalQ use the staic queue.\n";
	return 1;
}
#endif // GLOBALQ_DEBUG