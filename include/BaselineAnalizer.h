#ifndef BASELINE_ANALIZER_H
#define BASELINE_ANALIZER_H

#include <TH1.h>
#include <TF1.h>

class BaselineAnalizer{
public:
  //==== Constructor and Destructor ====//
  BaselineAnalizer(double *wave, int clock_length, int bit_num, 
		   const bool is_debug=false, const int module=0, const int ch=0, const int event=0);
  BaselineAnalizer();
  ~BaselineAnalizer();

  //==== Baseline analisys ====//
  void anaBaseline(double *wave, int bit_num, int baseline_min, int baseline_max,
		   const int module=0, const int ch=0, const int event=0);

  double getBaselinePeakBin();
  double getBaseline();
  double getBaselineSigma();

  //==== debug ====//
  void setDebug(bool is_debug){m_is_debug = is_debug;}

private:
  //==== Input parameters ====//
  double *m_wave;
  int m_bit_num;
  int m_module;
  int m_ch;
  int m_event;

  TH1D *h_baseline;
  TF1 *f_baseline;

  //==== Baseline info ====//
  double m_baseline_peak_bin;
  double m_baseline;
  double m_baseline_sigma;

  //==== Debug ====//
  bool m_is_debug;
  void saveCanvas();
};

#endif
