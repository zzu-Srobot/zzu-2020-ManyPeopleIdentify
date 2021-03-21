#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"

int makeTextToWav(const char* src_text, const char* params)
{
    int          ret          = -1;
    int          rc;
    const char*  sessionID    = NULL;
    unsigned int audio_len    = 0;
    unsigned int speech_len = 0;
    char* synth_speech = new char[2 * 1024 * 1024];
    int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

    sessionID = QTTSSessionBegin(params, &ret);
    QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
    
    unsigned int val;
    int dir=0;
    int size;
    snd_pcm_uframes_t frames;
    snd_pcm_uframes_t periodsize;
    snd_pcm_t *playback_handle;//PCM设备句柄pcm.h
    snd_pcm_hw_params_t *hw_params;//硬件信息和PCM流配置

    rc = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);//1. 打开PCM，最后一个参数为0意味着标准配置
    if (rc < 0)  
    {  
        std::cout<<"something woring!"<<std::endl;
    }  
    rc = snd_pcm_hw_params_malloc(&hw_params);//2. 分配snd_pcm_hw_params_t结构体
    rc = snd_pcm_hw_params_any(playback_handle, hw_params);//3. 初始化hw_params
    rc = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);//4. 初始化访问权限
    rc = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE); //5. 初始化采样格式SND_PCM_FORMAT_U8,8位

    val = 16000;//6. 设置采样率，如果硬件不支持我们设置的采样率，将使用最接近的 val = 44100,有些录音采样频率固定为8KHz
    rc = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &val, &dir);
    rc = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 1);//7. 设置通道数量  1
    frames = 32; /* Set period size to 32 frames. */
    periodsize = frames ;
    rc = snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &periodsize);
    periodsize /= 2;
    rc = snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &periodsize, 0);
    rc = snd_pcm_hw_params(playback_handle, hw_params); //8. 设置hw_params
    snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);/* Use a buffer large enough to hold one period */
    size = frames * 2; 


    while (1)
    {
        const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
        
        if (ret) break;
            if (NULL != data)
            {
                memcpy(synth_speech + speech_len, data, audio_len);
			    speech_len += audio_len;
            }
            if (MSP_TTS_FLAG_DATA_END == synth_status)break;
            usleep(1000); 
    }
    QTTSSessionEnd(sessionID, "Normal");
       
       
        char *buffer;
        buffer = (char *) malloc(size);
        int length_all=0;
while (1)
    {
        memcpy(buffer,synth_speech,size);
        synth_speech+=size;
        usleep(200);

        length_all+=size;
        if (length_all>speech_len){ break;}
        rc = snd_pcm_writei(playback_handle, buffer, frames);
        if (rc == -EPIPE) {fprintf(stderr, "underrun occurred\n");snd_pcm_prepare(playback_handle);} 
        else if (rc < 0) {fprintf(stderr,"error from writei: %s\n",snd_strerror(rc));}  
        else if (rc != (int)frames) {fprintf(stderr,"short write, write %d frames\n", rc);}
    }
    snd_pcm_drain(playback_handle);
    snd_pcm_close(playback_handle);//10. 关闭PCM设备句柄

    free(buffer);
    return 0;
}

int tts(const char* text)
{
    const char* session_begin_params = "voice_name = catherine, text_encoding = utf8, sample_rate = 16000, speed = 50, volume = 100, pitch = 50, rdn = 0";//catherine xiaoyan john
    makeTextToWav(text,session_begin_params);
    return 0;
}
