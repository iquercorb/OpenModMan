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
#ifndef OMSOCKET_H
#define OMSOCKET_H

#include "OmBase.h"

/// \brief Network socket object
///
/// Class to manage network connexions.
///
class OmSocket
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmSocket();

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmSocket();

    /// \brief Send HTTP GET request
    ///
    /// Send an HTTP GET request and store received data as null terminated
    /// string to the given string object.
    ///
    /// \param[in]  url   URL to perform HTTP GET.
    /// \param[out] data  String object to be set as received data.
    ///
    /// \return True if operation succeed and data received, false otherwise.
    ///
    bool httpGet(const wstring& url, string& data);

    /// \brief Send HTTP GET request
    ///
    /// Send an HTTP GET request and write received data to the specified file pointer.
    ///
    /// \param[in]  url         : URL to perform HTTP GET.
    /// \param[out] file        : Destination file pointer.
    /// \param[in]  download_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    /// \param[in]  resume      : Download resume offset in byte.
    ///
    /// \return True if operation succeed and data received, false otherwise.
    ///
    bool httpGet(const wstring& url, FILE* file, Om_downloadCb download_cb, void* user_ptr, uint64_t resume = 0L);

    /// \brief Check whether downloading
    ///
    /// Check whether this instance is currently processing download data.
    ///
    /// \return True if downloading, false otherwise.
    ///
    bool downloading() const {
      return this->_downloading;
    }

    /// \brief Get download progress
    ///
    /// Returns current download progression in percent.
    ///
    /// \return Download progression in percent.
    ///
    unsigned downPercent() const {
      return (static_cast<double>(_progress_now) / _progress_tot) * 100;
    }

    /// \brief Get download total
    ///
    /// Returns download total bytes to be downloaded.
    ///
    /// \return Download total bytes or zero if not downloading.
    ///
    size_t downTot() const {
      return _progress_tot;
    }

    /// \brief Get download current
    ///
    /// Returns download bytes downloaded so far.
    ///
    /// \return Downloaded bytes so far.
    ///
    size_t downCur() const {
      return _progress_now;
    }

    /// \brief Get download rate
    ///
    /// Returns download rate in bytes per second.
    ///
    /// \return Download rate in bytes per second.
    ///
    double downRate() const {
      return _progress_bps;
    }

    /// \brief Clear object.
    ///
    /// Reset connexion and data.
    ///
    void clear();

    /// \brief Get last error code.
    ///
    /// Returns last connexion error code.
    ///
    /// \return Last connexion error code.
    ///
    unsigned lastError() const {
      return _ercode;
    }

    /// \brief Get last error string.
    ///
    /// Returns the string corresponding to the last XML parsing error code.
    ///
    /// \return XML parsing error as string.
    ///
    wstring lastErrorStr() const;

  protected: ///        - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void*               _hcurl;

    size_t              _buff_fill;

    size_t              _buff_size;

    uint8_t*            _buff_data;

    void*               _out_file;

    Om_downloadCb       _user_download;

    void*               _user_ptr;

    bool                _downloading;

    int64_t             _rate_byte;

    double              _rate_time;

    double              _progress_bps;

    int64_t             _progress_tot;

    int64_t             _progress_now;

    int64_t             _progress_off;

    unsigned            _ercode;

    unsigned            _tpcode;

    static size_t       _writeMemCb(char*, size_t, size_t, void*);

    static size_t       _writeFileCb(char *, size_t, size_t, void*);

    static int          _progressCb(void*, int64_t, int64_t, int64_t, int64_t);
};

#endif // OMSOCKET_H
