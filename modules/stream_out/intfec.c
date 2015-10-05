#include <vlc_common.h>
#include "intfec.h"

intfec_encoder_t* intfec_create( uint8_t col, uint8_t row )
{
    intfec_encoder_t *intfec_encoder = malloc( sizeof( intfec_encoder_t ) );

    if( intfec_encoder == NULL )
        return NULL;

    intfec_encoder->col = col;
    intfec_encoder->row = row;

    intfec_encoder->start_encoding = false;

    return intfec_encoder;
}
