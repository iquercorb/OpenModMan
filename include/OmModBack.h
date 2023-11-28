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
#ifndef OMMODBACK_H
#define OMMODBACK_H

#include "OmBase.h"
#include "OmVersion.h"
#include <algorithm>

#define MODBACK_FILES_ROOT   L"root"

/// \brief Mod Entry structure
///
/// Structure to describe a Mod entry, which is a file or folder to be
/// installed or restored which Package or Backup contain or references.
///
struct OmModEnt_
{
  int32_t   zcdi;
  bool      is_dir;
  bool      to_del;
  wstring   path;
};

/// \brief Mod Backup Object
///
/// Object to interface Mod Backup data related to Mod Pack installation
///
class OmModBack_
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmModBack_();

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmModBack_();

    /// \brief Parse Mod Backup
    ///
    /// Parse the given file or directory as Mod Backup
    ///
    /// \param[in]  path    : Path to archive file or directory to parse.
    ///
    /// \return True if parse succeed, false otherwise
    ///
    bool parse(const wstring& path);

    /// \brief Mod Backup path
    ///
    /// Mod Backup file or directory path.
    ///
    /// \return Wide string
    ///
    const wstring& path() const {
      return this->_path;
    }

    /// \brief Backup entries count
    ///
    /// Returns count of entries this Mod Backup contain
    ///
    /// \return Unsigned integer
    ///
    const size_t entryCount() const {
      return this->_entry.size();
    }

    /// \brief Backup entry
    ///
    /// Returns Backup entry at specified index
    ///
    /// \param[in]  i   : Entry index to get
    ///
    /// \return Mod Entry (OmModEnt) structure
    ///
    const OmModEnt_& entry(size_t i) const {
      return this->_entry[i];
    }

    /// \brief Overlapped Mod count
    ///
    /// Returns count of overlapped Mod referenced by this Backup data.
    ///
    /// When Mods are installed it occurs that some installed files overlaps ones
    /// already modified files by the installation of a previous Mod. The Mod already
    /// installed whom installed files was subsequently overwritten by another Mod
    /// are called "overlapped".
    ///
    /// \return Unsigned integer
    ///
    size_t overlapCount() const {
      return _overlap.size();
    }

    /// \brief Overlapped Mod hash
    ///
    /// Returns overlapped Mod identity hash at specified index.
    ///
    /// When Mods are installed it occurs that some installed files overlaps ones
    /// already modified files by the installation of a previous Mod. The Mod already
    /// installed whom installed files was subsequently overwritten by another Mod
    /// are called "overlapped".
    ///
    /// \param[in]  i   : Reference index to get
    ///
    /// \return 64 bit unsigned integer xxHahs value
    ///
    size_t overlapHash(size_t i) const {
      return _overlap[i];
    }

    /// \brief Check for overlapped Mod hash
    ///
    /// Checks whether overlapped Mod identity hash is referenced by this
    /// Backup data.
    ///
    /// \param[in]  hash    : Mod identity hash to test
    ///
    /// \return True if Mod hash is referenced, false otherwise
    ///
    bool hasOverlap(uint64_t hash) const {
      return (std::find(this->_overlap.begin(), this->_overlap.end(), hash) != this->_overlap.end());
    }

    /// \brief Mod hash value
    ///
    /// Mod filename hash value the backup data is related to
    ///
    /// \return 64 bit unsigned integer xxHahs value
    ///
    uint64_t hash() const {
      return this->_hash;
    }

    /// \brief Mod identity
    ///
    /// Mod identity string the backup data is related to
    ///
    /// \return Wide string
    ///
    const wstring& iden() const {
      return this->_iden;
    }

    /// \brief Mod displayed name
    ///
    /// Mod displayed name string parsed from Mod identity.
    ///
    /// \return Wide string
    ///
    const wstring& name() const {
      return this->_name;
    }

    /// \brief Mod core name
    ///
    /// Mod core name string parsed from Mod identity.
    ///
    /// \return Wide string
    ///
    const wstring& core() const {
      return this->_core;
    }

    /// \brief Mod version
    ///
    /// Mod version parsed from Mod identity.
    ///
    /// \return Version (OmVersion) object
    ///
    const OmVersion& vers() const {
      return this->_vers;
    }

    /// \brief Check validity
    ///
    /// Returns whether this instance is a valid Mod Backup.
    ///
    /// \return True if valid Mod Backup, false otherwise
    ///
    bool valid() const {
      return this->_valid;
    }

    /// \brief Get last error string.
    ///
    /// Returns the last encountered error string.
    ///
    /// \return Wide string.
    ///
    const wstring& lastError() const {
      return this->_error;
    }

    /// \brief Clear instance
    ///
    /// Clear everything and resets instance to its initial state.
    ///
    /// \return Wide string.
    ///
    void clear();

  private:

    bool                  _valid;

    wstring               _path;

    wstring               _iden;

    uint64_t              _hash;

    wstring               _core;

    wstring               _name;

    OmVersion             _vers;

    bool                  _isdir;

    vector<OmModEnt_>      _entry;

    vector<uint64_t>      _overlap;

    wstring               _error;
};

#endif // OMMODBACK_H
