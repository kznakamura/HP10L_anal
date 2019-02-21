#include "../include/BaselineAnalizer.h"
#include <iostream>
#include <TH1.h>
#include <TF1.h>
#include <TStyle.h>
#include <TCanvas.h>

using namespace std;

BaselineAnalizer::BaselineAnalizer(double* wave, int clock_length, int bit_num, 
				   const bool is_debug, const int module, const int ch, const int event){

  m_is_debug = is_debug;  
  m_baseline = 0.0;
  m_baseline_sigma = 0.0;
  h_baseline = nullptr;
  f_baseline = nullptr;

  anaBaseline(wave, bit_num, 0, clock_length, module, ch, event);
  
}

BaselineAnalizer::BaselineAnalizer(){

  m_is_debug = false;
  m_baseline = 0.0;
  m_baseline_sigma = 0.0;
  h_baseline = nullptr;
  f_baseline = nullptr;
}

BaselineAnalizer::~BaselineAnalizer(){
  delete h_baseline;
  delete f_baseline;
  h_baseline = nullptr;
  f_baseline = nullptr;
}

void BaselineAnalizer::anaBaseline(double* wave, int bit_num, int baseline_min, int baseline_max, 
				   const int module, const int ch, const int event){
  m_wave = wave;
  m_bit_num = bit_num;
  m_module = module;
  m_ch = ch;
  m_event = event;
  
  h_baseline = new TH1D("h_baseline", "h_baseline", pow(2,m_bit_num), 0, pow(2,m_bit_num));
  for(int smp=baseline_min; smp<baseline_max; smp++){
    h_baseline -> Fill(m_wave[smp]);
  }
  f_baseline = new TF1("f_baseline", "gaus", 0, pow(2,m_bit_num));
  m_baseline_peak_bin = h_baseline->GetBinWidth(0) * h_baseline -> GetMaximumBin();
  f_baseline -> SetParLimits(1, m_baseline_peak_bin-20, m_baseline_peak_bin+20);
  h_baseline -> Fit("f_baseline", "Q", "", m_baseline_peak_bin-20, m_baseline_peak_bin+20);
  
  m_baseline = f_baseline -> GetParameter(1);
  m_baseline_sigma = f_baseline -> GetParameter(2);

  if(m_is_debug && event%1000==0){
    saveCanvas();
  }
}

double BaselineAnalizer::getBaselinePeakBin(){
  if(h_baseline==nullptr){
    cerr << "# Error: use function \"anaBaseline\" before using \"getBaselinePeakBin\"." << endl;
    exit(-1);
  }
  return m_baseline_peak_bin;
}

double BaselineAnalizer::getBaseline(){
  if(h_baseline==nullptr){
    cerr << "# Error: use function \"anaBaseline\" before using \"getBaseline\"." << endl;
    exit(-1);
  }
  return m_baseline;
}

double BaselineAnalizer::getBaselineSigma(){
  if(h_baseline==nullptr){
    cerr << "# Error: use function \"anaBaseline\" before using \"getBaselineSigma\"." << endl;
    exit(-1);
  }
  return m_baseline_sigma;
}

void BaselineAnalizer::saveCanvas(){
  TCanvas *c = new TCanvas(1);
  c -> cd() -> DrawFrame(m_baseline_peak_bin-20, 0, m_baseline_peak_bin+20, 700,
			 Form("baseline: module%d, ch%d, event%d;ADC count; # of entries",m_module,m_ch,m_event));
  gStyle -> SetOptFit();
  h_baseline -> Draw("sames");
  c -> Print("debug_baseline.gif+");
  cout << "debug_baseline.gif is saved" << endl;
  delete c;
}
