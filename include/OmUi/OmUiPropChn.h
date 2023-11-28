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
#ifndef OMUIPROPCHN_H
#define OMUIPROPCHN_H

#include "OmDialogProp.h"

class OmModChan;

/// \brief Mod Channel Properties parent dialog
///
/// OmDialogProp class derived for Mod Channel Properties parent dialog window.
///
class OmUiPropChn : public OmDialogProp
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropChn(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropChn();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Mod Channel.
    ///
    /// Returns associated Mod Channel object
    ///
    /// \return Associated Mod Channel or nullptr if none.
    ///
    OmModChan* ModChan() const {
      return this->_ModChan;
    }

    /// \brief Set associated Mod Channel.
    ///
    /// Define the associated Mod Channel, which the dialog is intended to
    /// work with and on. A valid Mod Channel must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  ModChan  : Mod Channel object to associate.
    ///
    void setModChan(OmModChan* ModChan) {
      this->_ModChan = ModChan;
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

    OmModChan*          _ModChan;

    INT_PTR             _onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPCHN_H
