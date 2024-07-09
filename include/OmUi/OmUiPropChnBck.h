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
#ifndef OMUIPROPCHNBCK_H
#define OMUIPROPCHNBCK_H

#include "OmDialogPropTab.h"

#define CHN_PROP_BCK_CUSTDIR      0
#define CHN_PROP_BCK_COMP_LEVEL   1
#define CHN_PROP_BCK_NO_OVERLAP   2

/// \brief Mod Channel Properties: "Data Backup" tab
///
/// OmDialogPropTab class derived for "Data Backup" tab child dialog window
///
class OmUiPropChnBck : public OmDialogPropTab
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropChnBck(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropChnBck();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _cust_backup_toggle();

    void                _browse_dir_backup();

    void                _comp_backup_toggle();

    void                _onTbInit();

    void                _onTbResize();

    void                _onTbRefresh();

    INT_PTR             _onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPCHNBCK_H
