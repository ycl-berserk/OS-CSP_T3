# OS-CSP_T3
将OS当中的一些实现转化为CSP考试T3（大模拟）的题型。在学习OS的同时练习手写代码，助力CSP300

## 📝 本地测试指南

本项目内置了一套**自动测试框架**，可以让你像在线判题系统（OJ）一样，在本地一键测试每个 OS 模拟题的正确性。

### 1. 项目结构

```
OS-CSP_T3/
├── src/                    # 各题解代码
│   ├── problem1/           # 题目1（例如：进程调度）
│   │   ├── problem1.cpp    # 你的实现代码
│   │   └── problem1.md     # 题目描述（可选）
│   └── problem2/           # 题目2（例如：内存分配）
│       ├── problem2.cpp
│       └── problem2.md
├── tests/                  # 测试框架
│   ├── test_framework.cpp  # 测试主程序
│   ├── Makefile
│   └── test_cases/         # 测试用例目录
│       ├── problem1/       # 对应 src/problem1 的测试数据
│       │   ├── 1.in        # 样例输入1
│       │   ├── 1.out       # 期望输出1
│       │   └── ...
│       └── problem2/
├── run_tests.sh            # Linux/macOS 测试脚本
├── run_tests.bat           # Windows 测试脚本
└── README.md
```

### 2. 环境要求

- **C++ 编译器**：支持 C++17 的 `g++` 或 `clang++`
- **Make**（Linux/macOS 通常自带，Windows 可使用 MinGW 的 mingw32-make）
- **Git Bash**（Windows 如需运行 `.sh` 脚本）

### 3. 运行测试

#### 测试单个题目

```bash
# Linux/macOS
./run_tests.sh 题目名

# 示例：测试“进程调度”题目
./run_tests.sh scheduler

# Windows
.\run_tests.bat 题目名
```

**输出示例**：
```
Building test framework...
Testing problem: scheduler
Source file: src/scheduler/scheduler.cpp
Test directory: tests/test_cases/scheduler

Running tests...
Compiling src/scheduler/scheduler.cpp...
Running test: 1
Input: 3
P1 P2 P3
✅ Passed

========== Summary ==========
Passed: 2/2
Score: 100%
✅ All tests passed for scheduler!
```

#### 带选项运行

```bash
# 显示题目描述（如果写了 .md 文件）
./run_tests.sh scheduler --description

# 设置时间限制（例如：2秒）
./run_tests.sh scheduler --time-limit 2.0

# 指定自定义源文件路径
./run_tests.sh scheduler -s src/scheduler/my_implementation.cpp
```

### 4. 测试框架的工作原理

1. **自动编译**：根据你指定的题目名，找到 `src/题目名/题目名.cpp` 并编译
2. **输入重定向**：依次读取 `tests/test_cases/题目名/X.in` 作为程序输入
3. **输出比较**：获取程序输出，与 `X.out` 比较（自动忽略格式差异）
4. **计时与限制**：监控运行时间，超时则判定为失败
5. **报告结果**：显示每个用例的通过/失败情况，最后给出总分

### 5. 常见问题

#### Q1: 编译错误：“g++: command not found”

**解决**：安装 C++ 编译器

- Ubuntu/Debian：`sudo apt install g++`
- macOS：`xcode-select --install`
- Windows：安装 [MinGW-w64](https://www.mingw-w64.org/) 并添加到 PATH

#### Q2: 测试输出与预期不符

**可能原因**：
- 多打印了调试信息（如 `cout << "debug"`）
- 输出格式有细微差异（空格、换行）
- 中英文标点混用

**调试方法**：框架会显示期望输出与实际输出的差异。

#### Q3: Windows 下运行 `.sh` 脚本报错

**解决**：
- 使用对应的 `.bat` 脚本（`run_tests.bat`、`create_problem.bat`）
- 或安装 [Git Bash](https://git-scm.com/) 来运行 `.sh` 脚本

### 6. 在 CSP 备考中使用本框架的建议

1. **对照大纲选题**：根据 CSP T3 常见考点（进程调度、内存管理、文件系统等）创建对应题目
2. **编写多组测试**：
   - 包含样例输入/输出
   - 添加边界条件测试（最小/最大输入）
   - 添加压力测试（大数据量）
3. **计时训练**：使用 `--time-limit` 模拟考试时间压力
4. **回归测试**：修改代码后一键运行所有历史测试，确保未破坏已有功能

------

## 📚 更多帮助

- 查看内置帮助：`./run_tests.sh -h` 或 `run_tests.bat -h`
- 提交 Issue：[GitHub Issues](https://github.com/ycl-berserk/OS-CSP_T3/issues)
- 贡献指南：欢迎提交 PR 增加新题目或改进测试框架

**祝你 CSP 取得好成绩！**
