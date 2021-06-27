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
#include "OmImage.h"

class OmLocation;

/// \brief Package Type Mask
///
/// Package type masks enumerator.
///
enum OmPkgType : unsigned {
  PKG_TYPE_SRC = 0x1,     ///< Package has Source
  PKG_TYPE_BCK = 0x2,     ///< Package has Backup
  PKG_TYPE_ZIP = 0x4,     ///< Zip file Package
};


/// \brief Package Item Type
///
/// Package Item Type enumerator. Describe whether a package item is a
/// file or a directory.
///
enum OmPkgItemType {
  PKGITEM_TYPE_F = 0,     ///< File
  PKGITEM_TYPE_D = 1      ///< Directory
};

/// \brief Package Item Destination
///
/// Package Item backup destination enumerator. Describe whether a package
/// backup item is destined to be copied or removed to/from its destination.
///
enum OmPkgItemDest {
  PKGITEM_DEST_NUL = 0,   ///< N/A or Undefined
  PKGITEM_DEST_CPY = 1,   ///< Copy and overwrite to destination
  PKGITEM_DEST_DEL = 2    ///< Delete destination
};


/// \brief Package item.
///
/// Simple structure for package file or folder item.
///
struct OmPkgItem
{
  OmPkgItemType   type; ///< Item type

  OmPkgItemDest   dest; ///< Item destination mask

  int             cdri; ///<  Zip CDR index

  wstring         path; ///<  Relative path
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

    /// \brief Get type.
    ///
    /// Returns package type bit field, this can be a combination of the following
    /// bit masks:
    ///
    ///  - PKG_TYPE_SRC : Package has (is) source data and can be installed.
    ///  - PKG_TYPE_ZIP : Package source data is a zip file (otherwise this is a folder).
    ///  - PKG_TYPE_BCK : Package has (is) backup data and can be restored.
    ///
    /// \return Bit field that describe package type.
    ///
    unsigned type() const {
      return _type;
    }

    /// \brief Check type.
    ///
    /// Checks whether remote package currently have the specified type mask defined,
    /// possibles masks are the followings:
    ///
    ///  - PKG_TYPE_SRC : Package has (is) source data and can be installed.
    ///  - PKG_TYPE_ZIP : Package source data is a zip file (otherwise this is a folder).
    ///  - PKG_TYPE_BCK : Package has (is) backup data and can be restored.
    ///
    /// \param[in]  mask    : Type mask to test.
    ///
    /// \return True if type matches the specified mask, false otherwise.
    ///
    bool isType(unsigned mask) const {
      return ((_type & mask) == mask);
    }

    /// \brief Get identity.
    ///
    /// Returns package identity, this is the package raw full name, it is
    /// the file name without file extension.
    ///
    /// This value is used to uniquely identify package with its specific
    /// version independently of its file type. A package may have the same
    /// identity simultaneously through three forms: as .zip file, .omp file
    /// or as folder.
    ///
    /// \return Package identity.
    ///
    const wstring& ident() const {
      return _ident;
    }

    /// \brief Get identity hash.
    ///
    /// Returns package identity hash, this is an hash value computed from
    /// filename, used to identify it as an unique file system entity.
    ///
    /// This value is used to uniquely identify each parsed package source
    /// or backup, since a packages may have the same identity simultaneously
    /// through three forms: as .zip file, .omp file or as folder.
    ///
    /// \return Package identity hash.
    ///
    uint64_t hash() const {
      return _hash;
    }

