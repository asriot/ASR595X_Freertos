/*
 * Copyright (c) 2022 ASR Microelectronics (Shanghai) Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lwip_app_iperf.h"
#ifdef LWIP_APP_IPERF
#include <unistd.h>
#include <lwip/inet.h>
#include <string.h>
#include <stdlib.h>
#include "iperf_printf.h"
#include "lega_rtos_api.h"
#ifdef CFG_PLF_RV32
#include "alto_common.h"
#else
#include "lega_common.h"
#endif

#define IPERF_PORT 5001
#define IPERF_CLIENT_TIMEOUT_MS 100

lega_semaphore_t iperf_Semaphore=NULL;
volatile IPERF_TCP_SERVER_STATUS iperf_tcp_server_status;
volatile IPERF_UDP_SERVER_STATUS iperf_udp_server_status;
volatile IPERF_TCP_CLINET_STATUS iperf_tcp_client_status;
volatile IPERF_UDP_CLIENT_STATUS iperf_udp_client_status;
//volatile int iperf_tcp_server_running;
//volatile int iperf_udp_server_running;
//volatile int iperf_tcp_client_running;
//volatile int iperf_udp_client_running;
char iperf_send_buf[SEND_BUFSIZE];
char *iperf_pbuf = NULL;
struct lwip_iperf_config_t txiperf_config;
struct lwip_iperf_config_t rxiperf_config;
lega_timer_t iperf_output_timer;
struct lwip_iperf_outputInfo txperf_outinfo;
struct lwip_iperf_outputInfo rxperf_outinfo;
struct lwip_iperf_outputInfo utxperf_outinfo;
struct lwip_iperf_outputInfo urxperf_outinfo;

void iperf_value_format(char* format_value,int format_value_len,u64_t value)
{
    u64_t gv=value/(1000000000ULL);
    u64_t mv=value/(1000000ULL);
    u64_t kv=value/(1000ULL);
    if(value<0) return;
#ifndef IPERF_SOCKET
    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
#endif
    if(gv==0){
        if(mv==0){
            if(kv==0){
                snprintf(format_value,format_value_len,"%llu",value);
            }
            else{
                snprintf(format_value,format_value_len,"%llu.%02uK",kv,(unsigned int)(value%1000)/10);
            }
        }
        else{
            snprintf(format_value,format_value_len,"%llu.%02uM",mv,(unsigned int)(value%1000000)/10000);
        }
    }
    else{
        snprintf(format_value,format_value_len,"%llu.%02uG",gv,(unsigned int)(value%1000000000)/10000000);
    }
#ifndef IPERF_SOCKET
    lega_rtos_exit_critical();
#endif
}

void do_iperf_terminate_timer(char* mode,struct lwip_iperf_outputInfo* outputInfo)
{
    u64_t bytes=outputInfo->currentByte;
    u64_t packets=outputInfo->currentPacketNum;
    char byteFormat[24]={0};
    char bitFormat[24]={0};
    lega_rtos_stop_timer(&iperf_output_timer);
    iperf_value_format(byteFormat,sizeof(byteFormat),bytes);
    iperf_value_format(bitFormat,sizeof(bitFormat),(bytes*(8ULL))/((u64_t)outputInfo->seconds));
    iperf_printf("%s:total %d sec    %llu packets    %s Bytes    %s bits/sec\r\n",mode,outputInfo->seconds,packets,byteFormat,bitFormat);
    memset(outputInfo,0,sizeof(struct lwip_iperf_outputInfo));
}
void do_iperf_terminate(char* mode,struct lwip_iperf_outputInfo* outputInfo,discon_handle_t discon)
{
    do_iperf_terminate_timer(mode,outputInfo);
    discon();
}

static void lega_wifi_iperf_usage()
{
    iperf_printf("usage: iperf -c host [-p port] [-t times] [-u] [-b bandwith] [-a]\r\n\
iperf -s [-p port] [-u] [-a]\r\n\
-c host     :run as iperf client connect to host\r\n\
-s          :run as iperf server\r\n\
-S tos      :TOS value(one of below): BE BK VI VO\r\n\
-B host     :multicast host\r\n\
-p port     :client connect to/server port default 5001\r\n\
-u          :use udp do iperf client/server\r\n\
             If -u not enable, use tcp default\r\n\
-b          :set transmission bandwith in Kbits/sec, udp client only\r\n\
-t          :set transmission timeout in seconds, client only\r\n\
-a          :abort iperf client/server/all\r\n\
-d          :tx packets delay time(ms) default 10\r\n");
}
void lega_wifi_iperf(int argc, char **argv)
{
    int c=0;
    struct lwip_iperf_config_t temp_config = {0};
    char tmp_tos[5] = {0};
#ifdef IPERF_MULTICAST
    ip_addr_t multiAddrMin, multiAddrMax;
    temp_config.multicastFlag = 0;
#endif

    temp_config.port=IPERF_PORT;
    temp_config.tx_delay_ms = 10;
#if 0
//something wrong with getopt
    while((c=getopt(argc,argv,"c:p:sut"))!=-1){
        switch(c){
            case 'c':
                iperf_config.mode=IPERF_MODE_CLIENT;
                if(inet_aton(optarg,&(iperf_config.ipaddr))==0){
                   goto IPERF_PARSE_FAIL;
                }
                break;
            case 'p':
                iperf_config.port=strtoul(optarg,NULL,10);
                if(iperf_config.port<=0 || iperf_config.port>65536)
                    goto IPERF_PARSE_FAIL;
                break;
            case 's':
                iperf_config.mode=IPERF_MODE_SERVER;
                break;
            case 'u':
                iperf_config.protocol=IPERF_PROTOCOL_UDP;
                break;
            case 't':
                iperf_config.termFlag=1;
                break;
            case '?':
            default:
               goto IPERF_PARSE_FAIL;
        }
    }
#else
    for(c=1;c<argc;c++){
        if(strcmp(argv[c],"-c")==0){
            temp_config.mode=IPERF_MODE_CLIENT;
            temp_config.clientFlag = 1;
            if(inet_aton(argv[++c],&(temp_config.ipaddr))==0){
               goto IPERF_PARSE_FAIL;
            }
        }else
#ifdef IPERF_MULTICAST
        if(strcmp(argv[c],"-B")==0){
            temp_config.multicastFlag=1;
            if(inet_aton(argv[++c],&(temp_config.ipaddr))==0){
               goto IPERF_PARSE_FAIL;
            }
        }else
#endif
        if(strcmp(argv[c],"-p")==0){
            temp_config.port=strtoul(argv[++c],NULL,10);
            if(temp_config.port<=0 || temp_config.port>65536)
                goto IPERF_PARSE_FAIL;
        }else
        if(strcmp(argv[c],"-mu")==0){
            temp_config.multi_udp=1;
        }else
        if(strcmp(argv[c],"-S")==0){
            strcpy(tmp_tos, argv[++c]);
            if(!strcmp(tmp_tos, "BE"))
                temp_config.tos=0x00;
            else if(!strcmp(tmp_tos, "BK"))
                temp_config.tos=0x20;
            else if(!strcmp(tmp_tos, "VI"))
                temp_config.tos=0xA0;
            else if(!strcmp(tmp_tos, "VO"))
                temp_config.tos=0xE0;
            else
                goto IPERF_PARSE_FAIL;
            iperf_printf("tos = %x\n", temp_config.tos);
        }else
        if(strcmp(argv[c],"-s")==0){
            temp_config.mode=IPERF_MODE_SERVER;
            temp_config.serverFlag = 1;
        }else
        if(strcmp(argv[c],"-u")==0){
            temp_config.protocol=IPERF_PROTOCOL_UDP;
        }else
        if(strcmp(argv[c],"-a")==0){
            temp_config.termFlag=1;
        }else
        if(strcmp(argv[c],"-d")==0){
            temp_config.tx_delay_ms=atoi(argv[c+1]);
            c++;
        }else
        if(strcmp(argv[c],"-t")==0){
            temp_config.timeout=atoi(argv[c+1]);
            c++;
        }else
        if(strcmp(argv[c],"-b")==0){
            temp_config.bandwith=atoi(argv[c+1]);
            c++;
            if(temp_config.bandwith <= 0 || temp_config.bandwith > 65536)
               goto IPERF_PARSE_FAIL;
        }else
        goto IPERF_PARSE_FAIL;
    }

    if((temp_config.protocol == IPERF_PROTOCOL_UDP) && (temp_config.bandwith != 0))
    {
        if( temp_config.bandwith <= 6000) //6 Mbit/s
        {
            if( temp_config.bandwith <= 12) //12 Kbit/s
                temp_config.tx_delay_ms = 1000;
            else
            {
                temp_config.tx_delay_ms = (UDP_SEND_BUFSIZE*8) / (temp_config.bandwith);
                temp_config.delay_offset = (temp_config.bandwith*1000) / (UDP_SEND_BUFSIZE*8) + 1;// total pkg
                temp_config.delay_offset = 1000 - temp_config.tx_delay_ms * temp_config.delay_offset;//offset ms
            }
        }
    }

    if(temp_config.clientFlag ==0 && temp_config.serverFlag==0){
        iperf_printf("please set -c or -s !\n");
        goto IPERF_PARSE_FAIL;
    }
    #ifdef IPERF_MULTICAST
    if(temp_config.multicastFlag==0){
        IP4_ADDR(&multiAddrMin, 224,0,0,0);
        IP4_ADDR(&multiAddrMax, 239,255,255,255);
        if(memcmp(&(temp_config.ipaddr), &multiAddrMin, sizeof(ip_addr_t)) >= 0)
            if(memcmp(&(temp_config.ipaddr), &multiAddrMax, sizeof(ip_addr_t)) <= 0)
                temp_config.multicastFlag=1;
    }
    #endif
    if(temp_config.clientFlag){
        if(temp_config.termFlag==0 &&txiperf_config.termFlag==0){
            iperf_printf("client already run, please close first!\n");
            goto IPERF_PARSE_FAIL;
        }else
        if(temp_config.termFlag==1 &&txiperf_config.termFlag==0){
            //close()
        }else
        if(temp_config.termFlag==0 &&txiperf_config.termFlag==1){
            //open
        }else{
            iperf_printf("client already close!\n");
            goto IPERF_PARSE_FAIL;
        }
        memcpy(&txiperf_config, &temp_config, sizeof(temp_config));
        if((txiperf_config.mode==IPERF_MODE_CLIENT) && (iperf_Semaphore!=NULL)){
            if(lega_rtos_set_semaphore(&iperf_Semaphore)==kNoErr){
                //iperf_printf("Begin to run iperf client:\n");
            }else{
                iperf_printf("Run iperf client fail for semaphore error\n");
            }
            return;
        }
    }else
    if(temp_config.serverFlag){
         if(temp_config.termFlag==0 &&rxiperf_config.termFlag==0){
            iperf_printf("client already run, please close first!\n");
            goto IPERF_PARSE_FAIL;
        }else
        if(temp_config.termFlag==1 &&rxiperf_config.termFlag==0){
            //close()
        }else
        if(temp_config.termFlag==0 &&rxiperf_config.termFlag==1){
            //open
        }else{
            iperf_printf("client already close!\n");
            goto IPERF_PARSE_FAIL;
        }
        memcpy(&rxiperf_config, &temp_config, sizeof(temp_config));

        if(iperf_pbuf == NULL && (temp_config.termFlag != 1))
        {
            iperf_pbuf = lega_rtos_malloc(SEND_BUFSIZE);
            memset(iperf_pbuf, 0, SEND_BUFSIZE);
            for (int i = 0; i < SEND_BUFSIZE; i++)
            iperf_pbuf[i] = (i % 10) + '0';
        }

        if(rxiperf_config.protocol==IPERF_PROTOCOL_TCP){
            //tcp server
            if(rxiperf_config.termFlag==1){
                //iperf_printf("Terminate iperf tcp server");
                //terminate_rxperf();
                if(iperf_pbuf != NULL)
                {
                    lega_rtos_free(iperf_pbuf);
                    iperf_pbuf = NULL;
                }
                do_iperf_terminate("tcp server",&rxperf_outinfo,clear_rxperf);
            }else{
                if(iperf_tcp_server_status<=IPERF_TCP_SERVER_INIT){
                    rxperf_init();
                    start_rxperf_application(rxiperf_config.port);
                }else
                    iperf_printf("iperf tcp server already running status:%d,please terminate it before run again!\r\n",iperf_tcp_server_status);
            }

        }
        else{
            //udp server
            if(rxiperf_config.termFlag==1){
                if(iperf_pbuf != NULL)
                {
                    lega_rtos_free(iperf_pbuf);
                    iperf_pbuf = NULL;
                }
                do_iperf_terminate("udp server",&urxperf_outinfo,clear_urxperf);
            }else{
                if(iperf_udp_server_status<=IPERF_UDP_SERVER_INIT){
                    iperf_udp_server_status=IPERF_UDP_SERVER_INIT;
                    urxperf_init();
                    start_urxperf_application(rxiperf_config.port);
                }else
                    iperf_printf("iperf udp server already running status:%d,please terminate it before run again!\r\n",iperf_udp_server_status);
            }
        }

    }
#if 0
    if((temp_config.termFlag == 0)&&(iperf_config.termFlag == 0)){
        if((temp_config.clientFlag == 1 && iperf_config.clientFlag == 1 ))
        {
            iperf_printf("client already run, please close first!\n");
            goto IPERF_PARSE_FAIL;
        }
        if((temp_config.serverFlag == 1 && iperf_config.serverFlag == 1 ))
        {
            iperf_printf("server already run, please close first!\n");
            goto IPERF_PARSE_FAIL;
        }
        memcpy(&iperf_config, &temp_config, sizeof(iperf_config));
    }else if((temp_config.termFlag == 1)&&(iperf_config.termFlag == 0)){
        iperf_config.termFlag=1;
    }else{
        memcpy(&iperf_config, &temp_config, sizeof(iperf_config));
    }

    if((iperf_config.mode==IPERF_MODE_UNINIT)&&(iperf_config.termFlag!=1)) goto IPERF_PARSE_FAIL;
#endif
#endif

    return;
IPERF_PARSE_FAIL:
    lega_wifi_iperf_usage();
    return;
}

void do_iperf_output(char* mode,struct lwip_iperf_outputInfo* outputInfo)
{
    u64_t bytes=outputInfo->currentByte-outputInfo->lastByte;
    u64_t packets=outputInfo->currentPacketNum-outputInfo->lastPacketNum;
    char byteFormat[24]={0};
    char bitFormat[24]={0};
    iperf_value_format(byteFormat,sizeof(byteFormat),bytes);
    iperf_value_format(bitFormat,sizeof(bitFormat),bytes*(8ULL));
    outputInfo->seconds++;
    if(bytes!=0)
        iperf_printf("%s:%d-%d sec    %llu packets    %s Bytes    %s bits/sec\r\n",mode,outputInfo->seconds-1,outputInfo->seconds,packets,byteFormat,bitFormat);
    outputInfo->lastByte=outputInfo->currentByte;
    outputInfo->lastPacketNum=outputInfo->currentPacketNum;
    //client timeout
    if((txiperf_config.timeout != 0)&&(txiperf_config.mode == IPERF_MODE_CLIENT))
    {
        if(outputInfo->seconds >= txiperf_config.timeout)
        {
            txiperf_config.termFlag = 1;

            if(iperf_Semaphore!=NULL){
                if(lega_rtos_set_semaphore(&iperf_Semaphore)!=kNoErr)
                    iperf_printf("Stop iperf client fail for semaphore error\n");
            }
        }
    }
}


void iperf_output(void *args)
{
    if(iperf_tcp_client_status==IPERF_TCP_CLIENT_TXRUNNING)    do_iperf_output("tcp client",&txperf_outinfo);
    if(iperf_tcp_server_status==IPERF_TCP_SERVER_RXRUNNING)    do_iperf_output("tcp server",&rxperf_outinfo);
    if(iperf_udp_client_status==IPERF_UDP_CLIENT_TXRUNNING)    do_iperf_output("udp client",&utxperf_outinfo);
    if(iperf_udp_server_status==IPERF_UDP_SERVER_RXRUNNING)    do_iperf_output("udp server",&urxperf_outinfo);

}
void lega_wifi_iperf_client_start()
{
    int count=0;
    uint32_t last_tick, current_tick, send_tick, current_sleep;
    if(lega_rtos_init_timer(&iperf_output_timer,IPERF_OUTPUT_INTERVIEW*1000,iperf_output,NULL)!=kNoErr)
        iperf_printf("iperf timer fail!\r\n");
    for( ;; ){
        if (lega_rtos_get_semaphore(&iperf_Semaphore, LEGA_NEVER_TIMEOUT)==kNoErr){
            //AT command set lwip_iperf_config over and begin to run

            if(txiperf_config.mode!=IPERF_MODE_CLIENT) continue;
            if(txiperf_config.protocol==IPERF_PROTOCOL_TCP){
                if(txiperf_config.termFlag==1){
#ifdef IPERF_SOCKET
                  if(iperf_tcp_client_status!=IPERF_TCP_CLINET_UNINIT) //avoid repetition in terminate semaphore
#endif
                    do_iperf_terminate("tcp client",&txperf_outinfo,disconnect_txperf);
                    continue;
                }

                if(iperf_tcp_client_status != IPERF_TCP_CLIENT_TXRUNNING){
                    if(iperf_tcp_client_status>=IPERF_TCP_CLIENT_CONNECTING){
                        iperf_printf("iperf tcp client already running, please terminate it before run again!\r\n");
                        continue;
                    }
                    txperf_init();
                    if(start_txperf_application(&(txiperf_config.ipaddr),txiperf_config.port)!=0){
                        iperf_printf("iperf tcp connect fail!\n");
                        continue;
                    }
                    count=0;
                    if(iperf_tcp_client_status==IPERF_TCP_CLIENT_TXRUNNING) break;
                    while(iperf_tcp_client_status<IPERF_TCP_CLIENT_STARTING){ //wait for tcp connect
                        lega_rtos_delay_milliseconds(100);
                        if(++count>100){
                            iperf_printf("iperf connect to %s:%d fail!\n",inet_ntoa(txiperf_config.ipaddr),txiperf_config.port);
                            break;
                        }
                    }
                    if(count>100) continue;
                    iperf_tcp_client_status=IPERF_TCP_CLIENT_TXRUNNING;
                }

                transfer_txperf_data();
            }
            else if(txiperf_config.protocol==IPERF_PROTOCOL_UDP){
                if(txiperf_config.termFlag==1){
                    // send the last datagram
                    transfer_utxperf_data(1);
                    do_iperf_terminate("udp client",&utxperf_outinfo,disconnect_utxperf);
                    continue;
                }
                if(iperf_udp_client_status>=IPERF_UDP_CLIENT_START){
                    iperf_printf("iperf udp client already running status:%d,please terminate it before run again!\r\n",iperf_udp_client_status);
                    continue;
                }
                utxperf_init();
                if(start_utxperf_application(&(txiperf_config.ipaddr),txiperf_config.port)!=0){
                    continue;
                }
                lega_rtos_delay_milliseconds(5);
                count=0;
                last_tick = lega_rtos_get_time();

                while(transfer_utxperf_data(0) != -2){
                    if(txiperf_config.delay_offset > 0)
                    {
                        current_tick = lega_rtos_get_time();
                        if(current_tick < last_tick)
                           send_tick = current_tick + 0xffffffffUL - last_tick;

                        else
                           send_tick = current_tick - last_tick;

                        if((count + txiperf_config.delay_offset) >= 1000)
                        {
                            current_sleep = txiperf_config.delay_offset - send_tick;
                            count = 0;
                        }
                        else
                        {
                            current_sleep = txiperf_config.tx_delay_ms - send_tick;
                            count += txiperf_config.tx_delay_ms;
                        }

                        if(current_sleep>0)
                           lega_rtos_delay_milliseconds(current_sleep);

                        last_tick = lega_rtos_get_time();
                    }
                    else if(txiperf_config.tx_delay_ms > 0)
                        lega_rtos_delay_milliseconds(txiperf_config.tx_delay_ms);
                    if(txiperf_config.termFlag) break;
                }
                if(txiperf_config.termFlag) continue;
            }
        }
    }
}

#define     IPERF_CLIENT_THREAD_NAME    "iperf-client"
#define     IPERF_CLIENT_PRIORITY       (28)
#define     IPERF_CLIENT_STACK_SIZE     2048
void lega_wifi_iperf_init()
{
    int i=0;

    rxperf_init();

    urxperf_init();
    utxperf_init();

    if(iperf_Semaphore==NULL)
        lega_rtos_init_semaphore(&iperf_Semaphore,0);

    /* initialize data buffer being sent */
    for (i = 0; i < SEND_BUFSIZE; i++)
        iperf_send_buf[i] = (i % 10) + '0';

    memset(&txiperf_config,0,sizeof(txiperf_config));
    txiperf_config.termFlag = 1;
    memset(&rxiperf_config,0,sizeof(rxiperf_config));
    rxiperf_config.termFlag = 1;

    lega_rtos_create_thread(NULL,IPERF_CLIENT_PRIORITY,IPERF_CLIENT_THREAD_NAME,lega_wifi_iperf_client_start,IPERF_CLIENT_STACK_SIZE,0);
}
#endif
