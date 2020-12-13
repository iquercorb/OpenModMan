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
#include "OmSocket.h"

#include <curl/curl.h>
/*
static int __my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
  const char *text;
  struct transfer *t = (struct transfer *)userp;
  unsigned int num = t->num;

  (void)handle; // prevent compiler warning

  switch(type)
  {
  case CURLINFO_TEXT:
    std::cout << "== " << num << " Info :" << data << "\n";
    // FALLTHROUGH
  default: // in case a new one is introduced to shock us
    return 0;

  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  }

  //dump(text, num, (unsigned char *)data, size, 1);
  return 0;
}
*/


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmSocket::OmSocket() :
  _hcurl(nullptr),
  _buff_fill(0),
  _buff_size(0),
  _buff_data(nullptr),
  _ercode(0),
  _tpcode(0)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmSocket::~OmSocket()
{
  clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmSocket::httpGet(const string& url, string& data)
{
  _hcurl = curl_easy_init();

  curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_URL, url.c_str());

  curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_HTTPGET, 1L);

  curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_WRITEFUNCTION, &this->_writeCb);
  curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_WRITEDATA, this);

  curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_NOPROGRESS, 1L);

  //curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_PROGRESSFUNCTION, &this->_pgresCb);
  //curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_PROGRESSDATA, this);
  //curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_XFERINFOFUNCTION, &this->_pgresCb);
  //curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_XFERINFODATA, this);

  /* we use a self-signed test server, skip verification during debugging */
  curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(reinterpret_cast<CURL*>(_hcurl), CURLOPT_FAILONERROR, 1L);

  _ercode = curl_easy_perform(reinterpret_cast<CURL*>(_hcurl));

  if(_ercode == CURLE_OK) {

    // If needed, realloc to add the null character
    if((_buff_fill + 1) > _buff_size) {
      _buff_size++;
      if((_buff_data = reinterpret_cast<uint8_t*>(realloc(_buff_data, _buff_size))) == nullptr)
        throw std::bad_alloc();
    }

    // add terminal null character to ensure we have a valid string
    _buff_data[_buff_fill] = 0;
    _buff_fill++;

    data = reinterpret_cast<char*>(_buff_data);
  } else {
    _tpcode = 0;
    curl_easy_getinfo(reinterpret_cast<CURL*>(_hcurl), CURLINFO_RESPONSE_CODE, &_tpcode);
  }

  // rester buffer
  if(_buff_data != nullptr)
    free(_buff_data);

  _buff_fill = 0;
  _buff_size = 0;

  // clean curl
  curl_easy_cleanup(reinterpret_cast<CURL*>(_hcurl));
  _hcurl = nullptr;

  return (_ercode == CURLE_OK);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmSocket::clear()
{
  if(_buff_data != nullptr)
    free(_buff_data);

  _buff_fill = 0;
  _buff_size = 0;

  if(_hcurl != nullptr)
    curl_easy_cleanup(reinterpret_cast<CURL*>(_hcurl));
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
  case CURLE_HTTP_RETURNED_ERROR:
    result = L"HTTP_RETURNED_ERROR ";
    result += _tpcode;
    break;
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
size_t OmSocket::_writeCb(char *recv_data, size_t recv_s, size_t recv_n, void *userptr)
{
  OmSocket* self = reinterpret_cast<OmSocket*>(userptr);

  size_t recv_size = recv_s * recv_n;
  size_t recv_tot = recv_size + self->_buff_fill;

  if(recv_tot > self->_buff_size) {

    if(!self->_buff_size)
      self->_buff_size = 1024; // initial minimum buffer size

    // increase size if needed
    while(recv_tot > self->_buff_size) {
      self->_buff_size *= 2;
    }

    // alloc or realloc buffer
    if(self->_buff_data != nullptr) {
      if((self->_buff_data = reinterpret_cast<uint8_t*>(realloc(self->_buff_data, self->_buff_size))) == nullptr)
        throw std::bad_alloc();
    } else {
      if((self->_buff_data = reinterpret_cast<uint8_t*>(malloc(self->_buff_size))) == nullptr)
        throw std::bad_alloc();
    }
  }

  // fill buffer with received data
  memcpy(self->_buff_data + self->_buff_fill, recv_data, recv_size);
  self->_buff_fill += recv_size;

  std::cout << "_write_cb : Received " << recv_size << " bytes of data\n";

  return recv_size;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmSocket::_pgresCb(void* userptr, double dltotal, double dlnow, double ultotal, double ulnow)
{
  OmSocket* self = reinterpret_cast<OmSocket*>(userptr);

  return 0;
}
