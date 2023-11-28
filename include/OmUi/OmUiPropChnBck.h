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

    void                _onCkBoxBck();

    void                _onBcBrwBck();

    void                _onCkBoxZip();

    void                _onBcDelBck();

    void                _delBck_init();

    void                _delBck_stop();

    void*               _delBck_hth;

    static DWORD WINAPI _delBck_fth(void*);

    static bool         _delBck_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data);

    void                _onTabInit();

    void                _onTabResize();

    void                _onTabRefresh();

    INT_PTR             _onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPCHNBCK_H
