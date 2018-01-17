#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include <bms/LeastSquare.h>
#include <bms/Problem.h>
#include <bms/QuadraticObjective.h>
#include <bms/SQP.h>

#include "converters.h"

namespace py = boost::python;
namespace np = boost::python::numpy;

BOOST_PYTHON_MODULE(PyBoundednessMPCSolver)
{
  Py_Initialize();
  np::initialize();
  pygen::convert<double>(pygen::Converters::Vector);

  using namespace bms;

  py::class_<LeastSquare>("LeastSquare", py::init<int>());

  py::class_<LeastSquareObjective>("LeastSquareObjective", py::init<const Eigen::VectorXd&>());

  LeastSquareObjective& (Problem::*pyObjective)() = &Problem::objective;
  BoundenessConstraint& (Problem::*pyNonLinearConstraint)() = &Problem::nonLinearConstraint;
  LinearConstraints& (Problem::*pyLinearConstraints)() = &Problem::linearConstraints;
  py::class_<Problem>("Problem", py::init<RawProblem>())
    .add_property("size", &Problem::size)
    .def("linear_constraints", pyLinearConstraints, py::return_internal_reference<>())
    .def("nonlinear_constraint", pyNonLinearConstraint, py::return_internal_reference<>())
    .def("objective", pyObjective, py::return_internal_reference<>())
    .def("set_init_omega", &Problem::set_init_omega)
    .def("set_init_omega_max", &Problem::set_init_omega_max)
    .def("set_init_omega_min", &Problem::set_init_omega_min)
    .def("set_init_zbar", &Problem::set_init_zbar)
    .def("set_init_zbar_deriv", &Problem::set_init_zbar_deriv)
    .def("set_lambda_max", &Problem::set_lambda_max)
    .def("set_lambda_min", &Problem::set_lambda_min)
    .def("set_lambdas", &Problem::set_lambdas)
    .def("set_target_height", &Problem::set_target_height);

  py::class_<RawProblem>("RawProblem")
    .def("read", &RawProblem::read)
    .add_property("delta", 
        py::make_getter(&RawProblem::delta, 
          py::return_value_policy<py::copy_non_const_reference>()),
        py::make_setter(&RawProblem::delta))
    .add_property("Phi_", 
        py::make_getter(&RawProblem::Phi_, 
          py::return_value_policy<py::copy_non_const_reference>()),
        py::make_setter(&RawProblem::Phi_))
    .def_readwrite("g", &RawProblem::g)
    .def_readwrite("lambda_min", &RawProblem::lambda_min)
    .def_readwrite("lambda_max", &RawProblem::lambda_max)
    .def_readwrite("init_omega_min", &RawProblem::init_omega_min)
    .def_readwrite("init_omega_max", &RawProblem::init_omega_max)
    .def_readwrite("init_zbar", &RawProblem::init_zbar)
    .def_readwrite("init_zbar_deriv", &RawProblem::init_zbar_deriv)
    .def_readwrite("target_height", &RawProblem::target_height);

  py::enum_<SolverStatus>("SolverStatus")
    .value("Converge", SolverStatus::Converge)
    .value("MaxIteration", SolverStatus::MaxIteration)
    .value("LineSearchFailed", SolverStatus::LineSearchFailed)
    .value("NoLinearlyFeasiblePoint", SolverStatus::NoLinearlyFeasiblePoint)
    .value("NumericallyEquivalentIterates", SolverStatus::NumericallyEquivalentIterates)
    .value("Fail", SolverStatus::Fail);

  py::class_<SQP>("SQP", py::init<int>())
    .add_property("nb_iter", &SQP::numberOfIterations)
    .def("lambda_", &SQP::lambda, py::return_value_policy<py::copy_const_reference>())
    .def("solve", &SQP::solve)
    .def("x", &SQP::x, py::return_value_policy<py::copy_const_reference>());
}