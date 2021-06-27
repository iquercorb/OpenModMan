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
#ifndef OMREMOTE_H
#define OMREMOTE_H

#include "OmGlobal.h"
#include "OmXmlDoc.h"
#include "OmVersion.h"
#include "OmImage.h"

class OmLocation;
class OmPackage;

/// \brief Remote package Type Mask
///
/// Remote package type masks enumerator.
///
enum OmRmtState : unsigned {
  RMT_STATE_NEW = 0x1,     ///< Remote package is new (not in local library)
  RMT_STATE_UPG = 0x2,     ///< Remote package is an upgrade (newer version)
  RMT_STATE_OLD = 0x4,     ///< Remote package is a downgrade (older version)
  RMT_STATE_DEP = 0x8,     ///< Remote package is downloaded but has missing dependencies
  RMT_STATE_DNL = 0x10,    ///< Remote package is downloading
  RMT_STATE_ERR = 0x20,    ///< Remote package download error
};

/// \brief Abstract remote package.
///
/// This class provide an abstracted interface for a remote package.
///
class OmRemote
{
  friend class OmLocation;

  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmRemote();

    /// \brief Constructor.
    ///
    /// Constructor with Location.
    ///
    /// \param[in]  pLoc    : Related Location for the Package.
    ///
    OmRemote(OmLocation* pLoc);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmRemote();

    /// \brief Parse definition.
    ///
    /// Parse the specified XML node as remote package definition.
    ///
    /// \param[in]  url    : Repository file path URL.
    /// \param[in]  xml    : XML repository <entry> node to parse.
    ///
    bool parse(const wstring& url, OmXmlNode entry);

    /// \brief Get last error string.
    ///
    /// Returns the last encountered error string.
    ///
    /// \return Last error string.
    ///
    const wstring& lastError() const {
      return _error;
    }

    /// \brief Download URL count.
    ///
    /// Returns count of defined download URL.
    ///
    /// \return Count of defined URL.
    ///
    size_t urlCount() const {
      return _url.size();
    }

    /// \brief Get download URL.
    ///
    /// Returns file download URL at index.
    ///
    /// \param[in] i      : Index to get URL.
    ///
    /// \return File download URL.
    ///
    const wstring& urlGet(unsigned i) const {
      return _url[i];
    }

    /// \brief Add download URL.
    ///
    /// Add a file download URL to list.
    ///
    /// \param[in] url    : URL string to add.
    ///
    /// \return True if URL appear valid, false otherwise.
    ///
    bool urlAdd(const wstring& url);

    /// \brief Get file name.
    ///
    /// Returns remote package file name.
    ///
    /// \return File name.
    ///
    const wstring& file() const {
      return _file;
    }

    /// \brief Get file size.
    ///
    /// Returns remote package file size in bytes.
    ///
    /// \return File size in bytes.
    ///
    size_t bytes() const {
      return _bytes;
    }

    /// \brief Get file checksum.
    ///
    /// Returns remote package file checksum.
    ///
    /// \return File checksum string.
    ///
    const wstring& checksum() const {
      return _checksum;
    }

    /// \brief Get state.
    ///
    /// Returns remote package state bit field, this can be a combination of the following
    /// bit masks:
    ///
    ///  - RMT_STATE_NEW : Remote package does not exists local library and can be downloaded.
    ///  - RMT_STATE_UPG : Remote package is an upgrade of an existing package in local library.
    ///  - RMT_STATE_OLD : Remote package is a downgrade of an existing package in local library.
    ///  - RMT_STATE_LOC : Remote package exists locally
    ///  - RMT_STATE_DNL : Remote package is downloading
    ///
    /// \return Bit field that describe remote package state.
    ///
    unsigned state() const {
      return _state;
    }

    /// \brief Check state.
    ///
    /// Checks whether remote package currently have the specified state mask defined,
    /// possibles masks are the followings:
    ///
    ///  - RMT_STATE_NEW : Remote package does not exists local library and can be downloaded.
    ///  - RMT_STATE_UPG : Remote package is an upgrade of an existing package in local library.
    ///  - RMT_STATE_OLD : Remote package is a downgrade of an existing package in local library.
    ///  - RMT_STATE_LOC : Remote package exists locally
    ///  - RMT_STATE_DNL : Remote package is downloading
    ///
    /// \param[in]  mask    : Type mask to test.
    ///
    /// \return True if state matches the specified mask, false otherwise.
    ///
    bool isState(unsigned mask) const {
      return ((_state & mask) == mask);
    }

