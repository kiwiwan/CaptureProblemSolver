#include <Problem.h>
#include <SQP.h>

using namespace Eigen;

namespace bms
{
  RawProblem resampleProblem(const RawProblem& raw, int n);
  RawProblem resampleProblem(const std::string& filepath, int n);
}