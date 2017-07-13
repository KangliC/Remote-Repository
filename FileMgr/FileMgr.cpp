/////////////////////////////////////////////////////////////////////
// FileMgr.cpp - find files matching specified patterns            //
//               on a specified path                               //
// ver 2.2                                                         //
// Author:		   Kangli Chu
// Source: Jim Fawcett, Help Session Demo, Spring 2015             //
/////////////////////////////////////////////////////////////////////

#include "FileMgr.h"
#include "../DataStore/DataStore.h"
#include <iostream>

#ifdef adTEST_FILEMGR

int main()
{
  std::cout << "\n  Testing FileMgr";
  std::cout << "\n =================";

  DataStore ds;
  DataStore filePath;
  FileMgr fm("..", ds, filePath);
  fm.addPattern("*.h");
  fm.addPattern("*.c");
  fm.addPattern("*.cpp");
  //fm.addPattern("*.partial");
  fm.search();

  std::cout << "\n\n  contents of DataStore";
  std::cout << "\n -----------------------";
  for (auto fs : ds)
  {
    std::cout << "\n  " << fs;
  }
  for (auto fs : filePath)
  {
	  std::cout << "\n  " << fs;
  }
  std::cout << "\n\n";
  return 0;
}
#endif
