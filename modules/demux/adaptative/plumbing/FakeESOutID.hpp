/*
 * FakeESOutID.hpp
 *****************************************************************************
 * Copyright © 2015 VideoLAN and VLC Authors
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
#ifndef FAKEESOUTID_HPP
#define FAKEESOUTID_HPP

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_es.h>

namespace adaptative
{
    class FakeESOut;

    class FakeESOutID
    {
        public:
            FakeESOutID( FakeESOut *, const es_format_t * );
            ~FakeESOutID();
            void setRealESID( es_out_id_t * );
            es_out_id_t * realESID();
            void release();
            void notifyData();
            bool isCompatible( const es_format_t * ) const;

        private:
            FakeESOut *fakeesout;
            es_out_id_t *p_real_es_id;
            es_format_t fmt;
    };
}

#endif // FAKEESOUTID_HPP
