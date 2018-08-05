//
//  StreamVideo.c
//  
//
//  Created by Yves BAZIN on 5/08/18.
//

#include "StreamVideo.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif


#define TOP_LEFT 2
#define TOP_RIGHT 3
#define DOWN_LEFT 0
#define DOWN_RIGHT 1
void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
    FILE *pFile;
    char szFilename[32];
    int  y;
    
    // Open file
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile=fopen(szFilename, "wb");
    if(pFile==NULL)
        return;
    
    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);
    
    // Write pixel data
    for(y=0; y<height; y++)
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
    
    // Close file
    fclose(pFile);
}

int main(int argc, char *argv[]) {
    //= //{
    //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,8,8,8,9,9,9,10,10,10,11,11,11,12,12,13,13,14,14,14,15,15,16,16,17,17,18,18,19,19,20,21,21,22,22,23,23,24,25,25,26,27,27,28,29,29,30,31,31,32,33,34,34,35,36,37,37,38,39,40,41,42,42,43,44,45,46,47,48,49,50,51,52,52,53,54,55,56,57,59,60,61,62,63,64,65,66,67,68,69,71,72,73,74,75,77,78,79,80,82,83,84,85,87,88,89,91,92,93,95,96,98,99,100,102,103,105,106,108,109,111,112,114,115,117,119,120,122,123,125,127,128,130,132,133,135,137,138,140,142,144,145,147,149,151,153,155,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,197,199,201,203,205,207,210,212,214,216,219,221,223,226,228,230,233,235,237,240,242,245,247,250,252,255,
    
    
    //  };
    char  gammared[260];
    char  gamma8[260];// = {
    //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,8,8,8,9,9,9,10,10,10,11,11,11,12,12,13,13,14,14,14,15,15,16,16,17,17,18,18,19,19,20,21,21,22,22,23,23,24,25,25,26,27,27,28,29,29,30,31,31,32,33,34,34,35,36,37,37,38,39,40,41,42,42,43,44,45,46,47,48,49,50,51,52,52,53,54,55,56,57,59,60,61,62,63,64,65,66,67,68,69,71,72,73,74,75,77,78,79,80,82,83,84,85,87,88,89,91,92,93,95,96,98,99,100,102,103,105,106,108,109,111,112,114,115,117,119,120,122,123,125,127,128,130,132,133,135,137,138,140,142,144,145,147,149,151,153,155,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,197,199,201,203,205,207,210,212,214,216,219,221,223,226,228,230,233,235,237,240,242,245,247,250,252,255,
    
    // };
    
    
    double gammaCorrection=2;
    double redgammaCorrection=2.2;
    float correctGamma=1;//Math.pow(tableBrightness/255,1/gammaCorrection);
    
    correctGamma=1;
    for (int i=0;i<256;i++)
    {
        
        char newValue=(uint8_t)255*powf((float)i/255,correctGamma*gammaCorrection);
        //Serial.println(newValue);
        gamma8[i]=newValue;
        //printf("%d\n",gamma8[i]);
        newValue=(uint8_t)255*powf((float)i/255,correctGamma*redgammaCorrection);
        gammared[i]=(short)newValue;
        //Serial.println(gamma8[i]);
    }
    // Initalizing these to NULL prevents segfaults!
    AVFormatContext   *pFormatCtx = NULL;
    int               i, videoStream;
    AVCodecContext    *pCodecCtxOrig = NULL;
    AVCodecContext    *pCodecCtx = NULL;
    AVCodec           *pCodec = NULL;
    AVFrame           *pFrame = NULL;
    AVFrame           *pFrameRGB = NULL;
    AVPacket          packet;
    int               frameFinished;
    int               numBytes;
    uint8_t           *buffer = NULL;
    struct SwsContext *sws_ctx = NULL;
    
    int socket_info;
    struct sockaddr_in server;
    char message[369*2+1];
    char incoming_message[100];
    
    
    int opt = 0;
    char *input= NULL;
    char *ipaddress = NULL;
    char *dimension=NULL;
    char *in_fname= NULL;
    char *out_fname = NULL;
    char *paneldirection=NULL;
    int panel_direction=DOWN_LEFT; //DOWN_LEFT
    
    int width,height,scalew,scaleh,decalagex,decalagey,offsety,offsetx=0;
    int brightness=100;
    
    while ((opt = getopt(argc, argv, "i:d:a:s:o:w:b:")) != -1) {
        switch(opt) {
            case 'b':
                dimension=optarg;
                 sscanf(optarg, "%d", &brightness);
                brightness=brightness%256;
                break;
            case 'w':
                paneldirection=optarg;
                break;
            case 'i':
                input = optarg;
                //printf("\nInput option value=%s", in_fname);
                /* int val=0;
                 sscanf(optarg, "%d", &val);
                 printf("\nThe value of x : %d\n ", val+23);
                 char *token = strtok(optarg, "x");
                 
                 // Keep printing tokens while one of the
                 // delimiters present in str[].
                 while (token != NULL)
                 {
                 printf("%s\n", token);
                 token = strtok(NULL, "x");
                 }*/
                break;
            case 'a':
                ipaddress = optarg;
                //printf("\nOutput option value=%s", out_fname);
                break;
            case 'd':
                dimension=optarg;
                int val=0;
                char *token = strtok(optarg, "x");
                
                // Keep printing tokens while one of the
                // delimiters present in str[].
                // while (token != NULL)
                //{
                sscanf(token, "%d", &width);
                //printf("%s\n", token);
                token = strtok(NULL, "x");
                sscanf(token, "%d", &height);
                //}
                break;
            case 's':
                dimension=optarg;
                //int val=0;
                char *token2 = strtok(optarg, "x");
                
                // Keep printing tokens while one of the
                // delimiters present in str[].
                // while (token != NULL)
                //{
                sscanf(token2, "%d", &scalew);
                //printf("%s\n", token);
                token2 = strtok(NULL, "x");
                sscanf(token2, "%d", &scaleh);
                //}
                break;
            case 'o':
                dimension=optarg;
                //int val=0;
                char *token3 = strtok(optarg, "x");
                
                // Keep printing tokens while one of the
                // delimiters present in str[].
                // while (token != NULL)
                //{
                sscanf(token3, "%d", &offsetx);
                //printf("%s\n", token);
                token3 = strtok(NULL, "x");
                sscanf(token3, "%d", &offsety);
                //}
                break;
            case '?':
                /* Case when user enters the command as
                 * $ ./cmd_exe -i
                 */
                if (optopt == 'i') {
                    printf("Missing mandatory input stream name -i inputfile\n");
                    /* Case when user enters the command as
                     * # ./cmd_exe -o
                     */
                } else if (optopt == 'd') {
                    printf("Missing mandatory input led panel size -d widthxheight\n");
                } else if (optopt =='a'){
                    printf("Missing mandatory input led streaminf address -a xx.xx.xx.xx\n");
                }
                else {
                    printf("\nInvalid option received");
                }
                break;
        }
    }
    
    if(input==NULL)
    {
        printf("Missing mandatory input stream name -i inputfile\n");
        return 0;
    }
    if(ipaddress==NULL)
    {
        printf("Missing mandatory input led streaminf address -a xx.xx.xx.xx\n");
        return 0;
    }
    if(width==0 | height==0)
    {
        printf("Missing mandatory input led panel size -d widthxheight\n");
        return 0;
    }
/*
 #define TOP_LEFT 2
 #define TOP_RIGHT 3
 #define DOWN_LEFT 0
 #define DOWN_RIGHT 1
 */
    if(paneldirection!=NULL)
    {

   if( strcmp(paneldirection,"TOP_LEFT")==0  )
       panel_direction=TOP_LEFT;

    if( strcmp(paneldirection,"TOP_RIGHT")==0  )
        panel_direction=TOP_RIGHT;
    if( strcmp(paneldirection,"DOWN_RIGHT")==0  )
        panel_direction=DOWN_RIGHT;
    if( strcmp(paneldirection,"DOWN_LEFT")==0  )
        panel_direction=DOWN_LEFT;
    }
    
    
    if(scaleh==0)
        scaleh=height;
    if(scalew==0)
        scalew=width;
    
    printf("input  :%s \naddress:%s \nwidth  :%d \nheight :%d \nscalex :%d \nscaley :%d \noffsetx:%d\noffsety:%d\nPannel direction:%d\nBrightness:%d\n",input,ipaddress,width,height,scalew,scaleh,offsetx,offsety,panel_direction,brightness);
   
    
    clock_t temps;
    socket_info = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_info == -1) {
        printf("Could not create socket");
    }
    
    //assign local values
    server.sin_addr.s_addr = inet_addr(ipaddress);
    server.sin_family = AF_INET;
    server.sin_port = htons( 100 );
    
    
    if (connect(socket_info, (struct sockaddr *)&server, sizeof(server)) <       0) {
        perror("Connection error");
        return 1;
    }
    puts("Connected");
    
    
   /*if(argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }*/
    // Register all formats and codecs
    av_register_all();
    
    // Open video file
    if(avformat_open_input(&pFormatCtx, input, NULL, NULL)!=0)
        return -1; // Couldn't open file
    
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
        return -1; // Couldn't find stream information
    
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0,input, 0);
    
    
    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            videoStream=i;
            break;
        }
    if(videoStream==-1)
        return -1; // Didn't find a video stream
    
    // Get a pointer to the codec context for the video stream
    pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;
    // Find the decoder for the video stream
    int numrate=pFormatCtx->streams[videoStream]->avg_frame_rate.num;
    int denumreate=pFormatCtx->streams[videoStream]->avg_frame_rate.den;
    printf("%f\n",(float)((float)numrate/(float)denumreate));
    //return 0;
    pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
    if(pCodec==NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    // Copy context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
        fprintf(stderr, "Couldn't copy codec context");
        return -1; // Error copying codec context
    }
    
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
        return -1; // Could not open codec
    
    // Allocate video frame
    pFrame=av_frame_alloc();
    
    // Allocate an AVFrame structure
    pFrameRGB=av_frame_alloc();
    if(pFrameRGB==NULL)
        return -1;
    
    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
                                pCodecCtx->height);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    
    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24,
                   pCodecCtx->width, pCodecCtx->height);
    
    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecCtx->width,
                             pCodecCtx->height,
                             pCodecCtx->pix_fmt,
                             scalew,//pCodecCtx->width,
                             scaleh,//pCodecCtx->height,
                             AV_PIX_FMT_RGB24,
                             SWS_BILINEAR,
                             NULL,
                             NULL,
                             NULL
                             );
    
    // Read frames and save first five frames to disk
    i=0;
    temps=clock();
    
    message[0]=255;
    message[1]=brightness;
    int off=0;
    if(send(socket_info, message, 2, 0) <0) {
        perror("Send failed");
        return 1;
    }
    while(av_read_frame(pFormatCtx, &packet)>=0) {
        
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream) {
            
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
            
            // Did we get a video frame?
            if(frameFinished) {
                
                // Convert the image from its native format to RGB
                sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGB->data, pFrameRGB->linesize);
                
                // Save the frame to disk
                /* if(++i<=200)
                 SaveFrame(pFrameRGB, 123,48,//pCodecCtx->width, pCodecCtx->height,
                 i);*/
                int m=1000000*(double)denumreate*9/(double)numrate/10-(double)(((clock()-temps)*1000000)/CLOCKS_PER_SEC);
                // printf("%d\n",m);
                if(m>0)
                    usleep(m)  ;
                temps=clock();
                int nbpacket=height/2;
                for(uint16_t s=0;s<nbpacket;s++)
                {
                    char b;
                    message[0]=s;
                    switch(panel_direction)
                    {
                        case DOWN_LEFT:
                            for(uint16_t g=0;g<width;g++)
                            {
                                
                                
                                // printf("value:%d\n",pFrame->data[0]+2*123*3*s);
                                //printf("address:%d\n",&(pFrame->data[0]));
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g,1);
                                
                                message[g*3+1]=gammared[(uint8_t)b];
                                
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g+1,1);
                                message[g*3+1+1]=gamma8[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g+2,1);
                                message[g*3+2+1]=gamma8[(uint8_t)b];
                                
                            }
                            //memcpy(&message[1],pFrame->data[0]+s*pFrame->linesize[0],123*3);
                            for(int g=0;g<width;g++)
                            {
                                
                                
                                // printf("value:%d\n",pFrame->data[0]+2*123*3*s);
                                //printf("address:%ld\n",&(pFrame->data[0]));
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+offsety)*pFrameRGB->linesize[0]+3*g,1);
                                message[(width*2-g-1)*3+1]=gammared[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+offsety)*pFrameRGB->linesize[0]+3*g+1,1);
                                message[(width*2-g-1)*3+1+1]=gamma8[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+offsety)*pFrameRGB->linesize[0]+3*g+2,1);
                                message[(width*2-g-1)*3+2+1]=gamma8[(uint8_t)b];
                                
                            }
                    
                            break;
                        case TOP_RIGHT:
                            for(uint16_t g=0;g<width;g++)
                            {
                                
                                
                                // printf("value:%d\n",pFrame->data[0]+2*123*3*s);
                                //printf("address:%d\n",&(pFrame->data[0]));
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+offsety)*pFrameRGB->linesize[0]+3*g,1);
                                
                                message[g*3+1]=gammared[(uint8_t)b];
                                
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+offsety)*pFrameRGB->linesize[0]+3*g+1,1);
                                message[g*3+1+1]=gamma8[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+offsety)*pFrameRGB->linesize[0]+3*g+2,1);
                                message[g*3+2+1]=gamma8[(uint8_t)b];
                                
                            }
                            //memcpy(&message[1],pFrame->data[0]+s*pFrame->linesize[0],123*3);
                            for(int g=0;g<width;g++)
                            {
                                
                                
                                // printf("value:%d\n",pFrame->data[0]+2*123*3*s);
                                //printf("address:%ld\n",&(pFrame->data[0]));
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g,1);
                                message[(width*2-g-1)*3+1]=gammared[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g+1,1);
                                message[(width*2-g-1)*3+1+1]=gamma8[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g+2,1);
                                message[(width*2-g-1)*3+2+1]=gamma8[(uint8_t)b];
                                
                            }
                            break;
                        case TOP_LEFT:
                            for(uint16_t g=0;g<width;g++)
                            {
                                
                                
                                // printf("value:%d\n",pFrame->data[0]+2*123*3*s);
                                //printf("address:%d\n",&(pFrame->data[0]));
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+offsety)*pFrameRGB->linesize[0]+3*g,1);
                                
                                message[(width-g-1)*3+1]=gammared[(uint8_t)b];
                                
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+offsety)*pFrameRGB->linesize[0]+3*g+1,1);
                                message[(width-g-1)*3+1+1]=gamma8[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+offsety)*pFrameRGB->linesize[0]+3*g+2,1);
                                message[(width-g-1)*3+2+1]=gamma8[(uint8_t)b];
                                
                            }
                            //memcpy(&message[1],pFrame->data[0]+s*pFrame->linesize[0],123*3);
                            for(int g=0;g<width;g++)
                            {
                                
                                
                                // printf("value:%d\n",pFrame->data[0]+2*123*3*s);
                                //printf("address:%ld\n",&(pFrame->data[0]));
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g,1);
                                message[(width+g)*3+1]=gammared[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g+1,1);
                                message[(width+g)*3+1+1]=gamma8[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g+2,1);
                                message[(width+g)*3+2+1]=gamma8[(uint8_t)b];
                                
                            }
                            
                            break;
                        case DOWN_RIGHT:
                            for(uint16_t g=0;g<width;g++)
                            {
                                
                                
                                // printf("value:%d\n",pFrame->data[0]+2*123*3*s);
                                //printf("address:%d\n",&(pFrame->data[0]));
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g,1);
                                
                                message[(width-g-1)*3+1]=gammared[(uint8_t)b];
                                
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g+1,1);
                                message[(width-g-1)*3+1+1]=gamma8[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+1+offsety)*pFrameRGB->linesize[0]+3*g+2,1);
                                message[(width-g-1)*3+2+1]=gamma8[(uint8_t)b];
                                
                            }
                            //memcpy(&message[1],pFrame->data[0]+s*pFrame->linesize[0],123*3);
                            for(int g=0;g<width;g++)
                            {
                                
                                
                                // printf("value:%d\n",pFrame->data[0]+2*123*3*s);
                                //printf("address:%ld\n",&(pFrame->data[0]));
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+offsety)*pFrameRGB->linesize[0]+3*g,1);
                                message[(width+g)*3+1]=gammared[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+offsety)*pFrameRGB->linesize[0]+3*g+1,1);
                                message[(width+g)*3+1+1]=gamma8[(uint8_t)b];
                                
                                memcpy(&b,pFrameRGB->data[0]+offsetx*3+((nbpacket-1-s)*2+offsety)*pFrameRGB->linesize[0]+3*g+2,1);
                                message[(width+g)*3+2+1]=gamma8[(uint8_t)b];
                                
                            }
                            
                            break;
                    }
                    
                    if(send(socket_info, message, width*3*2+1, 0) <0) {
                        perror("Send failed");
                        // return 1;
                    }
                    //usleep(600);
                }
                
            }
            
            
            
        }
        
        av_free_packet(&packet);
        /* for (int skip=0;i<20;i++)
         {
         if(av_read_frame(pFormatCtx, &packet)<0)
         return 0;
         av_free_packet(&packet);
         }*/
        // Free the packet that was allocated by av_read_frame
        
    }
    
    close(socket_info);
    // Free the RGB image
    av_free(buffer);
    av_frame_free(&pFrameRGB);
    
    // Free the YUV frame
    av_frame_free(&pFrame);
    
    // Close the codecs
    avcodec_close(pCodecCtx);
    avcodec_close(pCodecCtxOrig);
    
    // Close the video file
    avformat_close_input(&pFormatCtx);
    
    return 0;
}
