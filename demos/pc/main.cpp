#include <iostream>
#include <memory>

extern "C" {
#include <SDL2/SDL.h>
}

#include <string>
#include "../../lib/utils/test_shared_ptr.h"
#include "../../lib/utils/simple_ring_buffer.h"

using namespace std;

const int WIDTH = 640, HEIGHT = 480; // SDL窗口的宽和高

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

void test_loop_ref() {
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

void test_ring_buffer() {
    SimpleRingBuffer<int> ringBuffer(5);
    ringBuffer.push(1);
    ringBuffer.push(2);
    ringBuffer.push(3);
    ringBuffer.push(4);
    for (int i = 0; i < 4; i++)
        cout << ringBuffer.pop() << endl;
    ringBuffer.push(5);
    ringBuffer.push(5);
    ringBuffer.push(5);
    cout << ringBuffer.pop() << endl;
    cout << ringBuffer.pop() << endl;
    cout << ringBuffer.pop() << endl;
//    cout << ringBuffer.pop() << endl;
}

SDL_Window* init_sdl_window() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { // 初始化SDL
        cout << "SDL could not initialized with error: " << SDL_GetError() << endl;
    }
    SDL_Window *window = SDL_CreateWindow("Hello SDL!",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, WIDTH,
                                          HEIGHT,
                                          SDL_WINDOW_ALLOW_HIGHDPI); // 创建SDL窗口
    if (nullptr == window) {
        cout << "SDL could not create window with error: " << SDL_GetError() << endl;
    }

    return window;
}

void loop_sdl_window() {
    SDL_Event window_event; // SDL窗口事件
    while(true) {
        if (SDL_PollEvent(&window_event)) { // 对当前待处理事件进行轮询。
            if (SDL_QUIT == window_event.type) { // 如果事件为推出SDL，结束循环。
                cout << "SDL quit!!" << endl;
                break;
            }
        }
    }
}

void destroy_sdl_window(SDL_Window * window) {
    SDL_DestroyWindow(window); // 推出SDL窗体
    SDL_Quit(); // SDL推出
}

int main() {
    //std::weak_ptr 用来避免 std::shared_ptr 的循环引用
    test_loop_ref();
    test_ring_buffer();

    SDL_Window *window = init_sdl_window();
    loop_sdl_window();
    destroy_sdl_window(window);

    return 0;
}
