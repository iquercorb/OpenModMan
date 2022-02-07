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

#ifndef OMCONFIG_H
#define OMCONFIG_H

#include "OmBase.h"
#include "OmXmlDoc.h"

/// \brief Configuration file interface.
///
/// Object to provide interface for a generic XML configuration file.
///
class OmConfig
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor
    ///
    /// Default object constructor
    ///
    OmConfig();

    /// \brief Destructor
    ///
    /// Default object destructor
    ///
    ~OmConfig();


    /// \brief Get file path.
    ///
    /// Returns the path of the configuration file this instance represents.
    ///
    /// \return File path.
    ///
    const wstring& path() const {
      return _file;
    }

    /// \brief Get configuration XML root.
    ///
    /// If the configuration is validated, this method returns an OmXmlNode
    /// object corresponding to the configuration XML root, which is actually
    /// the signature node found using the OmConfig.validate method.
    ///
    /// \see OmXmlNode
    /// \see OmConfig.validate
    ///
    /// \return Configuration XML root node.
    ///
    OmXmlNode& xml() {
      return _cfgRoot;
    }

    /// \brief Validity check
    ///
    /// Checks whether this instance was previously validated with the desired
    /// signature and is safe for reading, edition and writing.
    ///
    /// \see OmConfig.validate
    ///
    /// \return True if configuration file is valid, false otherwise.
    ///
    bool valid() const;

    /// \brief Initialize a new configuration data.
    ///
    /// Creates a new empty configuration data with the desired
    /// signature.
    ///
    /// \param[in]  sign    : Desired signature for configuration data.
    ///
    void init(const wstring& sign);

    /// \brief Initialize a new configuration file.
    ///
    /// Creates a new empty configuration file with the desired
    /// signature at the given path.
    ///
    /// \param[in]  path    : Path to file to create and initialize.
    /// \param[in]  sign    : Desired signature for configuration data.
    ///
    /// \return True if write operation succeed, false otherwise.
    ///
    bool init(const wstring& path, const wstring& sign);

    /// \brief Open an existing configuration file.
    ///
    /// Try to open XML configuration file with the specified signature. If
    /// expected signature is not the same, the function fail.
    ///
    /// \param[in]  path    : Path to file to be parsed.
    /// \param[in]  sign    : Expected signature to validate.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool open(const wstring& path, const wstring& sign);

    /// \brief Parse configuration data.
    ///
    /// Try to parse XML configuration data with the specified signature. If
    /// expected signature is not the same, the function fail.
    ///
    /// \param[in]  xml     : XML content to parse.
    /// \param[in]  sign    : Expected signature to validate.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool parse(const wstring& xml, const wstring& sign);

    /// \brief Save configuration to file.
    ///
    /// Writes the current XML data state to the file specified at
    /// initialization.
    ///
    /// If the instance was initialized without file path this method will
    /// fail.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool save();

    /// \brief Save current configuration.
    ///
    /// Writes the current XML data state to the specified file. If the file
    /// already exists, overwrite it.
    ///
    /// If the instance was initialized with a specified file path, this method
    /// replace the related configuration file.
    ///
    /// \param[in]  path    : Path to file to write.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool save(const wstring& path);

    /// \brief Get configuration data.
    ///
    /// Returns the current XML document data content.
    ///
    /// \return Current XML document data.
    ///
    string data();

    /// \brief Close configuration.
    ///
    /// Resets the current configuration file handling and set the instance
    /// ready for a new configuration file.
    ///
    /// \param[in]  save    : If true, and if the current configuration is
    ///                       validated, saves current state to file before
    ///                       closing
    ///
    void close(bool save = false);

    /// \brief Get XML last error string.
    ///
    /// Returns the string corresponding to the last XML parsing error code.
    ///
    /// \return XML parsing error as string.
    ///
    wstring lastErrorStr() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    int                 _status;      //< Config status

    wstring             _file;        //< handled XML file path

    OmXmlDoc            _xmlDoc;      //< XML document instance

    OmXmlNode           _cfgRoot;     //< Configuration root node
};
#endif // OMCONFIG_H
