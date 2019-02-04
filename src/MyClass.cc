#include "../include/MyClass.h" 
// #include "MyClass.h" is OK with this Makefile. To include header in CINT, designate relative path.
#include <iostream>

using namespace std;

void MyClass::isMyClass(){

  cout<<"This is MyClass"<<endl;

}
