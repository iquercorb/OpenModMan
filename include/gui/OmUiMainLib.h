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
#ifndef OMUIMAINLIB_H
#define OMUIMAINLIB_H

#include "OmDialog.h"

class OmContext;

/// \brief Main window - Library tab child
///
/// OmDialog class derived for Main window Library tab child dialog window
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

    /// \brief Set dialog freeze mode
    ///
    /// Enable or disable the dialog freeze mode.
    ///
    /// The freeze mode is a modal-kind emulation for threaded operations,
    /// it disables (almost) all controls of the dialog and its children
    /// to prevent user to interact with elements while a threaded process
    /// is running.
    ///
    /// \param[in]  enable  : Enable or disable freeze mode.
    ///
    void freeze(bool enable);

    /// \brief Set dialog safe mode
    ///
    /// Enables or disable the dialog safe mode.
    ///
    /// The safe mode is used to operate modifications on sensitive
    /// or monitored elements such as deleting or moving Location in
    /// order to prevent conflicts or crash during process.
    ///
    /// \param[in]  enable  : Enable or disable safe mode.
    ///
    void safemode(bool enable);

    /// \brief Select Location
    ///
    /// Select or unselect Location then refresh dialog.
    ///
    /// \param[in]  i  Index of Location to select or -1 to select none.
    ///
    void locSel(int i);

    /// \brief Install selected packages.
    ///
    /// Public function to launch install process for selected packages if
    /// any.
    ///
    void pkgInst();

    /// \brief Uninstall selected packages.
    ///
    /// Public function to launch unsintall process for selected packages if
    /// any.
    ///
    void pkgUnin();

    /// \brief Toggle selected packages installation.
    ///
    /// Public function to toggle installation (either install or uninstall) of
    /// the selected packages if any.
    ///
    void pkgTogg();

    /// \brief View package details.
    ///
    /// Public function to open selected package property dialog.
    ///
    void pkgProp();

    /// \brief Move package to trash.
    ///
    /// Public function to move selected package to trash.
    ///
    void pkgTrsh();

    /// \brief Open package in explorer.
    ///
    /// Public function to open package in file explorer.
    ///
    void pkgOpen();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _dirMon_init(const wstring& path);

    void                _dirMon_stop();

    void*               _dirMon_hth;

    void*               _dirMon_hev[3];

    static DWORD WINAPI _dirMon_fth(void*);

    void                _pkgInst_init();

    void                _pkgInst_stop();

    void*               _pkgInst_hth;

    static DWORD WINAPI _pkgInst_fth(void*);

    void                _pkgUnin_init();

    void                _pkgUnin_stop();

    void*               _pkgUnin_hth;

    static DWORD WINAPI _pkgUnin_fth(void*);

    void                _batExe_init();

    void                _batExe_stop();

    void*               _batExe_hth;

    static DWORD WINAPI _batExe_fth(void*);

    bool                _thread_abort;

    void                _buildLvPkg();

    unsigned            _buildLvPkg_icSize;

    void                _buildCbLoc();

    void                _buildLbBat();

    void                _onCbLocSel();

    void                _onLvPkgRclk();

    void                _onLvPkgSel();

    void                _onLbBatSel();

    void                _onBcRunBat();

    void                _onBcNewBat();

    void                _onBcEdiBat();

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMAINLIB_H
