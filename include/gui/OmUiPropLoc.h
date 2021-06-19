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
#ifndef OMUIPROPLOC_H
#define OMUIPROPLOC_H

#include "OmDialogProp.h"

class OmLocation;

/// \brief Location Properties parent dialog
///
/// OmDialogProp class derived for Location Properties parent dialog window.
///
class OmUiPropLoc : public OmDialogProp
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropLoc(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropLoc();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Location.
    ///
    /// Returns associated Location object previously defined
    /// via OmUiPropLoc.setLocation
    ///
    /// \return Associated Location or nullptr if none.
    ///
    OmLocation* locCur() const {
      return _pLoc;
    }

    /// \brief Set associated Location.
    ///
    /// Define the associated Location, which the dialog is intended to
    /// work with and on. A valid Location must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  pLoc  : Location object to associate.
    ///
    void locSet(OmLocation* pLoc) {
      _pLoc = pLoc;
    }

    /// \brief Check for properties changes
    ///
    /// Checks whether dialog's dedicated properties changed then perform
    /// proper operations, such as enabling or disabling the Apply button.
    ///
    bool checkChanges();

    /// \brief Apply properties changes
    ///
    /// Retrieve dialog's dedicated properties then apply changes.
    ///
    bool applyChanges();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmLocation*         _pLoc;

    void*               _movBck_hth;

    void                _movBck_init();

    void                _movBck_stop();

    static DWORD WINAPI _movBck_fth(void*);

    bool                _onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPLOC_H
