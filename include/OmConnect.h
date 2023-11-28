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
#ifndef OMCONNECT_H
#define OMCONNECT_H

#include "OmBase.h"
#include "OmBaseWin.h"

/// \brief Network socket object
///
/// Class to manage network download and requests.
///
class OmConnect
{
  public: ///           - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmConnect();

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmConnect();

    /// \brief Http Get request data
    ///
    /// Send an HTTP GET request then return received data once done. This
    /// function does not use thread and block until response or time out.
    ///
    /// \param[in] url          : Target URL for HTTP request.
    /// \param[in] reponse      : Pointer to string to receive response data
    ///
    /// \return Result code of the request
    ///
    OmResult requestHttpGet(const OmWString& url, OmCString* reponse);

    /// \brief Http Get request once
    ///
    /// Send an HTTP GET request then provides received data once done.
    ///
    /// \param[in] url          : Target URL for HTTP request.
    /// \param[in] response_cb  : Callback to get request response data.
    /// \param[in] user_ptr     : Custom pointer to pass to callback.
    ///
    /// \return True if request sent, false if a previous request still performing.
    ///
    bool requestHttpGet(const OmWString& url, Om_responseCb response_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Http Get request download
    ///
    /// Send an HTTP GET request to download file at specified location
    ///
    /// \param[in] url          : Target URL for HTTP request.
    /// \param[in] path         : Download destination file path.
    /// \param[in] resume       : Resume download and append data to existing destination file.
    /// \param[in] result_cb    : Callback to get request result.
    /// \param[in] download_cb  : Callback for download progression.
    /// \param[in] user_ptr     : Custom pointer to pass to callback
    ///
    /// \return True if request sent, false if a previous request still performing.
    ///
    bool requestHttpGet(const OmWString& url, const OmWString& path, bool resume, Om_resultCb result_cb = nullptr, Om_downloadCb download_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Http Get response code
    ///
    /// Returns HTTP GET request response code of the last performed request.
    ///
    /// \return HTTP response code
    ///
    int32_t httpGetResponse() const {
      return this->_req_response;
    }

    /// \brief Checks whether is performing
    ///
    /// Check whether this instance is currently performing request/transfer
    ///
    /// \return True if instance is performing, false otherwise
    ///
    bool isPerforming() const;

    /// \brief Abort request
    ///
    /// Force to abort the currently performing request if any.
    ///
    void abortRequest();

    /// \brief Get last error string.
    ///
    /// Returns the string of the last performing request error.
    ///
    /// \return Request perform error string.
    ///
    OmWString lastError() const;

    /// \brief Clear instance.
    ///
    /// Reset and free all parameters and data.
    ///
    void clear();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void*               _heasy;

    void*               _hmult;

    OmCString           _req_url;

    uint32_t            _req_result;

    uint32_t            _req_response;

    void*               _req_user_ptr;

    Om_responseCb       _req_response_cb;

    Om_resultCb         _req_result_cb;

    Om_downloadCb       _req_download_cb;

    bool                _req_abort;

    uint8_t*            _get_data_buf;

    uint64_t            _get_data_len;

    uint64_t            _get_data_cap;

    void*               _get_file_hnd;

    uint32_t            _rate_accu;

    double              _rate_time;

    int64_t             _progress_off;

    int64_t             _progress_tot;

    int64_t             _progress_now;

    double              _progress_bps;

    void*               _perform_hth;

    void*               _perform_hwo;

    static DWORD WINAPI _perform_run_fn(void*);

    static VOID WINAPI  _perform_end_fn(void*,uint8_t);

    static size_t       _perform_write_mem_fn(char*, size_t, size_t, void*);

    static size_t       _perform_write_fio_fn(char*, size_t, size_t, void*);

    static int          _perform_progress_fn(void*, int64_t, int64_t, int64_t, int64_t);
};

#endif // OMCONNECT_H
