#define DEBUG 1

/*
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           timestamp                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           synchronization source (SSRC) identifier            |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |E|L|P|X|  CC   |M| PT recovery |          SN base              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           TS recovery                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |        length recovery        |     columns   |      rows     |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |        SN recovery            |      mask     |      count    |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |                            payload                            |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       0       |       1       |       2       |       3       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       4       |       5       |       6       |       7       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       8       |       9       |       10      |       11      |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |       12      |       13      |       14      |       15      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       16      |       17      |       18      |       19      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       20      |       21      |       22      |       23      |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |       24      |       25      |       26      |       27      |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |                            payload                            |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 */

typedef struct intfec_packet_t
{
    uint8_t col;
    uint8_t row;

    /* The FEC padding recovery, 1 bit */
    uint8_t padding_recovery;

    /* The FEC extension recovery, 1 bit */
    uint8_t ext_recovery;

    /* The FEC CSRC count recovery, 2 bits */
    uint8_t cc_recovery;

    /* The FEC marked recovery, 1 bit */
    uint8_t mk_recovery;

    /* The FEC payload type recovery, 7 bits */
    uint8_t pt_recovery;

    /* The base sequence number, 16 bits */
    uint16_t base_seq;

    /* The FEC timestamp recovery, 32bits */
    uint32_t ts_recovery;

    /* The FEC length recovery, 16bits */
    uint16_t len_recovery;

    /* The FEC sn recovery, 16bits */
    uint16_t sn_recovery;

    /* The FEC payload recovery */
    uint8_t *pl_recovery;
    uint16_t pl_len;

    /* The FEC packet */
    block_t *packet;

} intfec_packet_t;

typedef struct intfec_encoder_t
{
    bool start_encoding;
    uint8_t col;
    uint8_t row;

    /* The sequence number of first RTP packet in encoding block */
    uint16_t matrix_seq;

    intfec_packet_t **intfec_packets;
} intfec_encoder_t;

intfec_encoder_t* intfec_create( uint8_t, uint8_t );

intfec_packet_t* intfec_new( uint16_t, uint8_t, uint8_t, block_t* );
int intfec_add( intfec_packet_t*, uint16_t, block_t* );
void intfec_dump( intfec_packet_t* );
