#include "../include/DatReader.h" 
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

DatReader::DatReader(string filename, const bool is_debug){
  
  m_filename = filename;
  m_is_debug = is_debug;

  m_fin = nullptr;
  m_module_data = nullptr;
  m_adc = nullptr;
  m_clock = nullptr;
  
  if(!openFile(filename)){
    cerr << "cannot open file" << endl;
    exit(-1);
  }

  if(m_is_debug){
    cout << "# debug: DatReader with binary file is created" << endl;
  }
}

DatReader::DatReader(){

  m_is_debug = false;

  m_fin = nullptr;
  m_module_data = nullptr;
  m_adc = nullptr;
  m_clock = nullptr;

  if(m_is_debug){
    cout << "# debug: DatReader is created" << endl;
  }
}


DatReader::~DatReader(){
  m_fin -> close();
  delete m_fin;
  delete[] m_module_data;
  delete[] m_adc;
  delete[] m_clock;

  if(m_is_debug){
    cout << "# debug: DatReader is deleted" << endl;
  }

}


bool DatReader::openFile(string filename){

  if(m_fin!=nullptr){
    cerr << "# Error: "
	 <<"\""<< m_filename << "\" "
	 <<"is already opened" << endl;
    return false;
  }

  m_fin = new ifstream(filename.c_str());
  if(!m_fin->good()){
    cerr << "# Error: failed to open "
	 << "\"" << m_filename << "\"" << endl;
    m_fin = nullptr;
    return false;
  }


  m_current_event_num = -1;
  m_current_module_num = 0;
  m_current_clock_length = 0;
  m_current_maxch_num = 0;

  if(m_is_debug){
    cout << "# debug: opened binary file "
	 << "\"" << filename << "\"" << endl;
  }

  if(!readFileHeader()){
    return false;
  }

  return true;
}


bool DatReader::readFileHeader(){
  
  int file_header1[128] = {};
  m_fin -> seekg(0, ios_base::beg);
  m_fin -> read((char*)file_header1, sizeof(file_header1));
  
  m_max_event_num = file_header1[0];
  m_max_module_num = file_header1[1];
  
  for(int module_num=0; module_num<m_max_module_num; module_num++){
    m_model_id.push_back( file_header1[8+module_num] );
    m_serial_num.push_back( file_header1[16+module_num] );
    m_adc_bit_num.push_back( file_header1[24+module_num] );
    m_read_ch_num.push_back( file_header1[32+module_num] );
    m_sampling_us.push_back( file_header1[40+module_num] );
    m_post_trigger_rate.push_back( file_header1[64+module_num] );
    m_trigger_edge.push_back( file_header1[72+module_num] );
    m_ext_trigger_mode.push_back( file_header1[80+module_num] );
    m_record_length.push_back( file_header1[88+module_num] );
    m_save_enable_mask.push_back( file_header1[96+module_num] );
  }
      
  //---- check dummy of header ----//
  for(int module_num=0; module_num<::MAXMODULE; module_num++){
    if(file_header1[120+module_num] != 511){
      cerr << "# Error :File header format is broken" << endl;
      return false;
    }
  }

  setDataFormat();
  
  if(m_is_debug){
    cout << "# debug: File header is read" << endl;
  }
  return true;
}

bool DatReader::showFileHeader(){

    if(!checkFile()){
    return false;
  }
  
  cout << "\n";
  cout << "//------------ File Header List ----------//" << endl;
  int width = 25;
  cout << left;
  cout << setw(width) << "File event num" << "| " 
       << m_max_event_num << "\n";
  cout << setw(width) << "Module num"     << "| " 
       << m_max_module_num << "\n";
  
  for(int module_num=0; module_num<m_max_module_num; module_num++){
    cout << "\n"
	 << "<< module: " << module_num << " >>" << endl;
    cout << setw(width) << "Model ID"     << "| " 
	 << m_model_id.at(module_num) << "\n";
    cout << setw(width) << "Serial num"   << "| " 
	 << m_serial_num.at(module_num) << "\n";
    cout << setw(width) << "ADC bit num"  << "| " 
	 << m_adc_bit_num.at(module_num) << " bit\n";
    cout << setw(width) << "Module ch"    << "| " 
	 << m_read_ch_num.at(module_num) << " ch\n";
    cout << setw(width) << "Sampling period" << "| " 
	 << m_sampling_us.at(module_num) << " ns\n";
    cout << setw(width) << "Post trigger" << "| " 
	 << m_post_trigger_rate.at(module_num) << " \%\n";
    cout << setw(width) << "Trigger edge" << "| " 
	 << m_trigger_edge.at(module_num) << "\n";
    cout << setw(width) << "Ext trigger mode" << "| " 
	 << m_ext_trigger_mode.at(module_num) << "\n";
    cout << setw(width) << "Record length" << "| " 
	 << m_record_length.at(module_num) << " clock\n";
    cout << setw(width) << "Save enable mask" << "| " 
	 << "ch32<- " 
	 << bitset<32>(m_save_enable_mask.at(module_num)) 
	 << " ->ch0" 
	 << "\n";
  }

    cout << "\n";
    cout << "//------------ End of the List ----------//" << endl;
  
  return true;
    
}

