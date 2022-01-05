#include <iostream>
#include <memory>
#include <map>
#include <pthread.h>

extern "C" {
#include <SDL.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <string>
#include "../../lib/utils/test_shared_ptr.h"
#include "../../lib/utils/simple_ring_buffer.h"

using namespace std;

// ffmpeg
const static string SOURCE_URL = "http://clips.vorwaerts-gmbh.de/big_buck_bunny.mp4";
static AVFormatContext *av_fmt_ctx = nullptr;
static AVCodec *av_codec_video = nullptr;
static AVCodecContext *av_codec_ctx_video = nullptr;
static int index_stream_video = -1;
static int width_video = 1280;
static int height_video = 720;
static double fps_video = 25;

// SDL2
SDL_Renderer *sdl_renderer;
SDL_Texture *sdl_texture;
SDL_Window *sdl_window;

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

int init_sdl_window() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { // 初始化SDL
        cout << "SDL could not initialized with error: " << SDL_GetError() << endl;
        return -1;
    }
    sdl_window = SDL_CreateWindow("Hello SDL!",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  width_video,
                                  height_video,
                                  SDL_WINDOW_ALLOW_HIGHDPI); // 创建SDL窗口
    if (nullptr == sdl_window) {
        cout << "SDL could not create window with error: " << SDL_GetError() << endl;
        return -1;
    }

    // index -1表示flags标志的第一个可用驱动程序。flags：0，或者一个或多个SDL_RendererFlags合并在一起
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
    // 创建渲染器纹理，我的视频编码格式是SDL_PIXELFORMAT_IYUV，可以从FFmpeg探测到的实际结果设置颜色格式
    sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
                                    width_video, height_video);

    return 0;
}

void destroy_sdl_window() {
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyWindow(sdl_window); // 退出SDL窗体
    SDL_Quit(); // SDL退出
}

void initFfmpeg() {
    avformat_network_init();

    // 打开媒体源，构建AVFormatContext
    if (avformat_open_input(&av_fmt_ctx, SOURCE_URL.c_str(), nullptr, nullptr)) {
        cerr << "could not open source file:" << SOURCE_URL << endl;
        exit(1);
    }

    // 找到所有流,初始化一些基本参数
    index_stream_video = av_find_best_stream(
            av_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    width_video = av_fmt_ctx->streams[index_stream_video]->codecpar->width;
    height_video = av_fmt_ctx->streams[index_stream_video]->codecpar->height;
    fps_video = av_q2d(av_fmt_ctx->streams[index_stream_video]->r_frame_rate);

    // 找到并打开解码器
    av_codec_video = avcodec_find_decoder(av_fmt_ctx->streams[index_stream_video]->codecpar->codec_id);
    av_codec_ctx_video = avcodec_alloc_context3(av_codec_video); // 根据解码器类型分配解码器上下文内存
    avcodec_parameters_to_context(av_codec_ctx_video, av_fmt_ctx->streams[index_stream_video]->codecpar); // 拷贝参数
    av_codec_ctx_video->thread_count = 8; // 解码线程数量
    cout << "thread_count = " << av_codec_ctx_video->thread_count << endl;
    if ((avcodec_open2(av_codec_ctx_video, av_codec_video, nullptr)) < 0) {
        cout << "cannot open specified video codec" << endl;
    }
}

void handleFfmpegOperations() {
    // 分配解码后的数据存储位置
    AVPacket *av_packet = av_packet_alloc();
    AVFrame *av_frame = av_frame_alloc();
    // used later to handle quit event
    SDL_Event event;
    double sleep_time = 1.0 / (double)fps_video;

    // 读帧
    while (true) {
        int result = av_read_frame(av_fmt_ctx, av_packet);
        if (result < 0) {
            cout << "end of file" << endl;
//            avcodec_send_packet(av_codec_ctx_video, nullptr); // TODO有一个最后几帧收不到的问题需要这段代码调用解决
            break;
        }

        if (av_packet->stream_index != index_stream_video) { // 目前只显示视频数据
            av_packet_unref(av_packet); // 注意清理，容易造成内存泄漏
            continue;
        }
        // 发送到解码器线程解码， avPacket会被复制一份，所以avPacket可以直接清理掉
        result = avcodec_send_packet(av_codec_ctx_video, av_packet);
        av_packet_unref(av_packet); // 注意清理，容易造成内存泄漏
        if (result != 0) {
            cout << "av_packet_unref failed" << endl;
            continue;
        }
        while (true) { // 接收解码后的数据, 解码是在后台线程，packet 和 frame并不是一一对应，多次收帧防止漏掉。
            result = avcodec_receive_frame(av_codec_ctx_video, av_frame);
            if (result != 0) { // 收帧失败，说明现在还没有解码好的帧数据，退出收帧动作。
                break;
            }
            // 像素格式刚好是YUV420P的，不用做像素格式转换
            cout << "av_frame pts : " << av_frame->pts << " color format:" << av_frame->format << endl;
//            result = SDL_UpdateTexture(sdl_texture, nullptr, av_frame->data[0], av_frame->linesize[0]);
            result = SDL_UpdateYUVTexture(sdl_texture, nullptr,
                                          av_frame->data[0],
                                          av_frame->linesize[0],
                                          av_frame->data[1],
                                          av_frame->linesize[1],
                                          av_frame->data[2],
                                          av_frame->linesize[2]);
            if (result != 0) {
                cout << "SDL_UpdateTexture failed" << endl;
                continue;
            }
            SDL_RenderClear(sdl_renderer);
            // 纹理复制给渲染器，srcrect=NULL则表示整个纹理。dstrect=NULL表示整个渲染整个渲染区域
            SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
            SDL_RenderPresent(sdl_renderer);    // 显示，将渲染器上下文中的数据，渲染到关联窗体上去
            SDL_Delay((uint32_t)(1000 * sleep_time) - 10); // 根据fps调整视频播放速率

            // handle Ctrl + C event
            SDL_PollEvent(&event);
            switch (event.type) {
                case SDL_QUIT: {
                    SDL_Quit();
                    exit(0);
                }
                default: {
                    // nothing to do
                    break;
                }
            }
        }
    }
}

int main() {
    //std::weak_ptr 用来避免 std::shared_ptr 的循环引用
    test_loop_ref();
    test_ring_buffer();
    cout << avformat_configuration() << endl; // 打印libavformat构建时配置信息。

    initFfmpeg();
    init_sdl_window();
    handleFfmpegOperations();
    destroy_sdl_window();

    return 0;
}
