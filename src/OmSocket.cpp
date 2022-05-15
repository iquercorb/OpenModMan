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
#include "OmUtilStr.h"

#include <curl/curl.h>


///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmSocket.h"


/// \brief Initialized libCURL flag
///
/// Flag to tell whether libCURL must be initialized
///
static bool __curl_initialized = false;


/// \brief Initialize libCURL
///
/// Function to initialize libCURL once
///
static inline void __curl_init()
{
  if(!__curl_initialized) {

    curl_global_init(CURL_GLOBAL_ALL);

    // we need to initialize only once per process
    __curl_initialized = true;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmSocket::OmSocket() :
  _hcurl(nullptr),_buff_fill(0),_buff_size(0),_buff_data(nullptr),_out_file(nullptr),
  _user_download(nullptr),_user_ptr(nullptr),_downloading(false),_rate_byte(0),
  _rate_time(0.0),_progress_bps(0.0),_progress_tot(0),_progress_now(0),_ercode(0),
  _tpcode(0)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmSocket::~OmSocket()
{
  this->clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmSocket::httpGet(const wstring& url, string& data)
{
  this->clear();

  __curl_init(); //< initialize LibCURL (done only once)

  this->_hcurl = curl_easy_init();

  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_URL, Om_toUTF8(url).c_str());

  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_HTTPGET, 1L);

  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_WRITEFUNCTION, OmSocket::_writeMemCb);
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_WRITEDATA, this);

  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_NOPROGRESS, 1L);

  // follow HTTP redirections
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_FOLLOWLOCATION, 1L);

  // we use a self-signed test server, skip verification during debugging
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_FAILONERROR, 1L);

  // start timer
  this->_rate_time = clock();

  this->_ercode = curl_easy_perform(reinterpret_cast<CURL*>(this->_hcurl));

  this->_downloading = false;

  if(this->_ercode == CURLE_OK) {

    // If needed, realloc to add the null character
    if((this->_buff_fill + 1) > this->_buff_size) {
      this->_buff_size++;
      if((this->_buff_data = reinterpret_cast<uint8_t*>(Om_realloc(this->_buff_data, this->_buff_size))) == nullptr)
        throw std::bad_alloc();
    }

    // add terminal null character to ensure we have a valid string
    this->_buff_data[this->_buff_fill] = 0;
    this->_buff_fill++;

    data = reinterpret_cast<char*>(_buff_data);
  } else {
    this->_tpcode = 0;
    curl_easy_getinfo(reinterpret_cast<CURL*>(this->_hcurl), CURLINFO_RESPONSE_CODE, &this->_tpcode);
  }

  this->clear();

  return (this->_ercode == CURLE_OK);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmSocket::httpGet(const wstring& url, FILE* file, Om_downloadCb download_cb, void* user_ptr)
{
  this->clear();

  __curl_init(); //< initialize LibCURL (done only once)

  this->_hcurl = curl_easy_init();

  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_URL, Om_toUTF8(url).c_str());

  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_HTTPGET, 1L);

  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_WRITEFUNCTION, OmSocket::_writeFileCb);
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_WRITEDATA, this);

  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_XFERINFOFUNCTION, OmSocket::_progressCb);
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_XFERINFODATA, this);

  // follow HTTP redirections
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_FOLLOWLOCATION, 1L);

  // we use a self-signed test server, skip verification during debugging
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_FAILONERROR, 1L);

  // Set large buffer to optimize write/download rate
  curl_easy_setopt(reinterpret_cast<CURL*>(this->_hcurl), CURLOPT_BUFFERSIZE, 120000L);

  this->_out_file = file;
  this->_user_download = download_cb;
  this->_user_ptr = user_ptr;

  // start timer
  this->_rate_time = clock();

  this->_ercode = curl_easy_perform(reinterpret_cast<CURL*>(this->_hcurl));

  // get HTTP error if exists
   if(this->_ercode != CURLE_OK) {
    this->_tpcode = 0;
    curl_easy_getinfo(reinterpret_cast<CURL*>(this->_hcurl), CURLINFO_RESPONSE_CODE, &this->_tpcode);
  }

  this->_downloading = false;

  this->clear();

  return (this->_ercode == CURLE_OK);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmSocket::clear()
{
  if(this->_buff_data != nullptr) {
    Om_free(this->_buff_data);
    this->_buff_data = nullptr;
  }

  this->_buff_fill = 0;
  this->_buff_size = 0;

  this->_out_file = nullptr;
  this->_user_download = nullptr;
  this->_user_ptr = nullptr;

  this->_downloading = false;

  this->_rate_byte = 0;
  this->_rate_time = 0.0;
  this->_progress_bps = 0.0;
  this->_progress_tot = 0;
  this->_progress_now = 0;

  if(this->_hcurl != nullptr) {
    curl_easy_cleanup(reinterpret_cast<CURL*>(this->_hcurl));
    this->_hcurl = nullptr;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring OmSocket::lastErrorStr() const
{
  wstring result;

  switch(_ercode)
  {
  case CURLE_OK: result = L"OK"; break;
  case CURLE_UNSUPPORTED_PROTOCOL: result = L"UNSUPPORTED_PROTOCOL"; break;
  case CURLE_FAILED_INIT: result = L"FAILED_INIT"; break;
  case CURLE_URL_MALFORMAT: result = L"URL_MALFORMAT"; break;
  case CURLE_NOT_BUILT_IN: result = L"NOT_BUILT_IN"; break;
  case CURLE_COULDNT_RESOLVE_PROXY: result = L"COULDNT_RESOLVE_PROXY"; break;
  case CURLE_COULDNT_RESOLVE_HOST: result = L"COULDNT_RESOLVE_HOST"; break;
  case CURLE_COULDNT_CONNECT: result = L"COULDNT_CONNECT"; break;
  case CURLE_WEIRD_SERVER_REPLY: result = L"WEIRD_SERVER_REPLY"; break;
  case CURLE_REMOTE_ACCESS_DENIED: result = L"REMOTE_ACCESS_DENIED"; break;
  case CURLE_FTP_ACCEPT_FAILED: result = L"FTP_ACCEPT_FAILED"; break;
  case CURLE_FTP_WEIRD_PASS_REPLY: result = L"FTP_WEIRD_PASS_REPLY"; break;
  case CURLE_FTP_ACCEPT_TIMEOUT: result = L"FTP_ACCEPT_TIMEOUT"; break;
  case CURLE_FTP_WEIRD_PASV_REPLY: result = L"FTP_WEIRD_PASV_REPLY"; break;
  case CURLE_FTP_WEIRD_227_FORMAT: result = L"FTP_WEIRD_227_FORMAT"; break;
  case CURLE_FTP_CANT_GET_HOST: result = L"FTP_CANT_GET_HOST"; break;
  case CURLE_HTTP2: result = L"HTTP2"; break;
  case CURLE_FTP_COULDNT_SET_TYPE: result = L"FTP_COULDNT_SET_TYPE"; break;
  case CURLE_PARTIAL_FILE: result = L"PARTIAL_FILE"; break;
  case CURLE_FTP_COULDNT_RETR_FILE: result = L"FTP_COULDNT_RETR_FILE"; break;
  case CURLE_QUOTE_ERROR: result = L"QUOTE_ERROR"; break;
  case CURLE_HTTP_RETURNED_ERROR: result = L"HTTP_RETURNED_ERROR "; result += to_wstring(this->_tpcode); break;
  case CURLE_WRITE_ERROR: result = L"WRITE_ERROR"; break;
  case CURLE_UPLOAD_FAILED: result = L"UPLOAD_FAILED"; break;
  case CURLE_READ_ERROR: result = L"READ_ERROR"; break;
  case CURLE_OUT_OF_MEMORY: result = L"OUT_OF_MEMORY"; break;
  case CURLE_OPERATION_TIMEDOUT: result = L"OPERATION_TIMEDOUT"; break;
  case CURLE_FTP_PORT_FAILED: result = L"FTP_PORT_FAILED"; break;
  case CURLE_FTP_COULDNT_USE_REST: result = L"FTP_COULDNT_USE_REST"; break;
  case CURLE_RANGE_ERROR: result = L"RANGE_ERROR"; break;
  case CURLE_HTTP_POST_ERROR: result = L"HTTP_POST_ERROR"; break;
  case CURLE_SSL_CONNECT_ERROR: result = L"SSL_CONNECT_ERROR"; break;
  case CURLE_BAD_DOWNLOAD_RESUME: result = L"BAD_DOWNLOAD_RESUME"; break;
  case CURLE_FILE_COULDNT_READ_FILE: result = L"FILE_COULDNT_READ_FILE"; break;
  case CURLE_LDAP_CANNOT_BIND: result = L"LDAP_CANNOT_BIND"; break;
  case CURLE_LDAP_SEARCH_FAILED: result = L"LDAP_SEARCH_FAILED"; break;
  case CURLE_FUNCTION_NOT_FOUND: result = L"FUNCTION_NOT_FOUND"; break;
  case CURLE_ABORTED_BY_CALLBACK: result = L"ABORTED_BY_CALLBACK"; break;
  case CURLE_BAD_FUNCTION_ARGUMENT: result = L"BAD_FUNCTION_ARGUMENT"; break;
  case CURLE_INTERFACE_FAILED: result = L"INTERFACE_FAILED"; break;
  case CURLE_TOO_MANY_REDIRECTS: result = L"TOO_MANY_REDIRECTS"; break;
  case CURLE_UNKNOWN_OPTION: result = L"UNKNOWN_OPTION"; break;
  case CURLE_TELNET_OPTION_SYNTAX: result = L"TELNET_OPTION_SYNTAX"; break;
  case CURLE_GOT_NOTHING: result = L"GOT_NOTHING"; break;
  case CURLE_SSL_ENGINE_NOTFOUND: result = L"SSL_ENGINE_NOTFOUND"; break;
  case CURLE_SSL_ENGINE_SETFAILED: result = L"SSL_ENGINE_SETFAILED"; break;
  case CURLE_SEND_ERROR: result = L"SEND_ERROR"; break;
  case CURLE_RECV_ERROR: result = L"RECV_ERROR"; break;
  case CURLE_SSL_CERTPROBLEM: result = L"SSL_CERTPROBLEM"; break;
  case CURLE_SSL_CIPHER: result = L"SSL_CIPHER"; break;
  case CURLE_PEER_FAILED_VERIFICATION: result = L"PEER_FAILED_VERIFICATION"; break;
  case CURLE_BAD_CONTENT_ENCODING: result = L"BAD_CONTENT_ENCODING"; break;
  case CURLE_LDAP_INVALID_URL: result = L"LDAP_INVALID_URL"; break;
  case CURLE_FILESIZE_EXCEEDED: result = L"FILESIZE_EXCEEDED"; break;
  case CURLE_USE_SSL_FAILED: result = L"USE_SSL_FAILED"; break;
  case CURLE_SEND_FAIL_REWIND: result = L"SEND_FAIL_REWIND"; break;
  case CURLE_SSL_ENGINE_INITFAILED: result = L"SSL_ENGINE_INITFAILED"; break;
  case CURLE_LOGIN_DENIED: result = L"LOGIN_DENIED"; break;
  case CURLE_TFTP_NOTFOUND: result = L"TFTP_NOTFOUND"; break;
  case CURLE_TFTP_PERM: result = L"TFTP_PERM"; break;
  case CURLE_REMOTE_DISK_FULL: result = L"REMOTE_DISK_FULL"; break;
  case CURLE_TFTP_ILLEGAL: result = L"TFTP_ILLEGAL"; break;
  case CURLE_TFTP_UNKNOWNID: result = L"TFTP_UNKNOWNID"; break;
  case CURLE_REMOTE_FILE_EXISTS: result = L"REMOTE_FILE_EXISTS"; break;
  case CURLE_TFTP_NOSUCHUSER: result = L"TFTP_NOSUCHUSER"; break;
  case CURLE_CONV_FAILED: result = L"CONV_FAILED"; break;
  case CURLE_CONV_REQD: result = L"CONV_REQD"; break;
  case CURLE_SSL_CACERT_BADFILE: result = L"SSL_CACERT_BADFILE"; break;
  case CURLE_REMOTE_FILE_NOT_FOUND: result = L"REMOTE_FILE_NOT_FOUND"; break;
  case CURLE_SSH: result = L"SSH"; break;
  case CURLE_SSL_SHUTDOWN_FAILED: result = L"SSL_SHUTDOWN_FAILED"; break;
  case CURLE_AGAIN: result = L"AGAIN"; break;
  case CURLE_SSL_CRL_BADFILE: result = L"SSL_CRL_BADFILE"; break;
  case CURLE_SSL_ISSUER_ERROR: result = L"SSL_ISSUER_ERROR"; break;
  case CURLE_FTP_PRET_FAILED: result = L"FTP_PRET_FAILED"; break;
  case CURLE_RTSP_CSEQ_ERROR: result = L"RTSP_CSEQ_ERROR"; break;
  case CURLE_RTSP_SESSION_ERROR: result = L"RTSP_SESSION_ERROR"; break;
  case CURLE_FTP_BAD_FILE_LIST: result = L"FTP_BAD_FILE_LIST"; break;
  case CURLE_CHUNK_FAILED: result = L"CHUNK_FAILED"; break;
  case CURLE_NO_CONNECTION_AVAILABLE: result = L"NO_CONNECTION_AVAILABLE"; break;
  case CURLE_SSL_PINNEDPUBKEYNOTMATCH: result = L"SSL_PINNEDPUBKEYNOTMATCH"; break;
  case CURLE_SSL_INVALIDCERTSTATUS: result = L"SSL_INVALIDCERTSTATUS"; break;
  case CURLE_HTTP2_STREAM: result = L"HTTP2_STREAM"; break;
  case CURLE_RECURSIVE_API_CALL: result = L"RECURSIVE_API_CALL"; break;
  case CURLE_AUTH_ERROR: result = L"AUTH_ERROR"; break;
  case CURLE_HTTP3: result = L"HTTP3"; break;
  case CURLE_QUIC_CONNECT_ERROR: result = L"QUIC_CONNECT_ERROR"; break;
  case CURLE_PROXY: result = L"PROXY"; break;
  }

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmSocket::_writeMemCb(char *recv_data, size_t recv_s, size_t recv_n, void *userptr)
{
  OmSocket* self = static_cast<OmSocket*>(userptr);

  size_t recv_size = recv_s * recv_n;
  size_t recv_tot = recv_size + self->_buff_fill;

  // compute download rate
  self->_rate_byte += recv_size;
  double sec = static_cast<double>(clock() - self->_rate_time) / CLOCKS_PER_SEC;
  if(sec >= 0.5) { // 500 Ms
    self->_progress_bps = static_cast<double>(self->_rate_byte) / sec;
    self->_rate_byte = 0;
    self->_rate_time = clock();
  }

  if(recv_tot > self->_buff_size) {

    if(!self->_buff_size)
      self->_buff_size = 1024; // initial minimum buffer size

    // increase size if needed
    while(recv_tot > self->_buff_size) {
      self->_buff_size *= 2;
    }

    // alloc or realloc buffer
    if(self->_buff_data != nullptr) {
      if((self->_buff_data = reinterpret_cast<uint8_t*>(Om_realloc(self->_buff_data, self->_buff_size))) == nullptr)
        throw std::bad_alloc();
    } else {
      if((self->_buff_data = reinterpret_cast<uint8_t*>(Om_alloc(self->_buff_size))) == nullptr)
        throw std::bad_alloc();
    }
  }

  // fill buffer with received data
  memcpy(self->_buff_data + self->_buff_fill, recv_data, recv_size);
  self->_buff_fill += recv_size;

  return recv_size;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmSocket::_writeFileCb(char *recv_data, size_t recv_s, size_t recv_n, void *ptr)
{
  OmSocket* self = static_cast<OmSocket*>(ptr);

  size_t recv_size = recv_s * recv_n;

  // compute download rate
  self->_rate_byte += recv_size;
  double sec = static_cast<double>(clock() - self->_rate_time) / CLOCKS_PER_SEC;
  if(sec >= 0.5) { // 500 Ms
    self->_progress_bps = static_cast<double>(self->_rate_byte) / sec;
    self->_rate_byte = 0;
    self->_rate_time = clock();
  }

  /*
  DWORD dwB;
  WriteFile(static_cast<HANDLE>(self->_out_file), recv_data, recv_size, &dwB, nullptr);
  return recv_size;
  */

  return fwrite(recv_data, recv_s, recv_n, static_cast<FILE*>(self->_out_file));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmSocket::_progressCb(void* ptr, int64_t dltot, int64_t dlnow, int64_t ultot, int64_t ulnow)
{
  OmSocket* self = static_cast<OmSocket*>(ptr);

  self->_downloading = true;

  self->_progress_tot = dltot;
  self->_progress_now = dlnow;

  if(self->_user_download) {
    if(!self->_user_download(self->_user_ptr, dltot, dlnow, self->_progress_bps, 0)) {
      return 1; //< abort process
    }
  }

  return CURL_PROGRESSFUNC_CONTINUE;
}
