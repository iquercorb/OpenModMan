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

#include "OmGlobal.h"

///
/// Currently not used
///
//#define UINT64_C(n) n##ull

/// \brief CRC64 table.
///
/// CRC64 table for fast CRC calculation.
///
static const uint64_t crc64_tab[256] = {
    UINT64_C(0x0000000000000000), UINT64_C(0x7ad870c830358979),
    UINT64_C(0xf5b0e190606b12f2), UINT64_C(0x8f689158505e9b8b),
    UINT64_C(0xc038e5739841b68f), UINT64_C(0xbae095bba8743ff6),
    UINT64_C(0x358804e3f82aa47d), UINT64_C(0x4f50742bc81f2d04),
    UINT64_C(0xab28ecb46814fe75), UINT64_C(0xd1f09c7c5821770c),
    UINT64_C(0x5e980d24087fec87), UINT64_C(0x24407dec384a65fe),
    UINT64_C(0x6b1009c7f05548fa), UINT64_C(0x11c8790fc060c183),
    UINT64_C(0x9ea0e857903e5a08), UINT64_C(0xe478989fa00bd371),
    UINT64_C(0x7d08ff3b88be6f81), UINT64_C(0x07d08ff3b88be6f8),
    UINT64_C(0x88b81eabe8d57d73), UINT64_C(0xf2606e63d8e0f40a),
    UINT64_C(0xbd301a4810ffd90e), UINT64_C(0xc7e86a8020ca5077),
    UINT64_C(0x4880fbd87094cbfc), UINT64_C(0x32588b1040a14285),
    UINT64_C(0xd620138fe0aa91f4), UINT64_C(0xacf86347d09f188d),
    UINT64_C(0x2390f21f80c18306), UINT64_C(0x594882d7b0f40a7f),
    UINT64_C(0x1618f6fc78eb277b), UINT64_C(0x6cc0863448deae02),
    UINT64_C(0xe3a8176c18803589), UINT64_C(0x997067a428b5bcf0),
    UINT64_C(0xfa11fe77117cdf02), UINT64_C(0x80c98ebf2149567b),
    UINT64_C(0x0fa11fe77117cdf0), UINT64_C(0x75796f2f41224489),
    UINT64_C(0x3a291b04893d698d), UINT64_C(0x40f16bccb908e0f4),
    UINT64_C(0xcf99fa94e9567b7f), UINT64_C(0xb5418a5cd963f206),
    UINT64_C(0x513912c379682177), UINT64_C(0x2be1620b495da80e),
    UINT64_C(0xa489f35319033385), UINT64_C(0xde51839b2936bafc),
    UINT64_C(0x9101f7b0e12997f8), UINT64_C(0xebd98778d11c1e81),
    UINT64_C(0x64b116208142850a), UINT64_C(0x1e6966e8b1770c73),
    UINT64_C(0x8719014c99c2b083), UINT64_C(0xfdc17184a9f739fa),
    UINT64_C(0x72a9e0dcf9a9a271), UINT64_C(0x08719014c99c2b08),
    UINT64_C(0x4721e43f0183060c), UINT64_C(0x3df994f731b68f75),
    UINT64_C(0xb29105af61e814fe), UINT64_C(0xc849756751dd9d87),
    UINT64_C(0x2c31edf8f1d64ef6), UINT64_C(0x56e99d30c1e3c78f),
    UINT64_C(0xd9810c6891bd5c04), UINT64_C(0xa3597ca0a188d57d),
    UINT64_C(0xec09088b6997f879), UINT64_C(0x96d1784359a27100),
    UINT64_C(0x19b9e91b09fcea8b), UINT64_C(0x636199d339c963f2),
    UINT64_C(0xdf7adabd7a6e2d6f), UINT64_C(0xa5a2aa754a5ba416),
    UINT64_C(0x2aca3b2d1a053f9d), UINT64_C(0x50124be52a30b6e4),
    UINT64_C(0x1f423fcee22f9be0), UINT64_C(0x659a4f06d21a1299),
    UINT64_C(0xeaf2de5e82448912), UINT64_C(0x902aae96b271006b),
    UINT64_C(0x74523609127ad31a), UINT64_C(0x0e8a46c1224f5a63),
    UINT64_C(0x81e2d7997211c1e8), UINT64_C(0xfb3aa75142244891),
    UINT64_C(0xb46ad37a8a3b6595), UINT64_C(0xceb2a3b2ba0eecec),
    UINT64_C(0x41da32eaea507767), UINT64_C(0x3b024222da65fe1e),
    UINT64_C(0xa2722586f2d042ee), UINT64_C(0xd8aa554ec2e5cb97),
    UINT64_C(0x57c2c41692bb501c), UINT64_C(0x2d1ab4dea28ed965),
    UINT64_C(0x624ac0f56a91f461), UINT64_C(0x1892b03d5aa47d18),
    UINT64_C(0x97fa21650afae693), UINT64_C(0xed2251ad3acf6fea),
    UINT64_C(0x095ac9329ac4bc9b), UINT64_C(0x7382b9faaaf135e2),
    UINT64_C(0xfcea28a2faafae69), UINT64_C(0x8632586aca9a2710),
    UINT64_C(0xc9622c4102850a14), UINT64_C(0xb3ba5c8932b0836d),
    UINT64_C(0x3cd2cdd162ee18e6), UINT64_C(0x460abd1952db919f),
    UINT64_C(0x256b24ca6b12f26d), UINT64_C(0x5fb354025b277b14),
    UINT64_C(0xd0dbc55a0b79e09f), UINT64_C(0xaa03b5923b4c69e6),
    UINT64_C(0xe553c1b9f35344e2), UINT64_C(0x9f8bb171c366cd9b),
    UINT64_C(0x10e3202993385610), UINT64_C(0x6a3b50e1a30ddf69),
    UINT64_C(0x8e43c87e03060c18), UINT64_C(0xf49bb8b633338561),
    UINT64_C(0x7bf329ee636d1eea), UINT64_C(0x012b592653589793),
    UINT64_C(0x4e7b2d0d9b47ba97), UINT64_C(0x34a35dc5ab7233ee),
    UINT64_C(0xbbcbcc9dfb2ca865), UINT64_C(0xc113bc55cb19211c),
    UINT64_C(0x5863dbf1e3ac9dec), UINT64_C(0x22bbab39d3991495),
    UINT64_C(0xadd33a6183c78f1e), UINT64_C(0xd70b4aa9b3f20667),
    UINT64_C(0x985b3e827bed2b63), UINT64_C(0xe2834e4a4bd8a21a),
    UINT64_C(0x6debdf121b863991), UINT64_C(0x1733afda2bb3b0e8),
    UINT64_C(0xf34b37458bb86399), UINT64_C(0x8993478dbb8deae0),
    UINT64_C(0x06fbd6d5ebd3716b), UINT64_C(0x7c23a61ddbe6f812),
    UINT64_C(0x3373d23613f9d516), UINT64_C(0x49aba2fe23cc5c6f),
    UINT64_C(0xc6c333a67392c7e4), UINT64_C(0xbc1b436e43a74e9d),
    UINT64_C(0x95ac9329ac4bc9b5), UINT64_C(0xef74e3e19c7e40cc),
    UINT64_C(0x601c72b9cc20db47), UINT64_C(0x1ac40271fc15523e),
    UINT64_C(0x5594765a340a7f3a), UINT64_C(0x2f4c0692043ff643),
    UINT64_C(0xa02497ca54616dc8), UINT64_C(0xdafce7026454e4b1),
    UINT64_C(0x3e847f9dc45f37c0), UINT64_C(0x445c0f55f46abeb9),
    UINT64_C(0xcb349e0da4342532), UINT64_C(0xb1eceec59401ac4b),
    UINT64_C(0xfebc9aee5c1e814f), UINT64_C(0x8464ea266c2b0836),
    UINT64_C(0x0b0c7b7e3c7593bd), UINT64_C(0x71d40bb60c401ac4),
    UINT64_C(0xe8a46c1224f5a634), UINT64_C(0x927c1cda14c02f4d),
    UINT64_C(0x1d148d82449eb4c6), UINT64_C(0x67ccfd4a74ab3dbf),
    UINT64_C(0x289c8961bcb410bb), UINT64_C(0x5244f9a98c8199c2),
    UINT64_C(0xdd2c68f1dcdf0249), UINT64_C(0xa7f41839ecea8b30),
    UINT64_C(0x438c80a64ce15841), UINT64_C(0x3954f06e7cd4d138),
    UINT64_C(0xb63c61362c8a4ab3), UINT64_C(0xcce411fe1cbfc3ca),
    UINT64_C(0x83b465d5d4a0eece), UINT64_C(0xf96c151de49567b7),
    UINT64_C(0x76048445b4cbfc3c), UINT64_C(0x0cdcf48d84fe7545),
    UINT64_C(0x6fbd6d5ebd3716b7), UINT64_C(0x15651d968d029fce),
    UINT64_C(0x9a0d8ccedd5c0445), UINT64_C(0xe0d5fc06ed698d3c),
    UINT64_C(0xaf85882d2576a038), UINT64_C(0xd55df8e515432941),
    UINT64_C(0x5a3569bd451db2ca), UINT64_C(0x20ed197575283bb3),
    UINT64_C(0xc49581ead523e8c2), UINT64_C(0xbe4df122e51661bb),
    UINT64_C(0x3125607ab548fa30), UINT64_C(0x4bfd10b2857d7349),
    UINT64_C(0x04ad64994d625e4d), UINT64_C(0x7e7514517d57d734),
    UINT64_C(0xf11d85092d094cbf), UINT64_C(0x8bc5f5c11d3cc5c6),
    UINT64_C(0x12b5926535897936), UINT64_C(0x686de2ad05bcf04f),
    UINT64_C(0xe70573f555e26bc4), UINT64_C(0x9ddd033d65d7e2bd),
    UINT64_C(0xd28d7716adc8cfb9), UINT64_C(0xa85507de9dfd46c0),
    UINT64_C(0x273d9686cda3dd4b), UINT64_C(0x5de5e64efd965432),
    UINT64_C(0xb99d7ed15d9d8743), UINT64_C(0xc3450e196da80e3a),
    UINT64_C(0x4c2d9f413df695b1), UINT64_C(0x36f5ef890dc31cc8),
    UINT64_C(0x79a59ba2c5dc31cc), UINT64_C(0x037deb6af5e9b8b5),
    UINT64_C(0x8c157a32a5b7233e), UINT64_C(0xf6cd0afa9582aa47),
    UINT64_C(0x4ad64994d625e4da), UINT64_C(0x300e395ce6106da3),
    UINT64_C(0xbf66a804b64ef628), UINT64_C(0xc5bed8cc867b7f51),
    UINT64_C(0x8aeeace74e645255), UINT64_C(0xf036dc2f7e51db2c),
    UINT64_C(0x7f5e4d772e0f40a7), UINT64_C(0x05863dbf1e3ac9de),
    UINT64_C(0xe1fea520be311aaf), UINT64_C(0x9b26d5e88e0493d6),
    UINT64_C(0x144e44b0de5a085d), UINT64_C(0x6e963478ee6f8124),
    UINT64_C(0x21c640532670ac20), UINT64_C(0x5b1e309b16452559),
    UINT64_C(0xd476a1c3461bbed2), UINT64_C(0xaeaed10b762e37ab),
    UINT64_C(0x37deb6af5e9b8b5b), UINT64_C(0x4d06c6676eae0222),
    UINT64_C(0xc26e573f3ef099a9), UINT64_C(0xb8b627f70ec510d0),
    UINT64_C(0xf7e653dcc6da3dd4), UINT64_C(0x8d3e2314f6efb4ad),
    UINT64_C(0x0256b24ca6b12f26), UINT64_C(0x788ec2849684a65f),
    UINT64_C(0x9cf65a1b368f752e), UINT64_C(0xe62e2ad306bafc57),
    UINT64_C(0x6946bb8b56e467dc), UINT64_C(0x139ecb4366d1eea5),
    UINT64_C(0x5ccebf68aecec3a1), UINT64_C(0x2616cfa09efb4ad8),
    UINT64_C(0xa97e5ef8cea5d153), UINT64_C(0xd3a62e30fe90582a),
    UINT64_C(0xb0c7b7e3c7593bd8), UINT64_C(0xca1fc72bf76cb2a1),
    UINT64_C(0x45775673a732292a), UINT64_C(0x3faf26bb9707a053),
    UINT64_C(0x70ff52905f188d57), UINT64_C(0x0a2722586f2d042e),
    UINT64_C(0x854fb3003f739fa5), UINT64_C(0xff97c3c80f4616dc),
    UINT64_C(0x1bef5b57af4dc5ad), UINT64_C(0x61372b9f9f784cd4),
    UINT64_C(0xee5fbac7cf26d75f), UINT64_C(0x9487ca0fff135e26),
    UINT64_C(0xdbd7be24370c7322), UINT64_C(0xa10fceec0739fa5b),
    UINT64_C(0x2e675fb4576761d0), UINT64_C(0x54bf2f7c6752e8a9),
    UINT64_C(0xcdcf48d84fe75459), UINT64_C(0xb71738107fd2dd20),
    UINT64_C(0x387fa9482f8c46ab), UINT64_C(0x42a7d9801fb9cfd2),
    UINT64_C(0x0df7adabd7a6e2d6), UINT64_C(0x772fdd63e7936baf),
    UINT64_C(0xf8474c3bb7cdf024), UINT64_C(0x829f3cf387f8795d),
    UINT64_C(0x66e7a46c27f3aa2c), UINT64_C(0x1c3fd4a417c62355),
    UINT64_C(0x935745fc4798b8de), UINT64_C(0xe98f353477ad31a7),
    UINT64_C(0xa6df411fbfb21ca3), UINT64_C(0xdc0731d78f8795da),
    UINT64_C(0x536fa08fdfd90e51), UINT64_C(0x29b7d047efec8728),
};


