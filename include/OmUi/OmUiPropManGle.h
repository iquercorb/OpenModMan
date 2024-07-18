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
#define MAN_PROP_GLE_LINK_CONFIRM         2
#define MAN_PROP_GLE_START_LIST         3
#define MAN_PROP_GLE_SARRT_ORDER        4

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

    void                _starthub_toggle();

    void                _lb_path_on_selchg();

    void                _starthub_list_up();

    void                _starthub_list_dn();

    void                _starthub_delete();

    void                _browse_hub_file();

    void                _onTbInit();

    void                _onTbResize();

    void                _onTbRefresh();

    INT_PTR             _onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPMANGLE_H
