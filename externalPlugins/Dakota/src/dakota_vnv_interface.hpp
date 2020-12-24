#ifndef DAKOTA_VNV_INTERFACE_HPP
#define DAKOTA_VNV_INTERFACE_HPP

#include "VnV.h"
#include <sys/utsname.h>
#include <iostream>
#include "DakotaBuildInfo.hpp"
#include "ProblemDescDB.hpp"
#include "LibraryEnvironment.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "PluginSerialDirectApplicInterface.hpp"
#include "PluginParallelDirectApplicInterface.hpp"
#include <thread>

#define PNAME Dakota


// This is the class T should implement to make sure it works.
// Should have no constructor parameters,
class VnVStudy {
public:
    VnVStudy(){}
    virtual ~VnVStudy(){}

    virtual Dakota::LibraryEnvironment* setup(VnV::ITest* test)=0;

    virtual int setInputs(const Dakota::RealVector& c_vars) = 0;

    virtual int setOutputs(short asv,
                   Dakota::Real& fn_val,
                   Dakota::RealVector& fn_grad,
                   Dakota::RealSymMatrix& fn_hess
                  ) = 0;
};


template<typename T>
class VnVDakotaInterface : public Dakota::DirectApplicInterface {
public:

    int status = 0;
    T& eval;

    VnVDakotaInterface(const Dakota::ProblemDescDB& problem_db, T& eval_ ) : Dakota::DirectApplicInterface(problem_db), eval(eval_) {}
    ~VnVDakotaInterface() {}

  int funceval(const Dakota::RealVector& c_vars, short asv,
               Dakota::Real& fn_val, Dakota::RealVector& fn_grad,
               Dakota::RealSymMatrix& fn_hess) {

      // Set the input parameters.
      eval.setInputs(c_vars);
      status = 1;
      while (status == 1 ) {} //  This runs in a thread ONLY
      eval.setOutputs(asv,fn_val,fn_grad,fn_hess);
      status = 0;
      return 0;
  }

  int derived_map_ac(const Dakota::String& ac_name) {

       Dakota::RealVector fn_grad;
       Dakota::RealSymMatrix fn_hess;

       if (directFnASV[0] & 2)
            fn_grad = Teuchos::getCol(Teuchos::View, fnGrads, 0);
       if (directFnASV[0] & 4)
           fn_hess = Dakota::RealSymMatrix(Teuchos::View, fnHessians[0], fnHessians[0].numRows());

       int fail_code = funceval(xC, directFnASV[0], fnVals[0], fn_grad, fn_hess);

       if (fail_code) {
           std::string err_msg("Error evaluating plugin analysis_driver ");
           err_msg += ac_name;
           throw VnV::VnVExceptionBase(err_msg);
        }
        return 0;
    }


    void wait_local_evaluations(Dakota::PRPQueue& prp_queue) {

         for (Dakota::PRPQueueIter prp_iter = prp_queue.begin();
              prp_iter != prp_queue.end(); prp_iter++) {
            // For each job in the processing queue, evaluate the response

            int fn_eval_id = prp_iter->eval_id();
            const Dakota::Variables& vars = prp_iter->variables();
            const Dakota::ActiveSet& set  = prp_iter->active_set();
            Dakota::Response         resp = prp_iter->response(); // shared rep

            Dakota::RealVector fn_grad; Dakota::RealSymMatrix fn_hess;
            short asv = set.request_vector()[0];
            Dakota::Real& fn_val = resp.function_value_view(0);
            if (asv & 2) fn_grad = resp.function_gradient_view(0);
            if (asv & 4) fn_hess = resp.function_hessian_view(0);

            funceval(vars.continuous_variables(), asv, fn_val, fn_grad, fn_hess);

            completionSet.insert(fn_eval_id);

         }
     }


    void test_local_evaluations(Dakota::PRPQueue& prp_queue) {
        wait_local_evaluations(prp_queue);
    }

    void derived_map_asynch(const Dakota::ParamResponsePair& pair) {}

    void set_communicators_checks(int max_eval_concurrency){}

};

template<typename T>
class DakotaInterface {
public:
    Dakota::ProgramOptions opts;
    std::shared_ptr<Dakota::LibraryEnvironment> env;

    bool initialized;

    int status = 0;

    T eval;

    VnV::IOutputEngine *engine;
    std::vector<std::thread> workers;
    std::shared_ptr<VnVDakotaInterface<T>> serial_iface;

    void initialize(VnV::ITest *test) {
     if (!initialized) {
        initialized = true;


        // construction to allow update after all data is populated
        bool check_bcast_construct = false;

        // set up a Dakota instance, with the right MPI configuration if a
        // parallel run (don't need to pass the MPI comm here, just doing to
        // demonstrate/test).
        env.reset(eval.setup(test));

        std::string model_type(""); // demo: empty string will match any model type
        std::string interf_type("direct");
        std::string an_driver("plugin_vnv");

        Dakota::ProblemDescDB& problem_db = env->problem_description_db();
        serial_iface.reset( new VnVDakotaInterface<T>(problem_db, eval) );
        env->plugin_interface(model_type, interf_type, an_driver, serial_iface);

        // Need to start the dakota thread that calls the run function
        status = 2;
        //workers.push_back(std::thread([this](){
        //    env->execute();
        //    status = 2;
        //}));
     }
    }

    // 0 waiting for variables to be set.
    // 1 waiting for output to be calculated
    // 2 finished iterating.
    int runnerStatus() {
        return (status == 2) ? 2 : serial_iface->status;
    }

    int run(VnV::ITest* test, VnV::IOutputEngine* engine) {
        this->engine = engine;

        initialize(test);

        while (true) {
            int r = runnerStatus();
            if (r == 2) {
                return 0; // Done. no more iterations
            } else if (r == 1 ) {
                return 1;
            }
        }
    }
};


#endif // DAKOTA_VNV_INTERFACE_HPP
