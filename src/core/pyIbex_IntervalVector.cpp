//============================================================================
//                                P Y I B E X
// File        : pyIbex_IntervalVector.cpp
// Author      : Benoit Desrochers
// Copyright   : Benoit Desrochers
// License     : See the LICENSE file
// Created     : Dec 28, 2014
//============================================================================


#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pyIbex_type_caster.h>

#include "ibex_IntervalVector.h"
#include "ibex_IntervalMatrix.h"
#include <stdexcept>
#include <sstream>

namespace py = pybind11;
using py::self;

using ibex::Interval;
using ibex::IntervalVector;
using ibex::IntervalMatrix;
using ibex::Vector;

template <typename T>
bool check_cvt(py::object& obj){
  try{
    obj.cast<T>();
  } catch (py::cast_error){
    return false;
  }
  return true;
}

template<typename T>
std::vector<T> pyList2Vector(py::list &lst){
  std::vector<T> v;
  for (auto item : lst)
    v.push_back(item.cast<T>());
  return v;
}

template<typename T>
std::vector<T> pyTuple2Vector(py::tuple &tup){
  std::vector<T> v;
  for (auto item : tup)
    v.push_back(item.cast<T>());
  return v;
}

void CreateWithList(IntervalVector &instance,  py::list & lst){

  int size_n1 = lst.size();
  if (size_n1 < 1){
    throw std::invalid_argument("Size of the input list is 0");
  }
  py::object obj = lst[0];
  if (check_cvt<py::list>(obj)){
    double (*tmp)[2] = new double[size_n1][2];
    int i = 0;
    for (auto item : lst){
      int j = 0;
      for (auto val : item){
        tmp[i][j] = val.cast<double>();
        j++;
      }
      i++;
    }
    new(&instance) IntervalVector(size_n1, tmp);
    delete tmp;
  } else if (check_cvt<double>(obj)){
    Vector v(size_n1, &pyList2Vector<double>(lst)[0]);
    new(&instance) IntervalVector(v);
  } else {
    throw std::invalid_argument("Invalid arguments type");
  }
}
// boost::shared_ptr<ibex::IntervalVector> CreateWithList(const py::list & lst)
// {
//   // construct with a list here
//   extract<double> get_double(lst[0]);
//   if (get_double.check()){
//       std::vector<double> v = to_std_vector<double>(lst);
//       return shared_ptr<ibex::IntervalVector>(new ibex::IntervalVector( Vector(v.size(), &v[0])));
//   } else {
//         double (*tmp)[2] = new double[len(lst)][2];
//         for(int i = 0; i < len(lst); i++){
//             extract<list> get_list(lst[i]);
//             if (get_list.check()){
//                 assert(len(lst[i]) == 2);
//                 for(int j = 0; j < 2;j++){
//                     tmp[i][j] = extract<double>(lst[i][j]);
//                 }
//             }
//         }
//         boost::shared_ptr<ibex::IntervalVector> ptr =  shared_ptr<ibex::IntervalVector>(new ibex::IntervalVector( len(lst), tmp));
//         delete[] tmp;
//         return ptr;
//     }
// }
void CreateWithIntAndList(IntervalVector &instance, int ndim, py::list & lst){
    // construct with a list here
  py::object obj = lst[0];
  if (check_cvt<double>(obj) && lst.size() == 2){
    std::vector<double> v = pyList2Vector<double>(lst);
    new(&instance) IntervalVector(ndim, Interval(v[0], v[1]));
  } else {
    throw std::invalid_argument("syntax is IntervalVector(2, [1,2])");
  }
}

// boost::shared_ptr<ibex::IntervalVector> CreateWithIntAndList(int ndim, const py::list & lst){
//     // construct with a list here
//     extract<double> get_double(lst[0]);
//     if (get_double.check()){
//         std::vector<double> v = to_std_vector<double>(lst);
//         return shared_ptr<ibex::IntervalVector>(new ibex::IntervalVector(ndim, Interval(v[0], v[1])));
//     }
// }


void CreateWithTuple(IntervalVector &instance, py::tuple & tup)
{
  int size_n1 = tup.size();
  if (size_n1 < 1){
    throw std::invalid_argument("Size of the input list is 0");
  }
  py::object obj = tup[0];
  if (check_cvt<double>(obj)){
    Vector v(size_n1, &pyTuple2Vector<double>(tup)[0]);
    new(&instance) IntervalVector(v);
  } else if(check_cvt<Interval>(obj)){
    std::vector<Interval> v = pyTuple2Vector<Interval>(tup);
    new(&instance) IntervalVector(size_n1);
    for (int i = 0; i < v.size(); i++){
      instance[i] = v[i];
    }
  } else {
    throw std::invalid_argument("Invalid arguments type");
  }
}


