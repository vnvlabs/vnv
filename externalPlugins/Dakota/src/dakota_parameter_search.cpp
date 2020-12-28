#include "dakota_vnv_interface.hpp"
#include <iostream>

using namespace VnV;


class ParameterStudy : public VnVStudy {
public:
    ITest* test;
    double* x, *y, *z;

    std::string input_string(int size, int max_its, int tol, int vars, int ofncs) {

       std::ostringstream descs;
       for (int i = 0; i < vars; i++) {
           descs <<"'v" << i << "' ";
       }

       std::ostringstream oss;
       oss << "     method,"                                    << "\n"
           << "             optpp_q_newton"                     << "\n"
           << "               max_iterations =" << max_its      << "\n"
           <<"               convergence_tolerance =" << tol    << "\n"
           <<"     variables,"                                  << "\n"
           <<"             continuous_design =" << vars         << "\n"
           <<"               descriptors " << descs.str()       << "\n"
           <<"     interface,"                                  << "\n"
           <<"             direct"                              << "\n"
           <<"             processors_per_analysis =" << size   << "\n"
           <<"             analysis_driver = 'plugin_vnv'"      << "\n"
           <<"     responses,"                                  << "\n"
           <<"             num_objective_functions = " << ofncs << "\n"
           <<"             numerical_gradients"                 << "\n"
           <<"             no_hessians"                         << "\n";

       return oss.str();
    }

    virtual Dakota::LibraryEnvironment* setup(ITest* test, ICommunicator_ptr comm) override {

        this->test = test;
        x = test->getInputParameter<double>("x0","double");
        y = test->getInputParameter<double>("x1","double");
        z = test->getOutputParameter<double>("f","double");

        Dakota::ProgramOptions opts;

        std::string inp = input_string(comm->Size(),50,1e-4,2,1);
        opts.input_string(inp.c_str());

        if (comm->getName() == "VNV:mpi") {
            MPI_Comm* c = (MPI_Comm*) comm->raw();
            return new Dakota::LibraryEnvironment(*c,opts);
        }
        return new Dakota::LibraryEnvironment(opts);
    }

    virtual int setInputs(const Dakota::RealVector& c_vars) override {
        *x = c_vars[0];
        *y = c_vars[1];
        return 0;
    }

    virtual int setOutputs(short asv, Dakota::Real& fn_val, Dakota::RealVector& fn_grad, Dakota::RealSymMatrix& fn_hess) override {
        fn_val = *z;
        return 0;
    }

};
typedef DakotaInterface<ParameterStudy> VnVParameterStudy;

/**
 * @brief INJECTION_ITERATOR_R
 * @param x0
 * @param x1
 * @param f
 *
 * This one needs some work. All this code could be made more generic so we can support a bunch
 * of different tests with little changes. It also doesn't work because inputfile is hardcoded
 * string to a file that does not exist. WIP.
 *
 * The goal here is to create a set of dakota tests for a lot of different methods. We want to make
 * it really easy to write a dakota test. We want to hide the fact that dakota is doing all the work
 *
 * So a test called parameter search that accepts x, y, z, w, ... as parameters as allows the user to
 * run a parameter search.
 *
 * A test that does UQ on parameters
 * etc.
 *
 * Basically, we wrap dakota up so it looks like VnV then let dakota drive.
 *
 */
INJECTION_ITERATOR_R(PNAME, parameter_search, VnVParameterStudy, double x0, double x1, double f ) {
    std::cout << "HERE" << std::endl;
    return runner->run(this, engine, comm);
}
