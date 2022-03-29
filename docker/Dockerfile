


#from vnv_env:latest
ARG FROM_IMAGE=beon9273a/vnv:vnv_env
from ${FROM_IMAGE}



COPY . ./vv-neams
ENV VNV_DIR=/root/software/vnv
ENV VNV_MATCHER=${VNV_DIR}/bin/vnv-matcher
ENV VNV_CONFIG=${VNV_DIR}/.vnv-config.json

ENV LD_LIBRARY_PATH=${VNV_DIR}/lib

RUN cd vv-neams  && mkdir -p build && cd build && cmake -DCMAKE_INSTALL_PREFIX=${VNV_DIR} -DLLVM_DIR=${LLVM_DIR} -DADIOS2_DIR=${ADIOS_DIR}/lib/cmake/adios2 .. && make && make install

RUN rm -r /vv-neams

# Run the tests
RUN cd ${VNV_DIR}/test/ && ./test-runner 

# Run the examples. 
RUN cd ${VNV_DIR}/examples/c && ./csample.a && cd ../cpp && ./injectionPoint && ./iterator && ./line && ./distributed   

# Create the virtual environment for the report generation.
RUN cd ${VNV_DIR}/gui/report-generation && virtualenv virt && virt/bin/pip install -r ./requirements.txt

ARG COMMIT_TAG="unknown"
LABEL vnv=${COMMIT_TAG}
