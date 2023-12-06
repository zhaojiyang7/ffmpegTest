#include <stdio.h>
#include <libavformat/avio.h>
#include <libavutil/log.h>

int main(int argc, char* argv[])
{
    int ret;
    AVIODirContext *ctx = NULL;
    AVIODirEntry *entry = NULL;
    ret = avio_open_dir(&ctx,"./",NULL);
    if(ret<0){
        av_log(NULL,AV_LOG_ERROR,"avio_open_dir wrong");
        return -1;
    }
    while(1){
        ret = avio_read_dir(ctx,&entry);
        if(ret<0){
            av_log(NULL,AV_LOG_ERROR,"avio_read_dir wrong");
            avio_close_dir(&ctx);
            return -1;
        }
        if(!entry){
            break;
        }
        av_log(NULL,AV_LOG_INFO,"entry name is %s entry size is %ld \n",entry->name, entry->size);
        avio_free_directory_entry(&entry);
    }
    avio_close_dir(&ctx);
    return 0;
}