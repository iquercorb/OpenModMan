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
#include <random>
#include <ctime>

#include "OmBaseWin.h"        //< WinAPI

#include "xxhash/xxh3.h"
#include "md5/md5.h"

static std::mt19937                             __rnd_generator(time(0));
static std::uniform_int_distribution<uint8_t>   __rnd_uint8dist(0, 255);

#define READ_BUF_SIZE 262144
static uint8_t __read_buf[READ_BUF_SIZE];

/// \brief Swap bytes
///
/// Swap bytes order in 32 bits value, to convert endianes.
///
/// \param[in]  num : input 32 value to swap bytes.
///
/// \return Swapped bytes version of the input value
///
inline static uint32_t __bytes_swap(uint32_t num)
{
  return  ((num >> 24) & 0xff) |
          ((num <<  8) & 0xff0000) |
          ((num >>  8) & 0xff00) |
          ((num << 24) & 0xff000000);
}

/// \brief Hexadecimal digits
///
/// Static translation string to convert integer value to hexadecimal digit.
///
static const wchar_t __hex_digit[] = L"0123456789abcdef";

/// \brief Little-endian bytes to hexadecimal
///
/// Translate the given data buffers to its hexadecimal
/// string representation in little-endian way.
///
/// \param[out] hex   : String to be set as hexadecimal representation.
/// \param[in]  data  : Data to translate.
/// \param[in]  size  : Data size in bytes.
///
static inline void __bytes_to_hex_be(OmWString* dest, const uint8_t* data, size_t size)
{
  dest->clear();
  uint8_t c;
  size_t i = size;
  while(i--) {
    c = data[i];
    dest->push_back(__hex_digit[(c >> 4) & 0x0F]);
    dest->push_back(__hex_digit[(c)      & 0x0F]);
  }
}

/// \brief big-endian bytes to hexadecimal
///
/// Translate the given data buffers to its hexadecimal
/// string representation in big-endian way.
///
/// \param[out] dest  : String to be set as hexadecimal representation.
/// \param[in]  data  : Data to translate.
/// \param[in]  size  : Data size in bytes.
///
static inline void __bytes_to_hex_le(OmWString* dest, const uint8_t* data, size_t size)
{
  dest->clear();
  for(size_t i = 0; i < size; ++i) {
    uint8_t c = data[i];
    dest->push_back(__hex_digit[(c >> 4) & 0x0F]);
    dest->push_back(__hex_digit[(c)      & 0x0F]);
  }
}

