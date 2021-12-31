#include <iostream>
#include <memory>
#include <string>
#include "utils/test_shared_ptr.h"
using namespace std;

class B;
class A {
public:
    test_shared_ptr<class B> m_spB;
};

class B {
public:
    test_shared_ptr<class A> m_spA;
};

class D;
class C {
public:
    weak_ptr<class D> m_wpD;
};

class D {
public:
    weak_ptr<class C> m_wpC;
};

void test_loop_ref()
{
    test_shared_ptr<class A> wp1;
    {
        test_shared_ptr<class A> pA(new A());
        test_shared_ptr<class B> pB(new B());

        pA->m_spB = pB;
        pB->m_spA = pA;

        wp1 = pA;
    }   // 触发pA和pB的析构
    cout << "wp1 reference number: " << wp1.use_count() << "\n";// 存在内存泄露

    weak_ptr<class C> wp2;
    {
        auto pC = make_shared<class C>();
        auto pD = make_shared<class D>();

        pC->m_wpD = pD;
        pD->m_wpC = pC;

        wp2 = pC;
    }
    cout << "wp2 reference number: " << wp2.use_count() << "\n";// 没有内存泄露，打印如下，wp2 reference number:0
}

int main() {
    //std::weak_ptr 用来避免 std::shared_ptr 的循环引用
    test_loop_ref();

    return 0;
}
