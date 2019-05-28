Installation
----------

This procedure was tested on Ubuntu 18.04.2 LTS.

0. Verify that the following packages are installed 
```
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install cmake
sudo apt-get install swig
sudo apt-get install libedit-dev
sudo apt-get install python-dev
```

1. Clone the LLVM repository
```
git clone https://github.com/llvm/llvm-project.git
```

2. Configure and Compile LLVM 
```
cd llvm-project
cd llvm
mkdir OBJ_DIR
cd OBJ_DIR
cmake -G "Unix Makefiles" -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi;libunwind;lldb;compiler-rt;lld;polly;debuginfo-tests" ..
make -j2
```

3. Clone this repository in llvm/lib/Transform/.
```
cd llvm-project/llvm/lib/Transform 
git clone git@github.com:giuliostramondo/DependencyGraphGen.git 
```
4. Add the directory DependencyGraphGen to llvm/lib/Transform/CMakeLists.txt 
```
cd llvm-project/llvm/lib/Transform 
echo "add_subdirectory(DependencyGraphGen)" >> CMakeLists.txt 
```

5. Recompile LLVM with the DependencyGraphGen library.
```
cd llvm-project
cd llvm
mkdir OBJ_DIR
cd OBJ_DIR
cmake -G "Unix Makefiles" DLLVM_ENABLE_RTTI=on -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi;libunwind;lldb;compiler-rt;lld;polly;debuginfo-tests" ..
make -j2
```

Usage
------
A new library will be generated for the opt tool.

```
<PATH_TO_LLVM_OBJECTS>/bin/opt -load <PATH_TO_LLVM_OBJECTS>/lib/LLVMDependencyGraph.so -dependencyGraph <PATH_TO_INPUT_BC>/input.bc
```

Preprocess input code to obtain the input bitecode
---------------------------------------------------
The input.bc is obtained by a source file that needs to be preprocessed.
The following steps need to be perfomed to obtain a .bc file that is compatible with this tool.

0. The loops in the function of interest need to have a pragma for the full loop unroll.
```
#pragma clang loop unroll(full)
```

1. Emit the LLVM-IR representation.
```
clang -emit-llvm -Xclang -disable-O0-optnone input_source.c -c -o input_bitcode.bc
```
Optional, you can inspect the bitcode transforming it in its human-readable counterpart.
```
llvm-dis input_bitcode.bc > input_bitcode_human_readable.ll 
```

2. Extract the function of interest.
```
llvm-extract -func function_to_extract input_bitcode.bc > input_bitcode_func.bc 
```

3. Perform full loop unroll.

```
opt -mem2reg -simplifycfg -loops -lcssa -loop-simplify -loop-rotate -loop-unroll  -simplifycfg -instnamer input_bitcode_func.bc -debug > input_bitcode_func_unrolled.bc
```

4. Call this tool to generate the Data Dependency Graph.

```
~/Projects/llvm-project/llvm/OBJ_ROOT/bin/opt -load ~/Projects/llvm-project/llvm/OBJ_ROOT/./lib/LLVMDependencyGraph.so -dependencyGraph input_bitcode_func_unrolled.bc > /dev/null 2>dataflow_graph.dot
dot -Tpng dataflow_graph.dot -o dataflow_graph.png
```





