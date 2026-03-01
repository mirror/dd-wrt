/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "../utilities.h"
#include "transport.h"
#include "transport_private.h"
#include "sdio_over_spi.h"
#include "../portable_endian.h"

#define MM_ADDR_BOUNDARY                (0xFFFF0000)
#define MM_ADDR_BOUNDARY_OFFSET         (0x00010000)
#define MM_KEYHOLE_ADDR_WIN0            (BIT(16))
#define MM_KEYHOLE_ADDR_WIN1            (BIT(16) | BIT(0))
#define MM_KEYHOLE_ADDR_CFG             (BIT(16) | BIT(1))
#define MM_ADDR_TO_KEYHOLE_WIN0(addr)   (((addr) >> 16) & 0xFF)
#define MM_ADDR_TO_KEYHOLE_WIN1(addr)   (((addr) >> 24) & 0xFF)
#define MM_SIZE_TO_CFG(size)            ((size) & 0x3)

#define SDIO_FUNC_REG                   (2)
#define SDIO_FUNC_MEM_BLOCK             (2)

#define SDIO_CMD_HDR_EXTRA_LEN          (13)
#define SDIO_CMD_HDR_LEN                (7)

#define SDIO_KEYHOLE_SIZE               (2)

#define SDIO_MULTI_BLOCK_START_TOKEN    (0xFC)
#define SDIO_BLOCK_END_TOKEN            (0xFD)
#define SDIO_SINGLE_START_TOKEN         (0xFE)
#define SDIO_TOKEN_LEN                  (1)
#define SDIO_TOKEN_BLOCK_READ_LEN       (2)
#define SDIO_TOKEN_BYTE_READ_LEN        (4)
#define SDIO_JUNK_TOKEN                 (0xFF)
#define SDIO_JUNK_TOKEN_LEN             (1)
#define SDIO_DATA_RESP_TOKEN_MASK       (0x1F)
#define SDIO_DATA_RESP_TOKEN_VALID_MASK (0x11)
#define SDIO_DATA_RESP_TOKEN_VALID      (0x01)
#define SDIO_DATA_RESP_TOKEN_ACPT       (0x05)
#define SDIO_DATA_RESP_TOKEN_CRCE       (0x0B)
#define SDIO_DATA_RESP_TOKEN_WE         (0x0D)
#define SDIO_CMD_RESP_TOKEN_MASK        (0xFE)
#define SDIO_CMD_RESP_EARLY_TRANS       (0xFE)
#define SDIO_CMD_RESP_TOKEN_SUCC        (0x00)

#define SDIO_CMD53_ADDR_MASK            (0xFFFF)
#define SDIO_CMD53_RESP_SIZE            (4)

#define SDIO_STOP_BIT                   BIT(0)
#define SDIO_DIR_BIT                    BIT(6)
#define SDIO_RW_BIT                     BIT(7)
#define SDIO_CMD_MASK                   (0x3F)
#define SDIO_FUNC_OFFSET                (4)
#define SDIO_BLOCK_BIT                  BIT(3)
#define SDIO_RAW_BIT                    BIT(3)
#define SDIO_OP_BIT                     BIT(2)
#define SDIO_COUNT_MASK                 (0x01)
#define SDIO_COUNT_OFFSET               (8)
#define SDIO_ADDR0_OFFSET               (1)
#define SDIO_ADDR0_BITS                 (8 - SDIO_ADDR0_OFFSET)
#define SDIO_ADDR1_OFFSET               (SDIO_ADDR0_BITS)
#define SDIO_ADDR1_BITS                 (8)
#define SDIO_ADDR2_OFFSET               (SDIO_ADDR0_BITS + SDIO_ADDR1_BITS)
#define SDIO_ADDR2_BITS                 (17 - SDIO_ADDR0_BITS - SDIO_ADDR1_BITS)
#define SDIO_CRC_OFFSET                 (1)
#define SDIO_CRC_BITS                   (8 * 5)
#define SDIO_CRC_OCTETS                 (2)
#define SDIO_CRC_READ_OCTETS            (4)

#define SDIO_R5_IDLE_BIT                BIT(0)
#define SDIO_R5_ILG_CMD_BIT             BIT(2)
#define SDIO_R5_CRC_ERR_BIT             BIT(3)
#define SDIO_R5_FUNC_ERR_BIT            BIT(4)
#define SDIO_R5_PAR_ERR_BIT             BIT(6)

#define SDIO_CMD_TIMEOUT_ATTEMPTS       (5000)

/* An optimisation would be to calculate this from the frequency. */
#define SDIO_INTERBLOCK_DELAY_OCTETS    (250UL)

/* TODO Optimise this value. */
#define SDIO_POST_BYTE_DELAY_OCTETS     (30)
#define SDIO_POST_CMD53_DELAY_OCTETS    (4)

