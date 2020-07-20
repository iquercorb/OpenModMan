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

#ifndef OMPACKAGE_H
#define OMPACKAGE_H

#include "OmGlobal.h"
#include "OmVersion.h"

class OmLocation;

/// \brief Package Type Mask
///
/// Package type masks enumerator.
///
enum PackageType : unsigned {
  PKG_TYPE_SRC = 0x1,     ///< Package has Source
  PKG_TYPE_BCK = 0x2,     ///< Package has Backup
  PKG_TYPE_ZIP = 0x4,     ///< Zip file Package
};


/// \brief Package Item Type
///
/// Package Item Type enumerator. Describe whether a package item is a
/// file or a directory.
///
enum OmPackageItemType {
  PKGITEM_TYPE_F = 0,     ///< File
  PKGITEM_TYPE_D = 1      ///< Directory
};

/// \brief Package Item Destination
///
/// Package Item backup destination enumerator. Describe whether a package
/// backup item is destined to be copied or removed to/from its destination.
///
enum OmPackageItemDest {
  PKGITEM_DEST_NUL = 0,   ///< N/A or Undefined
  PKGITEM_DEST_CPY = 1,   ///< Copy and overwrite to destination
  PKGITEM_DEST_DEL = 2    ///< Delete destination
};


/// \brief Package item.
///
/// Simple structure for package file or folder item.
///
struct OmPackageItem {

  OmPackageItemType   type; ///< Item type

  OmPackageItemDest   dest; ///< Item destination mask

  int                 cdri; ///<  Zip CDR index

  wstring             path; ///<  Relative path

};


/// \brief Abstract package object.
///
/// This class provide an abstracted interface for a package.
///
/// one package can represent two things which are the same in the
/// user's point of view, but totally different in software context: A "Backup"
/// and/or a "Source".
///
/// The "Backup" refers to saved files modified by the installed package in the
/// destination directory, while the "Source" refers to the package itself, the
/// files to be installed.

/// Our Package object is double-sided, it has one "Source" side and another
/// "Backup" side, and each Package object can have either "Source" or "Backup"
/// or both sides.
///
///     Backup File
///              \ _ Package Object
///              /
///     Source File
///
/// In this function, we create the package list the user will manipulate, and
/// Package objects will be created with one, or both side depending what is
/// found in backup and repository folders.
///
/// "Source" and "Backup" are linked together using an Uid which is the CRS64
/// of the file name string, for example:
///
///    MyPackage1.zip => fd964e3a45c789c7
///
/// When a "Source" is installed, this Uid is stored in the backup database of
/// this specific "Backup" file and will be used to link this "Backup" with an
/// existing "Source".
///
/// Notice that if user renames its package "Source" file, it will no longer be
/// linkable to an existing "Backup", it will be threated as a separate Package.
///
class OmPackage
{
  friend class OmLocation;

  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmPackage();

    /// \brief Constructor.
    ///
    /// Constructor with Location.
    ///
    /// \param[in]  pLoc    : Related Location for the Package.
    ///
    OmPackage(OmLocation* pLoc);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmPackage();

    /// \brief Get last error string.
    ///
    /// Returns the last encountered error string.
    ///
    /// \return Last error string.
    ///
    const wstring& lastError() const {
      return _error;
    }

    /// \brief Get package type.
    ///
    /// Returns package current type. Package type is bitwise mask which define
    /// what this package "is". The type mask are the followings:
    ///
    ///  - PKG_SRC : Package has (is) Source file and can be installed.
    ///  - PKG_ZIP : Package Source "is" a zip file (otherwise this is a folder).
    ///  - PKG_BCK : Package has (is) Backup file and can be restored.
    ///
    /// \return Current package type mask.
    ///
    unsigned type() const {
      return _type;
    }

    /// \brief Check package type.
    ///
    /// Checks whether package currently have the specified type, Package type is
    /// bitwise mask which define what this package "is". The type mask are the
    /// followings:
    ///
    ///  - PKG_SRC : Package has (is) Source file and can be installed.
    ///  - PKG_ZIP : Package Source "is" a zip file (otherwise this is a folder).
    ///  - PKG_BCK : Package has (is) Backup file and can be restored.
    ///
    /// \param[in]  mask    : Package type mask to test.
    ///
    /// \return True if package type matches the supplied mask.
    ///
    bool isType(unsigned mask) const {
      return ((_type & mask) == mask);
    }

