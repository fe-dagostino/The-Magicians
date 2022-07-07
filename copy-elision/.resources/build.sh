#!/bin/bash
echo g++ -std=c++20 -O3 main.cpp -o gcc_ceVSref
g++ -std=c++20 -O3 main.cpp -o gcc_ceVSref

echo clang++ -std=c++20 -O3 main.cpp -o clang_ceVSref
clang++ -std=c++20 -O3 main.cpp -o clang_ceVSref

echo g++ -std=c++20 -O3 main_runtime.cpp -o gcc_rt
g++ -std=c++20 -O3 main_runtime.cpp -o gcc_rt

echo clang++ -std=c++20 -O3 main_runtime.cpp -o clang_rt
clang++ -std=c++20 -O3 main_runtime.cpp -o clang_rt

