#include "DatReader.h"
#include "BaselineAnalizer.h"
#include <iostream>
#include <string>
#include <sstream>
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>


using namespace std;

const int MAXCYCLE=50;

class Integrator{
public:
  Integrator(string header_name, int max_cycle, string output_dir, const bool is_debug=false);
  ~Integrator();

private:
  DatReader *wfm;
  
  int m_MAXCYCLE = MAXCYCLE;
  
  string m_header_name;
  bool m_is_debug;

  double *m_rawwave;
  
  int m_integral_hist_min = -400, m_integral_hist_max = 200000;
  int m_dark_hist_min = -400, m_dark_hist_max = 200000;

  //== branch for integral_header====//
  int m_integral_range_min = 450, m_integral_range_max = 650;
  int m_dark_range_min = 200, m_dark_range_max = 400;

  //==== branch for integral_tree ====//
  int m_max_cycle;
  int m_ch;
  double m_integral_mean[MAXCYCLE] = {};
  double m_integral_error[MAXCYCLE] = {};
  double m_dark_mean[MAXCYCLE] = {};
  double m_dark_error[MAXCYCLE] = {};

  //==== branch for integral_event_tree ====//
  int m_cycle;
  int m_event;
  double m_integral_event;
  double m_dark_event;
  double m_baseline;
  double m_baseline_sigma;  
};

