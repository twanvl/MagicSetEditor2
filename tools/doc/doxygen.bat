@echo off

REM 1. Run doxygen

doxygen.exe doxygen.cfg

REM 2. Do some modificatios of the output

perl fix.pl