#ifndef FILEMGR_H
#define FILEMGR_H
/////////////////////////////////////////////////////////////////////
// FileMgr.h - find files matching specified patterns              //
//             on a specified path                                 //
// ver 2.2 : 04/07/2016
// - add queue to the package
// ver 2.1                                                         //
//  Author:		   Kangli Chu
//  Source: Jim Fawcett, Help Session Demo, Spring 2015            //
/////////////////////////////////////////////////////////////////////
/* ver 2.2 : 06 April 2016
 * - add processworkitem
 * ver 2.1 : 13 Mar 2016
 * - add filepath and change the initializer.
 * ver 2.0 : 12 Mar 2016
 * - fixes bug in directory recursion in find(path)
 * - reduces scope of for loop in find for efficiency
 * ver 1.0 : 19 Feb 2016
 * - first release
 */
//#include "FileSystem.h"
#include  "../FileSystem/FileSystem.h"
#include "../DataStore/DataStore.h"
#include <iostream>
#include "../QueuedWorkItems/QueuedWorkItems.h"

using WorkResult = std::string;

class FileMgr
{
public:
  using iterator = DataStore::iterator;
  using patterns = std::vector<std::string>;

  FileMgr(const std::string& path, DataStore& ds, DataStore& filePath) : path_(path), store_(ds), filePath_(filePath)
  {
    patterns_.push_back("*.*");
  }
  void config(BlockingQueue<WorkResult>* pQueue) {
	  pQueue_ = pQueue;
  }
  void addPattern(const std::string& patt)
  {
    if (patterns_.size() == 1 && patterns_[0] == "*.*")
      patterns_.pop_back();
    patterns_.push_back(patt);
  }

  void search()
  {
    find(path_);
	//stop sign "\n"
	pQueue_->enQ("\n");
  }
    //---------------------------------------------------------
    // The statement above is not necessary, but helps to
    // make the processing clearer.  To show that, comment
    // it out and uncomment the following line.
    //
    //std::string fpath = path;
    //
    // Remove the std::cout statements below after you're 
    // convinced that everything works as it should.
    //---------------------------------------------------------
  void find(const std::string& path)
  {
    std::string fpath = FileSystem::Path::getFullFileSpec(path);

    for (auto patt : patterns_)  // the scope of this for loop was
    {                            // changed to improve efficiency
      std::vector<std::string> files = FileSystem::Directory::getFiles(fpath, patt);
	  //if (files.size() != 0)
		//  ;// std::cout << "\n  ++ " << fpath;
      for (auto f : files)
      {
        //std::cout << "\n  --   " << f;
		std::string fullpath = fpath + "\\" + f;
		pQueue_->enQ(fullpath);
		//store full path
		filePath_.save(fullpath);
		//store file name only
        store_.save(f);
      }
    }
    std::vector<std::string> dirs = FileSystem::Directory::getDirectories(fpath);
    for (auto d : dirs)
    {
      if (d == "." || d == "..")
        continue;
      std::string dpath = fpath + "\\" + d;  // here's the fix
      find(dpath);
    }
  }
private:
  BlockingQueue<WorkResult>* pQueue_;
  std::string path_;
  DataStore& store_;
  DataStore& filePath_;
  patterns patterns_;
};

#endif