    /// \brief Get Package identity.
    ///
    /// Returns package identity. This is raw file name, without zip extension,
    /// including version substring if exists.
    ///
    /// The package identity is used to identify one package with a specific
    /// version accross multiples
    ///
    /// \return Package display name.
    ///
    const wstring& ident() const {
      return _ident;
    }

    /// \brief Get package Hash.
    ///
    /// Returns package Hash computed from the package full filename, including
    /// extension if any.
    ///
    /// \return Package Hash.
    ///
    uint64_t hash() const {
      return _hash;
    }

    /// \brief Get Package name.
    ///
    /// Returns package display name. This is displayed name, built from file
    /// or folder file name, or as retrieved in the Backup definition.
    ///
    /// \return Package display name.
    ///
    const wstring& name() const {
      return _name;
    }

    /// \brief Get package Source file path.
    ///
    /// Returns package Source file path. This is the path to the file or folder
    /// this Package is referring to.
    ///
    /// \return Package Source file path.
    ///
    const wstring& sourcePath() const {
      return _source;
    }

    /// \brief Get package Backup file path.
    ///
    /// Returns package Backup file path. This is the path to the backup file this
    /// Package is referring to.
    ///
    /// \return Package Backup file path.
    ///
    const wstring& backupPath() const {
      return _backup;
    }

    /// \brief Check if this Package is Backup of a Source.
    ///
    /// Checks whether this package has the Backup side of the specified
    /// Source file based on UID.
    ///
    /// \param[in]  path    : Path to Source file or folder to test.
    ///
    /// \return True if this Package has the Backup side of the specified Source
    /// file, false otherwise.
    ///
    bool isBackupOf(const wstring& path) {
      return (Om_getXXHash3(Om_getFilePart(path)) == _hash);
    }

    /// \brief Parse Source file or folder.
    ///
    /// Analyses the supplied file or folder to check whether this is a valid
    /// package Source, and if yes, build up Source properties.
    ///
    /// \param[in]  path    : Path to Source file or folder to parse.
    ///
    /// \return True if supplied file or folder is a valid package source and
    /// operation succeed, false otherwise.
    ///
    bool sourceParse(const wstring& path);

    /// \brief Parse Backup file.
    ///
    /// Analyses the supplied file to check whether this is a valid
    /// package Backup, and if yes, build up Backup properties.
    ///
    /// \param[in]  path    : Path to Backup file to parse.
    ///
    /// \return True if supplied file is a valid package source and operation
    /// succeed, false otherwise.
    ///
    bool backupParse(const wstring& path);

    /// \brief Revoke Source.
    ///
    /// Revoke the Source side of this Package so it will no longer have
    /// a valid Source side.
    ///
    void sourceClear();

    /// \brief Revoke Backup.
    ///
    /// Revoke the Backup side of this Package so it will no longer have
    /// a valid Backup side.
    ///
    void backupClear();

    /// \brief Check source validity.
    ///
    /// Checks whether the previously parsed source of the package still
    /// available and valid. Otherwise, the Source status of Package is revoked
    /// and function returns false.
    ///
    /// This function is intended to be used as an ultimate verification to
    /// prevent unexpected crash or corruption in case the source file(s) no
    /// longer available since the last Package status refresh.
    ///
    /// \return True if previously parsed Sources still valid, false otherwise.
    ///
    bool sourceValid();

    /// \brief Check backup validity.
    ///
    /// Checks whether the previously parsed backup of the package still
    /// available and valid. Otherwise, the Backup status of Package is revoked
    /// and function returns false.
    ///
    /// This function is intended to be used as an ultimate verification to
    /// prevent unexpected crash or corruption in case the backup file(s) no
    /// longer available since the last Package status refresh.
    ///
    /// \return True if previously parsed backup still valid, false otherwise.
    ///
    bool backupValid();

