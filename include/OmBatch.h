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

#include "OmBase.h"
#include "OmConfig.h"

class OmContext;
class OmLocation;
class OmPackage;

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
    OmBatch(OmContext* pCtx);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmBatch();

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
    bool open(const wstring& path);

    /// \brief Initialize Batch definition file.
    ///
    /// Create new Batch definition file at given path.
    ///
    /// \param[in]  path    : Path to definition file to create.
    /// \param[in]  title   : Installation Batch title.
    /// \param[in]  index   : Initial list index to set.
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool init(const wstring& path, const wstring& title, unsigned index = 0);

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

    /// \brief Get Install-Only option.
    ///
    /// Returns the Install-Only option for this instance
    ///
    /// \return Install-Only option value.
    ///
    bool installOnly() {
      return _instOnly;
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

    /// \brief Set Install-Only option.
    ///
    /// Defines the Install-Only option for this instance
    ///
    /// \param[in]  enable  : Value to set.
    ///
    void setInstallOnly(bool enable);

    /// \brief Get Target Location count
    ///
    /// Returns count of referenced Target Location in this instance.
    ///
    /// \return Count of referenced Target Location
    ///
    size_t locCount();

    /// \brief Get Target Location uuid
    ///
    /// Returns referenced Target Location UUID at specified index.
    ///
    /// \param[in]  i    : Target Location index.
    ///
    /// \return Target Location's UUID.
    ///
    wstring locUuid(unsigned i);

    /// \brief Discard Target Location
    ///
    /// Removes reference and install list of the  Target Location
    /// with the specified UUID.
    ///
    /// \param[in]  uuid : Target Location UUID.
    ///
    /// \return True if operation succeed, false if reference not found.
    ///
    bool locDiscard(const wstring& uuid);

    /// \brief Clear install list.
    ///
    /// Clear the install list of the specified Target Location, if Target
    /// Location has no reference in the current Batch this operation has
    /// no effect.
    ///
    /// \param[in]  pLoc    : Pointer to Target Location.
    ///
    void instClear(const OmLocation* pLoc);

    /// \brief Add package to install list.
    ///
    /// Add the given Package references to the installation list of the
    /// specified Target Location.
    ///
    /// \param[in]  pLoc    : Pointer to Target Location object.
    /// \param[in]  pPkg    : Pointer to Package object to reference.
    ///
    void instAdd(const OmLocation* pLoc, const OmPackage* pPkg);

    /// \brief Discard install list package
    ///
    /// Removes the specified Package reference from the install list of the.
    /// specified Target Location.
    ///
    /// \param[in]  pLoc    : Pointer to Target Location object.
    /// \param[in]  pPkg    : Pointer to Package object to reference.
    ///
    void instDiscard(const OmLocation* pLoc, const OmPackage* pPkg);

    /// \brief Get install list size.
    ///
    /// Returns size of the install list for the specified Target Location.
    ///
    /// \param[in]  pLoc    : Pointer to Target Location object.
    ///
    /// \return Size of install list or 0 if Target Location not found.
    ///
    size_t instSize(const OmLocation* pLoc);

    /// \brief Get install list package
    ///
    /// Returns the found package, in the given Target Location, corresponding
    /// to the referenced item in the install list.
    ///
    /// \param[in]  pLoc    : Pointer to Target Location object.
    /// \param[in]  i       : Index of reference in install list.
    ///
    /// \return Pointer to Package object or nullptr if not found.
    ///
    OmPackage* instGet(const OmLocation* pLoc, unsigned i);

    /// \brief Get install list
    ///
    /// Returns the list of packages, in the given Target Location,
    /// corresponding to referenced install list items.
    ///
    /// \param[in]  pLoc    : Pointer to Target Location object.
    /// \param[in]  pkg_ls  : Array that receive list of Package objects.
    ///
    /// \return Count of found item.
    ///
    size_t instGetList(const OmLocation* pLoc, vector<OmPackage*>& pkg_ls);

    /// \brief Repair config.
    ///
    /// Verify then remove or repair invalid or broken references according
    /// current bound context.
    ///
    /// \return True if operation succeed, false if bound context missing.
    ///
    bool repair();

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

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const wstring& lastError() const {
      return _error;
    }

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

    /// \brief Get Context.
    ///
    /// Returns Batch related Context
    ///
    /// \return Pointer to Batch related Context.
    ///
    OmContext* pCtx() const {
      return _context;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmContext*          _context;

    OmConfig            _config;

    wstring             _path;

    wstring             _uuid;

    wstring             _title;

    unsigned            _index;

    bool                _instOnly;

    wstring             _error;
};

#endif // OMBATCH_H
