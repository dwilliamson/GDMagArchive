#include <fluid/Utility/IntHash.h>
#include <iostream>

int main()
{
  IntHash<char> mySet;
  IntHash<float> myMap;

  mySet.insert(0xffff,0xffff,0xffff);
  mySet.insert(0xffff,0xffff,0x00ff);
  mySet.insert(5,4,8);
  mySet.insert(0,0,0);
  mySet.insert(18,3,1);
  mySet.insert(1,1,1);
  mySet.insert(1524,231,450);
  mySet.insert(948,302,56);
  mySet.insert(9599,382,1909);
  mySet.insert(50493,4992,19028);
  mySet.insert(193,294,2958);

  std::cout << "true: " << mySet.doesExist(5,4,8) << std::endl;
  std::cout << "true: " << mySet.doesExist(0xffff,0xffff,0xffff) << std::endl;
  std::cout << "true: " << mySet.doesExist(0,0,0) << std::endl;
  std::cout << "true: " << mySet.doesExist(0xffff,0xffff,0x00ff) << std::endl;
  std::cout << "true: " << mySet.doesExist(18,3,1) << std::endl;
  std::cout << "true: " << mySet.doesExist(1,1,1) << std::endl;
  std::cout << "true: " << mySet.doesExist(1524,231,450) << std::endl;
  std::cout << "true: " << mySet.doesExist(948,302,56) << std::endl;
  std::cout << "true: " << mySet.doesExist(9599,382,1909) << std::endl;
  std::cout << "true: " << mySet.doesExist(50493,4992,19028) << std::endl;
  std::cout << "true: " << mySet.doesExist(193,294,2958) << std::endl;

  std::cout << "false: " << mySet.doesExist(0xff, 0xff, 0xff) << std::endl;
  std::cout << "false: " << mySet.doesExist(15, 241, 321) << std::endl;
  std::cout << "false: " << mySet.doesExist(43, 3653, 4444) << std::endl;
  std::cout << "false: " << mySet.doesExist(67, 42, 2593) << std::endl;
  std::cout << "false: " << mySet.doesExist(120, 534, 43921) << std::endl;
  std::cout << "false: " << mySet.doesExist(302, 24, 302) << std::endl;

  myMap.insert(0xffff,0xffff,0xffff, 0.1);
  myMap.insert(0xffff,0xffff,0x00ff, 0.2);
  myMap.insert(5,4,8, 0.3);
  myMap.insert(0,0,0, 0.4);
  myMap.insert(18,3,1, 0.5);
  myMap.insert(1,1,1, 0.6);
  myMap.insert(1524,231,450, 7);
  myMap.insert(948,302,56, 80.0);
  myMap.insert(9599,382,1909, 900.0);
  myMap.insert(50493,4992,19028, 1000.0);
  myMap.insert(193,294,2958, 666.666);

  std::cout << "value: " << *myMap.find(0xffff,0xffff,0xffff) << std::endl;
  std::cout << "value: " << *myMap.find(0xffff,0xffff,0x00ff) << std::endl;
  std::cout << "value: " << *myMap.find(5,4,8) << std::endl;
  std::cout << "value: " << *myMap.find(0,0,0) << std::endl;
  std::cout << "value: " << *myMap.find(18,3,1) << std::endl;
  std::cout << "value: " << *myMap.find(1,1,1) << std::endl;
  std::cout << "value: " << *myMap.find(1524,231,450) << std::endl;
  std::cout << "value: " << *myMap.find(948,302,56) << std::endl;
  std::cout << "value: " << *myMap.find(9599,382,1909) << std::endl;
  std::cout << "value: " << *myMap.find(50493,4992,19028) << std::endl;
  std::cout << "value: " << *myMap.find(193,294,2958) << std::endl;

  std::cout << "null: " << myMap.find(0xff, 0xff, 0xff) << std::endl;
  std::cout << "null: " << myMap.find(15, 241, 321) << std::endl;
  std::cout << "null: " << myMap.find(43, 3653, 4444) << std::endl;
  std::cout << "null: " << myMap.find(67, 42, 2593) << std::endl;
  std::cout << "null: " << myMap.find(120, 534, 43921) << std::endl;
  std::cout << "null: " << myMap.find(302, 24, 302) << std::endl;
  return 0;
}