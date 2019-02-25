#ifndef RECOVERY_ANALIZER_H
#define RECOVERY_ANALIZER_H

#include <string>
#include <TGraphErrors.h>
#include <TF1.h>

namespace{
  const int MAXCYCLE = 50;
}

class RecoveryAnalizer{
public:
  RecoveryAnalizer(int ch, double mppc_adc_gain, int max_cycle,
		   double* calib_integral_mean, double* ref_integral_mean,
		   double* calib_integral_error=nullptr, double* ref_integral_error=nullptr,
		   double fit_range_min=1.0e5, double fit_range_max=1.0e5);
  
  ~RecoveryAnalizer();

  //---- return recovery-time parameters ----//
  double getOneparTau(){return m_onepar_tau;}
  double getOneparSlope(){return m_onepar_slope;}
  double getTwoparTau1(){return m_twopar_tau1;}
  double getTwoparTau2(){return m_twopar_tau2;}
  double getTwoparAlpha(){return m_twopar_alpha;}
  double getTwoparBeta(){return m_twopar_beta;}

  //---- return root objects ----//
  TGraphErrors* getAdcPlot(){return (TGraphErrors*)g_adc->Clone();}
  TF1* getLinearFit(){return (TF1*)f_linear->Clone();}
  TGraphErrors* getLinearResidual(){return (TGraphErrors*)g_linear_residual->Clone();}
  
  TGraphErrors* getPhotonPlot(){return (TGraphErrors*)g_photon->Clone();}
  TF1* getOneparFit(){return (TF1*)f_onepar->Clone();}
  TF1* getTwoparFit(){return (TF1*)f_twopar->Clone();}
  TGraphErrors* getOneparResidual(){return (TGraphErrors*)g_onepar_residual->Clone();}
  TGraphErrors* getTwoparResidual(){return (TGraphErrors*)g_twopar_residual->Clone();}
  

private:
  int m_ch;
  double m_mppc_adc_gain;
  int m_max_cycle;
  
  //==== parameters for recovery analysis ====//
  const double m_mppc_pixel = 3600.0; 
  const double m_led_width = 1000.0; // ns unit
  const std::string onepar_formula = "x*[1]/(1+[0]*x)";
  const std::string twopar_formula = "x*[2]/(1+[0]*x)+x*[3]/(1+[1]*x)";

  //==== anaAdc2Photon ====//
  void anaAdc2Photon();

  double m_calib_integral_mean[MAXCYCLE] = {};
  double m_ref_integral_mean[MAXCYCLE] = {};
  double m_calib_integral_error[MAXCYCLE] = {};
  double m_ref_integral_error[MAXCYCLE] = {};
  TGraphErrors *g_adc;
  TF1 *f_linear;

  int m_linear_fit_num = 5;
  double m_linear_residual[MAXCYCLE] = {};
  double m_linear_residual_error[MAXCYCLE] = {};
  TGraphErrors *g_linear_residual;

  double m_nobs_mean[MAXCYCLE] = {};
  double m_nref_mean[MAXCYCLE] = {};
  double m_nobs_error[MAXCYCLE] = {};
  double m_nref_error[MAXCYCLE] = {};
  TGraphErrors *g_photon;

  //==== anaRecovery ====//
  void anaRecovery(double fit_range_min, double fit_range_max);

  double m_fit_range_min;
  double m_fit_range_max;

  TF1 *f_onepar;
  TF1 *f_twopar;  
  double m_onepar_tau;
  double m_onepar_slope;
  double m_twopar_tau1, m_twopar_tau2;
  double m_twopar_alpha, m_twopar_beta;

  double m_onepar_residual[MAXCYCLE] = {};
  double m_twopar_residual[MAXCYCLE] = {};
  double m_onepar_residual_error[MAXCYCLE] = {};
  double m_twopar_residual_error[MAXCYCLE] = {};
  TGraphErrors *g_onepar_residual;
  TGraphErrors *g_twopar_residual;
  
};
#endif
