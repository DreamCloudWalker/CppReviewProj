//
// Created by 邓健 on 2022/1/28.
//
#include <iostream>
#include <fstream>
#include <string>

int main() {
    freopen("README.md", "r", stdin);
    if (nullptr == freopen("README.md", "r", stdin)) {
        std::cout << "Read File Error!!!" << std::endl;
    } else {
        std::cout << "Read File Success." << std::endl;
    }

    std::ifstream file("README.md");    // fileName内容读取到file中
    std::string line;
    assert(file.is_open());   // 确定文件打开了；
    getline(file, line);        // 读取文件第一行，并输出
    std::cout << line << std::endl;

    return 0;
}