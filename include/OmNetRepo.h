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
#ifndef OMNETREPO_H
#define OMNETREPO_H

#include "OmBase.h"

#include "OmXmlConf.h"
#include "OmConnect.h"

class OmModChan;
class OmModPack;
class OmImage;

/// \brief Network Mod repository object
///
/// The Repository object hold parameters and data related to
/// distant server that provide Mods for download
///
class OmNetRepo
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in] ModChan  : Pointer to related Mod Channel
    ///
    OmNetRepo(OmModChan* ModChan);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmNetRepo();

    /// \brief Clear instance
    ///
    /// Reset all instance data and parameters to initial state
    ///
    void clear();

    /// \brief Initialize new repository
    ///
    /// Initializes new repository definition basis using the given parameters, this
    /// operation erase any previously parsed data.
    ///
    /// \param[in] title    : Repository title (description)
    ///
    void init(const OmWString& title);

    /// \brief Set base parameters.
    ///
    /// Set repository network coordinates for network query
    ///
    /// \param[in]  base    : Repository HTTP base address.
    /// \param[in]  name    : Repository HTTP suffix name.
    ///
    /// \return True if parameters makes a valid HTTP URL, false otherwise.
    ///
    bool setCoordinates(const OmWString& base, const OmWString& name);

    /// \brief Set temporary title
    ///
    /// Set temporary repository title (short description), the title
    /// is set in local variable but not written into definition and will
    /// be replace as soon as new definition is loaded.
    ///
    /// \param[in]  title   : Temporary title to set.
    ///
    void setTempTitle(const OmWString& title) {
      this->_title = title;
    }

    /// \brief Parse definition
    ///
    /// Parse given XML data as repository definition to set data of this instance.
    ///
    /// \param[in] data     : XML data to parse.
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool parse(const OmWString& data);

    /// \brief Load repository definition
    ///
    /// Load repository definition from local file system.
    ///
    /// \param[in] path     : Path to XML repository definition to load.
    ///
    /// \return Operation result code.
    ///
    OmResult load(const OmWString& path);

    /// \brief Save repository definition
    ///
    /// Save repository definition to local file system.
    ///
    /// \param[in] path     : Path to file to save XML definition
    ///
    /// \return Operation result code.
    ///
    OmResult save(const OmWString& path);

    /// \brief Query repository.
    ///
    /// Try connect to repository to get definition file repository data. This
    /// function does not use thread and block until request response or timeout.
    ///
    /// \return True if query succeed, false if an error occurred.
    ///
    OmResult query();

    /// \brief Abort query
    ///
    /// Abort the current pending query if any
    ///
    void abortQuery();

    /// \brief Query result
    ///
    /// Returns last query result code
    ///
    OmResult queryResult() const {
      return this->_query_result;
    }

    /// \brief Query HTTP response code
    ///
    /// Returns last query HTTP response code
    ///
    uint32_t queryResponseCode() const {
      return this->_query_respcode;
    }

    /// \brief Query HTTP response data
    ///
    /// Returns last query HTTP response raw data
    ///
    /// \return UTF-16 converted response data
    ///
    const OmWString& queryResponseData() const {
      return this->_query_respdata;
    }

    /// \brief Query last error message
    ///
    /// Returns error message specific to last query
    ///
    /// \return String describing last error
    ///
    const OmWString& queryLastError() const {
      return this->_query_lasterr;
    }

    /// \brief Get URL base address
    ///
    /// Returns repository HTTP base address.
    ///
    /// \return HTTP base address.
    ///
    const OmWString& base() const {
      return this->_base;
    }

    /// \brief Get URL suffix name
    ///
    /// Returns repository suffix name (definition name).
    ///
    /// \return Suffix name.
    ///
    const OmWString& name() const {
      return this->_name;
    }

    /// \brief Get access path or URL
    ///
    /// Return repository XML access URL or local file Path depending context.
    ///
    /// \return XML file local path or internet URL.
    ///
    const OmWString& path() const {
      return this->_path;
    }

    /// \brief Get title.
    ///
    /// Returns repository indicative title.
    ///
    /// \return Indicative title.
    ///
    const OmWString& uuid() const {
      return this->_uuid;
    }

    /// \brief Get title.
    ///
    /// Returns repository indicative title (description).
    ///
    /// \return Wide string.
    ///
    const OmWString& title() const {
      return this->_title;
    }

    /// \brief Set title.
    ///
    /// Set repository indicative title (description).
    ///
    /// \param[in] title  : Title to set.
    ///
    void setTitle(const OmWString& title);

    /// \brief Get download path.
    ///
    /// Returns repository common download path, this is the
    /// path added to base address where to find files to be
    /// downloaded.
    ///
    /// \return Common download path.
    ///
    const OmWString& downpath() const {
      return this->_downpath;
    }

    /// \brief Set download path.
    ///
    /// Set repository common download path, this is the
    /// path added to base address where to find files to be
    /// downloaded.
    ///
    /// \param[in] downpath  : Default download path to set.
    ///
    void setDownpath(const OmWString& downpath);

    /// \brief Get Mod reference count.
    ///
    /// Returns count of Mod this repository references.
    ///
    /// \return Mods count.
    ///
    size_t referenceCount() const {
      return this->_reference_list.size();
    }

    /// \brief Get Mod reference.
    ///
    /// Returns repository Mod reference as XML node.
    ///
    /// \param[in] index  : Index of reference to get
    ///
    /// \return XML node.
    ///
    const OmXmlNode& getReference(size_t index) const {
      return this->_reference_list[index];
    }

    /// \brief Check for reference.
    ///
    /// Checks whether this instance has Mod identity as reference
    ///
    /// \param[in] iden   : Mod identity to search
    ///
    /// \return True if reference was found, false otherwise
    ///
    bool hasReference(const OmWString& iden) const;

    /// \brief Get reference index
    ///
    /// Search for referenced Mod identity and returns its current
    /// index in list.
    ///
    /// \param[in] iden   : Mod identity to search
    ///
    /// \return Reference index or -1 if not found.
    ///
    int32_t indexOfReference(const OmWString& iden) const;

    /// \brief Find dependency reference index
    ///
    /// Search for referenced Mod identity that match the specified dependency string
    /// and returns its current index in list.
    ///
    /// \param[in] depend : Dependency string to parse and match
    ///
    /// \return Reference index or -1 if not found.
    ///
    int32_t findDependReference(const OmWString& depend) const;

    /// \brief Delete reference
    ///
    /// Search for referenced Mod identity and delete it.
    ///
    /// \param[in] iden   : Mod identity to search
    ///
    /// \return True if reference found and deleted, false otherwise
    ///
    bool deleteReference(const OmWString& iden);

    /// \brief Delete reference
    ///
    /// Search for referenced Mod identity and delete it.
    ///
    /// \param[in] iden   : Index of reference to delete
    ///
    void deleteReference(size_t index);

    /// \brief Add Mod reference
    ///
    /// Add new Mod reference to this instance
    ///
    /// \param[in] ModPack : Pointer to Mod Pack to add reference from
    ///
    /// \return If reference with same identity was found, the
    ///         index of updated reference, otherwise -1 is returned
    ///
    int32_t addReference(const OmModPack* ModPack);

    /// \brief Set Mod reference custom URL
    ///
    /// Set or replace Custom URL of the Mod reference at specified index
    ///
    /// \param[in] index  : Index of Mod reference to modify
    /// \param[in] url    : Custom URL to set
    ///
    void setReferenceUrl(size_t index, const OmWString& url);

    /// \brief Get Mod reference custom URL
    ///
    /// Get the custom URL of the Mod reference at specified index
    ///
    /// \param[in] index  : Index of Mod reference
    ///
    /// \return Pointer to null-terminated wide string, or nullptr.
    ///
    OmWString getReferenceUrl(size_t index) const;

    /// \brief Check whether reference has custom URL
    ///
    /// Returns whether the reference at specified index has custom URL configured
    ///
    /// \param[in] index  : Index of Mod reference
    ///
    /// \return True if reference has custom URL, false otherwise
    ///
    bool referenceHasUrl(size_t index) const;

    /// \brief Set Mod reference thumbnail
    ///
    /// Set or replace thumbnail image of the Mod reference at specified index
    ///
    /// \param[in] index  : Index of Mod reference to modify
    /// \param[in] image  : Thumbnail image to set
    /// \param[in] level  : Jpeg compression level (0-100)
    ///
    void setReferenceThumbnail(size_t index, const OmImage& image, uint8_t level = 70);

    /// \brief Get Mod reference thumbnail
    ///
    /// Get the thumbnail image of the Mod reference at specified index
    ///
    /// \param[in] index  : Index of Mod reference
    ///
    /// \return thumbnail image object
    ///
    OmImage getReferenceThumbnail(size_t index) const;

    /// \brief Check whether reference has thumbnail
    ///
    /// Returns whether the reference at specified index has thumbnail image
    ///
    /// \param[in] index  : Index of Mod reference
    ///
    /// \return True if reference has thumbnail, false otherwise
    ///
    bool referenceHasThumbnail(size_t index) const;

    /// \brief Set Mod reference description
    ///
    /// Set or replace description text of the Mod reference at specified index
    ///
    /// \param[in] index  : Index of Mod reference to modify
    /// \param[in] text   : Thumbnail image to set
    /// \param[in] level  : Deflate compression level
    ///
    void setReferenceDescription(size_t index, const OmWString& text, uint8_t level = 6);

    /// \brief Get Mod reference description
    ///
    /// Get the description text of the Mod reference at specified index
    ///
    /// \param[in] index  : Index of Mod reference
    ///
    /// \return description text
    ///
    OmWString getReferenceDescription(size_t index) const;

    /// \brief Check whether reference has description
    ///
    /// Returns whether the reference at specified index has description text
    ///
    /// \param[in] index  : Index of Mod reference
    ///
    /// \return True if reference has description, false otherwise
    ///
    bool referenceHasDescription(size_t index) const;

    /// \brief Get Mod reference dependencies
    ///
    /// Get the dependencies list of the Mod reference at specified index
    ///
    /// \param[in] index  : Index of Mod reference
    ///
    /// \return description text
    ///
    void getReferenceDepends(size_t index, OmWStringArray* depends) const;

    /// \brief Get Mod reference dependencies count
    ///
    /// Get the count of dependencies of the Mod reference at specified index
    ///
    /// \param[in] index  : Index of Mod reference
    ///
    /// \return Dependencies count
    ///
    size_t getReferenceDependCount(size_t index) const;

    /// \brief Get Mod Channel.
    ///
    /// Returns Mod Channel this instance is related to.
    ///
    /// \return Pointer to Mod Channel object
    ///
    OmModChan* ModChan() const {
      return this->_ModChan;
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

  protected:

  private:

    // linking
    OmModChan*          _ModChan;

    // Coordinates
    OmWString           _base;

    OmWString           _name;

    // XML definition
    OmWString           _path;

    OmXmlConf           _xml;

    // general parameters
    OmWString           _uuid;

    OmWString           _title;

    OmWString           _downpath;

    // referenced mods
    OmXmlNodeArray      _reference_list;

    // query stuff
    OmConnect           _query_connect;

    OmResult            _query_result;

    uint32_t            _query_respcode;

    OmWString           _query_respdata;

    OmWString           _query_lasterr;

    // reference build helpers
    bool                _save_thumbnail(OmXmlNode&, const OmImage&, uint8_t level = 70);

    bool                _save_description(OmXmlNode&, const OmWString&, uint8_t level = 6);

    // logs and errors
    void                _log(unsigned level, const OmWString& origin, const OmWString& detail) const;

    void                _error(const OmWString& origin, const OmWString& detail);

    OmWString           _lasterr;
};

/// \brief OmNetRepo pointer array
///
/// Typedef for an STL vector of OmNetRepo pointer type
///
typedef std::vector<OmNetRepo*> OmPNetRepoArray;


/// \brief OmNetRepo pointer array
///
/// Typedef for an STL deque of OmNetRepo pointer type
///
typedef std::deque<OmNetRepo*> OmPNetRepoQueue;

#endif // OMNETREPO_H
