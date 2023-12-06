#include <stdio.h>
#include <libavutil/log.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>

int main(int argc, char* argv[]){
    int ret = -1;
    int index = 0;

    // 1 处理一些参数
    char* src;
    char* dst;
    AVFormatContext *pFmtCtx=NULL;
    AVFormatContext *oFmtCtx=NULL;
    AVOutputFormat *outFmt = NULL;
    AVStream *outStream = NULL;
    AVStream *inStream = NULL;
    AVPacket pkt;
    av_log_set_level(AV_LOG_ERROR);
    if(argc<3){
        av_log(NULL, AV_LOG_INFO,"too few args\n");
        exit(-1);
    }
    src=argv[1];
    dst=argv[2];
    // 2 打开多媒体文件
    if(ret = avformat_open_input(&pFmtCtx,src,NULL,NULL)<0){
        av_log(NULL,AV_LOG_ERROR,"%s\n",av_err2str(ret));
        exit(-1);
    }
    // 3 从多媒体文件中找到音频流
    if(index = av_find_best_stream(pFmtCtx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0)<0){
        av_log(pFmtCtx,AV_LOG_ERROR,"does not include audio\n");
        goto _ERROR;
    }
    // 4 打开目的文件的上下文
    oFmtCtx = avformat_alloc_context();
    if(!oFmtCtx){
        av_log(NULL,AV_LOG_ERROR,"no memory");
        goto _ERROR;
    }
    outFmt=av_guess_format(NULL,dst,NULL);
    oFmtCtx->oformat = outFmt;
    // 5 用一个新的音频流写入目的文件
    outStream = avformat_new_stream(oFmtCtx,NULL);
    // 6 设置输出的音频参数
    inStream = pFmtCtx->streams[index];
    avcodec_parameters_copy(outStream->codecpar,inStream->codecpar);
    outStream->codecpar->codec_tag = 0;

    // 绑定
    ret = avio_open2(&oFmtCtx->pb,dst,AVIO_FLAG_WRITE,NULL,NULL);
    if(ret<0){
        av_log(oFmtCtx,AV_LOG_ERROR,"%s\n",av_err2str(ret));
        goto _ERROR;
    }
    // 7 给目的文件写文件头
    avformat_write_header(oFmtCtx,NULL);
    // 8 从源媒体读出来音频数据写入目的文件
    while(av_read_frame(pFmtCtx,&pkt)>=0){
        if(pkt.stream_index = index){
            pkt.pts = av_rescale_q_rnd(pkt.pts,inStream->time_base, outStream->time_base, (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = pkt.pts;
            pkt.duration = av_rescale_q(pkt.duration,inStream->time_base,outStream->time_base);
            pkt.stream_index = 0;
            pkt.pos = -1;
            av_interleaved_write_frame(oFmtCtx,&pkt);
            av_packet_unref(&pkt);
        }
    }
    // 9 给目的文件写文件尾
    av_write_trailer(oFmtCtx);
    // 10 释放资源
_ERROR:
    if(pFmtCtx){
        avformat_close_input(&pFmtCtx);
        pFmtCtx = NULL;
    }
    if(oFmtCtx->pb){
        avio_close(oFmtCtx->pb);
    }
    if(oFmtCtx){
        avformat_free_context(oFmtCtx);
        oFmtCtx = NULL;
    }
    return 0;
}