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
#include <string.h>
#include "fugue.h"
#include "asr_sd.h"

extern const sd_hc_driver_t sd_hc_driver;
static asr_sd_card_t s_sd_card;
static uint8_t __attribute__((aligned(4096))) s_data_buf[512 * SD_BUF_BLK_CNT];

void sd_process_card_insert(void)
{
    const sd_hc_driver_t *driver = &sd_hc_driver;
    asr_sd_card_t *card = &s_sd_card;

    driver->init();

    /* notify sd disk is plug in */
    card->handler(SD_EVENT_PLUG_IN, card);
}

void sd_process_card_removal(void)
{
    asr_sd_card_t *card = &s_sd_card;

    /* notify sd disk is plug out */
    card->handler(SD_EVENT_PLUG_OUT, card);
}

static void asr_sd_send_cmd(sd_command_t *cmd)
{
    const sd_hc_driver_t *driver = &sd_hc_driver;

    memset(cmd->resp, 0, sizeof(cmd->resp));

    driver->send_cmd(cmd);
}

static int asr_sd_wait_cmd_done(sd_command_t *cmd)
{
    const sd_hc_driver_t *driver = &sd_hc_driver;

    return driver->wait_cmd_done(cmd);
}

static int asr_sd_wait_data_done(void)
{
    const sd_hc_driver_t *driver = &sd_hc_driver;

    return driver->wait_data_done();
}

static int asr_sd_go_idle_state(void)
{
    sd_command_t cmd = {0};

    cmd.cmd_index = SD_CMD_GO_IDLE_STATE; // CMD0
    cmd.argument = 0;
    cmd.resp_type = SD_RESP_NONE;

    asr_sd_send_cmd(&cmd);

    return asr_sd_wait_cmd_done(&cmd);
}

static int asr_sd_send_if_cond(asr_sd_card_t *card)
{
    sd_command_t cmd = {0};
    uint8_t resp_pattern;
    int ret;

    cmd.cmd_index = SD_CMD_SEND_IF_COND; // CMD8
    cmd.argument = 1 << 8 | SD_CHECK_PATTERN;
    cmd.resp_type = SD_RESP_R7;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret == SD_OK)
    {
        resp_pattern = cmd.resp[0] & 0xFF;
        if (resp_pattern != SD_CHECK_PATTERN)
        {
            SD_TRC("Send if cond check pattern error!");
            return SD_CHECK_PATTERN_ERR;
        }

        card->card_type = SD_CARD_V2_0_STD_CAP;
    }

    return ret;
}

static int asr_sd_app_cmd(asr_sd_card_t *card)
{
    sd_command_t cmd = {0};
    int ret;

    cmd.cmd_index = SD_CMD_APP_CMD; // CMD55
    cmd.argument = (uint32_t)card->rca << 16;
    cmd.resp_type = SD_RESP_R1;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret == SD_OK)
    {
        if (!(cmd.resp[0] & SD_CARD_STATUS_APP_CMD))
            ret = SD_APP_NOT_SUPPORT;
    }

    return ret;
}

static int asr_sd_app_send_op_cond(asr_sd_card_t *card)
{
    sd_command_t cmd = {0};
    int ret;

    cmd.cmd_index = SD_CMD_APP_SEND_OP_COND; // CMD41
    if (card->card_type == SD_CARD_V2_0_STD_CAP)
        cmd.argument = 0x40100000;
    else
        cmd.argument = 0x100000;
    cmd.resp_type = SD_RESP_R3;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret == SD_OK)
    {
        while (!((cmd.resp[1] << 16) & SD_OCR_CARD_POWER_UP_STATUS))
        {
            ret = asr_sd_app_cmd(card);
            if (ret)
                return ret;

            asr_sd_send_cmd(&cmd);
            ret = asr_sd_wait_cmd_done(&cmd);
            if (ret)
                return ret;
        }

        if ((cmd.resp[1] << 16) & SD_OCR_CARD_CAP_STATUS)
            card->card_type = SD_CARD_V2_0_HIGH_CAP;
    }

    //printf("sd card type: %x\n", card->card_type);

    return ret;
}

