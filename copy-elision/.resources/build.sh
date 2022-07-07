#!/bin/bash
echo g++ -std=c++20 -O3 main_test1.cpp -o gcc_test1
g++ -std=c++20 -O3 main_test1.cpp -o gcc_test1

echo clang++ -std=c++20 -O3 main_test1.cpp -o clang_test1
clang++ -std=c++20 -O3 main_test1.cpp -o clang_test1

echo g++ -std=c++20 -O3 main_test2.cpp -o gcc_test2
g++ -std=c++20 -O3 main_test2.cpp -o gcc_test2

echo clang++ -std=c++20 -O3 main_test2.cpp -o clang_test2
clang++ -std=c++20 -O3 main_test2.cpp -o clang_test2

