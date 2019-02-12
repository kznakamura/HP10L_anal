#ifndef DATREADER_H
#define DATREADER_H

#include <fstream>
#include <vector>

const int MAXMODULE=8;

class DatReader{
public:
  //==== Constructor and Destructor ==== //
  DatReader(std::string filename, const bool is_debug=false);
  DatReader();
  ~DatReader();

  //=== debug ===//
  void setDebug(bool is_debug){m_is_debug=is_debug;}

  //==== file reading ====//
  bool openFile(std::string filename);
  std::string getFilename(){return m_filename;}

  //==== get data ====//
  //---- get wave ----//
  bool getEvent(int event_num, int read_module_num);
  int getCurrentEvent(){return m_current_event_num;}
  int getCurrentModule(){return m_current_module_num;}
  double* getAdc(int read_ch);
  double* getClock();
  int getCurrentClockLength(){return m_current_clock_length;}
  int getCurrentMaxchNum(){return m_current_maxch_num;}

  //---- get file header ----//
  int getMaxEventNum(){return m_max_event_num;}
  int getMaxModuleNum(){return m_max_module_num;}
  int getModelId(int read_module_num){return m_model_id.at(read_module_num);}
  int getSerialNum(int read_module_num){return m_serial_num.at(read_module_num);}
  int getAdcBitNum(int read_module_num){return m_adc_bit_num.at(read_module_num);}
  int getReadChNum(int read_module_num){return m_read_ch_num.at(read_module_num);}
  int getSamplingUs(int read_module_num){return m_sampling_us.at(read_module_num);}
  int getPostTriggerRate(int read_module_num){return m_post_trigger_rate.at(read_module_num);}
  int getTriggerEdge(int read_module_num){return m_trigger_edge.at(read_module_num);}
  int getExtTriggerMode(int read_module_num){return m_ext_trigger_mode.at(read_module_num);}
  int getRecordLength(int read_module_num){return m_record_length.at(read_module_num);}
  int getSaveEnableMask(int read_module_num){return m_save_enable_mask.at(read_module_num);}
  bool showFileHeader();
  
  //---- get event header ----//
  int getEventId(){return m_event_id;}
  int getEventUnixtime(){return m_event_unixtime;}

  //---- get module header ----//
  int getBoardId(){return m_board_id;}
  
private:
  std::string m_filename;
  bool checkFile();
  bool m_is_debug;

  std::ifstream *m_fin;
  
  //---- file header ----//
  int m_max_event_num;
  int m_max_module_num;
  std::vector<int> m_model_id;
  std::vector<int> m_serial_num;
  std::vector<int> m_adc_bit_num;
  std::vector<int> m_read_ch_num;
  std::vector<int> m_sampling_us;
  std::vector<int> m_post_trigger_rate;
  std::vector<int> m_trigger_edge;
  std::vector<int> m_ext_trigger_mode;
  std::vector<int> m_record_length;
  std::vector<int> m_save_enable_mask;
  bool readFileHeader();

  //---- event header ----//
  int m_event_id;
  int m_event_unixtime;
  bool readEventHeader(int event_num);
  
  //---- module header and data ----//
  int m_board_id;
  short *m_module_data;
  double *m_adc;
  double *m_clock;
  int m_current_event_num;
  int m_current_module_num;
  int m_current_clock_length;
  int m_current_maxch_num;
  
  bool readModuleData(int event_num, int read_module_num);
  bool checkEvent();
    
  //---- data format (byte unit) ---//
  const int m_file_header_size = 2560;
  const int m_event_header_size = 16;
  const int m_module_header_size = 144;
  std::vector<int> m_module_data_size;
  int m_event_size = 0;
  void setDataFormat();
  
};

#endif