/// \brief Calculate CRC64.
///
/// Calculates and return the CRC64 value of the specified data chunk.
///
/// \param[in]  crc     : seed value for computation. 0 or (u64)~0 for a new CRC
///                       calculation, or the previous crc64 value if computing
///                       incrementally.
/// \param[in]  s       :  pointer to data chunk over which CRC64 is run.
/// \param[in]  l       :  data chunk size in bytes.
///
/// \return Converted 64 bits integer value.
///
static uint64_t __CRC64(uint64_t crc, const unsigned char *s, uint64_t l)
{
  uint64_t j;
  for (j = 0; j < l; ++j) {
      uint8_t byte = s[j];
      crc = crc64_tab[(uint8_t)crc ^ byte] ^ (crc >> 8);
  }
  return crc;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t Om_getCRC64(const void* data, size_t size)
{
  return __CRC64(0, (unsigned char*)data, size);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t Om_getCRC64(const wstring& str)
{
  return __CRC64(0, (unsigned char*)str.c_str(), str.size()*sizeof(wchar_t));
}

#include "thirdparty/xxhash/xxh3.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t Om_getXXHash3(const void* data, size_t size)
{
  return static_cast<uint64_t>(XXH3_64bits(data, size)); // XXH64_hash_t
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t Om_getXXHash3(const wstring& str)
{
  return static_cast<uint64_t>(XXH3_64bits((unsigned char*)str.c_str(), str.size()*sizeof(wchar_t))); // XXH64_hash_t
}

#include <ctime>
#include <random>

static mt19937                             __rnd_generator(time(0));
static uniform_int_distribution<uint8_t>   __rnd_uint8dist(0, 255);

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_getRandBytes(uint8_t* dest, size_t size)
{
  for(size_t i = 0; i < size; ++i) {
    dest[i] = __rnd_uint8dist(__rnd_generator);
  }
}

static const wchar_t __hex_digit[] = L"0123456789abcdef";

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_genUUID()
{
  uint8_t uuid[16];

  // Generate random bytes sequence
  for(unsigned i = 0; i < 16; ++i) {
    uuid[i] = __rnd_uint8dist(__rnd_generator);
  }

  uuid[6] = (uuid[6] & 0x0F) | 0x40; //< Set version 4
  uuid[8] = (uuid[8] & 0x3F) | 0x80; //< Set DCE variant

  // Create formated UUID string from random sequence
  wstring str(L"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");

  unsigned u = 0;
  unsigned c = 0;

  while(c < 36) {

    if(str[c] == L'-') {
      ++c; continue;
    }

    str[c] = __hex_digit[uuid[u] >>  4]; ++c;
    str[c] = __hex_digit[uuid[u] & 0xf]; ++c;

    ++u;
  }

  return str;
}


///
/// Buffer size definition for conversion functions
///
#define MBS_SIZE 3120 //< Multibyte string buffer
#define WCS_SIZE 1040  //< Wide string buffer


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
string Om_toMbString(const wstring& wcs)
{
  char buff[MBS_SIZE];

  string result;

  size_t s = wcs.size();
  if(s > 0) {
    if(s > 1024) {
      const wchar_t* cstr = wcs.c_str();
      size_t r, p = 0;
      while(p < s-1) {
        r = wcstombs(buff, &cstr[p], 1024); buff[r] = 0; p += r;
        result.append(buff);
      }
      return result;
    } else {
      wcstombs(buff, wcs.c_str(), 1024);
      result.assign(buff);
      return result;
    }
  }

  return result;

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_toMbString(string& mbs, const wstring& wcs)
{
  char cbuf[MBS_SIZE];

  size_t s = wcs.size();
  if(s > 0) {
    if(s > 1024) {
      const wchar_t* cstr = wcs.c_str();
      mbs.clear();
      size_t r, p = 0;
      while(p < s-1) {
        r = wcstombs(cbuf, &cstr[p], 1024); cbuf[r] = 0; p += r;
        mbs.append(cbuf);
      }
      return;
    } else {
      wcstombs(cbuf, wcs.c_str(), 1024);
      mbs.assign(cbuf);
      return;
    }
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_toWcString(const string& mbs)
{
  wchar_t wcbuf[WCS_SIZE];

  wstring result;

  size_t s = mbs.size();
  if(s > 0) {
    if(s > 1024) {
      const char* cstr = mbs.c_str();
      size_t r, p = 0;
      while(p < s-1) {
        r = mbstowcs(wcbuf, &cstr[p], 1024); wcbuf[r] = 0; p += r;
        result.append(wcbuf);
      }
      return result;
    } else {
      mbstowcs(wcbuf, mbs.c_str(), 1024);
      result.assign(wcbuf);
      return result;
    }
  }

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_toWcString(wstring& wcs, const string& mbs)
{
  wchar_t wcbuf[WCS_SIZE];

  size_t s = mbs.size();
  if(s > 0) {
    if(s > 1024) {
      const char* cstr = mbs.c_str();
      wcs.clear();
      size_t r, p = 0;
      while(p < s-1) {
        r = mbstowcs(wcbuf, &cstr[p], 1024); wcbuf[r] = 0; p += r;
        wcs.append(wcbuf);
      }
      return;
    } else {
      mbstowcs(wcbuf, mbs.c_str(), 1024);
      wcs.assign(wcbuf);
      return;
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_toZipPath(wstring& zpath, const wstring& path)
{
  if(path[0] == L'\\') {
    zpath = path.substr(1,wstring::npos);
  } else {
    zpath = path;
  }
  replace(zpath.begin(), zpath.end(), L'\\', L'/');
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_toZipPath(const wstring& path)
{
  wstring zpath;
  if(path[0] == L'\\') {
    zpath = path.substr(1,wstring::npos);
  } else {
    zpath = path;
  }
  replace(zpath.begin(), zpath.end(), L'\\', L'/');
  return zpath;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_fromZipPath(string& path, const string& zpath)
{
  path = "\\"; path += zpath;
  replace(path.begin(), path.end(), L'/', L'\\');
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
string Om_fromZipPath(const string& zpath)
{
  string path;
  path = "\\"; path += zpath;
  replace(path.begin(), path.end(), L'/', L'\\');
  return path;
}



/// \brief Sort comparison function
///
/// Comparison callback function for std::sort() to sort strings in
/// alphabetical order
///
/// \param[in]  a   : left string to compare.
/// \param[in]  b   : right string to compare.
///
/// \return True if a is "before" b, false otherwise
///
static bool __sortStrings_Func(const wstring& a, const wstring& b)
{
  // get size of the shorter string to compare
  size_t l = a.size() > b.size() ? b.size() : a.size();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {
    if(towupper(a[i]) != towupper(b[i])) {
      if(towupper(a[i]) < towupper(b[i])) {
        return true;
      } else {
        return false;
      }
    }
  }

  // if function does not returned at this stage, this mean strings
  // are equals in the tested portion, so the longer one is after
  if(a.size() < b.size())
    return true;

  // if the two strings are strictly equals, then, we don't care
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_sortStrings(vector<wstring>* strings)
{
  std::sort(strings->begin(), strings->end(), __sortStrings_Func);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidName(const wchar_t* name)
{
  const wchar_t check[] = L"/*?\"<>|";

  for(unsigned i = 0; i < 7; ++i)
    if(wcschr(name, check[i]))
      return false;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidName(const wstring& name)
{
  const wchar_t check[] = L"/*?\"<>|";

  for(unsigned i = 0; i < 7; ++i)
    if(name.find_first_of(check[i]) != wstring::npos)
      return false;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_sizeString(size_t bytes, bool octet)
{
  wchar_t wcbuf[256];
  wchar_t unit = (octet) ? 'o' : 'B';

  double fbytes;
  wstring result;

  if(bytes < 1024) { // 1 Ko
    if(octet) {
      swprintf(wcbuf, 256, L"%d Octet(s)", bytes);
    } else {
      swprintf(wcbuf, 256, L"%d Byte(s)", bytes);
    }
    result = wcbuf;
    return result;
  }

  if(bytes < 1048576) { // 1 Mo
    fbytes = (double)bytes / 1024.0;
    swprintf(wcbuf, 256, L"%.1f Ki%lc", fbytes, unit);
    result = wcbuf;
    return result;
  }

  if(bytes < 1073741824) { // 1 Go
    fbytes = (double)bytes / 1048576.0;
    swprintf(wcbuf, 256, L"%.1f Mi%lc", fbytes, unit);
    result = wcbuf;
    return result;
  }

  fbytes = (double)bytes / 1073741824.0;
  swprintf(wcbuf, 256, L"%.1f Gi%lc", fbytes, unit);
  result = wcbuf;
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isFileZip(const wstring& path) {

  // Microsoft functions are ugly (this is not new) but they are proven to be
  // the most efficient for file I/O... maybe because they are directly sticked
  // to the kernel at low level...
  HANDLE hFile = CreateFileW(path.c_str(),
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             nullptr,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return false;

  // We have to seek over the file until we found something else
  // that zero, because a zip file can begin with a blank space before the
  // signature...

  char buf[1024]; // <- our read buffer
  unsigned* sign; // <- our future 4 bytes signature
  DWORD r = 0; //< count of bytes read

  do {
    SetFilePointer(hFile, r, nullptr, FILE_CURRENT);
    ReadFile(hFile, &buf, 1024, &r, nullptr);
    for(unsigned i = 0; i < r; ++i) {
      // check for something else than zero
      if(buf[i] != 0) {
        // we got something, we don't go further we close the file
        // and check the result.
        CloseHandle(hFile);
        sign = (unsigned*)&buf[i]; //< cast our buffer in unsigned pointer
        return ( *sign == 0x04034b50 );
      }
    }
  } while(r == 1024);

  CloseHandle(hFile);
  return false; // PKWARE Zip file signature
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsDir(vector<wstring>* ls, const wstring& orig, bool absolute)
{
  wstring item;

  wstring srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    ls->clear();
    do {
      if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
        // skip this and parent folder
        if(!wcscmp(fd.cFileName, L".")) continue;
        if(!wcscmp(fd.cFileName, L"..")) continue;

        if(absolute) {
          item = orig; item += L"\\"; item += fd.cFileName;
          ls->push_back(item);
        } else {
          ls->push_back(fd.cFileName);
        }
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsFile(vector<wstring>* ls, const wstring& orig, bool absolute)
{
  wstring item;

  wstring srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    ls->clear();
    do {
      if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
        if(absolute) {
          item = orig; item += L"\\"; item += fd.cFileName;
          ls->push_back(item);
        } else {
          ls->push_back(fd.cFileName);
        }
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}


/// \brief List files recursively
///
/// This is the private function used to list files recursively.
///
/// \param[out] ls      : Pointer to array of wstring to be filled with result.
/// \param[in]  orig    : Path where to list items from.
/// \param[in]  from    : Path to prepend to result to obtain the item full
///                       path from the beginning of the tree exploration.
///
static void __lsFile_Recurse(vector<wstring>* ls, const wstring& orig, const wstring& from)
{
  wstring item;
  wstring root;

  wstring srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        item = from; item += L"\\"; item += fd.cFileName;
        root = orig; root += L"\\"; root += fd.cFileName;
        // go deep in tree
        __lsFile_Recurse(ls, root, item);

      } else {
        item = from; item += L"\\"; item += fd.cFileName;
        ls->push_back(item);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsFileRecursive(vector<wstring>* ls, const wstring& origin, bool absolute)
{
  ls->clear();
  if(absolute) {
    __lsFile_Recurse(ls, origin.c_str(), origin.c_str());
  } else {
    __lsFile_Recurse(ls, origin.c_str(), L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsFileFiltered(vector<wstring>* ls, const wstring& orig, const wstring& filter, bool absolute)
{
  wstring item;
  wstring root;

  wstring srch(orig);
  srch += L"\\";
  srch += filter;

  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    ls->clear();
    do {
      if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
        if(absolute) {
          item = orig; item += L"\\"; item += fd.cFileName;
          ls->push_back(item);
        } else {
          ls->push_back(fd.cFileName);
        }
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsAll(vector<wstring>* ls, const wstring& orig, bool absolute)
{
  wstring item;

  wstring srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    ls->clear();
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      if(absolute) {
        item = orig; item += L"\\"; item += fd.cFileName;
        ls->push_back(item);
      } else {
        ls->push_back(fd.cFileName);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}


/// \brief List files and folders recursively
///
/// This is the private function used to list files and folder recursively.
///
/// \param[out] ls      : Pointer to array of wstring to be filled with result.
/// \param[in]  orig    : Path where to list items from.
/// \param[in]  from    : Path to prepend to result to obtain the item full
///                       path from the beginning of the tree exploration.
///
static void __lsAll_Recurse(vector<wstring>* ls, const wstring& orig, const wstring& from)
{
  wstring item;
  wstring root;

  wstring srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        item = from; item += L"\\"; item += fd.cFileName;
        root = orig; root += L"\\"; root += fd.cFileName;
        ls->push_back(item);

        // go deep in tree
        __lsAll_Recurse(ls, root, item);

      } else {
        item = from; item += L"\\"; item += fd.cFileName;
        ls->push_back(item);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsAllRecursive(vector<wstring>* ls, const wstring& origin, bool absolute)
{
  ls->clear();
  if(absolute) {
    __lsAll_Recurse(ls, origin.c_str(), origin.c_str());
  } else {
    __lsAll_Recurse(ls, origin.c_str(), L"");
  }
}


/// \brief Compute folder size recursively
///
/// Private function to calculate the total size of folder content including
/// all it sub-folders. The function act recursively.
///
/// \param[out] size    : Pointer to variable to be incremented as result.
/// \param[in]  orig    : Path of folder to get total size (start of recursive
///                       exploration).
///
void __folderSize(size_t* size, const wstring& orig)
{
  wstring root;

  wstring srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        root = orig; root += L"\\"; root += fd.cFileName;

        // go deep in tree
        __folderSize(size, root);

      } else {
        root = orig; root += L"\\"; root += fd.cFileName;
        HANDLE hFile = CreateFileW(root.c_str(),
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   nullptr,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   nullptr);
        *size += GetFileSize(hFile, nullptr);
        CloseHandle(hFile);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_itemSize(const wstring& path)
{
  size_t ret;

  if(Om_isFile(path)) {
    HANDLE hFile = CreateFileW(path.c_str(),
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               nullptr,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               nullptr);
    ret = GetFileSize(hFile, nullptr);
    CloseHandle(hFile);
  } else {
    ret = 0;
    __folderSize(&ret, path);
  }

  return ret;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
time_t Om_itemTime(const wstring& path)
{
  time_t ret;
  FILETIME fTime;

  if(Om_isFile(path)) {
    HANDLE hFile = CreateFileW(path.c_str(),
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               nullptr,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               nullptr);

    GetFileTime(hFile, nullptr, nullptr, &fTime);
    CloseHandle(hFile);

    ULARGE_INTEGER ull;
    ull.LowPart = fTime.dwLowDateTime;
    ull.HighPart = fTime.dwHighDateTime;
    ret = ull.QuadPart / 10000000ULL - 11644473600ULL;

  } else {
    ret = 0;
  }

  return ret;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_moveToTrash(const wstring& path)
{
  wchar_t wcbuf[512];
  wcscpy(wcbuf, path.c_str());
  wcbuf[path.size()+1] = 0;

  SHFILEOPSTRUCTW fop = {};
  fop.pFrom = wcbuf;
  fop.wFunc = FO_DELETE;
  fop.fFlags = FOF_NO_UI|FOF_ALLOWUNDO;

  return SHFileOperationW(&fop);
}


/// \brief check file permission
///
/// Checks whether the current process application have the specified
/// permissions on the given file or folder.
///
/// The possibles values for permission mask are the following:
/// - \c FILE_GENERIC_READ
/// - \c FILE_GENERIC_WRITE
/// - \c FILE_GENERIC_EXECUTE
/// - \c FILE_ALL_ACCESS
///
/// \param[in]  path    : Path to file or folder to check permission on.
/// \param[in]  reqMask : Mask for requested permission.
///
/// \return True if requested permission are allowed, false otherwise
///
inline static bool __checkAccess(const wstring& path, DWORD reqMask)
{
  std::wcout << L"__checkAccess start for \"" << path <<"\" \n";
  // Thanks to this article for giving some clues :
  // http://blog.aaronballman.com/2011/08/how-to-check-access-rights/

  // STEP 1 - creates an access "generic mapping", which is the access
  // we want to check the reqMask against
  GENERIC_MAPPING gm;                       //< Mask Map of Access to check for
  gm.GenericRead =    FILE_GENERIC_READ;    //< check for FILE_GENERIC_READ
  gm.GenericWrite =   FILE_GENERIC_WRITE;   //< check for FILE_GENERIC_WRITE
  gm.GenericExecute = FILE_GENERIC_EXECUTE; //< check for FILE_GENERIC_EXECUTE
  gm.GenericAll =     FILE_ALL_ACCESS;      //< check for FILE_ALL_ACCESS

  // STEP 2 - retrieve the "security descriptor" (i.e owner, group, access
  // rights, etc. ) of the specified file or folder.
  SECURITY_DESCRIPTOR* sd;
  DWORD sdSize;
  // here is the mask for file permission informations we want to retrieve it
  // seem to be the minimum required for an access check request
  DWORD sdMask =  OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
                | DACL_SECURITY_INFORMATION;
  // first call to get required SECURITY_DESCRIPTOR size
  GetFileSecurityW(path.c_str(), sdMask, nullptr, 0, &sdSize);
  // allocate new SECURITY_DESCRIPTOR of the proper size
  sd = reinterpret_cast<SECURITY_DESCRIPTOR*>(new char[sdSize+1]);
  // second call to get SECURITY_DESCRIPTOR data
  if(!GetFileSecurityW(path.c_str(), sdMask, sd, sdSize, &sdSize)) {
    delete sd;
    DWORD err = GetLastError();
    std::wcout << L"GetFileSecurityW failed with code : " << err << L"\n";
    return false;
  }

  // STEP 3 - creates a "security token" of the current application process
  //to be checked against the file or folder "security descriptor"
  DWORD daMask =  TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE
                | STANDARD_RIGHTS_READ;
  HANDLE hTokenProc = nullptr;
  if(!OpenProcessToken(GetCurrentProcess(), daMask, &hTokenProc)) {
    delete sd;
    DWORD err = GetLastError();
    std::wcout << L"OpenProcessToken failed with code : " << err << L"\n";
    return false;
  }
  // the current process token is a "primary" one (don't know what that mean)
  // so we need to duplicate it to transform it into a standard "user" token by
  // impersonate it...
  HANDLE hTokenUser = nullptr;
  if(!DuplicateToken(hTokenProc, SecurityImpersonation, &hTokenUser)) {
    CloseHandle(hTokenProc); delete sd;
    DWORD err = GetLastError();
    std::wcout << L"DuplicateToken failed with code : " << err << L"\n";
    return false;
  }

  // STEP 4 - Finally check if "security token" have the requested
  // "generic mapping" access to the "security descriptor" of the specified file
  // or folder
  PRIVILEGE_SET ps;
  memset(&ps, 0, sizeof(PRIVILEGE_SET)); // to be SURE well zeroing
  DWORD psSize = sizeof(PRIVILEGE_SET);
  DWORD allowed = 0;      //< mask of allowed access
  BOOL  status = false;   //< access status according supplied GENERIC_MAPPING
  if(!AccessCheck(sd, hTokenUser, reqMask, &gm, &ps, &psSize, &allowed, &status)) {
    DWORD err = GetLastError();
    std::wcout << L"AccessCheck failed with code : " << err << L"\n";
  }

  CloseHandle(hTokenProc);
  CloseHandle(hTokenUser);
  delete sd;

  std::wcout << L"__checkAccess for \"" << path << L" returned " << (int)status << L"\n";

  return status;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_checkAccessRead(const wstring& path)
{
  return __checkAccess(path, FILE_GENERIC_READ);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_checkAccessWrite(const wstring& path)
{
  return __checkAccess(path, FILE_GENERIC_WRITE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_checkAccessReadWrite(const wstring& path)
{
  return __checkAccess(path, FILE_GENERIC_READ|FILE_GENERIC_WRITE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dialogBoxErr(HWND hWnd, const wstring& header, const wstring& detail)
{
  wstring message = header + L"\n\n"; message += detail;
  MessageBoxW(hWnd, message.c_str(), OMM_APP_NAME, MB_OK|MB_ICONERROR);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dialogBoxWarn(HWND hWnd, const wstring& header, const wstring& detail)
{
  wstring message = header + L"\n\n"; message += detail;
  MessageBoxW(hWnd, message.c_str(), OMM_APP_NAME, MB_OK|MB_ICONWARNING);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dialogBoxInfo(HWND hWnd, const wstring& header, const wstring& detail)
{
  wstring message = header + L"\n\n"; message += detail;
  MessageBoxW(hWnd, message.c_str(), OMM_APP_NAME, MB_OK|MB_ICONINFORMATION);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dialogBoxQuerry(HWND hWnd, const wstring& header, const wstring& detail)
{
  wstring message = header + L"\n\n"; message += detail;
  return (IDYES == MessageBoxW(hWnd, message.c_str(), OMM_APP_NAME, MB_YESNO|MB_ICONQUESTION));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dialogBoxQuerryWarn(HWND hWnd, const wstring& header, const wstring& detail)
{
  wstring message = header + L"\n\n"; message += detail;
  return (IDYES == MessageBoxW(hWnd, message.c_str(), OMM_APP_NAME, MB_YESNO|MB_ICONWARNING));
}


/// \brief Browse dialog callback.
///
/// Callback function for folder browse dialog window. It is used like a
/// WindowProc to select a default start folder when dialog window is
/// initialized.
///
INT CALLBACK __dialogBrowseDir_Proc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  if(uMsg == BFFM_INITIALIZED) { //< Brother dialog windows is initialized
    SendMessage((HWND)hwnd, BFFM_SETSELECTION, false, lpData);  //< set the selected folder
  }
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dialogBrowseDir(wchar_t* result, HWND hWnd, const wchar_t* title, const wchar_t* start, bool captive)
{
  BROWSEINFOW br = {};
  br.hwndOwner = hWnd;
  br.lpszTitle = title;
  br.ulFlags = BIF_USENEWUI|BIF_RETURNONLYFSDIRS;

  if(captive) {
    // this is the standard easy way to use SHBrowseForFolderW, this will
    // set the "start" path as the root of browsing, so the user cannot go up
    // to parent folder
    if(start != nullptr) {
      LPITEMIDLIST pIdl = nullptr;
      SHParseDisplayName(start, nullptr, &pIdl, 0, nullptr); //< convert path string to LPITEMIDLIST
      br.pidlRoot = pIdl;
    }
  } else {
      // this is the advanced way to use SHBrowseForFolderW, here we use a
      // callback function to handle the dialog window initialization, the "start"
      // path object will be passed as lParam to the callback with the
      // BFFM_INITIALIZED message.
    if(start != nullptr) {
      LPITEMIDLIST pIdl = nullptr;
      SHParseDisplayName(start, nullptr, &pIdl, 0, nullptr); //< convert path string to LPITEMIDLIST
      br.lpfn = __dialogBrowseDir_Proc;
      br.lParam = (LPARAM)pIdl;
    }
  }

  LPITEMIDLIST pIdl;
  if((pIdl = SHBrowseForFolderW(&br)) != nullptr) {
    if(SHGetPathFromIDListW(pIdl, result)) {
      return true;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dialogOpenFile(wchar_t* result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const wchar_t* start)
{
  OPENFILENAMEW ofn = {};
  ofn.lStructSize = sizeof(OPENFILENAMEW);

  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = filter; //L"Mod archive (*.zip)\0*.ZIP;\0";

  ofn.lpstrFile = result;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = OMM_MAX_PATH;

  ofn.lpstrInitialDir = start;

  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER|OFN_NONETWORKBUTTON|OFN_NOTESTFILECREATE;

  return GetOpenFileNameW(&ofn);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dialogSaveFile(wchar_t* result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const wchar_t* start)
{
  OPENFILENAMEW ofn = {};
  ofn.lStructSize = sizeof(OPENFILENAMEW);

  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = filter;

  ofn.lpstrFile = result;
  ofn.nMaxFile = OMM_MAX_PATH;

  ofn.lpstrInitialDir = start;

  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER|OFN_NONETWORKBUTTON|OFN_NOTESTFILECREATE;

  return GetSaveFileNameW(&ofn);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
string Om_loadPlainText(const wstring& path)
{
  string result;

  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return result;

  DWORD rb;
  char buff[MBS_SIZE];

  while(ReadFile(hFile, buff, 2048, &rb, nullptr)) {

    if(rb == 0)
      break;

    buff[rb] = '\0';
    result.append(buff);
  }

  CloseHandle(hFile);

  return result;
}

/// \brief Write a BMP file.
///
/// Create a new BMP file at the specified location from the specified
/// pixels data.
///
/// \param[in]  path    : File full path and name to save.
/// \param[in]  w       : Image width in pixel.
/// \param[in]  h       : Image depth in pixel.
/// \param[in]  d       : Image bits per pixel, either 24 or 32 are supported.
/// \param[in]  pixels  : Image pixels data.
///
/// \return True if succeed, false if write error occurred.
///
static inline bool __writeBmp(const char* path, unsigned w, unsigned h, unsigned d, const unsigned char* pixels)
{
  // Since Microsoft is not able to provide a correct function to save
  // a BMP or PNG file ast 32 bits depth with alpha, I have to implement it
  // myself. By chance, I done this time ago for another project...

  // Open for writing
  FILE* fp;
  fp = fopen(path, "wb");
  if(!fp) {
    return false;
  }

  // BMP header...
  typedef struct {
     unsigned size;        // size of the whole .bmp file
     short    reserved1;   // must be 0
     short    reserved2;   // must be 0
     unsigned offbits;
  } BMP_FILE_HEADER;

  typedef struct {
     unsigned size;            // size of the structure
     int      width;           // image width
     int      height;          // image height
     short    planes;          // bitplanes
     short    bpp;             // resolution
     unsigned compression;     // compression
     unsigned sizeimage;       // size of the image
     int      xppm;            // pixels per meter X
     int      yppm;            // pixels per meter Y
     unsigned clrused;         // colors used
     unsigned clrimportant;    // important colors

  } BMP_INFO_HEADER;

  // Here we create BMP data
  unsigned pix_size = w * h * (d / 8);

  // BMP signature ASCII "BM"
  short type = 0x4d42; //< BMP signature

  BMP_FILE_HEADER bmpf;
  bmpf.size = pix_size + sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER) + 2;
  bmpf.reserved1 = 0;
  bmpf.reserved2 = 0;
  bmpf.offbits = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER) + 2;

  BMP_INFO_HEADER bmpi;
  bmpi.size = sizeof(BMP_INFO_HEADER);
  bmpi.width = w;
  bmpi.height = h;
  bmpi.planes = 1;
  bmpi.bpp = d;
  bmpi.compression = 0;
  bmpi.sizeimage = pix_size;
  bmpi.xppm = 0x0ec4;
  bmpi.yppm = 0x0ec4;
  bmpi.clrused = 0;
  bmpi.clrimportant = 0;

  if(fwrite(&type, 1, 2, fp) != 2) {
    fclose(fp);
    return false;
  }
  if(fwrite(&bmpf, 1, sizeof(BMP_FILE_HEADER), fp) != sizeof(BMP_FILE_HEADER)) {
    fclose(fp);
    return false;
  }
  if(fwrite(&bmpi, 1, sizeof(BMP_INFO_HEADER), fp) != sizeof(BMP_INFO_HEADER)) {
    fclose(fp);
    return false;
  }
  if(fwrite(pixels, 1, pix_size, fp) != pix_size) {
    fclose(fp);
    return false;
  }

  fclose(fp);
  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_saveBitmap(const wstring& path, unsigned w, unsigned h, unsigned d, const unsigned char* pixels)
{
  return __writeBmp(Om_toMbString(path).c_str(), w, h, d, pixels);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_saveBitmap(const wstring& path, void* hBmp)
{
  BITMAP bitmap;
  GetObject((HBITMAP)hBmp, sizeof(BITMAP), (LPVOID)&bitmap);
  return __writeBmp(Om_toMbString(path).c_str(), bitmap.bmWidth, bitmap.bmHeight, bitmap.bmBitsPixel, (unsigned char*)bitmap.bmBits);
}



/// \brief Microsoft image format CLSID
///
/// Microsoft image format Class ID objects local definition, not used here but
/// can be useful later.
///
const CLSID _MMbmp_Clsid = {0x557cf400, 0x1a04, 0x11d3, {0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
const CLSID _MMjpg_Clsid = {0x557cf401, 0x1a04, 0x11d3, {0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
const CLSID _MMgif_Clsid = {0x557cf402, 0x1a04, 0x11d3, {0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
const CLSID _MMtif_Clsid = {0x557cf405, 0x1a04, 0x11d3, {0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
const CLSID _MMpng_Clsid = {0x557cf406, 0x1a04, 0x11d3, {0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_loadBitmap(const wstring& path, unsigned width, unsigned height, bool aspect)
{
  HBITMAP result = nullptr;

  if(!Om_isFile(path))
    return result;

  // Init GDI+
  Gdiplus::GdiplusStartupInput gdiSi;
  ULONG_PTR gdiTok;
  GdiplusStartup(&gdiTok, &gdiSi, nullptr);

  // Resize image
  if(width > 0 && height > 0) {

    Gdiplus::Bitmap* dst_bitmap;
    Gdiplus::Bitmap* src_bitmap = new Gdiplus::Bitmap(path.c_str());

    if(aspect) { //< keep aspect ratio
      int w = src_bitmap->GetWidth();
      int h = src_bitmap->GetHeight();
      float a = (float)w/h;
      if(a > (float)width/height) {
        dst_bitmap = (Gdiplus::Bitmap*)src_bitmap->GetThumbnailImage(width, height*((float)h/w), nullptr, nullptr);
      } else {
        dst_bitmap = (Gdiplus::Bitmap*)src_bitmap->GetThumbnailImage(width*a, height, nullptr, nullptr);
      }
    } else {
      dst_bitmap = (Gdiplus::Bitmap*)src_bitmap->GetThumbnailImage(width, height, nullptr, nullptr);
    }

    // GetHBITMAP request a "background color" for transparency...
    // WHY, for the love of god, it don't use the RGB components of the image
    // itself, instead of creating such ugly alpha artifacts ?!
    dst_bitmap->GetHBITMAP(Gdiplus::Color(0,0,0,0), &result);

    //delete bitmap;
    delete dst_bitmap;
    delete src_bitmap;

  } else { //< image as original size

    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(path.c_str());
    bitmap->GetHBITMAP(Gdiplus::Color(0,0,0,0), &result);
    delete bitmap;
  }

  // Stop GDI+
  Gdiplus::GdiplusShutdown(gdiTok);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_loadBitmap(const void* data, size_t size, unsigned width, unsigned height, bool aspect)
{
  HBITMAP result = nullptr;

  // The 6 lines of code below is a typical example of why Microsoft developers
  // should have consult a psychiatrist.
  //
  // How the heck all this garbage can be required to simply load a file from
  // a buffer ? This is INSANE...

  HGLOBAL hMem = GlobalAlloc(GMEM_FIXED, size); //< Le memory allocation object
  void* pmem = GlobalLock(hMem);                //< Le pointer to locked memory area
  memcpy(pmem, data, size);                     //< Le usual C memcpy
  GlobalUnlock(hMem);                           //< Le memory unlock
  IStream* IStr;                                //< Le memory stream object
  CreateStreamOnHGlobal(hMem, true, &IStr);     //< Le function to create a memory stream

  // Init GDI+
  Gdiplus::GdiplusStartupInput gdiSi;
  ULONG_PTR gdiTok;
  GdiplusStartup(&gdiTok, &gdiSi, nullptr);

  // Resize image
  if(width > 0 && height > 0) {

    Gdiplus::Bitmap* dst_bitmap;
    Gdiplus::Bitmap* src_bitmap = new Gdiplus::Bitmap(IStr); //< Le usage of memory stream

    if(aspect) { // keep aspect ratio
      int w = src_bitmap->GetWidth();
      int h = src_bitmap->GetHeight();
      float a = (float)w/h;
      if(a > (float)width/height) {
        dst_bitmap = (Gdiplus::Bitmap*)src_bitmap->GetThumbnailImage(width, height*((float)h/w), nullptr, nullptr);
      } else {
        dst_bitmap = (Gdiplus::Bitmap*)src_bitmap->GetThumbnailImage(width*a, height, nullptr, nullptr);
      }
    } else {
      dst_bitmap = (Gdiplus::Bitmap*)src_bitmap->GetThumbnailImage(width, height, nullptr, nullptr);
    }

    // GetHBITMAP request a "background color" for transparency...
    // WHY, for the love of god, it don't use the RGB components of the image
    // itself, instead of creating such ugly alpha artifacts ?!
    dst_bitmap->GetHBITMAP(Gdiplus::Color(0,0,0,0), &result);

    //delete bitmap;
    delete dst_bitmap;
    delete src_bitmap;

  } else { // image as original size

    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(IStr); //< Le usage of memory stream
    bitmap->GetHBITMAP(Gdiplus::Color(0,0,0,0), &result);
    delete bitmap;
  }

  GlobalFree(hMem);

  // Stop GDI+
  Gdiplus::GdiplusShutdown(gdiTok);

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_getBitmapThumbnail(HBITMAP hBmp, unsigned width, unsigned height, bool aspect)
{
  HBITMAP result = nullptr;

  // Init GDI+
  Gdiplus::GdiplusStartupInput gdiSi;
  ULONG_PTR gdiTok;
  GdiplusStartup(&gdiTok, &gdiSi, nullptr);

  HBITMAP hDib = static_cast<HBITMAP>(CopyImage(hBmp,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION));
  Gdiplus::Bitmap* dst_bitmap;
  Gdiplus::Bitmap* src_bitmap = new Gdiplus::Bitmap(hDib, nullptr);
  DeleteObject(hDib);

  if(aspect) { // keep aspect ratio
    int w = src_bitmap->GetWidth();
    int h = src_bitmap->GetHeight();
    float a = (float)w/h;
    if(a > (float)width/height) {
      dst_bitmap = (Gdiplus::Bitmap*)src_bitmap->GetThumbnailImage(width, height*((float)h/w), nullptr, nullptr);
    } else {
      dst_bitmap = (Gdiplus::Bitmap*)src_bitmap->GetThumbnailImage(width*a, height, nullptr, nullptr);
    }
  } else {
    dst_bitmap = (Gdiplus::Bitmap*)src_bitmap->GetThumbnailImage(width, height, nullptr, nullptr);
  }

  // GetHBITMAP request a "background color" for transparency...
  // WHY, for the love of god, it don't use the RGB components of the image
  // itself, instead of creating such ugly alpha artifacts ?!
  dst_bitmap->GetHBITMAP(Gdiplus::Color(0,0,0,0), &result);

  //delete bitmap;
  delete dst_bitmap;
  delete src_bitmap;

  // Stop GDI+
  Gdiplus::GdiplusShutdown(gdiTok);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void* Om_loadShellIcon(unsigned id, bool large)
{
  SHSTOCKICONINFO sii = {};
  sii.cbSize = sizeof(SHSTOCKICONINFO);

  SHGetStockIconInfo(static_cast<SHSTOCKICONID>(id),
                    (large) ? SHGSI_ICON|SHGSI_LARGEICON : SHGSI_ICON|SHGSI_SMALLICON,
                    &sii);
  return sii.hIcon;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void* Om_loadShellBitmap(unsigned id, bool large)
{
  SHSTOCKICONINFO sii = {};
  sii.cbSize = sizeof(SHSTOCKICONINFO);

  SHGetStockIconInfo(static_cast<SHSTOCKICONID>(id),
                    (large) ? SHGSI_ICON|SHGSI_LARGEICON : SHGSI_ICON|SHGSI_SMALLICON,
                    &sii);

  ICONINFO ici = {};
  GetIconInfo(sii.hIcon, &ici);
  return CopyImage(ici.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_COPYDELETEORG);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_getErrorStr(int code) {

  wchar_t wcbuf[256];
  swprintf(wcbuf, 256, L"%x", code);

  wstring ret = L"(0x"; ret += wcbuf; ret += L") ";

  switch(code)
  {
  case ERROR_FILE_NOT_FOUND:
    ret += L"FILE_NOT_FOUND"; break;
  case ERROR_PATH_NOT_FOUND:
    ret += L"PATH_NOT_FOUND"; break;
  case ERROR_TOO_MANY_OPEN_FILES:
    ret += L"TOO_MANY_OPEN_FILES"; break;
  case ERROR_ACCESS_DENIED:
    ret += L"ACCESS_DENIED"; break;
  case ERROR_ARENA_TRASHED:
    ret += L"ARENA_TRASHED"; break;
  case ERROR_NOT_ENOUGH_MEMORY:
    ret += L"NOT_ENOUGH_MEMORY"; break;
  case ERROR_INVALID_BLOCK:
    ret += L"INVALID_BLOCK"; break;
  case ERROR_INVALID_ACCESS:
    ret += L"INVALID_ACCESS"; break;
  case ERROR_INVALID_DATA:
    ret += L"INVALID_DATA"; break;
  case ERROR_OUTOFMEMORY:
    ret += L"OUTOFMEMORY"; break;
  case ERROR_INVALID_DRIVE:
    ret += L"INVALID_DRIVE"; break;
  case ERROR_CURRENT_DIRECTORY:
    ret += L"CURRENT_DIRECTORY"; break;
  case ERROR_NOT_SAME_DEVICE:
    ret += L"NOT_SAME_DEVICE"; break;
  case ERROR_WRITE_PROTECT:
    ret += L"WRITE_PROTECT"; break;
  case ERROR_CRC:
    ret += L"ERROR_CRC"; break;
  case ERROR_SEEK:
    ret += L"ERROR_SEEK"; break;
  case ERROR_WRITE_FAULT:
    ret += L"WRITE_FAULT"; break;
  case ERROR_READ_FAULT:
    ret += L"READ_FAULT"; break;
  case ERROR_SHARING_VIOLATION:
    ret += L"SHARING_VIOLATION"; break;
  case ERROR_LOCK_VIOLATION:
    ret += L"LOCK_VIOLATION"; break;
  case ERROR_WRONG_DISK:
    ret += L"WRONG_DISK"; break;
  case ERROR_HANDLE_DISK_FULL:
    ret += L"HANDLE_DISK_FULL"; break;
  case ERROR_FILE_EXISTS:
    ret += L"FILE_EXISTS"; break;
  case ERROR_DRIVE_LOCKED:
    ret += L"DRIVE_LOCKED"; break;
  case ERROR_OPEN_FAILED:
    ret += L"OPEN_FAILED"; break;
  case ERROR_BUFFER_OVERFLOW:
    ret += L"BUFFER_OVERFLOW"; break;
  case ERROR_DISK_FULL:
    ret += L"DISK_FULL"; break;
  case ERROR_INVALID_NAME:
    ret += L"INVALID_NAME"; break;
  case ERROR_DIR_NOT_EMPTY:
    ret += L"DIR_NOT_EMPTY"; break;
  case ERROR_ALREADY_EXISTS:
    ret += L"ALREADY_EXISTS"; break;
  }

  return ret;
}
