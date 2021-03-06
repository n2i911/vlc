/*
 * HLSStreams.hpp
 *****************************************************************************
 * Copyright (C) 2015 - VideoLAN and VLC authors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
#ifndef HLSSTREAM_HPP
#define HLSSTREAM_HPP

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "../adaptative/plumbing/StreamOutput.hpp"

namespace hls
{
    using namespace adaptative;

    class HLSStreamOutputFactory : public AbstractStreamOutputFactory
    {
        public:
            virtual AbstractStreamOutput *create(demux_t*, const StreamFormat &,
                                                 AbstractStreamOutput * = NULL) const;
    };

    class HLSPackedStreamOutput : public BaseStreamOutput
    {
        public:
            HLSPackedStreamOutput(demux_t *, const StreamFormat &, const std::string &,
                                  AbstractStreamOutput * = NULL);
            virtual void pushBlock(block_t *, bool); /* reimpl */
            virtual void setPosition(mtime_t); /* reimpl */

        private:
            bool b_timestamps_offset_set;
    };
}
#endif // HLSSTREAMS_HPP
