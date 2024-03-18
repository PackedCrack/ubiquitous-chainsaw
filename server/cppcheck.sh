#!/bin/bash


echo
echo "---------CPPCHECK - SERVER----------"
if [ ! -d build-cppcheck ]; then
    mkdir build-cppcheck
fi
cppcheck --cppcheck-build-dir=build-cppcheck --error-exitcode=1 --enable=all --suppressions-list=.suppress.cppcheck --std=c++20 main/*.cpp main/*.hpp
result=$?
if [ $result -ne 0 ]; then
    echo "Cppcheck failed with exit code $result"
    exit $result
fi
cd ..
echo "-------CPPCHECK DONE----------"
echo