@echo off
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%"

REM 如果没有参数，显示帮助
if "%~1"=="" goto :show_usage
if "%~1"=="-h" goto :show_usage
if "%~1"=="--help" goto :show_usage

REM 获取问题名称（第一个参数）
set "PROBLEM_NAME=%~1"
shift

REM 解析其他选项
set "TIME_LIMIT="
set "SOURCE_FILE="
set "SHOW_DESC=0"

:parse_options
if "%~1"=="" goto :run_test
if "%~1"=="-l" (
    set "TIME_LIMIT=%~2"
    shift
    shift
    goto :parse_options
)
if "%~1"=="-s" (
    set "SOURCE_FILE=%~2"
    shift
    shift
    goto :parse_options
)
if "%~1"=="-d" (
    set "SHOW_DESC=1"
    shift
    goto :parse_options
)
echo [ERROR] Unknown option: %~1
goto :show_usage

:run_test
REM 设置默认源文件路径
if "%SOURCE_FILE%"=="" (
    set "SOURCE_FILE=src\%PROBLEM_NAME%\%PROBLEM_NAME%.cpp"
)

REM 检查 g++
where g++ >nul 2>nul
if errorlevel 1 (
    echo [ERROR] g++ not found. Please install MinGW-w64 and add it to PATH.
    echo Download: https://www.mingw-w64.org/
    pause
    exit /b 1
)

REM 编译测试框架
if not exist "tests\test_framework.exe" (
    echo [INFO] Building test framework...
    cd tests
    g++ -std=c++17 test_framework.cpp -o test_framework.exe
    if errorlevel 1 (
        echo [ERROR] Failed to build test framework
        pause
        exit /b 1
    )
    cd /d "%SCRIPT_DIR%"
)

REM 检查源文件
if not exist "%SOURCE_FILE%" (
    echo [ERROR] Source file not found: %SOURCE_FILE%
    echo.
    echo Make sure the problem '%PROBLEM_NAME%' exists in src\%PROBLEM_NAME%\%PROBLEM_NAME%.cpp
    pause
    exit /b 1
)

REM 检查测试用例目录
set "TEST_DIR=tests\test_cases\%PROBLEM_NAME%"
if not exist "%TEST_DIR%" (
    echo [ERROR] Test directory not found: %TEST_DIR%
    echo.
    echo Please create test cases in tests\test_cases\%PROBLEM_NAME%\
    echo Each test case needs a .in file and a .out file.
    pause
    exit /b 1
)

REM 显示问题描述
if "%SHOW_DESC%"=="1" (
    set "DESC_FILE=src\%PROBLEM_NAME%\%PROBLEM_NAME%.md"
    if exist "!DESC_FILE!" (
        echo.
        echo ========================================
        echo    Problem: %PROBLEM_NAME%
        echo ========================================
        echo.
        type "!DESC_FILE!"
        echo.
        echo ========================================
        echo.
    ) else (
        echo [WARNING] Description file not found: !DESC_FILE!
    )
)

REM 显示测试信息
echo.
echo ========================================
echo    Testing: %PROBLEM_NAME%
echo ========================================
echo Source: %SOURCE_FILE%
echo Tests:  %TEST_DIR%
if not "%TIME_LIMIT%"=="" echo Limit:  %TIME_LIMIT% seconds
echo.

REM 构建并运行测试命令
set "TEST_CMD=tests\test_framework.exe "%SOURCE_FILE%" -t "%TEST_DIR%""
if not "%TIME_LIMIT%"=="" set "TEST_CMD=!TEST_CMD! -l %TIME_LIMIT%"

!TEST_CMD!
set "TEST_RESULT=%errorlevel%"

REM 显示结果
echo.
if %TEST_RESULT% equ 0 (
    echo ========================================
    echo    [SUCCESS] All tests passed!
    echo ========================================
) else (
    echo ========================================
    echo    [FAILED] Some tests failed!
    echo ========================================
)

REM 清理
del tests\temp_*.txt tests\temp_*.exe 2>nul

pause
exit /b %TEST_RESULT%

:show_usage
echo ========================================
echo    OS-CSP_T3 Local Test Tool
echo ========================================
echo.
echo Usage: run_tests.bat ^<problem_name^> [options]
echo.
echo Arguments:
echo   problem_name   Name of the problem (folder name in src/)
echo.
echo Options:
echo   -l ^<sec^>         Set time limit in seconds (default: 1.0)
echo   -s ^<file^>        Specify source file path
echo   -d               Show problem description
echo   -h, --help       Show this help message
echo.
echo Examples:
echo   run_tests.bat problem1
echo   run_tests.bat problem2 -l 2.0
echo   run_tests.bat problem3 -d
echo   run_tests.bat problem1 -s src\problem1\my_solution.cpp
echo.
pause
exit /b 0