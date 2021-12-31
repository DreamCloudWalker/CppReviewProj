//
// Created by 邓健 on 2021/12/31.
//
#ifndef CLION_SIMPLE_RING_BUFFER_H
#define CLION_SIMPLE_RING_BUFFER_H
#include <iostream>

template<class T>
class SimpleRingBuffer {
private:
    unsigned int m_size;
    int m_front;
    int m_rear;
    T *m_data;
public:
    SimpleRingBuffer(unsigned size)
            : m_size(size),
              m_front(0),
              m_rear(0) {
        m_data = new T[size];
    }

    ~SimpleRingBuffer() {
        delete[] m_data;
    }

    bool isEmpty() {
        return m_front == m_rear;
    }

    bool isFull() {
        return m_front == (m_rear + 1) % m_size;
    }

    void push(T ele) noexcept(false) {
        if (isFull()) {
            throw std::bad_exception();
        }
        m_data[m_rear] = ele;
        m_rear = (m_rear + 1) % m_size;
    }

    T pop() noexcept(false) {
        if (isEmpty()) {
            throw std::bad_exception();
        }
        T tmp = m_data[m_front];
        m_front = (m_front + 1) % m_size;
        return tmp;
    }
};

#endif //CLION_SIMPLE_RING_BUFFER_H
