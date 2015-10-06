
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

#define DEBUG 1
#define DEBUG_DECODE 0

typedef struct intfec_decoder_t
{
    block_t *intfec_blocks; /* re-ordered intfec blocks queue */
    uint16_t intfec_depth;
    block_t *rtp_blocks;    /* re-ordered rtp blocks queue */
    uint16_t rtp_depth;

} intfec_decoder_t;

uint8_t intfec_cc (const block_t *block);
uint8_t intfec_mk (const block_t *block);
uint8_t intfec_pt (const block_t *block);
uint16_t intfec_baseseq (const block_t *block);
uint32_t intfec_timestamp (const block_t *block);
uint16_t intfec_len (const block_t *block);
uint8_t intfec_col (const block_t *block);
uint8_t intfec_row (const block_t *block);
uint16_t intfec_sn (const block_t *block);
uint8_t intfec_count (const block_t *block);
int intfec_dump (block_t *intfec);

int intfec_inc_count (block_t *intfec);
int intfec_set_p_x_cc (block_t *intfec, block_t *rtp);
int intfec_set_mk_pt (block_t *intfec, block_t *rtp);
int intfec_set_ts (block_t *intfec, block_t *rtp);
int intfec_set_len (block_t *intfec, block_t *rtp);
int intfec_set_sn (block_t *intfec, block_t *rtp);
int intfec_set_pl (block_t *intfec, block_t *rtp);

int intfec_sameGroup (block_t *intfec, block_t *rtp);
int intfec_decode (block_t *intfec, block_t *rtp);
int rtp_dump (block_t *rtp);
block_t* intfec_new_RTP (block_t *intfec);
void intfec_blocklist_insert (block_t **head, block_t *block);