Integrator::Integrator(string header_name, int max_cycle, string output_filename, const bool is_debug){
  m_header_name = header_name;
  m_max_cycle = max_cycle;
  m_is_debug = is_debug;
  
  TFile *ofile = new TFile(Form("%s", output_filename.c_str()), "recreate");
  TTree *integral_header = new TTree("integral_header","integral_header");
  TTree *integral_tree = new TTree("integral_tree", "integral_tree");
  TTree *integral_event_tree = new TTree("integral_event_tree", "integral_event_tree");
  
  integral_header -> Branch("integral_range_min", &m_integral_range_min);
  integral_header -> Branch("integral_range_max", &m_integral_range_max);
  integral_header -> Branch("dark_range_min", &m_dark_range_min);
  integral_header -> Branch("dark_range_max", &m_dark_range_max);
  integral_header -> Branch("MAXCYCLE", &m_MAXCYCLE);
  integral_header -> Fill();
  integral_header -> Write();
    
  integral_tree -> Branch("max_cycle", &m_max_cycle, "max_cycle/I");
  integral_tree -> Branch("ch", &m_ch);
  integral_tree -> Branch("integral_mean", m_integral_mean, "integral_mean[max_cycle]/D");
  integral_tree -> Branch("integral_error", m_integral_error, "integral_error[max_cycle]/D");
  integral_tree -> Branch("dark_mean", m_dark_mean, "dark_mean[max_cycle]/D");
  integral_tree -> Branch("dark_error", m_dark_error, "dark_error[max_cycle]/D");

  integral_event_tree -> Branch("cycle", &m_cycle);
  integral_event_tree -> Branch("event", &m_event);
  integral_event_tree -> Branch("ch", &m_ch);
  integral_event_tree -> Branch("integral_event", &m_integral_event);
  integral_event_tree -> Branch("dark_event", &m_dark_event);
  integral_event_tree -> Branch("baseline", &m_baseline);
  integral_event_tree -> Branch("baseline_sigma", &m_baseline_sigma);

  TCanvas *c_integral = new TCanvas("c_integral", "c_integral",700,500);
  TCanvas *c_dark = new TCanvas("c_dark", "c_dark",700,500);
  gSystem -> Unlink("debug_baseline.gif");
  gSystem -> Unlink("debug_integral.gif");
  gSystem -> Unlink("debug_dark.gif");

  for(int ch=0; ch<64; ch++){
    m_ch = ch;
    int module_num;
    if(ch>=0 && ch<32){
      module_num = 0;
    }else if(ch>=32 && ch<64){
      module_num = 1;
    }
    
    for(int cy=0; cy<max_cycle+1; cy++){
            
      stringstream ss_filenum;
      ss_filenum << setw(2) << setfill('0') << cy;
      string input_filename = header_name + "_" + ss_filenum.str() + ".dat";
      wfm = new DatReader(input_filename);
      int max_event_num = wfm -> getMaxEventNum();
      
      TH1D *h_integral = new TH1D("h_integral", 
				  "h_integral", 
				  (m_integral_hist_max-m_integral_hist_min)/10, 
				  m_integral_hist_min, 
				  m_integral_hist_max);
      TH1D *h_dark = new TH1D("h_dark", 
			      "h_dark", 
			      (m_dark_hist_max-m_dark_hist_min)/10, 
			      m_dark_hist_min, 
			      m_dark_hist_max);
      
      for(int ev=0; ev<max_event_num; ev++){
	if(ev%1000==0){
	  cout << "ch=" << ch 
	       << ", cycle=" << cy 
	       << ", event=" << ev << endl;
	}
	
	wfm -> getEvent(ev,module_num);
	m_rawwave = wfm -> getAdc(ch%32);
	int clock_length = wfm -> getCurrentClockLength();
	
	BaselineAnalizer *base_anal 
	  = new BaselineAnalizer(m_rawwave, clock_length, wfm->getCurrentBitNum(), 
				 m_is_debug, module_num, ch%32, ev);
	m_baseline = base_anal -> getBaseline();
	m_baseline_sigma = base_anal -> getBaselineSigma();

	m_integral_event = 0;
	for(int sample=m_integral_range_min; sample<m_integral_range_max; sample++){
	  m_integral_event += m_baseline - m_rawwave[sample];
	}
	m_dark_event = 0;
	for(int sample=m_dark_range_min; sample<m_dark_range_max; sample++){
	  m_dark_event += m_baseline - m_rawwave[sample];
	}

	if(h_integral->Fill(m_integral_event) == -1){
	  cout << "#Error: h_integral is overflowed or underflowed" << endl;
	  cout << "cycle=" << cy+1
	       << ", ch=" << ch
	       << ", event=" << ev
	       << ", baseline_peak_bin=" << base_anal->getBaselinePeakBin()
	       << ", baseline=" << m_baseline 
	       << ", value=" << m_integral_event << endl;
	  exit(-1);
	}
	if(h_dark->Fill(m_dark_event) == -1){
	  cout << "#Error: h_dark is oveflowed or underflowed" << endl;
	  cout << "ch=" << ch
	       << ", event=" << ev
	       << ", value=" << m_dark_event << endl;
	  exit(-1);
	}
	integral_tree -> Fill();
	delete base_anal;
      }
      
      double integral_peak_bin = m_integral_hist_min 
	+ h_integral->GetBinWidth(0) * h_integral->GetMaximumBin();
      double dark_peak_bin = m_dark_hist_min 
	+ h_dark->GetBinWidth(0) * h_dark->GetMaximumBin();
      double hist_integral_sigma = h_integral->GetStdDev();
      double hist_dark_sigma = h_dark->GetStdDev();

      TF1 *f_integral = new TF1("f_integral", "gaus", m_integral_hist_min, m_integral_hist_max);
      TF1 *f_dark = new TF1("f_dark", "gaus", m_dark_hist_min, m_dark_hist_max);

      h_integral -> Fit("f_integral", "Q", "", integral_peak_bin-hist_integral_sigma*3, integral_peak_bin+hist_integral_sigma*3);
      h_dark -> Fit("f_dark", "Q", "", dark_peak_bin-hist_dark_sigma*3, dark_peak_bin+hist_dark_sigma*3);
      double integral_fit_mean= f_integral->GetParameter(1);
      double integral_fit_sigma = f_integral->GetParameter(2);
      double dark_fit_mean = f_dark->GetParameter(1);
      double dark_fit_sigma = f_dark->GetParameter(2);
      
      if(m_is_debug){
	c_integral -> cd() -> DrawFrame(-50,0,integral_peak_bin*2.0,h_integral->GetMaximum()*1.5,Form("Integral: ch%d, cycle%d; integral, # of entries",ch,cy));
	h_integral -> Draw("sames");
	c_integral -> Modified();
	c_integral -> Update();
	c_integral -> Print("debug_integral.gif+");
	cout << "debug_integral.gif is saved" << endl; 
	c_dark -> cd() -> DrawFrame(-50,0,300,800,Form("Dark: ch%d, cycle%d; integral, # of entries",ch,cy));
	h_dark -> Draw("sames");
	c_dark -> Modified();
	c_dark -> Update();
	c_dark -> Print("debug_dark.gif+");
	cout << "debug_dark.gif is saved" << endl; 
      }
         
      m_integral_mean[cy] = integral_fit_mean;
      m_integral_error[cy] = integral_fit_sigma/sqrt(max_event_num+1);
      m_dark_mean[cy] = dark_fit_mean;
      m_dark_error[cy] = dark_fit_sigma/sqrt(max_event_num+1);
      if(cy!=0 && m_integral_mean[cy]<m_integral_mean[cy-1]){
	cout << "\n# Warning: current integral value is smaller than the previous integral value." << endl;
	m_integral_mean[cy] = m_integral_mean[cy-1];
	m_integral_error[cy] = m_integral_error[cy-1];
	m_dark_mean[cy] = m_dark_mean[cy-1];
	m_dark_error[cy] = m_dark_error[cy-1];
	}
      delete h_integral;
      delete h_dark;
      delete f_integral;
      delete f_dark;
      delete wfm;
    }
    integral_tree -> Fill();
  }
  delete c_integral;
  delete c_dark;
  integral_tree -> Write();
  integral_event_tree -> Write();
  ofile -> Close();
}

Integrator::~Integrator(){

}


int main(int argc, char* argv[]){
  if(argc!=4){
    cout << "# Usage: " << argv[0] << " "
	 << "[integ header name] [# of files] [output file name]"
	 << endl;
    return -1;
  }
  const string header_name  = argv[1];
  const int max_cycle = atoi(argv[2]);
  const string output_filename = argv[3];
  bool is_debug = true;
  
  Integrator *integ = new Integrator(header_name, max_cycle, output_filename, is_debug);

  delete integ;
}
