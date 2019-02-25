#include "../include/MyCanvas.h"
#include <string>
#include <iostream>
#include <TCanvas.h>

using namespace std;

namespace{
  extern const int MAXDRAWCH;
}

MyCanvas::MyCanvas(string canvas_name){

m_canvas_name = canvas_name;
canvas = new TCanvas(m_canvas_name.c_str(), m_canvas_name.c_str(), 100, 100, 800*5, 600*5);
makeCanvas();

}

void MyCanvas::makeCanvas(){

  canvas -> cd();
  double padsize_x = 1.0/10.0, padsize_y = 1.0/8.0;
  double position_x, position_y;
  
  for(int draw_ch=0; draw_ch<MAXDRAWCH; draw_ch++){
    if(draw_ch>=0 && draw_ch<6){
      position_x = padsize_x * (draw_ch%10 + 2.0);
      position_y = 7.0/8.0;
    }else if(draw_ch>=6 && draw_ch<13){
      position_x = padsize_x * ((draw_ch-6)%10 + 1.5);
      position_y = 6.0/8.0;
    }else if(draw_ch>=13 && draw_ch<21){
      position_x = padsize_x * ((draw_ch-13)%10 + 1.0);
      position_y = 5.0/8.0;
    }else if(draw_ch>=21 && draw_ch<30){
      position_x = padsize_x * ((draw_ch-21)%10 +0.5);
      position_y = 4.0/8.0;
    }else if(draw_ch>=30 && draw_ch<40){
      position_x = padsize_x * ((draw_ch-30)%10 + 0.0);
      position_y = 3.0/8.0;
    }else if(draw_ch>=40 && draw_ch<49){
      position_x = padsize_x * ((draw_ch-40)%10 +0.5);
      position_y = 2.0/8.0;
    }else if(draw_ch>=49 && draw_ch<57){
      position_x = padsize_x * ((draw_ch-49)%10 + 1.0);
      position_y = 1.0/8.0;
    }else if(draw_ch>=57 && draw_ch<64){
      position_x = padsize_x * ((draw_ch-57)%10 + 1.5);
      position_y = 0.0/8.0;
    }else{
      cout << "draw_ch is out of range" << endl;
      exit(-1);
    }

    pad[draw_ch] = new TPad(Form("ch%d",draw_ch), 
			    Form("ch%d",draw_ch), 
			    position_x, position_y, 
			    position_x+padsize_x, position_y+padsize_y, 
			    0, 0.01, 0);    
    pad[draw_ch] -> Draw();
    pad[draw_ch] -> SetNumber(draw_ch+1);
  }
}

MyCanvas::~MyCanvas(){
  delete canvas;
  for(int draw_ch=0; draw_ch<MAXDRAWCH; draw_ch++){
    delete pad[draw_ch];
  }
}