    /// \brief Get backup list item count.
    ///
    /// Returns count of item in backup tree list.
    ///
    /// \return Count of item in backup tree list.
    ///
    size_t backupItemCount() const {
      return _backupItem.size();
    }

    /// \brief Get backup list item.
    ///
    /// Returns backup list item at specified index.
    ///
    /// \param[in]  i       : Index to get backup item.
    ///
    /// \return Backup list item at specified index.
    ///
    const OmPackageItem& backupItem(unsigned i) const {
      return _backupItem[i];
    }

    /// \brief Get source list item count.
    ///
    /// Returns count of item in source tree list.
    ///
    /// \return Count of item in source tree list.
    ///
    size_t sourceItemCount() const {
      return _sourceItem.size();
    }

    /// \brief Get source list item.
    ///
    /// Returns source list item at specified index.
    ///
    /// \param[in]  i       : Index to get source item.
    ///
    /// \return Source list item at specified index.
    ///
    const OmPackageItem& sourceItem(unsigned i) const {
      return _sourceItem[i];
    }

    /// \brief Get source dependencies count.
    ///
    /// Returns count of source dependencies packages.
    ///
    /// \return Count of source dependencies packages.
    ///
    size_t dependCount() const {
      return _depends.size();
    }

    /// \brief Get source dependency package.
    ///
    /// Returns source dependency Package.
    ///
    /// \param[in]  i       : Index of dependency Package.
    ///
    /// \return Source dependency Package.
    ///
    const wstring& depend(unsigned i) const {
      return _depends[i];
    }

    /// \brief Add Dependency
    ///
    /// Add dependency Package to this instance.
    ///
    /// \param[in]  name    : Dependency Package name to add
    ///
    void addDepend(const wstring& name) {
      for(size_t i = 0; i < _depends.size(); ++i) {
        if(name == _depends[i]) return;
      }
      _depends.push_back(name);
    }

    /// \brief Check installation overlapping.
    ///
    /// Checks whether this Package is susceptible to overwrite files that are
    /// already modified or installed by another one.
    ///
    /// \param[in]  other   : Other Package to check overlapping.
    ///
    /// \return True if the specified Package already have installed one or more
    /// files which this one could overwrite.
    ///
    bool couldOverlap(const OmPackage* other) const;

    /// \brief Get overlapped Package count.
    ///
    /// Returns count of overlapped Package by this one.
    ///
    /// \return Count of overlapped Package by this one.
    ///
    size_t overlapCount() const {
      return _overlap.size();
    }

    /// \brief Get overlapped package.
    ///
    /// Returns overlapped Package Hash value at specified index.
    ///
    /// \param[in]  i       : Index of overlapped Package.
    ///
    /// \return Hash value of overlapped Package.
    ///
    uint64_t overlap(unsigned i) const {
      return _overlap[i];
    }

    /// \brief Check overlapped package Hash.
    ///
    /// Checks whether this instance overlaps the specified Package.
    ///
    /// \param[in]  hash    : Package Hash to check.
    ///
    /// \return Hash value of overlapped Package.
    ///
    bool hasOverlap(uint64_t hash) const {
      for(size_t i = 0; i < _overlap.size(); ++i) {
        if(hash == _overlap[i]) return true;
      }
      return false;
    }

    /// \brief Peform uninstall.
    ///
    /// Restores the Package backup if it exists and cleanup
    /// destination directory.
    ///
    /// \param[in]  hSc    : Optional progress bar control handle (HWND) to step.
    /// \param[in]  pAbort  : Optional pointer to boolean to cancel operation.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool uninst(HWND hPb = nullptr, const bool *pAbort = nullptr);

    /// \brief Perform installation.
    ///
    /// Install Package files to the destination directory.
    ///
    /// \param[in]  zipLvl    : Zip compression level for backup.
    /// \param[in]  hSc       : Optional progress bar control handle (HWND) to step.
    /// \param[in]  pAbort    : Optional pointer to boolean to cancel operation.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool install(unsigned zipLvl = 0, HWND hPb = nullptr, const bool *pAbort = nullptr);

