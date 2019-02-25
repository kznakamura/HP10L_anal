#include "DatReader.h"
#include "MyCanvas.h"
#include <iostream>
#include <TGraph.h>
#include <TCanvas.h>
#include <TApplication.h>
#include <TText.h>

using namespace std;

int main(int argc, char* argv[]){
  if(argc<4){
    cout << "# Usage: " << argv[0] << " "
	 << "[drawing datfile] [draw event] [output canvas name]"
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
  TText *t_xlabel = new TText(0.75, 0.01,"clock");
  TText *t_ylabel = new TText(0.07, 0.5, "ADC count");
  t_xlabel -> SetTextSize(0.08);
  t_ylabel -> SetTextSize(0.08);
  t_xlabel -> SetNDC(1);
  t_ylabel -> SetNDC(1);
  t_ylabel -> SetTextAngle(90);
  
  TApplication app("app",&argc, argv);
  
  MyCanvas *my_canvas = new MyCanvas("mycanvas");
  TCanvas *c = my_canvas->cloneCanvas();

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
  app.Run();
  
  c -> Print(output_filename.c_str());
  cout << "# Save: --> \"" << output_filename << "\"" << endl;
  
  delete wfm;
  delete my_canvas;
  
}
