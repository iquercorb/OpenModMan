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
#ifndef OMVERSION_H
#define OMVERSION_H

#include "OmBase.h"

/// \brief Version object.
///
/// This class provide object to manipulate a version number.
///
class OmVersion
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmVersion();

    /// \brief Constructor.
    ///
    /// Initialization constructor.
    ///
    /// \param[in]  str     : Version string to parse.
    ///
    OmVersion(const OmWString& str);

    /// \brief Constructor.
    ///
    /// Copy constructor.
    ///
    /// \param[in]  other   : Other instance to copy.
    ///
    OmVersion(const OmVersion& other);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmVersion();

    /// \brief Assign operator.
    ///
    /// Copy from another instance.
    ///
    /// \param[in]  other   : Other instance to copy.
    ///
    /// \return Reference to this.
    ///
    OmVersion& operator=(const OmVersion& other);

    /// \brief Assign operator.
    ///
    /// Assign from version string.
    ///
    /// \param[in]  str     : Version string to parse.
    ///
    /// \return Reference to this.
    ///
    OmVersion& operator=(const OmWString& str);

    /// \brief Parse version string.
    ///
    /// Parses the supplied string to get version data.
    ///
    /// \param[in]  str     : Version string to parse.
    ///
    /// \return True if parsing succeed, false otherwise.
    ///
    bool parse(const OmWString& str);

    /// \brief Define version.
    ///
    /// Defines this instance according the given values.
    ///
    /// \param[in]  maj     : Major number to set.
    /// \param[in]  min     : Minor number to set.
    /// \param[in]  rev     : Revision number to set.
    ///
    void define(unsigned maj, unsigned min, unsigned rev);

    /// \brief Check validity
    ///
    /// Check whether this instance has valid parsed value.
    ///
    /// \return True if version is valid, false otherwise
    ///
    bool valid() const {
      return !this->_str.empty();
    }

    /// \brief Major number.
    ///
    /// Returns version major number.
    ///
    /// \return Version major number.
    ///
    unsigned major() const {
      return _maj;
    }

    /// \brief Minor number.
    ///
    /// Returns version minor number.
    ///
    /// \return Version minor number.
    ///
    unsigned minor() const {
      return _min;
    }

    /// \brief Revision number.
    ///
    /// Returns version revision number.
    ///
    /// \return Version revision number.
    ///
    unsigned revis() const {
      return _rev;
    }

    /// \brief Get as string.
    ///
    /// Returns string representation of this instance.
    ///
    /// \return Version string.
    ///
    OmWString asString() const;

    /// \brief Array subscript.
    ///
    /// Returns either major, minor or revision number according given index.
    ///
    /// \param[in]  i       : Version slot index.
    ///
    /// \return Version number.
    ///
    unsigned operator[](unsigned i) const {
      switch(i) {
        case 0: return _maj;
        case 1: return _min;
        default: return _rev;
      }
    }

    /// \brief Equal operator.
    ///
    /// Equality comparison between two instances.
    ///
    /// \param[in]  other   : Other instance to compare.
    ///
    /// \return True if other is equal to this instance, false otherwise.
    ///
    bool operator==(const OmVersion& other) const {
      return (_maj == other._maj && _min == other._min && _rev == other._rev);
    }

    /// \brief Not equal operator.
    ///
    /// Non-equality comparison between two instances.
    ///
    /// \param[in]  other   : Other instance to compare.
    ///
    /// \return True if other is not equal to this instance, false otherwise.
    ///
    bool operator!=(const OmVersion& other) const {
      return (_maj != other._maj || _min != other._min || _rev != other._rev);
    }

    /// \brief Less operator.
    ///
    /// Less comparison between two instances.
    ///
    /// \param[in]  other   : Other instance to compare.
    ///
    /// \return True if this instance is less than the other, false otherwise.
    ///
    bool operator<(const OmVersion& other) const;

    /// \brief Greater operator.
    ///
    /// Greater comparison between two instances.
    ///
    /// \param[in]  other   : Other instance to compare.
    ///
    /// \return True if this instance is greater than the other, false otherwise.
    ///
    bool operator>(const OmVersion& other) const;

    /// \brief Less or equal operator.
    ///
    /// Less or equality comparison between two instances.
    ///
    /// \param[in]  other   : Other instance to compare.
    ///
    /// \return True if this instance is less or equal to the other, false otherwise.
    ///
    bool operator<=(const OmVersion& other) const;

    /// \brief Greater or equal operator.
    ///
    /// Greater or equality comparison between two instances.
    ///
    /// \param[in]  other   : Other instance to compare.
    ///
    /// \return True if this instance is greater or equal to the other, false otherwise.
    ///
    bool operator>=(const OmVersion& other) const;

    /// \brief Check whether is null.
    ///
    /// Checks whether this instance is null.
    ///
    /// \return True if all version components are zero, false otherwise.
    ///
    bool isNull() const {
      return (_maj == 0 && _min == 0 && _rev == 0);
    }

    /// \brief Clear instance
    ///
    /// Reset instance to its initial values
    ///
    void clear();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmWStringArray     _str;   //< version as string

    unsigned            _maj;   //< version major number

    unsigned            _min;   //< version minor number

    unsigned            _rev;   //< version revision number
};

#endif // OMVERSION_H
