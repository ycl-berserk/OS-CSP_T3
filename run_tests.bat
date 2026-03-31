@echo off
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%"

REM 颜色代码（Windows 10+ 支持）
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

echo %YELLOW%Building test framework...%NC%
cd tests
g++ -std=c++17 test_framework.cpp -o test_framework.exe
if errorlevel 1 (
    echo %RED%Failed to build test framework%NC%
    exit /b 1
)
cd /d "%SCRIPT_DIR%"

REM 显示使用方法
:show_usage
echo Usage: run_tests.bat ^<problem_name^> [options]
echo.
echo Arguments:
echo   problem_name   Name of the problem (should match folder name in src/)
echo.
echo Options:
echo   -l ^<sec^>         Set time limit in seconds (default: 1.0^)
echo   -s ^<file^>        Specify source file path (default: src\problem_name\problem_name.cpp^)
echo   -d               Show problem description before running tests
echo   -h               Show this help message
echo.
echo Examples:
echo   run_tests.bat problem1
echo   run_tests.bat problem2 -l 2.0
echo   run_tests.bat problem3 -d
echo   run_tests.bat problem1 -s src\problem1\custom_name.cpp
exit /b 0

REM 显示问题描述
:show_description
set "problem_name=%~1"
set "desc_file=src\%problem_name%\%problem_name%.md"

if exist "%desc_file%" (
    echo %BLUE%========================================%NC%
    echo %GREEN%Problem Description: %problem_name%%NC%
    echo %BLUE%========================================%NC%
    type "%desc_file%"
    echo %BLUE%========================================%NC%
    echo.
) else (
    echo %YELLOW%Warning: Description file not found: %desc_file%%NC%
)
exit /b 0

REM 参数解析
set "PROBLEM_NAME="
set "TIME_LIMIT="
set "SOURCE_FILE="
set "SHOW_DESC=0"

:parse_args
if "%~1"=="" goto :check_args
if "%~1"=="-l" (
    set "TIME_LIMIT=%~2"
    shift
    shift
    goto :parse_args
)
if "%~1"=="-s" (
    set "SOURCE_FILE=%~2"
    shift
    shift
    goto :parse_args
)
if "%~1"=="-d" (
    set "SHOW_DESC=1"
    shift
    goto :parse_args
)
if "%~1"=="-h" (
    call :show_usage
    exit /b 0
)
if "%~1"=="--help" (
    call :show_usage
    exit /b 0
)
if "%PROBLEM_NAME%"=="" (
    set "PROBLEM_NAME=%~1"
    shift
    goto :parse_args
)
echo %RED%Unknown argument: %~1%NC%
call :show_usage
exit /b 1

:check_args
if "%PROBLEM_NAME%"=="" (
    echo %RED%Error: Problem name not specified%NC%
    call :show_usage
    exit /b 1
)

REM 设置默认源文件路径
if "%SOURCE_FILE%"=="" (
    set "SOURCE_FILE=src\%PROBLEM_NAME%\%PROBLEM_NAME%.cpp"
)

REM 检查源文件是否存在
if not exist "!SOURCE_FILE!" (
    echo %RED%Error: Source file '!SOURCE_FILE!' not found%NC%
    echo %YELLOW%Make sure the problem '%PROBLEM_NAME%' exists in src\%PROBLEM_NAME%\%PROBLEM_NAME%.cpp%NC%
    exit /b 1
)

REM 检查测试用例目录
set "TEST_DIR=tests\test_cases\%PROBLEM_NAME%"
if not exist "!TEST_DIR!" (
    echo %RED%Error: Test directory '!TEST_DIR!' not found%NC%
    echo %YELLOW%Please create test cases in tests\test_cases\%PROBLEM_NAME%\%NC%
    exit /b 1
)

REM 显示问题描述（如果需要）
if "!SHOW_DESC!"=="1" (
    call :show_description "%PROBLEM_NAME%"
)

REM 构建测试命令
echo %YELLOW%Testing problem: %PROBLEM_NAME%%NC%
echo %YELLOW%Source file: %SOURCE_FILE%%NC%
echo %YELLOW%Test directory: %TEST_DIR%%NC%
echo.

set "TEST_CMD=tests\test_framework.exe "!SOURCE_FILE!" -t "!TEST_DIR!""

REM 添加时间限制
if not "%TIME_LIMIT%"=="" (
    set "TEST_CMD=!TEST_CMD! -l !TIME_LIMIT!"
)

REM 运行测试
echo %YELLOW%Running tests...%NC%
!TEST_CMD!

REM 获取退出码
set "TEST_RESULT=%errorlevel%"

REM 清理
cd tests
del test_framework.exe temp_executable.exe 2>nul
cd /d "%SCRIPT_DIR%"

REM 输出结果
echo.
if %TEST_RESULT% equ 0 (
    echo %GREEN%✅ All tests passed for %PROBLEM_NAME%!%NC%
) else (
    echo %RED%❌ Some tests failed for %PROBLEM_NAME%!%NC%
)

exit /b %TEST_RESULT%