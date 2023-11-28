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
#ifndef OMUIPROPHUB_H
#define OMUIPROPHUB_H

#include "OmDialogProp.h"

class OmModHub;

/// \brief Mod Hub Properties parent dialog
///
/// OmDialogProp class derived for Mod Hub Properties parent dialog window.
///
class OmUiPropHub : public OmDialogProp
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropHub(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropHub();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Mod Hub.
    ///
    /// Returns associated Mod Hub object
    ///
    /// \return Associated Mod Hub or nullptr if none.
    ///
    OmModHub* modHub() const {
      return _modHub;
    }

    /// \brief Set associated Mod Hub.
    ///
    /// Define the associated Mod Hub, which the dialog is intended to
    /// work with and on. A valid Mod Hub must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  pModHub  : Mod Hub object to associate.
    ///
    void setModHub(OmModHub* pModHub) {
      _modHub = pModHub;
    }

    /// \brief Check for properties changes
    ///
    /// Checks whether dialog's dedicated properties changed then perform
    /// proper operations, such as enabling or disabling the Apply button.
    ///
    bool checkChanges();

    /// \brief Validate properties changes
    ///
    /// Checks whether dialog's changed properties are valid and suitable
    /// then alert user or perform required action in context.
    ///
    bool validChanges();

    /// \brief Apply properties changes
    ///
    /// Retrieve dialog's dedicated properties then apply changes.
    ///
    bool applyChanges();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmModHub*           _modHub;
};

#endif // OMUIPROPHUB_H
