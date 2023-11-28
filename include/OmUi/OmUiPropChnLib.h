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
#ifndef OMUIPROPCHNLIB_H
#define OMUIPROPCHNLIB_H

#include "OmDialogPropTab.h"

#define CHN_PROP_LIB_CUSTDIR     0
#define CHN_PROP_LIB_DEVMODE     1
#define CHN_PROP_LIB_WARNINGS    2
#define CHN_PROP_LIB_SHOWHIDDEN  3

/// \brief Mod Channel Properties: "Mods Library" tab
///
/// OmDialogPropTab class derived for "Mods Library" tab child dialog window
///
class OmUiPropChnLib : public OmDialogPropTab
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropChnLib(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropChnLib();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _onCkBoxLib();

    void                _onBcBrwLib();

    void                _onCkBoxDev();

    void                _onCkBoxWrn();

    void                _onCkBoxHid();

    void                _onTabInit();

    void                _onTabResize();

    void                _onTabRefresh();

    INT_PTR             _onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPCHNLIB_H
