#ifndef MY_CANVAS_H
#define MY_CANVAS_H

#include <string>
#include <TCanvas.h>
namespace{
  const int MAXDRAWCH = 64;
}

class MyCanvas{
public:
  MyCanvas(std::string canvas_name);
  ~MyCanvas();
  TCanvas* cloneCanvas(){return canvas;}

private:
  std::string m_canvas_name;
  TCanvas *canvas;
  TPad *pad[MAXDRAWCH];
  void makeCanvas();
};


#endif
