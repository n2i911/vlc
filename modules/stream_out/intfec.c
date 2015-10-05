#include <vlc_common.h>
#include "intfec.h"

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