#define MM610X_REG_RESET_ADDR           (0x10054050)
#define MM610X_REG_RESET_VALUE          (0xDEAD)
#define MM610X_REG_CLK_CTRL_ADDR        (0x1005406C)
#define MM610X_REG_CLK_CTRL_EARLY_VALUE (0xE5)
#define MM610X_REG_CHIP_ID_ADDR         (0x10054d20)
#define MM610X_REG_HOST_MAN_PTR_ADDR    (0x10054d40)

static uint32_t fn_max_block_size[] = { 4, 8, 512 };

/**
 * @brief Prints an error message if possible.
 *
 * @param transport     Transport to print the error message from.
 * @param error_code    Error code.
 * @param error_msg     Error message.
 */
static void sdio_over_spi_error(struct morsectrl_transport *transport,
                                int error_code, char *error_msg)
{
    morsectrl_transport_err("SPI", error_code, error_msg);
}

/**
 * @brief Allocate a memory buffer for an SDIO over SPI command.
 *
 * @param transport The transport structure.
 * @param cmd       The SDIO CMD to allocate.
 * @return          Returns a transport memory buffer or NULL on failure.
 */
static struct morsectrl_transport_buff *sdio_over_spi_alloc_cmd(
    struct morsectrl_transport *transport,
    uint8_t cmd)
{
    switch (cmd)
    {
    case 0:
    case 63:
    case 52:
        return transport->tops->write_alloc(transport, SDIO_CMD_HDR_LEN + SDIO_CMD_HDR_EXTRA_LEN);

    case 53:
    default:
        break;
    }

    return NULL;
}

/**
 * @brief Prepares an SDIO CMD by setting/clearing bits common between commands.
 *
 * @param cmd       The CMD to prepare.
 * @param cmd_hdr   Pointer to the command header.
 */
static void sdio_over_spi_prep_cmd(uint8_t cmd, uint8_t *cmd_hdr)
{
    if (!cmd_hdr)
        return;

    /* MSB first is the norm and what the FTDI4232H does. First octet send is junk. */
    cmd_hdr[0] = SDIO_JUNK_TOKEN;
    /* Add (start bit), direction and command. */
    cmd_hdr[1] = SDIO_DIR_BIT + (cmd & SDIO_CMD_MASK);
    /* Add stop bit. */
    cmd_hdr[6] = 0x01;
    memset(&cmd_hdr[SDIO_CMD_HDR_LEN], SDIO_JUNK_TOKEN, SDIO_CMD_HDR_EXTRA_LEN);
}

/**
 * @brief Calculate the CRC7 for the SDIO CMD.
 *
 * @param data  Pointer to the start of the SDIO CMD from the octet with the start bit.
 * @return      CRC7
 */
static uint8_t sdio_over_spi_calc_cmd_crc_octet(uint8_t *data)
{
    uint8_t crc7;
    uint64_t number = (((uint64_t)data[0]) << 32) |
                      ((uint64_t)data[1] << 24) |
                      ((uint64_t)data[2] << 16) |
                      ((uint64_t)data[3] << 8) |
                      ((uint64_t)data[4]);

    crc7 = crc7_gen(number, SDIO_CRC_BITS);
    return ((crc7 << SDIO_CRC_OFFSET) | SDIO_STOP_BIT);
}

/**
 * @brief Find the response to an SDIO CMD.
 *
 * @param transport The transport structure.
 * @param data      Pointer to the start of data to search for the response.
 * @param size      The size of the data to search.
 * @return          a pointer to the the first octet after the response on success, otherwise NULL.
 */
static uint8_t *sdio_over_spi_cmd_find_resp(struct morsectrl_transport *transport,
                                            uint8_t *data, uint32_t size)
{
    uint32_t ii;

    for (ii = 0; ii < (size - 1); ii++)
    {
        if ((data[ii] == SDIO_JUNK_TOKEN) || (data[ii] == SDIO_CMD_RESP_EARLY_TRANS))
            continue;

        /* If we find the token return the start of the data. */
        if ((data[ii] & SDIO_CMD_RESP_TOKEN_MASK) == SDIO_CMD_RESP_TOKEN_SUCC)
        {
            /* There can sometimes be an additional status byte. */
            if ((ii < size - 2) &&
                ((data[ii + 1] & SDIO_CMD_RESP_TOKEN_MASK) == SDIO_CMD_RESP_TOKEN_SUCC))
            {
                return &data[ii + 2];
            }

            return &data[ii + 1];
        }

        sdio_over_spi_error(transport, data[ii], "CMD Response Error");

        return NULL;
    }

    sdio_over_spi_error(transport, -ETRANSERR, "CMD Response Missing");
    return NULL;
}

