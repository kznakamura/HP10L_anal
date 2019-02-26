#include "RecoveryDerive.h"


using namespace std;

//==== One parameter fit ====//

RecoveryOnepar::RecoveryOnepar(int ch, double mppc_adc_gain, int max_cycle,
			       double* calib_integral_mean, double* ref_integral_mean,
			       double* calib_integral_error, double* ref_integral_error,
			       double fit_range_min, double fit_range_max, const bool is_debug)
  :RecoveryBase(ch, mppc_adc_gain, max_cycle,
		    calib_integral_mean, ref_integral_mean,
		    calib_integral_error, ref_integral_error,
		    fit_range_min, fit_range_max, is_debug){

  anaRecovery();
}

void RecoveryOnepar::anaRecovery(){  
  f_recovery = new TF1("f_recovery", "x*[1]/(1+[0]*x)",0, 1.0e5);
  f_recovery -> SetParameters(50/m_mppc_pixel/m_led_width,1);
  g_photon -> Fit("f_recovery", "Q0", "", m_fit_range_min, m_fit_range_max);
  
  m_tau = (f_recovery -> GetParameter(0))*m_mppc_pixel*m_led_width;
  m_slope = (f_recovery -> GetParameter(1));
  
  setResidual(m_nref_mean, m_nobs_mean, f_recovery, m_recovery_residual_mean);
  setResidual(m_nref_mean, m_nobs_error, f_recovery, m_recovery_residual_error);

  g_recovery_residual = new TGraphErrors(m_max_cycle, 
					 m_nref_mean, m_recovery_residual_mean, 
					 m_nref_error, m_recovery_residual_error);

}
//==== end ====//


//==== Two parameter fit ====//

RecoveryTwopar::RecoveryTwopar(int ch, double mppc_adc_gain, int max_cycle,
			       double* calib_integral_mean, double* ref_integral_mean,
			       double* calib_integral_error, double* ref_integral_error,
			       double fit_range_min, double fit_range_max, const bool is_debug)
  :RecoveryBase(ch, mppc_adc_gain, max_cycle,
		calib_integral_mean, ref_integral_mean,
		calib_integral_error, ref_integral_error,
		fit_range_min, fit_range_max, is_debug){
  
  anaRecovery();
}

void RecoveryTwopar::anaRecovery(){
  f_recovery = new TF1("f_recovery", "x*[2]/(1+[0]*x)+x*[3]/(1+[1]*x)",0, 1.0e5);
  f_recovery -> SetParameters(50/m_mppc_pixel/m_led_width,
			      100/m_mppc_pixel/m_led_width, 
			      0.5, 0.5);
  
  g_photon -> Fit("f_recovery", "Q0", "", m_fit_range_min, m_fit_range_max);

  m_tau1 = (f_recovery -> GetParameter(0))*m_mppc_pixel*m_led_width;
  m_tau2 = (f_recovery -> GetParameter(1))*m_mppc_pixel*m_led_width;
  m_alpha = (f_recovery -> GetParameter(2));
  m_beta = (f_recovery -> GetParameter(3));

  if(m_tau1>m_tau2){
    swap(m_tau1, m_tau2);
    swap(m_alpha, m_beta);
  }

  setResidual(m_nref_mean, m_nobs_mean, f_recovery, m_recovery_residual_mean);
  setResidual(m_nref_mean, m_nobs_error, f_recovery, m_recovery_residual_error);  

  g_recovery_residual = new TGraphErrors(m_max_cycle, 
					 m_nref_mean, m_recovery_residual_mean, 
					 m_nref_error, m_recovery_residual_error);
  
}
//==== end ====//
