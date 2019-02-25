#include "../include/RecoveryAnalizer.h"
#include <iostream>
#include <utility>
#include <string>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TApplication.h>

using namespace std;

RecoveryAnalizer::RecoveryAnalizer(int ch, double mppc_adc_gain, int max_cycle, 
				   double* calib_integral_mean, double* ref_integral_mean, 
				   double* calib_integral_error, double* ref_integral_error,
				   double fit_range_min, double fit_range_max){
				   
  m_ch = ch;
  if(mppc_adc_gain < 0.1){
    cerr << "# Error : input mppc_adc_gain is too small" << endl;
    exit(-1);
  }
  m_mppc_adc_gain = mppc_adc_gain;
  if(max_cycle>::MAXCYCLE){
    cout << "# Error: "
	 << " input max_cycle is bigger than MAXCYCLE=" << ::MAXCYCLE
	 << endl;
    exit(-1);
  }
  m_max_cycle = max_cycle;
  for(int cy=0; cy<m_max_cycle; cy++){
    m_calib_integral_mean[cy] = calib_integral_mean[cy];
    m_ref_integral_mean[cy] = ref_integral_mean[cy];
    if(calib_integral_error!=nullptr && ref_integral_error!=nullptr){
      m_calib_integral_error[cy] = calib_integral_error[cy];
      m_ref_integral_error[cy] = ref_integral_error[cy];
    }else{
      for(int cy=0; cy<m_max_cycle; cy++){
	m_calib_integral_error[cy] = 0.0;
	m_calib_integral_error[cy] = 0.0;
      }
    }
  }

  anaAdc2Photon();
  anaRecovery(fit_range_min, fit_range_max);
}

RecoveryAnalizer::~RecoveryAnalizer(){
  delete g_adc;
  delete f_linear;
  delete g_linear_residual;
  delete g_photon;
  delete g_onepar_residual;
  delete g_twopar_residual;
}

void RecoveryAnalizer::anaAdc2Photon(){  

  g_adc = new TGraphErrors(m_max_cycle, 
			   m_ref_integral_mean, m_calib_integral_mean, 
			   m_ref_integral_error, m_calib_integral_error);
  f_linear = new TF1(Form("f_linear_ch%d",m_ch), "[0]*x+[1]", 0, 1.0e5);
  f_linear -> SetParameters(10,0);
  g_adc -> Fit(Form("f_linear_ch%d",m_ch),"Q0","",0,m_ref_integral_mean[m_linear_fit_num]);
  double slope = f_linear -> GetParameter(0);
  double seppen = f_linear -> GetParameter(1);

  for(int cy=0; cy<m_max_cycle; cy++){
    m_linear_residual[cy] = m_calib_integral_mean[cy]/f_linear->Eval(m_ref_integral_mean[cy]);
    m_linear_residual_error[cy] = m_calib_integral_error[cy]/f_linear->Eval(m_ref_integral_mean[cy]);
  }

  g_linear_residual = new TGraphErrors(m_max_cycle, 
				       m_ref_integral_mean, m_linear_residual, 
				       m_ref_integral_error, m_linear_residual_error);

  for(int cy=0; cy<m_max_cycle; cy++){
    m_nobs_mean[cy] = (m_calib_integral_mean[cy]-seppen)/m_mppc_adc_gain;
    m_nref_mean[cy] = (m_ref_integral_mean[cy] * slope)/m_mppc_adc_gain;
    m_nobs_error[cy] = (m_calib_integral_error[cy])/m_mppc_adc_gain;
    m_nref_error[cy] = (m_ref_integral_error[cy] * slope)/m_mppc_adc_gain;
  }
  g_photon = new TGraphErrors(m_max_cycle, 
			      m_nref_mean, m_nobs_mean,
			      m_nref_error, m_nref_error);
}

void RecoveryAnalizer::anaRecovery(double fit_range_min, double fit_range_max){

  if(fit_range_max > m_nref_mean[m_max_cycle-1]){
    cout << "# Message: "
	 << "input fit_ramge_max is changed to the maximum of m_nref_mean. "
	 << endl;
    fit_range_max = m_nref_mean[m_max_cycle-1];
  }
  m_fit_range_min = fit_range_min;
  m_fit_range_max = fit_range_max;

  f_onepar = new TF1("f_onepar", onepar_formula.c_str(),0, 1.0e5);
  f_onepar -> SetParameters(50/m_mppc_pixel/m_led_width,1);
  g_photon -> Fit("f_onepar", "Q0", "", m_fit_range_min, m_fit_range_max);
 
  f_twopar = new TF1("f_twopar", twopar_formula.c_str(),0, 1.0e5);
  f_twopar -> SetParameters(50/m_mppc_pixel/m_led_width,
			    100/m_mppc_pixel/m_led_width, 
			    0.5, 0.5);
  f_twopar -> SetParLimits(2, 0, 1);
  f_twopar -> SetParLimits(3, 0, 1);
  g_photon -> Fit("f_twopar", "Q0", "", m_fit_range_min, m_fit_range_max);
  
  m_onepar_tau = (f_onepar -> GetParameter(0))*m_mppc_pixel*m_led_width;
  m_onepar_slope = (f_onepar -> GetParameter(1));
  m_twopar_tau1 = (f_twopar -> GetParameter(0))*m_mppc_pixel*m_led_width;
  m_twopar_tau2 = (f_twopar -> GetParameter(1))*m_mppc_pixel*m_led_width;
  m_twopar_alpha = (f_twopar -> GetParameter(2));
  m_twopar_beta = (f_twopar -> GetParameter(3));
  if(m_twopar_tau1>m_twopar_tau2){
    swap(m_twopar_tau1, m_twopar_tau2);
    swap(m_twopar_alpha, m_twopar_beta);
  }

  for(int cy=0; cy<m_max_cycle; cy++){
    m_onepar_residual[cy] = m_nobs_mean[cy]/f_onepar->Eval(m_nref_mean[cy]);
    m_twopar_residual[cy] = m_nobs_mean[cy]/f_twopar->Eval(m_nref_mean[cy]);
    m_onepar_residual_error[cy] = m_nobs_error[cy]/f_onepar->Eval(m_nref_mean[cy]);
    m_twopar_residual_error[cy] = m_nobs_error[cy]/f_twopar->Eval(m_nref_mean[cy]);
  }
  
  g_onepar_residual = new TGraphErrors(m_max_cycle, 
				       m_nref_mean, m_onepar_residual, 
				       m_nref_error, m_onepar_residual_error);
  g_twopar_residual = new TGraphErrors(m_max_cycle, 
				       m_nref_mean, m_twopar_residual, 
				       m_nref_error, m_twopar_residual_error);
}

