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
#ifndef OMUIPROPHUBCHN_H
#define OMUIPROPHUBCHN_H

#include "OmDialogPropTab.h"

#define HUB_PROP_CHN_ORDER   0

/// \brief Mod Hub Properties / Mod Channel tab child
///
/// OmDialog class derived for Mod Hub Properties / Mod Channel tab child dialog window
///
class OmUiPropHubChn : public OmDialogPropTab
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropHubChn(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropHubChn();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void*               _chn_delete_hth;

    void*               _chn_delete_hwo;

    int32_t             _chn_delete_chn;

    static DWORD WINAPI _chn_delete_run_fn(void*);

    static  VOID WINAPI _chn_delete_end_fn(void*,uint8_t);

    static bool         _chn_delete_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t param);


    void                _lb_chn_on_selchg();

    void                _bc_up_pressed();

    void                _bc_dn_pressed();

    void                _bc_del_pressed();

    void                _bd_edi_pressed();

    void                _bc_add_pressed();

    void                _onTabInit();

    void                _onTabResize();

    void                _onTabRefresh();

    INT_PTR             _onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPHUBCHN_H
