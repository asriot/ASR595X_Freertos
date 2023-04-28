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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ota_port.h"
#include "sockets.h"
#include "netdb.h"
#include "http_ota.h"
#include "lega_rtos_api.h"

/** @brief : update the ota bin file from http server
 *  @param url    : user url pointer
 *  @example : http://192.168.21.137/asr_wifi.ota.bin
            or http://test.asr_iot.com/dev/ota/download
*/
#ifdef HTTP_OTA_SUPPORT
int http_update_ota(char *url)
{
    int fd;
    struct sockaddr_in serv_addr;
    struct hostent *host;
    char *host_name, *down_file_path;
    char *message;
    char *server_reply;
    lega_ota_boot_param_t ota_boot_para;
    int *ota_offset;

    int total_len = 0;
    unsigned long file_len = 0;
    int received_len;
    int temp = 10;
    char url_copy[50]={0};

    strcpy(url_copy, url);
    host_name = strtok(url+strlen("http://"), "/");
    printf("host name:%s \r\n", host_name);
    down_file_path = url +strlen("http://")+ strlen(host_name) + 1;
    printf("download file path:%s \r\n", down_file_path);

    if((host = gethostbyname(host_name))==NULL)
    {
        printf("Gethostname error\r\n");
        return -1;
    }
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    bcopy((char *)host->h_addr, (char *)&serv_addr.sin_addr.s_addr, host->h_length);
    printf("%s's ip address:%d.%d.%d.%d \r\n", host->h_name, (unsigned char)host->h_addr[0],\
        (unsigned char)host->h_addr[1], (unsigned char)host->h_addr[2], (unsigned char)host->h_addr[3]);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("Create socket failed\r\n");
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connect failed\r\n");
        return -1;
    }
    message = (char *)lega_rtos_malloc(200);
    if (message  == NULL)
    {
        printf("malloc memory failed \r\n");
        return -1;
    }
    memset(message,'\0',200);
    sprintf(message, "GET %s HTTP/1.1\r\nHost: %d.%d.%d.%d \r\n\r\n Connection: keep-alive\r\n\r\n",url_copy, \
            (unsigned char)host->h_addr[0], (unsigned char)host->h_addr[1], \
            (unsigned char)host->h_addr[2], (unsigned char)host->h_addr[3]);

    printf("get send message:%s \r\n", message);

    if(send(fd, message, strlen(message), 0) < 0)
    {
        printf("Send failed\r\n");
        lega_rtos_free(message);
        message = NULL;
        return -1;
    }
    lega_rtos_free(message);
    message = NULL;

    ota_boot_para.off_bp = 0;
    lega_ota_init(&ota_boot_para);
    ota_offset = NULL;

    /*
        as the first recv will get both the response of 'GET' command and the datas,
        so need locate the boundery of the response and the data, and get image size from the response;
    */
    server_reply = (char *)lega_rtos_malloc(600);
    if (server_reply  == NULL)
    {
        printf("malloc memory failed \r\n");
        return -1;
    }
    memset(server_reply,'\0',600);

    received_len = recv(fd, server_reply, 600, 0);
    printf("download from server:%d\r\n", received_len);

    if( received_len < 0 )
    {
        printf("Recv Failed\r\n");
        lega_rtos_free(server_reply);
        server_reply = NULL;
        return -1;
    }
    if (strstr(server_reply, "HTTP/1.1 200 OK") == NULL)
    {
        printf("Response not 200 OK\n");
        lega_rtos_free(server_reply);
        server_reply = NULL;
        return -1;
    }
    else
    {
        printf("get  receive message:%s \r\n", server_reply);
        char *p1, *p2, *ptemp;
        char file_lens[70];
        int data_cnt = 0;
        int ret;
        //get the OTA bin file length
        if ((p1 = strstr(server_reply, "Content-Length: ")) != NULL && (p2 = strstr(server_reply, "Connection:")) != NULL)
        {
            p1 += strlen("Content-Length: ");
            strncpy(file_lens, p1, p2 - p1);
            file_len = strtol((const char *)file_lens, NULL, 10);
            printf("download file length:%ld\r\n", file_len);
        }
        ptemp = server_reply;
        for(temp = 0;temp < received_len;temp++)
        {
            if (strncmp(ptemp,"WIFI 5501 A0V2",strlen("WIFI 5501 A0V2")) == 0)  //find the header of OTA bin
            {
                data_cnt = received_len - (ptemp - server_reply);//data_cnt is how many datas in first recv
                break;
            }
            ptemp++;
        }
        if(temp == received_len)
        {
            printf("received the ASR OTA bin falied \r\n");
            lega_rtos_free(server_reply);
            server_reply = NULL;
            return -1;
        }

        printf("Firmware upgrade start\r\n");
        ret = lega_ota_write(ota_offset, ptemp, data_cnt);
        if(ret != 0 )
        {
            printf("ota write flash Failed\r\n");
            lega_rtos_free(server_reply);
            server_reply = NULL;
            return -1;
        }
        total_len += data_cnt;
        printf("data byte size = %d total length = %d\r\n", data_cnt, total_len);
        data_cnt = 0;
        while(1)
        {
            received_len = recv(fd, server_reply , 600 , 0);
            /* check if the connection is closed */
            if( received_len < 0 )
            {
                printf("Recv Failed\r\n");
                lega_rtos_free(server_reply);
                server_reply = NULL;
                return -1;
            }
            if( total_len >= file_len )
            {
                printf("image has been received!\r\n\r\n\r\n");
                break;
            }
            else
            {
                if((file_len-total_len) >= 600)
                {
                    total_len += received_len;
                    ret = lega_ota_write(ota_offset, server_reply, received_len);
                    if(ret != 0 )
                    {
                        printf("ota write flash Failed\r\n");
                        lega_rtos_free(server_reply);
                        server_reply = NULL;
                        return -1;
                    }
                    printf("data byte size = %d total length = %d\r\n", received_len, total_len);
                }
                else
                {
                    data_cnt = file_len-total_len;          //get the length with the last packet received
                    total_len += data_cnt;
                    ret = lega_ota_write(ota_offset, server_reply, data_cnt);
                    if(ret != 0)
                    {
                        printf("ota write flash Failed\r\n");
                        lega_rtos_free(server_reply);
                        server_reply = NULL;
                        return -1;
                    }
                    printf("data byte size = %d total length = %d\r\n", data_cnt, total_len);
                    memset(server_reply,'\0',received_len);
                    printf("recv end\r\n");
                    break;
                }
                memset(server_reply,'\0',received_len);
            }
        }
    }

    lega_rtos_free(server_reply);
    server_reply = NULL;

    printf("total data byte size:%d\n", total_len);

    ota_boot_para.res_type = LEGA_OTA_FINISH;
    temp = lega_ota_set_boot(&ota_boot_para);
    printf("lega ota set mode:%d\n", temp);

    return (total_len == file_len);
}

#endif
