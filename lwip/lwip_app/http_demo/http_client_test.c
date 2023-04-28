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
#include "sockets.h"
#include "netdb.h"
#include "http_client_test.h"
#include "lega_rtos_api.h"

#ifdef HTTP_CLIENT_DEMO_SUPPORT
/*
* http_client_demo
* usage : http_demo http://www.baidu.com/index.html
*/
int http_client_demo(char *url)
{
    int fd;
    struct sockaddr_in serv_addr;
    struct hostent *host;
    char *host_name, *down_file_path;
    char *message;
    char *server_reply;
    int received_len;
    char url_copy[50]={0};

    strcpy(url_copy, url);
    host_name = strtok(url+strlen("http://"), "/");
    printf("Host name:%s \r\n", host_name);
    down_file_path = url +strlen("http://")+ strlen(host_name) + 1;
    printf("Download file path:%s \r\n", down_file_path);

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
        printf("Malloc memory failed \r\n");
        return -1;
    }
    memset(message,'\0',200);
    sprintf(message, "GET %s HTTP/1.1\r\nHost: %d.%d.%d.%d \r\n\r\n Connection: keep-alive\r\n\r\n",url_copy, \
            (unsigned char)host->h_addr[0], (unsigned char)host->h_addr[1], \
            (unsigned char)host->h_addr[2], (unsigned char)host->h_addr[3]);

    printf("Get send message:%s \r\n", message);

    if(send(fd, message, strlen(message), 0) < 0)
    {
        printf("Send failed\r\n");
        lega_rtos_free(message);
        message = NULL;
        return -1;
    }
    lega_rtos_free(message);
    message = NULL;
    printf("Send message %s\r\n", message);

    server_reply = (char *)lega_rtos_malloc(1024);
    if (server_reply  == NULL)
    {
        printf("Malloc memory failed \r\n");
        return -1;
    }
    memset(server_reply,'\0',1024);
    received_len = recv(fd, server_reply, 1024, 0);
    printf("Download from server:%d\r\n", received_len);
    printf("Download data is : %s\r\n",server_reply);
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

    printf("http demo done\r\n");
    return 0;
}
#endif