static int asr_sd_all_send_cid(asr_sd_card_t *card)
{
    sd_command_t cmd = {0};
    int ret;

    cmd.cmd_index = SD_CMD_ALL_SEND_CID; // CMD2
    cmd.argument = 0;
    cmd.resp_type = SD_RESP_R2;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret == SD_OK)
    {
        card->cid[0] = ((uint32_t)cmd.resp[1] << 24) | ((uint32_t)cmd.resp[0] << 8) | 0x1;
        card->cid[1] = ((uint32_t)cmd.resp[3] << 24) | ((uint32_t)cmd.resp[2] << 8) | (cmd.resp[1] >> 8);
        card->cid[2] = ((uint32_t)cmd.resp[5] << 24) | ((uint32_t)cmd.resp[4] << 8) | (cmd.resp[3] >> 8);
        card->cid[3] = ((uint32_t)cmd.resp[7] << 24) | ((uint32_t)cmd.resp[6] << 8) | (cmd.resp[5] >> 8);

        //printf("card cid[0] = 0x%lx\n", card->cid[0]);
        //printf("card cid[1] = 0x%lx\n", card->cid[1]);
        //printf("card cid[2] = 0x%lx\n", card->cid[2]);
        //printf("card cid[3] = 0x%lx\n", card->cid[3]);
    }

    return ret;
}

static int asr_sd_send_relative_addr(asr_sd_card_t *card)
{
    sd_command_t cmd = {0};
    int ret;

    cmd.cmd_index = SD_CMD_SET_RELATIVE_ADDR; // CMD3
    cmd.argument = 0;
    cmd.resp_type = SD_RESP_R6;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret == SD_OK)
    {
        card->rca= cmd.resp[1];
        //printf("card rca = 0x%x\n", card->rca);
    }

    return ret;
}

static int asr_sd_send_csd(asr_sd_card_t *card)
{
    sd_command_t cmd = {0};
    int ret;

    cmd.cmd_index = SD_CMD_SEND_CSD; // CMD9
    cmd.argument = (uint32_t)card->rca << 16;
    cmd.resp_type = SD_RESP_R2;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret == SD_OK)
    {
        card->csd[0] = ((uint32_t)cmd.resp[1] << 24) | ((uint32_t)cmd.resp[0] << 8) | 0x1;
        card->csd[1] = ((uint32_t)cmd.resp[3] << 24) | ((uint32_t)cmd.resp[2] << 8) | (cmd.resp[1] >> 8);
        card->csd[2] = ((uint32_t)cmd.resp[5] << 24) | ((uint32_t)cmd.resp[4] << 8) | (cmd.resp[3] >> 8);
        card->csd[3] = ((uint32_t)cmd.resp[7] << 24) | ((uint32_t)cmd.resp[6] << 8) | (cmd.resp[5] >> 8);

        card->max_write_blk_len = 1 << ((card->csd[0] >> 22) & 0x0f);
        card->max_read_blk_len = 1 << ((card->csd[2] >> 16) & 0x0f);
        card->write_blk_partial_support = (card->csd[0] >> 21) & 0x01;
        card->read_blk_partial_support = (card->csd[2] >> 15) & 0x01;

        //printf("card csd[0] = 0x%lx\n", card->csd[0]);
        //printf("card csd[1] = 0x%lx\n", card->csd[1]);
        //printf("card csd[2] = 0x%lx\n", card->csd[2]);
        //printf("card csd[3] = 0x%lx\n", card->csd[3]);
        //printf("max write blk len = 0x%lx\n", card->max_write_blk_len);
        //printf("max read blk len = 0x%lx\n", card->max_read_blk_len);
        //printf("write block partial support = %x\n", card->write_blk_partial_support);
        //printf("read block partial support = %x\n", card->read_blk_partial_support);
    }

    return ret;
}

