#include "../src/DatReader.cc"

void mcr_DatReader(string filename = "dummy"){
  if(filename == "dummy"){
    cout << "usage: root \'mcr_DatReader.cc(\"filename.dat\")\'" << endl;
    return 0;
  }

  DatReader *m = new DatReader(filename, false);;
  double *x, *y;
  int length;
  TGraph *g;
  TCanvas *c1 = new TCanvas(1);
  
  string flag;
  int event, module, ch;
  int max_event_num = m->getMaxEventNum();
  int max_module_num = m->getMaxModuleNum();
  int save_enable_mask;

  m -> showFileHeader(); 
  for(int i=0; i<10; i++){
    cout << "//---- input event and module ----//" << endl;
    cout << "event num (0~" << max_event_num-1 << "): " << endl;
    cin >> event;
    if(event<0 || event>=max_event_num){
      cout << "input event num is out of range" << endl;
      continue;
    }
    cout << "module num (0~" << max_module_num-1 <<"): " << endl;
    cin >> module;
    if(module<0 || module>=max_module_num){
      cout << "input module num is out of range" << endl;
      continue;
    }
    m -> getEvent(event,module);
    save_enable_mask = m-> getSaveEnableMask(module);
    cout << "draw ch (emable_mask; ch32<- " << bitset<32>(save_enable_mask) << " ->ch0): " << endl;
    cin >> ch;

    if( (y = m->getAdc(ch)) == nullptr ) continue;
    x = m->getClock();
    length = m->getCurrentClockLength();

    g = new TGraph(length, x, y);
    g -> Draw("apl");
    c1 -> Update();
    gSystem -> ProcessEvents();
    cout << "Do you continue? Type \"y\" to continue." << endl;
    cin >> flag;
    if(flag!="y") break;
    delete g;
    }

}
