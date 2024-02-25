
#ifndef MEMORYVIDEOWRITER_H
#define MEMORYVIDEOWRITER_H

extern "C" // ffmpeg는 gcc로만 컴파일됨
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

class MemoryVideoWriter
{
public:
    MemoryVideoWriter(int width, int height, int fps);

    ~MemoryVideoWriter();

    void Reset(int fps, uint8_t *buffer);

    void WriteFrame(const cv::Mat &frame);

    uint8_t *GetMemoryBuffer();
    // std::vector<uint8_t> &GetMemoryBuffer();

    int &GetMemoryBufferSize();

private:
    void Initialize();
    void CvMatToAVFrame(const cv::Mat &frame);
    void EncodeVideoFrame(AVFrame *av_frame);

    const char *AvErr2strCustom(int errnum);

private:
    int width_;
    int height_;
    int fps_;

    int frame_count_ = 0;
    int encoded_frame_count_ = 0;

    AVFormatContext *format_ctx_ = nullptr;
    AVCodecContext *codec_ctx_ = nullptr;
    AVStream *video_stream_ = nullptr;
    const AVCodec *codec_enc_ = nullptr;
    AVDictionary *codec_options_ = nullptr;
    AVIOContext *avio_ctx_ = nullptr;

    AVPacket *pkt_ = av_packet_alloc();
    AVFrame *av_frame_ = av_frame_alloc();
    uint8_t *buffer_ = nullptr;
    AVRational dst_fps_;
    int buffer_size_ = 0;
};

#endif