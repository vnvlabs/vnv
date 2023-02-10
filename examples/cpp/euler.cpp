// Modified Euler's Method for differential equations
#include <math.h>

#include <iomanip>
#include <iostream>

#include "VnV.h"

double df(double x, double y) {
  double a = x + y;  // function for defining dy/dx
  return a;
}

/**
 * Solving A Differential Equation using the modified Euler Method.
 * ----------------------------------------------------------------
 *
 * This application uses the Euler method to solve the following differential equation
 *
 * .. vnv-math::
 *
 *      y' - y =  - \frac{1}{2}{{\bf{e}}^{\frac{t}{2}}}\sin \left( {5t} \right) + 5{{\bf{e}}^{\frac{t}{2}}}\cos \left(
 * {5t} \right)\hspace{0.25in}y\left( 0 \right) = 0
 *
 * It turns out that we can actually solve this one analytically -- the true solution  is:
 *
 * .. vnv-math::
 *
 *      y\left( t \right) = {{\bf{e}}^{\frac{t}{2}}}\sin \left( {5t} \right)
 *
 * .. vnv-plotly::
 *      :trace.sol: scatter
 *      :sol.x: [0.0]
 *      :sol.y: [0.0]
 *
 *      {
 *          "sol.x" : json.dumps(np.linspace(0,5,1000).tolist()),
 *          "sol.y" : json.dumps( (np.exp(np.linspace(0,5,1000)/2) * np.sin(5*np.linspace(0,5,1000))).tolist() )
 *      }
 *
 *
 * As you will see, Eulers method is not that great for solving problems where the function changes
 * rapidly like this one.
 *
 */
INJECTION_EXECUTABLE(Euler)

struct OptionsStruct {
  double dt;
  double stop;
};

INJECTION_OPTIONS(Euler, R"({
    "type" : "object",
    "properties" : {
        "dt" :   {"type" : "integer", "default" : 0.2, "min" : 0.000001 , "max" : 1 },
        "stop" : {"type" : "integer", "default" : 5, "min" : 0.1, "max" : 10 }
    }
})",
                  void) {
  // @beon memory leak? Provide a way to destroy;
  OptionsStruct* a = new OptionsStruct();
  a->dt = config.value("dt", 0.2);
  a->stop = config.value("stop", 5.0);
  return a;
}

INJECTION_CODEBLOCK_START(Euler, Derivitive)
double deriv(double y, double t) { return y - 0.5 * exp(t / 2.) * sin(5 * t) + 5 * exp(t / 2.) * cos(5 * t); }
INJECTION_CODEBLOCK_END(Euler, Derivitive)

double solve(double dt, double stop) {
  double t = 0;
  double y = 0;

  /**
   *
   * @title Iterative Euler Solve Results:
   * @shortTitle Eulers Method with dt = :vnv:`dt`
   *
   * In this section we solve the differential equation using Eulers method. The time step was dt = :vnv:`dt`
   *
   * .. vnv-plotly::
   *      :trace.approx: scatter
   *      :trace.exact: scatter
   *      :approx.x: {{t}}
   *      :approx.y: {{y}}
   *      :exact.x: {{t}}
   *      :exact.y: {{exact}}
   *
   * In this next chart we plot the relative error against time. We expect that it will grow ....
   *
   * .. vnv-plotly::
   *      :trace.error: scatter
   *      :approx.x: {{t}}
   *      :approx.y: {{error}}
   *
   *
   */
  INJECTION_LOOP_BEGIN(
      Euler, VSELF, Eulers, VNV_CALLBACK { data.engine->Put("dt", dt); }, t, y);

  while (t < stop) {
    INJECTION_LOOP_ITER(
        Euler, Eulers, "TimeStep", VNV_CALLBACK {
          double exact = exp(t / 2.) * sin(5 * t);
          data.engine->Put("y", y);
          data.engine->Put("t", t);
          data.engine->Put("exact", exact);
          data.engine->Put("error", fabs(exact - y));
        });

    dt = ((stop - t) > dt) ? dt : stop - t;
    y = y + dt * deriv(y, t);
    t += dt;
  }

  INJECTION_LOOP_END(Euler, Eulers, VNV_NOCALLBACK);

  double exact = exp(stop / 2.) * sin(5 * stop);
  return fabs((y - exact) / exact);
}