/*
 * CMD0 bits
 * | Start bit  | 1
 * | Direction  | 1
 * | Command    | 6
 * | Data       | 32
 * | CRC7       | 7
 * | End bit    | 1
 *
 * Data should be 0 when we send this.
 */

/**
 * @brief Send an SDIO CMD0 (reset).
 *
 * @param transport The transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int sdio_over_spi_cmd0(struct morsectrl_transport *transport)
{
    const struct morsectrl_transport_ops *tops = transport->tops;
    struct morsectrl_transport_buff *cmd_buff = sdio_over_spi_alloc_cmd(transport, 0);
    struct morsectrl_transport_buff *resp_buff;
    int ret = ETRANSSUCC;

    if (!cmd_buff)
    {
        return -ETRANSERR;
    }

    resp_buff = transport->tops->read_alloc(transport, cmd_buff->data_len);
    if (!resp_buff)
    {
        ret = -ETRANSERR;
        goto exit;
    }

    sdio_over_spi_prep_cmd(0, cmd_buff->data);
    memset(&cmd_buff->data[2], 0, sizeof(*(cmd_buff->data)) * 4);
    cmd_buff->data[6] = sdio_over_spi_calc_cmd_crc_octet(&cmd_buff->data[1]);
    memset(&cmd_buff->data[7], SDIO_JUNK_TOKEN, SDIO_CMD_HDR_EXTRA_LEN);

    ret = tops->raw_read_write(transport, resp_buff, cmd_buff, true, true);
    if (ret)
        goto exit;

    if (!sdio_over_spi_cmd_find_resp(transport,
                                     &resp_buff->data[SDIO_CMD_HDR_LEN],
                                     SDIO_CMD_HDR_EXTRA_LEN))
    {
        ret = -ETRANSERR;
    }

exit:
    morsectrl_transport_buff_free(cmd_buff);
    morsectrl_transport_buff_free(resp_buff);
    return ret;
}

/**
 * @brief Send an SDIO CMD63 (enter SPI mode).
 *
 * @param transport The transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int sdio_over_spi_cmd63(struct morsectrl_transport *transport)
{
    struct morsectrl_transport_buff *cmd_buff = sdio_over_spi_alloc_cmd(transport, 63);
    struct morsectrl_transport_buff *resp_buff;
    const struct morsectrl_transport_ops *tops = transport->tops;
    int ret = ETRANSSUCC;

    if (!cmd_buff)
    {
        return -ETRANSERR;
    }

    resp_buff = transport->tops->read_alloc(transport, cmd_buff->data_len);
    if (!resp_buff)
    {
        ret = -ETRANSERR;
        goto exit;
    }

    sdio_over_spi_prep_cmd(63, cmd_buff->data);
    memset(&cmd_buff->data[2], 0, sizeof(*(cmd_buff->data)) * 4);
    cmd_buff->data[6] = sdio_over_spi_calc_cmd_crc_octet(&cmd_buff->data[1]);
    memset(&cmd_buff->data[SDIO_CMD_HDR_LEN], SDIO_JUNK_TOKEN, SDIO_CMD_HDR_EXTRA_LEN);

    ret = tops->raw_read_write(transport, resp_buff, cmd_buff, true, true);
    if (ret)
        goto exit;

    if (!sdio_over_spi_cmd_find_resp(transport,
                                     &resp_buff->data[SDIO_CMD_HDR_LEN],
                                     SDIO_CMD_HDR_EXTRA_LEN))
    {
        ret = -ETRANSERR;
    }

exit:
    morsectrl_transport_buff_free(cmd_buff);
    morsectrl_transport_buff_free(resp_buff);
    return ret;
}

/*
 * CMD52 bits
 * | Start bit  | 1
 * | Direction  | 1
 * | Command    | 6
 * | R/W        | 1
 * | Func       | 3
 * | RAW        | 1
 * | Stuff      | 1
 * | Reg Addr   | 17
 * | Stuff      | 1
 * | Data       | 8
 * | CRC7       | 7
 * | End bit    | 1
 *
 * Data is included in the command (8-bit value).
 */

/**
 * @brief Send an SDIO CMD52. This allows reading and writing of an 8-bit data value in the SDIO
 *        memory space (17-bit). If the upper address bit is set then the read/write is from/to a
 *        keyhole register to set the upper 16 address bits for the chip memory space.
 *
 * @param transport The transport structure.
 * @param write     Whether this command is a write (otherwise it is a read).
 * @param func      The function to perform the CMD52 on.
 * @param addr      The address to write to or read from.
 * @param data      The buffer to read data from or write data to.
 * @return          0 on success otherwise relevant error.
 */