void DatReader::setDataFormat(){
  
  for(int module_num=0; module_num<m_max_module_num; module_num++){
    m_module_data_size.push_back( 
				 sizeof(short) 
				 *__builtin_popcount(m_save_enable_mask.at(module_num))
				 *m_record_length.at(module_num) 
				  );
  }
  
  m_event_size = 0;
  m_event_size += m_event_header_size;

  for(int module_num=0; module_num<m_max_module_num; module_num++){
    m_event_size += m_module_header_size + m_module_data_size.at( module_num );
  }
  
  if(m_is_debug){
    cout << "# debug: m_event_size=" << m_event_size << endl;
  }
}


bool DatReader::readEventHeader(int event_num){

  if(event_num<0 || event_num>=m_max_event_num){
    cerr << "# Error: input event_num is out of range" << endl;
    return false;
  }

  int event_header[4] = {};
  m_fin -> seekg( m_file_header_size + event_num * m_event_size , ios_base::beg);
  m_fin -> read((char*)event_header, sizeof(event_header));

  //---- chack dummy of header ----//
  if(event_header[2]!=500 || event_header[3]!=500){
    cerr << "# Error: event header format is broken" << endl;
    return false;
  } 

  m_event_id = event_header[0];
  m_event_unixtime = event_header[1];

  if(m_is_debug){
    cout << "# debug: event header is read" << endl;
  }
  return true;
}



bool DatReader::readModuleData(int event_num, int read_module_num){

  if(event_num<0 || event_num>=m_max_event_num){ 
    cerr << "# Error: input event_num is out of range" << endl;
    return false;
  }else if(read_module_num<0 || read_module_num>=m_max_module_num){
    cerr << "# Error: input read_module_num is out of range" << endl;
    return false;
  }

  int file_pointer 
    = m_file_header_size 
    + event_num * m_event_size 
    + m_event_header_size;

  for(int module_num=0; module_num<read_module_num; module_num++){
    file_pointer += m_module_header_size + m_module_data_size.at(module_num);
  }
  
  int module_header[36] = {};
  m_fin -> seekg( file_pointer, ios_base::beg );
  m_fin -> read((char*)module_header, sizeof(module_header));

  //---- chack dummy of header ----//
  if(module_header[3]!=500){
    cerr << "# Error: Module header format is broken" << endl;
    return false;
  }

  m_board_id = module_header[0];
  if(m_is_debug){
    cout << "# debug: Module header is read" << endl;
  }

  if(m_module_data!=nullptr){
    delete[] m_module_data;
    delete[] m_adc;
    delete[] m_clock;
    delete[] m_time;
    m_module_data = nullptr;
    m_adc = nullptr;
    m_clock = nullptr;
    m_time = nullptr;
    if(m_is_debug){
      cout << "# debug: m_module_data is deleted" << endl;
    }
  }
  
  int array_size = m_record_length.at(read_module_num)*m_read_ch_num.at(read_module_num);
  m_module_data = new short[array_size];
  
  m_fin -> read((char*)m_module_data, m_module_data_size.at(read_module_num));
      
  m_adc = new double[array_size];
  m_clock = new double[array_size];
  m_time = new double[array_size];
  for(int sample=0; sample<array_size; sample++){
    m_adc[sample] = (double)m_module_data[sample];
    m_clock[sample] = (double)sample;
    m_time[sample] = (double)(m_sampling_us.at(read_module_num)*sample);
  }
  
  return true;
}

bool DatReader::getEvent(int event_num, int read_module_num){

  if(!checkFile()){
    return false;
  }

  if(!readEventHeader(event_num)){
    cerr << "# Error: cannot read event header" << endl;
    m_current_event_num = -1;
    return false;
  }
  if(!readModuleData(event_num, read_module_num)){
    cerr << "# Error: cannot read module data" << endl;
    m_current_module_num = 0;
    return false;
  }
  
  m_current_event_num = event_num;
  m_current_module_num = read_module_num;
  m_current_clock_length = m_record_length.at(read_module_num);
  m_current_maxch_num = m_read_ch_num.at(read_module_num);
  m_current_bit_num = m_adc_bit_num.at(read_module_num);

  return true;
}

double* DatReader::getAdc(int read_ch){

  if(!checkFile() || !checkEvent()){
    return nullptr;
  }
  
  if( read_ch<0 || !((m_save_enable_mask.at(m_current_module_num)>>read_ch) & 0b1) ){
    cerr << "# Error: read_ch is out of range" << endl;
    return nullptr;
  }

  int zero_bit_num = 0;
  for(int i=0; i<read_ch; i++){
    if( !( m_save_enable_mask.at(m_current_module_num)>>i & 0b1 ) ){
      zero_bit_num++;
    }
  }
  if(m_is_debug){
    cout << "# debug: zero_bit_num = " << zero_bit_num << endl;
  }
  
  return m_adc+m_current_clock_length*(read_ch-zero_bit_num);
}

double* DatReader::getClock(){  
  
  if(!checkFile() || !checkEvent()){
    return nullptr;
  }

  return m_clock;
}

double* DatReader::getTime(){

 if(!checkFile() || !checkEvent()){
    return nullptr;
  }

  return m_time;
}

bool DatReader::checkFile(){
  
  if(m_fin==nullptr){
    cerr << "# Error: use function \"openFile\" before using this function" << endl;
    return false;
  }

  return true;
 }
  
bool DatReader::checkEvent(){

  if(m_current_event_num == -1){
    cerr << "# Error: use function \"getEvent\" befor using this function" << endl;
    return false;
  }
  
  return true;
}
    