static int asr_sd_app_send_scr(asr_sd_card_t *card)
{
    sd_command_t cmd = {0};
    sd_data_t data = {0};
    uint8_t buf[8] = {0};
    int ret;

    cmd.cmd_index = SD_CMD_APP_SEND_SCR; // ACMD51
    cmd.argument = 0;
    cmd.resp_type = SD_RESP_R1;

    cmd.data = &data;
    cmd.data->block_size = 8;
    cmd.data->block_cnt = 1;
    cmd.data->transfer_dir = SD_DATA_CARD_TO_HOST;
    cmd.data->buf = buf;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret)
        return ret;

    ret = asr_sd_wait_data_done();
    if (ret == SD_OK)
    {
        card->scr[0] = ((uint32_t)buf[4] << 24) | ((uint32_t)buf[5] << 16)
                            | ((uint32_t)buf[6] << 8) | (uint32_t)buf[7];
        card->scr[1] = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16)
                            | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];

        card->bus_width_support = (card->scr[1] >> 16) & 0x0f;

        //printf("card scr[0] = 0x%lx\n", card->scr[0]);
        //printf("card scr[1] = 0x%lx\n", card->scr[1]);
        //printf("card bus width support = 0x%x\n", card->bus_width_support);
    }

    return ret;
}

static int asr_sd_set_block_len(uint32_t blk_len)
{
    sd_command_t cmd = {0};

    cmd.cmd_index = SD_CMD_SET_BLOCKLEN; // CMD16
    cmd.argument = blk_len;
    cmd.resp_type = SD_RESP_R1;

    asr_sd_send_cmd(&cmd);

    return asr_sd_wait_cmd_done(&cmd);
}
/*
static int asr_sd_set_block_cnt(uint32_t blk_cnt)
{
    sd_command_t cmd = {0};

    cmd.cmd_index = SD_CMD_SET_BLOCK_COUNT; // CMD23
    cmd.argument = blk_cnt;
    cmd.resp_type = SD_RESP_R1;

    asr_sd_send_cmd(&cmd);

    return asr_sd_wait_cmd_done(&cmd);
}
*/
static int asr_sd_app_set_bus_width(uint8_t width)
{
    sd_command_t cmd = {0};

    cmd.cmd_index = SD_CMD_APP_SET_BUS_WIDTH; // ACMD6
    cmd.argument = width;
    cmd.resp_type = SD_RESP_R1;

    asr_sd_send_cmd(&cmd);

    return asr_sd_wait_cmd_done(&cmd);
}

void asr_sd_init(void)
{
    const sd_hc_driver_t *driver = &sd_hc_driver;

    driver->init();
}

void asr_sd_reg_handler(asr_sd_evt_handler_t handler)
{
    s_sd_card.handler = handler;
}

void asr_sd_startup(void)
{
    const sd_hc_driver_t *driver = &sd_hc_driver;

    driver->startup();
}

int asr_sd_init_card(asr_sd_card_t *card)
{
    int ret;

    card->card_type = SD_CARD_V1_1_STD_CAP;
    card->rca = 0;

    ret = asr_sd_go_idle_state();
    if (ret)
        return ret;

    ret = asr_sd_send_if_cond(card);
    /* Response Timeout: Voltage Mismatch or Not Support Version 2.0 */
    if ((ret != SD_OK) && (ret != SD_CMD_RESP_TIMEOUT))
        return ret;

    ret = asr_sd_app_cmd(card);
    if (ret)
        return ret;

    ret = asr_sd_app_send_op_cond(card);
    if (ret)
        return ret;

    ret = asr_sd_all_send_cid(card);
    if (ret)
        return ret;

    ret = asr_sd_send_relative_addr(card);
    if (ret)
        return ret;

    ret = asr_sd_send_csd(card);
    if (ret)
        return ret;

    return ret;
}

