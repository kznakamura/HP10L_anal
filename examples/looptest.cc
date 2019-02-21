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
  DatReader *wfm;
  
  string m_header_name;
  bool m_is_debug;
  int m_max_cycle;

  double *m_rawwave;
  double m_baseline, m_baseline_sigma;
  double m_integral_event;
};

Integrator::Integrator(string header_name, int max_cycle, string output_filename, const bool is_debug){
  m_header_name = header_name;
  m_max_cycle = max_cycle;
  m_is_debug = is_debug;

  cout << "#### analysis start ####" << endl;  
  for(int cy=0; cy<max_cycle+1; cy++){      

    stringstream ss_filenum;
    ss_filenum << setw(2) << setfill('0') << cy;
    string input_filename = header_name + "_" + ss_filenum.str() + ".dat";
    wfm = new DatReader(input_filename);
    cout << "#### New dat file is inputed ####" << endl;
    int max_event_num = wfm -> getMaxEventNum();

    for(int ev=0; ev<max_event_num; ev++){
      //      if(ev%100==0){
	cout << "cycle=" << cy 
	     << ", event=" << ev << endl;
	// }
 
      for(int ch=0; ch<64; ch++){
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

	int m_integral_event = 0;
	//#pragma omp parallel for reduction(+:m_integral_event)
	for(int sample=450; sample<650; sample++){
	  m_integral_event += m_baseline - m_rawwave[sample];
	}
	//	cout << "m_integral_event= " << m_integral_event << endl;

	delete base_anal;
      }
    }
    delete wfm;
  }

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
  bool is_debug = false;
  
  Integrator *integ = new Integrator(header_name, max_cycle, output_filename, is_debug);

  delete integ;
}
