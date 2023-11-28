/*
  This file is part of Open Mod Manager.

  Open Mod Manager is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Mod Manager is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Open Mod Manager. If not, see <http://www.gnu.org/licenses/>.
*/
#include "OmBase.h"           //< string, vector, Om_alloc, OM_MAX_PATH, etc.
#include "OmBaseWin.h"        //< WinAPI

#include "zlib-ng/zlib.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_zDeflate(size_t* out_size, const uint8_t* in_data, size_t in_size, unsigned level)
{
  // initialize values
  (*out_size) = 0;

  // check for valid parameters
  if(!out_size || !in_size || !in_data)
    return nullptr;

  // clamp level
  if(level > 9) level = 9;

  // new Z stream structure
  z_stream zs = {};

  // initialize compressor
  if(deflateInit(&zs, level) != Z_OK)
    return nullptr;

  // allocate new output buffer
  uint8_t* buff = reinterpret_cast<uint8_t*>(Om_alloc(in_size));
  if(!buff) return nullptr;

  // setup sizes and pointers
  zs.next_out = buff;
  zs.avail_out = in_size;

  zs.next_in = const_cast<uint8_t*>(in_data);
  zs.avail_in = in_size;

  // deflate data
  if(deflate(&zs, Z_FINISH) != Z_STREAM_END) {
    Om_free(buff);
    deflateEnd(&zs);
    return nullptr;
  }

  (*out_size) = zs.total_out;

  // clean compressor
  deflateEnd(&zs);

  return buff;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_zInflate(const uint8_t* in_data, size_t in_size, size_t def_size)
{
  // check for valid parameters
  if(!in_data || !in_size || !def_size)
    return nullptr;

  // new Z stream structure
  z_stream zs = {};

  // setup input sizes and pointers
  zs.next_in = const_cast<uint8_t*>(in_data);
  zs.avail_in = in_size;

  if(inflateInit(&zs) != Z_OK)
    return nullptr;

  // allocate new output buffer
  uint8_t* buff = reinterpret_cast<uint8_t*>(Om_alloc(def_size));
  if(!buff) return nullptr;

  // setup ouput sizes and pointers
  zs.next_out = buff;
  zs.avail_out = def_size;

  // inflate data
  if(inflate(&zs, Z_NO_FLUSH) != Z_STREAM_END) {
    Om_free(buff);
    inflateEnd(&zs);
    return nullptr;
  }

  // clean decompressor
  inflateEnd(&zs);

  return buff;
}