Interval& getitem(IntervalVector& X, int i){
      return X[i];
}

void setitem(IntervalVector& X, int i, Interval& itv){
      X[i] = itv;
}

std::vector<IntervalVector> complementary_wrapper(IntervalVector& X){
      IntervalVector* result;
      int n = X.complementary(result);
      std::vector<IntervalVector> v;
      v.assign(result, result+n);
      return v;
}

std::vector<IntervalVector> diff_wrapper(IntervalVector& X, IntervalVector& Y){
    IntervalVector* result;
    int n = X.diff(Y, result);
    std::vector<IntervalVector> v;
    v.assign(result, result+n);
    return v;
}


void assignItv(IntervalVector& self, const IntervalVector& other){
    self = other;
}

IntervalVector my_copy(IntervalVector& itv){
    return IntervalVector(itv);
}

std::string to_string(const IntervalVector& a){
  std::stringstream ss;
  ss << a;
  return ss.str();
}


// BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(bisect_overloads, bisect, 1,2);

void export_IntervalVector(py::module& m){



    py::class_<IntervalVector>(m, "IntervalVector")
            .def(py::init<int>())
            .def(py::init<int,const Interval>())
            .def(py::init<const IntervalVector&>() )
            .def("__init__", &CreateWithList) //make_constructor(&CreateWithList))
            .def("__init__", &CreateWithIntAndList)
            .def("__init__", &CreateWithTuple)

            // .def("__init__", make_constructor(&CreateWithPyArrayObject))
            // .def( "__getitem__", &IntervalVector::getitem, return_value_policy<copy_const_reference>()  )
            // .def( "__setitem__", &IntervalVector::setitem)
            .def("__getitem__", getitem, py::return_value_policy::reference_internal)
            .def("__setitem__", setitem)

            .def("assign", &assignItv)
            .def( self == self )
            .def( self != self )
            .def( self + self )
            .def( self - self )
            .def( self * self)
            .def( self & self )
            .def( self | self )
            .def( -self )

            .def( self += self)
            .def( self += double())

            .def( self -= self)
            .def( self -= double())

            .def( self *= double())
            .def( "__imul__", [](IntervalVector& a, const Interval& x){return a*=x;})
            // .def( "__mul__", [](IntervalVector& a, const Interval& x){return x*a;})
            // Move to Interval wrapper
            // .def( "__rmul__", [](Interval& x, IntervalVector& a){return x*a;})
            // .def( "__rmul__", [](IntervalVector& a, Interval& x ){return x*a;})
            // .def(Interval() * self)

            .def( "__mul__", [](IntervalVector& a, const Vector& x){return a*x;})
            .def( "__rmul__", [](IntervalVector& a, const Vector& x){return x*a;})


            // .def( self *= Interval())
            // .def( self * Vector())

            .def( self &= self)
            .def( self |= self)




            // .def (self + Vector())
            .def("__add__", [](IntervalVector& a, const Vector& x){return a+x;})
            .def("__iadd__", [](IntervalVector& a, const Vector& x){return a+x;})
            .def("__radd__", [](IntervalVector& a, const Vector& x){return a+x;})

            .def("__sub__", [](IntervalVector& a, const Vector& x){return a-x;})
            .def("__isub__", [](IntervalVector& a, const Vector& x){return a-=x;})
            .def("__rsub__", [](IntervalVector& a, const Vector& x){return x-a;})

            // .def ( Vector() + self )

            // .def (self - Vector())
            // .def ( Vector() - self )

            // .def (self * Vector())
            // .def ( Vector() * self )
            .def (double() * self)
            // .def (constInterval&>() * Vector())
            // .def (Interval() * self)




            // .def( self - double())
            // .def( double() - self)
            // .def( self * other<double>())
            // .def( other<double>() * self)
            // .def( self / double())
            // .def( double() / self)

            // .def( -self )
            // .def( self + double())

            // .def(repr(self))
            .def("__repr__", &to_string)
            .def("copy", &my_copy)
            .def("__len__", &IntervalVector::size )
            .def("size", &IntervalVector::size )
            .def_static( "empty", &IntervalVector::empty ) //.staticmethod("empty")
            .def( "set_empty", &IntervalVector::set_empty)
            .def( "clear", &IntervalVector::clear)
            .def( "init", &IntervalVector::init)
            .def( "inflate", &IntervalVector::inflate, py::return_value_policy::reference_internal)
            .def( "resize", &IntervalVector::resize)
            .def( "subvector", &IntervalVector::subvector) //, return_value_policy<return_by_value>())
            .def( "put", &IntervalVector::put)
            .def( "size" , &IntervalVector::size )
            .def( "lb" , &IntervalVector::lb )
            .def( "ub" , &IntervalVector::ub )
            .def( "mid" , &IntervalVector::mid )
            .def( "mig" , &IntervalVector::mig )
            .def( "mag" , &IntervalVector::mag )
            .def( "is_empty" , &IntervalVector::is_empty )
            .def( "is_flat" , &IntervalVector::is_flat )
            .def( "is_unbounded" , &IntervalVector::is_unbounded )
            .def( "is_subset" , &IntervalVector::is_subset )
            .def( "is_strict_subset" , &IntervalVector::is_strict_subset )
            .def( "is_interior_subset" , &IntervalVector::is_interior_subset )
            .def( "is_strict_interior_subset" , &IntervalVector::is_strict_interior_subset )
            .def( "is_superset" , &IntervalVector::is_superset )
            .def( "is_strict_superset" , &IntervalVector::is_strict_superset )
            .def( "contains" , &IntervalVector::contains )
            .def( "interior_contains" , &IntervalVector::interior_contains )
            .def( "intersects" , &IntervalVector::intersects )
            .def( "overlaps" , &IntervalVector::overlaps )
            .def( "is_disjoint" , &IntervalVector::is_disjoint )
            .def( "is_zero" , &IntervalVector::is_zero )
            .def( "is_bisectable" , &IntervalVector::is_bisectable )
            .def( "rad",    &IntervalVector::rad )
            .def( "diam",   &IntervalVector::diam )
            .def( "sort_indices",   &IntervalVector::sort_indices )
            .def( "max_diam",   &IntervalVector::max_diam, "return maximal diameter of (*this)" )
            .def( "min_diam",   &IntervalVector::min_diam, "return minimal diameter of (*this)")
            .def( "volume",     &IntervalVector::volume )
            .def( "perimeter",  &IntervalVector::perimeter )
            .def( "rel_distance",   &IntervalVector::rel_distance )

            .def( "diff",   &diff_wrapper )
            .def( "complementary",  &complementary_wrapper )
            .def( "bisect",     &IntervalVector::bisect, "bisect interval vector", py::arg("i"), py::arg("ratio")=0.5 )
            ;

            m.def( "abs", ( IntervalVector (*) (const IntervalVector& ) ) &ibex::abs);
            m.def( "bwd_add", (bool (*) (const IntervalVector&, IntervalVector&, IntervalVector&) ) &ibex::bwd_add);
            m.def( "bwd_sub", (bool (*) (const IntervalVector&, IntervalVector&, IntervalVector&) ) &ibex::bwd_sub);
            m.def( "cart_prod", (IntervalVector (*) (const IntervalVector&, const IntervalVector&)) &ibex::cart_prod);

            // hadamard_product overloading
            // IntervalVector (*hadamard_product_1) (const IntervalVector& , const Vector& ) = &ibex::hadamard_product;
            // IntervalVector (*hadamard_product_2) (const Vector& , const IntervalVector& ) = &ibex::hadamard_product;
            // IntervalVector (*hadamard_product_3) (const IntervalVector& , const IntervalVector& ) = &ibex::hadamard_product;
            m.def( "hadamard_product", (IntervalVector (*) (const IntervalVector& , const Vector& ) ) &ibex::hadamard_product);
            m.def( "hadamard_product", (IntervalVector (*) (const Vector& , const IntervalVector& ) ) &ibex::hadamard_product);
            m.def( "hadamard_product", (IntervalVector (*) (const IntervalVector& , const IntervalVector& ) ) &ibex::hadamard_product);

            // bwd_mul overloading
            // bool (*bwd_mul_1) (const IntervalVector&, Interval&, IntervalVector&) = &ibex::bwd_mul;
            // bool (*bwd_mul_2) (const Interval&, IntervalVector&, IntervalVector&) = &ibex::bwd_mul;
            m.def( "bwd_mul", (bool (*) (const IntervalVector&, Interval&, IntervalVector&)) &ibex::bwd_mul);
            m.def( "bwd_mul", (bool (*) (const Interval&, IntervalVector&, IntervalVector&)) &ibex::bwd_mul);

};
