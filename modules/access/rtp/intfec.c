/**
 * @file intfec.c
 * @brief interleaved FEC packet utilities
 */
/*****************************************************************************
 * Copyright © 2013-2014 Sean Chiang <n2i911@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/

#include <vlc_common.h>
#include <vlc_demux.h>
#include <vlc_network.h>

#include <assert.h>
#include <errno.h>

#include "rtp.h"
#include "intfec.h"

uint8_t intfec_cc (const block_t *block)
{
    return (block->p_buffer[12] & 0x0f);
}

uint8_t intfec_mk (const block_t *block)
{
    return (block->p_buffer[13] >> 7);
}

uint8_t intfec_pt (const block_t *block)
{
    return (block->p_buffer[13] & 0x7f);
}

uint16_t intfec_baseseq (const block_t *block)
{
    return GetWBE (block->p_buffer + 14);
}

uint32_t intfec_timestamp (const block_t *block)
{
    return GetDWBE (block->p_buffer + 16);
}

uint16_t intfec_len (const block_t *block)
{
    return GetWBE (block->p_buffer + 20);
}

uint8_t intfec_col (const block_t *block)
{
    return block->p_buffer[22];
}

uint8_t intfec_row (const block_t *block)
{
    return block->p_buffer[23];
}

uint16_t intfec_sn (const block_t *block)
{
    return GetWBE (block->p_buffer + 24);
}

uint8_t intfec_dim (const block_t *block)
{
    return block->p_buffer[26];
}

uint8_t intfec_count (const block_t *block)
{
    return block->p_buffer[27];
}

uint16_t intfec_rtpdepth (const block_t *block)
{
    return GetWBE (block->p_buffer + 28);
}

uint16_t intfec_intfecdepth (const block_t *block)
{
    return GetWBE (block->p_buffer + 30);
}

int intfec_dump (block_t *intfec)
{
    uint8_t i = 0;

    printf ("%s, sn: %u, type: %u\n", __func__, rtp_seq (intfec), rtp_ptype (intfec));
    printf ("%s, ts: %u, ssrc: %u\n", __func__, rtp_timestamp (intfec), rtp_ssrc (intfec));

    printf ("%s, SN Base: 0x%x, %u\n", __func__, intfec_baseseq (intfec), intfec_baseseq (intfec));
    printf ("%s, TS Recovery: 0x%x, %u\n", __func__, intfec_timestamp (intfec), intfec_timestamp (intfec));
    printf ("%s, Length Recovery: 0x%x\n", __func__, intfec_len (intfec));
    printf ("%s, Col: 0x%x, Row: 0x%x\n", __func__, intfec_col (intfec), intfec_row (intfec));

    printf ("%s, SN Recovery: 0x%x, %u\n", __func__, intfec_sn (intfec), intfec_sn (intfec));
    printf ("%s, Dim: %u\n", __func__, intfec_dim (intfec));

    printf ("%s, Max rtp depth: 0x%x, %u\n", __func__, intfec_rtpdepth (intfec), intfec_rtpdepth (intfec));
    printf ("%s, Max intfec depth: 0x%x, %u\n", __func__, intfec_intfecdepth (intfec), intfec_intfecdepth (intfec));

    printf ("%s, Payload Recovery:\n", __func__);
    for (i = 0; i < 16; i++)
        printf ("0x%x ", intfec->p_buffer[i+12+INTFEC_HEADER_LEN]);

    printf ("\n--------------------------------------\n");

    return 0;
}

int rtp_dump (block_t *rtp)
{
    uint8_t i = 0;

    printf ("%s, byte 0: 0x%x\n", __func__, rtp->p_buffer[0]);
    printf ("%s, byte 1: 0x%x\n", __func__, rtp->p_buffer[1]);

    printf ("%s, type: %u\n", __func__, rtp_ptype (rtp));
    printf ("%s, sn: %u\n", __func__, rtp_seq (rtp));
    printf ("%s, ts: %u\n", __func__, rtp_timestamp (rtp));
    printf ("%s, ssrc: %u\n", __func__, rtp_ssrc (rtp));

    printf ("%s, Payload:\n", __func__);
    for (i = 0; i < 16; i++)
        printf ("0x%x ", rtp->p_buffer[i+12]);

    printf ("\n--------------------------------------\n");

    return 0;
}

int intfec_sameGroup (block_t *intfec, block_t *rtp)
{
    uint32_t sn;
    uint16_t base;
    uint16_t seq;
    uint8_t i;
    uint8_t col;
    uint8_t row;

    base = intfec_baseseq (intfec);
    seq = rtp_seq (rtp);

    col = intfec_col (intfec);
    row = intfec_row (intfec);

    for (i = 0; i < row; i++)
    {
        sn = base + (col * i);

        if (sn > 65535)
            sn = sn - 65535 - 1;

        if (sn == seq)
            return 1;
    }
    return 0;
}

int intfec_inc_count (block_t *intfec)
{
    assert (intfec != NULL);

    intfec->p_buffer[27]++;

    return 0;
}

int intfec_set_p_x_cc (block_t *intfec, block_t *rtp)
{
    assert (rtp != NULL);
    assert (intfec != NULL);

    uint8_t padding_ext_cc_recovery = (intfec->p_buffer[12] & 0x3f);
    uint8_t padding_ext_cc = (rtp->p_buffer[0] & 0x3f);

    if (DEBUG_DECODE) printf( "%s - padding_ext_cc_recovery: %u, padding_ext_cc: %u\n", __func__, padding_ext_cc_recovery, padding_ext_cc);

    padding_ext_cc_recovery ^= padding_ext_cc;

    intfec->p_buffer[12] = (intfec->p_buffer[12] & 0xc0) | padding_ext_cc_recovery;

    if (DEBUG_DECODE) printf( "%s - padding_ext_cc_recovery: %u, 0x%x\n", __func__, padding_ext_cc_recovery, intfec->p_buffer[12]);

    return 0;
}

int intfec_set_mk_pt (block_t *intfec, block_t *rtp)
{
    assert (rtp != NULL);
    assert (intfec != NULL);

    uint8_t mk_pt_recovery = intfec->p_buffer[13];
    uint8_t mk_pt = rtp->p_buffer[1];

    if (DEBUG_DECODE) printf ("%s - mk_pt_recovery: %u, mk_pt: %u\n", __func__, mk_pt_recovery, mk_pt);

    intfec->p_buffer[13] ^= rtp->p_buffer[1];

    if (DEBUG_DECODE) printf ("%s - mk_pt_recovery: %u\n", __func__, intfec->p_buffer[13]);

    return 0;
}

int intfec_set_ts (block_t *intfec, block_t *rtp)
{
    assert (rtp != NULL);
    assert (intfec != NULL);

    uint32_t ts_recovery = intfec_timestamp (intfec);
    uint32_t ts = rtp_timestamp (rtp);

    if (DEBUG_DECODE) printf ("%s - ts_recovery: %u, ts: %u\n", __func__, ts_recovery, ts);

    ts_recovery ^= ts;

    intfec->p_buffer[16] = (ts_recovery >> 24) & 0xff;
    intfec->p_buffer[17] = (ts_recovery >> 16) & 0xff;
    intfec->p_buffer[18] = (ts_recovery >> 8 ) & 0xff;
    intfec->p_buffer[19] = (ts_recovery      ) & 0xff;

    if (DEBUG_DECODE) printf ("%s - ts_recovery: %u\n", __func__, ts_recovery);

    return 0;
}

int intfec_set_len (block_t *intfec, block_t *rtp)
{
    assert (rtp != NULL);
    assert (intfec != NULL);

    /* Size of RTP header is 12 bytes
     * Len would be only for the payload */
    uint16_t len_recovery = intfec_len (intfec);
    uint16_t len = (rtp->i_buffer - 12);

    if (DEBUG_DECODE) printf ("%s - len_recovery: %u, len: %u\n", __func__, len_recovery, len);

    len_recovery ^= len;

    intfec->p_buffer[20] = (len_recovery >> 8) & 0xff;
    intfec->p_buffer[21] = (len_recovery     ) & 0xff;

    if (DEBUG_DECODE) printf ("%s - len_recovery: %u\n", __func__, len_recovery);

    return 0;
}

