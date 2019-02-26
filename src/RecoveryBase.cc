#include "../include/RecoveryBase.h"
#include <iostream>
#include <utility>
#include <string>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TApplication.h>

using namespace std;

RecoveryBase::RecoveryBase(int ch, double mppc_adc_gain, int max_cycle, 
				   double* calib_integral_mean, double* ref_integral_mean, 
				   double* calib_integral_error, double* ref_integral_error,
				   double fit_range_min, double fit_range_max, const bool is_debug){

  
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
  m_fit_range_min = fit_range_min;
  m_fit_range_max = fit_range_max;
  m_is_debug = is_debug;
  f_recovery = nullptr;
 

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
}

RecoveryBase::~RecoveryBase(){
  delete g_adc;
  delete f_linear;
  delete g_photon;
  delete g_linear_residual;
  delete g_recovery_residual;
  if(m_is_debug){
    cout << "# debug: RecoveryBase is deleted" << endl;
  }
}

void RecoveryBase::anaAdc2Photon(){  

  g_adc = new TGraphErrors(m_max_cycle, 
			   m_ref_integral_mean, m_calib_integral_mean, 
			   m_ref_integral_error, m_calib_integral_error);
  f_linear = new TF1(Form("f_linear_ch%d",m_ch), "[0]*x+[1]", 0, 1.0e5);
  f_linear -> SetParameters(10,0);
  g_adc -> Fit(Form("f_linear_ch%d",m_ch),"Q0","",0,m_ref_integral_mean[m_linear_fit_num]);
  double slope = f_linear -> GetParameter(0);
  double seppen = f_linear -> GetParameter(1);

  setResidual(m_calib_integral_mean, m_ref_integral_mean, f_linear, m_linear_residual_mean);
  setResidual(m_calib_integral_mean, m_ref_integral_error, f_linear, m_linear_residual_error);
  
  g_linear_residual = new TGraphErrors(m_max_cycle, 
				       m_ref_integral_mean, m_linear_residual_mean, 
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


void RecoveryBase::setResidual(double* input_X, double* input_Y, TF1* f_compare, double* residual_array){
  for(int cy=0; cy<m_max_cycle; cy++){
    residual_array[cy] = input_Y[cy]/f_compare->Eval(input_X[cy]);
  }
}
