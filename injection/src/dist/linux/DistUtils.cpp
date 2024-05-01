#include "shared/DistUtils.h"
#include "shared/exceptions.h"
#include "dist/DistUtils.h"
#include <dirent.h>
#include <errno.h>
#include <link.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <sstream>

namespace VnV
{
  namespace DistUtils
  {

    class ActualProcess
    {
    public:
      pid_t pid;

      ActualProcess(std::string command)
      {
        pid_t child_pid;

        if ((child_pid = fork()) == -1)
        {
          perror("fork");
          exit(1);
        }

        /* child process */
        if (child_pid == 0)
        {
          setpgid(child_pid, child_pid); // Needed so negative PIDs can kill children of /bin/sh

          // Write the command to file as a temporary file.
          std::string s = getTempFile();
          std::ofstream ofs(s);
          ofs << "#!/bin/env bash \n\n"
              << command << "\n\n";
          ofs.close();
          permissions(s, true, true, true); // Not sure if needed.
          execl(s.c_str(), s.c_str(), (char *)NULL);
          _exit(0);
        }
        else
        {
        }

        pid = child_pid;
      }

      int wait()
      {
        if (pid > -1)
        {
          int stat;
          while (waitpid(pid, &stat, 0) == -1)
          {
            if (errno != EINTR)
            {
              stat = -1;
              break;
            }
          }
          return stat;
        }
        return 0;
      }

      void cancel()
      {
        kill(pid, 9);
        pid = -1;
      }

      ~ActualProcess() { wait(); }
    };

    class LinuxProcess : public VnVProcess
    {
      int exitStatus = 0;
      std::string stdo = "";
      std::string stde = "";
      std::string cfile = StringUtils::random(10);
      std::unique_ptr<ActualProcess> process;

    public:
      LinuxProcess(std::string cmd)
      {
        // Put it all in a script -- All stdout will be piped through
        // to a file. The exit status is piped to another file.

        // Get a unique file name
        while (fileExists(getTempFile(cfile)))
        {
          cfile = StringUtils::random(10);
        }

        // Write the users command into a new script file.
        std::string cfn = getTempFile(cfile);
        std::ofstream cf(cfn);
        cf << cmd;
        cf.close();

        // Make it executable
        permissions(cfn, true, true, true);

        // Execute it, mapping stdout, stderr and the exit status to cfile.stderr, cfile.stdout and cfile.stdexit
        std::ostringstream oss;
        oss << cfn << " 1>" << getTempFile(cfile, ".stdout") << " 2>" << getTempFile(cfile, ".stderr") << "; echo $? > "
            << getTempFile(cfile, ".exit");
        process.reset(new ActualProcess(oss.str()));
      }

      int getExitStatus() override
      {
        wait();
        return exitStatus;
      }

      std::string getStdout() override
      {
        wait();
        return stdo;
      }
      std::string getStdError() override
      {
        wait();
        return stde;
      }

      virtual void wait() override
      {
        if (process != nullptr)
        {
          process->wait(); // Calls destructor which waits for process to end.
          stdo = rfile(getTempFile(cfile, ".stdout"));
          stde = rfile(getTempFile(cfile, ".stderr"));
          exitStatus = std::atoi(rfile(getTempFile(cfile, ".exit")).c_str());
        }
      }

      std::string rfile(std::string filename)
      {
        std::ifstream inFile;
        inFile.open(filename.c_str()); // open the input file
        std::stringstream strStream1;
        strStream1 << inFile.rdbuf(); // read the file
        return strStream1.str();      // str holds the content of the file
      }

      virtual void cancel() override
      {
        if (process != nullptr)
        {
          process->cancel();
        }
      }

      // Still running if we cant find complete.
      virtual bool running() override
      {
        std::ifstream df(getTempFile(cfile, ".exit"));
        if (!df.good())
        {
          return true;
        }
        return false;
      };

      virtual ~LinuxProcess() {}
    };

    std::shared_ptr<VnVProcess> exec(std::string cmd) { return std::make_shared<LinuxProcess>(cmd); }

  

    static int info_callback(struct dl_phdr_info *info, size_t /*size*/, void *data)
    {
      std::string name(info->dlpi_name);
      if (name.empty())
        return 0;
      unsigned long add(info->dlpi_addr);
      libData *x = static_cast<libData *>(data);
      x->libs.push_back(DistUtils::getLibInfo(name, add));
      return 0;
    }

    void getAllLinkedLibraryData(libData *data) { dl_iterate_phdr(info_callback, data); }

    void *loadLibrary(std::string name)
    {
      if (name.empty())
      {
        throw INJECTION_EXCEPTION("File Name %s is invalid", name.c_str());
      }
      void *dllib = dlopen(name.c_str(), RTLD_NOW);

      return dllib;
    }

    registrationCallBack searchLibrary(void *dylib, std::string packageName)
    {
      void *callback = dlsym(dylib, packageName.c_str());
      if (callback != nullptr)
      {
        return ((registrationCallBack)callback);
      }
      throw INJECTION_EXCEPTION("Library Registration symbol not found for package %s", packageName.c_str());
    }

  }
}
