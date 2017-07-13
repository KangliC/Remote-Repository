/////////////////////////////////////////////////////////////////
// MTree.cpp - M-ary Tree data structure                       //
//                                                             //
// Application: Help for CSE687 Pr#2, Spring 2015              //
// Platform:    Dell XPS 2720, Win 8.1 Pro, Visual Studio 2013 //
// Author: Kangli Chu
// Source:      Jim Fawcett, CST 4-187, 443-3948               //
//              jfawcett@twcny.rr.com                          //
/////////////////////////////////////////////////////////////////

#include "MTree.h"
#include <string>
#include <iostream>

#ifdef aTEST_MTREE

using MNodeStr = MNode < std::string > ;
using sPtr = std::shared_ptr < MNodeStr > ;

int main()
{
  title("Testing MTree<T> class", '=');

  sPtr pNode (new MNodeStr("original"));
  pNode->addChild("first child");
  pNode->addChild("second child");
  pNode->addChild("third child");
  
  MNodeStr& node = *pNode;
  node[1]->addChild("second child's child");
  MNodeStr& secondChild = *node[1];
  secondChild[0]->addChild("second child's grandchild");
  node[2]->addChild("third's child");

  // Note that reference qualifiers for node and secondChild are necessary.
  // We don't want to add children to copies of the nodes.

  sPtr pAugmented = pNode->clone();
  pAugmented->addChild("second child");
  MNode<std::string>& Augmented = *pAugmented;
  sPtr pSecondChild = Augmented[1];
  MNode<std::string>& SecondChild = *pSecondChild;
  sPtr pSecondChildChild = SecondChild[0];
  pSecondChildChild->id() = "second's grandChildId";
  MNode<std::string>& SecondChildChild = *pSecondChildChild;
  SecondChildChild[0]->addChild("second child's greatGrandChild");
  (*SecondChildChild[0])[0]->addChild("second child's greatGreatGrandChild");
  title("building tree");

  MTree<std::string> tree(pAugmented);
  tree.root()->value() = "Augmented Root Node";
  show(tree);
  std::cout << "\n";

  title("testing construction from T value");
  MTree<std::string> newValueTree("new value");
  newValueTree.root()->addChild("a child");
  show(newValueTree);
  std::cout << "\n";

  title("testing copy construction");
  MTree<std::string> copy = tree;
  copy.root()->value() = "copy";
  show(copy);
  std::cout << "\n";

  title("testing move construction");
  MTree<std::string> move = std::move(copy);
  move.root()->value() = "move";
  show(move);
  std::cout << "\n";

  title("testing copy assignment");
  copy = newValueTree;
  copy.root()->value() = "copy assigned";
  show(copy);
  std::cout << "\n";

  title("testing move assignment");
  copy = std::move(move);
  copy.root()->value() = "move assigned";
  show(copy);
  std::cout << "\n";

  mtLog.start();
  title("testing MTree<T>::findByValue(\"second child\")");
  std::vector<sPtr> ptrs = tree.findByValue("second child");
  if (ptrs.size() > 0)
  {
    std::cout << "\n  found:";
    for (auto ptr : ptrs)
      std::cout << "\n    " << ptr->value();
  }
  else
    std::cout << "\n  not found";
  std::cout << "\n";

  title("testing MTree<T>::findByValue(\"no_such_value\")");
  ptrs = tree.findByValue("no_such_value");
  if (ptrs.size() > 0)
  {
    std::cout << "\n  found:";
    for (auto ptr : ptrs)
      std::cout << "\n    " << ptr->value();
  }
  else
    std::cout << "\n  not found";
  std::cout << "\n";

  title("testing MTree<T>::findById(\"second's grandChildId\")");
  sPtr ptr = tree.findById("second's grandChildId");
  if (ptr != nullptr)
  {
    std::cout << "\n  found:";
    std::cout << "\n    value = " << ptr->value();
    std::cout << "\n       id = " << ptr->id();
  }
  else
    std::cout << "\n  not found";
  std::cout << "\n";

  title("testing MTree<T>::findById(\"no_such_id\")");
  ptr = tree.findById("no_such_id");
  if (ptr != nullptr)
  {
    std::cout << "\n  found:";
    std::cout << "\n    value = " << ptr->value();
    std::cout << "\n       id = " << ptr->id();
  }
  else
    std::cout << "\n  not found";
  std::cout << "\n";

  title("testing children of node with value \"Augmented Root Node\"");
  std::vector<std::shared_ptr<MNode<std::string>>> childr = tree.children("Augmented Root Node");
  for (auto pChild : childr)
    std::cout << "\n  " << pChild->value();
  std::cout << "\n";

  std::vector<std::shared_ptr<MNode<std::string>>> temp = tree.findByValue("second child");
  if (temp.size() > 1)
    temp[1]->addChild("added for children test");

  title("testing children of node with value \"second child\"");
  childr = tree.children("second child", 2);  // pick second node with value if it exists
  for (auto pChild : childr)
    std::cout << "\n  " << pChild->value();
  std::cout << "\n";

  title("testing descendents of node with value \"second child\"");
  std::vector<std::shared_ptr<MNode<std::string>>> desc = tree.descendents("second child");
  for (auto pChild : desc)
    std::cout << "\n  " << pChild->value();
  std::cout << "\n";

  title("showing log");
  mtLog.show();
  mtLog.clear();
  title("showing cleared log");
  mtLog.show();
  std::cout << "\n\n";
}

#endif
