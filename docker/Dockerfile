#from vnv_env:latest
ARG FROM_IMAGE=beon9273a/vnv:vnv_env
from ${FROM_IMAGE}

# Tag it
ARG COMMIT_HASH="unknown"
LABEL vnv.version.vnv=${COMMIT_HASH}

ENV VNV_DIR=${SOFTWARE_DIR}/vnv
ENV VNV_MATCHER=${VNV_DIR}/bin/vnv-matcher
ENV VNV_CONFIG=${VNV_DIR}/config/.vnv-config.json
ENV LD_LIBRARY_PATH=${VNV_DIR}/lib

# This one is used by the extraction code to fix issues with openmp 
ENV OPENMP_INCLUDE_DIR=/usr/lib/gcc/x86_64-linux-gnu/9/include

COPY . ${SOURCE_DIR}/vnv

#Build it
RUN cd ${SOURCE_DIR}/vnv  \
    && mkdir -p build \
    && cd build \
    && cmake -DCMAKE_INSTALL_PREFIX=${VNV_DIR} -DLLVM_DIR=${LLVM_DIR} -DADIOS2_DIR=${ADIOS_DIR}/lib/cmake/adios2 .. \
    && make -j 8 \
    && make install


# Run the tests
RUN cd ${VNV_DIR}/test/ && ./test-runner 

# Run the examples. 
RUN cd ${VNV_DIR}/examples/c && ./csample.a && cd ../cpp && ./injectionPoint && ./iterator && ./euler && ./line && ./distributed   

# Create the virtual environment for the report generation (Should happen in Cmake but doesn't for some reason.
RUN cd ${VNV_DIR}/gui/report-generation && virtualenv virt && virt/bin/pip install -r ./requirements.txt

ENV VNV_CONFIG=${VNV_CONFIG}:${SOURCE_DIR}/vnv/build/vnv.__registration__
