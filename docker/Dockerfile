ARG FROM_IMAGE=beon9273a/vnv:vnv_env
from ${FROM_IMAGE}

# Tag it
ARG COMMIT_HASH="unknown"
LABEL vnv.version.vnv=${COMMIT_HASH}

ENV VNV_DIR=${SOFTWARE_DIR}/vnv
ENV VNV_MATCHER=${VNV_DIR}/bin/vnv-matcher
ENV VNV_CONFIG=${VNV_DIR}/config/vnv-config.json
ENV LD_LIBRARY_PATH=${VNV_DIR}/lib

# This one is used by the extraction code to fix issues with openmp. This is the directory containing
# the Clang version of the openmp omp.h header file. We need the clang version because libtooling uses
# clang to process the input files,  not gcc. 

ENV OPENMP_INCLUDE_DIR=/usr/lib/llvm-10/include/openmp

COPY . ${SOURCE_DIR}/vnv


#Build it
RUN cd ${SOURCE_DIR}/vnv  \
    && mkdir -p build \
    && cd build \
    && cmake -DCMAKE_INSTALL_PREFIX=${VNV_DIR} -DLLVM_DIR=${LLVM_DIR} -DADIOS2_DIR=${ADIOS_DIR}/lib/cmake/adios2 .. \
    && make -j 8 \
    && make install


# Register the file for reading during launch.
ENV VNV_CONFIG=${VNV_CONFIG}:${SOURCE_DIR}/vnv/build/vnv.__registration__

