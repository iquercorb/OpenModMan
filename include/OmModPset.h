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

#ifndef OMMODPSET_H
#define OMMODPSET_H

#include "OmBase.h"
#include "OmXmlConf.h"

class OmModHub;
class OmModChan;
class OmModPack;

/// \brief Batch object.
///
/// Object for Package installation batch
///
class OmModPset
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmModPset();

    /// \brief Constructor.
    ///
    /// Constructor with Mod Hub.
    ///
    /// \param[in]  pModHub  : Related Mod Hub for the Batch.
    ///
    OmModPset(OmModHub* pModHub);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmModPset();

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
    bool open(const OmWString& path);

    /// \brief Save definition
    ///
    /// Save the Preset definition file.
    ///
    void save();

    /// \brief Close batch file
    ///
    /// Close and reset the current batch data and configuration file.
    ///
    void close();

    /// \brief Get Batch uuid.
    ///
    /// Returns Batch uuid.
    ///
    /// \return Batch uuid.
    ///
    const OmWString& uuid() const {
      return _uuid;
    }

    /// \brief Get Batch title.
    ///
    /// Returns Batch title.
    ///
    /// \return Batch title.
    ///
    const OmWString& title() const {
      return _title;
    }

    /// \brief Get Preset index.
    ///
    /// Returns Preset ordering index.
    ///
    /// \return Ordering index number.
    ///
    unsigned index() const {
      return _index;
    }

    /// \brief Get Preset definition file path.
    ///
    /// Returns Preset definition file path.
    ///
    /// \return Preset definition file path.
    ///
    const OmWString& path() const {
      return _path;
    }

    /// \brief Get Install-Only option.
    ///
    /// Returns the Install-Only option for this instance
    ///
    /// \return Install-Only option value.
    ///
    bool installOnly() {
      return _installonly;
    }

    /// \brief Set Preset title.
    ///
    /// Defines and save Preset title.
    ///
    /// \param[in]  title   : Title to defines and save
    ///
    void setTitle(const OmWString& title);

    /// \brief Set Preset index.
    ///
    /// Defines and save Preset index for ordering.
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

    /// \brief Get Setup count
    ///
    /// Returns count of referenced Mod Channel Setup in this instance.
    ///
    /// \return Count of referenced Setups
    ///
    size_t setupCount();

    /// \brief Discard Setup
    ///
    /// Removes reference and install list of the specified Mod Channel setup
    ///
    /// \param[in] uuid   : Mod Channel UUID
    ///
    /// \return True if operation succeed, false if reference not found.
    ///
    bool discardSetup(const OmWString& uuid);

    /// \brief Discard Setup
    ///
    /// Removes reference and install list of the specified Mod Channel Setup
    ///
    /// \param[in] ModChan  : Pointer to Mod Channel
    ///
    /// \return True if operation succeed, false if reference not found.
    ///
    bool discardSetup(const OmModChan* ModChan) {
      return this->discardSetup(ModChan->uuid());
    }

    /// \brief Add package to install list.
    ///
    /// Add the given Package references to the installation list of the
    /// specified Mod Channel.
    ///
    /// \param[in]  ModChan : Pointer to Mod Channel object.
    /// \param[in]  ModPack : Pointer to Package object to reference.
    ///
    void addSetupEntry(const OmModChan* ModChan, const OmModPack* ModPack);

    /// \brief Remove package from install list.
    ///
    /// Remove the specified Package references from the installation list of
    /// the specified Mod Channel.
    ///
    /// \param[in]  ModChan : Pointer to Mod Channel object.
    /// \param[in]  iden    : Package identity to search and remove.
    ///
    /// \return true if reference was removed, false otherwise
    ///
    bool deleteSetupEntry(const OmModChan* ModChan, const OmWString& iden);

    /// \brief Replace package entry of install list.
    ///
    /// Replace the specified Package references by the new specified one.
    ///
    /// \param[in]  ModChan : Pointer to Mod Channel object.
    /// \param[in]  iden    : Package identity to be replaced.
    /// \param[in]  ModPack : Pointer to new Package object.
    ///
    /// \return true if reference was removed, false otherwise
    ///
    bool replaceSetupEntry(const OmModChan* ModChan, const OmWString& iden, const OmModPack* ModPack);

    /// \brief Get install list size.
    ///
    /// Returns size of the install list for the specified Mod Channel.
    ///
    /// \param[in]  ModChan : Pointer to Mod Channel object.
    ///
    /// \return Size of install list or 0 if Mod Channel not found.
    ///
    size_t setupEntryCount(const OmModChan* ModChan);

    /// \brief Get install list Mod Pack
    ///
    /// Returns the found Mod Pack, in the given Mod Channel, corresponding
    /// to the referenced item in the install list.
    ///
    /// \param[in]  ModChan : Pointer to Mod Channel object.
    /// \param[in]  index   : Index of reference in install list.
    ///
    /// \return Pointer to Package object or nullptr if not found.
    ///
    OmModPack* getSetupEntry(const OmModChan* ModChan, size_t index);

    /// \brief Install list has Mod
    ///
    /// Checks whether Install List for the given Mod Channel contain
    /// the specified Mod Package
    ///
    /// \param[in]  ModChan : Pointer to Mod Channel object
    /// \param[in]  ModPack : Pointer to Mod Pack to search
    ///
    /// \return True if Mod Mod Pack was found, false otherwise
    ///
    bool setupHasEntry(const OmModChan* ModChan, OmModPack* ModPack);

    /// \brief Get install list
    ///
    /// Returns the list of packages, in the given Mod Channel,
    /// corresponding to referenced install list items.
    ///
    /// \param[in]  ModChan : Pointer to Mod Channel object
    /// \param[in]  mod_ls  : Pointer to Mod Pack array to be filled
    ///
    /// \return Count of found item.
    ///
    size_t getSetupEntryList(const OmModChan* ModChan, OmPModPackArray* mod_ls);

    /// \brief Repair config.
    ///
    /// Verify then remove or repair invalid or broken references according
    /// current bound context.
    ///
    /// \return True if operation succeed, false if bound context missing.
    ///
    bool repair();

    /// \brief Lock instance
    ///
    /// Lock this instance so it cannot be modified.
    ///
    void lock() {
      this->_locked = true;
    }

    /// \brief Unlock instance
    ///
    /// Unlock this instance so it can be modified.
    ///
    void unlock() {
      this->_locked = false;
    }

    /// \brief Check whether instance is locked
    ///
    /// Returns whether this instance is locked and cannot be modified
    ///
    /// \return True if instance is locked, false otherwise
    ///
    bool locked() const {
      return this->_locked;
    }

    /// \brief Get Mod Hub.
    ///
    /// Returns Preset related Mod Hub
    ///
    /// \return Pointer to Preset related Mod Hub.
    ///
    OmModHub* ModHub() const {
      return _ModHub;
    }

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const OmWString& lastError() const {
      return _lasterr;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // linking
    OmModHub*           _ModHub;

    // XML definition
    OmXmlConf           _xml;

    // contextual properties
    bool                _locked;

    // common properties
    OmWString           _path;

    OmWString           _uuid;

    OmWString           _title;

    unsigned            _index;

    // optional properties
    bool                _installonly;

    // logs and errors
    void                _log(unsigned level, const OmWString& origin, const OmWString& detail);

    void                _error(const OmWString& origin, const OmWString& detail);

    OmWString           _lasterr;
};

/// \brief OmModPset pointer array
///
/// Typedef for an STL vector of OmModPset pointer type
///
typedef std::vector<OmModPset*> OmPModPsetArray;

/// \brief OmModPset pointer queue
///
/// Typedef for an STL deque of OmModPset pointer type
///
typedef std::deque<OmModPset*> OmPModPsetQueue;

#endif // OMMODPSET_H