int main(int argc, char** argv) {
  /** @title Eulers method:
   *
   * This application uses the Euler method to solve the following differential equation
   *
   * .. vnv-math::
   *
   *      y'  = y - \frac{1}{2}{{\bf{e}}^{\frac{t}{2}}}\sin \left( {5t} \right) + 5{{\bf{e}}^{\frac{t}{2}}}\cos \left(
   * {5t} \right)\hspace{0.25in}y\left( 0 \right) = 0
   *
   * It turns out that we can actually solve this one analytically -- the true solution  is:
   *
   * .. vnv-math::
   *
   *      y\left( t \right) = {{\bf{e}}^{\frac{t}{2}}}\sin \left( {5t} \right)
   *
   * Eulers method is really just a first order taylor expansion of the function. For instance, consider the taylor
   * expansion of a function :vnvmath:`y(t)` around :vnvmath:`t_0`
   *
   * .. vnv-math::
   *
   *    y(t_0 + dt ) = y(t_0) + dt*y'(t_0) + H.O.T
   *
   * Then, all we do for eulers method is ignore the high order terms. In other words, the High order terms become
   * our error.....(teaching is hard, just look on wikipedia :)
   *
   * Anyway, the exact solution looks like this:
   *
   * .. vnv-plotly::
   *      :trace.sol: scatter
   *      :sol.x: [0.0]
   *      :sol.y: [0.0]
   *
   *      {
   *          "sol.x" : json.dumps(np.linspace(0,5,1000).tolist()),
   *          "sol.y" : json.dumps( (np.exp(np.linspace(0,5,1000)/2) * np.sin(5*np.linspace(0,5,1000))).tolist() )
   *      }
   *
   * The Fish has :vnv:`Fish`
   *
   * As you will see, Eulers method is not that great for solving problems where the function changes
   * rapidly like this one.
   *
   */
  INJECTION_INITIALIZE_C(Euler, &argc, &argv, (argc > 1) ? argv[1] : "inputfiles/euler.json",
                         [](VnV_Comm comm, VnV::IOutputEngine* engine) { engine->Put("Fish", "Power"); });

  auto config = (OptionsStruct*)INJECTION_GET_CONFIG(Euler);
  auto dt = config->dt;
  auto stop = config->stop;
  double error = 0;

  solve(dt, stop);

  /**
   * @title Whats step size got to do with it?
   * @shortTitle Changing the Step Size.
   *
   * The step size plays a huge role in accuracy of the method. The error of the
   * euler method grows by O(dt^2) at each step. Taking n steps means the error will
   * grow by O(dt) globally.
   *
   * The figure below plots the realative error at t = :vnv:`stop[0]` for a range of step
   * sizes. We would expect the error to grow linearly with step size.
   *
   * .. vnv-plotly::
   *    :trace.error: scatter
   *    :error.x: {{dt}}
   *    :error.y: {{error}}
   *
   * .. note::
   *
   *    This plot is known as a scaling study. Scaling studies are a usefull approach for finding
   *    bugs in your implementation. If the analysis says it should scale linearly, but it actually
   *    scales sublinearly, then something is probably wrong with your code.
   *
   *
   */
  INJECTION_LOOP_BEGIN(
      Euler, VSELF, StepSize, VNV_CALLBACK { data.engine->Put("stop", stop); }, stop, dt, error);

  for (int i = 1; i < 5; i += 1) {
    dt = stop / (pow(10, i));
    error = solve(dt, stop);

    INJECTION_LOOP_ITER(
        Euler, StepSize, "Step", VNV_CALLBACK {
          data.engine->Put("dt", dt);
          data.engine->Put("error", error);
        });
  }

  INJECTION_LOOP_END(Euler, StepSize, VNV_NOCALLBACK);

  INJECTION_FINALIZE(Euler);

  return 0;
}