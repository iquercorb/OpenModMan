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

#ifndef OMUIMAINLIBRARY_H
#define OMUIMAINLIBRARY_H

#include "OmDialog.h"

/// \brief Main window Library Tab
///
/// OmDialog class derived for Main Window Library Tab_Control child
/// dialog window
///
class OmUiMainLib : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiMainLib(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiMainLib();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Select Location
    ///
    /// Select or unselect Location then refresh dialog.
    ///
    /// \param[in]  i  Index of Location to select or -1 to select none.
    ///
    void selLocation(int i);

    /// \brief Install selected packages.
    ///
    /// Public function to launch install process for selected packages if
    /// any.
    ///
    void install();

    /// \brief Uninstall selected packages.
    ///
    /// Public function to launch unsintall process for selected packages if
    /// any.
    ///
    void uninstall();

    /// \brief Toggle selected packages installation.
    ///
    /// Public function to toggle installation (either install or uninstall) of
    /// the selected packages if any.
    ///
    void toggle();

    /// \brief View package details.
    ///
    /// Public function to open selected package property dialog.
    ///
    void viewDetails();

    /// \brief Move package to trash.
    ///
    /// Public function to move selected package to trash.
    ///
    void moveTrash();

    /// \brief Open package in explorer.
    ///
    /// Public function to open package in file explorer.
    ///
    void openExplore();

    /// \brief Launch selected batch.
    ///
    /// Public function to launch the selected batch if any.
    ///
    void batch();

    /// \brief Delete selected batch.
    ///
    /// Public function to delete the selected batch if any.
    ///
    bool remBatch();

    /// \brief Modify selected batch.
    ///
    /// Public function to modify the selected batch if any.
    ///
    bool ediBatch();

    /// \brief Set dialog on-process state.
    ///
    /// Enable or disable the dialog on process state. The on-process state
    /// disable all dialog controls except the Abort button to avoid bogus user
    /// interactions.
    ///
    /// \param[in]  enable  Enable or disable on-process state.
    ///
    void setOnProcess(bool enable);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    bool                _onProcess;

    unsigned            _lvIconsSize;

    void                _onSelectPkg();

    void                _onSelectBat();

    HFONT               _hFtTitle;

    HFONT               _hFtMonos;

    HBITMAP             _hBmBlank;

    HBITMAP             _hBmBcNew;

    HBITMAP             _hBmBcDel;

    HBITMAP             _hBmBcMod;

    void                _reloadLibEc();

    void                _reloadLibLv(bool clear = false);

    void                _reloadBatLb();

    void                _reloadLocCb();

    void                _reloadIcons();

    void                _showPkgPopup();

    bool                _abortPending;

    void*               _install_hth;

    static DWORD WINAPI _install_fth(void*);

    void*               _uninstall_hth;

    static DWORD WINAPI _uninstall_fth(void*);

    void*               _batch_hth;

    static DWORD WINAPI _batch_fth(void*);

    void*               _monitor_hth;

    static DWORD WINAPI _monitor_fth(void*);

    void*               _monitor_hev[3];

    void                _monitor_init(const wstring& path);

    void                _monitor_stop();

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMAINLIBRARY_H
