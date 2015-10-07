/**
 * @file rtp.h
 * @brief RTP demux module shared declarations
 */
/*****************************************************************************
 * Copyright © 2008 Rémi Denis-Courmont
 * Copyright © 2013-2014 Sean Chiang <n2i911@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/

typedef struct rtp_pt_t rtp_pt_t;
typedef struct rtp_session_t rtp_session_t;

/** @section RTP payload format */
struct rtp_pt_t
{
    void   *(*init) (demux_t *);
    void    (*destroy) (demux_t *, void *);
    void    (*decode) (demux_t *, void *, block_t *);
    uint32_t  frequency; /* RTP clock rate (Hz) */
    uint8_t   number;
};
void rtp_autodetect (demux_t *, rtp_session_t *, const block_t *);

static inline uint8_t rtp_cc (const block_t *block)
{
    return block->p_buffer[0] & 0x0f;
}

static inline uint8_t rtp_mk (const block_t *block)
{
    return (block->p_buffer[1] >> 7);
}

static inline uint8_t rtp_ptype (const block_t *block)
{
    return block->p_buffer[1] & 0x7F;
}

static inline uint16_t rtp_seq (const block_t *block)
{
    assert (block->i_buffer >= 4);
    return GetWBE (block->p_buffer + 2);
}

static inline uint32_t rtp_timestamp (const block_t *block)
{
    assert (block->i_buffer >= 12);
    return GetDWBE (block->p_buffer + 4);
}

static inline uint32_t rtp_ssrc (const block_t *block)
{
    assert (block->i_buffer >= 12);
    return GetDWBE (block->p_buffer + 8);
}

void *codec_init (demux_t *demux, es_format_t *fmt);
void codec_destroy (demux_t *demux, void *data);
void codec_decode (demux_t *demux, void *data, block_t *block);

void *theora_init (demux_t *demux);
void xiph_destroy (demux_t *demux, void *data);
void xiph_decode (demux_t *demux, void *data, block_t *block);

/** @section RTP session */
rtp_session_t *rtp_session_create (demux_t *);
void rtp_session_destroy (demux_t *, rtp_session_t *);
void rtp_queue (demux_t *, rtp_session_t *, block_t *);
void rtp_enqueue (demux_t *, rtp_session_t *, block_t *);
void intfec_queue (demux_t *, rtp_session_t *, block_t *);
bool rtp_dequeue (demux_t *, const rtp_session_t *, mtime_t *);
void rtp_dequeue_force (demux_t *, const rtp_session_t *);
int rtp_add_type (demux_t *demux, rtp_session_t *ses, const rtp_pt_t *pt);

void *rtp_dgram_thread (void *data);
void *rtp_stream_thread (void *data);

/* Global data */
struct demux_sys_t
{
    rtp_session_t *session;
    stream_t *chained_demux;
#ifdef HAVE_SRTP
    struct srtp_session_t *srtp;
#endif
    int           fd;
    int           rtcp_fd;
    vlc_thread_t  thread;

    bool b_intfec;
    uint16_t max_rtpdepth;
    uint16_t max_fecdepth;

    mtime_t       timeout;
    uint16_t      max_dropout; /**< Max packet forward misordering */
    uint16_t      max_misorder; /**< Max packet backward misordering */
    uint8_t       max_src; /**< Max simultaneous RTP sources */
    bool          thread_ready;
    bool          autodetect; /**< Payload type autodetection pending */
};

