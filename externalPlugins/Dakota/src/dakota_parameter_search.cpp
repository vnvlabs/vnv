#include "dakota_vnv_interface.hpp"


using namespace VnV;

static const char serial_input[] =
  "     method,"
  "             optpp_q_newton"
  "               max_iterations = 50"
  "               convergence_tolerance = 1e-4"
  "     variables,"
  "             continuous_design = 2"
  "               descriptors 'x' 'y'"
  "     interface,"
  "             direct"
  "               analysis_driver = 'plugin_vnv'"
  "     responses,"
  "             num_objective_functions = 1"
  "             numerical_gradients"
  "             no_hessians";


class ParameterStudy : public VnVStudy {
public:
    ITest* test;
    double* x, *y, *z;

    virtual Dakota::LibraryEnvironment* setup(ITest* tests) override {

        this->test = test;
        x = test->getInputParameter<double>("x","double");
        y = test->getInputParameter<double>("y","double");
        z = test->getOutputParameter<double>("z","double");

        Dakota::ProgramOptions opts;
        opts.input_string(serial_input);
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
    return runner->run(this, engine);
}
