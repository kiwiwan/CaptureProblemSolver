#pragma once
/* Copyright 2018 CNRS-AIST JRL, CNRS-UM LIRMM
 *
 * This file is part of CPS.
 *
 * CPS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CPS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with CPS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vector>

#include <Eigen/Core>

#include <cps/cps_api.h>
#include <cps/defs.h>

namespace cps
{
  enum class Activation
  {
    None,
    Lower,
    Upper,
    Equal
  };

  /** An information returned by the search of a feasible point
    *  - Found: the point was found
    *  - Infeasible: the constraints are not compatible (may be due to active 
    *    constraints if they are taken into account)
    *  - NumericalWarningXXX: the lower bound on x_n, its upper bound or both
    *    are numerically redundant with the other constraints. Please consider
    *    moving them.
    *  - TooSmallZonotope: the n first constraints define a zonotope too small.
    *  - TooSmallFeasibilityZone: the feasible zone is too small.
    */
  enum class FeasiblePointInfo
  {
    Found,
    Infeasible,
    NumericalWarningLower,
    NumericalWarningUpper,
    NumericalWarningBoth,
    TooSmallZonotope,
    TooSmallFeasibilityZone
  };


  /** A class to manipulate the linear constraints of the problem.
    * l_i <= x_i-x_{i-1} <= u_i for i=0..n-1  (x_{-1} = 0)
    * xln <= x_{n-1} <= xun
    * Constraints are considered in this order.
    *
    * In particular, the class is in charge of the activation status of the constraints.
    */
  class CPS_DLLAPI LinearConstraints
  {
  private:
    /** Intermediate temporary structure for shifting the constraints. See shift.*/
    struct Shift
    {
      const LinearConstraints& c;
      const Eigen::VectorXd& x;
      bool feasible;
    };

  public:
    /** Create uninitialized constraints with memory allocated for a problem with size n.*/
    LinearConstraints(int n);
    /** Create the constraints with initialized data.*/
    LinearConstraints(const Eigen::VectorXd& l, const Eigen::VectorXd& u, double xln, double xun);

    /** See shift*/
    LinearConstraints& operator= (const Shift& s);

    /** Build the constraints on p such that on s.x+p verifies the original constraints s.c.
      * If feasible is true, try to correct numerical errors by making:
      * - l<=0
      * - u>=0
      * - l[i] = u[i] = 0 for equality constraints
      * It is the user responsibility to ensure that x is indeed feasible in this case.
      */
    Shift shift(const Eigen::VectorXd& x, bool feasible=false) const;

    /** Get the lower bound [l;xln] of the constraints.*/
    const Eigen::VectorXd& l() const;
    /** Get the upper bound [u;xun] of the constraints.*/
    const Eigen::VectorXd& u() const;

    /** Return a feasible point for the constraints.
      *
      * If takeActivationIntoAccount is true, the point is on the activated constraints.
      * eps is the margin to infeasibility for which the info returned is a warning.
      */
    std::pair<FeasiblePointInfo, Eigen::VectorXd> initialPoint(bool takeActivationIntoAccount = false, double eps = 1e-8) const;

    /** Size n of the problem.*/
    Eigen::DenseIndex size() const;
    /** Size of the nullspace of the active constraints.*/
    Eigen::DenseIndex nullSpaceSize() const;

    /** Activate the i-th constraint
      *
      * If a = Activation::None, this effectively deactivate the i-th constraint
      */
    void activate(size_t i, Activation a);
    /** Deactivate the i-th constraint*/
    void deactivate(size_t i);
    /** Deactivate all non-equality constraints*/
    void resetActivation();

    /** Get the activation status of the i-th constraint (0-base index).*/
    Activation activationStatus(size_t i) const;

    /** Get the current active set. v[i] is true if the i-th constraint is active.*/
    const std::vector<bool>& activeSet() const;
    /** Get the current activation status of all constraints.*/
    const std::vector<Activation>& activationStatus() const;
    /** Set the activation status of all constraints.*/
    void setActivationStatus(const std::vector<Activation>& act);
    /** Index of active constraints.*/
    const std::vector<Eigen::DenseIndex>& activeSetIdx() const;
    /** Number na of active constraints.*/
    Eigen::DenseIndex numberOfActiveConstraints() const;

    /** Change the bounds of the i-th constraint to l and u 0<= i <=n.*/
    void changeBounds(Eigen::DenseIndex i, double l, double u);
    /** Change the bounds of the n first or all constraints, depending on the
      * size of l and u.
      */
    void changeBounds(const VectorConstRef l, const VectorConstRef u);

    /** Y = X*N_A */
    template<typename Derived1, typename Derived2>
    void applyNullSpaceOnTheRight(const Eigen::MatrixBase<Derived1>& Y, const Eigen::MatrixBase<Derived2>& X) const;
    /** Y = N_A X */
    void applyNullSpaceOnTheLeft(MatrixRef Y, const MatrixConstRef& X) const;

    /** Y = C * X */
    template<typename Derived1, typename Derived2>
    void mult(const Eigen::MatrixBase<Derived1>& Y, const Eigen::MatrixBase<Derived2>& X) const;
    /** Y = C^T * X */
    template<typename Derived1, typename Derived2>
    void transposeMult(const Eigen::MatrixBase<Derived1>& Y, const Eigen::MatrixBase<Derived2>& X) const;
    /** Y = pinv(C_A^T)*X */
    void pinvTransposeMultAct(MatrixRef Y, const MatrixConstRef& X) const;

    /** fill y such that y(!active) = 0 and y(active) = x*/
    void expandActive(VectorRef y, const VectorConstRef& x) const;

    /** Move the furthest possible on the segment x+ap for 0<=a<=1. Stop at the
      * first constraint encountered and activate it. Changes x to x+ap.
      *
      * If fullstep is passed by the user, set it to true when a full step is
      * performed, false otherwise
      */
    void performQPstep(Eigen::VectorXd& x, const Eigen::VectorXd& p, bool* fullstep = nullptr);

    /** Deactivate the constraint with largest lambda violation. */
    void deactivateMaxLambda(const VectorConstRef& lambda);

    /** The matrix of all constraints. For debugging purposes.*/
    Eigen::MatrixXd matrix() const;
    /** The matrix C_A of active constraints. For debugging purposes.*/
    Eigen::MatrixXd matrixAct() const;

    /** Check if x verifies the constraints*/
    bool checkPrimal(const VectorConstRef& x, double eps = 1e-15) const;
    /** Check the signs of lambda*/
    bool checkDual(const VectorConstRef& lambda, double eps = 1e-15) const;

  private:
    void computeIdx() const;
    void computeActIdx() const;

    //data
    Eigen::DenseIndex n_;
    Eigen::VectorXd l_;   //[l;xln]
    Eigen::VectorXd u_;   //[u;xun]

    //active-set management
    Eigen::DenseIndex na_; //number of activated constraint
    std::vector<Activation> activationStatus_; //type of activation of the constraints
    std::vector<bool> activeSet_; //Invariant: activeSet_[i] == (activationStatus_[i] != Activation::None) for all i

    //internal intermediate data
    mutable bool validIdx_;                         // indicate if idx_ is valid
    mutable bool validActIdx_;                      // indicate if idx_ is valid
    mutable std::vector<Eigen::DenseIndex> idx_;    // indices used for the premultiplication by N_A
    mutable std::vector<Eigen::DenseIndex> actIdx_; // indices of active constraints, ordered.
    mutable Eigen::VectorXd Cx_;                    // to store the results of C*x
    mutable Eigen::VectorXd Cp_;                    // to store the results of C*p
    mutable Eigen::VectorXd actl_;                  // lower bounds when taking into account the activation, for computing the initial point
    mutable Eigen::VectorXd actu_;                  // upper bounds when taking into account the activation, for computing the initial point
  };

  template<typename Derived1, typename Derived2>
  inline void LinearConstraints::applyNullSpaceOnTheRight(const Eigen::MatrixBase<Derived1>& Y_, const Eigen::MatrixBase<Derived2>& X) const
  {
    Eigen::MatrixBase<Derived1>& Y = const_cast<Eigen::MatrixBase<Derived1>&>(Y_);
    assert(X.cols() == n_);
    assert(Y.rows() == X.rows() && Y.cols() == n_ - na_);

    Eigen::DenseIndex c = -1;
    Eigen::DenseIndex k = 0;
    std::fill(idx_.begin(), idx_.end(), -1);
    actIdx_.resize(na_);

    //skip the first group of activated constraints
    while (k < n_ && activeSet_[static_cast<size_t>(k)])
    {
      actIdx_[static_cast<size_t>(k)] = k;
      ++k;
    }

    size_t ca = static_cast<size_t>(k);
    auto i = k;
    for (; i < n_; ++i)
    {
      if (activeSet_[static_cast<size_t>(i)])
      {
        Y.col(c) += X.col(i);
        actIdx_[ca] = i;
        ++ca;
      }
      else
      {
        ++c;
        if (c >= n_ - na_)
          break;
        Y.col(c) = X.col(i);
      }
      idx_[static_cast<size_t>(i)] = c;
    }
    for (++i; i < n_; ++i)
    {
      actIdx_[ca] = i;
      ++ca;
    }
    if (activeSet_.back())
      actIdx_[ca] = n_;

    validIdx_ = true;
    validActIdx_ = true;
  }

  template<typename Derived1, typename Derived2>
  inline void LinearConstraints::mult(const Eigen::MatrixBase<Derived1>& Y_, const Eigen::MatrixBase<Derived2>& X) const
  {
    Eigen::MatrixBase<Derived1>& Y = const_cast<Eigen::MatrixBase<Derived1>&>(Y_);
    assert(X.rows() == n_);
    assert(Y.rows() == n_ + 1 && Y.cols() == X.cols());

    Y.topRows(n_) = X;
    Y.middleRows(1, n_ - 1) -= X.topRows(n_ - 1);
    Y.template bottomRows<1>() = X.template bottomRows<1>();
  }

  template<typename Derived1, typename Derived2>
  inline void LinearConstraints::transposeMult(const Eigen::MatrixBase<Derived1>& Y_, const Eigen::MatrixBase<Derived2>& X) const
  {
    Eigen::MatrixBase<Derived1>& Y = const_cast<Eigen::MatrixBase<Derived1>&>(Y_);
    assert(X.rows() == n_ + 1);
    assert(Y.rows() == n_ && Y.cols() == X.cols());

    Y = X.topRows(n_);
    Y.topRows(n_ - 1) -= X.middleRows(1, n_ - 1);
    Y.template bottomRows<1>() += X.template bottomRows<1>();
  }
}
