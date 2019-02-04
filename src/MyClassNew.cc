#include "../include/MyClassNew.h"
// #include "MyClassNew.h" is OK with this Makefile. To include header in CINT, designate relative path. 
#include <iostream>

using namespace std;

void MyClassNew::isMyClassNew(){

  cout<<"This is MyClassNew"<<endl;

}
