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
#ifndef OMUIPROPNET_H
#define OMUIPROPNET_H

#include "OmDialogProp.h"

class OmNetPack;

/// \brief Remote package Details dialog
///
/// OmDialogProp class derived for Remote package Details dialog window
///
class OmUiPropNet : public OmDialogProp
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropNet(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropNet();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Network Mod.
    ///
    /// Returns associated Network Mod object previously defined
    ///
    /// \return Pointer to Net Pack or nullptr if none.
    ///
    OmNetPack* NetPack() const {
      return this->_NetPack;
    }
    /// \brief Set associated Network Mod.
    ///
    /// Define the associated Network Mod, which the dialog is intended to
    /// work with and on. A valid Network Mod must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  NetPack  : Net Pack object to associate.
    ///
    void setNetPack(OmNetPack* NetPack) {
      this->_NetPack = NetPack;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmNetPack*          _NetPack;
};

#endif // OMUIPROPNET_H
