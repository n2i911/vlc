#include <vlc_common.h>
#include <vlc_block.h>

#include <assert.h>

#include "intfec.h"

#undef DEBUG
#define DEBUG 0

static int intfec_set_padding( intfec_packet_t *intfec_packet, block_t *out )
{
    assert( intfec_packet != NULL );

    uint8_t padding_recovery = ((out->p_buffer[0] & 0x20) >> 5);

    if( DEBUG ) printf( "%s - %u\n", __func__, padding_recovery );

    intfec_packet->padding_recovery ^= padding_recovery;

    if( DEBUG ) printf( "%s - %u\n", __func__, intfec_packet->padding_recovery );

    return 0;
}

static int intfec_set_ext( intfec_packet_t *intfec_packet, block_t *out )
{
    assert( intfec_packet != NULL );

    uint8_t ext_recovery = ((out->p_buffer[0] & 0x10) >> 4);

    if( DEBUG ) printf( "%s - %u\n", __func__, ext_recovery );

    intfec_packet->ext_recovery ^= ext_recovery;

    if( DEBUG ) printf( "%s - %u\n", __func__, intfec_packet->ext_recovery );

    return 0;
}

static int intfec_set_cc( intfec_packet_t *intfec_packet, block_t *out )
{
    assert( intfec_packet != NULL );

    uint8_t cc_recovery = (out->p_buffer[0] & 0x0f);

    if( DEBUG ) printf( "%s - %u\n", __func__, cc_recovery );

    intfec_packet->cc_recovery ^= cc_recovery;

    if( DEBUG ) printf( "%s - %u\n", __func__, intfec_packet->cc_recovery );

    return 0;
}

static int intfec_set_mk( intfec_packet_t *intfec_packet, block_t *out )
{
    assert( intfec_packet != NULL );

    uint8_t mk_recovery = (out->p_buffer[1] >> 7);

    if( DEBUG ) printf( "%s - %u\n", __func__, mk_recovery );

    intfec_packet->mk_recovery ^= mk_recovery;

    if( DEBUG ) printf( "%s - %u\n", __func__, intfec_packet->mk_recovery );

    return 0;
}

static int intfec_set_pt( intfec_packet_t *intfec_packet, block_t *out )
{
    assert( intfec_packet != NULL );

    uint8_t pt_recovery = (out->p_buffer[1] & 0x7f);

    if( DEBUG ) printf( "%s - %u\n", __func__, pt_recovery );

    intfec_packet->pt_recovery ^= pt_recovery;

    if( DEBUG ) printf( "%s - %u\n", __func__, intfec_packet->pt_recovery );

    return 0;
}

static int intfec_set_ts( intfec_packet_t *intfec_packet, block_t *out )
{
    assert( intfec_packet != NULL );

    uint32_t ts_recovery = GetDWBE( out->p_buffer + 4 );

    if( DEBUG ) printf( "%s - %u\n", __func__, ts_recovery );

    intfec_packet->ts_recovery ^= ts_recovery;

    if( DEBUG ) printf( "%s - %u\n", __func__, intfec_packet->ts_recovery );

    return 0;
}

static int intfec_set_len( intfec_packet_t *intfec_packet, block_t *out )
{
    assert( intfec_packet != NULL );

    /* Size of RTP header is 12 bytes */
    uint16_t len_recovery = (out->i_buffer - 12);

    if( DEBUG ) printf( "%s - %u\n", __func__, len_recovery );

    intfec_packet->len_recovery ^= len_recovery;

    if( DEBUG ) printf( "%s - %u\n", __func__, intfec_packet->len_recovery );

    return 0;
}

static int intfec_set_sn( intfec_packet_t *intfec_packet, uint16_t sn )
{
    assert( intfec_packet != NULL );

    if( DEBUG ) printf( "%s - %u\n", __func__, sn );

    intfec_packet->sn_recovery ^= sn;

    if( DEBUG ) printf( "%s - %u\n", __func__, intfec_packet->sn_recovery );

    return 0;
}

