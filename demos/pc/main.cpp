#include <iostream>
#include <memory>

extern "C" {
#include <SDL.h>
#include <SDL2/SDL_image.h>
}

#include <string>
#include "../../lib/utils/test_shared_ptr.h"
#include "../../lib/utils/simple_ring_buffer.h"

using namespace std;

const int WIDTH = 720, HEIGHT = 1280; // SDL窗口的宽和高
SDL_Surface *imageSurface = nullptr;   // 申明用于加载图片的SDL_Surface
SDL_Surface *windowSurface = nullptr;  // 申明用于窗体相关的SDL_Surface

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
                                          SDL_WINDOWPOS_CENTERED,
                                          WIDTH,
                                          HEIGHT,
                                          SDL_WINDOW_ALLOW_HIGHDPI); // 创建SDL窗口
    if (nullptr == window) {
        cout << "SDL could not create window with error: " << SDL_GetError() << endl;
        return window;
    }

    if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG)) {
        cout << "SDL_image could not init with error: " << IMG_GetError() << endl;
        return window;
    }

    windowSurface = SDL_GetWindowSurface(window);
    imageSurface = IMG_Load("tifa.jpeg");
    if (nullptr == imageSurface) {
        cout << "SDL could not load image with error: " << SDL_GetError() << endl;
    }

    return window;
}

void loop_sdl_window(SDL_Window * window) {
    SDL_Event window_event; // SDL窗口事件
    while(true) {
        if (SDL_PollEvent(&window_event)) { // 对当前待处理事件进行轮询。
            if (SDL_QUIT == window_event.type) { // 如果事件为推出SDL，结束循环。
                cout << "SDL quit!!" << endl;
                break;
            }
        }
        SDL_BlitSurface(imageSurface, nullptr, windowSurface, nullptr);
        SDL_UpdateWindowSurface(window);
    }
}

void destroy_sdl_window(SDL_Window * window) {
    imageSurface = nullptr;
    windowSurface = nullptr;
    SDL_DestroyWindow(window); // 推出SDL窗体
    SDL_Quit(); // SDL推出
}

int main() {
    //std::weak_ptr 用来避免 std::shared_ptr 的循环引用
    test_loop_ref();
    test_ring_buffer();

    // SDL2 part
    SDL_Window *window = init_sdl_window();
    loop_sdl_window(window);
    destroy_sdl_window(window);

    return 0;
}
