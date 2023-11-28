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

    /// \brief Set base parameters.
    ///
    /// Defines repository address parameter to query data.
    ///
    /// \param[in]  base    : Repository HTTP base address.
    /// \param[in]  name    : Repository HTTP suffix name.
    ///
    /// \return True if parameters makes a valid HTTP URL, false otherwise.
    ///
    bool setup(const OmWString& base, const OmWString& name);

    /// \brief Set base parameters.
    ///
    /// Defines repository address parameter to query data.
    ///
    /// \param[in]  base    : Repository HTTP base address.
    /// \param[in]  name    : Repository HTTP suffix name.
    /// \param[in]  title   : Repository title
    ///
    /// \return True if parameters makes a valid HTTP URL, false otherwise.
    ///
    bool setup(const OmWString& base, const OmWString& name, const OmWString& title);

    /// \brief Get URL base address
    ///
    /// Returns repository HTTP base address.
    ///
    /// \return HTTP base address.
    ///
    const OmWString& urlBase() const {
      return this->_url_base;
    }

    /// \brief Get URL suffix name
    ///
    /// Returns repository suffix name (definition name).
    ///
    /// \return Suffix name.
    ///
    const OmWString& urlName() const {
      return this->_url_name;
    }

    /// \brief Get full URL
    ///
    /// Returns real XML file URL as combination of base and name.
    ///
    /// \return Repo definition file URL.
    ///
    const OmWString& urlFull() const {
      return this->_url_full;
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

    /// \brief Query repository.
    ///
    /// Try connect to repository to get definition file repository data. This
    /// function does not use thread and block until request response or timeout.
    ///
    /// \return True if query succeed, false if an error occured.
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
      return this->_qry_result;
    }

    /// \brief Get Mod reference count.
    ///
    /// Returns count of Mod this repository references.
    ///
    /// \return Mods count.
    ///
    size_t referenceCount() const {
      return this->_reference.size();
    }

    /// \brief Get Mod reference.
    ///
    /// Returns repository Mod reference as XML node.
    ///
    /// \return XML node.
    ///
    const OmXmlNode& getReference(size_t i) const {
      return this->_reference[i];
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

    /// \brief Get reference index.
    ///
    /// Search for referenced Mod identity and returns its current
    /// index in list.
    ///
    /// \param[in] iden   : Mod identity to search
    ///
    /// \return Reference index or -1 if not found.
    ///
    int32_t referenceIndex(const OmWString& iden) const;

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

    // management
    void                _log(unsigned level, const OmWString& origin, const OmWString& detail) const;

    void                _error(const OmWString& origin, const OmWString& detail);

    OmWString           _lasterr;

    // linking
    OmModChan*          _ModChan;

    // internal data
    OmXmlConf           _xmlconf;

    // general parameters

    OmWString           _uuid;

    OmWString           _title;

    OmWString           _downpath;

    OmWString           _url_base;

    OmWString           _url_name;

    OmWString           _url_full;

    // referenced mods
    OmXmlNodeArray      _reference;

    // query stuff
    OmConnect           _connect;

    OmResult            _qry_result;
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
