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
#ifndef OMUIPROPHUBSETG_H
#define OMUIPROPHUBSETG_H

#include "OmDialogPropTab.h"

#define HUB_PROP_STG_TITLE   0
#define HUB_PROP_STG_ICON    1

/// \brief Mod Hub Properties / Settings tab child
///
/// OmDialog class derived for Mod Hub Properties / Settings tab child dialog window
///
class OmUiPropHubStg : public OmDialogPropTab
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropHubStg(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropHubStg();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _onBcBrwIcon();

    void                _onBcDelIcon();

    void                _onTabInit();

    void                _onTabResize();

    void                _onTabRefresh();

    INT_PTR             _onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPHUBSETG_H
