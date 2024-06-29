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
#ifndef OMUIMGRFOOT_H
#define OMUIMGRFOOT_H

#include "OmDialog.h"

class OmUiMan;
class OmModPack;
class OmNetPack;

/// \brief Window Secondary (bottom) Frame.
///
/// OmDialog class derived for the main window bottom frame which contain
/// package snapshot and description view.
///
class OmUiManFoot : public OmDialog
{
  public: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiManFoot(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiManFoot();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Set selected Mod
    ///
    /// Defines Mod Pack as selected Mod
    ///
    /// \param[in]  ModPack    : Pointer to Mod Pack object.
    ///
    void selectItem(OmModPack* ModPack);

    /// \brief Set selected item
    ///
    /// Defines Network Mod Pack as selected item
    ///
    /// \param[in]  NetPack    : Pointer to Network Mod Pack.
    ///
    void selectItem(OmNetPack* NetPack);

    /// \brief Clear selected item
    ///
    /// Clears current selected item for all child tabs
    ///
    void clearItem();

    /// \brief Selected Mod Pack
    ///
    /// Returns currently selected Mod Pack pointer if any
    ///
    /// \return Currently selected Mod Pack pointer or null pointer if none
    ///
    OmModPack* ModPack() const {
      return this->_ModPack;
    }

    /// \brief Selected Network Mod Pack
    ///
    /// Returns currently selected Network Mod Pack pointer if any
    ///
    /// \return Currently selected Network Mod Pack pointer or null pointer if none
    ///
    OmNetPack* NetPack() const {
      return this->_NetPack;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiMan*            _UiMan;

    // child tabs
    OmWStringArray     _tab_name;

    std::vector<OmDialog*>   _tab_dialog;

    void                _tab_add_dialog(const OmWString& name, OmDialog* dialog);

    OmDialog*           _tab_get_dialog(int32_t id);

    // Item references
    OmModPack*          _ModPack;

    OmNetPack*          _NetPack;

    // Dialog common functions
    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMGRFOOT_H