static int intfec_set_pl( intfec_packet_t *intfec_packet, block_t *out )
{
    assert( intfec_packet != NULL );

    uint8_t *tmp = NULL;

    uint16_t i = 0;
    uint16_t len = (out->i_buffer - 12);


    if( intfec_packet->pl_len == 0 )
    {
        intfec_packet->pl_len = len;
        intfec_packet->pl_recovery = malloc( (len * sizeof( uint8_t )) );

        memcpy( intfec_packet->pl_recovery, &out->p_buffer[12], len );
    }
    else if( intfec_packet->pl_len < len )
    {
        tmp = malloc( (len * sizeof( uint8_t )) );

        assert( tmp != NULL );

        memset( tmp, 0, len );

        memcpy( tmp, intfec_packet->pl_recovery, intfec_packet->pl_len );

        free( intfec_packet->pl_recovery );

        intfec_packet->pl_len = len;
        intfec_packet->pl_recovery = tmp;

        for( i = 0; i < len; i++ )
        {
            intfec_packet->pl_recovery[i] ^= out->p_buffer[i+12];
        }
    }
    else
    {
        for( i = 0; i < len; i++ )
        {
            intfec_packet->pl_recovery[i] ^= out->p_buffer[i+12];
        }
    }

    return 0;
}

intfec_encoder_t* intfec_create( uint8_t col, uint8_t row )
{
    int i = 0;

    intfec_encoder_t *intfec_encoder = malloc( sizeof( intfec_encoder_t ) );

    if( intfec_encoder == NULL )
        return NULL;

    intfec_encoder->col = col;
    intfec_encoder->row = row;

    intfec_encoder->start_encoding = false;

    intfec_encoder->intfec_packets = malloc( col * sizeof( intfec_packet_t* ) );

    for( i = 0; i < col; i++ )
    {
        intfec_encoder->intfec_packets[i] = NULL;
    }

    return intfec_encoder;
}

intfec_packet_t* intfec_new( uint16_t sn, uint8_t col, uint8_t row, block_t *out )
{
    intfec_packet_t *intfec_packet = malloc( sizeof( intfec_packet_t ) );

    if( intfec_packet != NULL)
    {
        if( DEBUG ) printf( "%s\n", __func__ );

        memset( intfec_packet, 0, sizeof( intfec_packet_t ) );

        intfec_packet->col = col;
        intfec_packet->row = row;

        intfec_packet->base_seq = sn;

        intfec_set_padding( intfec_packet, out );

        intfec_set_ext( intfec_packet, out );

        intfec_set_cc( intfec_packet, out );

        intfec_set_mk( intfec_packet, out );

        intfec_set_pt( intfec_packet, out );

        intfec_set_ts( intfec_packet, out );

        intfec_set_len( intfec_packet, out );

        intfec_set_sn( intfec_packet, sn );

        intfec_packet->pl_len = 0;
        intfec_packet->pl_recovery = NULL;

        intfec_set_pl( intfec_packet, out );

        return intfec_packet;
    }

    return NULL;
}

int intfec_add( intfec_packet_t *intfec_packet, uint16_t sn, block_t *out )
{
    if( DEBUG ) printf( "%s\n", __func__ );

    intfec_set_padding( intfec_packet, out );

    intfec_set_ext( intfec_packet, out );

    intfec_set_cc( intfec_packet, out );

    intfec_set_mk( intfec_packet, out );

    intfec_set_pt( intfec_packet, out );

    intfec_set_ts( intfec_packet, out );

    intfec_set_len( intfec_packet, out );

    intfec_set_sn( intfec_packet, sn );

    intfec_set_pl( intfec_packet, out );

    return 0;
}

void intfec_dump( intfec_packet_t *intfec_packet )
{
    uint16_t i = 0;

    printf( "%s, %u\n", intfec_packet->base_seq );
    printf( "%s, base_seq: 0x%x, pl_len: %u\n", __func__, intfec_packet->base_seq, intfec_packet->pl_len );
    printf( "%s, padding_recovery: %u, ext_recovery: %u, cc_recovery: %u\n", __func__, intfec_packet->padding_recovery, intfec_packet->ext_recovery, intfec_packet->cc_recovery );
    printf( "%s, mk_recovery: %u, pt_recovery: 0x%x, sn_recovery: 0x%x\n", __func__, intfec_packet->mk_recovery, intfec_packet->pt_recovery, intfec_packet->sn_recovery );
    printf( "%s, col: 0x%x, row: 0x%x\n", __func__, intfec_packet->col, intfec_packet->row );
    printf( "%s, ts_recovery: 0x%x, len_recovery: %u\n", __func__, intfec_packet->ts_recovery, intfec_packet->len_recovery );

    for( i = 0; i < 16; i++ )
    {
        printf( "%x, ", intfec_packet->pl_recovery[i]);
    }

    printf( "\n" );
}
