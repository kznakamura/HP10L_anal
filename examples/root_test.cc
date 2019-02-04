#include "MyClass.h"
#include "MyClassNew.h"
#include <iostream>

using namespace std;

int main(){

  MyClass m;
  m.h -> Fill(2);
  cout << m.h -> GetEntries() << endl;
  m.isMyClass();

  MyClassNew m_new;
  m_new.isMyClassNew();
}
