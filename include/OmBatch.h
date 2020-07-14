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

#ifndef OMBATCH_H
#define OMBATCH_H

#include "OmGlobal.h"
#include "OmConfig.h"

class OmContext;

/// \brief Batch object.
///
/// Object for Package installation batch
///
class OmBatch
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmBatch();

    /// \brief Constructor.
    ///
    /// Constructor with Context.
    ///
    /// \param[in]  ctx     : Related Context for the Batch.
    ///
    OmBatch(OmContext* ctx);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmBatch();

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const wstring& lastError() const {
      return _error;
    }

    /// \brief Get Batch uuid.
    ///
    /// Returns Batch uuid.
    ///
    /// \return Batch uuid.
    ///
    const wstring& uuid() const {
      return _uuid;
    }

    /// \brief Get Batch title.
    ///
    /// Returns Batch title.
    ///
    /// \return Batch title.
    ///
    const wstring& title() const {
      return _title;
    }

    /// \brief Get Batch index.
    ///
    /// Returns Batch ordering index.
    ///
    /// \return Ordering index number.
    ///
    unsigned index() const {
      return _index;
    }

    /// \brief Get Batch definition file path.
    ///
    /// Returns Batch definition file path.
    ///
    /// \return Batch definition file path.
    ///
    const wstring& path() const {
      return _path;
    }

    /// \brief Parse Batch definition file.
    ///
    /// Analyses the supplied file to check whether this is a valid
    /// Batch definition, and if yes, parse its content.
    ///
    /// \param[in]  path    : Path to definition file to parse.
    ///
    /// \return True if supplied file is a valid Batch and operation succeed,
    /// false otherwise.
    ///
    bool parse(const wstring& path);

    /// \brief Check whether has Location.
    ///
    /// Check whether batch has the the specified Location.
    ///
    /// \param[in]  uuid    : Location UUID to check.
    ///
    /// \return True if the Location was found, false otherwise.
    ///
    bool hasLocation(const wstring& uuid);

    /// \brief Get Location count.
    ///
    /// Returns the count of Location defined in batch.
    ///
    /// \return Count of Location.
    ///
    size_t locationCount() {
      return this->_locUuid.size();
    }

    /// \brief Get Location UUID.
    ///
    /// Returns the Location UUID at the given index.
    ///
    /// \param[in]  l    : Location index.
    ///
    /// \return Location UUID string.
    ///
    wstring getLocationUuid(unsigned l) {
      return this->_locUuid[l];
    }

    /// \brief Get Install entry count.
    ///
    /// Returns the batch install entry count for the
    /// specified Location.
    ///
    /// \param[in]  l    : Location index.
    ///
    /// \return Package hash as 64 bits unsigned integer
    ///
    size_t getInstallCount(unsigned l) {
      return this->_instHash[l].size();
    }

    /// \brief Check whether has install hash.
    ///
    /// Checks whether Location has an install entry with
    /// the specified package hash.
    ///
    /// \param[in]  l    : Location index.
    /// \param[in]  hash : Package hash to search.
    ///
    /// \return True if package hash was found, false otherwise.
    ///
    bool hasInstallHash(unsigned l, uint64_t hash);

    /// \brief Get Install entry hash.
    ///
    /// Returns the batch install entry package hash for the
    /// specified Location.
    ///
    /// \param[in]  l    : Location index.
    /// \param[in]  i    : Install entry index.
    ///
    /// \return Package hash as 64 bits unsigned integer
    ///
    uint64_t getInstallHash(unsigned l, unsigned i) {
      return this->_instHash[l][i];
    }

    /// \brief Check whether has install hash.
    ///
    /// Checks whether Location has an install entry with
    /// the specified package hash.
    ///
    /// \param[in]  l     : Location index.
    /// \param[in]  ident : Package ident string to search.
    ///
    /// \return True if package hash was found, false otherwise.
    ///
    bool hasInstallIdent(unsigned l, const wstring& ident);

    /// \brief Get Install entry identity.
    ///
    /// Returns the batch install entry package identity for the
    /// specified Location.
    ///
    /// \param[in]  l    : Location index.
    /// \param[in]  i    : Install entry index.
    ///
    /// \return Package identity string.
    ///
    wstring getInstallIdent(unsigned l, unsigned i) {
      return this->_instIden[l][i];
    }

    /// \brief Set Batch title.
    ///
    /// Defines and save Batch title.
    ///
    /// \param[in]  title   : Title to defines and save
    ///
    void setTitle(const wstring& title);

    /// \brief Set Batch index.
    ///
    /// Defines and save Batch index for ordering.
    ///
    /// \param[in]  index   : Index number to defines and save
    ///
    void setIndex(unsigned index);

    /// \brief Add Location entry.
    ///
    /// Add the specified Location entry.
    ///
    /// \param[in]  uuid        : Location UUID.
    ///
    void addLocation(const wstring& uuid);

    /// \brief Remove Location from Batch.
    ///
    /// Remove an existing Location entry from Batch.
    ///
    /// \param[in]  uuid   : Location UUID.
    ///
    void remLocation(const wstring& uuid);

    /// \brief Get Context.
    ///
    /// Returns Batch related Context
    ///
    /// \return Pointer to Batch related Context.
    ///
    OmContext* context() const {
      return _context;
    }

    /// \brief Rename Batch definition file.
    ///
    /// Rename the Batch definition file associated with this instance.
    ///
    /// \param[in]  name   : New Batch file name without extension.
    ///
    bool rename(const wstring& name);

    /// \brief Close batch file
    ///
    /// Close and reset the current batch data and configuration file.
    ///
    void close();

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmContext*          _context;

    OmConfig            _config;

    wstring             _path;

    wstring             _uuid;

    wstring             _title;

    unsigned            _index;

    vector<wstring>     _locUuid;

    vector<vector<uint64_t>>  _instHash;

    vector<vector<wstring>>   _instIden;

    wstring             _error;
};

#endif // OMBATCH_H
