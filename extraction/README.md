# VnV Processor

## install clang on ubuntu:

```
sudo apt install clang-9 (not sure if needed) 
sudo apt install libclang-9-dev (definetly needed) 
```

## install on macos (Mohave)

```
brew install llvm
export CXX=/usr/local/Cellar/llvm/10.0.0_3/bin/clang++
export CC=/usr/local/Cellar/llvm/10.0.0_3/bin/clang
export CXXFLAGS=-I/usr/local/Cellar/llvm/10.0.0_3//lib/clang/10.0.0/include
export CFLAGS=-I/usr/local/Cellar/llvm/10.0.0_3//lib/clang/10.0.0/include
cmake -DLLVM_DIR=/usr/local/Cellar/llvm/10.0.0_3 ../vv-neams
```

This is a first attempt at writing a clang tool for 
- checking that the types passed as parameters are correct
- generating a injection point configuration file for a library. 
- eventually, writing a source-to-source translation for annotation based injection point specification.

TODO WIP
