
from vnv_env:latest
COPY . ./vv-neams
ENV VNV_DIR=/root/software/vnv
ENV VNV_MATCHER=${VNV_DIR}/bin/vnv-matcher
RUN cd vv-neams && mkdir -p build && cd build && cmake -DCMAKE_INSTALL_PREFIX=${VNV_DIR} -DLLVM_DIR=${LLVM_DIR} -DADIOS2_DIR=${ADIOS_DIR}/lib/cmake/adios2 .. && make && make install

