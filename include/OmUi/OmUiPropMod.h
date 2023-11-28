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
#ifndef OMUIPROPMOD_H
#define OMUIPROPMOD_H

#include "OmDialogProp.h"

class OmModPack;

/// \brief Package Details dialog
///
/// OmDialogProp class derived for Package Details dialog window
///
class OmUiPropMod : public OmDialogProp
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropMod(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropMod();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Package.
    ///
    /// Returns associated Package object previously defined
    /// via OmUiPropChn.setPackage
    ///
    /// \return Associated Package or nullptr if none.
    ///
    OmModPack* getModPack() const {
      return _ModPack;
    }

    /// \brief Set associated Package.
    ///
    /// Define the associated Package, which the dialog is intended to
    /// work with and on. A valid Package must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  pPkg  : Package object to associate.
    ///
    void setModPack(OmModPack* ModPack) {
      _ModPack = ModPack;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmModPack*          _ModPack;
};

#endif // OMUIPROPMOD_H
