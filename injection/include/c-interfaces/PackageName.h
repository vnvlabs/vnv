#ifndef PACKAGENAME_H
#define PACKAGENAME_H

#ifndef PACKAGENAME
#  error "VnV: PACKAGENAME is not defined. Please Ensure a Macro called PACKAGENAME is defined before VnV.h is loaded"
#endif

#define VnV_E_STR(x) #x
#define VNV_STR(x) VnV_E_STR(x)
#define VNV_EX(x) x


#define PACKAGENAME_S  VNV_STR(PACKAGENAME)

#endif // PACKAGENAME_H