int asr_sd_select_card(asr_sd_card_t *card)
{
    sd_command_t cmd = {0};

    cmd.cmd_index = SD_CMD_SELECT_CARD; // CMD7
    cmd.argument = (uint32_t)card->rca << 16;
    cmd.resp_type = SD_RESP_R1B;

    asr_sd_send_cmd(&cmd);

    return asr_sd_wait_cmd_done(&cmd);
}

int asr_sd_enable_4bit_bus(asr_sd_card_t *card)
{
    const sd_hc_driver_t *driver = &sd_hc_driver;
    int ret;

    ret = asr_sd_set_block_len(8);
    if (ret)
        return ret;

    ret = asr_sd_app_cmd(card);
    if (ret)
        return ret;

    ret = asr_sd_app_send_scr(card);
    if (ret)
        return ret;

    /* For SDHC, Block Len Fix 512 Bytes */
    if (card->card_type == SD_CARD_V2_0_HIGH_CAP)
    {
        ret = asr_sd_set_block_len(512);
        if (ret)
            return ret;
    }

    if (card->bus_width_support & SD_CARD_4BIT_BUS_SUPPORT)
    {
        ret = asr_sd_app_cmd(card);
        if (ret)
            return ret;

        ret = asr_sd_app_set_bus_width(SD_BUS_WIDTH_4);
        if (ret)
            return ret;
    }

    driver->enable_4bit_bus(card);

    return ret;
}

void asr_sd_config_clk(asr_sd_clk_sel_t clk_sel)
{
    const sd_hc_driver_t *driver = &sd_hc_driver;

    driver->config_clk(clk_sel);
}

int asr_sd_erase(uint32_t start, uint32_t end, uint32_t erase_fun)
{
    sd_command_t cmd = {0};
    int ret;

    ret = asr_sd_set_block_len(512);
    if (ret)
        return ret;

    cmd.cmd_index = SD_CMD_ERASE_WR_BLK_START; // CMD32
    cmd.argument = start;
    cmd.resp_type = SD_RESP_R1;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret)
        return ret;

    cmd.cmd_index = SD_CMD_ERASE_WR_BLK_END; // CMD33
    cmd.argument = end;
    cmd.resp_type = SD_RESP_R1;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret)
        return ret;

    cmd.cmd_index = SD_CMD_ERASE; // CMD38
    cmd.argument = erase_fun;
    cmd.resp_type = SD_RESP_R1B;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);

    return ret;
}

int asr_sd_write_block(asr_sd_card_t *card, uint8_t *buf, uint32_t addr, uint32_t blk_len)
{
    sd_command_t cmd = {0};
    sd_data_t data = {0};
    int ret;

    if (card->card_type == SD_CARD_V2_0_HIGH_CAP)
    {
        if (blk_len != 512)
            return SD_INVALID_PARAMETER;
    }
    else
    {
        if (card->write_blk_partial_support == true)
        {
            if ((blk_len < 1) || (blk_len > 512))
                return SD_INVALID_PARAMETER;
        }
        else
        {
            if (blk_len != 512)
                return SD_INVALID_PARAMETER;
        }

        ret = asr_sd_set_block_len(blk_len);
        if (ret)
            return ret;
    }

    cmd.cmd_index = SD_CMD_WRITE_BLOCK; // CMD24
    cmd.argument = addr;
    cmd.resp_type = SD_RESP_R1;

    cmd.data = &data;
    cmd.data->block_size = blk_len;
    cmd.data->block_cnt = 1;
    cmd.data->transfer_dir = SD_DATA_HOST_TO_CARD;
    if ((uint32_t) buf % 4096)
    {
        memcpy(s_data_buf, buf, blk_len);
        cmd.data->buf = s_data_buf;
    }
    else
    {
        cmd.data->buf = buf;
    }

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret)
        return ret;

    ret = asr_sd_wait_data_done();
    if (ret)
        return ret;

    return ret;
}

