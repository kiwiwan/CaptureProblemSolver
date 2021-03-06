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

#ifdef USE_STATS
# define STATISTICS(x) x
#else
# define STATISTICS(x) (void)0
#endif

namespace cps
{
  namespace stats
  {
    /** A structure gathering statistics about a LS run.*/
    struct LSStats
    {
      void reset();

      int iter;                 //number of iterations
      int activation;           //number of activations
      int deactivation;         //number of deactivations
      int rankLoss;             //number of time a rank loss was detected in the objective matrix
      int activeConstraints;    //number of active constraints
    };

    /** A structure gathering statistics about a SQP run.*/
    struct SQPStats
    {
      void reset();

      std::vector<int> lineSearchSteps; //number of steps in the line search for each iteration
      std::vector<LSStats> lsStats;     //statistics of the LS runs for each iterations
    };

    inline void LSStats::reset()
    {
      iter = 0;
      activation = 0;
      deactivation = 0;
      rankLoss = 0;
      activeConstraints = 0;
    }

    inline void SQPStats::reset()
    {
      lineSearchSteps.clear();
      lsStats.clear();
    }
  }
}