#ifndef ARGTYPE_H
#define ARGTYPE_H
namespace VnV {
  template<typename T> struct argument_type;
  template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };
}
#define VnV_Arg_Type(TYPE) VnV::argument_type<void(TYPE)>::type
#endif // ARGTYPE_H
