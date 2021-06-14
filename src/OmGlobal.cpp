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
#include <regex>

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
/// \param[in]  data  : Data size in bytes.
///
static inline void __bytes_to_hex_be(wstring& hex, const uint8_t* data, size_t size)
{
  hex.clear();
  uint8_t c;
  size_t i = size;
  while(i--) {
    c = data[i];
    hex.push_back(__hex_digit[(c >> 4) & 0x0F]);
    hex.push_back(__hex_digit[(c)      & 0x0F]);
  }
}

/// \brief big-endian bytes to hexadecimal
///
/// Translate the given data buffers to its hexadecimal
/// string representation in big-endian way.
///
/// \param[out] hex   : String to be set as hexadecimal representation.
/// \param[in]  data  : Data to translate.
/// \param[in]  data  : Data size in bytes.
///
static inline void __bytes_to_hex_le(wstring& hex, const uint8_t* data, size_t size)
{
  hex.clear();
  uint8_t c;
  for(size_t i = 0; i < size; ++i) {
    c = data[i];
    hex.push_back(__hex_digit[(c >> 4) & 0x0F]);
    hex.push_back(__hex_digit[(c)      & 0x0F]);
  }
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


/// \brief Generate checksum
///
/// Generate checksum string using XXHash3 128 bits algorithm
///
/// \param[out] str   : String to be set as checksum.
/// \param[in]  data  : Data to create checksum from.
/// \param[in]  size  : Data size in bytes.
///
static inline void __XXHash3_gen_checksum(wstring& str, const void* data, size_t size)
{
  wchar_t buff[17];
  uint64_t sum = static_cast<uint64_t>(XXH3_64bits(data, size));
  swprintf(buff, 17, L"%016llx", sum);
  str = buff;
}

static inline bool __XXHash3_cmp_checksum(const wstring& hex, const void* data, size_t size)
{
  uint64_t hash1 = wcstoull(hex.c_str(), nullptr, 16);
  uint64_t hahs2 = static_cast<uint64_t>(XXH3_64bits(data, size));
  return (hash1 == hahs2);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_getChecksum(const wstring& path)
{
  wstring hex;

  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return hex;

  size_t size = GetFileSize(hFile, nullptr);

  uint8_t* data = reinterpret_cast<uint8_t*>(Om_alloc(size));
  if(!data) return hex;

  DWORD rb;
  ReadFile(hFile, data, size, &rb, nullptr);

  CloseHandle(hFile);

  __XXHash3_gen_checksum(hex, data, size);

  Om_free(data);

  return hex;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_getChecksum(wstring& hex, const wstring& path)
{
  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return false;

  size_t size = GetFileSize(hFile, nullptr);

  uint8_t* data = reinterpret_cast<uint8_t*>(Om_alloc(size));
  if(!data) return false;

  DWORD rb;
  bool result = ReadFile(hFile, data, size, &rb, nullptr);

  CloseHandle(hFile);

  __XXHash3_gen_checksum(hex, data, size);

  Om_free(data);

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_cmpChecksum(const wstring& path, const wstring& hex)
{
  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return false;

  size_t size = GetFileSize(hFile, nullptr);

  uint8_t* data = reinterpret_cast<uint8_t*>(Om_alloc(size));
  if(!data) return false;

  DWORD rb;
  ReadFile(hFile, data, size, &rb, nullptr);

  CloseHandle(hFile);

  bool result = __XXHash3_cmp_checksum(hex, data, size);

  Om_free(data);

  return result;
}


static const wchar_t __b64_enc_table[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const uint8_t __b64_dec_table[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,62, 0, 0, 0,63,52,53,54,55,56,57,58,59,60,61, 0, 0, 0, 0, 0, 0,
                                          0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25, 0, 0, 0, 0, 0,
                                          0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51, 0, 0, 0, 0, 0};

/// \brief Base64 encode.
///
/// Encode given data to Base64 string.
///
/// \param[out] out_b64 : Output Base64 encoding result string.
/// \param[in]  in_data : Input data to encode.
/// \param[in]  in_size : Input size of data in bytes.
///
static inline void __base64_encode(wstring& out_b64, const uint8_t* in_data, size_t in_size)
{
  // compute string size now
  size_t size = 4 * ((in_size + 2) / 3);

  // reserve buffer for encoded data
  out_b64.clear();
  out_b64.reserve(size);

  uint8_t b[3];
  uint32_t t;

  // main block, per triplets
  for(unsigned i = 0; i < in_size; ) {
    b[0] = (i < in_size) ? in_data[i++] : 0;
    b[1] = (i < in_size) ? in_data[i++] : 0;
    b[2] = (i < in_size) ? in_data[i++] : 0;
    t = (b[0] << 0x10) + (b[1] << 0x08) + b[2];
    out_b64.push_back(__b64_enc_table[0x3F & (t >> 18)]);
    out_b64.push_back(__b64_enc_table[0x3F & (t >> 12)]);
    out_b64.push_back(__b64_enc_table[0x3F & (t >>  6)]);
    out_b64.push_back(__b64_enc_table[0x3F & (t)]);
  }

  unsigned r = in_size % 3; //< remaining bytes after per-triplet division
  if(r > 0) {
    for(unsigned i = 0; i < 3 - r; ++i)
      out_b64[(size - 1) - i] = L'=';
  }
}


/// \brief Base64 decode.
///
/// Decode Base64 string to buffer.
///
/// \param[out] out_size  : Output size of decoded data in bytes.
/// \param[in]  in_b64    : Input Base64 string to decode.
///
/// \return Pointer to decoded data.
///
static inline uint8_t* __base64_decode(size_t* out_size, const wstring& in_b64)
{
  // check whether input data is valid
  if(in_b64.size() % 4 != 0)
    return nullptr;

  // compute output data size now
  size_t size = (in_b64.size() / 4) * 3;
  if(in_b64[in_b64.size() - 1] == '=') size--;
  if(in_b64[in_b64.size() - 2] == '=') size--;

  // allocate output data buffer
  uint8_t* data = reinterpret_cast<uint8_t*>(Om_alloc(size));
  if(!data) return nullptr;

  // decode data
  uint32_t t;
  uint8_t s[4];

  for(unsigned i = 0, j = 0; i < in_b64.size(); ) {
    s[0] = (in_b64[i] == L'=')? 0 : __b64_dec_table[static_cast<uint8_t>(in_b64[i])]; i++;
    s[1] = (in_b64[i] == L'=')? 0 : __b64_dec_table[static_cast<uint8_t>(in_b64[i])]; i++;
    s[2] = (in_b64[i] == L'=')? 0 : __b64_dec_table[static_cast<uint8_t>(in_b64[i])]; i++;
    s[3] = (in_b64[i] == L'=')? 0 : __b64_dec_table[static_cast<uint8_t>(in_b64[i])]; i++;
    t = (s[0] << 18) + (s[1] << 12) + (s[2] << 6) + s[3];
    if(j < size) data[j++] = 0xFF & (t >> 16);
    if(j < size) data[j++] = 0xFF & (t >>  8);
    if(j < size) data[j++] = 0xFF & (t);
  }

  (*out_size) = size;

  return data;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_toBase64(const uint8_t* data, size_t size)
{
  wstring b64;
  __base64_encode(b64, data, size);
  return b64;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_toBase64(wstring& b64, const uint8_t* data, size_t size)
{
  __base64_encode(b64, data, size);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_fromBase64(size_t* size, const wstring& b64)
{
  return __base64_decode(size, b64);
}

/// \brief Data URI Regex pattern
///
/// Regular expression pattern for Data URI.
///
//static const std::wregex __data_uri_reg(LR"(^(data:)([\w\/-]+);([\w]+)=?([\w\d-]+)?,([\w\W]+))"); // cause crash
static const std::wregex __data_uri_reg(LR"(^(data:)([\w\/-]+);([\w]+)=?([\w\d-]+)?,)");


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_encodeDataUri(wstring& uri, const wstring& mime_type, const wstring& charset, const uint8_t* data, size_t size)
{
  // compose final data URI string
  uri.clear();
  uri.append(L"data:");
  uri.append(mime_type);

  wstring uri_data;

  if(!charset.empty()) {
    uri.append(L";charset=");
    uri.append(charset);
    // convert data to wstring
    for(unsigned i = 0; i < size; ++i)
      uri_data.push_back(static_cast<wchar_t>(data[i]));
  } else {
    uri.append(L";base64");
    // encode binary data to base64
    __base64_encode(uri_data, data, size);
  }

  // finally append data
  uri.push_back(L',');
  uri.append(uri_data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_decodeDataUri(size_t* size, wstring& mime_type, wstring& charset, const wstring& uri)
{
  // initialize values
  (*size) = 0;
  mime_type = L"";
  charset = L"";

  // check for regex matches
  std::match_results<const wchar_t*> matches;
  if(std::regex_search(uri.c_str(), matches, __data_uri_reg)) {

    // matches :
    // 0) full match
    // 1) data:
    // 2) mime-type (eg. image/jpeg)
    // 3) encoding (eg. base64) or literally "charset"
    // 4) used charset (eg. UTF-8)
    // suffix()) the data...

    // search for full match
    if(matches.size()) {

      // set data mime type
      mime_type = matches[2];

      // check whether we got a charset
      if(matches[3] == L"charset") {

        // set the parsed charset
        charset = matches[4];

        // get data as wide string
        wstring wdata = matches.suffix();

        // allocate new buffer to hold data
        uint8_t* ascii = reinterpret_cast<uint8_t*>(Om_alloc(wdata.size()));
        if(!ascii) return nullptr;

        // convert the wchar_t to uint8_t by value. Since
        // plain text data URI should always have 8 bits
        // content, this should be OK
        for(unsigned i = 0; i < wdata.size(); ++i)
          ascii[i] = wdata[i];

        (*size) = wdata.size();

        return ascii;

      } else if(matches[3] == L"base64") {
        // decode base64 data
        return __base64_decode(size, matches.suffix());
      }
    }
  }

  return nullptr;
}


#include <ctime>
time_t __time_rtime;
struct tm* __time_ltime;

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_getTime(int *t_sec, int *t_min, int *t_hour)
{
  time(&__time_rtime);
  __time_ltime = localtime(&__time_rtime);

  if(t_sec != nullptr) *t_sec = __time_ltime->tm_sec;
  if(t_min != nullptr) *t_min = __time_ltime->tm_min;
  if(t_hour != nullptr) *t_hour = __time_ltime->tm_hour;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_getDate(int *t_day, int *t_mon, int *t_year)
{
  time(&__time_rtime);
  __time_ltime = localtime(&__time_rtime);

  if(t_day != nullptr) *t_day = __time_ltime->tm_mday;
  if(t_mon != nullptr) *t_mon = __time_ltime->tm_mon;
  if(t_year != nullptr) *t_year = __time_ltime->tm_year;
}


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

/// \brief UTF-8 decode
///
/// Convert the given multibyte string into wide char string assuming UTF-8
/// encoding.
///
/// This function si no longer used in current implementation but conserved
/// here in cas this is needed.
///
/// \param[in]  wstr    : Wide char string to receive conversion result.
/// \param[out] utf8    : Pointer to null-terminated Multibyte string to convert.
///
inline static void __utf8_decode(wstring& wstr, const char* utf8)
{
  wstr.clear();
  wstr.reserve(strlen(utf8));

  uint32_t u;
  unsigned char c;
  size_t p = 0;

  while(utf8[p] != 0) {

    c = utf8[p];

    if(c <= 0x7F) {
      u = c;
    } else if(c <= 0xBF) {
      u = (u << 6) | (c & 0x3F);
    } else if(c <= 0xdf) {
      u = c & 0x1F;
    } else if(c <= 0xEF) {
      u = c & 0x0F;
    } else {
      u = c & 0x07;
    }

    ++p;

    if(((utf8[p] & 0xC0) != 0x80) && (u <= 0x10FFFF)) {
      if(u > 0xFFFF)  {
        wstr.push_back(static_cast<wchar_t>(0xD800 + (u >> 10)));
        wstr.push_back(static_cast<wchar_t>(0xDC00 + (u & 0x03FF)));
      } else if(u < 0xD800 || u >= 0xE000) {
        wstr.push_back(static_cast<wchar_t>(u));
      }
    }
  }
}


/// \brief UTF-8 encode
///
/// Convert the given wide char string to multibyte string using UTF-8
/// encoding.
///
/// This function si no longer used in current implementation but conserved
/// here in cas this is needed.
///
/// \param[out] utf8    : Multibyte string to receive conversion result.
/// \param[in]  wstr    : Wide char string to convert.
///
inline static void __utf8_encode(string& utf8, const wstring& wstr)
{
  utf8.clear();
  utf8.reserve(wstr.size() * 4);

  char16_t c;
  uint32_t u;

  for(size_t i = 0; i < wstr.size(); ++i) {

    c = wstr[i];

    if(c >= 0xD800 && c <= 0xDBFF) {
      u = ((c - 0xD800) << 10) + 0x10000;
      continue; //< get next char
    } else if(c >= 0xDC00 && c <= 0xDFFF) {
      u |= c - 0xDC00;
    } else {
      u = c;
    }

    if(u <= 0x7F) {
      utf8.push_back(static_cast<char>(u));
    }else if(u <= 0x7FF) {
      utf8.push_back(static_cast<char>(0xC0 | ((u >> 6) & 0x1F)));
      utf8.push_back(static_cast<char>(0x80 | ( u       & 0x3F)));
    } else if(u <= 0xFFFF) {
      utf8.push_back(static_cast<char>(0xE0 | ((u >> 12) & 0x0F)));
      utf8.push_back(static_cast<char>(0x80 | ((u >>  6) & 0x3F)));
      utf8.push_back(static_cast<char>(0x80 | ( u        & 0x3F)));
    } else {
      utf8.push_back(static_cast<char>(0xF0 | ((u >> 18) & 0x07)));
      utf8.push_back(static_cast<char>(0x80 | ((u >> 12) & 0x3F)));
      utf8.push_back(static_cast<char>(0x80 | ((u >>  6) & 0x3F)));
      utf8.push_back(static_cast<char>(0x80 | ( u        & 0x3F)));
    }

    u = 0;
  }
}

/// \brief Multibyte Decode
///
/// Static inlined function to convert the given multibyte string into wide
/// char string assuming the specified encoding.
///
/// This function use the WinAPI MultiByteToWideChar implementation witch is
/// currently the known fastest way.
///
/// \param[in]  cp      : Code page to use in performing the conversion.
/// \param[in]  wstr    : Wide char string to receive conversion result.
/// \param[out] utf8    : Pointer to null-terminated Multibyte string to convert.
///
inline static size_t __multibyte_decode(UINT cp, wstring& wstr, const char* utf8)
{
  int n = MultiByteToWideChar(cp, 0, utf8, -1, nullptr, 0);

  if(n > 0) {
    wstr.resize(n - 1);
    // NOTICE: here bellow, the string object is used as C char buffer, in
    // theory this is not allowed since std::string is not required to store
    // its contents contiguously in memory.
    //
    // HOWEVER, in practice, there is no know situation where std::string does
    // not store its content contiguous, so, this should work anyway.
    //
    // If some problem emerge from this function, change this implementation for
    // a more regular approach.
    return static_cast<size_t>(MultiByteToWideChar(cp, 0, utf8, -1, &wstr[0], n));
  }

  return 0;
}


/// \brief Multibyte encode
///
/// Static inlined function to convert the given wide char string to multibyte
/// string using the specified encoding.
///
/// This function use the WinAPI MultiByteToWideChar implementation witch is
/// currently the known fastest way.
///
/// \param[in]  cp      : Code page to use in performing the conversion.
/// \param[out] utf8    : Multibyte string to receive conversion result.
/// \param[in]  wstr    : Wide char string to convert.
///
inline static size_t __multibyte_encode(UINT cp, string& utf8, const wchar_t* wstr)
{
  BOOL pBool;
  int n = WideCharToMultiByte(cp, 0, wstr, -1, nullptr, 0, nullptr, &pBool);
  if(n > 0) {
    utf8.resize(n - 1);
    // NOTICE: here bellow, the string object is used as C char buffer, in
    // theory this is not allowed since std::string is not required to store
    // its contents contiguously in memory.
    //
    // HOWEVER, in practice, there is no know situation where std::string does
    // not store its content contiguous, so, this should work anyway.
    //
    // If some problem emerge from this function, change this implementation for
    // a more regular approach.
    return static_cast<size_t>(WideCharToMultiByte(cp, 0, wstr, -1, &utf8[0], n, nullptr, &pBool));
  }
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_fromUtf8(const char* utf8)
{
  wstring result;
  __multibyte_decode(CP_UTF8, result, utf8);
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_fromUtf8(wstring& wstr, const char* utf8)
{
  return __multibyte_decode(CP_UTF8, wstr, utf8);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toUtf8(char* utf8, size_t len, const wstring& wstr)
{
  // The WinAPI implementation is the fastest one at this time
  BOOL pBool;
  int n = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8, len, nullptr, &pBool);
  return (n > 0) ? static_cast<size_t>(n) : 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
string Om_toUtf8(const wstring& wstr)
{
  string result;
  __multibyte_encode(CP_UTF8, result, wstr.c_str());
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toUtf8(string& utf8, const wstring& wstr)
{
  return __multibyte_encode(CP_UTF8, utf8, wstr.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toAnsiCp(char* ansi, size_t len, const wstring& wstr)
{
  // The WinAPI implementation is the fastest one at this time
  BOOL pBool;
  int n = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, ansi, len, nullptr, &pBool);
  return (n > 0) ? static_cast<size_t>(n) : 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toAnsiCp(string& ansi, const wstring& wstr)
{
  return __multibyte_encode(CP_ACP, ansi, wstr.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_fromAnsiCp(wstring& wstr, const char* ansi)
{
  return __multibyte_decode(CP_ACP, wstr, ansi);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toZipCDR(char* cdr, size_t len, const wstring& wstr)
{
  // The WinAPI implementation is the fastest one at this time
  BOOL pBool;
  int n = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, cdr, len, nullptr, &pBool);

  for(size_t i = 0; cdr[i] != 0; ++i) {
    if(cdr[i] == '\\') cdr[i] = '/';
  }

  return (n > 0) ? static_cast<size_t>(n) : 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toZipCDR(string& cdr, const wstring& wstr)
{
  __multibyte_encode(CP_UTF8, cdr, wstr.c_str());

  for(size_t i = 0; i < cdr.size(); ++i) {
    if(cdr[i] == '\\') cdr[i] = '/';
  }

  return cdr.size();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_fromZipCDR(wstring& wstr, const char* cdr)
{
  __multibyte_decode(CP_UTF8, wstr, cdr);

  for(size_t i = 0; i < wstr.size(); ++i) {
    if(wstr[i] == L'/') wstr[i] = L'\\';
  }

  return wstr.size();
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


/// \brief Illegal Windows characters
///
/// List of forbidden characters to test validity of file name or path.
///
static const wchar_t __illegal_win_chr[] = L"/*?\"<>|\\";


/// \brief URL Regex pattern
///
/// Regular expression pattern for URL.
///
static const std::wregex __url_reg(LR"(^(https?:\/\/)([\da-z\.-]+)(:[\d]+)?([\/\w\.%-]*)(\?[\w%-=&]+)?)");

/// \brief Illegal URL path characters
///
/// List of forbidden characters to test validity of URL path.
///
static const wchar_t __illegal_url_chr[] = L"#\"<>|\\{}^[]`+:@$";

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidUrl(const wchar_t* url)
{
  std::match_results<const wchar_t*> matches;

  if(std::regex_match(url, matches, __url_reg)) {

    // matches :
    // 0) full match (almost never happen)
    // 1) http(s)://
    // 2) xxx.www.domain.tld
    // 3) :1234
    // 4) /folder/file.ext
    // 5) ?x=1&y=2...

    // search for minimum required matches to have a full valid URL
    if(matches[1].length() != 0) { //< http(s)://
      if(matches[2].length() != 0) { //< xxx.www.domain.tld
        return (matches[4].length() != 0); //< /folder/file.ext
      }
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidUrl(const wstring& url)
{
  return Om_isValidUrl(url.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidUrlPath(const wchar_t* path)
{
  if(!wcslen(path))
    return false;

  for(unsigned i = 0; i < 16; ++i) // forbids all including back-slash
    if(wcschr(path, __illegal_url_chr[i]))
      return false;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidUrlPath(const wstring& path)
{
  if(path.empty())
    return false;

  for(unsigned i = 0; i < 16; ++i) // forbids all including back-slash
    if(path.find_first_of(__illegal_url_chr[i]) != wstring::npos)
      return false;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidName(const wchar_t* name)
{
  if(!wcslen(name))
    return false;

  for(unsigned i = 0; i < 8; ++i) // forbids all including back-slash
    if(wcschr(name, __illegal_win_chr[i]))
      return false;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidName(const wstring& name)
{
  if(name.empty())
    return false;

  for(unsigned i = 0; i < 8; ++i) // forbids all including back-slash
    if(name.find_first_of(__illegal_win_chr[i]) != wstring::npos)
      return false;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidPath(const wchar_t* path)
{
  if(!wcslen(path))
    return false;

  // check for illegal characters in path
  for(unsigned i = 0; i < 7; ++i)  // excluding back-slash
    if(wcschr(path, __illegal_win_chr[i]))
      return false;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidPath(const wstring& path)
{
  if(path.empty())
    return false;

  for(unsigned i = 0; i < 7; ++i)  // excluding back-slash
    if(path.find_first_of(__illegal_win_chr[i]) != wstring::npos)
      return false;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_sizeString(size_t bytes, bool octet)
{
  wchar_t swp_buf[64];
  wchar_t unit = (octet) ? 'o' : 'B';

  double fbytes;
  wstring result;

  if(bytes < 1024) { // 1 Ko
    if(octet) {
      swprintf(swp_buf, 64, L"%d Octet(s)", bytes);
    } else {
      swprintf(swp_buf, 64, L"%d Byte(s)", bytes);
    }
  } else if(bytes < 1048576) { // 1 Mo
    fbytes = (double)bytes / 1024.0;
    swprintf(swp_buf, 64, L"%.1f Ki%lc", fbytes, unit);
  } else if(bytes < 1073741824) { // 1 Go
    fbytes = (double)bytes / 1048576.0;
    swprintf(swp_buf, 64, L"%.1f Mi%lc", fbytes, unit);
  } else {
    fbytes = (double)bytes / 1073741824.0;
    swprintf(swp_buf, 64, L"%.1f Gi%lc", fbytes, unit);
  }

  result = swp_buf;
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isVersionStr(const wstring& str)
{
  unsigned n = 0;
  unsigned j = 0;

  for(unsigned i = 0; i < str.size(); ++i) {

    if(str[i] > 47 && str[i] < 58) { // 0123456789

      if(j < 15) {
        ++j;
      } else {
        return false;
      }

    } else {

      if(str[i] == L'.') {
        if(j > 0) {
          n++; j = 0;
        }
      }

      if(n > 2)
        break;
    }
  }

  return (j > 0 || n > 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_parsePkgIdent(wstring& name, wstring& vers, const wstring& filename, bool isfile, bool us2spc)
{
  wstring ident;

  // Get the proper part of the full filename
  if(isfile) {
    // get file name without file extension
    ident = Om_getNamePart(filename);
  } else {
    // this extract the folder name from the full path
    ident = Om_getFilePart(filename);
  }

  // parse raw name to get display name and potential version
  bool has_version = false;
  // we search a version part in the name, this must be the letter V preceded
  // by a common separator, like space, minus or underscore character, followed
  // by a number
  size_t v_pos = ident.find_last_of(L"vV");
  if(v_pos > 0) {
    // verify the V letter is preceded by a common separator
    wchar_t wc = ident[v_pos - 1];
    if(wc == L' ' || wc == L'_' || wc == L'-') {
      // verify the V letter is followed by a number
      wc = ident[v_pos + 1];
      if(wc > 0x29 && wc < 0x40) { // 0123456789
        // get the substring from v char to the end of string
        vers = ident.substr(v_pos+1, -1);
        has_version = Om_isVersionStr(vers);
      }
    }
  }

  if(has_version) {
    // we extract the substring from the beginning to the version substring
    name = ident.substr(0, v_pos);
  } else {
    vers.clear();
    name = ident;
  }

  // replace all underscores by spaces
  if(us2spc) {
    std::replace(name.begin(), name.end(), L'_', L' ');
  }

  return has_version;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_dirDeleteRecursive(const wstring& path)
{
  wchar_t path_buf[OMM_MAX_PATH];

  wcscpy(path_buf, path.c_str());
  path_buf[path.size()+1] = 0; // the buffer must end with double null character

  SHFILEOPSTRUCTW fop = {};
  fop.pFrom = path_buf;
  fop.wFunc = FO_DELETE;
  fop.fFlags = FOF_NO_UI;

  int result = SHFileOperationW(&fop);

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
  wchar_t path_buf[OMM_MAX_PATH];

  wcscpy(path_buf, path.c_str());
  path_buf[path.size()+1] = 0;

  SHFILEOPSTRUCTW fop = {};
  fop.pFrom = path_buf;
  fop.wFunc = FO_DELETE;
  fop.fFlags = FOF_NO_UI|FOF_ALLOWUNDO;

  int result = SHFileOperationW(&fop);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_checkAccess(const wstring& path, unsigned mask)
{
  //return __checkAccess(path, mask);

  // Thanks to this article for giving some clues :
  // http://blog.aaronballman.com/2011/08/how-to-check-access-rights/

  // STEP 1 - retrieve the "security descriptor" (i.e owner, group, access
  // rights, etc. ) of the specified file or folder.
  SECURITY_DESCRIPTOR* pSd;
  DWORD sdSize;
  // here is the mask for file permission informations we want to retrieve it
  // seem to be the minimum required for an access check request
  DWORD sdMask =  OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
                | DACL_SECURITY_INFORMATION;
  // first call to get required SECURITY_DESCRIPTOR size
  GetFileSecurityW(path.c_str(), sdMask, nullptr, 0, &sdSize);
  // allocate new SECURITY_DESCRIPTOR of the proper size
  pSd = reinterpret_cast<SECURITY_DESCRIPTOR*>(Om_alloc(sdSize + 1));
  // second call to get SECURITY_DESCRIPTOR data
  if(!GetFileSecurityW(path.c_str(), sdMask, pSd, sdSize, &sdSize)) {
    Om_free(pSd); return false;
  }

  // STEP 2 - creates a "security token" of the current application process
  //to be checked against the file or folder "security descriptor"
  DWORD daMask =  TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE
                | STANDARD_RIGHTS_READ;
  HANDLE hTokenProc = nullptr;
  if(!OpenProcessToken(GetCurrentProcess(), daMask, &hTokenProc)) {
    Om_free(pSd); return false;
  }
  // the current process token is a "primary" one (don't know what that mean)
  // so we need to duplicate it to transform it into a standard "user" token by
  // impersonate it...
  HANDLE hTokenUser = nullptr;
  if(!DuplicateToken(hTokenProc, SecurityImpersonation, &hTokenUser)) {
    CloseHandle(hTokenProc); Om_free(pSd);
    return false;
  }

  // STEP 3 - Finally check if "security token" have the requested
  // "mask" access to the "security descriptor" of the specified file
  // or folder

  // the GENERIC_MAPPING seem to be never used in most common scenarios,
  // we set it here because the parameter is mandatory.
  GENERIC_MAPPING gm = {GENERIC_READ,GENERIC_WRITE,GENERIC_EXECUTE,GENERIC_ALL};
  PRIVILEGE_SET ps = {};
  DWORD psSize = sizeof(PRIVILEGE_SET);
  DWORD allowed = 0;      //< mask of allowed access
  BOOL  status = false;   //< access status according supplied GENERIC_MAPPING
  AccessCheck(pSd, hTokenUser, mask, &gm, &ps, &psSize, &allowed, &status);

  if(!status) {
    std::wcout << L"__checkAccess: denied, allowed access (mask): \n";
    AccessCheck(pSd, hTokenUser, FILE_READ_DATA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  FILE_READ_DATA + LIST_DIRECTORY\n";
    AccessCheck(pSd, hTokenUser, FILE_WRITE_DATA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  FILE_WRITE_DATA + ADD_FILE\n";
    AccessCheck(pSd, hTokenUser, FILE_APPEND_DATA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  FILE_APPEND_DATA + ADD_SUBDIRECTORY\n";
    AccessCheck(pSd, hTokenUser, FILE_READ_EA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  FILE_READ_EA\n";
    AccessCheck(pSd, hTokenUser, FILE_WRITE_EA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  FILE_WRITE_EA\n";
    AccessCheck(pSd, hTokenUser, FILE_EXECUTE, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  FILE_EXECUTE + TRAVERSE\n";
    AccessCheck(pSd, hTokenUser, FILE_DELETE_CHILD, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  FILE_DELETE_CHILD\n";
    AccessCheck(pSd, hTokenUser, FILE_READ_ATTRIBUTES, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  FILE_READ_ATTRIBUTES\n";
    AccessCheck(pSd, hTokenUser, FILE_WRITE_ATTRIBUTES, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  FILE_WRITE_ATTRIBUTES\n";
    AccessCheck(pSd, hTokenUser, DELETE, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  DELETE\n";
    AccessCheck(pSd, hTokenUser, READ_CONTROL, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  READ_CONTROL\n";
    AccessCheck(pSd, hTokenUser, WRITE_DAC, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  WRITE_DAC\n";
    AccessCheck(pSd, hTokenUser, WRITE_OWNER, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  WRITE_OWNER\n";
    AccessCheck(pSd, hTokenUser, SYNCHRONIZE, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? L"[x]" : L"[ ]") << L"  SYNCHRONIZE\n";
    status = 0;
  }

  CloseHandle(hTokenProc);
  CloseHandle(hTokenUser);
  Om_free(pSd);

  return status;
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
INT CALLBACK __dialogBrowseDir_Proc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  if(uMsg == BFFM_INITIALIZED) { //< Brother dialog windows is initialized
    SendMessageW(hWnd, BFFM_SETSELECTION, false, lpData);  //< set the selected folder
  }
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dialogBrowseDir(wstring& result, HWND hWnd, const wchar_t* title, const wstring& start, bool captive)
{
  BROWSEINFOW bI = {};
  bI.hwndOwner = hWnd;
  bI.lpszTitle = title;
  bI.ulFlags = BIF_USENEWUI|BIF_RETURNONLYFSDIRS|BIF_VALIDATE;

  if(captive) {
    // this is the standard easy way to use SHBrowseForFolderW, this will
    // set the "start" path as the root of browsing, so the user cannot go up
    // to parent folder
    if(start.size()) {
      PIDLIST_ABSOLUTE pIdl = nullptr;
      SHParseDisplayName(start.c_str(), nullptr, &pIdl, 0, nullptr); //< convert path string to LPITEMIDLIST
      bI.pidlRoot = pIdl;
    }
  } else {
      // this is the advanced way to use SHBrowseForFolderW, here we use a
      // callback function to handle the dialog window initialization, the "start"
      // path object will be passed as lParam to the callback with the
      // BFFM_INITIALIZED message.
    if(start.size()) {
      PIDLIST_ABSOLUTE pIdl = nullptr;
      SHParseDisplayName(start.c_str(), nullptr, &pIdl, 0, nullptr); //< convert path string to LPITEMIDLIST
      bI.lpfn = __dialogBrowseDir_Proc;
      bI.lParam = reinterpret_cast<LPARAM>(pIdl);
    }
  }

  bool suceess = false;

  LPITEMIDLIST pIdl;
  if((pIdl = SHBrowseForFolderW(&bI)) != nullptr) {

    wchar_t psz_path[OMM_MAX_PATH];

    psz_path[0] = 0;
    if(SHGetPathFromIDListEx(pIdl, psz_path, OMM_MAX_PATH, GPFIDL_DEFAULT)) {
      result = psz_path;
      suceess = true;
    }
  }

  CoTaskMemFree(pIdl);

  return suceess;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dialogOpenFile(wstring& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const wstring& start)
{
  wchar_t str_file[OMM_MAX_PATH];

  OPENFILENAMEW ofn = {};
  ofn.lStructSize = sizeof(OPENFILENAMEW);

  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = filter;

  // Oyé oyé, dear me in the future trying to fix this...

  // lpstrInitialDir does not work as attended, since Windows 7 if it has the
  // same value as was passed the first time the application used an Open or
  // Save As dialog box, the path most recently selected by the user is used
  // as the initial directory.
  ofn.lpstrInitialDir = start.c_str();

  // As workaround for the lpstrInitialDir behavior, we could set lpstrFile
  // with an initial path with a wildcard as file name (eg. C:\folder\*.ext)
  // However, within the context of this function, this would require to add
  // an new argument specify the wildcard to set, or to parse the filter
  // string which would be such monstrous routine for a so little thing.
  ofn.lpstrFile = str_file;
  ofn.lpstrFile[0] = L'\0';

  ofn.nMaxFile = OMM_MAX_PATH;

  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER|OFN_NONETWORKBUTTON|OFN_NOTESTFILECREATE;

  if(GetOpenFileNameW(&ofn)) {
    result = str_file;
    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dialogSaveFile(wstring& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const wstring& start)
{
  wchar_t str_file[OMM_MAX_PATH];
  swprintf(str_file, OMM_MAX_PATH, L"%ls", result.c_str());

  OPENFILENAMEW ofn = {};
  ofn.lStructSize = sizeof(OPENFILENAMEW);

  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = filter;

  ofn.lpstrFile = str_file;
  ofn.nMaxFile = OMM_MAX_PATH;

  ofn.lpstrInitialDir = start.c_str();

  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER|OFN_NONETWORKBUTTON|OFN_NOTESTFILECREATE;

  if(GetSaveFileNameW(&ofn)) {
    result = str_file;
    return true;
  }

  return false;
}


/// \brief Load plain text.
///
/// Loads content of the specified file as plain-text into the given
/// string object.
///
/// \param[in] text    : String to receive loaded text data.
/// \param[in] path    : Path to text file to be loaded.
///
/// \return Count of bytes read.
///
inline static size_t __load_plaintxt(string& txt, const wchar_t* path)
{
  txt.clear();

  HANDLE hFile = CreateFileW( path, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return 0;

  DWORD rb;
  size_t rt = 0;
  char cbuf[4097];

  while(ReadFile(hFile, cbuf, 4096, &rb, nullptr)) {

    if(rb == 0)
      break;

    rt += rb;

    cbuf[rb] = '\0';

    txt.append(cbuf);
  }

  CloseHandle(hFile);

  return rt;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
string Om_loadPlainText(const wstring& path)
{
  string result;
  __load_plaintxt(result, path.c_str());
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_loadPlainText(string& text, const wstring& path)
{
  return __load_plaintxt(text, path.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_loadBinary(size_t* size, const wstring& path)
{
  // initialize size
  (*size) = 0;

  // open file for reading
  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return nullptr;

  size_t data_size = GetFileSize(hFile, nullptr);

  // allocate buffer and read
  uint8_t* data = reinterpret_cast<uint8_t*>(Om_alloc(data_size));
  if(!data) return nullptr;

  // read full data at once
  DWORD rb;
  bool result = ReadFile(hFile, data, data_size, &rb, nullptr);

  // close file
  CloseHandle(hFile);

  if(!result) {
    Om_free(data);
    return nullptr;
  }

  (*size) = data_size;

  return data;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
#include "thirdparty/jpeg/jpeglib.h"
#include "thirdparty/png/png.h"
#include "thirdparty/gif/gif_lib.h"
//#include "thirdparty/gif/quantize.c"

/* we make sure structures are packed to be properly aligned with
 read buffer */
#pragma pack(1)
/// \brief BMP info header
///
/// Structure for BMP file info header
struct OMM_BITMAPINFOHEADER {
  uint32_t  size;           ///< size of the structure
  uint32_t  width;          ///< image width
  uint32_t  height;         ///< image height
  uint16_t  planes;         ///< bit planes
  int16_t   bpp;            ///< bit per pixel
  uint32_t  compression;    ///< compression
  uint32_t  sizeimage;      ///< size of the image
  int32_t   xppm;           ///< pixels per meter X
  int32_t   yppm;           ///< pixels per meter Y
  uint32_t  clrused;        ///< colors used
  uint32_t  clrimportant;   ///< important colors
}; // 40 bytes
/// \brief BMP base header
///
/// Structure for BMP file base header
struct OMM_BITMAPHEADER {
  uint8_t   signature[2];   ///< BM magic word
  uint32_t  size;           ///< size of the whole .bmp file
  uint16_t  reserved1;      ///< must be 0
  uint16_t  reserved2;      ///< must be 0
  uint32_t  offbits;        ///< where bitmap data begins
}; // 14 bytes
#pragma pack()

/// BMP file specific signature / magic number
static const unsigned char __sign_bmp[] = "BM";
/// JPG file specific signature / magic number
static const unsigned char __sign_jpg[] = {0xFF, 0xD8, 0xFF};
/// PNG file specific signature / magic number
static const unsigned char __sign_png[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
/// GIF file specific signature / magic number
static const unsigned char __sign_gif[] = "GIF89a";

/// \brief compare buffer to known images signatures
///
/// Check whether the given buffer matches any known file
/// signature or magic number.
///
/// \param[in]  buff  : Buffer to compare known signatures with
///
/// \return found image type or 0
///
inline static unsigned __image_sign_matches(const uint8_t* buff)
{
  // Test BMP signature
  if(0 == memcmp(buff, __sign_bmp, 2)) return OMM_IMAGE_TYPE_BMP;
  // Test JPG signature
  if(0 == memcmp(buff, __sign_jpg, 3)) return OMM_IMAGE_TYPE_JPG;
  // Test PNG signature
  if(0 == memcmp(buff, __sign_png, 8)) return OMM_IMAGE_TYPE_PNG;
  // Test GIF signature
  if(0 == memcmp(buff, __sign_gif, 6)) return OMM_IMAGE_TYPE_GIF;

  return 0;
}

/// \brief Quantized color node
///
/// Structure for linked list node describing a quantized color
///
struct __qz_rgb {
  uint8_t   rgb[3];       //< RGB color
  uint8_t   pos;          //< Position index in linked-list
  int32_t   ref_count;    //< Count pixel that reference this color
  __qz_rgb* next;         //< Next node in linked-list
};

/// \brief Quantized map
///
/// Structure for quantized color map
///
struct __qz_map {
  uint8_t   rgb_min[3];
  uint8_t   rgb_rng[3];
  uint32_t  idx_count;     //< Total number of pixels in all the entries
  uint32_t  size;          //< # of __qz_rgb in linked list below
  __qz_rgb* node_list;
};

/// \brief Quantized color sorting function
///
/// Quantized color node sorting function along Red axis
///
static bool __qz_sort_r_fn(const __qz_rgb* a, const __qz_rgb* b)
{
  unsigned h1 = a->rgb[0] * 256 * 256 + a->rgb[1] * 256 + a->rgb[2];
  unsigned h2 = b->rgb[0] * 256 * 256 + b->rgb[1] * 256 + b->rgb[2];
  return (h1 < h2);
}

/// \brief Quantized color sorting function
///
/// Quantized color node sorting function along Green axis
///
static bool __qz_sort_g_fn(const __qz_rgb* a, const __qz_rgb* b)
{
  unsigned h1 = a->rgb[1] * 256 * 256 + a->rgb[2] * 256 + a->rgb[0];
  unsigned h2 = b->rgb[1] * 256 * 256 + b->rgb[2] * 256 + b->rgb[0];
  return (h1 < h2);
}

/// \brief Quantized color sorting function
///
/// Quantized color node sorting function along Blue axis
///
static bool __qz_sort_b_fn(const __qz_rgb* a, const __qz_rgb* b)
{
  unsigned h1 = a->rgb[2] * 256 * 256 + a->rgb[0] * 256 + a->rgb[1];
  unsigned h2 = b->rgb[2] * 256 * 256 + b->rgb[0] * 256 + b->rgb[1];
  return (h1 < h2);
}

/// \brief Quantized color sort functions array
///
/// Array containing pointers to quantized color sort functions
///
static bool (*__qz_sort_fn[])(const __qz_rgb*, const __qz_rgb*) = {
  __qz_sort_r_fn,
  __qz_sort_g_fn,
  __qz_sort_b_fn
  };

/// \brief Quantization subdivision
///
/// Color quantization function to subdivide the RGB space recursively
/// using median cut in each axes alternatingly until ColorMapSize different
/// cubes exists.
/// The biggest cube in one dimension is subdivide unless it has only one entry.
///
/// \param[in]  cmap      : Pointer to color map to subdivide
/// \param[in]  in_size   : Initial size of the supplied color map
/// \param[in]  out_size  : New size of the subdivided color map
///
/// the following implementation is a rewriting of the SubdivColorMap
/// function from the quantize.c file of the GifLib library.
///
static inline void __image_quantize_subdiv(__qz_map* cmap, unsigned* out_size, unsigned in_size)
{
  __qz_rgb* node;
  std::vector<__qz_rgb*> sort_list;
  unsigned sort_axis, min_color, max_color, n, c, i, j, u = 0;
  int rng_max, r;

  while(in_size > *out_size) {
    // Find candidate for subdivision:
    rng_max = -1;
    for(i = 0; i < *out_size; ++i) {
      for(j = 0; j < 3; ++j) {
        if((static_cast<int>(cmap[i].rgb_rng[j]) > rng_max) && (cmap[i].size > 1)) {
          rng_max = cmap[i].rgb_rng[j];
          u = i;
          sort_axis = j;
        }
      }
    }

    if(rng_max == -1)
      return;

    // Split the entry Index into two along the axis SortRGBAxis:

    // Sort all elements in that entry along the given axis and split at
    // the median.
    sort_list.reserve(cmap[u].size);

    for(j = 0, node = cmap[u].node_list; j < cmap[u].size && node != nullptr; j++, node = node->next) {
      sort_list.push_back(node);
    }

    // Because qsort isn't stable, this can produce differing
    // results for the order of tuples depending on platform
    // details of how qsort() is implemented.
    //
    // We mitigate this problem by sorting on all three axes rather
    // than only the one specied by SortRGBAxis; that way the instability
    // can only become an issue if there are multiple color indices
    // referring to identical RGB tuples.  Older versions of this
    // sorted on only the one axis.
    std::sort(sort_list.begin(), sort_list.end(), __qz_sort_fn[sort_axis]);

    for(j = 0; j < cmap[u].size - 1; ++j)
      sort_list[j]->next = sort_list[j+1];

    sort_list[cmap[u].size - 1]->next = nullptr;
    cmap[u].node_list = node = sort_list[0];

    sort_list.clear();

    // Now simply add the Counts until we have half of the Count:
    r = cmap[u].idx_count / 2 - node->ref_count;
    n = 1;
    c = node->ref_count;
    while(node->next != nullptr && (r -= node->next->ref_count) >= 0 && node->next->next != nullptr) {
      node = node->next;
      n++;
      c += node->ref_count;
    }
    // Save the values of the last color of the first half, and first
    // of the second half so we can update the Bounding Boxes later.
    // Also as the colors are quantized and the BBoxes are full 0..255,
    // they need to be rescaled.
    max_color = node->rgb[sort_axis]; //< Max. of first half
    // coverity[var_deref_op]
    min_color = node->next->rgb[sort_axis]; //< of second
    max_color <<= 3;
    min_color <<= 3;

    // Partition right here:
    cmap[*out_size].node_list = node->next;
    node->next = nullptr;
    cmap[*out_size].idx_count = c;
    cmap[u].idx_count -= c;
    cmap[*out_size].size = cmap[u].size - n;
    cmap[u].size = n;
    for(j = 0; j < 3; ++j) {
      cmap[*out_size].rgb_min[j] = cmap[u].rgb_min[j];
      cmap[*out_size].rgb_rng[j] = cmap[u].rgb_rng[j];
    }
    cmap[*out_size].rgb_rng[sort_axis] = cmap[*out_size].rgb_min[sort_axis] + cmap[*out_size].rgb_rng[sort_axis] - min_color;
    cmap[*out_size].rgb_min[sort_axis] = min_color;

    cmap[u].rgb_rng[sort_axis] = max_color - cmap[u].rgb_min[sort_axis];

    (*out_size)++;
  }
}

/// \brief Color quantization
///
/// Function to Quantize high resolution image into lower one. Input image
/// consists of a 2D array for each of the RGB colors with size Width by Height.
/// There is no Color map for the input. Output is a quantized image with 2D
/// array of indexes into the output color map.
/// Note input image can be 24 bits at the most (8 for red/green/blue) and
/// the output has 256 colors at the most (256 entries in the color map.).
/// ColorMapSize specifies size of color map up to 256 and will be updated to
/// real size before returning.
/// Also non of the parameter are allocated by this routine.
///
/// the following implementation is a rewriting of the GifQuantizeBuffer
/// function from the quantize.c file of the GifLib library.
///
/// \param[out]   out_idx   : Output image pixels color indices (must be allocated).
/// \param[out]   out_map   : Output image color map (must be allocated).
/// \param[out]   map_size  : As input, the desired maximum size of color map, as output, the actual final size of color map.
/// \param[in]    in_rgb    : Input image RGB(A) pixel data.
/// \param[in]    in_w      : Input image width in pixels.
/// \param[in]    in_w      : Input image height in pixels.
/// \param[in]    in_c      : Input image color component count (bytes per pixel).
///
/// \return true if operation succeed, false otherwise.
///
static bool __image_quantize(uint8_t* out_idx, uint8_t* out_map, unsigned* map_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  __qz_map new_cmap[256];
  __qz_rgb *node_list;
  __qz_rgb *node;

  unsigned u, i, j;

  const uint8_t* sp;
  uint8_t* dp;

  size_t mtx_bytes = in_w * in_h;

  node_list = reinterpret_cast<__qz_rgb*>(Om_alloc(sizeof(__qz_rgb) * 32768));
  if(!node_list) return false;

  for(i = 0; i < 32768; ++i) {
    node_list[i].rgb[0] =  (i >> 10);
    node_list[i].rgb[1] =  (i >>  5) & 0x1F;
    node_list[i].rgb[2] =  (i      ) & 0x1F;
    node_list[i].ref_count = 0;
  }

  // Sample the colors and their distribution:
  for(i = 0, sp = in_rgb; i < mtx_bytes; ++i, sp += in_c) {
    u = ((sp[0] >> 3) << 10) + ((sp[1] >> 3) << 5) + (sp[2] >> 3);
    node_list[u].ref_count++;
  }

  /* Put all the colors in the first entry of the color map, and call the
   * recursive subdivision process.  */
  for(i = 0; i < 256; i++) {
    new_cmap[i].node_list = nullptr;
    new_cmap[i].idx_count = 0;
    new_cmap[i].size = 0;
    for(j = 0; j < 3; j++) {
      new_cmap[i].rgb_min[j] = 0;
      new_cmap[i].rgb_rng[j] = 255;
    }
  }

  /* Find the non empty entries in the color table and chain them: */
  for(i = 0; i < 32768; ++i) {
    if(node_list[i].ref_count > 0) break;
  }

  node = new_cmap[0].node_list = &node_list[i];
  unsigned n = 1;
  while(++i < 32768) {
    if(node_list[i].ref_count > 0) {
      node->next = &node_list[i];
      node = &node_list[i];
      n++;
    }
  }
  node->next = nullptr;

  new_cmap[0].size = n;               //< Different sampled colors
  new_cmap[0].idx_count = mtx_bytes;   //< Pixels

  unsigned new_size = 1;

  __image_quantize_subdiv(new_cmap, &new_size, (*map_size));

  if(new_size < (*map_size)) {
    // And clear rest of color map:
    memset(out_map + (new_size * 3), 0, ((*map_size) - new_size) * 3);
  }

  // Average the colors in each entry to be the color to be used in the
  // output color map, and plug it into the output color map itself.
  unsigned r, g, b;
  for(i = 0, dp = out_map; i < new_size; ++i, dp += 3) {
    if((j = new_cmap[i].size) > 0) {
      node = new_cmap[i].node_list;
      r = g = b = 0;
      while(node) {
        node->pos = i;
        r += node->rgb[0];
        g += node->rgb[1];
        b += node->rgb[2];
        node = node->next;
      }
      dp[0] = (r << 3) / j;
      dp[1] = (g << 3) / j;
      dp[2] = (b << 3) / j;
    }
  }

  // Finally scan the input buffer again and put the mapped index in the
  // output buffer.
  for(i = 0, sp = in_rgb; i < mtx_bytes; ++i, sp += in_c) {
    u = ((sp[0] >> 3) << 10) + ((sp[1] >> 3) <<  5) + (sp[2] >> 3);
    out_idx[i] = node_list[u].pos;
  }

  Om_free(node_list);

  (*map_size) = new_size;

  return true;
}

/// \brief Custom GIF reader
///
/// Custom read function for GIF library to read a file pointer.
///
/// \param[in]  gif     Decoder structure pointer.
/// \param[in]  dst     Destination buffer.
/// \param[in]  len     Length of data that should be read.
///
/// \return  The number of bytes that were read
///
static int __gif_read_file_fn(GifFileType* gif, uint8_t* dst, int len)
{
  return fread(dst, 1, len, reinterpret_cast<FILE*>(gif->UserData));
}

/// \brief Custom GIF write struct
///
/// Custom structure for custom GIF write routine.
///
struct __gif_read_st {
  const uint8_t*  src_data;
  size_t    src_seek;
};

/// \brief Custom GIF reader
///
/// Custom read function for GIF library to read memory buffer.
///
/// \param[in]  gif     Decoder structure pointer.
/// \param[in]  dst     Destination buffer.
/// \param[in]  len     Length of data that should be read.
///
/// \return  The number of bytes that were read
///
static int __gif_read_buff_fn(GifFileType* gif, uint8_t* dst, int len)
{
  __gif_read_st* read_st = reinterpret_cast<__gif_read_st*>(gif->UserData);
  memcpy(dst, read_st->src_data + read_st->src_seek, len);
  read_st->src_seek += len;
  return len;
}

/// \brief Custom GIF writer
///
/// Custom read function for GIF library to write a file pointer.
///
/// \param[in]  gif     Decoder structure pointer.
/// \param[in]  src     Source buffer.
/// \param[in]  len     Length of data that should be read.
///
/// \return  The number of length that were read
///
static int __gif_write_file_fn(GifFileType* gif, const uint8_t* src, int len)
{
  return fwrite(src, 1, len, reinterpret_cast<FILE*>(gif->UserData));
}

/// \brief Custom GIF write struct
///
/// Custom structure for custom GIF write routine.
///
struct __gif_write_st {
  uint8_t*  dst_data;
  size_t    dst_size;
  size_t    dst_seek;
};

/// \brief Custom GIF writer
///
/// Custom write function for GIF library to encode to memory.
///
/// \param[in]  gif     Decoder structure pointer.
/// \param[in]  src     Source buffer.
/// \param[in]  len     Length of data that should be read.
///
/// \return  The number of length that were read
///
static int __gif_write_buff_fn(GifFileType* gif, const uint8_t* src, int len)
{
  __gif_write_st* write_st = reinterpret_cast<__gif_write_st*>(gif->UserData);
  write_st->dst_size += len;
  if(write_st->dst_data) {
    write_st->dst_data = reinterpret_cast<uint8_t*>(Om_realloc(write_st->dst_data, write_st->dst_size));
  } else {
    write_st->dst_data = reinterpret_cast<uint8_t*>(Om_alloc(write_st->dst_size));
  }
  memcpy(write_st->dst_data + write_st->dst_seek, src, len);
  write_st->dst_seek += len;
  return len;
}

/// \brief Decode GIF.
///
/// Common function to decode GIF using the given GIF decoder structure.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  gif_dec : GIF decoder structure pointer.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB image data or nullptr if failed.
///
static uint8_t* __gif_decode_common(unsigned* out_w, unsigned* out_h, unsigned* out_c, void* gif_dec, bool flip_y)
{
  // Retrieve GIF decoder struct
  int error;
  GifFileType* gif = reinterpret_cast<GifFileType*>(gif_dec);

  // Load GIF content
  if(DGifSlurp(gif) == GIF_ERROR) {
    DGifCloseFile(gif, &error);
    return nullptr;
  }

  // Get image list, we care only about the first one
  SavedImage* images = gif->SavedImages;

  // Get image informations
  unsigned w = images[0].ImageDesc.Width;
  unsigned h = images[0].ImageDesc.Height;

  // Pointer to color table
  ColorMapObject* table;

  // check whether we got a local color table (may never happen)
  if(images[0].ImageDesc.ColorMap) {
    table = images[0].ImageDesc.ColorMap;
  } else {
    table = gif->SColorMap;
  }

  // define some useful sizes
  size_t row_bytes = w * 3;
  size_t tot_bytes = h * row_bytes;

  // allocate new buffer for RGB data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) {
    DGifCloseFile(gif, &error);
    return nullptr;
  }

  // get GIF index list
  uint8_t* sp = static_cast<uint8_t*>(images[0].RasterBits);

  // destination pointer
  uint8_t* dp;

  // here we go to translate indexed color to RGB
  for(unsigned y = 0; y < h; ++y) {

    dp = (flip_y) ? rgb + (row_bytes * ((h -1) - y)) : rgb + (y * row_bytes);

    for(unsigned x = 0; x < w; ++x) {
      dp[0] = table->Colors[*sp].Red;
      dp[1] = table->Colors[*sp].Green;
      dp[2] = table->Colors[*sp].Blue;
      dp += 3; sp++;
    }
  }

  // free decoder
  DGifCloseFile(gif, &error);

  // assign output values
  (*out_w) = w; (*out_h) = h; (*out_c) = 3;

  return rgb;
}

/// \brief Encode GIF.
///
/// Common function to encode GIF using the given GIF encoder structure.
///
/// \param[in]  gif_enc : GIF encoder structure pointer.
/// \param[in]  in_rgb  : Input image RGB(A) data to encode.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count.
///
/// \return True if operation succeed, false otherwise
///
static bool __gif_encode_common(void* gif_enc, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // Retrieve GIF encoder struct
  int error;
  GifFileType* gif = reinterpret_cast<GifFileType*>(gif_enc);

  // define useful sizes
  size_t mtx_bytes = in_w * in_h; //< image matrix size, one byte per pixel

  // allocate new buffer to receive color indices
  uint8_t* imtx = reinterpret_cast<uint8_t*>(Om_alloc(mtx_bytes));
  if(!imtx) {
    EGifCloseFile(gif, &error);
    return false;
  }

  // allocate new color map of 256 colors
  unsigned cmap_size = 256;
  uint8_t* cmap = reinterpret_cast<uint8_t*>(Om_alloc(cmap_size * 3)); //< cmap_size * RGB
  if(!cmap) {
    Om_free(imtx);
    EGifCloseFile(gif, &error);
    return false;
  }

  // quantize image
  if(!__image_quantize(imtx, cmap, &cmap_size, in_rgb, in_w, in_h, in_c)) {
    Om_free(imtx); Om_free(cmap);
    EGifCloseFile(gif, &error);
    return false;
  }

  // set GIF global parameters
  gif->SWidth = in_w;
  gif->SHeight = in_h;
  gif->SColorResolution = 8;
  gif->SBackGroundColor = 0;
  gif->SColorMap = GifMakeMapObject(cmap_size, reinterpret_cast<GifColorType*>(cmap)); //< global color table

  // set image parameters
  SavedImage image;
  image.ImageDesc.Left = 0;
  image.ImageDesc.Top = 0;
  image.ImageDesc.Width = in_w;
  image.ImageDesc.Height = in_h;
  image.ImageDesc.Interlace = false;
  image.ImageDesc.ColorMap = nullptr; //< no local color table
  image.RasterBits = imtx; //< our color indices
  image.ExtensionBlockCount = 0;
  image.ExtensionBlocks = nullptr;

  // add image to gif encoder
  GifMakeSavedImage(gif, &image);

  // encode GIF
  if(GIF_OK != EGifSpew(gif)) {
    Om_free(imtx);
    Om_free(cmap);
    EGifCloseFile(gif, &error);
    return false;
  }

  // free allocated data
  Om_free(imtx);
  Om_free(cmap);

  return true;
}

/// \brief Read GIF file.
///
/// Read GIF data from file pointer.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_file : Input file pointer to read data from.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB image data or nullptr if failed.
///
static uint8_t* __gif_read(unsigned* out_w, unsigned* out_h, unsigned* out_c, FILE* in_file, bool flip_y)
{
  int error;
  GifFileType* gif;

  // make sure we start at beginning
  fseek(in_file, 0, SEEK_SET);

  // Define custom read function and load GIF header
  gif = DGifOpen(in_file, __gif_read_file_fn, &error);
  if(gif == nullptr)
    return nullptr;

  // Decode GIF data
  return __gif_decode_common(out_w, out_h, out_c, gif, flip_y);
}

/// \brief Decode GIF data.
///
/// Decode GIF data from buffer in memory.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_data : Input GIF data to decode.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB image data or nullptr if failed.
///
static uint8_t* __gif_decode(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, bool flip_y)
{
  int error;
  GifFileType* gif;

  // custom read structure
  __gif_read_st read_st;
  read_st.src_data = in_data;
  read_st.src_seek = 0;

  // Define custom read function and load GIF header
  gif = DGifOpen(&read_st, __gif_read_buff_fn, &error);
  if(gif == nullptr)
    return nullptr;

  // Decode GIF data
  return __gif_decode_common(out_w, out_h, out_c, gif, flip_y);
}

/// \brief Write GIF file.
///
/// Write GIF data to file pointer.
///
/// \param[out] out_file  : File pointer to write to.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return True if operation succeed, false otherwise
///
static bool __gif_write(FILE* out_file, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  int error;
  GifFileType* gif;

  // Define custom read function and load GIF header
  gif = EGifOpen(out_file, __gif_write_file_fn, &error);
  if(gif == nullptr)
    return false;

  // Encode RGB to GIF data
  return __gif_encode_common(gif, in_rgb, in_w, in_h, in_c);
}

/// \brief Encode GIF data.
///
/// Encode GIF data to buffer in memory.
///
/// \param[out] out_size  : Output GIF data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return Pointer to encoded GIF image data or nullptr if failed.
///
static uint8_t* __gif_encode(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  int error;
  GifFileType* gif;

  // custom write structure
  __gif_write_st write_st;
  write_st.dst_data = nullptr;
  write_st.dst_size = 0;
  write_st.dst_seek = 0;

  // Define custom read function and load GIF header
  gif = EGifOpen(&write_st, __gif_write_buff_fn, &error);
  if(gif == nullptr)
    return nullptr;

  // Encode RGB to GIF data
  if(!__gif_encode_common(gif, in_rgb, in_w, in_h, in_c))
    return nullptr;

  // assign output values
  (*out_size) = write_st.dst_size;

  return write_st.dst_data;
}

/// \brief Read BMP file.
///
/// Read BMP data from file pointer.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_file : Input file pointer to read data from.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB(A) image data or nullptr if failed.
///
static uint8_t* __bmp_read(unsigned* out_w, unsigned* out_h, unsigned* out_c, FILE* in_file, bool flip_y)
{
  // make sure we start at begining
  fseek(in_file, 0, SEEK_SET);

  // BMP headers structures
  OMM_BITMAPHEADER bmp_head;
  OMM_BITMAPINFOHEADER bmp_info;
  // read base header
  if(fread(&bmp_head, 1, 14, in_file) < 14)
    return nullptr;
  // check BM signature
  if(0 != memcmp(bmp_head.signature, "BM", 2))
    return nullptr;
  // read info header
  if(fread(&bmp_info, 1, 40, in_file) < 40)
    return nullptr;

  // we support only 24 or 32 bpp
  if(bmp_info.bpp < 24)
    return nullptr;
  // get BMP image parameters
  unsigned w = bmp_info.width;
  unsigned h = bmp_info.height;
  unsigned c = bmp_info.bpp / 8; // channel count

  // define some useful sizes
  size_t row_bytes = w * c;
  size_t tot_bytes = h * row_bytes;

  // allocate new buffer to receive rgb data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) return nullptr;

  // seek to bitmap data location and read
  fseek(in_file, bmp_head.offbits, SEEK_SET);

  // BMP data is natively stored upside down
  if(flip_y) {
    // read all data at once from
    if(fread(rgb, 1, tot_bytes, in_file) != tot_bytes) {
      Om_free(rgb); return nullptr;
    }
  } else {
    // read rows in reverse order
    unsigned hmax = (h - 1);
    for(unsigned y = 0; y < h; ++y) {
      if(fread(rgb + (row_bytes * (hmax - y)), 1, row_bytes, in_file) != row_bytes) {
        Om_free(rgb); return nullptr;
      }
    }
  }

  // finally swap components order BGR to RGB
  for(unsigned i = 0; i < tot_bytes; i += c)
    rgb[i  ] ^= rgb[i+2] ^= rgb[i  ] ^= rgb[i+2]; //< BGR => RGB

  // assign output values
  (*out_w) = w; (*out_h) = h; (*out_c) = c;

  return rgb;
}

/// \brief Decode BMP data.
///
/// Decode BMP data from buffer in memory.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_data : Input BMP data to decode.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB(A) image data or nullptr if failed.
///
static uint8_t* __bmp_decode(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, bool flip_y)
{
  // pointer to input data
  const uint8_t* in_ptr = in_data;

  // BMP headers structures
  OMM_BITMAPHEADER bmp_head;
  OMM_BITMAPINFOHEADER bmp_info;
  // get base header
  memcpy(&bmp_head, in_ptr, 14); in_ptr += 14;
  // check BM signature
  if(0 != memcmp(bmp_head.signature, "BM", 2))
    return nullptr;
  // get info header
  memcpy(&bmp_info, in_ptr, 40); in_ptr += 40;

  // we support only 24 or 32 bpp
  if(bmp_info.bpp < 24)
    return nullptr;

  // get BMP image parameters
  unsigned w = bmp_info.width;
  unsigned h = bmp_info.height;
  unsigned c = bmp_info.bpp / 8; // channel count

  // define some useful sizes
  size_t row_bytes = w * c;
  size_t tot_bytes = h * row_bytes;

  // allocate new buffer to receive rgb data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) return nullptr;

  // seek to bitmap data location
  in_ptr = in_data + bmp_head.offbits;

  // BMP data is natively stored upside down
  if(flip_y) {
    // read all data at once from
    memcpy(rgb, in_ptr, tot_bytes); in_ptr += tot_bytes;
  } else {
    // read rows in reverse order
    unsigned hmax = (h - 1);
    for(unsigned y = 0; y < h; ++y) {
      memcpy(rgb + (row_bytes * (hmax - y)), in_ptr, row_bytes);
      in_ptr += row_bytes;
    }
  }

  // finally swap components order BGR to RGB
  for(unsigned i = 0; i < tot_bytes; i += c)
    rgb[i  ] ^= rgb[i+2] ^= rgb[i  ] ^= rgb[i+2]; //< BGR => RGB

  // assign output values
  (*out_w) = w; (*out_h) = h; (*out_c) = c;

  return rgb;
}

/// \brief Write BMP file.
///
/// Write BMP data to file pointer.
///
/// \param[out] out_file  : File pointer to write to.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return True if operation succeed, false otherwise
///
static bool __bmp_write(FILE* out_file, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // compute data sizes
  size_t hdr_bytes = sizeof(OMM_BITMAPHEADER) + sizeof(OMM_BITMAPINFOHEADER);
  size_t row_bytes = in_w * in_c;                   //< row size in bytes
  size_t r4b_bytes = row_bytes + (row_bytes % 4);   //< row size rounded up to a multiple of 4 bytes
  size_t tot_bytes = r4b_bytes * in_h;
  size_t bmp_bytes = tot_bytes + hdr_bytes;

  // BMP headers structure
  OMM_BITMAPHEADER bmp_head = {};
  bmp_head.signature[0] = 0x42;
  bmp_head.signature[1] = 0x4D; // BM signature
  bmp_head.offbits = 54; // file header + info header = 54 bytes
  bmp_head.size = bmp_bytes;

  OMM_BITMAPINFOHEADER bmp_info = {};
  bmp_info.size = sizeof(OMM_BITMAPINFOHEADER);
  bmp_info.width = in_w;
  bmp_info.height = in_h;
  bmp_info.planes = 1;
  bmp_info.bpp = in_c * 8;
  bmp_info.compression = 0;
  bmp_info.sizeimage = tot_bytes;
  bmp_info.xppm = bmp_info.yppm = 0x0ec4;

  // make sure we start at begining
  fseek(out_file, 0, SEEK_SET);

  // write file header
  if(fwrite(&bmp_head, 1, sizeof(OMM_BITMAPHEADER), out_file) != sizeof(OMM_BITMAPHEADER))
    return false;
  // write info header
  if(fwrite(&bmp_info, 1, sizeof(OMM_BITMAPINFOHEADER), out_file) != sizeof(OMM_BITMAPINFOHEADER))
    return false;

  // allocate buffer for data translation
  uint8_t* row = reinterpret_cast<uint8_t*>(Om_alloc(r4b_bytes));
  if(!row) return false;

  // useful values for translation
  const uint8_t* sp;
  uint8_t* dp;
  unsigned hmax = (in_h - 1);

  for(unsigned y = 0; y < in_h; ++y) {
    sp = in_rgb + (row_bytes * (hmax - y)); // reverse row up to bottom
    dp = row;
    for(unsigned x = 0; x < in_w; ++x) {
      // convert RGBA to BGRA
      dp[0] = sp[2];
      dp[1] = sp[1];
      dp[2] = sp[0];
      if(in_c == 4) dp[3] = sp[3];
      sp += in_c;
      dp += in_c;
    }
    // write row to file
    if(fwrite(row, 1, r4b_bytes, out_file) != r4b_bytes) {
      Om_free(row); return false;
    }
  }

  Om_free(row);

  return true;
}

/// \brief Encode BMP data.
///
/// Encode BMP data to buffer in memory.
///
/// \param[out] out_size  : Output BMP data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return Pointer to encoded BMP image data or nullptr if failed.
///
static uint8_t* __bmp_encode(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // compute data sizes
  size_t hdr_bytes = sizeof(OMM_BITMAPHEADER) + sizeof(OMM_BITMAPINFOHEADER);
  size_t row_bytes = in_w * in_c;                   //< row size in bytes
  size_t r4b_bytes = row_bytes + (row_bytes % 4);   //< row size rounded up to a multiple of 4 bytes
  size_t tot_bytes = r4b_bytes * in_h;
  size_t bmp_bytes = tot_bytes + hdr_bytes;

  // BMP headers structure
  OMM_BITMAPHEADER bmp_head = {};
  bmp_head.signature[0] = 0x42; bmp_head.signature[1] = 0x4D; // BM signature
  bmp_head.offbits = hdr_bytes; // file header + info header = 54 bytes
  bmp_head.size = bmp_bytes;

  OMM_BITMAPINFOHEADER bmp_info = {};
  bmp_info.size = sizeof(OMM_BITMAPINFOHEADER);
  bmp_info.width = in_w;
  bmp_info.height = in_h;
  bmp_info.planes = 1;
  bmp_info.bpp = in_c * 8;
  bmp_info.compression = 0;
  bmp_info.sizeimage = tot_bytes;
  bmp_info.xppm = bmp_info.yppm = 0x0ec4;

  // allocate buffer for BMP data
  uint8_t* bmp = reinterpret_cast<uint8_t*>(Om_alloc(bmp_bytes));
  if(!bmp) return nullptr;

  // keep pointer to buffer
  uint8_t* bmp_ptr = bmp;

  // write file header
  memcpy(bmp_ptr, &bmp_head, sizeof(OMM_BITMAPHEADER));
  bmp_ptr += sizeof(OMM_BITMAPHEADER);
  // write info header
  memcpy(bmp_ptr, &bmp_info, sizeof(OMM_BITMAPINFOHEADER));
  bmp_ptr += sizeof(OMM_BITMAPINFOHEADER);

  // allocate buffer for data translation
  uint8_t* row = reinterpret_cast<uint8_t*>(Om_alloc(r4b_bytes));
  if(!row) {
    Om_free(bmp);
    return nullptr;
  }

  // useful values for translation
  const uint8_t* sp;
  uint8_t* dp;
  unsigned hmax = (in_h - 1);

  for(unsigned y = 0; y < in_h; ++y) {
    sp = in_rgb + (row_bytes * (hmax - y)); // reverse row up to bottom
    dp = row;
    for(unsigned x = 0; x < in_w; ++x) {
      // convert RGBA to BGRA
      dp[0] = sp[2];
      dp[1] = sp[1];
      dp[2] = sp[0];
      if(in_c == 4) dp[3] = sp[3];
      sp += in_c;
      dp += in_c;
    }

    // write row to buffer
    memcpy(bmp_ptr, row, r4b_bytes);
    bmp_ptr += r4b_bytes;
  }

  Om_free(row);

  (*out_size) = bmp_bytes;

  return bmp;
}

/// \brief Decode JPEG.
///
/// Common function to decode JPEG using the given GIF decoder structure.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  jpg_dec : JPEG decoder structure pointer.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB image data or nullptr if failed.
///
static uint8_t* __jpg_decode_common(unsigned* out_w, unsigned* out_h, unsigned* out_c, void* jpg_dec, bool flip_y)
{
  jpeg_decompress_struct* jpg = reinterpret_cast<jpeg_decompress_struct*>(jpg_dec);

  // read jpeg header
  if(jpeg_read_header(jpg, true) != 1)
    return nullptr;

  // initialize decompression
  jpeg_start_decompress(jpg);

  // get image parameters
	unsigned w = jpg->output_width;
	unsigned h = jpg->output_height;
	unsigned c = jpg->output_components;

	// define sizes
  size_t row_bytes = w * c;
  size_t tot_bytes = row_bytes * h;

  // allocate buffer to receive RGB data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) return nullptr;

  // row list pointer for jpeg decoder
  uint8_t* row_p[1];

  if(flip_y) {
    unsigned hmax = h - 1;
    while(jpg->output_scanline < jpg->output_height) {
      row_p[0] = rgb + ((hmax - jpg->output_scanline) * row_bytes);
      jpeg_read_scanlines(jpg, row_p, 1); //< read one row (scanline)
    }
  } else {
    while(jpg->output_scanline < jpg->output_height) {
      row_p[0] = rgb + (jpg->output_scanline * row_bytes);
      jpeg_read_scanlines(jpg, row_p, 1); //< read one row (scanline)
    }
  }

	// finalize reading
	jpeg_finish_decompress(jpg);

  // cleanup decoder
	jpeg_destroy_decompress(jpg);

	// assign output values
	(*out_w) = w; (*out_h) = h; (*out_c) = c;

	return rgb;
}

/// \brief Encode JEPG.
///
/// Common function to encode JEPG using the given JEPG encoder structure.
///
/// \param[in]  jpg_enc : JEPG encoder structure pointer.
/// \param[in]  in_rgb  : Input image RGB(A) data to encode.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count.
/// \param[in]  level   : JPEG compression quality level 0 to 100.
///
/// \return True if operation succeed, false otherwise
///
static bool __jpg_encode_common(void* jpg_enc, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  jpeg_compress_struct* jpg = reinterpret_cast<jpeg_compress_struct*>(jpg_enc);

  // clamp quality value
  if(level < 0) level = 0;
  if(level > 100) level = 100;

  // define image parameters
  jpg->image_width = in_w;        //< Image width in pixels
  jpg->image_height = in_h;       //< Image height in pixels
  jpg->input_components = 3;      //< per pixel color components
  jpg->in_color_space = JCS_RGB;  //< pixel format

  // set parameters to encoder
  jpeg_set_defaults(jpg);
  // set compression quality
  jpeg_set_quality(jpg, level, true); // quality is 0-100 scaled
  // initialize encoder
  jpeg_start_compress(jpg, true);

  // hold row size in bytes
  unsigned row_bytes = in_w * in_c;

  if(in_c == 4) {

    // JPEG encoder does not handle RGBA source we must convert data
    const uint8_t* sp;
    uint8_t* dp;

    // create new buffer for one RGB row
    uint8_t* row = reinterpret_cast<uint8_t*>(Om_alloc(in_w * 3));
    if(!row) return false;

    // give RGB data to JPEG encoder
    while(jpg->next_scanline < jpg->image_height) {
      // set source and destination pointers
      sp = in_rgb + (jpg->next_scanline * row_bytes);
      dp = row;
      // convert RGBA to RGB
      for(unsigned i = 0; i < in_w; ++i) {
        dp[0] = sp[0];
        dp[1] = sp[1];
        dp[2] = sp[2];
        sp += 4;
        dp += 3;
      }
      // send to encoder
      jpeg_write_scanlines(jpg, &row, 1);
    }

    Om_free(row);

  } else {

    // pointer to source RGB row
    uint8_t* row_p;

    // give RGB data to JPEG encoder
    while(jpg->next_scanline < jpg->image_height) {
      // get pointer to rows
      row_p = const_cast<uint8_t*>(in_rgb + (jpg->next_scanline * row_bytes));
      // send to encoder
      jpeg_write_scanlines(jpg, &row_p, 1);
    }
  }

  // finalize compression
  jpeg_finish_compress(jpg);
  // destroy encoder
  jpeg_destroy_compress(jpg);

  return true;
}

/// \brief Read JPEG file.
///
/// Read JPEG data from file pointer.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_file : Input file pointer to read data from.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGB data or nullptr if failed.
///
static uint8_t* __jpg_read(unsigned* out_w, unsigned* out_h, unsigned* out_c, FILE* in_file, bool flip_y)
{
  // create base object for jpeg decoder
  jpeg_decompress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg decoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&jpg);

  // make sure we start at beginning and setup jpeg IO
  fseek(in_file, 0, SEEK_SET);
  jpeg_stdio_src(&jpg, in_file);

  return __jpg_decode_common(out_w, out_h, out_c, &jpg, flip_y);
}

/// \brief Decode JPEG data.
///
/// Decode JPEG data from buffer in memory.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_data : Input JPEG data to decode.
/// \param[in]  in_size : Input JPEG data size in bytes.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGB data or nullptr if failed.
///
static uint8_t* __jpg_decode(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, size_t in_size, bool flip_y)
{
  // create base object for jpeg decoder
  jpeg_decompress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg decoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&jpg);

  // set read data pointer
  jpeg_mem_src(&jpg, in_data, in_size);

  return __jpg_decode_common(out_w, out_h, out_c, &jpg, flip_y);
}

/// \brief Write JPEG file.
///
/// Write JPEG data to file pointer.
///
/// \param[out] out_file  : File pointer to write to.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : JPEG compression quality level 0 to 10.
///
/// \return True if operation succeed, false otherwise
///
static bool __jpg_write(FILE* out_file, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // create base object for jpeg encoder
  jpeg_compress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg encoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&jpg);

  // make sure we start at beginning and setup jpeg IO
  fseek(out_file, 0, SEEK_SET);
  jpeg_stdio_dest(&jpg, out_file);

  return __jpg_encode_common(&jpg, in_rgb, in_w, in_h, in_c, level * 10);
}

/// \brief Encode JPEG data.
///
/// Encode JPEG data to buffer in memory.
///
/// \param[out] out_size  : Output JPEG data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : JPEG compression quality level 0 to 10.
///
/// \return Pointer to decoded RGB(A) image data or nullptr if failed.
///
static uint8_t* __jpg_encode(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // create base object for jpeg encoder
  jpeg_compress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg encoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&jpg);

  // pointer to be allocated
  uint8_t* jpg_data = nullptr;

  // set pointer params
  unsigned long jpg_size = 0;
  jpeg_mem_dest(&jpg, &jpg_data, &jpg_size);

  if(!__jpg_encode_common(&jpg, in_rgb, in_w, in_h, in_c, level * 10))
    return nullptr;

  (*out_size) = jpg_size;

  return jpg_data;
}

/// \brief Decode PNG.
///
/// Common function to decode PNG using the given PNG decoder structure.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  png_dec : PNG decoder structure pointer.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB(A) image data or nullptr if failed.
///
static uint8_t* __png_decode_common(unsigned* out_w, unsigned* out_h, unsigned* out_c, void* png_dec, bool flip_y)
{
  // get decoder
  png_structp png = reinterpret_cast<png_structp>(png_dec);

  // create PNG info structure
  png_infop png_info = png_create_info_struct(png);

  // get image properties
  png_read_info(png, png_info);
  unsigned w = png_get_image_width(png, png_info);
  unsigned h = png_get_image_height(png, png_info);
  unsigned c = png_get_channels(png, png_info);

  // retrieve and define useful sizes
  size_t row_bytes = png_get_rowbytes(png, png_info);
  size_t tot_bytes = h * row_bytes;

  // allocate pointer to receive RGB(A) data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) return nullptr;

  // allocate list of pointers for output RGB(A) rows
  uint8_t** rows_p = reinterpret_cast<uint8_t**>(Om_alloc(sizeof(void*)*h));
  if(!rows_p) {
    Om_free(rgb);
    return nullptr;
  }

  // define pointers to each row in output RGB(A) data
  if(flip_y) {
    unsigned hmax = h - 1;
    for(unsigned y = 0; y < h; ++y)
      rows_p[y] = rgb + ((hmax - y) * row_bytes);
  } else {
    for(unsigned y = 0; y < h; ++y)
      rows_p[y] = rgb + (y * row_bytes);
  }

  // read all rows at once
  png_read_image(png, rows_p);

  // cleanup
  png_destroy_read_struct(&png, &png_info, nullptr);

  // delete list of pointers
  Om_free(rows_p);

  // assign output values
  (*out_w) = w; (*out_h) = h; (*out_c) = c;

  return rgb;
}

/// \brief Encode PNG.
///
/// Common function to encode PNG using the given PNG encoder structure.
///
/// \param[in]  png_enc : PNG encoder structure pointer.
/// \param[in]  in_rgb  : Input image RGB(A) data to encode.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count.
/// \param[in]  level   : PNG compression level 0 to 9.
///
/// \return True if operation succeed, false otherwise
///
static bool __png_encode_common(void* png_enc, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // get encoder
  png_structp png = reinterpret_cast<png_structp>(png_enc);

  // create PNG info structure
  png_infop png_info = png_create_info_struct(png);

  // set PNG parameters
  png_set_IHDR(png, png_info, in_w, in_h, 8,
               (in_c == 4) ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  // clamp compression level
  if(level < 0) level = 0;
  if(level > 9) level = 9;

  // set compression level
  png_set_compression_level(png, level);

  // write info to PNG
  png_write_info(png, png_info);

  // define useful sizes
  size_t row_bytes = in_w * in_c;

  // allocate list of pointers for input RGB(A) rows
  const uint8_t** rows_p = reinterpret_cast<const uint8_t**>(Om_alloc(sizeof(void*)*in_h));
  if(!rows_p) {
    png_destroy_write_struct(&png, &png_info);
    png_free_data(png, png_info, PNG_FREE_ALL, -1);
    return false;
  }

  // define pointers to each row in input RGB(A) data
  for(unsigned y = 0; y < in_h; ++y)
    rows_p[y] = in_rgb + (y * row_bytes);

  // supply all data at once to encoder
  png_write_image(png, const_cast<uint8_t**>(rows_p));

  // finalize write process
  png_write_end(png, nullptr);

  // clear PGN encoder
  png_destroy_write_struct(&png, &png_info);
  png_free_data(png, png_info, PNG_FREE_ALL, -1);

  // delete list of pointers
  Om_free(rows_p);

  return true;
}

/// \brief Read PNG file.
///
/// Read PNG data from file pointer.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_file : Input file pointer to read data from.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGB(A) data or nullptr if failed.
///
static uint8_t* __png_read(unsigned* out_w, unsigned* out_h, unsigned* out_c, FILE* in_file, bool flip_y)
{
  // create PNG decoder structure
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // make sure we start at beginning and setup png IO
  fseek(in_file, 0, SEEK_SET);
  png_init_io(png, in_file);

  // decode PNG data
  return __png_decode_common(out_w, out_h, out_c, png, flip_y);
}

/// \brief Custom PNG read struct
///
/// Custom structure for custom PNG read function
///
struct __png_read_st {
  const uint8_t*  src_data;
  size_t          src_seek;
};

/// \brief Custom PNG read
///
/// Custom read function for PNG library to read from memory.
///
/// \param[in]  png     Decoder structure pointer.
/// \param[in]  dst     Destination buffer.
/// \param[in]  len     Length of data that should be read.
///
void __png_read_buff_fn(png_structp png, uint8_t* dst, size_t len)
{
  __png_read_st *read_st = static_cast<__png_read_st*>(png_get_io_ptr(png));
  memcpy(dst, read_st->src_data + read_st->src_seek, len);
  read_st->src_seek += len;
}

/// \brief Decode PNG.
///
/// Decode PNG data from buffer in memory.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_data : Input PNG data to decode.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGB(A) data or nullptr if failed.
///
static uint8_t* __png_decode(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, bool flip_y)
{
  // create PNG decoder structure
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // custom read structure
  __png_read_st read_st;
  read_st.src_data = in_data;
  read_st.src_seek = 0;

  // set custom read process
  png_set_read_fn(png, &read_st, __png_read_buff_fn);

  // decode PNG data
  return __png_decode_common(out_w, out_h, out_c, png, flip_y);
}

/// \brief Write PNG file.
///
/// Write PNG data to file pointer.
///
/// \param[out] out_file  : File pointer to write to.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : PNG compression level 0 to 9.
///
/// \return True if operation succeed, false otherwise
///
static bool __png_write(FILE* out_file, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // create PNG encoder structure
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // make sure we start at beginning and setup png IO
  fseek(out_file, 0, SEEK_SET);
  png_init_io(png, out_file);

  return __png_encode_common(png, in_rgb, in_w, in_h, in_c, level);
}

/// \brief Custom PNG write struct
///
/// Custom structure for custom PNG write function
///
struct __png_write_st {
  uint8_t*  dst_data;
  size_t    dst_size;
  size_t    dst_seek;
};

/// \brief Custom PNG writer
///
/// Custom write function for PNG library to encode to memory.
///
/// \param[in]  png     Decoder structure pointer.
/// \param[in]  src     Source data buffer.
/// \param[in]  len     Length of data that should be written.
///
void __png_write_buff_fn(png_structp png, uint8_t* src, size_t len)
{
  __png_write_st *write_st = static_cast<__png_write_st*>(png_get_io_ptr(png));
  write_st->dst_size += len;
  if(write_st->dst_data) {
    write_st->dst_data = reinterpret_cast<uint8_t*>(Om_realloc(write_st->dst_data, write_st->dst_size));
  } else {
    write_st->dst_data = reinterpret_cast<uint8_t*>(Om_alloc(write_st->dst_size));
  }
  if(!write_st->dst_data) png_error(png, "alloc error in __png_write_fn");
  memcpy(write_st->dst_data + write_st->dst_seek, src, len);
  write_st->dst_seek += len;
}

/// \brief Custom PNG flush function
///
/// Custom callback function for PNG encoder flush.
///
void __png_flush_fn(png_structp png)
{
}

/// \brief Encode PNG data.
///
/// Common function to encode PNG using the given PNG encoder structure.
///
/// \param[in]  out_size  : Pointer to receive encoded data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count.
/// \param[in]  level     : PNG compression level 0 to 9.
///
/// \return Pointer to encoded PNG data
///
static uint8_t* __png_encode(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // create PNG encoder structure
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // custom write structure
  __png_write_st write_st;
  write_st.dst_data = nullptr;
  write_st.dst_size = 0;
  write_st.dst_seek = 0;

  // custom write process
  png_set_write_fn(png, &write_st, __png_write_buff_fn, __png_flush_fn);

  if(!__png_encode_common(png, in_rgb, in_w, in_h, in_c, level))
    return nullptr;

  (*out_size) = write_st.dst_size;

  return write_st.dst_data;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_imageType(uint8_t* data)
{
  // identify image format
  return __image_sign_matches(data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_imageType(FILE* file)
{
  // read first 8 bytes of the file
  uint8_t buff[8];
  fseek(file, 0, SEEK_SET);
  if(fread(buff, 1, 8, file) < 8) return -1;

  // identify image format
  return __image_sign_matches(buff);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_loadImage(unsigned* out_w, unsigned* out_h, unsigned* out_c, const wstring& in_path, bool flip_y)
{
  // initialize output values
  (*out_w) = 0;
  (*out_h) = 0;
  (*out_c) = 0;

  // Open file for writing
  FILE* in_file;
  if((in_file = _wfopen(in_path.c_str(), L"rb")) == nullptr)
    return nullptr;

  // read first 8 bytes of the file
  uint8_t buff[8];
  fseek(in_file, 0, SEEK_SET);
  if(fread(buff, 1, 8, in_file) < 8)
    return nullptr;

  // identify image format
  int type = __image_sign_matches(buff);

  // load according image format
  if(type != 0) {
    switch(type)
    {
    case OMM_IMAGE_TYPE_BMP:
      return __bmp_read(out_w, out_h, out_c, in_file, flip_y);
    case OMM_IMAGE_TYPE_JPG:
      return __jpg_read(out_w, out_h, out_c, in_file, flip_y);
    case OMM_IMAGE_TYPE_PNG:
      return __png_read(out_w, out_h, out_c, in_file, flip_y);
    case OMM_IMAGE_TYPE_GIF:
      return __gif_read(out_w, out_h, out_c, in_file, flip_y);
    }
  }

  fclose(in_file);

  return nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_loadImage(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, size_t in_size, bool flip_y)
{
  // initialize output values
  (*out_w) = 0;
  (*out_h) = 0;
  (*out_c) = 0;

  // prevent idiot attempts
  if(!in_data || !in_size)
    return nullptr;

  // identify image format
  int type = __image_sign_matches(in_data);

  // load according image format
  if(type != 0) {
    switch(type)
    {
    case OMM_IMAGE_TYPE_BMP:
      return __bmp_decode(out_w, out_h, out_c, in_data, flip_y);
    case OMM_IMAGE_TYPE_JPG:
      return __jpg_decode(out_w, out_h, out_c, in_data, in_size, flip_y);
    case OMM_IMAGE_TYPE_PNG:
      return __png_decode(out_w, out_h, out_c, in_data, flip_y);
    case OMM_IMAGE_TYPE_GIF:
      return __gif_decode(out_w, out_h, out_c, in_data, flip_y);
    }
  }

  return nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_saveBmp(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return false;

  // open file
  FILE* out_file;

  //if((fp = fopen(Om_toUtf8(out_path).c_str(), "rb")) == nullptr) {
  if((out_file = _wfopen(out_path.c_str(), L"wb")) == nullptr)
    return false;

  bool result = __bmp_write(out_file, in_rgb, in_w, in_h, in_c);

  fclose(out_file);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_saveJpg(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return false;

  // open file
  FILE* out_file;

  //if((fp = fopen(Om_toUtf8(out_path).c_str(), "rb")) == nullptr) {
  if((out_file = _wfopen(out_path.c_str(), L"wb")) == nullptr)
    return false;

  bool result = __jpg_write(out_file, in_rgb, in_w, in_h, in_c, level);

  fclose(out_file);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_savePng(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return false;

  // open file
  FILE* out_file;

  //if((fp = fopen(Om_toUtf8(out_path).c_str(), "rb")) == nullptr) {
  if((out_file = _wfopen(out_path.c_str(), L"wb")) == nullptr)
    return false;

  bool result = __png_write(out_file, in_rgb, in_w, in_h, in_c, level);

  fclose(out_file);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_saveGif(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return false;

  // open file
  FILE* out_file;

  //if((fp = fopen(Om_toUtf8(out_path).c_str(), "rb")) == nullptr) {
  if((out_file = _wfopen(out_path.c_str(), L"wb")) == nullptr)
    return false;

  bool result = __gif_write(out_file, in_rgb, in_w, in_h, in_c);

  fclose(out_file);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_encodeBmp(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __bmp_encode(out_size, in_rgb, in_w, in_h, in_c);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_encodeJpg(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __jpg_encode(out_size, in_rgb, in_w, in_h, in_c, level);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_encodePng(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __png_encode(out_size, in_rgb, in_w, in_h, in_c, level);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_encodeGif(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __gif_encode(out_size, in_rgb, in_w, in_h, in_c);
}


/// \brief Compute cubic interpolation.
///
/// Compute cubic the interpolation of the specified values.
///
/// \param[in]  a : Start handle
/// \param[in]  b : Start node
/// \param[in]  c : End node
/// \param[in]  d : End handle
/// \param[in]  t : Interpolation phase
///
/// \return cubic interpolated value
///
static inline float __interp_cubic(float a, float b, float c, float d, float t)
{
  return b + 0.5f * t * (c - a + t * (2.0f * a - 5.0f * b + 4.0f * c - d + t * (3.0f * (b - c) + d - a)));
}

/// \brief Get BiCubic interpolated pixel.
///
/// Compute the BiCubic interpolated pixel at the specified coordinates.
///
/// \param[in]  out_pix : Array to receive interpolated pixel components.
/// \param[in]  u       : Sample horizontal coordinate in image, from 0.0 to 1.0.
/// \param[in]  v       : Sample vertical coordinate in image, from 0.0 to 1.0.
/// \param[in]  in_rgb  : Input image RGB(A) data.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count (bytes per pixel).
///
static inline void __image_sample_bicubic(uint8_t* out_pix, float u, float v, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  float x, y;

  float xf = modf((u * in_w) - 0.5f, &x);
  float yf = modf((v * in_h) - 0.5f, &y);

  int ix = static_cast<int>(x) - 1;
  int iy = static_cast<int>(y) - 1;

  const uint8_t* sp[4];

  float r[4]; float g[4]; float b[4]; float a[4];

  int x_b, y_b;
  int hmax = in_h - 1;
  int wmax = in_w - 1;

  for(int j = 0; j < 4; ++j) {
    y_b = (std::max(0, std::min(iy + j, hmax)) * in_w) * in_c;
    for(int i = 0; i < 4; ++i) {
      x_b = std::max(0, std::min(ix + i, wmax)) * in_c;
      sp[i] = in_rgb + (y_b + x_b);
    }
    r[j] = __interp_cubic(sp[0][0], sp[1][0], sp[2][0], sp[3][0], xf);
    g[j] = __interp_cubic(sp[0][1], sp[1][1], sp[2][1], sp[3][1], xf);
    b[j] = __interp_cubic(sp[0][2], sp[1][2], sp[2][2], sp[3][2], xf);
    if(in_c == 4) a[j] = __interp_cubic(sp[0][3], sp[1][3], sp[2][3], sp[3][3], xf);
  }

  float m[4];
  m[0] = __interp_cubic(r[0], r[1], r[2], r[3], yf);
  m[1] = __interp_cubic(g[0], g[1], g[2], g[3], yf);
  m[2] = __interp_cubic(b[0], b[1], b[2], b[3], yf);
  if(in_c == 4) m[3] = __interp_cubic(a[0], a[1], a[2], a[3], yf);

  for(unsigned i = 0; i < in_c; ++i) {
    out_pix[i] = static_cast<uint8_t>(std::max(0.0f, std::min(m[i], 255.0f)));
  }
}

/// \brief Get box interpolated pixel.
///
/// Compute the box interpolated pixel at the specified coordinates.
///
/// \param[in]  out_pix : Array to receive interpolated pixel components.
/// \param[in]  box_w   : Box width in pixels.
/// \param[in]  box_h   : Box height in pixels.
/// \param[in]  u       : Sample horizontal coordinate in image, from 0.0 to 1.0.
/// \param[in]  v       : Sample vertical coordinate in image, from 0.0 to 1.0.
/// \param[in]  in_rgb  : Input image RGB(A) data.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count (bytes per pixel).
///
static inline void __image_sample_box(uint8_t* out_pix, int box_w, int box_h, float u, float v, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  float r = 0.0f; float g = 0.0f; float b = 0.0f; float a = 0.0f;

  const uint8_t *sp;

  int x_b, y_b;
  int wmax = (in_w - 1);
  int hmax = (in_h - 1);

  int x = u * wmax;
  int y = v * hmax;

  for(int j = 0; j < box_h; ++j) {
    y_b = (std::max(0, std::min(y + j, hmax)) * in_w) * in_c;
    for(int i = 0; i < box_w; ++i) {
      x_b = std::max(0, std::min(x + i, wmax)) * in_c;
      sp = in_rgb + (y_b + x_b);
      r += sp[0]; g += sp[1]; b += sp[2];
      if(in_c == 4) a += sp[3];
    }
  }

  float f = box_h * box_w;

  out_pix[0] = r / f; out_pix[1] = g / f; out_pix[2] = b / f;
  if(in_c == 4) out_pix[3] = a / f;
}

/// \brief Box filter downsample image.
///
/// Reduce image resolution using box filtering.
///
/// \param[out] out_rgb : Output destination pointer.
/// \param[in]  w       : target width.
/// \param[in]  w       : target height.
/// \param[in]  in_rgb  : Source image RGB(A) data.
/// \param[in]  in_w    : Source image width.
/// \param[in]  in_h    : source image height.
/// \param[in]  in_c    : Source image component count (bytes per pixel)
///
static void __image_dsample(uint8_t* out_rgb, unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  unsigned box_w = floor(static_cast<float>(in_w) / w);
  unsigned box_h = floor(static_cast<float>(in_h) / h);

  float inv_w = 1.0f / (static_cast<float>(w) - 1);
  float inv_h = 1.0f / (static_cast<float>(h) - 1);

  uint8_t smpl[4];

  uint8_t* dp;
  float u, v;

  for(unsigned y = 0; y < h; ++y) {
    dp = out_rgb + ((w * in_c) * y);
    v = y * inv_h;
    for(unsigned x = 0; x < w; ++x) {
      u = x * inv_w;
      __image_sample_box(smpl, box_w, box_h, u, v, in_rgb, in_w, in_h, in_c);
      dp[0] = smpl[0]; dp[1] = smpl[1]; dp[2] = smpl[2];
      if(in_c == 4) dp[3] = smpl[3];
      dp += in_c;
    }
  }
}

/// \brief Bicubic filter upsample image
///
/// Increase image resolution using bicubic filtering.
///
/// \param[out] out_rgb : Output destination pointer.
/// \param[in]  w       : target width.
/// \param[in]  w       : target height.
/// \param[in]  in_rgb  : Source image RGB(A) data.
/// \param[in]  in_w    : Source image width.
/// \param[in]  in_h    : source image height.
/// \param[in]  in_c    : Source image component count (bytes per pixel)
///
static void __image_usample(uint8_t* out_rgb, unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  float inv_w = 1.0f / (static_cast<float>(w) - 1);
  float inv_h = 1.0f / (static_cast<float>(h) - 1);

  uint8_t smpl[4];

  uint8_t* dp;
  float u, v;

  for(unsigned y = 0; y < h; ++y) {
    dp = out_rgb + ((w * in_c) * y);
    v = y * inv_h;
    for(unsigned x = 0; x < w; ++x) {
      u = x * inv_w;
      __image_sample_bicubic(smpl, u, v, in_rgb, in_w, in_h, in_c);
      dp[0] = smpl[0]; dp[1] = smpl[1]; dp[2] = smpl[2];
      if(in_c == 4) dp[3] = smpl[3];
      dp += in_c;
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_resizeImage(unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!w  || !h || !in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  // define useful sizes
  size_t out_row_bytes = w * in_c;

  uint8_t* out_rgb = reinterpret_cast<uint8_t*>(Om_alloc(out_row_bytes * h));
  if(!out_rgb) return nullptr;

  // resize image to fit desired square
  if(in_w != w || in_h != h) {

    if(w > in_w || h > in_h) {
      __image_usample(out_rgb, w, h, in_rgb, in_w, in_h, in_c);
    } else {
      __image_dsample(out_rgb, w, h, in_rgb, in_w, in_h, in_c);
    }

  } else {

    memcpy(out_rgb, in_rgb, (in_w * in_c) * in_h);
  }

  return out_rgb;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_cropImage(unsigned x, unsigned y, unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!w  || !h || !in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  if((x + w) > in_w || (y + h) > in_h)
    return nullptr;

  // define useful sizes
  size_t in_row_bytes = in_w * in_c;
  size_t out_row_bytes = w * in_c;

  // allocate new buffer for cropped data
  uint8_t* out_rgb = reinterpret_cast<uint8_t*>(Om_alloc(out_row_bytes * h));
  if(!out_rgb) return nullptr;

  // copy required RGB data
  const uint8_t* sp;
  uint8_t* dp;

  unsigned x_shift = (x * in_c);

  for(unsigned j = 0; j < h; ++j) {

    dp = out_rgb + (out_row_bytes * j);
    sp = in_rgb + ((in_row_bytes * (j + y)) + x_shift);

    for(unsigned i = 0; i < w; ++i) {

      dp[0] = sp[0]; dp[1] = sp[1]; dp[2] = sp[2];
      if(in_c == 4) dp[3] = sp[3];

      dp += in_c; sp += in_c;
    }
  }

  return out_rgb;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_thumbnailImage(unsigned size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!size || !in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  // create locale copy of original data
  uint8_t* out_rgb = reinterpret_cast<uint8_t*>(Om_alloc((in_w * in_c) * in_h));
  if(!out_rgb) return nullptr;

  memcpy(out_rgb, in_rgb, (in_w * in_c) * in_h);

  // resize image to fit desired size
  if(in_w != size || in_h != size) {

    unsigned out_w, out_h;
    // determine target size according image aspect ratio
    float a = static_cast<float>(in_w) / in_h;
    if(a > 1.0f) {
      out_w = static_cast<float>(size) * a;
      out_h = size;
    } else {
      out_w = size;
      out_h = static_cast<float>(size) / a;
    }

    uint8_t* tmp_rgb = Om_resizeImage(out_w, out_h, out_rgb, in_w, in_h, in_c);
    if(tmp_rgb == nullptr) {
      return nullptr;
    }

    // swap buffers
    Om_free(out_rgb);
    out_rgb = tmp_rgb;

    // update input width and height
    in_h = out_h;
    in_w = out_w;
  }

  // crop image to square
  if(in_w != in_h) {

    unsigned x, y;

    if(in_w > in_h) {
      x = (in_w * 0.5f) - (in_h * 0.5f);
      y = 0;
    } else {
      x = 0;
      y = (in_h * 0.5f) - (in_w * 0.5f);
    }

    uint8_t* tmp_rgb = Om_cropImage(x, y, size, size, out_rgb, in_w, in_h, in_c);
    if(tmp_rgb == nullptr) {
      return nullptr;
    }

    // swap buffers
    Om_free(out_rgb);
    out_rgb = tmp_rgb;
  }

  return out_rgb;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_hbitmapImage(const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  // BITMAP with 24 bits pixels data seem to be always interpreted with
  // transparent alpha once supplied to STATIC control, so we always
  // convert to 32 bits pixels data.

  // allocate buffer for 32 bits BMP data
  uint8_t* bmp = reinterpret_cast<uint8_t*>(Om_alloc((in_w * 4) * in_h));
  if(!bmp) return nullptr;

  if(in_c == 4) {

    // compute data sizes
    size_t tot_bytes = (in_w * in_c) * in_h;

    //copy RGBA data to temp buffer
    memcpy(bmp, in_rgb, tot_bytes);

    // swap pixels components from RGBA to BGRA
    for(unsigned i = 0; i < tot_bytes; i += 4)
      bmp[i  ] ^= bmp[i+2] ^= bmp[i  ] ^= bmp[i+2]; //< RGB => BGR

  } else {

    // compute data sizes
    size_t row3_bytes = in_w * 3;
    size_t row4_bytes = in_w * 4;

    // pointers for translation
    const uint8_t* sp;
    uint8_t* dp;

    for(unsigned y = 0; y < in_h; ++y) {

      sp = in_rgb + (row3_bytes * y);
      dp = bmp + (row4_bytes * y);

      for(unsigned x = 0; x < in_w; ++x) {

        // convert RGBA to BGRA
        dp[0] = sp[2];
        dp[1] = sp[1];
        dp[2] = sp[0];
        dp[3] = 0xFF;
        sp += 3;
        dp += 4;
      }
    }
  }

  HBITMAP hBmp = CreateBitmap(in_w, in_h, 1, 32, bmp);

  Om_free(bmp);

  return hBmp;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HICON Om_loadShellIcon(unsigned id, bool large)
{
  SHSTOCKICONINFO sIi = {};
  sIi.cbSize = sizeof(SHSTOCKICONINFO);

  SHGetStockIconInfo(static_cast<SHSTOCKICONID>(id),
                    (large) ? SHGSI_ICON|SHGSI_LARGEICON : SHGSI_ICON|SHGSI_SMALLICON,
                    &sIi);
  return sIi.hIcon;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_loadShellBitmap(unsigned id, bool large)
{
  SHSTOCKICONINFO sIi = {};
  sIi.cbSize = sizeof(SHSTOCKICONINFO);

  SHGetStockIconInfo(static_cast<SHSTOCKICONID>(id),
                    (large) ? SHGSI_ICON|SHGSI_LARGEICON : SHGSI_ICON|SHGSI_SMALLICON,
                    &sIi);

  ICONINFO iCi = {};
  GetIconInfo(sIi.hIcon, &iCi);
  HBITMAP result = static_cast<HBITMAP>(CopyImage(iCi.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_COPYDELETEORG));
  DestroyIcon(sIi.hIcon);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HFONT Om_createFont(unsigned pt, unsigned weight, const wchar_t* name)
{
  return CreateFontW( pt, 0, 0, 0, weight,
                      false, false, false,
                      ANSI_CHARSET,
                      OUT_TT_PRECIS, 0, CLEARTYPE_QUALITY, 0,
                      name);
}

#include "thirdparty/zlib/zlib.h"


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


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_getErrorStr(int code) {

  wchar_t num_buf[32];
  swprintf(num_buf, 32, L"%x", code);

  wstring ret = L"(0x"; ret.append(num_buf); ret.append(L") ");

  switch(code)
  {
  case ERROR_FILE_NOT_FOUND: ret.append(L"FILE_NOT_FOUND"); break;
  case ERROR_PATH_NOT_FOUND: ret.append(L"PATH_NOT_FOUND"); break;
  case ERROR_TOO_MANY_OPEN_FILES: ret.append(L"TOO_MANY_OPEN_FILES"); break;
  case ERROR_ACCESS_DENIED: ret.append(L"ACCESS_DENIED"); break;
  case ERROR_ARENA_TRASHED: ret.append(L"ARENA_TRASHED"); break;
  case ERROR_NOT_ENOUGH_MEMORY: ret.append(L"NOT_ENOUGH_MEMORY"); break;
  case ERROR_INVALID_BLOCK: ret.append(L"INVALID_BLOCK"); break;
  case ERROR_INVALID_ACCESS: ret.append(L"INVALID_ACCESS"); break;
  case ERROR_INVALID_DATA: ret.append(L"INVALID_DATA"); break;
  case ERROR_OUTOFMEMORY: ret.append(L"OUTOFMEMORY"); break;
  case ERROR_INVALID_DRIVE: ret.append(L"INVALID_DRIVE"); break;
  case ERROR_CURRENT_DIRECTORY: ret.append(L"CURRENT_DIRECTORY"); break;
  case ERROR_NOT_SAME_DEVICE: ret.append(L"NOT_SAME_DEVICE"); break;
  case ERROR_WRITE_PROTECT: ret.append(L"WRITE_PROTECT"); break;
  case ERROR_CRC: ret.append(L"ERROR_CRC"); break;
  case ERROR_SEEK: ret.append(L"ERROR_SEEK"); break;
  case ERROR_WRITE_FAULT: ret.append(L"WRITE_FAULT"); break;
  case ERROR_READ_FAULT: ret.append(L"READ_FAULT"); break;
  case ERROR_SHARING_VIOLATION: ret.append(L"SHARING_VIOLATION"); break;
  case ERROR_LOCK_VIOLATION: ret.append(L"LOCK_VIOLATION"); break;
  case ERROR_WRONG_DISK: ret.append(L"WRONG_DISK"); break;
  case ERROR_HANDLE_DISK_FULL: ret.append(L"HANDLE_DISK_FULL"); break;
  case ERROR_FILE_EXISTS: ret.append(L"FILE_EXISTS"); break;
  case ERROR_DRIVE_LOCKED: ret.append(L"DRIVE_LOCKED"); break;
  case ERROR_OPEN_FAILED: ret.append(L"OPEN_FAILED"); break;
  case ERROR_BUFFER_OVERFLOW: ret.append(L"BUFFER_OVERFLOW"); break;
  case ERROR_DISK_FULL: ret.append(L"DISK_FULL"); break;
  case ERROR_INVALID_NAME: ret.append(L"INVALID_NAME"); break;
  case ERROR_DIR_NOT_EMPTY: ret.append(L"DIR_NOT_EMPTY"); break;
  case ERROR_ALREADY_EXISTS: ret.append(L"ALREADY_EXISTS"); break;
  }

  return ret;
}
