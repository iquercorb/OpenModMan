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
#ifndef OMUIPROPMANGLE_H
#define OMUIPROPMANGLE_H

#include "OmDialogPropTab.h"

#define MAN_PROP_GLE_ICON_SIZE          0
#define MAN_PROP_GLE_NO_MDPARSE         1
#define MAN_PROP_GLE_START_LIST         2
#define MAN_PROP_GLE_SARRT_ORDER        3

/// \brief Manager Options / General tab child
///
/// OmDialog class derived for Manager Options / General tab child dialog window
///
class OmUiPropManGle : public OmDialogPropTab
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropManGle(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropManGle();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _onCkBoxRaw();

    void                _onCkBoxStr();

    void                _onLbStrlsSel();

    void                _onBcUpStr();

    void                _onBcDnStr();

    void                _onBcBrwStr();

    void                _onBcRemStr();

    void                _onTabInit();

    void                _onTabResize();

    void                _onTabRefresh();

    INT_PTR             _onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPMANGLE_H