static int sdio_over_spi_cmd52(struct morsectrl_transport *transport,
                               bool write,
                               uint8_t func,
                               uint32_t addr,
                               uint8_t *data)
{
    int ret = ETRANSSUCC;
    struct morsectrl_transport_buff *cmd_buff = sdio_over_spi_alloc_cmd(transport, 52);
    struct morsectrl_transport_buff *resp_buff;

    if (!cmd_buff)
    {
        return -ETRANSERR;
    }

    resp_buff = transport->tops->read_alloc(transport, cmd_buff->data_len);
    if (!resp_buff)
    {
        ret = -ETRANSERR;
        goto exit;
    }

    sdio_over_spi_prep_cmd(52, cmd_buff->data);
    cmd_buff->data[2] = (func << SDIO_FUNC_OFFSET);
    cmd_buff->data[2] |= (addr >> SDIO_ADDR2_OFFSET);
    cmd_buff->data[3] = (addr >> SDIO_ADDR1_OFFSET);
    cmd_buff->data[4] = (addr << SDIO_ADDR0_OFFSET);
    cmd_buff->data[5] = *data;

    if (write)
    {
        if (transport->debug)
            mctrl_print("CMD52 Write 0x%02x to 0x%08x\n", *data, addr);

        cmd_buff->data[2] |= SDIO_RW_BIT;
    }
    else
    {
        if (transport->debug)
            mctrl_print("CMD52 Read from 0x%08x\n", addr);
    }

    cmd_buff->data[6] = sdio_over_spi_calc_cmd_crc_octet(&cmd_buff->data[1]);
    memset(&cmd_buff->data[SDIO_CMD_HDR_LEN], SDIO_JUNK_TOKEN, SDIO_CMD_HDR_EXTRA_LEN);

    ret = transport->tops->raw_read_write(transport, resp_buff, cmd_buff, true, true);
    if (ret)
    {
        sdio_over_spi_error(transport, ret, "Failed to perform CMD52 transaction");
        goto exit;
    }

    if (!sdio_over_spi_cmd_find_resp(transport,
                                     &resp_buff->data[SDIO_CMD_HDR_LEN],
                                     SDIO_CMD_HDR_EXTRA_LEN))
    {
        ret = -ETRANSERR;
        sdio_over_spi_error(transport, ret, "Failed to find CMD52 response");
    }

exit:
    morsectrl_transport_buff_free(cmd_buff);
    morsectrl_transport_buff_free(resp_buff);
    return ret;
}

/*
 * CMD53 bits
 * | Start bit  | 1
 * | Direction  | 1
 * | Command    | 6
 * | R/W        | 1
 * | Func       | 3
 * | Block mode | 1
 * | OP Code    | 1
 * | Reg Addr   | 17
 * | Count      | 9
 * | CRC7       | 7
 * | End bit    | 1
 *
 * Data follows this.
 */

/**
 * @brief Find the block start token to an SDIO CMD53.
 *
 * @param transport The transport structure.
 * @param data      Pointer to the start of data to search for the response.
 * @param size      The size of the data to search.
 * @return          a pointer to the the first octet after the block start token on success,
 *                  otherwise NULL.
 */
static uint8_t *sdio_over_spi_cmd53_find_token(struct morsectrl_transport *transport,
                                               uint8_t *data, uint32_t size)
{
    uint32_t ii;

    for (ii = 0; ii < (size - 1); ii++)
    {
        if (data[ii] == SDIO_JUNK_TOKEN)
            continue;

        /* If we find the token return the start of the data. */
        if ((data[ii] == SDIO_MULTI_BLOCK_START_TOKEN) || (data[ii] == SDIO_SINGLE_START_TOKEN))
            return &data[ii + 1];

        sdio_over_spi_error(transport, -ETRANSERR, "Unrecognised block start token");

        return NULL;
    }

    sdio_over_spi_error(transport, -ETRANSERR, "Failed to find block start token");
    return NULL;
}

/**
 * @brief Find the acknowledge to an SDIO CMD53 data block transmission.
 *
 * @param transport The transport structure.
 * @param data      Pointer to the start of data to search for the acknowledgement.
 * @param size      The size of the data to search.
 * @return          a pointer to the the first octet after the acknowledgment on success, otherwise NULL.
 */
