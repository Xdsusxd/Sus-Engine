@echo off
cmake --preset windows-msvc -B build
cmake --build build --config Release --parallel