int asr_sd_write_multi_block(asr_sd_card_t *card, uint8_t *buf, uint32_t addr,
                                        uint32_t blk_len, uint32_t blk_cnt)
{
    sd_command_t cmd = {0};
    sd_data_t data = {0};
    int ret, i;
    uint32_t unit, wcnt, bcnt;
    uint8_t *wbuf;

    if (card->card_type == SD_CARD_V2_0_HIGH_CAP)
    {
        if (blk_len != 512)
            return SD_INVALID_PARAMETER;
        unit = 1;
    }
    else
    {
        if (card->write_blk_partial_support == true)
        {
            if ((blk_len < 1) || (blk_len > 512))
                return SD_INVALID_PARAMETER;
            unit = blk_len;
        }
        else
        {
            if (blk_len != 512)
                return SD_INVALID_PARAMETER;
            unit = 512;
        }

        ret = asr_sd_set_block_len(blk_len);
        if (ret)
            return ret;
    }

    wcnt = (blk_cnt - 1) / SD_BUF_BLK_CNT + 1;
    for (i = 0; i < wcnt; i++)
    {
        if (i < (wcnt - 1))
            bcnt = SD_BUF_BLK_CNT;
        else
            bcnt = blk_cnt - SD_BUF_BLK_CNT * (wcnt - 1);

        if ((uint32_t) buf % 4096)
        {
            memcpy(s_data_buf, buf, bcnt * blk_len);
            wbuf = s_data_buf;
        }
        else
        {
            wbuf = buf;
        }

        cmd.cmd_index = SD_CMD_WRITE_MULTIPLE_BLOCK; // CMD25
        cmd.argument = addr + SD_BUF_BLK_CNT * i * unit;
        cmd.resp_type = SD_RESP_R1;

        cmd.data = &data;
        cmd.data->block_size = blk_len;
        cmd.data->block_cnt = bcnt;
        cmd.data->transfer_dir = SD_DATA_HOST_TO_CARD;
        cmd.data->buf = wbuf;

        asr_sd_send_cmd(&cmd);

        ret = asr_sd_wait_cmd_done(&cmd);
        if (ret)
            return ret;

        ret = asr_sd_wait_data_done();
        if (ret)
            return ret;

        buf += bcnt * blk_len;
     }

    return ret;
}

int asr_sd_read_block(asr_sd_card_t *card, uint8_t *buf, uint32_t addr, uint32_t blk_len)
{
    sd_command_t cmd = {0};
    sd_data_t data = {0};
    int ret;

    if (card->card_type == SD_CARD_V2_0_HIGH_CAP)
    {
        if (blk_len != 512)
            return SD_INVALID_PARAMETER;
    }
    else
    {
        if ((blk_len < 1) || (blk_len > 512))
            return SD_INVALID_PARAMETER;

        ret = asr_sd_set_block_len(blk_len);
        if (ret)
            return ret;
    }

    cmd.cmd_index = SD_CMD_READ_SINGLE_BLOCK; // CMD17
    cmd.argument = addr;
    cmd.resp_type = SD_RESP_R1;

    cmd.data = &data;
    cmd.data->block_size = blk_len;
    cmd.data->block_cnt = 1;
    cmd.data->transfer_dir = SD_DATA_CARD_TO_HOST;
    if ((uint32_t) buf % 4096)
        cmd.data->buf = s_data_buf;
    else
        cmd.data->buf = buf;

    asr_sd_send_cmd(&cmd);

    ret = asr_sd_wait_cmd_done(&cmd);
    if (ret)
        return ret;

    ret = asr_sd_wait_data_done();
    if (ret)
        return ret;

    if ((uint32_t) buf % 4096)
        memcpy(buf, s_data_buf, blk_len);

    return ret;
}

