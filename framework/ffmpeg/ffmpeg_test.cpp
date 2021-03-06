//
// Created by moqi on 2019-07-05.
//

#define LOG_TAG "xxx"

#include <af_config.h>

void testPacket();

void testFrame();

void testDemuxer();

#include <utils/frame_work_log.h>
#include <cassert>
#include "base/media/AVAFPacket.h"
#include "demuxer/avFormatDemuxer.h"
#include "../codec/avcodecDecoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
};

using namespace Cicada;

void av_buffer_free(void *opaque, uint8_t *data)
{
    AF_LOGD("xxx\n");
    av_buffer_default_free(opaque, data);
}

int main()
{
//    testPacket();
//
//    testFrame();
#if AF_HAVE_PTHREAD
//#error "have thread"
#endif
    testDemuxer();
    return 0;
}

static int callback_read(void *arg, uint8_t *buffer, int size)
{
}

void testDemuxer()
{
    int ret = 0;
    avFormatDemuxer demuxer("/Users/moqi/work/xx/framework/cmake-build-debug/aliyunmedia.ts");
//    demuxer.SetDataCallBack(callback_read, nullptr, nullptr, nullptr, nullptr);
    ret = demuxer.Open();

    if (ret < 0) {
        AF_LOGE("demuxer open error\n");
        return;
    }

    Stream_meta meta{};
    std::map<int, std::unique_ptr<avcodecDecoder>> decoders{};

    for (int i = 0; i < demuxer.GetNbStreams(); ++i) {
        demuxer.OpenStream(i);
        demuxer.GetStreamMeta(&meta, i);
        decoders[i] = std::unique_ptr<avcodecDecoder>(new avcodecDecoder());
        decoders[i]->open(&meta, nullptr, 0);
        releaseMeta(&meta);
    }

    int count = 0;
//    demuxer.Seek(260000000, 0, -1);
    demuxer.Start();

    do {
        std::unique_ptr<IAFPacket> pkt;
        ret = demuxer.ReadPacket(pkt, 0);

        if (ret > 0) {
            pkt->getInfo().dump();
            assert(pkt != nullptr);
//            _hex_dump(pkt->getData(), 16);
            AF_LOGD("pkt size is %lld\n", pkt->getSize());

            do {
                unique_ptr<IAFFrame> frame{};
                decoders[pkt->getInfo().streamIndex]->getFrame(frame, 0);

                if (frame != nullptr) {
                    frame->dump();
                }

                int ret1 = decoders[pkt->getInfo().streamIndex]->send_packet(pkt, 0);

                if (ret1 & STATUS_RETRY_IN) {
                    usleep(10000);
                } else {
                    break;
                }
            } while (true);
        }

        if (ret == -EAGAIN) {
            usleep(10000);
            continue;
        }

//        if (count++ > 10)
//            break;
    } while (ret > 0 || ret == -EAGAIN);

    // EOS
    std::unique_ptr<IAFPacket> pkt{nullptr};

    for (auto &item : decoders) {
        item.second->send_packet(pkt, 10000);

        do {
            unique_ptr<IAFFrame> frame{};
            ret = item.second->getFrame(frame, 0);

            if (frame != nullptr) {
                frame->dump();
            }
        } while (ret != STATUS_EOS);
    }

    demuxer.Stop();
}

#if 1

void testFrame()
{
    AVFrame *frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = 1920;
    frame->height = 1080;
    int ret = av_frame_get_buffer(frame, 32);

    if (ret < 0) {
        AF_LOGE("Could not allocate the video frame data\n");
    }

    frame->pts = 100;
    AVAFFrame avafFrame{frame};
    av_frame_free(&frame);
    auto clone_frame = avafFrame.clone();
    auto &info = clone_frame->getInfo();
    info.pts = 200;
    clone_frame->dump();
    avafFrame.dump();
}

void testPacket()
{
    AVPacket opkt;
    av_init_packet(&opkt);
    opkt.pts = 100;
    opkt.duration = 10;
    opkt.flags = 1;
    opkt.data = static_cast<uint8_t *>(av_malloc(10));
    opkt.size = 10;
    memset(opkt.data, 0x11, 10);
    opkt.buf = av_buffer_create(opkt.data, opkt.size, av_buffer_free, nullptr, 0);
    AVAFPacket avafPacket{opkt};
    //  av_packet_unref(&opkt);
    std::unique_ptr<IAFPacket> clone_pkt = avafPacket.clone();
    AF_DUMP_INT(clone_pkt->getInfo().duration);
    AF_DUMP_INT(clone_pkt->getInfo().pts);
    AF_DUMP_INT(clone_pkt->getSize());
    AF_LOGD("buffer is %p\n", clone_pkt->getData());
}

#endif

