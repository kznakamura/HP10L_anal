#ifndef MYCLASS_H
#define MYCLASS_H

#include <string>
#include <TH1.h>

class MyClass{
 private:
  std::string m_string;
  int m_new_num;
 public:
  int m_hoge;
  TH1D *h = new TH1D("h","h",10,0,10);
  void isMyClass();

};

#endif
