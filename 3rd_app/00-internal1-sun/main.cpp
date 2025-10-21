#include <QApplication>
#include "SingleInstance_SharedMemory.h"

#include "net_mainwindow.h"

int main(int argc, char* argv[]) {




  QApplication a(argc, argv);



  SingleInstance_SharedMemory guard("com.yourco.Net_Router_509");
  if (!guard.ensureSingle()) {
      // 已有实例在运行
      return 0;
  }

  a.setStyle("fusion");  // looks good with dark color scheme

  net_mainwindow w;
  w.show();
  return a.exec();


}
