ARG FROM_IMAGE=beon9273a/vnv:vnv_env
from ${FROM_IMAGE}

ENV VNV_DIR=${SOFTWARE_DIR}/vnv
ENV VNV_MATCHER=${VNV_DIR}/bin/vnv-matcher
ENV VNV_REGISTER=${VNV_DIR}/bin/register 
ENV VNV_MERGE=${VNV_DIR}/bin/merge
ENV LD_LIBRARY_PATH=${VNV_DIR}/lib
ENV OPENMP_INCLUDE_DIR=/usr/lib/llvm-10/include/openmp
ENV Injection_DIR=${VNV_DIR}/lib/cmake

# This one is used by the extraction code to fix issues with openmp. This is the directory containing
# the Clang version of the openmp omp.h header file. We need the clang version because libtooling uses
# clang to process the input files,  not gcc. 
COPY . ${SOURCE_DIR}/vnv

#Build it
RUN cd ${SOURCE_DIR}/vnv  \
    && mkdir -p build \
    && cd build \
    && cmake -DCMAKE_INSTALL_PREFIX=${VNV_DIR} -DLLVM_DIR=${LLVM_DIR} -DADIOS2_DIR=${ADIOS_DIR}/lib/cmake/adios2 .. \
    && make -j 8 \
    && make install 

RUN cp -r ${SOURCE_DIR}/vnv/gui ${SOFTWARE_DIR}/vnv/gui 
RUN ${VNV_REGISTER} gui VnV ${VNV_DIR}/gui/vnv-config.json

RUN rm -r ${SOURCE_DIR}/vnv



