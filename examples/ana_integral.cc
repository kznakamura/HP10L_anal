#include <iostream>
#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TSystem.h>

using namespace std;

const int MAXCYCLE = 50;

class IntegralAnalizer{
public:
  IntegralAnalizer(string input_filename, string output_filename, const bool is_debug);

private:
  string m_input_filename;
  bool m_is_debug;
  string m_output_filename;

  int m_integral_hist_min = -400, m_integral_hist_max = 200000;
  int m_dark_hist_min = -400, m_dark_hist_max = 200000;

  //==== branch for integral_event_tree ====//
  double m_integral_event;
  double m_dark_event;
  
  //==== branch for integral_header ====//
  int m_MAXCYCLE = MAXCYCLE;

  //==== branch for integral_tree ====//
  int m_max_cycle;
  int m_ch;
  double m_integral_mean[MAXCYCLE] = {};
  double m_integral_error[MAXCYCLE] = {};
  double m_dark_mean[MAXCYCLE] = {};
  double m_dark_error[MAXCYCLE] = {};
};


IntegralAnalizer::IntegralAnalizer(string input_filename, string output_filename, const bool is_debug){
  m_input_filename = input_filename;
  m_output_filename = output_filename;
  m_is_debug = is_debug;

  TFile *ifile = new TFile(m_input_filename.c_str());
  TFile *ofile = new TFile(m_output_filename.c_str(), "recreate");
  TTree *integral_event_header = ((TTree*)ifile -> Get("integral_event_header")) -> CloneTree();
  TTree *integral_event_tree = (TTree*)ifile -> Get("integral_event_tree");
  integral_event_header -> Write();
  m_max_cycle = integral_event_tree -> GetMaximum("cycle") + 1;
  int max_event = integral_event_tree -> GetMaximum("event");

  TTree *integral_header = new TTree("integral_header","integral_header");
  TTree *integral_tree = new TTree("integral_tree", "integral_tree");

  
  integral_header -> Branch("MAXCYCLE", &m_MAXCYCLE);
  integral_header -> Fill();
  integral_header -> Write();

  integral_tree -> Branch("max_cycle", &m_max_cycle);
  integral_tree -> Branch("ch", &m_ch);
  integral_tree -> Branch("integral_mean", m_integral_mean, "integral_mean[max_cycle]/D");
  integral_tree -> Branch("integral_error", m_integral_error, "integral_error[max_cycle]/D");
  integral_tree -> Branch("dark_mean", m_dark_mean, "dark_mean[max_cycle]/D");
  integral_tree -> Branch("dark_error", m_dark_error, "dark_error[max_cycle]/D");
  
  TCanvas *c_integral = new TCanvas("c_integral", "c_integral", 700, 500);
  TCanvas *c_dark = new TCanvas("c_dark", "c_dark", 700, 500);
  gSystem -> Unlink("debug_integral.gif");
  gSystem -> Unlink("debug_dark.gif");
  
  for(int ch=0; ch<64; ch++){
    cout << endl;
    cout << ch << " / 64 ch: ";
    m_ch = ch;
    for(int cy=0; cy<m_max_cycle; cy++){
      cout << ". ";
      TTree *copy_integral_tree = integral_event_tree->CopyTree(Form("ch==%d && cycle==%d",ch,cy));
      copy_integral_tree -> SetBranchAddress("integral_event", &m_integral_event);
      copy_integral_tree -> SetBranchAddress("dark_event", &m_dark_event);
      
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

      for(int ev=0; ev<max_event; ev++){
	copy_integral_tree -> GetEntry(ev);
	if(h_integral->Fill(m_integral_event) == -1){
	  cout << "#Error: h_integral is overflowed or underflowed" << endl;
	  cout << "cycle=" << cy
	       << ", ch=" << ch
	       << ", event=" << ev
	       << ", value=" << m_integral_event << endl;
	  exit(-1);
	}
	if(h_dark->Fill(m_dark_event) == -1){
	  cout << "#Error: h_dark is oveflowed or underflowed" << endl;
	  cout << "cycle=" << cy
	       << ", ch=" << ch
	       << ", event=" << ev
	       << ", value=" << m_dark_event << endl;
	  exit(-1);
	}
      }

      double integral_peak_bin = m_integral_hist_min 
	+ h_integral->GetBinWidth(0) * h_integral->GetMaximumBin();
      double dark_peak_bin = m_dark_hist_min 
	+ h_dark->GetBinWidth(0) * h_dark->GetMaximumBin();
      double hist_integral_sigma = h_integral->GetStdDev();
      double hist_dark_sigma = h_dark->GetStdDev();
      
      TF1 *f_integral = new TF1("f_integral", "gaus", m_integral_hist_min, m_integral_hist_max);
      TF1 *f_dark = new TF1("f_dark", "gaus", m_dark_hist_min, m_dark_hist_max);

      h_integral -> Fit("f_integral", "Q", "", 
			integral_peak_bin-hist_integral_sigma*3, integral_peak_bin+hist_integral_sigma*3);
      h_dark -> Fit("f_dark", "Q", "", 
		    dark_peak_bin-hist_dark_sigma*3, dark_peak_bin+hist_dark_sigma*3);
      double integral_fit_mean= f_integral->GetParameter(1);
      double integral_fit_sigma = f_integral->GetParameter(2);
      double dark_fit_mean = f_dark->GetParameter(1);
      double dark_fit_sigma = f_dark->GetParameter(2);
      
      if(m_is_debug){
	c_integral -> cd() -> DrawFrame(-50,0,integral_peak_bin*2.0,h_integral->GetMaximum()*1.5,
					Form("Integral: ch%d, cycle%d; integral, # of entries",ch,cy));
	h_integral -> Draw("sames");
	c_integral -> Modified();
	c_integral -> Update();
	c_integral -> Print("debug_integral.gif+");
	cout << "debug_integral.gif is saved" << endl; 
	c_dark -> cd() -> DrawFrame(-50,0,300,800,
				    Form("Dark: ch%d, cycle%d; integral, # of entries",ch,cy));
	h_dark -> Draw("sames");
	c_dark -> Modified();
	c_dark -> Update();
	c_dark -> Print("debug_dark.gif+");
	cout << "debug_dark.gif is saved" << endl; 
      }

      m_integral_mean[cy] = integral_fit_mean;
      m_integral_error[cy] = integral_fit_sigma/sqrt(max_event+1);
      m_dark_mean[cy] = dark_fit_mean;
      m_dark_error[cy] = dark_fit_sigma/sqrt(max_event+1);
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
      cout << flush;
    }
    integral_tree -> Fill();
  }
  delete c_integral;
  delete c_dark;
  integral_tree -> Write();
  ofile -> Close();
  cout << endl;
}

int main(int argc, char* argv[]){
  if(argc!=3){
    cout << "# Usage: " << argv[0] << " "
	 << "[path for sum_integ_***.root] [output_filename]"
	 << endl;
    return -1;
  }

  const string input_filename = argv[1];
  const string output_filename = argv[2];
  bool is_debug = false;

  IntegralAnalizer *integ_ana = new IntegralAnalizer(input_filename, output_filename, is_debug);

  delete integ_ana;
}


