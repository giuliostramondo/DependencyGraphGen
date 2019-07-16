cppJSON\_parserGenerator
------------------------

This script takes a json file as input and generates a c++ header file with a struct matching the json file structure and 
a parsing function. The llvm JSON support library is required (it can be found in llvm/Support/JSON.h)


Example
=======

```
python3 cppJSON_parserGenerator.py conf_1.json mem_comp_paramJSON
Redirecting output to mem_comp_paramJSON.hpp and mem_comp_paramJSON.cpp
```
