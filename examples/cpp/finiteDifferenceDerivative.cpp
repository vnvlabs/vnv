#include <cmath>
#include <vector>

#include "VnV.h"

using namespace VnV;

const double PI = 3.1415926535;

/**
 * @title Plot of Sine, Cosine, and the Numerical Derivative of Sine
 *
 * Plot of sine and true cosine vs phase:
 *
 * .. vnv-plotly::
 *    :trace.sine: scatter
 *    :trace.cosine: scatter
 *    :sine.x: {{as_json(phase_vals)}}
 *    :sine.y: {{as_json(sine_vals)}}
 *    :cosine.x: {{as_json(phase_vals)}}
 *    :cosine.y: {{as_json(cosine_vals)}}
 *
 * Function to populate the vectors for the sine and true cosine:
 *
 * .. code:: cpp
 *
 *    // create true version of sinusoids
 *    auto true_ptsPerPeriod = 15000;
 *    auto true_step = 2*PI/true_ptsPerPeriod;
 *    //
 *    std::vector<double> phase_vals;
 *    std::vector<double> sine_vals;
 *    std::vector<double> cosine_vals;
 *    auto phi = 0.0;
 *    while (phi <= 2*PI) {
 *        phase_vals.push_back(phi);
 *        sine_vals.push_back(std::sin(phi));
 *        cosine_vals.push_back(std::cos(phi));
 *
 *        phi = phi+true_step;
 *    }
 *
 * Plot of the forward finite difference numerical derivative of sine. This is a mesh of 100 elements. The true cosine
 *is also plotted for reference.
 *
 * .. vnv-plotly::
 *    :trace.derivative: scatter
 *    :trace.true: scatter
 *    :derivative.x: {{as_json(mesh_phase_vals)}}
 *    :derivative.y: {{as_json(diff_cosine_vals)}}
 *    :true.x: {{as_json(mesh_phase_vals)}}
 *    :true.y: {{as_json(mesh_cosine_vals)}}
 *
 *
 * Function to compute the numerical derivative
 *
 * .. code:: cpp
 *
 *    double deriv(std::vector<double>& vals, int& i, double& step, char type) {
 *        double deriv = 0.0;
 *        switch (type) {
 *            case 'f':
 *                deriv = (i == vals.size()-1)
 *                ? (vals.at(1) - vals.at(i))/step
 *                : (vals.at(i+1) - vals.at(i))/step;
 *            case 'b':
 *                deriv = (i == 0)
 *                ? (vals.at(i) - vals.at(vals.size()-2))/step
 *                : (vals.at(i) - vals.at(i-1))/step;
 *        }
 *        return deriv;
 *    }
 *
 **/
INJECTION_TEST(FiniteDifferenceDerivative, finiteDifferenceDerivative) {
  auto phase_vals = GetRef_NoCheck("phase_vals", std::vector<double>);
  auto sine_vals = GetRef_NoCheck("sine_vals", std::vector<double>);
  auto cosine_vals = GetRef_NoCheck("cosine_vals", std::vector<double>);
  auto mesh_phase_vals = GetRef_NoCheck("mesh_phase_vals", std::vector<double>);
  auto diff_cosine_vals = GetRef_NoCheck("diff_cosine_vals", std::vector<double>);
  auto mesh_cosine_vals = GetRef_NoCheck("mesh_cosine_vals", std::vector<double>);

  engine->Put_Vector("phase_vals", phase_vals);
  engine->Put_Vector("sine_vals", sine_vals);
  engine->Put_Vector("cosine_vals", cosine_vals);
  engine->Put_Vector("mesh_phase_vals", mesh_phase_vals);
  engine->Put_Vector("diff_cosine_vals", diff_cosine_vals);
  engine->Put_Vector("mesh_cosine_vals", mesh_cosine_vals);

  VnV_Debug(FiniteDifferenceDerivative, "GEEEERER %s", "sdfsdf");

  return SUCCESS;
}

/**
 * @brief
 *
 * @return * Register
 */
INJECTION_EXECUTABLE(FiniteDifferenceDerivative)

/**
 *
 */
INJECTION_OPTIONS(FiniteDifferenceDerivative, "{}") { return NULL; }

double deriv(std::vector<double>&, int&, double&, char);

int main(int argc, char** argv) {
  INJECTION_INITIALIZE(FiniteDifferenceDerivative, &argc, &argv,
                       (argc == 2) ? argv[1] : "./inputfiles/finiteDifferenceDerivative.json");

  // create true version of sinusoids
  auto true_ptsPerPeriod = 15000;
  auto true_step = 2 * PI / true_ptsPerPeriod;
  //
  std::vector<double> phase_vals;
  std::vector<double> sine_vals;
  std::vector<double> cosine_vals;
  auto phi = 0.0;
  while (phi <= 2 * PI) {
    phase_vals.push_back(phi);
    sine_vals.push_back(std::sin(phi));
    cosine_vals.push_back(std::cos(phi));

    phi = phi + true_step;
  }

  // create meshes of sinusoids
  auto mesh_ptsPerPeriod = 100;
  auto mesh_step = 2 * PI / mesh_ptsPerPeriod;
  //
  std::vector<double> mesh_phase_vals;
  std::vector<double> mesh_sine_vals;
  std::vector<double> mesh_cosine_vals;
  phi = 0.0;
  while (phi <= 2 * PI) {
    mesh_phase_vals.push_back(phi);
    mesh_sine_vals.push_back(std::sin(phi));
    mesh_cosine_vals.push_back(std::cos(phi));

    phi = phi + mesh_step;
  }

  // create cosine via differentiation of mesh of sine
  std::vector<double> diff_cosine_vals;
  for (auto i = 0; i < mesh_phase_vals.size(); ++i) {
    for (auto j = 0; j < phase_vals.size(); ++j) {
      if (std::abs(phase_vals.at(j) - mesh_phase_vals.at(i)) <= (0.0000001)) {
        auto diff_cosine = deriv(mesh_sine_vals, i, mesh_step, 'f');
        diff_cosine_vals.push_back(diff_cosine);
      }
    }
  }

  /**
   * @title Injection Point: Finite Difference Derivative
   * @description Plots sine, cosine, and the numerical derivative of sine. This derivative is calculated through a
   *1st-order finite difference method.
   *
   * Plots sine, cosine, and the numerical derivative of sine. This derivative is calculated through a 1st-order finite
   *difference method.
   *
   **/
  INJECTION_POINT(FiniteDifferenceDerivative, VWORLD, mainpoint, phase_vals, sine_vals, cosine_vals, mesh_phase_vals,
                  mesh_cosine_vals, diff_cosine_vals);

  INJECTION_FINALIZE(SPNAME);

  return 0;
}

double deriv(std::vector<double>& vals, int& i, double& step, char type) {
  double deriv = 0.0;
  switch (type) {
  case 'f':
    deriv = (i == vals.size() - 1) ? (vals.at(1) - vals.at(i)) / step : (vals.at(i + 1) - vals.at(i)) / step;
  case 'b':
    deriv = (i == 0) ? (vals.at(i) - vals.at(vals.size() - 2)) / step : (vals.at(i) - vals.at(i - 1)) / step;
  }
  return deriv;
}
