
from vnv_env:latest
ENV VNV_DIR=~/software/vnv
ENV VNV_MATCHER=${VNV_DIR}/bin/vnv-matcher
COPY . .
RUN mkdir -f build && cd build && cmake -DCMAKE_INSTALL_PREFIX=${VNV_DIR} -DLLVM_DIR=${LLVM_DIR} -DADIOS2_DIR=${ADIOS_DIR}/lib/cmake/adios2 .. && make && make install

