#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <cstdlib>
#include <cstring>

namespace fs = std::filesystem;

struct TestCase {
    std::string input;
    std::string expected_output;
    std::string name;
};

class Tester {
private:
    std::string target_program;
    std::string test_dir;
    double time_limit_sec;
    
    std::string exec(const std::string& cmd, const std::string& input) {
        // 创建临时文件存储输入
        std::string input_file = "temp_input.txt";
        std::string output_file = "temp_output.txt";
        std::string error_file = "temp_error.txt";
        
        std::ofstream infile(input_file);
        infile << input;
        infile.close();
        
        // 构建完整命令
        std::string full_cmd = cmd + " < " + input_file + " > " + output_file + " 2> " + error_file;
        
        auto start = std::chrono::high_resolution_clock::now();
        int ret = system(full_cmd.c_str());
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        if (duration.count() > time_limit_sec * 1000) {
            return "TIME_LIMIT_EXCEEDED";
        }
        
        if (ret != 0) {
            std::ifstream errfile(error_file);
            std::string error((std::istreambuf_iterator<char>(errfile)),
                               std::istreambuf_iterator<char>());
            return "RUNTIME_ERROR: " + error;
        }
        
        std::ifstream outfile(output_file);
        std::string output((std::istreambuf_iterator<char>(outfile)),
                          std::istreambuf_iterator<char>());
        
        // 清理临时文件
        fs::remove(input_file);
        fs::remove(output_file);
        fs::remove(error_file);
        
        return output;
    }
    
    std::string normalize_output(const std::string& output) {
        std::string result;
        bool in_space = false;
        
        for (char c : output) {
            if (std::isspace(c)) {
                if (!in_space) {
                    result += ' ';
                    in_space = true;
                }
            } else {
                result += c;
                in_space = false;
            }
        }
        
        // 去除首尾空格
        size_t start = result.find_first_not_of(" \n\r\t");
        size_t end = result.find_last_not_of(" \n\r\t");
        
        if (start == std::string::npos) return "";
        return result.substr(start, end - start + 1);
    }
    
public:
    Tester(const std::string& program, double time_limit = 1.0) 
        : target_program(program), time_limit_sec(time_limit) {}
    
    bool run_single_test(const TestCase& test) {
        std::cout << "Running test: " << test.name << std::endl;
        std::cout << "Input: " << test.input << std::endl;
        
        std::string output = exec(target_program, test.input);
        
        if (output.find("TIME_LIMIT_EXCEEDED") != std::string::npos) {
            std::cout << "❌ Time Limit Exceeded (" << time_limit_sec << "s)" << std::endl;
            return false;
        }
        
        if (output.find("RUNTIME_ERROR") != std::string::npos) {
            std::cout << "❌ Runtime Error: " << output << std::endl;
            return false;
        }
        
        std::string normalized_output = normalize_output(output);
        std::string normalized_expected = normalize_output(test.expected_output);
        
        if (normalized_output == normalized_expected) {
            std::cout << "✅ Passed" << std::endl;
            return true;
        } else {
            std::cout << "❌ Failed" << std::endl;
            std::cout << "Expected: " << normalized_expected << std::endl;
            std::cout << "Got: " << normalized_output << std::endl;
            return false;
        }
    }
    
    std::vector<TestCase> load_tests_from_dir(const std::string& dir) {
        std::vector<TestCase> tests;
        
        if (!fs::exists(dir)) {
            std::cerr << "Test directory not found: " << dir << std::endl;
            return tests;
        }
        
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.path().extension() == ".in") {
                std::string base_name = entry.path().stem().string();
                std::string out_file = dir + "/" + base_name + ".out";
                
                if (fs::exists(out_file)) {
                    std::ifstream infile(entry.path());
                    std::ifstream outfile(out_file);
                    
                    std::string input((std::istreambuf_iterator<char>(infile)),
                                     std::istreambuf_iterator<char>());
                    std::string expected((std::istreambuf_iterator<char>(outfile)),
                                        std::istreambuf_iterator<char>());
                    
                    tests.push_back({input, expected, base_name});
                }
            }
        }
        
        return tests;
    }
    
    bool run_all_tests(const std::string& test_dir) {
        auto tests = load_tests_from_dir(test_dir);
        
        if (tests.empty()) {
            std::cout << "No test cases found in " << test_dir << std::endl;
            return false;
        }
        
        int passed = 0;
        for (auto& test : tests) {
            if (run_single_test(test)) {
                passed++;
            }
            std::cout << std::endl;
        }
        
        std::cout << "========== Summary ==========" << std::endl;
        std::cout << "Passed: " << passed << "/" << tests.size() << std::endl;
        std::cout << "Score: " << (passed * 100 / tests.size()) << "%" << std::endl;
        
        return passed == tests.size();
    }
};

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS] <source_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -t, --test <test_dir>    Run tests from specified directory" << std::endl;
    std::cout << "  -l, --time-limit <sec>   Set time limit in seconds (default: 1.0)" << std::endl;
    std::cout << "  -h, --help               Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " main.cpp -t tests/problem1" << std::endl;
    std::cout << "  " << program_name << " solution.cpp -t tests/array_sum --time-limit 2.0" << std::endl;
}

std::string find_source_file(const std::string& input) {
    // 如果直接提供了文件路径，直接返回
    if (fs::exists(input)) {
        return input;
    }
    
    // 尝试在 src 目录下查找
    std::vector<std::string> possible_paths = {
        "src/" + input + "/" + input + ".cpp",
        "src/" + input + ".cpp",
        input + "/" + input + ".cpp",
        input + ".cpp"
    };
    
    for (const auto& path : possible_paths) {
        if (fs::exists(path)) {
            std::cout << "Found source file: " << path << std::endl;
            return path;
        }
    }
    
    return input; // 返回原始输入，让后续错误处理
}

int main(int argc, char* argv[]) {
    std::string source_file;
    std::string test_dir;
    double time_limit = 1.0;
    
    // 简单命令行解析
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--test") == 0) {
            if (i + 1 < argc) {
                test_dir = argv[++i];
            }
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--time-limit") == 0) {
            if (i + 1 < argc) {
                time_limit = std::stod(argv[++i]);
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            source_file = argv[i];
        }
    }
    
    if (source_file.empty() || test_dir.empty()) {
        print_usage(argv[0]);
        return 1;
    }
    
    // 查找源文件
    std::string actual_source = find_source_file(source_file);
    
    if (!fs::exists(actual_source)) {
        std::cerr << "Error: Source file not found: " << source_file << std::endl;
        return 1;
    }
    
    // 编译源文件
    std::string executable = "temp_executable";
    std::string compile_cmd = "g++ -std=c++17 " + actual_source + " -o " + executable;
    
    std::cout << "Compiling " << actual_source << "..." << std::endl;
    if (system(compile_cmd.c_str()) != 0) {
        std::cerr << "Compilation failed!" << std::endl;
        return 1;
    }
    
    // 运行测试
    Tester tester("./" + executable, time_limit);
    bool all_passed = tester.run_all_tests(test_dir);
    
    // 清理
    fs::remove(executable);
    
    return all_passed ? 0 : 1;
}