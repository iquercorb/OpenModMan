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

#include <winsock.h>

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmConnect.h"


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
OmConnect::OmConnect() :
  _heasy(nullptr),
  _hmult(nullptr),
  _req_result(0),
  _req_response(0),
  _req_user_ptr(nullptr),
  _req_response_cb(nullptr),
  _req_result_cb(nullptr),
  _req_download_cb(nullptr),
  _req_abort(false),
  _get_data_buf(nullptr),
  _get_data_len(0),
  _get_data_cap(0),
  _get_file_hnd(nullptr),
  _rate_accu(0),
  _rate_time(0.0),
  _progress_off(0L),
  _progress_tot(0L),
  _progress_now(0L),
  _progress_bps(0.0),
  _perform_hth(nullptr),
  _perform_hwo(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmConnect::~OmConnect()
{
  this->clear();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmConnect::requestHttpGet(const OmWString& url, OmCString* reponse)
{
  __curl_init();

  this->clear();

  this->_heasy = curl_easy_init();
  this->_hmult = curl_multi_init();

  CURL* curl_easy = reinterpret_cast<CURL*>(this->_heasy);
  CURLM* curl_mult = reinterpret_cast<CURLM*>(this->_hmult);

  this->_req_url.clear();
  Om_toUTF8(&this->_req_url, url);

  curl_easy_setopt(curl_easy, CURLOPT_URL, this->_req_url.c_str());

  curl_easy_setopt(curl_easy, CURLOPT_HTTPGET, 1L);

  curl_easy_setopt(curl_easy, CURLOPT_WRITEFUNCTION, OmConnect::_perform_write_mem_fn);
  curl_easy_setopt(curl_easy, CURLOPT_WRITEDATA, this);

  curl_easy_setopt(curl_easy, CURLOPT_NOPROGRESS, 1L);

  // follow HTTP redirections
  curl_easy_setopt(curl_easy, CURLOPT_FOLLOWLOCATION, 1L);

  // we use a self-signed test server, skip verification during debugging
  curl_easy_setopt(curl_easy, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl_easy, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl_easy, CURLOPT_FAILONERROR, 1L);

  // Set large buffer to optimize write/download rate
  curl_easy_setopt(curl_easy, CURLOPT_BUFFERSIZE, 262144L);

  curl_multi_add_handle(curl_mult, curl_easy);

  #ifdef DEBUG
  curl_easy_setopt(curl_easy, CURLOPT_BUFFERSIZE, 64000L);
  curl_easy_setopt(curl_easy, CURLOPT_MAX_RECV_SPEED_LARGE, 150000L);
  curl_easy_setopt(curl_easy, CURLOPT_UPLOAD_BUFFERSIZE, 64000L);
  curl_easy_setopt(curl_easy, CURLOPT_MAX_SEND_SPEED_LARGE, 150000L);
  #endif

  // artificially set peforming thread handle
  this->_perform_hth = reinterpret_cast<void*>(0x1);

  // number of running handles
  int32_t running_count = 1;

  while(running_count) {

    CURLMcode curlm_code = curl_multi_perform(curl_mult, &running_count);

    if(curlm_code == CURLM_OK && !this->_req_abort) { // wait for activity or timeout
      curlm_code = curl_multi_poll(curl_mult, nullptr, 0, 100, nullptr);
    }

    if(curlm_code != CURLM_OK || this->_req_abort)
      break;
  }

  CURLMsg* curl_msg;
  int msgs_left;

  // get result message
  while((curl_msg = curl_multi_info_read(curl_mult, &msgs_left))) {
    if(curl_msg->msg == CURLMSG_DONE) {
      // get transfer result code
      this->_req_result = curl_msg->data.result;
      // get HTTP response code
      curl_easy_getinfo(curl_msg->easy_handle, CURLINFO_RESPONSE_CODE, &this->_req_response);
    }
  }

  curl_multi_remove_handle(curl_mult, curl_easy);


  /*
  // launch request
  this->_req_result = curl_easy_perform(curl_easy);

  curl_easy_getinfo(curl_easy, CURLINFO_RESPONSE_CODE, &this->_req_response);
  */


  #ifdef DEBUG
  std::cout << "\n";
  std::cout << "DEBUG => OmConnect::requestHttpGet : _req_result=" << std::to_string(this->_req_result) << " (" << curl_easy_strerror((CURLcode)this->_req_result) << ")\n";
  #endif // DEBUG

  if(this->_req_result != CURLE_OK) {

    Om_free(this->_get_data_buf);
    this->_get_data_buf = nullptr;

    this->_get_data_len = 0;
    this->_get_data_cap = 0;

  } else {

    // in the extremely improbable case capacity is not
    //  enough to add null char we reallocate buffer
    if(this->_get_data_len + 1 > this->_get_data_cap) {
      this->_get_data_cap++;
      this->_get_data_buf = static_cast<uint8_t*>(Om_realloc(this->_get_data_buf, this->_get_data_cap));
    }

    // add null-char or die
    if(this->_get_data_buf) {
      this->_get_data_buf[this->_get_data_len] = '\0';
      *reponse = reinterpret_cast<char*>(this->_get_data_buf);
    } else {
      this->_get_data_len = 0;
      this->_get_data_cap = 0;
    }
  }

  OmResult result;

  if(this->_req_abort) {
    result = OM_RESULT_ABORT;
  } else {
    result = (this->_req_result == CURLE_OK) ? OM_RESULT_OK : OM_RESULT_ERROR;
  }

  // reset performing thread handle
  this->_perform_hth = nullptr;

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmConnect::requestHttpGet(const OmWString& url, Om_responseCb response_cb, void* user_ptr)
{
  if(this->_perform_hth)
    return false;

  __curl_init();

  this->clear();

  this->_heasy = curl_easy_init();
  this->_hmult = curl_multi_init();

  CURL* curl_easy = reinterpret_cast<CURL*>(this->_heasy);

  this->_req_url.clear();
  Om_toUTF8(&this->_req_url, url);

  curl_easy_setopt(curl_easy, CURLOPT_URL, this->_req_url.c_str());

  curl_easy_setopt(curl_easy, CURLOPT_HTTPGET, 1L);

  curl_easy_setopt(curl_easy, CURLOPT_WRITEFUNCTION, OmConnect::_perform_write_mem_fn);
  curl_easy_setopt(curl_easy, CURLOPT_WRITEDATA, this);

  curl_easy_setopt(curl_easy, CURLOPT_NOPROGRESS, 1L);

  this->_req_user_ptr = user_ptr;
  this->_req_response_cb = response_cb;

  this->_req_abort = false;

  // launch new download thread
  this->_perform_hth = Om_createThread(OmConnect::_perform_run_fn, this);

  // register wait object to track thread end
  this->_perform_hwo = Om_waitForThread(this->_perform_hth, OmConnect::_perform_end_fn, this);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmConnect::requestHttpGet(const OmWString& url, const OmWString& path, bool resume, Om_resultCb result_cb, Om_downloadCb download_cb, void* user_ptr)
{
  if(this->_perform_hth)
    return false;

  this->clear();

  __curl_init();

  DWORD disp = resume ? OPEN_ALWAYS : CREATE_ALWAYS;

  this->_get_file_hnd = CreateFileW(  path.c_str(),
                                      GENERIC_WRITE,
                                      FILE_SHARE_READ,
                                      nullptr,
                                      disp,
                                      FILE_ATTRIBUTE_NORMAL,
                                      nullptr);

  if(!this->_get_file_hnd) {

    return false;
  }

  LARGE_INTEGER FileSize;
  GetFileSizeEx(static_cast<HANDLE>(this->_get_file_hnd), &FileSize);
  int64_t resume_off = FileSize.QuadPart;

  this->_heasy = curl_easy_init();
  this->_hmult = curl_multi_init();

  CURL* curl_easy = reinterpret_cast<CURL*>(this->_heasy);

  this->_req_url.clear();
  Om_toUTF8(&this->_req_url, url);

  curl_easy_setopt(curl_easy, CURLOPT_URL, this->_req_url.c_str());

  curl_easy_setopt(curl_easy, CURLOPT_HTTPGET, 1L);

  curl_easy_setopt(curl_easy, CURLOPT_WRITEFUNCTION, OmConnect::_perform_write_fio_fn);
  curl_easy_setopt(curl_easy, CURLOPT_WRITEDATA, this);

  curl_easy_setopt(curl_easy, CURLOPT_XFERINFOFUNCTION, OmConnect::_perform_progress_fn);
  curl_easy_setopt(curl_easy, CURLOPT_XFERINFODATA, this);
  curl_easy_setopt(curl_easy, CURLOPT_NOPROGRESS, 0L);

  curl_easy_setopt(curl_easy, CURLOPT_VERBOSE, 0L);

  if(resume_off > 0L) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmConnect::requestHttpGet : resume from :" << resume_off << "\n";
    #endif // DEBUG

    SetFilePointer(static_cast<HANDLE>(this->_get_file_hnd), 0, nullptr, FILE_END);
    curl_easy_setopt(curl_easy, CURLOPT_RESUME_FROM_LARGE, resume_off);
    this->_progress_off = resume_off;
  }

  this->_req_user_ptr = user_ptr;
  this->_req_result_cb = result_cb;
  this->_req_download_cb = download_cb;

  // initialize download statistics
  this->_rate_accu = 0;
  this->_rate_time = clock();

  this->_progress_tot = 0L;
  this->_progress_now = 0L;
  this->_progress_bps = 0.0;

  this->_req_abort = false;

  // launch new download thread
  this->_perform_hth = Om_createThread(OmConnect::_perform_run_fn, this);
  // register wait object to track thread end
  this->_perform_hwo = Om_waitForThread(this->_perform_hth, OmConnect::_perform_end_fn, this);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmConnect::abortRequest()
{
  if(this->_perform_hth) {

    CURLM* curl_mult = reinterpret_cast<CURLM*>(this->_hmult);

    // set abort signal
    this->_req_abort = true;

    // wake up curl_multi_poll to exit loop as soon as possible
    curl_multi_wakeup(curl_mult);

    // et short timeout to abort connection attempt
    //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1);

    // close socket to abort transfert
    /*
    curl_socket_t socket;
    CURLcode result = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &socket);
    if(result == CURLE_OK) {

      #ifdef DEBUG
      std::cout << "DEBUG => OmConnect::abortRequest : socket closed\n";
      #endif // DEBUG

      shutdown(socket, SD_BOTH);
      closesocket(socket);
    }
    */
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmConnect::isPerforming() const
{
  return (this->_perform_hth != nullptr);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmConnect::clear()
{
  if(this->_heasy) {
    if(this->_hmult)
      curl_multi_remove_handle(reinterpret_cast<CURLM*>(this->_hmult), this->_heasy);
    curl_easy_cleanup(reinterpret_cast<CURL*>(this->_heasy));
    this->_heasy = nullptr;
  }

  if(this->_hmult) {
    curl_multi_cleanup(reinterpret_cast<CURLM*>(this->_hmult));
    this->_hmult = nullptr;
  }

  this->_req_url.clear();

  this->_req_result = 0;
  this->_req_response = 0;
  this->_req_user_ptr = nullptr;
  this->_req_response_cb = nullptr;
  this->_req_result_cb = nullptr;
  this->_req_download_cb = nullptr;
  this->_req_abort = false;

  if(this->_get_data_buf) {
    Om_free(this->_get_data_buf);
    this->_get_data_buf = nullptr;
  }
  this->_get_data_len = 0;
  this->_get_data_cap = 0;

  this->_get_file_hnd = nullptr;

  this->_rate_accu = 0;
  this->_rate_time = 0.0;

  this->_progress_off = 0L;
  this->_progress_tot = 0L;
  this->_progress_now = 0L;
  this->_progress_bps = 0.0;

  Om_clearThread(this->_perform_hth, this->_perform_hwo);
  this->_perform_hth = nullptr;
  this->_perform_hwo = nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmConnect::_perform_run_fn(void* ptr)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmConnect::_perform_run_fn : enter\n";
  #endif // DEBUG

  OmConnect* self = static_cast<OmConnect*>(ptr);

  CURL* curl_easy = reinterpret_cast<CURL*>(self->_heasy);
  CURLM* curl_mult = reinterpret_cast<CURLM*>(self->_hmult);

  // follow HTTP redirections
  curl_easy_setopt(curl_easy, CURLOPT_FOLLOWLOCATION, 1L);

  // we use a self-signed test server, skip verification during debugging
  curl_easy_setopt(curl_easy, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl_easy, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl_easy, CURLOPT_FAILONERROR, 1L);

  // Set large buffer to optimize write/download rate
  curl_easy_setopt(curl_easy, CURLOPT_BUFFERSIZE, 262144L);

  curl_multi_add_handle(curl_mult, curl_easy);

  #ifdef DEBUG
  curl_easy_setopt(curl_easy, CURLOPT_BUFFERSIZE, 64000L);
  curl_easy_setopt(curl_easy, CURLOPT_MAX_RECV_SPEED_LARGE, 150000L);
  curl_easy_setopt(curl_easy, CURLOPT_UPLOAD_BUFFERSIZE, 64000L);
  curl_easy_setopt(curl_easy, CURLOPT_MAX_SEND_SPEED_LARGE, 150000L);
  #endif

  // number of running handles
  int32_t running_count = 1;

  while(running_count) {

    CURLMcode curlm_code = curl_multi_perform(curl_mult, &running_count);

    if(curlm_code == CURLM_OK && !self->_req_abort) { // wait for activity or timeout
      curlm_code = curl_multi_poll(curl_mult, nullptr, 0, 500, nullptr);
    }

    if(curlm_code != CURLM_OK || self->_req_abort)
      break;
  }

  CURLMsg* curl_msg;
  int msgs_left;

  // get result message
  while((curl_msg = curl_multi_info_read(curl_mult, &msgs_left))) {
    if(curl_msg->msg == CURLMSG_DONE) {
      // get transfer result code
      self->_req_result = curl_msg->data.result;
      // get HTTP response code
      curl_easy_getinfo(curl_msg->easy_handle, CURLINFO_RESPONSE_CODE, &self->_req_response);
    }
  }

  curl_multi_remove_handle(curl_mult, curl_easy);

  #ifdef DEBUG
  std::cout << "\n";
  std::cout << "DEBUG => OmConnect::_perform_run_fn : _req_result=" << std::to_string(self->_req_result) << " (" << curl_easy_strerror((CURLcode)self->_req_result) << ")\n";
  #endif // DEBUG

  DWORD resultCode = 0;

  // close file handle
  if(self->_get_file_hnd) {
    CloseHandle(self->_get_file_hnd);
  }

  if(self->_get_data_buf) {

    if(self->_req_result != CURLE_OK) {

      Om_free(self->_get_data_buf);
      self->_get_data_buf = nullptr;

      self->_get_data_len = 0;
      self->_get_data_cap = 0;

    } else {

      // in the extremely improbable case capacity is not
      //  enough to add null char we reallocate buffer
      if(self->_get_data_len + 1 > self->_get_data_cap) {
        self->_get_data_cap++;
        self->_get_data_buf = static_cast<uint8_t*>(Om_realloc(self->_get_data_buf, self->_get_data_cap));
      }

      // add null-char or die
      if(self->_get_data_buf) {
        self->_get_data_buf[self->_get_data_len] = '\0';
      } else {
        self->_get_data_len = 0;
        self->_get_data_cap = 0;
      }

    }
  }

  #ifdef DEBUG
  std::cout << "DEBUG => OmConnect::_perform_run_fn : leave\n";
  #endif // DEBUG

  return resultCode;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID CALLBACK OmConnect::_perform_end_fn(void* ptr, uint8_t timer)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmConnect::_perform_end : enter\n";
  #endif // DEBUG

  OmConnect* self = static_cast<OmConnect*>(ptr);

  // free and reset all thread data
  Om_clearThread(self->_perform_hth, self->_perform_hwo);
  self->_perform_hth = nullptr;
  self->_perform_hwo = nullptr;

  if(self->_req_result_cb) {

    OmResult result;

    if(self->_req_abort) {
      result = OM_RESULT_ABORT;
    } else {
      result = (self->_req_result == CURLE_OK) ? OM_RESULT_OK : OM_RESULT_ERROR;
    }

    self->_req_result_cb(self->_req_user_ptr, result, self->_req_response);
  }

  if(self->_req_response_cb)
    self->_req_response_cb(self->_req_user_ptr, self->_get_data_buf, self->_get_data_len, self->_req_response);

  // clear instance
  self->clear();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmConnect::_perform_write_mem_fn(char *recv_data, size_t recv_s, size_t recv_n, void *ptr)
{
  OmConnect* self = static_cast<OmConnect*>(ptr);

  size_t recv_len = recv_s * recv_n;

  size_t recv_tot = self->_get_data_len + recv_len;


  if(recv_tot > self->_get_data_cap) {

    if(self->_get_data_cap == 0)
      self->_get_data_cap = 10240; //< initial buffer size

    // increase capacity if needed
    while(recv_tot > self->_get_data_cap) {
      self->_get_data_cap *= 2;
    }

    if(self->_get_data_buf) {
      // reallocate buffer with new capacity
      self->_get_data_buf = reinterpret_cast<uint8_t*>(Om_realloc(self->_get_data_buf, self->_get_data_cap));
    } else {
      // initial buffer allocation
      self->_get_data_buf = reinterpret_cast<uint8_t*>(Om_alloc(self->_get_data_cap));
    }

    // check for allocation error
    if(!self->_get_data_buf) {
      self->_get_data_cap = 0;
      self->_get_data_len = 0;
      return CURL_WRITEFUNC_ERROR;
    }
  }

  // fill buffer with received data
  memcpy(self->_get_data_buf + self->_get_data_len, recv_data, recv_len);

  self->_get_data_len += recv_len;

  return recv_len;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmConnect::_perform_write_fio_fn(char *recv_data, size_t recv_s, size_t recv_n, void *ptr)
{
  OmConnect* self = static_cast<OmConnect*>(ptr);

  DWORD dwBytesWritten;

  WriteFile(static_cast<HANDLE>(  self->_get_file_hnd),
                                  recv_data,
                                  recv_s * recv_n,
                                  &dwBytesWritten,
                                  nullptr);

  if(self->_req_abort)
    return CURL_WRITEFUNC_ERROR;

  return dwBytesWritten;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmConnect::_perform_progress_fn(void* ptr, int64_t dltot, int64_t dlnow, int64_t ultot, int64_t ulnow)
{
  OmConnect* self = static_cast<OmConnect*>(ptr);

  self->_progress_tot = dltot + self->_progress_off;
  self->_progress_now = dlnow + self->_progress_off;

  double seconds = static_cast<double>(clock() - self->_rate_time) / CLOCKS_PER_SEC;

  if(seconds >= 0.5 && self->_rate_accu != dlnow) { // 500 Ms
    self->_progress_bps = static_cast<double>(dlnow - self->_rate_accu) / seconds;
    self->_rate_accu = dlnow;
    self->_rate_time = clock();
  }

  if(self->_req_download_cb) {
    if(!self->_req_download_cb( self->_req_user_ptr,
                                self->_progress_tot,
                                self->_progress_now,
                                self->_progress_bps,
                                0)) {
      self->_req_abort = true;
      return 1; //< abort process
    }
  }

  return CURL_PROGRESSFUNC_CONTINUE;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString OmConnect::lastError() const
{
  OmWString str(Om_toUTF16(curl_easy_strerror((CURLcode)this->_req_result)));

  if(this->_req_result == CURLE_HTTP_RETURNED_ERROR) {
    str.append(L" ("); str.append(std::to_wstring(this->_req_response)); str.append(L")");
  }

  return str;
}
