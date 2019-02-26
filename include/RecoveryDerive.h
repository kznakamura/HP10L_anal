#ifndef RECOVERY_DERIVE_H
#define RECOVERY_DERIVE_H

#include "RecoveryBase.h"

class RecoveryOnepar:public  RecoveryBase{
public:
  RecoveryOnepar(int ch, double mppc_adc_gain, int max_cycle,
		 double* calib_integral_mean, double* ref_integral_mean,
		 double* calib_integral_error=nullptr, double* ref_integral_error=nullptr,
		 double fit_range_min=1.0e5, double fit_range_max=1.0e5, const bool is_debug=false);

  double getTau(){return m_tau;}
  double getSlope(){return m_slope;}
  
private:
  void anaRecovery();
  double m_tau;
  double m_slope;
};

class RecoveryTwopar:public  RecoveryBase{
public:
  RecoveryTwopar(int ch, double mppc_adc_gain, int max_cycle,
		 double* calib_integral_mean, double* ref_integral_mean,
		 double* calib_integral_error=nullptr, double* ref_integral_error=nullptr,
		 double fit_range_min=1.0e5, double fit_range_max=1.0e5, const bool is_debug=false);
  
  double getTau1(){return m_tau1;}
  double getTau2(){return m_tau2;}
  double getAlpha(){return m_alpha;}
  double getBeta(){return m_beta;}
  
private:
  void anaRecovery();
  double m_tau1;
  double m_tau2;
  double m_alpha;
  double m_beta;
};

#endif
