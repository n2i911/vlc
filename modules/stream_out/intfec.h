
typedef struct intfec_encoder_t
{
    bool start_encoding;
    uint8_t col;
    uint8_t row;

    /* The sequence number of first RTP packet in encoding block */
    uint16_t matrix_seq;
} intfec_encoder_t;

intfec_encoder_t* intfec_create( uint8_t, uint8_t );
