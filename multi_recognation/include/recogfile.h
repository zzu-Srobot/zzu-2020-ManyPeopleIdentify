#include <alsa/asoundlib.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include <sys/time.h> 



std::string recog() 
{
    int				aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;		
	int				ep_stat = MSP_EP_LOOKING_FOR_SPEECH;		
	int				rec_stat = MSP_REC_STATUS_SUCCESS;			
	int				errcode = MSP_SUCCESS;
    std::string     resultresult = "";
    char			hints[100]={}; 
    const char* session_id = QISRSessionBegin(NULL, "sub = iat, domain = iat, language = en_us, accent = mandarin, sample_rate = 8000, result_type = plain, result_encoding = utf8, vad_eos=2000", &errcode);// zh_cn en_us
    long loops;
    int rc;
    int size;
    int dir;
    int pcm_count=0;
    int total_len = 0;
    unsigned int val;
    snd_pcm_t *rec_handle;
    snd_pcm_hw_params_t *rec_params;
    snd_pcm_uframes_t frames;
    char *buffer=NULL;
    //WAVEHDR wavheader;
    //FILE *fp_rec = fopen("rec.wav", "wb");
    //if(fp_rec==NULL) return 0;

    //wav_start_write(fp_rec, &wavheader);
    rc = snd_pcm_open(&rec_handle, "default",SND_PCM_STREAM_CAPTURE, 0);/* Open PCM device for recording (capture). */
    
    if (rc < 0) {
        std::cout<<"Unable to open pcm device!!!"<<std::endl;
        exit(1);}

    snd_pcm_hw_params_alloca(&rec_params);          /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_any(rec_handle, rec_params);          /* Fill it in with default values. */
    snd_pcm_hw_params_set_access(rec_handle, rec_params, SND_PCM_ACCESS_RW_INTERLEAVED);        /* Set the desired hardware parameters. Interleaved mode */
    snd_pcm_hw_params_set_format(rec_handle, rec_params,SND_PCM_FORMAT_S16_LE);      /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_channels(rec_handle, rec_params, 1);       /* Two channels (stereo) */
    val = 8000;         /* 44100 bits/second sampling rate (CD quality) */
    snd_pcm_hw_params_set_rate_near(rec_handle, rec_params,&val, &dir);
    frames = 32;            /* Set period size to 32 frames. */
    snd_pcm_hw_params_set_period_size_near(rec_handle,rec_params, &frames, &dir);
    rc = snd_pcm_hw_params(rec_handle, rec_params);             /* Write the parameters to the driver */
    if (rc < 0) 
    {
        std::cout<<"unable to set hw parameters!!!"<<std::endl;
        exit(1);
    }
    snd_pcm_hw_params_get_period_size(rec_params,  &frames, &dir);              /* Use a buffer large enough to hold one period */
    size = frames * 2;          /* 2 bytes/sample, 2 channels */
    snd_pcm_hw_params_get_period_time(rec_params, &val, &dir);
    loops = 50000;               /* We want to loop for 5 seconds */
    while (loops-- > 0) 
    {
        buffer = (char *) malloc(size);
        rc = snd_pcm_readi(rec_handle, buffer, frames); 
        if (rc == -EPIPE) 
            {
                std::cout<<"Overrun occurred!!!"<<std::endl; /* EPIPE means overrun */
                snd_pcm_prepare(rec_handle);
            } 
            else if (rc < 0) 
            {
                std::cout<<"Error from read!!!"<<std::endl; /* EPIPE means overrun */
            } 
            else if (rc != (int)frames) 
            {
                std::cout<<"short read, read "<<rc<<"frames"<<std::endl; /* EPIPE means overrun */
            }

            if (0 == pcm_count) 
                  aud_stat = MSP_AUDIO_SAMPLE_FIRST;
            else   
                  aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
                  
		    errcode = QISRAudioWrite(session_id, buffer, size, aud_stat, &ep_stat, &rec_stat);
            if (errcode != MSP_SUCCESS)
            {
                exit(0);
            }
            if (MSP_EP_AFTER_SPEECH == ep_stat)
            {
                break;
            }
            //rc = fwrite(buffer, 1, size, fp_rec);
            //total_len += size;
            //if (rc != size)
               // std::cout<<"short write, write "<<rc<<"bytes"<<std::endl; /* EPIPE means overrun */
    }
    
    //wav_stop_write(fp_rec, &wavheader, total_len);
    snd_pcm_drain(rec_handle);
    snd_pcm_close(rec_handle);
    free(buffer);
    //fclose(fp_rec);

    std::string  str;

    long timeuse;
    timeval start,end;
    gettimeofday(&start, NULL);
    while (MSP_REC_STATUS_COMPLETE != rec_stat&& MSP_SUCCESS == errcode)
	{
		const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
        gettimeofday(&end, NULL);
        timeuse = 1000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
        if(timeuse>60000) break;
		if (NULL != rslt)
		{
			str = rslt;
			resultresult += str;
		}
		usleep(150);
	}
    QISRSessionEnd(session_id, hints);
    std::cout<<"O_o:  "<<"\033[31;4m"<<resultresult<<"\033[0m"<<std::endl;
    
    return resultresult;
}
