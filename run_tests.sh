#!/bin/bash

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 获取脚本所在目录（仓库根目录）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 编译测试框架
echo -e "${YELLOW}Building test framework...${NC}"
cd tests
make
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to build test framework${NC}"
    exit 1
fi
cd "$SCRIPT_DIR"

# 显示使用方法
show_usage() {
    echo "Usage: ./run_tests.sh <problem_name> [options]"
    echo ""
    echo "Arguments:"
    echo "  problem_name   Name of the problem (should match folder name in src/)"
    echo ""
    echo "Options:"
    echo "  -l, --time-limit SEC   Set time limit in seconds (default: 1.0)"
    echo "  -s, --source FILE      Specify source file path (default: src/problem_name/problem_name.cpp)"
    echo "  -d, --description      Show problem description before running tests"
    echo "  -h, --help             Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./run_tests.sh problem1"
    echo "  ./run_tests.sh problem2 --time-limit 2.0"
    echo "  ./run_tests.sh problem3 -d"
    echo "  ./run_tests.sh problem1 -s src/problem1/custom_name.cpp"
}

# 显示问题描述
show_description() {
    local problem_name=$1
    local desc_file="src/${problem_name}/${problem_name}.md"
    
    if [ -f "$desc_file" ]; then
        echo -e "${BLUE}========================================${NC}"
        echo -e "${GREEN}Problem Description: ${problem_name}${NC}"
        echo -e "${BLUE}========================================${NC}"
        cat "$desc_file"
        echo -e "${BLUE}========================================${NC}"
        echo ""
    else
        echo -e "${YELLOW}Warning: Description file not found: ${desc_file}${NC}"
    fi
}

# 参数解析
PROBLEM_NAME=""
TIME_LIMIT=""
SOURCE_FILE=""
SHOW_DESC=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -l|--time-limit)
            TIME_LIMIT="$2"
            shift 2
            ;;
        -s|--source)
            SOURCE_FILE="$2"
            shift 2
            ;;
        -d|--description)
            SHOW_DESC=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            if [ -z "$PROBLEM_NAME" ]; then
                PROBLEM_NAME="$1"
            else
                echo -e "${RED}Unknown argument: $1${NC}"
                show_usage
                exit 1
            fi
            shift
            ;;
    esac
done

# 检查参数
if [ -z "$PROBLEM_NAME" ]; then
    echo -e "${RED}Error: Problem name not specified${NC}"
    show_usage
    exit 1
fi

# 设置默认源文件路径
if [ -z "$SOURCE_FILE" ]; then
    SOURCE_FILE="src/${PROBLEM_NAME}/${PROBLEM_NAME}.cpp"
fi

# 检查源文件是否存在
if [ ! -f "$SOURCE_FILE" ]; then
    echo -e "${RED}Error: Source file '$SOURCE_FILE' not found${NC}"
    echo -e "${YELLOW}Make sure the problem '$PROBLEM_NAME' exists in src/${PROBLEM_NAME}/${PROBLEM_NAME}.cpp${NC}"
    exit 1
fi

# 检查测试用例目录
TEST_DIR="tests/test_cases/${PROBLEM_NAME}"
if [ ! -d "$TEST_DIR" ]; then
    echo -e "${RED}Error: Test directory '$TEST_DIR' not found${NC}"
    echo -e "${YELLOW}Please create test cases in tests/test_cases/${PROBLEM_NAME}/${NC}"
    exit 1
fi

# 显示问题描述（如果需要）
if [ "$SHOW_DESC" = true ]; then
    show_description "$PROBLEM_NAME"
fi

# 构建测试命令
echo -e "${YELLOW}Testing problem: ${PROBLEM_NAME}${NC}"
echo -e "${YELLOW}Source file: ${SOURCE_FILE}${NC}"
echo -e "${YELLOW}Test directory: ${TEST_DIR}${NC}"
echo ""

TEST_CMD="./tests/test_framework \"$SOURCE_FILE\" -t \"$TEST_DIR\""

# 添加时间限制
if [ -n "$TIME_LIMIT" ]; then
    TEST_CMD="$TEST_CMD -l $TIME_LIMIT"
fi

# 运行测试
echo -e "${YELLOW}Running tests...${NC}"
eval $TEST_CMD

# 获取退出码
TEST_RESULT=$?

# 清理
cd tests && make clean > /dev/null 2>&1
cd "$SCRIPT_DIR"

# 输出结果
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "\n${GREEN}✅ All tests passed for ${PROBLEM_NAME}!${NC}"
else
    echo -e "\n${RED}❌ Some tests failed for ${PROBLEM_NAME}!${NC}"
fi

exit $TEST_RESULT