    /// \brief Get core name.
    ///
    /// Returns package core name, this is the "master" name of package
    /// used to identify it whatever its version.
    ///
    /// This value is used to to evaluate changes in versions of what should
    /// be considered as the same package.
    ///
    /// \return Package core name.
    ///
    const wstring& core() const {
      return _core;
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

    /// \brief Get displayed name.
    ///
    /// Returns package displayed name, this is the prettified name to
    /// be displayed, it has only a cosmetic role.
    ///
    /// \return Package displayed name.
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
    const wstring& srcPath() const {
      return _src;
    }

    /// \brief Get package Backup file path.
    ///
    /// Returns package Backup file path. This is the path to the backup file this
    /// Package is referring to.
    ///
    /// \return Package Backup file path.
    ///
    const wstring& bckPath() const {
      return _bck;
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
    bool isBckOf(const wstring& path) {
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
    bool srcParse(const wstring& path);

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
    bool bckParse(const wstring& path);

    /// \brief Revoke Source.
    ///
    /// Revoke the Source side of this Package so it will no longer have
    /// a valid Source side.
    ///
    void srcClear();

    /// \brief Revoke Backup.
    ///
    /// Revoke the Backup side of this Package so it will no longer have
    /// a valid Backup side.
    ///
    void bckClear();

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
    bool srcValid();

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
    bool bckValid();

    /// \brief Get backup list item count.
    ///
    /// Returns count of item in backup tree list.
    ///
    /// \return Count of item in backup tree list.
    ///
    size_t bckItemCount() const {
      return _bckItemLs.size();
    }

    /// \brief Get backup list item.
    ///
    /// Returns backup list item at specified index.
    ///
    /// \param[in]  i       : Index to get backup item.
    ///
    /// \return Backup list item at specified index.
    ///
    const OmPkgItem& bckItemGet(unsigned i) const {
      return _bckItemLs[i];
    }

    /// \brief Get source list item count.
    ///
    /// Returns count of item in source tree list.
    ///
    /// \return Count of item in source tree list.
    ///
    size_t srcItemCount() const {
      return _srcItemLs.size();
    }

    /// \brief Get source list item.
    ///
    /// Returns source list item at specified index.
    ///
    /// \param[in]  i       : Index to get source item.
    ///
    /// \return Source list item at specified index.
    ///
    const OmPkgItem& srcItemGet(unsigned i) const {
      return _srcItemLs[i];
    }

    /// \brief Get dependencies count.
    ///
    /// Returns count of source dependencies packages.
    ///
    /// \return Count of source dependencies packages.
    ///
    size_t depCount() const {
      return _depLs.size();
    }

    /// \brief Get dependency identity.
    ///
    /// Returns dependency package identity at index.
    ///
    /// \param[in]  i       : Index of dependency.
    ///
    /// \return Dependency identity.
    ///
    const wstring& depGet(unsigned i) const {
      return _depLs[i];
    }

    /// \brief Get dependency list.
    ///
    /// Returns list of dependencies identity strings.
    ///
    /// \return Identity strings array.
    ///
    const vector<wstring>& depList() const {
      return _depLs;
    }

    /// \brief Add Dependency
    ///
    /// Add dependency package identity to this instance.
    ///
    /// \param[in]  ident    : Dependency identity identity to add
    ///
    void depAdd(const wstring& ident) {
      for(size_t i = 0; i < this->_depLs.size(); ++i) {
        if(ident == this->_depLs[i]) return;
      }
      this->_depLs.push_back(ident);
    }

    /// \brief Check dependency
    ///
    /// Checks whether package has the specified package identity as dependency.
    ///
    /// \param[in]  ident    : Dependency package identity to check
    ///
    bool depHas(const wstring& ident) {
      for(size_t i = 0; i < this->_depLs.size(); ++i) {
        if(ident == this->_depLs[i]) return true;
      }
      return false;
    }

    /// \brief Get install footprint.
    ///
    /// Simulate package installation and create backup item list as if
    /// package were installed according current state.
    ///
    /// \param[out] footprint  : Output backup item list resulting of simulated install.
    ///
    void footprint(vector<OmPkgItem>& footprint) const;

    /// \brief Check installation overlapping.
    ///
    /// Checks whether this Package is susceptible to overwrite files that are
    /// already modified or installed by another one.
    ///
    /// \param[in]  footprint : Backup item list check overlapping.
    ///
    /// \return True if the specified Package already have installed one or more
    /// files which this one could overwrite.
    ///
    bool ovrTest(const vector<OmPkgItem>& footprint) const;

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
    bool ovrTest(const OmPackage* other) const {
      return ovrTest(other->_bckItemLs);
    }

    /// \brief Get overlapped Package count.
    ///
    /// Returns count of overlapped Package by this one.
    ///
    /// \return Count of overlapped Package by this one.
    ///
    size_t ovrCount() const {
      return _ovrLs.size();
    }

    /// \brief Get overlapped package.
    ///
    /// Returns overlapped Package Hash value at specified index.
    ///
    /// \param[in]  i       : Index of overlapped Package.
    ///
    /// \return Hash value of overlapped Package.
    ///
    uint64_t ovrGet(unsigned i) const {
      return _ovrLs[i];
    }

    /// \brief Check overlapped package Hash.
    ///
    /// Checks whether this instance overlaps the specified Package.
    ///
    /// \param[in]  hash    : Package Hash to check.
    ///
    /// \return Hash value of overlapped Package.
    ///
    bool ovrHas(uint64_t hash) const {
      for(size_t i = 0; i < this->_ovrLs.size(); ++i) {
        if(hash == this->_ovrLs[i]) return true;
      }
      return false;
    }

    /// \brief Peform uninstall.
    ///
    /// Restores the Package backup if it exists and cleanup
    /// destination directory.
    ///
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool uninst(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Perform installation.
    ///
    /// Install Package files to the destination directory.
    ///
    /// \param[in]  zipLvl      : Zip compression level for backup.
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool install(unsigned zipLvl = 0, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Perform unbackup.
    ///
    /// Delete the Package backup without restoring data, set it as uninstalled.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool unbackup();

    /// \brief Check whether has Backup.
    ///
    /// Checks whether this Package has Backup data to be restored.
    ///
    /// \return True if Package is installed and has backup available.
    ///
    bool hasBck() const {
      return (_type & PKG_TYPE_BCK);
    }

    /// \brief Check whether has Source.
    ///
    /// Checks whether this Package has Source data and can be installed.
    ///
    /// \return True if Package has Source data to be installed.
    ///
    bool hasSrc() const {
      return (_type & PKG_TYPE_SRC);
    }

    /// \brief Check whether is an archive.
    ///
    /// Checks whether this Package Source is a Zip archive.
    ///
    /// \return True if Package Source is a Zip archive, false otherwise.
    ///
    bool isZip() const {
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

    /// \brief Get image.
    ///
    /// Returns package image object.
    ///
    /// \return Package image object.
    ///
    const OmImage& image() const {
      return _image;
    }

    /// \brief Set image.
    ///
    /// Load an image and create thumbnail for
    /// this Package.
    ///
    /// \param[in]  path  : Path to image file to load.
    /// \param[in]  size  : Image thumbnail size.
    ///
    void loadImage(const wstring& path, unsigned size);

    /// \brief Clear image.
    ///
    /// Clear image data from this Package.
    ///
    void clearImage();

    /// \brief Get owner Location.
    ///
    /// Returns Location that own this package.
    ///
    /// \return Pointer to Location or nullptr.
    ///
    OmLocation* ownerLoc() const {
      return _location;
    }

    /// \brief Save Package.
    ///
    /// Create a new package file from this package
    ///
    /// \param[in]  path        : Destination path and filename to save package as.
    /// \param[in]  zipLvl      : Package Zip compression level.
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return Pointer to Package related Location.
    ///
    bool save(const wstring& path, unsigned zipLvl = 2, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

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
    /// \param[in]  zipLvl      : Zip compression level for backup.
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool _doBackup(int zipLvl = 0, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Install source data.
    ///
    /// Sub-routine for package install from source
    ///
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool _doInstall(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Uninstall package and restore backup data.
    ///
    /// Sub-routine for uninstall and backup data restoration
    ///
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool _doUninst(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Undo backup data.
    ///
    /// Sub-routine to restore partial, erroneous or aborted backup
    ///
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    void _undoInstall(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Discard backup data.
    ///
    /// Sub-routine to delete backup data without restoring.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool _doUnbackup();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmLocation*         _location;

    unsigned            _type;

    wstring             _ident;

    uint64_t            _hash;

    wstring             _core;

    OmVersion           _version;

    wstring             _name;

    wstring             _src;

    wstring             _srcDir;

    vector<OmPkgItem>   _srcItemLs;

    vector<wstring>     _depLs;

    wstring             _bck;

    wstring             _bckDir;

    vector<OmPkgItem>   _bckItemLs;

    vector<uint64_t>    _ovrLs;

    wstring             _desc;

    OmImage             _image;

    wstring             _error;
};

#endif // OMPACKAGE_H