    /// \brief Check whether has Backup.
    ///
    /// Checks whether this Package has Backup data to be restored.
    ///
    /// \return True if Package is installed and has backup available.
    ///
    bool hasBackup() const {
      return (_type & PKG_TYPE_BCK);
    }

    /// \brief Check whether has Source.
    ///
    /// Checks whether this Package has Source data and can be installed.
    ///
    /// \return True if Package has Source data to be installed.
    ///
    bool hasSource() const {
      return (_type & PKG_TYPE_SRC);
    }

    /// \brief Check whether is an archive.
    ///
    /// Checks whether this Package Source is a Zip archive.
    ///
    /// \return True if Package Source is a Zip archive, false otherwise.
    ///
    bool isArchive() const {
      return (_type & PKG_TYPE_ZIP);
    }

    /// \brief Get description.
    ///
    /// Returns package description (README) content if any.
    ///
    /// \return Package description.
    ///
    const wstring& desc() const {
      return _desc;
    }

    /// \brief Set description.
    ///
    /// Sets Package description ((README) content.
    ///
    /// \param[in]  text    : Description string to set.
    ///
    void setDesc(const wstring& text) {
      _desc = text;
    }

    /// \brief Get version.
    ///
    /// Returns package version.
    ///
    /// \return Package version.
    ///
    const OmVersion& version() const {
      return _version;
    }

    /// \brief Get image.
    ///
    /// Returns package image if any.
    ///
    /// \return Package image or nullptr if not exists.
    ///
    HBITMAP picture() const {
      return _picture;
    }

    /// \brief Set image.
    ///
    /// Sets an image to this Package.
    ///
    /// \param[in]  hBmp    : Handle to bitmap (HBITMAP) to set as image.
    ///
    void setPicture(HBITMAP hBmp);

    /// \brief Get Location.
    ///
    /// Returns package related Location
    ///
    /// \return Pointer to Package related Location.
    ///
    OmLocation* location() const {
      return _location;
    }

    bool save(const wstring& path, unsigned zipLvl = 2, HWND hPb = nullptr, HWND hSc = nullptr, const bool *pAbort = nullptr);

    /// \brief Clear object.
    ///
    /// Reset this instance by clearing strings and data lists.
    ///
    void clear();

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Create backup data.
    ///
    /// Sub-routine for backup data creation
    ///
    /// \param[in]  zipLvl    : Zip compression level for backup.
    /// \param[in]  hSc       : Optional Progress bar control handle (HWND) to step.
    /// \param[in]  pAbort    : Optional Pointer to boolean to cancel operation.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool _doBackup(int zipLvl = 0, HWND hPb = nullptr, const bool *pAbort = nullptr);

    /// \brief Install source data.
    ///
    /// Sub-routine for package install from source
    ///
    /// \param[in]  hSc      : Optional Progress bar control handle (HWND) to step.
    /// \param[in]  pAbort   : Optional Pointer to boolean to cancel operation.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool _doInstall(HWND hPb = nullptr, const bool *pAbort = nullptr);

    /// \brief Uinstall package and restore backup data.
    ///
    /// Sub-routine for uninstall and backup data restoration
    ///
    /// \param[in]  hSc      : Optional Progress bar control handle (HWND) to step.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool _doUninst(HWND hPb = nullptr);

    /// \brief Undo backup data.
    ///
    /// Sub-routine to restore partial, erroneous or aborted backup
    ///
    /// \param[in]  hSc      : Optional Progress bar control handle (HWND) to step.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    void _undoInstall(HWND hPb = nullptr);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    unsigned            _type;

    wstring             _ident;

    uint64_t            _hash;

    wstring             _name;

    wstring             _source;

    wstring             _sourceDir;

    vector<OmPackageItem>  _sourceItem;

    vector<wstring>     _depends;

    wstring             _backup;

    wstring             _backupDir;

    vector<OmPackageItem>  _backupItem;

    vector<uint64_t>    _overlap;

    wstring             _desc;

    OmVersion           _version;

    HBITMAP             _picture;

    OmLocation*         _location;

    wstring             _error;
};

#endif // OMPACKAGE_H