    /// \brief Get identity.
    ///
    /// Returns remote package identity, this is the package raw full name, it is
    /// the file name without file extension.
    ///
    /// This value is used to uniquely identify package with its specific
    /// version independently of its file type. A package may have the same
    /// identity simultaneously through three forms: as .zip file, .omp file
    /// or as folder.
    ///
    /// \return Remote package identity.
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
    /// \return Remote package identity hash.
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
    /// \return Remote package core name.
    ///
    const wstring& core() const {
      return _core;
    }

    /// \brief Get version.
    ///
    /// Returns remote package version object, as standard package.
    ///
    /// \return Remote package version.
    ///
    const OmVersion& version() const {
      return _version;
    }

    /// \brief Get displayed name.
    ///
    /// Returns package displayed name, this is the prettified name to
    /// be displayed, it has only a cosmetic role.
    ///
    /// \return Remote package displayed name.
    ///
    const wstring& name() const {
      return _name;
    }

    /// \brief Get description.
    ///
    /// Returns remote package description if any.
    ///
    /// \return Description string.
    ///
    const wstring& desc() const {
      return _desc;
    }

    /// \brief Get image.
    ///
    /// Returns remote package loaded image object.
    ///
    /// \return Image object.
    ///
    const OmImage& image() const {
      return _image;
    }

    /// \brief Get dependencies count.
    ///
    /// Returns count of remote package dependencies.
    ///
    /// \return Count of remote package dependencies.
    ///
    size_t depCount() const {
      return _depLs.size();
    }

    /// \brief Get dependency identity.
    ///
    /// Returns dependency package identity.
    ///
    /// \param[in]  i       : Index of dependency.
    ///
    /// \return Source dependency package identity.
    ///
    const wstring& depGet(unsigned i) const {
      return _depLs[i];
    }

    /// \brief Check dependency
    ///
    /// Checks whether package has the specified package identity as dependency
    ///
    /// \param[in]  ident    : Dependency package identity to check
    ///
    bool depHas(const wstring& ident) {
      for(size_t i = 0; i < this->_depLs.size(); ++i) {
        if(ident == this->_depLs[i]) return true;
      }
      return false;
    }

    /// \brief Get owner Location.
    ///
    /// Returns Location that own this remote package.
    ///
    /// \return Pointer to Location or nullptr.
    ///
    OmLocation* ownerLoc() const {
      return _location;
    }

    /// \brief Download remote package.
    ///
    /// Starts download process of the remote package represented by this instance.
    ///
    /// \param[in]  path        : Destination folder path to save downloaded file.
    /// \param[in]  supersedes  : Delete (move to trash) superseded packages if download succeed.
    /// \param[in]  download_cb : Optional callback function for download progression.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to callback function.
    ///
    /// \return True if process started correctly, false otherwise.
    ///
    bool download(const wstring& path, bool supersedes = false, Om_downloadCb download_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Check whether downloading.
    ///
    /// Check whether this instance currently runs a download process.
    ///
    /// \return True if currently downloading, false otherwise.
    ///
    bool downloading() const {
      return (_downl_hth != nullptr);
    }

    /// \brief Get download percent.
    ///
    /// Returns completion percentage of the current running download.
    ///
    /// \return Download percent.
    ///
    unsigned downPercent() const {
      return _downl_percent;
    }

    /// \brief Get superseded count.
    ///
    /// Returns count of package this one could supersedes.
    ///
    /// \return Count of remote package dependencies.
    ///
    size_t supCount() const {
      return _supLs.size();
    }

    /// \brief Get superseded package.
    ///
    /// Returns superseded package.
    ///
    /// \param[in]  i       : Index of superseded package.
    ///
    /// \return Source superseded package identity.
    ///
    OmPackage* supGet(unsigned i) const {
      return _supLs[i];
    }

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

    OmLocation*         _location;

    vector<wstring>     _url;

    wstring             _file;

    size_t              _bytes;

    wstring             _checksum;

    unsigned            _state;

    wstring             _ident;

    uint64_t            _hash;

    wstring             _core;

    OmVersion           _version;

    wstring             _name;

    vector<wstring>     _depLs;

    wstring             _desc;

    OmImage             _image;

    vector<OmPackage*>  _supLs;

    wstring             _error;

    FILE*               _downl_file;

    wstring             _downl_path;

    wstring             _downl_temp;

    bool                _downl_spsd;

    Om_downloadCb       _downl_user_download;

    void*               _downl_user_ptr;

    void*               _downl_hth;

    unsigned            _downl_percent;

    static DWORD WINAPI _downl_fth(void*);

    static bool         _downl_download(void* ptr, double tot, double cur, double rate, uint64_t data);
};

#endif // OMREMOTE_H
