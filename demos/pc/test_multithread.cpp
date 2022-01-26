//
// Created by 邓健 on 2022/1/25.
//
#include <iostream>
#include <sstream>
#include <future>
#include <cmath>

using namespace std;

std::string stringify(bool value) {
    ostringstream ret_str;
    ret_str << value;

    return ret_str.str();
}

std::string sspe_get_thread_id() {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string stid = oss.str();
    unsigned long long tid = std::stoull(stid);

    return stid;
}

bool better_is_prime(int x) {
    if (x < 2) {
        return false;
    }
    if (2 == x || 3 == x) {
        return true;
    }
    if (x % 2 == 0) {   // 偶数
        return false;
    }
    double sq = sqrt(x);
    for (int i = 3; i <= sq; i += 2) {
        if (x % i == 0) {
            return false;
        }
    }
    return true;
}

// 一个比较耗时的质数判断方法
bool is_prime(int x) {
    for (int i = 2; i < x; i++) {
        if (x % i == 0) {
            return false;
        }
    }
    return true;
}

void print_future(std::future<int> &fut) {
    int x = fut.get();
    cout << "threadId = "<< sspe_get_thread_id() << ", x = " << x << endl;
}

int main() {
    // std::async会首先创建线程执行is_prime， 任务创建之后，std::async立即返回一个std::future对象
    future<bool> future_obj = async(is_prime, 700020007);
    cout << "please wait";
    chrono::milliseconds span(100);
    while (future_obj.wait_for(span) != future_status::ready)
        cout << ".";
    cout<< endl;
    bool ret = future_obj.get();
    cout << "threadId = "<< sspe_get_thread_id() << ", final result: " << stringify(ret) << endl;

    // std::promise的作用就是提供一个不同线程之间的数据同步机制，它可以存储一个某种类型的值，
    // 并将其传递给对应的future， 即使这个future不在同一个线程中也可以安全的访问到这个值。
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    std::thread th1(print_future, std::ref(fut));
    prom.set_value(10);
    th1.join();

    return 0;
}
