#include "MemoryVideoWriter.h"

MemoryVideoWriter::MemoryVideoWriter(int width, int height, int fps) : width_(width), height_(height), fps_(fps), dst_fps_({fps, 1})
{
    // Initialize FFmpeg network components
    avformat_network_init();
    initialize();
}

MemoryVideoWriter ::~MemoryVideoWriter()
{

    // Free AVFrame
    av_frame_free(&av_frame);

    // Close memory buffer
    // avio_close(format_ctx_->pb);

    // Free contexts
    av_packet_free(&pkt);
    avformat_free_context(format_ctx_);
    avcodec_free_context(&codec_ctx_);
    // avio_context_free(&avio_ctx_);
}

void MemoryVideoWriter ::initialize()
{
    // Allocate memory context
    if (avio_open_dyn_buf(&avio_ctx_) < 0)
    {
        std::cerr << "Failed to open dynamic buffer" << std::endl;
        exit(1);
    }

    // Create output format context
    avformat_alloc_output_context2(&format_ctx_, NULL, "mp4", NULL);
    if (!format_ctx_)
    {
        std::cerr << "Failed to allocate output format context" << std::endl;
        exit(1);
    }

    // Set codec parameters
    // const AVCodec *codec_enc_temp = avcodec_find_encoder(AV_CODEC_ID_H264);
    const AVCodec *codec_enc_temp = avcodec_find_encoder(format_ctx_->oformat->video_codec);

    codec_enc_ = codec_enc_temp;
    if (codec_enc_ == NULL)
    {
        fprintf(stderr, "encoder not found ...\n");
        exit(1);
    }
    codec_ctx_ = avcodec_alloc_context3(codec_enc_);
    if (codec_ctx_ == NULL)
    {
        fprintf(stderr, "avcodec_alloc_context3 failed\n");
        exit(1);
    }

    if (codec_enc_->capabilities & AV_CODEC_CAP_ENCODER_FLUSH)
    {
        // AV_CODEC_CAP_ENCODER_FLUSH를 지원하는 경우 처리할 내용
        std::cout << "support  AV_CODEC_CAP_ENCODER_FLUSH" << std::endl;
    }
    else
    {
        std::cout << "not support  AV_CODEC_CAP_ENCODER_FLUSH" << std::endl;
    }

    // codec_ctx_->codec_id = AV_CODEC_ID_H264;
    // codec_ctx_->delay = 0;
    codec_ctx_->codec = codec_enc_;
    codec_ctx_->codec_type = AVMEDIA_TYPE_VIDEO;
    codec_ctx_->width = width_;
    codec_ctx_->height = height_;
    codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx_->time_base = av_inv_q(dst_fps_);
    codec_ctx_->framerate = dst_fps_;
    codec_ctx_->gop_size = 12;

    if (format_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
        codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // Set codec parameters for libx264
    av_dict_set(&codec_options, "preset", "medium", 0); // You can adjust the preset as needed

    if (avcodec_open2(codec_ctx_, codec_ctx_->codec, &codec_options) < 0)
    {
        std::cerr << "Failed to open codec" << std::endl;
        exit(1);
    }

    // Create video stream
    video_stream_ = avformat_new_stream(format_ctx_, codec_enc_);
    if (!video_stream_)
    {
        std::cerr << "Failed to create video stream" << std::endl;
        exit(1);
    }
    // Set stream parameters
    avcodec_parameters_from_context(video_stream_->codecpar, codec_ctx_);
    video_stream_->time_base = {1, fps_};
    video_stream_->r_frame_rate = {fps_, 1};

    // set buffer for encoding
    format_ctx_->pb = avio_ctx_;

    // Write file header
    if (avformat_write_header(format_ctx_, &codec_options) < 0)
    {
        std::cerr << "Failed to write file header" << std::endl;
        exit(1);
    }
}
void MemoryVideoWriter ::reset(int fps, uint8_t *buffer)
{
    delete (buffer);
    avformat_free_context(format_ctx_);
    avcodec_free_context(&codec_ctx_);
    av_dict_free(&codec_options);
    // avcodec_flush_buffers(codec_ctx_);
    fps_ = fps;
    dst_fps_ = {fps, 1};
    frame_count_ = 0;
    encoded_frame_count_ = 0;

    initialize();
}

void MemoryVideoWriter ::WriteFrame(const cv::Mat &frame)
{
    AVFrame *av_frame = cvMatToAVFrame(frame);

    // Set timestamp
    av_frame->pts = frame_count_; // 예: 프레임 번호를 사용한 간단한 설정
    av_frame->pkt_dts = av_frame->pts;

    // Encode video frame
    encodeVideoFrame(av_frame);

    frame_count_++;
    // av_frame_unref(av_frame);
}

uint8_t *MemoryVideoWriter::GetMemoryBuffer()
{

    std::cout << "frame_count: " << frame_count_ << std::endl;
    std::cout << "encoded_frame_count: " << encoded_frame_count_ << std::endl;
    encodeVideoFrame(NULL);
    std::cout << "frame_count: " << frame_count_ << std::endl;
    std::cout << "encoded_frame_count: " << encoded_frame_count_ << std::endl;

    // Write file trailer
    av_write_trailer(format_ctx_);
    // 동적 버퍼 닫기
    buffer_size_ = avio_close_dyn_buf(avio_ctx_, &buffer);
    if (buffer_size_ < 0)
    {
        // 오류 처리
        exit(1);
    }
    // avformat_free_context(format_ctx_);
    return buffer;
}

int &MemoryVideoWriter::GetMemoryBufferSize() // Create output format context
{
    return buffer_size_;
}

AVFrame *MemoryVideoWriter::cvMatToAVFrame(const cv::Mat &frame)
{
    av_frame_unref(av_frame);
    std::cout << "mat" << std::endl;
    // Set codec parameters for the video stream
    video_stream_->codecpar->format = AV_PIX_FMT_YUV420P;
    video_stream_->codecpar->width = frame.cols;
    video_stream_->codecpar->height = frame.rows;

    // Set AVFrame parameters
    av_frame->width = frame.cols;
    av_frame->height = frame.rows;
    av_frame->format = AV_PIX_FMT_YUV420P;

    // Set up buffer for the frame
    av_frame_get_buffer(av_frame, 32);

    // Convert BGR to YUV (adjust parameters as needed)
    cv::Mat yuvFrame;
    cv::cvtColor(frame, yuvFrame, cv::COLOR_BGR2YUV_I420);

    // Set linesize to the width of the image (Stride)
    // av_frame->linesize[0] = yuvFrame.step[0]; // Y 플레인의 라인 크기 설정
    // av_frame->linesize[1] = yuvFrame.step[1]; // U 플레인의 라인 크기 설정
    // av_frame->linesize[2] = yuvFrame.step[2]; // V 플레인의 라인 크기 설정

    // Copy data from OpenCV Mat to AVFrame
    memcpy(av_frame->data[0], yuvFrame.data, av_frame->width * av_frame->height);
    memcpy(av_frame->data[1], yuvFrame.data + av_frame->width * av_frame->height, av_frame->width * av_frame->height / 4);
    memcpy(av_frame->data[2], yuvFrame.data + av_frame->width * av_frame->height * 5 / 4, av_frame->width * av_frame->height / 4);

    return av_frame;
}

void MemoryVideoWriter ::encodeVideoFrame(AVFrame *av_frame)
{
    int ret;

    // Send frame for encoding
    if ((ret = avcodec_send_frame(codec_ctx_, av_frame)) < 0) // 비디오 코덱에게 인코딩해달라고 보냄 (메모리 버퍼에 read write 하는게 x)
    {
        std::cerr << "Failed to send frame for encoding: " << av_err2str_custom(ret) << std::endl;
        exit(1);
    }

    // Receive encoded packet
    while (ret >= 0)
    {
        // av_packet_unref(pkt);
        // 인코딩된 패킷을 받아오는 함수로, 여러 번 호출되어 더 이상 받아올 패킷이 없을 때 AVERROR(EAGAIN)이 반환
        // AVERROR_EOF:-541478725
        // AVERROR(EAGAIN) : -11
        ret = avcodec_receive_packet(codec_ctx_, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            // Need more data, continue encoding
            break;
        }
        if (ret < 0)
        {
            std::cerr << "Error during encoding: " << av_err2str_custom(ret) << std::endl;
            exit(1);
        }
        pkt->duration = 1; // 1 = time_base(1/fps)만큼 패킷 지속
        av_packet_rescale_ts(pkt, codec_ctx_->time_base, video_stream_->time_base);
        av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");

        // Write packet to memory buffer
        // av_interleaved_write_frame(format_ctx_,pkt);
        // avio_write(avio_ctx_, pkt->data, pkt->size);
        av_write_frame(format_ctx_, pkt);

        encoded_frame_count_++;
        std::cout << "done encoding frame!!" << std::endl;
        // av_packet_unref(pkt);
    }
}

// 사용자 정의 av_err2str 함수
const char *MemoryVideoWriter ::av_err2str_custom(int errnum)
{
    static char str[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(errnum, str, AV_ERROR_MAX_STRING_SIZE);
    return str;
}
