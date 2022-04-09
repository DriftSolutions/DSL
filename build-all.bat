set MTYPE=NMake Makefiles JOM
set MBUILD=jom -j 128
REM set MTYPE=NMake Makefiles
REM set MBUILD=nmake

del /s /q CMakeCache.txt CMakeFiles lib
mkdir lib

cmake -G "%MTYPE%" -DCMAKE_BUILD_TYPE=Debug ..
%MBUILD%
@echo off
IF %ERRORLEVEL% NEQ 0 (
  echo Build failure, abort...
  exit /b 1
)
@echo on

del /s /q CMakeCache.txt CMakeFiles
cmake -G "%MTYPE%" -DCMAKE_BUILD_TYPE=Release ..
%MBUILD%
@echo off
IF %ERRORLEVEL% NEQ 0 (
  echo Build failure, abort...
  exit /b 1
)
@echo on

copy dsl-*.lib lib
copy dsl-*.pdb lib
copy dsl-*.dll lib

IF [%1]==[] GOTO EOF
copy lib\*.* "%1"

:EOF
