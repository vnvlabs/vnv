


#from vnv_env:latest
from beon9273a/vnv:vnv_env

COPY . ./vv-neams
ENV VNV_DIR=/root/software/vnv
ENV VNV_MATCHER=${VNV_DIR}/bin/vnv-matcher
ENV LD_LIBRARY_PATH=${VNV_DIR}/lib
RUN cd vv-neams  && mkdir -p build && cd build && cmake -DCMAKE_INSTALL_PREFIX=${VNV_DIR} -DLLVM_DIR=${LLVM_DIR} -DADIOS2_DIR=${ADIOS_DIR}/lib/cmake/adios2 .. && make && make install

# Run the tests
RUN cd vv-neams/build/test/ && ./test-runner 

# Run the examples. 
RUN cd vv-neams/build/examples/c && ./csample.a && cd ../cpp && ./injectionPoint && ./iterator && ./distributed && cd ../usage && ./App && ./Noclang
RUN cd vv-neams/build/report-generation && virtualenv virt && virt/bin/pip install -r ./requirements.txt

CMD ["./vv-neams/build/report-generation/launch.sh" , "5001" ]
