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

uint8_t intfec_count (const block_t *block)
{
    return block->p_buffer[27];
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

    printf ("%s, Payload Recovery:\n", __func__);
    for (i = 0; i < 16; i++)
        printf ("0x%x ", intfec->p_buffer[i+28]);

    printf ("\n--------------------------------------\n");

    return 0;
}

int intfec_sameGroup (block_t *intfec, block_t *rtp)
{
    uint32_t sn;
    uint16_t base;
    uint8_t i;
    uint8_t col;
    uint8_t row;

    base = intfec_baseseq (intfec);

    col = intfec_col (intfec);
    row = intfec_row (intfec);

    for (i = 0; i < row; i++)
    {
        sn = base + (col * i);

        if (sn > 65535)
            sn = sn - 65535 - 1;

        if (sn == rtp_seq (rtp))
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

    uint8_t *tmp = NULL;

    uint16_t i = 0;
    uint16_t len = (rtp->i_buffer - 12);

    for (i = 0; i < len; i++)
    {
        intfec->p_buffer[i+28] ^= rtp->p_buffer[i+12];
    }

    return 0;
}
