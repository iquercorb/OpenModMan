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

#include "OmGlobal.h"

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
    bool httpGet(const string& url, string& data);

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

    unsigned            _ercode;

    unsigned            _tpcode;


    static size_t       _writeCb(char*, size_t, size_t, void*);

    static size_t       _pgresCb(void*, double, double, double, double);
};

#endif // OMSOCKET_H