int intfec_set_sn (block_t *intfec, block_t *rtp)
{
    assert (rtp != NULL);
    assert (intfec != NULL);

    uint16_t sn_recovery = intfec_sn (intfec);
    uint16_t sn = rtp_seq (rtp);

    if (DEBUG_DECODE) printf ("%s - sn_recovery: %u, sn: %u\n", __func__, sn_recovery, sn);

    sn_recovery ^= sn;

    intfec->p_buffer[24] = (sn_recovery >> 8) & 0xff;
    intfec->p_buffer[25] = (sn_recovery     ) & 0xff;

    if (DEBUG_DECODE) printf ("%s - sn_recovery: %u\n", __func__, sn_recovery);

    return 0;
}

int intfec_set_pl (block_t *intfec, block_t *rtp)
{
    assert (rtp != NULL);
    assert (intfec != NULL);

    uint16_t i = 0;
    uint16_t len = (rtp->i_buffer - 12);

    for (i = 0; i < len; i++)
    {
        intfec->p_buffer[i+12+INTFEC_HEADER_LEN] ^= rtp->p_buffer[i+12];
    }

    return 0;
}

int intfec_decode (block_t *intfec, block_t *rtp)
{
    intfec_set_p_x_cc (intfec, rtp);
    intfec_set_mk_pt (intfec, rtp);

    intfec_set_ts (intfec, rtp);
    intfec_set_len (intfec, rtp);
    intfec_set_sn (intfec, rtp);
    intfec_set_pl (intfec, rtp);

    intfec_inc_count (intfec);

    return 0;
}