static uint8_t *sdio_over_spi_cmd53_find_ack(struct morsectrl_transport *transport,
                                             uint8_t *data, uint32_t size)
{
    uint32_t ii;

    for (ii = 0; ii < (size - 1); ii++)
    {
        if (data[ii] == SDIO_JUNK_TOKEN)
        {
            continue;
        }

        /* If we find the token return the start of the data. */
        if (((data[ii] & SDIO_DATA_RESP_TOKEN_VALID_MASK) == SDIO_DATA_RESP_TOKEN_VALID) &&
            ((data[ii] & SDIO_DATA_RESP_TOKEN_MASK) == SDIO_DATA_RESP_TOKEN_ACPT))
        {
            return &data[ii + 1];
        }

        if ((data[ii] & SDIO_DATA_RESP_TOKEN_MASK) == SDIO_DATA_RESP_TOKEN_CRCE)
        {
            sdio_over_spi_error(transport, -ETRANSERR, "Block CRC Error");
        }
        else
        {
            sdio_over_spi_error(transport, -ETRANSERR, "Unknown Block Response");
        }

        return NULL;
    }

    return NULL;
}

/**
 * @brief Perform an SDIO CMD53. Read/writes word aligned data from/to a 32-bit chip memory address.
 *
 * @note The upper 16-bits of the address must be set using CMD52 to write the keyhole registers
 *       first.
 *
 * @param transport     The transport structure.
 * @param data          Buffer to read data from or write data into.
 * @param write         Whether this command is a write (otherwise it is a read).
 * @param func          Function to perform the command on.
 * @param block_mode    Whether transaction uses block mode.
 * @param addr          Address to write to or read from.
 * @param count         Number of blocks in block mode, otherwise number of octets (word aligned).
 * @return              0 on success otherwise relevant error.
 */
