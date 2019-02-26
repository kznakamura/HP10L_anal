#ifndef RECOVERY_BASE_H
#define RECOVERY_BASE_H

#include <string>
#include <TGraphErrors.h>
#include <TF1.h>

namespace{
  const int MAXCYCLE = 50;
}

class RecoveryBase{
public:
  RecoveryBase(int ch, double mppc_adc_gain, int max_cycle,
		   double* calib_integral_mean, double* ref_integral_mean,
		   double* calib_integral_error=nullptr, double* ref_integral_error=nullptr,
		   double fit_range_min=1.0e5, double fit_range_max=1.0e5, const bool is_debug=false);
  
  virtual ~RecoveryBase();
  
  //---- return root objects ----//
  TGraphErrors* getAdcPlot(){return (TGraphErrors*)g_adc->Clone();}
  TF1* getLinearFit(){return (TF1*)f_linear->Clone();}
  TGraphErrors* getLinearResidualPlot(){return (TGraphErrors*)g_linear_residual->Clone();}
  
  TGraphErrors* getPhotonPlot(){return (TGraphErrors*)g_photon->Clone();}
  TF1* getRecoveryFit(){return (TF1*)f_recovery->Clone();}
  TGraphErrors* getRecoveryResidualPlot(){return (TGraphErrors*)g_recovery_residual->Clone();}

  //---- return arrays ----//
  double* getNobsMean(){return m_nobs_mean;}
  double* getNobsError(){return m_nobs_error;}
  double* getNrefMean(){return m_nref_mean;}
  double* getNrefError(){return m_nref_error;}

  double* getLinearResidualMean(){return m_linear_residual_mean;}
  double* getLinearResidualError(){return m_linear_residual_error;}
  double* getRecoveryResidualMean(){return m_recovery_residual_mean;}
  double* getRecoveryResidualError(){return m_recovery_residual_error;}


protected:
  int m_ch;
  double m_mppc_adc_gain;
  int m_max_cycle;
  bool m_is_debug;
  
  //==== parameters for recovery analysis ====//
  const double m_mppc_pixel = 3600.0; 
  const double m_led_width = 1000.0; // ns unit

  //==== anaAdc2Photon ====//
  void anaAdc2Photon();

  double m_calib_integral_mean[MAXCYCLE] = {};
  double m_ref_integral_mean[MAXCYCLE] = {};
  double m_calib_integral_error[MAXCYCLE] = {};
  double m_ref_integral_error[MAXCYCLE] = {};
  TGraphErrors *g_adc;
  TF1 *f_linear;

  int m_linear_fit_num = 5;

  double m_nobs_mean[MAXCYCLE] = {};
  double m_nref_mean[MAXCYCLE] = {};
  double m_nobs_error[MAXCYCLE] = {};
  double m_nref_error[MAXCYCLE] = {};
  TGraphErrors *g_photon;
  
  //==== anaRecovery ====//
  virtual void anaRecovery() = 0;
  double m_fit_range_min;
  double m_fit_range_max;
  TF1 *f_recovery;

  //==== setResidual ====//
  void setResidual(double* input_X, double* input_Y, TF1* f_compare, double* residual_array);
  double m_linear_residual_mean[MAXCYCLE] = {};
  double m_linear_residual_error[MAXCYCLE] = {};
  TGraphErrors *g_linear_residual;

  double m_recovery_residual_mean[MAXCYCLE] = {};
  double m_recovery_residual_error[MAXCYCLE] = {};
  TGraphErrors *g_recovery_residual;

};
#endif
