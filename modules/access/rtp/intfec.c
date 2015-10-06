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