static int sdio_over_spi_cmd53(struct morsectrl_transport *transport,
                               struct morsectrl_transport_buff *data,
                               bool write,
                               uint8_t func,
                               bool block_mode,
                               uint32_t addr,
                               uint16_t count)
{
    const struct morsectrl_transport_ops *tops = transport->tops;
    struct morsectrl_transport_buff *full_trans;
    struct morsectrl_transport_buff *resp;
    uint8_t *cmd_hdr;
    size_t full_trans_size;
    uint32_t block_size = block_mode ? fn_max_block_size[func] : count;
    uint16_t post_block_delay_bytes;
    size_t total_block_size;
    uint16_t loop_count = block_mode ? count : 1;
    int ii;
    uint8_t offset;
    int ret;
    uint16_t crc16;

    /* Constuct a complete transaction with CMD, CMD response, Read/Write. */
    if (write)
    {
        post_block_delay_bytes =
            block_mode ? SDIO_INTERBLOCK_DELAY_OCTETS : SDIO_POST_BYTE_DELAY_OCTETS;
        total_block_size = SDIO_TOKEN_LEN + block_size + SDIO_CRC_OCTETS + post_block_delay_bytes;
        if (!block_mode)
        {
            full_trans_size = SDIO_CMD_HDR_LEN +
                              SDIO_CMD53_RESP_SIZE +
                              SDIO_POST_CMD53_DELAY_OCTETS +
                              total_block_size;
        }
        else
        {
            full_trans_size = SDIO_CMD_HDR_LEN +
                              SDIO_CMD53_RESP_SIZE +
                              SDIO_POST_CMD53_DELAY_OCTETS +
                              (count * total_block_size);
        }
    }
    else /* Read */
    {
        if (!block_mode)
        {
            /* Interblock delay is timing based so scale with size when less than a full block.
             * TODO optimise this value.
             */
            post_block_delay_bytes =
                SDIO_INTERBLOCK_DELAY_OCTETS;

            total_block_size = SDIO_TOKEN_BYTE_READ_LEN +
                               block_size +
                               SDIO_CRC_READ_OCTETS +
                               post_block_delay_bytes;

            full_trans_size = SDIO_CMD_HDR_LEN + SDIO_POST_CMD53_DELAY_OCTETS + total_block_size;
        }
        else
        {
            post_block_delay_bytes = SDIO_POST_BYTE_DELAY_OCTETS;
            total_block_size = SDIO_TOKEN_BLOCK_READ_LEN +
                               block_size +
                               post_block_delay_bytes;

            full_trans_size = SDIO_CMD_HDR_LEN +
                              SDIO_CMD53_RESP_SIZE +
                              SDIO_POST_CMD53_DELAY_OCTETS +
                              (total_block_size * count);
        }
    }

    if (transport->debug)
    {
        mctrl_print("block_mode: %s\n", block_mode ? "true" : "false");
        mctrl_print("post_block_delay_bytes: %u\n", post_block_delay_bytes);
        mctrl_print("total_block_size: %zu\n", total_block_size);
        mctrl_print("block_size: %u\n", block_size);
        mctrl_print("full_trans_size: %zu\n", full_trans_size);
        mctrl_print("loop_count: %u\n", loop_count);
    }

    /* Allocate some buffers. */
    full_trans = morsectrl_transport_raw_write_alloc(transport, full_trans_size);
    resp = morsectrl_transport_raw_read_alloc(transport, full_trans_size);
    if (!full_trans || !resp)
    {
        ret = -ETRANSERR;
        sdio_over_spi_error(transport, ret, "CMD53 failed to allocate buffers");
        morsectrl_transport_buff_free(full_trans);
        morsectrl_transport_buff_free(resp);
        return ret;
    }

    cmd_hdr = full_trans->data;
    sdio_over_spi_prep_cmd(53, cmd_hdr);

    cmd_hdr[2] = (func << SDIO_FUNC_OFFSET);
    cmd_hdr[2] |= write ? SDIO_RW_BIT : 0;
    cmd_hdr[2] |= block_mode ? SDIO_BLOCK_BIT : 0;
    cmd_hdr[2] |= SDIO_OP_BIT;
    cmd_hdr[2] |= ((addr & SDIO_CMD53_ADDR_MASK) >> SDIO_ADDR2_OFFSET);
    cmd_hdr[3] = ((addr & SDIO_CMD53_ADDR_MASK) >> SDIO_ADDR1_OFFSET);
    cmd_hdr[4] = ((addr & SDIO_CMD53_ADDR_MASK) << SDIO_ADDR0_OFFSET);
    cmd_hdr[4] |= (count >> SDIO_COUNT_OFFSET) & SDIO_COUNT_MASK;
    cmd_hdr[5] = (uint8_t)count;
    cmd_hdr[6] = sdio_over_spi_calc_cmd_crc_octet(&cmd_hdr[1]);

    if (write)
    {
        offset = SDIO_CMD_HDR_LEN + SDIO_CMD53_RESP_SIZE + SDIO_POST_CMD53_DELAY_OCTETS;
        memset(&full_trans->data[SDIO_CMD_HDR_LEN], SDIO_JUNK_TOKEN, offset - SDIO_CMD_HDR_LEN);

        for (ii = 0; ii < loop_count; ii++)
        {
            size_t token_offset = offset + (ii * total_block_size);
            size_t block_offset = token_offset + SDIO_TOKEN_LEN;
            size_t crc_offset = block_offset + block_size;
            size_t interblock_offset = crc_offset + SDIO_CRC_OCTETS;

            /* Write the start token. */
            if (block_mode)
                full_trans->data[token_offset] = SDIO_MULTI_BLOCK_START_TOKEN;
            else
                full_trans->data[token_offset] = SDIO_SINGLE_START_TOKEN;

            /* Copy a data block. */
            memcpy(&full_trans->data[block_offset],
                   &data->data[ii * block_size],
                   block_size);

            crc16 = crc16_gen(&data->data[ii * block_size], block_size);
            full_trans->data[crc_offset] = crc16 >> 8;
            full_trans->data[crc_offset + 1] = crc16 & 0xFF;

            /* Copy 0xFF for interblock or post byte writing. */
            memset(&full_trans->data[interblock_offset],
                   SDIO_JUNK_TOKEN,
                   post_block_delay_bytes);
        }
    }
    else
    {
        offset = SDIO_CMD_HDR_LEN + SDIO_CMD53_RESP_SIZE;
        memset(&full_trans->data[SDIO_CMD_HDR_LEN],
               SDIO_JUNK_TOKEN,
               full_trans->data_len - SDIO_CMD_HDR_LEN);
    }

    /* Send the entire command, including the data. */
    ret = tops->raw_read_write(transport, resp, full_trans, true, true);
    if (ret)
    {
        sdio_over_spi_error(transport, ret, "CMD53 Read/Write error");
        goto exit;
    }

    /* Check for command success. */
    if (!sdio_over_spi_cmd_find_resp(transport,
                                     &resp->data[SDIO_CMD_HDR_LEN],
                                     SDIO_CMD53_RESP_SIZE))
    {
        ret = -ETRANSERR;
        sdio_over_spi_error(transport, ret, "CMD53 Error");
        goto exit;
    }

    /* Process write acks. */
    if (write)
    {
        uint8_t *ptr = &resp->data[offset];

        for (ii = 0; ii < loop_count; ii++)
        {
            size_t start_idx = ii * total_block_size;
            uint8_t *ack;

            ack = sdio_over_spi_cmd53_find_ack(transport, &ptr[start_idx], total_block_size);
            if (!ack)
            {
                ret = -ETRANSERR;
                sdio_over_spi_error(transport, ret, "CMD53 Write block ack error");
                goto exit;
            }
        }
    }
    /* Process read. */
    else
    {
        uint8_t *ptr = &resp->data[offset];

        for (ii = 0; ii < loop_count; ii++)
        {
            ptr = sdio_over_spi_cmd53_find_token(transport, ptr, post_block_delay_bytes);

            if (!ptr)
            {
                ret = -ETRANSERR;
                sdio_over_spi_error(transport, ret, "CMD53 Read start token missing.");
                goto exit;
            }

            if (!crc16_check(ptr, block_size, (ptr[block_size] << 8) + ptr[block_size + 1]))
            {
                ret = -ETRANSERR;
                sdio_over_spi_error(transport, ret, "CMD53 Read block CRC error");
                goto exit;
            }

            memcpy(&data->data[ii * block_size], ptr, block_size);
            ptr += block_size + SDIO_CRC_READ_OCTETS;
        }
    }

exit:
    morsectrl_transport_buff_free(full_trans);
    morsectrl_transport_buff_free(resp);

    return ret;
}