int asr_sd_read_multi_block(asr_sd_card_t *card, uint8_t *buf, uint32_t addr,
                                    uint32_t blk_len, uint32_t blk_cnt)
{
    sd_command_t cmd = {0};
    sd_data_t data = {0};
    int ret, i;
    uint32_t unit, rcnt, bcnt;
    uint8_t *rbuf;

    if (card->card_type == SD_CARD_V2_0_HIGH_CAP)
    {
        if (blk_len != 512)
            return SD_INVALID_PARAMETER;
        unit = 1;
    }
    else
    {
        if ((blk_len < 1) || (blk_len > 512))
            return SD_INVALID_PARAMETER;
        unit = blk_len;

        ret = asr_sd_set_block_len(blk_len);
        if (ret)
            return ret;
    }

    rcnt = (blk_cnt - 1) / SD_BUF_BLK_CNT + 1;
    for (i = 0; i < rcnt; i++)
    {
        if (i < (rcnt - 1))
            bcnt = SD_BUF_BLK_CNT;
        else
            bcnt = blk_cnt - SD_BUF_BLK_CNT * (rcnt - 1);

        if ((uint32_t) buf % 4096)
            rbuf = s_data_buf;
        else
            rbuf = buf;

        cmd.cmd_index = SD_CMD_READ_MULTIPLE_BLOCK; // CMD18
        cmd.argument = addr + SD_BUF_BLK_CNT * i * unit;
        cmd.resp_type = SD_RESP_R1;

        cmd.data = &data;
        cmd.data->block_size = blk_len;
        cmd.data->block_cnt = bcnt;
        cmd.data->transfer_dir = SD_DATA_CARD_TO_HOST;
        cmd.data->buf = rbuf;

        asr_sd_send_cmd(&cmd);

        ret = asr_sd_wait_cmd_done(&cmd);
        if (ret)
            return ret;

        ret = asr_sd_wait_data_done();
        if (ret)
            return ret;

        if ((uint32_t) buf % 4096)
            memcpy(buf, rbuf, bcnt * blk_len);
        buf += bcnt * blk_len;
    }

    return ret;
}

int asr_sd_init_disk(void *disk)
{
    asr_sd_card_t *card = (asr_sd_card_t *) disk;
    int ret;

    if (disk)
    {
        /* init sd card */
        ret = asr_sd_init_card(card);
        if (ret)
        {
            SD_TRC("sd init card fail: %d", ret);
            return ret;
        }

        /* sd select card */
        ret = asr_sd_select_card(card);
        if (ret)
        {
            SD_TRC("sd select card fail: %d", ret);
            return ret;
        }

        /* switch to 4-bit mode */
        ret = asr_sd_enable_4bit_bus(card);
        if (ret)
        {
            SD_TRC("sd enable 4-bit mode fail: %d", ret);
            return ret;
        }

        /* config sd clock */
        asr_sd_config_clk(SD_CLK_20M);

        return ret;
    }
    else
    {
        return SD_NO_DISK;
    }
}

int asr_sd_write_disk(void *disk, uint8_t *buf, uint32_t sector, uint32_t count)
{
    asr_sd_card_t *card = (asr_sd_card_t *) disk;
    int ret;
    uint32_t addr;

    if (disk)
    {
        if (card->card_type == SD_CARD_V2_0_HIGH_CAP)
            addr = sector; // block unit address (512 Bytes unit)
        else
            addr = (uint32_t)(sector << 9); // byte unit address

        if (count == 1)
            ret = asr_sd_write_block(card, buf, addr, 512);
        else
            ret = asr_sd_write_multi_block(card, buf, addr, 512, count);

        return ret;
    }
    else
    {
        return SD_NO_DISK;
    }
}

int asr_sd_read_disk(void *disk, uint8_t *buf, uint32_t sector, uint32_t count)
{
    asr_sd_card_t *card = (asr_sd_card_t *) disk;
    int ret;
    uint32_t addr;

    if (disk)
    {
        if (card->card_type == SD_CARD_V2_0_HIGH_CAP)
            addr = sector; // block unit address (512 Bytes unit)
        else
            addr = (uint32_t)(sector << 9); // byte unit address

        if (count == 1)
            ret = asr_sd_read_block(card, buf, addr, 512);
        else
            ret = asr_sd_read_multi_block(card, buf, addr, 512, count);

        return ret;
    }
    else
    {
        return SD_NO_DISK;
    }
}

