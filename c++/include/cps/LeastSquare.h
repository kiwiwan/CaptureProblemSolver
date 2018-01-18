#pragma once

#include <Eigen/Core>

#include <cps/cps_api.h>
#include <cps/defs.h>
#include <cps/LinearConstraints.h>
#include <cps/QuadraticObjective.h>
#include <cps/Statistics.h>

namespace cps
{
  /** Information returned by the solvers upon completion.*/
  enum class SolverStatus
  {
    Converge,                       /** A solution was found*/
    MaxIteration,                   /** The maximum number of iteration was reached*/
    LineSearchFailed,               /** Step in the line search became too small*/
    NoLinearlyFeasiblePoint,        /** Linear inequality constraints are incompatible*/
    NumericallyEquivalentIterates,  /** Previous and new iterates are equal*/
    Fail
  };

  /** A solver dedicated solver for the problem for the least-square subproblem
    * occuring in cps::SQP.
    *
    * The class in itself handle the memory and statistics useful for its two
    * main methods:
    *  - solve
    *  - solveFeasibility
    */
  class CPS_DLLAPI LeastSquare
  {
  public:
    /** A set of parameters for controlling the execution of the resolution.*/
    class Parameters
    {
    public:
      int    maxIter()       const { return maxIter_; }
      double minNorm_p()     const { return minNorm_p_; }
      double dualEps()       const { return dualEps_; }
      double rankThreshold() const { return rankThreshold_; }

      Parameters& maxIter      (int i)    { maxIter_ = i;       return *this; }
      Parameters& minNorm_p    (double d) { minNorm_p_ = d;     return *this; }
      Parameters& dualEps      (double d) { dualEps_ = d;       return *this; }
      Parameters& rankThreshold(double d) { rankThreshold_ = d; return *this; }

    private:
      int maxIter_ = -1;              //maximum number of active set iterations. If <= 0, defaulted to 10*n
      double minNorm_p_ = 1e-10;      //bound on the L-inf norm of p under which p is treated as 0
      double dualEps_ = 1e-15;        //margin on the 0 for the Lagrange multipliers
      double rankThreshold_ = 1e-12;  //threshold on the last diagonal value in the QR of A, to detect rank loss
    };

    /** Create a solver for a problem of size n.*/
    LeastSquare(int n);

    /** Change parameters.*/
    void parameters(const Parameters& param);
    /** Get the current parameters.*/
    const Parameters& parameters() const;

    /** Solve the following problem:
      *
      * min. ||J x + Jx0||^2 + ||j^T x + c||^2
      * s.t. l_i <= x_i-x_{i-1} <= u_i for i=0..n-1  (x_{-1} = 0)
      *      xln <= x_{n-1} <= xun
      *
      * where J is described by obj, and the constraints by lc
      */
    SolverStatus solve(const LeastSquareObjective& obj, const VectorConstRef& Jx0, const VectorConstRef& j, double c, LinearConstraints& lc);

    /** Solve the following problem:
    *
    * min. ||j^T x + c||^2
    * s.t. l_i <= x_i-x_{i-1} <= u_i for i=0..n-1  (x_{-1} = 0)
    *      xln <= x_{n-1} <= xun
    *
    * where the constraints are described by lc
    */
    SolverStatus solveFeasibility(const VectorConstRef& j, double c, LinearConstraints& lc);

    /** Retrieve the solution (after call to solve() or solveFeasibility())*/
    const Eigen::VectorXd& x() const;
    /** Retrieve the Lagrange multipliers (after call to solve() or solveFeasibility())*/
    const Eigen::VectorXd& lambda() const;

    /** Get stats on the last run. Only meaningful if USE_STATS is defined*/
    const stats::LSStats& statistics() const;

  private:
    // size of the problem
    Eigen::DenseIndex n_;

    //solver parameters;
    Parameters params_;

    //computation data
    Eigen::VectorXd x_;
    Eigen::VectorXd p_;
    Eigen::VectorXd z_;
    Eigen::VectorXd lambda_;
    Eigen::VectorXd lambdaAct_;
    Eigen::VectorXd jN_;
    Eigen::MatrixXd A_;
    Eigen::VectorXd b_;
    Eigen::VectorXd Jx_;
    Eigen::VectorXd JtJx_;
    Eigen::VectorXd tmp_;
    GivensSequence Qg_;
    CondensedOrthogonalMatrix Q_;

    //stats
    stats::LSStats stats_;
  };
}
