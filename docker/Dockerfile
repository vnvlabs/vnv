
from vnv_env:latest
COPY . ./vv-neams
ENV VNV_DIR=/root/software/vnv
ENV VNV_MATCHER=${VNV_DIR}/bin/vnv-matcher
ENV LD_LIBRARY_PATH=${VNV_DIR}/lib
RUN cd vv-neams  && mkdir -p build && cd build && cmake -DCMAKE_INSTALL_PREFIX=${VNV_DIR} -DLLVM_DIR=${LLVM_DIR} -DADIOS2_DIR=${ADIOS_DIR}/lib/cmake/adios2 .. && make && make install

# Run the tests
RUN cd vv-neams/build/tests/ && ./test-runner 

# Run the examples. 
RUN cd vv-neams/build/examples/c && ./csample.a && cd ../cpp && ./injectionPoint && ./iterator && ./distributed && ../usage && cd vv-neams/build/examples/usage/ && ./App && ./NoClang