int sdio_over_spi_read_reg_32bit(struct morsectrl_transport *transport,
                                 uint32_t addr,
                                 uint32_t *data)
{
    struct morsectrl_transport_buff *read;
    int ret;

    if (!transport || !transport->tops || !transport->tops->raw_read)
        return -ETRANSERR;

    read = transport->tops->read_alloc(transport, sizeof(uint32_t));

    if (!read)
        return -ETRANSERR;

    ret = sdio_over_spi_read_memblock(transport, read, addr);
    if (ret)
    {
        morsectrl_transport_buff_free(read);
        return ret;
    }

    memcpy((uint8_t *)data, read->data, sizeof(*data));
    *data = le32toh(*data);

    morsectrl_transport_buff_free(read);
    return ETRANSSUCC;
}

int sdio_over_spi_write_reg_32bit(struct morsectrl_transport *transport,
                                  uint32_t addr,
                                  uint32_t data)
{
    struct morsectrl_transport_buff *write;
    uint32_t le_data;
    int ret;

    if (!transport || !transport->tops || !transport->tops->raw_write)
        return -ETRANSERR;

    write = transport->tops->write_alloc(transport, sizeof(uint32_t));

    if (!write)
        return -ETRANSERR;

    le_data = htole32(data);
    memcpy(write->data, (uint8_t *)&le_data, sizeof(data));

    if (transport->debug)
    {
        mctrl_print("data: 0x%08x, %02x %02x %02x %02x\n", data,
               write->data[0], write->data[1], write->data[2], write->data[3]);
    }

    ret = sdio_over_spi_write_memblock(transport, write, addr);

    morsectrl_transport_buff_free(write);
    return ret;
}

static int sdio_over_spi_setup_keyhole(struct morsectrl_transport *transport,
                                       uint32_t addr, size_t size)
{
    int ret;
    uint8_t key_win0 = MM_ADDR_TO_KEYHOLE_WIN0(addr);
    uint8_t key_win1 = MM_ADDR_TO_KEYHOLE_WIN1(addr);
    uint8_t key_cfg = MM_SIZE_TO_CFG(size);

    /* Always write keyhole registers because the overhead should be small. */
    ret = sdio_over_spi_cmd52(transport, true, SDIO_FUNC_REG, MM_KEYHOLE_ADDR_WIN0, &key_win0);
    if (ret)
    {
        sdio_over_spi_error(transport, ret, "Failed to set window0 keyhole reg");
        return ret;
    }
    ret = sdio_over_spi_cmd52(transport, true, SDIO_FUNC_REG, MM_KEYHOLE_ADDR_WIN1, &key_win1);
    if (ret)
    {
        sdio_over_spi_error(transport, ret, "Failed to set window1 keyhole reg");
        return ret;
    }
    ret = sdio_over_spi_cmd52(transport, true, SDIO_FUNC_REG, MM_KEYHOLE_ADDR_CFG, &key_cfg);
    if (ret)
    {
        sdio_over_spi_error(transport, ret, "Failed to set cfg keyhole reg");
        return ret;
    }

    return ret;
}

