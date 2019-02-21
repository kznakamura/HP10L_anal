#include "../src/DatReader.cc"

void mcr_DatReader(string filename = "dummy"){
  if(filename == "dummy"){
    cout << "usage: root \'mcr_DatReader.cc(\"filename.dat\")\'" << endl;
    return 0;
  }

  DatReader *m = new DatReader(filename);;
  double *x, *y;
  int length;
  TGraph *g;
  TCanvas *c1 = new TCanvas(1);
  
  string flag;
  int event, module, ch;
  int max_event_num = m->getMaxEventNum();
  int max_module_num = m->getMaxModuleNum();
  int current_maxch_num;

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
    current_maxch_num = m-> getCurrentMaxchNum();
    cout << "draw ch (0~" << current_maxch_num-1 << "): " << endl;
    cin >> ch;
    if(ch<0 || ch>= current_maxch_num){
      cout << "input ch num is out of range" << endl;
      continue;
    }
    x = m->getClock();
    y = m->getAdc(ch);
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
