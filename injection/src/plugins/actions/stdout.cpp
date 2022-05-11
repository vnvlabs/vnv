

#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#include "VnV.h"


class StdoutAction : public VnV::IAction {
  
  int pipefd[2];
  int stdout_bk; 
  char buf[101];
  VnV::MetaData meta;
  void _read() {
      int n = -1;
      while ( (n = read(pipefd[0],buf,100)) > 0 ) {
          write(stdout_bk,buf,n);
          std::string s(buf,n);
          getEngine()->Put("buff",s, meta );
      }

  }

public:

  StdoutAction() {
    implements_injectionPointEnd = true;
    implements_injectionPointIter = true;
    implements_injectionPointStart = true;
  }

  void initialize() override {
      pipe2(pipefd,O_NONBLOCK);
      stdout_bk = dup(fileno(stdout));
      dup2(pipefd[1], fileno(stdout));
      meta["proc"] = std::to_string(getComm()->Rank());      
  };

  void injectionPointStart(std::string packageName, std::string id)override {
    _read();
  };  

  void injectionPointIteration(std::string stageId) override {
    _read();
  };

  void injectionPointEnd() override {
    _read();
  };

  void finalize() override{
    _read();
    close(pipefd[0]);
    dup2(stdout_bk, pipefd[1]);
  }
};


/**
 * Standard Output Captured During Execution
 * -----------------------------------------
 * 
 * .. vnv-terminal::
 * 
 *     {{vnv_join("",buff)}}
 * 
 * 
 * .. note:: 
 *   
 *    Standard Output is only captured between VnV Initialization and VnV Finalization calls. 
 * 
 */
INJECTION_ACTION(VNVPACKAGENAME, stdout, "{\"type\":\"object\"}") {
  return new StdoutAction();
}