static int sdio_over_spi_memblock_common(struct morsectrl_transport *transport,
                                  struct morsectrl_transport_buff *buff,
                                  bool write,
                                  uint32_t addr)
{
    int ret = ETRANSSUCC;
    uint8_t *orig_data = buff->data;
    size_t orig_data_len = buff->data_len;
    size_t remaining_data_len = buff->data_len;
    uint32_t chip_mem_addr = addr;
    uint16_t num_blocks;

    if (transport->debug)
    {
        mctrl_print("Total %s size 0x%08zX\n", write ? "write" : "read", orig_data_len);
        mctrl_print("Start address for %s 0x%08" PRIX32 "\n", write ? "write" : "read", addr);
    }

    /* Perform writes withing 64k boundaries. */
    while (remaining_data_len)
    {
        size_t current_section_end = (chip_mem_addr & MM_ADDR_BOUNDARY) + MM_ADDR_BOUNDARY_OFFSET;
        size_t current_size = MIN(current_section_end - chip_mem_addr, remaining_data_len);
        uint32_t byte_mode_count;

        if (transport->debug)
        {
            mctrl_print("%s from 0x%08X to 0x%08zX\n", write ? "write" : "read",
                   chip_mem_addr, chip_mem_addr + current_size - 1);
        }

        ret = sdio_over_spi_setup_keyhole(transport, chip_mem_addr, SDIO_KEYHOLE_SIZE);
        if (ret)
        {
            sdio_over_spi_error(transport, ret, "Failed to set keyhole registers");
            return ret;
        }

        num_blocks = current_size / fn_max_block_size[SDIO_FUNC_MEM_BLOCK];
        byte_mode_count = current_size % fn_max_block_size[SDIO_FUNC_MEM_BLOCK];
        if (transport->debug)
            mctrl_print("%d blocks\n", num_blocks);

        if (current_size / fn_max_block_size[SDIO_FUNC_MEM_BLOCK])
        {
            /* Read/write blocks first. */
            ret = sdio_over_spi_cmd53(transport,
                                      buff,
                                      write,
                                      SDIO_FUNC_MEM_BLOCK,
                                      true,
                                      chip_mem_addr,
                                      current_size / fn_max_block_size[SDIO_FUNC_MEM_BLOCK]);
        }
        buff->data += (current_size - byte_mode_count);

        if (transport->debug)
        {
            mctrl_print("chip mem addr 0x%08x, currentsize %zu, byte_mode_count %u\n",
                   chip_mem_addr, current_size, byte_mode_count);
            mctrl_print("%d octets 'cleanup'\n", byte_mode_count);
        }

        /* Read/write octets to cleanup if required. */
        if (byte_mode_count)
        {
            /* Align to words, extra octets can be garbage. Upper layers should have done the
             * alignment already but enforce it here. */
            size_t aligned_count = align_size(byte_mode_count, sizeof(uint32_t));

            ret = sdio_over_spi_cmd53(transport,
                                      buff,
                                      write,
                                      SDIO_FUNC_MEM_BLOCK,
                                      false,
                                      chip_mem_addr + current_size - byte_mode_count,
                                      aligned_count);
        }

        chip_mem_addr += current_size;
        buff->data += byte_mode_count;
        remaining_data_len -= current_size;
    }

    /* Restore data pointer and length. */
    buff->data = orig_data;
    buff->data_len = orig_data_len;

    return ret;
}

int sdio_over_spi_read_memblock(struct morsectrl_transport *transport,
                                struct morsectrl_transport_buff *buff,
                                uint32_t addr)
{
    return sdio_over_spi_memblock_common(transport, buff, false, addr);
}

int sdio_over_spi_write_memblock(struct morsectrl_transport *transport,
                                 struct morsectrl_transport_buff *buff,
                                 uint32_t addr)
{
    return sdio_over_spi_memblock_common(transport, buff, true, addr);
}

int sdio_over_spi_post_hard_reset(struct morsectrl_transport *transport)
{
    const struct morsectrl_transport_ops *tops = transport->tops;
    int ii;
    int ret;
    uint32_t data32;

    /* First Send a CMD63 */
    for (ii = 0; ii < 3; ii++)
    {
        sdio_over_spi_cmd63(transport);
        sdio_over_spi_cmd0(transport);
    }

    ret = tops->reg_read(transport, MM610X_REG_CHIP_ID_ADDR, &data32);
    if (ret)
    {
        morsectrl_transport_err("Pre Firmware DL", -ETRANSERR, "Failed to read chip id reg\n");
        return ret;
    }

    if (transport->debug)
        mctrl_print("Chip ID: 0x%08x\n", data32);

    for (ii = 0; ii < 3; ii++)
    {
        /* Register writes to get things moving. */
        ret = tops->reg_write(transport, MM610X_REG_RESET_ADDR, MM610X_REG_RESET_VALUE);
        if (ret)
        {
            morsectrl_transport_err("Pre Firmware DL", -ETRANSERR, "Failed to write reset reg\n");

            sleep_ms(400);
            continue;
        }

        sleep_ms(400);

        ret = tops->reg_write(transport, MM610X_REG_CLK_CTRL_ADDR, MM610X_REG_CLK_CTRL_EARLY_VALUE);
        if (ret)
        {
            morsectrl_transport_err("Pre Firmware DL", -ETRANSERR,
                                    "Failed to write clk ctrl reg\n");
            continue;
        }
        else
            break;
    }
    if (ret)
    {
        return ret;
    }

    /* Invalidation of host manifest pointer? */
    ret = tops->reg_write(transport, MM610X_REG_HOST_MAN_PTR_ADDR, 0);
    if (ret)
    {
        morsectrl_transport_err("Pre Firmware DL", -ETRANSERR,
                                "Failed to reset host manifest ptr\n");
        return ret;
    }

    return ret;
}
