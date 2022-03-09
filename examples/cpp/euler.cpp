//Modified Euler's Method for differential equations
#include<iostream>
#include<math.h>
#include<iomanip>
#include "VnV.h"

double df(double x, double y)
{
    double a=x+y;            //function for defining dy/dx
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
 *      y' - y =  - \frac{1}{2}{{\bf{e}}^{\frac{t}{2}}}\sin \left( {5t} \right) + 5{{\bf{e}}^{\frac{t}{2}}}\cos \left( {5t} \right)\hspace{0.25in}y\left( 0 \right) = 0 
 *
 * It turns out that we can actually solve this one analytically -- the true solution  is:
 * 
 * .. vnv-math::
 * 
 *      y\left( t \right) = {{\bf{e}}^{\frac{t}{2}}}\sin \left( {5t} \right)  
 * 
 * .. vnv-plotly:: 
 *      :trace.sol: scatter
 *      :sol.x: [0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0]
 *      :sol.y: [0.0, 0.9299692608141624, 1.1106183851128328, 0.1904920858048083, -1.1290166537369088, -1.5809988486278084, -0.5091282323887432, 1.323008541941781, 2.2018572708018813, 1.013647908465914, -1.478802700028677, -3.0041366027987997, -1.7814848253353737, 1.5417175080714458, 4.017110915969929, 2.914387905805897, -1.4259944625329628, -5.2626392930871715, -4.5432080932387295, 1.0020632031098577, 6.745803672878749, 6.832272102593218, -0.07988318583356165, -8.440356708230206, -9.982350010891526, -1.6123743962546553]
 *      :layout.title.text: Plot of the equation
 *      :layout.xaxis.text: Time
 *      :layout.yaxis.text: y(t)
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

INJECTION_OPTIONS(Euler,R"({
    "type" : "object",
    "properties" : {
        "dt" :   {"type" : "integer", "default" : 0.2, "min" : 0.000001 , "max" : 1 },
        "stop" : {"type" : "integer", "default" : 5, "min" : 0.1, "max" : 10 }
    }
})") {

  // @beon memory leak? Provide a way to destroy;
  OptionsStruct *a = new OptionsStruct();
  a->dt = config.value("dt",0.2);
  a->stop = config.value("stop",5.0);
  return a;

}


double deriv(double y, double t) {
    return y - 0.5*exp(t/2.)*sin(5*t) + 5*exp(t/2.)*cos(5*t); 
}

double solve( double dt, double stop) {
    
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
    INJECTION_LOOP_BEGIN_C("Euler",VSELF,"Eulers",IPCALLBACK {
       if (type == VnV::InjectionPointType::Begin) {
           engine->Put("dt",dt);
       } else if (type == VnV::InjectionPointType::Iter) {
          double exact = exp(t/2.)*sin(5*t);
          
          engine->Put("y", y);
          engine->Put("t", t);
          engine->Put("exact", exact);
          engine->Put("error", fabs(exact-y));
       }
    }, t, y);

    while (t < stop) {

        INJECTION_LOOP_ITER("Euler", "Eulers", "TimeStep");
        dt =  ( (stop - t) > dt ) ? dt : stop - t ;
        y = y + dt * deriv(y,t);
        t += dt;
    }
    
    INJECTION_LOOP_END("Euler","Eulers");
    
    double exact = exp(stop/2.)*sin(5*stop);
    return fabs((y-exact)/exact);
}


int main(int argc, char** argv) {

   /** @title Eulers method:
    * 
    * This application uses the Euler method to solve the following differential equation
    * 
    * .. vnv-math::
    *      
    *      y'  = y - \frac{1}{2}{{\bf{e}}^{\frac{t}{2}}}\sin \left( {5t} \right) + 5{{\bf{e}}^{\frac{t}{2}}}\cos \left( {5t} \right)\hspace{0.25in}y\left( 0 \right) = 0 
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
    *      :sol.x: [0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0]
    *      :sol.y: [0.0, 0.9299692608141624, 1.1106183851128328, 0.1904920858048083, -1.1290166537369088, -1.5809988486278084, -0.5091282323887432, 1.323008541941781, 2.2018572708018813, 1.013647908465914, -1.478802700028677, -3.0041366027987997, -1.7814848253353737, 1.5417175080714458, 4.017110915969929, 2.914387905805897, -1.4259944625329628, -5.2626392930871715, -4.5432080932387295, 1.0020632031098577, 6.745803672878749, 6.832272102593218, -0.07988318583356165, -8.440356708230206, -9.982350010891526, -1.6123743962546553]
    * 
    * 
    * As you will see, Eulers method is not that great for solving problems where the function changes 
    * rapidly like this one. 
    * 
    */
    INJECTION_INITIALIZE(Euler, &argc, &argv,"inputfiles/euler.json");

    auto config = (OptionsStruct*) INJECTION_GET_CONFIG(Euler);
    auto dt = config->dt;
    auto stop = config->stop;
    double error = 0;

    solve(dt,stop);

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
     INJECTION_LOOP_BEGIN_C("Euler",VSELF,"StepSize", IPCALLBACK {
        if (type == VnV::InjectionPointType::Begin) {
            engine->Put("stop", stop);
        } else if (type == VnV::InjectionPointType::Iter) {
            engine->Put("dt", dt);
            engine->Put("error", error);
        }
     }, stop, dt, error );
     for ( int i = 1; i < 5; i+=1 ) {
         dt = stop / (pow(10,i) );
         error = solve(dt,stop);
         INJECTION_LOOP_ITER("Euler","StepSize","Step");
     }
     INJECTION_LOOP_END("Euler","StepSize");



    INJECTION_FINALIZE(Euler);

    return 0;
}