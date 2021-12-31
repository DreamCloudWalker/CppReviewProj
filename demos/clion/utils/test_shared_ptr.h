//
// Created by 邓健 on 2021/12/30.
//
#ifndef CLION_TEST_SHARED_PTR_H
#define CLION_TEST_SHARED_PTR_H

template<typename T>
class test_shared_ptr {
private:
    T *object;  // 指向管理的对象
    int *cnt;   // 引用计数
public:
    test_shared_ptr() {
        cnt = new int(1);
        object = NULL;
    }

    test_shared_ptr(T *t) : object(t) {
        cnt = new int(1);
    }

    ~test_shared_ptr() {
        if (--(*cnt) == 0) {
            if (object) {
                delete object;
                object = NULL;
            }
            delete cnt;
            cnt = nullptr;
        }
    }

    // 拷贝构造函数
    test_shared_ptr(const test_shared_ptr<T> &p) {
        ++(*p.cnt);

        // 要把前一个对象释放，才在后面赋值下一个对象
        if (--(*cnt) == 0) {
            if (object) {
                delete object;
                object = NULL;
            }
            delete cnt;
            cnt = nullptr;
        }

        object = p.object;
        cnt = p.cnt;
    }

    test_shared_ptr<T> & operator = (const test_shared_ptr<T> & p) {
        ++(*p.cnt);

        if (--(*cnt) == 0) {
            if (object) {
                delete object;
                object = nullptr;
            }
            delete cnt;
            cnt = nullptr;
        }

        object = p.object;
        cnt = p.cnt;

        return *this;   // 运算符重载的返回值
    }

    T* operator->() const _NOEXCEPT {
        return object;
    }

    int use_count() {
        return *(this->cnt);
    }
};

#endif //CLION_TEST_SHARED_PTR_H