/// \brief big-endian hexadecimal to uint64
///
/// Translate the given hexadecimal number string to 64 bits
/// unsigned integer value.
///
/// \param[in] str  : Hexadecimal number null-terminated string to be parsed.
///
/// \return 64 bits unsigned integer value
///
static inline uint64_t __hex_to_uint64(const wchar_t* str)
{
  uint64_t u64 = 0;

  unsigned char c;

  while(*str != '\0') {

    c = *str++;

    if(c >= '0' && c <= '9') {
      c -= '0';
    } else if(c >= 'a' && c <= 'z') {
      c -= ('a' - 10);
    } else if(c >= 'A' && c <= 'Z') {
      c -= ('A' - 10);
    } else {
      break;
    }

    u64 *= 16;
    u64 += c;
  }

  return u64;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_bytesToStrBe(OmWString* dest, const uint8_t* data, size_t size)
{
  __bytes_to_hex_be(dest, data, size);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_bytesToStrLe(OmWString* dest, const uint8_t* data, size_t size)
{
  __bytes_to_hex_le(dest, data, size);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t Om_strToUint64(const OmWString& str) {
  return __hex_to_uint64(str.data());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t Om_strToUint64(const wchar_t* str) {
  return __hex_to_uint64(str);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_uint64ToStr(uint64_t num)
{
  OmWString dest;
  uint8_t c, b = 64;
  while(b) {
    b -= 8;
    c = (uint8_t)((num >> b) & 0xFF);
    dest.push_back(__hex_digit[(c >> 4) & 0x0F]);
    dest.push_back(__hex_digit[(c)      & 0x0F]);
  }
  return dest;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_uint64ToStr(OmWString* dest, uint64_t num)
{
  dest->clear();
  uint8_t c, b = 64;
  while(b) {
    b -= 8;
    c = (uint8_t)((num >> b) & 0xFF);
    dest->push_back(__hex_digit[(c >> 4) & 0x0F]);
    dest->push_back(__hex_digit[(c)      & 0x0F]);
  }
}

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
uint64_t Om_getCRC64(const OmWString& str)
{
  return __CRC64(0, (unsigned char*)str.c_str(), str.size()*sizeof(wchar_t));
}

/// \brief Generate file XXHash3 digest (checksum)
///
/// Generate file digest (checksum) string using XXHash3 64 bits digest
/// algorithm
///
/// \param[out] xxh   : Pointer to uint64_t that receive hash value.
/// \param[in]  path  : Path to file to generate digest from.
///
/// \return True if operation succeed, false if file open error.
///
static inline bool __XXHash3_file_digest(uint64_t* xxh, const OmWString& path)
{
  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return false;

  DWORD rb;

  XXH3_state_t xxhst;
  XXH3_64bits_reset(&xxhst);

  while(ReadFile(hFile, __read_buf, READ_BUF_SIZE, &rb, nullptr)) {

    if(rb == 0)
      break;

    XXH3_64bits_update(&xxhst, __read_buf, rb);
  }

  CloseHandle(hFile);

  *xxh = XXH3_64bits_digest(&xxhst);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t Om_getXXHash3(const void* data, size_t size)
{
  return XXH3_64bits(data, size);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t Om_getXXHash3(const OmWString& str)
{
  return XXH3_64bits(str.data(), str.size() * sizeof(wchar_t));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_getXXHdigest(uint64_t* xxh, const OmWString& path)
{
  return __XXHash3_file_digest(xxh, path);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_getXXHsum(const OmWString& path)
{
  uint64_t xxh;

  OmWString str;

  if(__XXHash3_file_digest(&xxh, path)) {
    __bytes_to_hex_be(&str, reinterpret_cast<const uint8_t*>(&xxh), 8);
  }

  return str;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_getXXHsum(OmWString* pstr, const OmWString& path)
{
  uint64_t xxh;

  if(!__XXHash3_file_digest(&xxh, path))
    return false;

  __bytes_to_hex_be(pstr, reinterpret_cast<const uint8_t*>(&xxh), 8);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_cmpXXHsum(const OmWString& path, const OmWString& str)
{
  uint64_t xxh_l, xxh_r;

  if(!__XXHash3_file_digest(&xxh_l, path))
    return false;

  xxh_r = __hex_to_uint64(str.data());

  return (xxh_l == xxh_r);
}


/// \brief Generate file MD5 digest (checksum)
///
/// Generate file digest (checksum) using MD5 digest algorithm
///
/// \param[out] md5   : 16 bytes buffer that receive digest result.
/// \param[in]  path  : Path to file to generate digest from.
///
/// \return True if operation succeed, false if file open error.
///
static inline bool __MD5_file_digest(uint8_t* md5, const OmWString& path)
{
  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return false;

  DWORD rb;

  MD5_CTX md5ct;
  MD5_Init(&md5ct);

  while(ReadFile(hFile, __read_buf, READ_BUF_SIZE, &rb, nullptr)) {

    if(rb == 0)
      break;

    MD5_Update(&md5ct, __read_buf, rb);
  }

  CloseHandle(hFile);

  MD5_Final(md5, &md5ct);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_getMD5digest(uint8_t* md5, const OmWString& path)
{
  return __MD5_file_digest(md5, path);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_getMD5sum(const OmWString& path)
{
  uint8_t md5[16] = {};

  OmWString str;

  if(__MD5_file_digest(md5, path)) {
    __bytes_to_hex_le(&str, md5, 16);
  }

  return str;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_getMD5sum(OmWString* pstr, const OmWString& path)
{
  uint8_t md5[16] = {};

  if(!__MD5_file_digest(md5, path))
    return false;

  __bytes_to_hex_le(pstr, md5, 16);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_cmpMD5sum(const OmWString& path, const OmWString& str)
{
  uint8_t md5[16] = {};

  if(!__MD5_file_digest(md5, path))
    return false;

  OmWString ctrl;

  __bytes_to_hex_le(&ctrl, md5, 16);

  return (str == ctrl);
}




///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_getRandBytes(uint8_t* dest, size_t size)
{
  for(size_t i = 0; i < size; ++i) {
    dest[i] = __rnd_uint8dist(__rnd_generator);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_genUUID()
{
  uint8_t uuid[16];

  // Generate random bytes sequence
  for(unsigned i = 0; i < 16; ++i) {
    uuid[i] = __rnd_uint8dist(__rnd_generator);
  }

  uuid[6] = (uuid[6] & 0x0F) | 0x40; //< Set version 4
  uuid[8] = (uuid[8] & 0x3F) | 0x80; //< Set DCE variant

  // Create formated UUID string from random sequence
  OmWString str(L"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");

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