block_t* intfec_new_RTP (block_t *intfec)
{
    assert (intfec != NULL);

    uint16_t len = intfec_len (intfec);

    block_t *block = block_Alloc (len + 12);

    if (unlikely (block == NULL))
        return NULL;

    /* P,X,CC */
    block->p_buffer[0] = 0x80 | intfec->p_buffer[12];

    /* Maker, Payload type */
    block->p_buffer[1] = intfec->p_buffer[13];

    /* SN */
    block->p_buffer[2] = intfec->p_buffer[24];
    block->p_buffer[3] = intfec->p_buffer[25];

    /* Timestamp */
    block->p_buffer[4] = intfec->p_buffer[16];
    block->p_buffer[5] = intfec->p_buffer[17];
    block->p_buffer[6] = intfec->p_buffer[18];
    block->p_buffer[7] = intfec->p_buffer[19];

    /* SSRC */
    block->p_buffer[8]  = intfec->p_buffer[8];
    block->p_buffer[9]  = intfec->p_buffer[9];
    block->p_buffer[10] = intfec->p_buffer[10];
    block->p_buffer[11] = intfec->p_buffer[11];

    /* Payload */
    memcpy (&block->p_buffer[12], &intfec->p_buffer[12+INTFEC_HEADER_LEN], len);

    return block;
}

uint8_t intfec_blocklist_insert (block_t **head, block_t *block, uint16_t *depth)
{
    assert (head != NULL);
    assert (block != NULL);

    uint16_t seq  = rtp_seq (block);

    /* insert new block by ascent order */
    for (block_t *prev = *head; prev != NULL; prev = *head)
    {
        if (DEBUG_VVV) printf ("%s, %u\n", __func__, rtp_seq (prev));

        int16_t delta_seq = seq - rtp_seq (prev);
        if (delta_seq < 0)
            break;
        if (delta_seq == 0)
        {
            if (DEBUG_VV) printf ("%s, duplicate packet (sequence: %u)\n", __func__, rtp_seq (prev));
            goto drop; /* duplicate */
        }
        head = &prev->p_next;
    }
    block->p_next = *head;
    *head = block;

    (*depth)++;

    if (DEBUG_VV) printf ("%s, seq: %u, depth: %u\n", __func__, rtp_seq (block), *depth);

    return 0;

drop:
    block_Release (block);
    return 1;
}

block_t* intfec_blocklist_pop (block_t **head, uint16_t *depth)
{
    assert (head != NULL);

    block_t *prev = *head;

    if (prev != NULL)
    {
        *head = prev->p_next;
        (*depth)--;

        if (DEBUG_VV) printf ("%s, seq: %u, depth: %u\n", __func__, rtp_seq (prev), *depth);

        return prev;
    }

    return NULL;
}

/**
 * @return NULL if no such this node in the block list */
block_t* intfec_blocklist_remove (block_t **head, block_t *block, uint16_t *depth)
{
    assert (head != NULL);
    assert (block != NULL);

    for (block_t *prev = *head; prev != NULL; prev = *head)
    {
        if (prev == block)
        {
            *head = prev->p_next;

            (*depth)--;

            if (DEBUG_VV) printf ("%s, seq: %u, depth: %u\n", __func__, rtp_seq (block), *depth);

            return block;
        }
        head = &prev->p_next;
    }

    return NULL;
}
