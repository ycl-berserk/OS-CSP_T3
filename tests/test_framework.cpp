#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace fs = std::filesystem;

struct TestCase {
    std::string input;
    std::string expected_output;
    std::string name;
};

class Tester {
private:
    std::string target_program;
    double time_limit_sec;
    int total_tests = 0;
    int passed_tests = 0;
    
    // 安全删除文件的辅助函数
    bool safe_remove(const std::string& filename) {
        for (int i = 0; i < 10; i++) {
            if (!fs::exists(filename)) return true;
            try {
                fs::remove(filename);
                return true;
            } catch (const std::exception& e) {
                #ifdef _WIN32
                    Sleep(10);
                #else
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                #endif
            }
        }
        return false;
    }
    
    std::string exec(const std::string& input) {
        std::string input_file = "temp_input.txt";
        std::string output_file = "temp_output.txt";
        std::string error_file = "temp_error.txt";
        
        std::ofstream infile(input_file);
        infile << input;
        infile.close();
        
        std::string full_cmd = target_program + " < " + input_file + " > " + output_file + " 2> " + error_file;
        
        auto start = std::chrono::high_resolution_clock::now();
        int ret = system(full_cmd.c_str());
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::string output;
        std::string error;
        
        if (duration.count() > time_limit_sec * 1000) {
            output = "TIME_LIMIT_EXCEEDED";
        } else if (ret != 0) {
            std::ifstream errfile(error_file);
            error = std::string((std::istreambuf_iterator<char>(errfile)),
                               std::istreambuf_iterator<char>());
            output = "RUNTIME_ERROR: " + error;
        } else {
            std::ifstream outfile(output_file);
            output = std::string((std::istreambuf_iterator<char>(outfile)),
                                std::istreambuf_iterator<char>());
        }
        
        safe_remove(input_file);
        safe_remove(output_file);
        safe_remove(error_file);
        
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
        
        size_t start = result.find_first_not_of(" \n\r\t");
        size_t end = result.find_last_not_of(" \n\r\t");
        
        if (start == std::string::npos) return "";
        return result.substr(start, end - start + 1);
    }
    
public:
    Tester(const std::string& program, double time_limit = 1.0) 
        : target_program(program), time_limit_sec(time_limit) {}
    
    bool run_single_test(const TestCase& test) {
        total_tests++;
        
        std::string output = exec(test.input);
        
        if (output.find("TIME_LIMIT_EXCEEDED") != std::string::npos) {
            std::cout << "  [FAIL] Test " << test.name << ": Time Limit Exceeded (" << time_limit_sec << "s)" << std::endl;
            return false;
        }
        
        if (output.find("RUNTIME_ERROR") != std::string::npos) {
            std::cout << "  [FAIL] Test " << test.name << ": Runtime Error" << std::endl;
            return false;
        }
        
        std::string normalized_output = normalize_output(output);
        std::string normalized_expected = normalize_output(test.expected_output);
        
        if (normalized_output == normalized_expected) {
            std::cout << "  [PASS] Test " << test.name << ": Passed" << std::endl;
            passed_tests++;
            return true;
        } else {
            std::cout << "  [FAIL] Test " << test.name << ": Failed" << std::endl;
            // 只在失败时显示详细差异
            std::cout << "         Expected: " << normalized_expected << std::endl;
            std::cout << "         Got:      " << normalized_output << std::endl;
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
        
        std::cout << "\nRunning " << tests.size() << " test(s)..." << std::endl;
        std::cout << "================================" << std::endl;
        
        for (auto& test : tests) {
            run_single_test(test);
        }
        
        std::cout << "================================" << std::endl;
        std::cout << "Result: " << passed_tests << "/" << total_tests << " passed" << std::endl;
        
        if (passed_tests == total_tests) {
            std::cout << "All tests passed!" << std::endl;
        } else {
            std::cout << (total_tests - passed_tests) << " test(s) failed" << std::endl;
        }
        std::cout << std::endl;
        
        return passed_tests == total_tests;
    }
};

void print_usage() {
    std::cout << "Usage: test_framework.exe <source_file> -t <test_dir> [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -t, --test <dir>       Test directory" << std::endl;
    std::cout << "  -l, --time-limit <sec> Time limit in seconds (default: 1.0)" << std::endl;
    std::cout << "  -h, --help             Show help" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  test_framework.exe ../src/problem1/problem1.cpp -t test_cases/problem1" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string source_file;
    std::string test_dir;
    double time_limit = 1.0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--test") == 0) {
            if (i + 1 < argc) test_dir = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--time-limit") == 0) {
            if (i + 1 < argc) time_limit = std::stod(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage();
            return 0;
        } else {
            source_file = argv[i];
        }
    }
    
    if (source_file.empty() || test_dir.empty()) {
        print_usage();
        return 1;
    }
    
    // 编译
    std::string executable = "temp_prog.exe";
    std::string compile_cmd = "g++ -std=c++17 " + source_file + " -o " + executable;
    
    std::cout << "Compiling " << source_file << "..." << std::endl;
    
    if (system(compile_cmd.c_str()) != 0) {
        std::cerr << "Compilation failed!" << std::endl;
        return 1;
    }
    
    // 运行测试
    #ifdef _WIN32
        // Windows: 直接使用可执行文件名
        Tester tester(executable, time_limit);
    #else
        // Linux/Mac: 需要添加 ./
        Tester tester("./" + executable, time_limit);
    #endif
    bool all_passed = tester.run_all_tests(test_dir);
    
    // 清理
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    fs::remove(executable);
    
    return all_passed ? 0 : 1;
}