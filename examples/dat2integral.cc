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

class Integrator{
public:
  Integrator(string header_name, int max_cycle, string output_dir, const bool is_debug=false);
  ~Integrator();

private:
  string m_header_name;
  bool m_is_debug;

  double *m_rawwave;
  
  //== branch for integral_header====//
  int m_integral_range_min = 450, m_integral_range_max = 650;
  int m_dark_range_min = 200, m_dark_range_max = 400;
  int m_max_cycle;

  //==== branch for integral_tree ====//
  int m_cycle;
  int m_event;
  int m_ch;
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
  
  integral_header -> Branch("integral_range_min", &m_integral_range_min);
  integral_header -> Branch("integral_range_max", &m_integral_range_max);
  integral_header -> Branch("dark_range_min", &m_dark_range_min);
  integral_header -> Branch("dark_range_max", &m_dark_range_max);
  integral_header -> Branch("max_cycle", &m_max_cycle);
  integral_header -> Fill();
  integral_header -> Write();
  
  integral_tree -> Branch("cycle", &m_cycle);
  integral_tree -> Branch("event", &m_event);
  integral_tree -> Branch("ch", &m_ch);
  integral_tree -> Branch("integral_event", &m_integral_event);
  integral_tree -> Branch("dark_event", &m_dark_event);
  integral_tree -> Branch("baseline", &m_baseline);
  integral_tree -> Branch("baseline_sigma", &m_baseline_sigma);

  gSystem -> Unlink("debug_baseline.gif");

  for(int cy=0; cy<max_cycle+1; cy++){
    cout << endl;
    cout << cy << " / " << max_cycle << " cycle: ";
    m_cycle = cy;
    stringstream ss_filenum;
    ss_filenum << setw(2) << setfill('0') << cy;
    string input_filename = m_header_name + "_" + ss_filenum.str() + ".dat";
    DatReader *wfm = new DatReader(input_filename, m_is_debug);
    int max_event_num = wfm -> getMaxEventNum();
    
    for(int ev=0; ev<max_event_num; ev++){
      if(ev%100==0){
	cout << ". ";
      }
      for(int ch=0; ch<64; ch++){
	m_ch = ch;
	int module_num;
	if(ch==0){
	  module_num = 0;
	  wfm -> getEvent(ev,module_num);
	}else if(ch==32){
	  module_num = 1;
	  wfm -> getEvent(ev,module_num);
	}
	
	m_rawwave = wfm -> getAdc(ch%32);
	int clock_length = wfm -> getCurrentClockLength();	
	
	BaselineAnalizer *base_anal 
	  = new BaselineAnalizer(m_rawwave, clock_length, wfm->getCurrentBitNum(), 
				 m_is_debug, module_num, ch%32, ev);
	m_baseline = base_anal -> getBaseline();
	m_baseline_sigma = base_anal -> getBaselineSigma();
	
	double integral_event = 0.0;
#pragma omp parallel for reduction(+:integral_event)
	for(int sample=m_integral_range_min; sample<m_integral_range_max; sample++){
	  integral_event += m_baseline - m_rawwave[sample];
	}
	m_integral_event = integral_event;
	  
	double dark_event = 0.0;
#pragma omp parallel for reduction(+:dark_event)
	for(int sample=m_dark_range_min; sample<m_dark_range_max; sample++){
	  dark_event += m_baseline - m_rawwave[sample];
	}
	m_dark_event = dark_event;
	
	delete base_anal;
	integral_tree -> Fill();
      }
      cout << flush;
    }
    delete wfm;
  }
  integral_tree -> Write();
  ofile -> Close();
}

Integrator::~Integrator(){

}


int main(int argc, char* argv[]){
  if(argc!=4){
    cout << "# Usage: " << argv[0] << " "
	 << "[integ header name] [max cycle] [output file name]"
	 << endl;
    return -1;
  }
  const string header_name  = argv[1];
  const int max_cycle = atoi(argv[2]);
  const string output_filename = argv[3];
  bool is_debug = false;
  
  Integrator *integ = new Integrator(header_name, max_cycle, output_filename, is_debug);

  delete integ;
}
