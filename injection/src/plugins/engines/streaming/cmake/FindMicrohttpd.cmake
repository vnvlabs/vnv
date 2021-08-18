# - Find microhttpd
# Find the microhttpd includes and library
#
#  MICROHTTPD_INCLUDE_DIR - Where to find microhttpd includes
#  MICROHTTPD_LIBRARIES   - List of libraries when using microhttpd
#  MICROHTTPD_FOUND       - True if microhttpd was found

IF(MICROHTTPD_INCLUDE_DIR)
  SET(MICROHTTPD_FIND_QUIETLY TRUE)
ENDIF(MICROHTTPD_INCLUDE_DIR)

FIND_PATH(MICROHTTPD_INCLUDE_DIR "microhttpd.h"
  PATHS
  $ENV{MICROHTTP_DIR}/include
  $ENV{EXTERNLIBS}/microhttpd/include
  ~/Library/Frameworks/include
  /Library/Frameworks/include
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
  DOC "microhttpd - Headers"
)

SET(MICROHTTPD_NAMES microhttpd libmicrohttpd)
SET(MICROHTTPD_DBG_NAMES microhttpdd libmicrohttpdd)

FIND_LIBRARY(MICROHTTPD_LIBRARY NAMES ${MICROHTTPD_NAMES}
  PATHS
  ${MICROHTTPD_DIR}
  $ENV{MICROHTTPD_DIR}
  $ENV{EXTERNLIBS}/microhttpd
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  PATH_SUFFIXES lib lib64
  DOC "microhttpd - Library"
)

INCLUDE(FindPackageHandleStandardArgs)

SET(MICROHTTPD_LIBRARIES ${MICROHTTPD_LIBRARY} ${microhttpdPLATFORM_LIBRARY})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Microhttpd DEFAULT_MSG MICROHTTPD_LIBRARY MICROHTTPD_INCLUDE_DIR)
  
MARK_AS_ADVANCED(MICROHTTPD_LIBRARY microhttpdPLATFORM_LIBRARY MICROHTTPD_INCLUDE_DIR)
  
IF(MICROHTTPD_FOUND)
  SET(MICROHTTPD_INCLUDE_DIRS ${MICROHTTPD_INCLUDE_DIR})
ENDIF(MICROHTTPD_FOUND)
