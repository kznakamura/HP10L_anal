#include "DatReader.h"
#include <iostream>
#include <TGraph.h>
#include <TCanvas.h>
#include <TApplication.h>
#include <TText.h>

using namespace std;

int main(int argc, char* argv[]){
  if(argc<4){
    cout << "# Usage: " << argv[0] << " "
	 << "[drawing datfile] [draw event] [output filename]"
	 << endl;
    return -1;
  }
  const string read_filename = argv[1];
  const int draw_event = atoi(argv[2]);
  const string output_filename = argv[3];
  
  DatReader *wfm = new DatReader(read_filename.c_str());
  int max_draw_ch = 64;
  TGraph *g[max_draw_ch];
  double *x, *y;
  int length;
  TText *t_ch[max_draw_ch];
  TText *t_xlabel = new TText(0.75, 0.01,"ns");
  TText *t_ylabel = new TText(0.07, 0.5, "ADC count");
  t_xlabel -> SetTextSize(0.08);
  t_ylabel -> SetTextSize(0.08);
  t_xlabel -> SetNDC(1);
  t_ylabel -> SetNDC(1);
  t_ylabel -> SetTextAngle(90);
  
  //  TApplication app("app",&argc, argv);

  TCanvas *c = new TCanvas("c","c",100,100,800*5,600*5);
  c -> cd();
  TPad *pad[max_draw_ch];
  double padsize_x = 1.0/10.0, padsize_y = 1.0/8.0;
  double position_x, position_y;

  for(int draw_ch=0; draw_ch<max_draw_ch; draw_ch++){
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
      return -1;
      }

    pad[draw_ch] = new TPad(Form("ch%d",draw_ch), Form("ch%d",draw_ch), position_x, position_y, position_x+padsize_x, position_y+padsize_y, 0, 0.01, 0);    
    pad[draw_ch] -> Draw();
    pad[draw_ch] -> SetNumber(draw_ch+1);
  }

  for(int module=0; module<2; module++){
    wfm -> getEvent(draw_event, module);
    int current_maxch_num = wfm -> getCurrentMaxchNum();
    for(int ch=0; ch<current_maxch_num; ch++){
      int draw_ch = module*current_maxch_num + ch;
      c -> cd(draw_ch+1);
     
      x = wfm -> getClock();
      y = wfm -> getAdc(ch);
      length = wfm -> getCurrentClockLength();
      g[draw_ch] = new TGraph(length,x,y);
      g[draw_ch] -> SetLineColor(4);
      g[draw_ch] -> SetTitle(";;");
      g[draw_ch] -> Draw("apl");
      t_ch[draw_ch] = new TText(0.15, 0.25, Form("ch%d",draw_ch));
      t_ch[draw_ch] -> SetNDC(1);
      t_ch[draw_ch] -> SetTextSize(0.15);
      t_ch[draw_ch] -> Draw();
      t_xlabel -> Draw("same");
      t_ylabel -> Draw("same");
    }
  }
  //app.Run();
  
  c -> Print(output_filename.c_str());
  cout << "# Save: --> \"" << output_filename << "\"" << endl;
  
  delete wfm;
  
}
