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
#ifndef OMUIMAINNET_H
#define OMUIMAINNET_H

#include "OmDialog.h"

/// \brief Main window - Network Tab child
///
/// OmDialog class derived for Main window Network tab child dialog window
///
class OmUiMainNet : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiMainNet(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiMainNet();

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
    /// \param[in]  i     : Index of Location to select or -1 to select none.
    ///
    void locSel(int i);

    /// \brief Download selection
    ///
    /// Launch the download for the selected remote packages.
    ///
    /// \param[in]  upgrade : Indicate the download process is an upgrade (remove superseded packages)
    ///
    void rmtDown(bool upgrade = false);

    /// \brief Fix selection dependencies
    ///
    /// Launch the download for the required dependencies of selected remote package.
    ///
    /// \param[in]  upgrade : Indicate the download process is an upgrade (remove superseded packages)
    ///
    void rmtFixd(bool upgrade = false);

    /// \brief View remote package details.
    ///
    /// Public function to open selected remote package property dialog.
    ///
    void rmtProp();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _dirMon_init(const wstring& path);

    void                _dirMon_stop();

    void*               _dirMon_hth;

    void*               _dirMon_hev[3];

    static DWORD WINAPI _dirMon_fth(void*);

    void                _repQry_init(int sel = -1);

    int                 _repQry_sel;

    void                _repQry_stop();

    void*               _repQryt_hth;

    static DWORD WINAPI _repQry_fth(void*);

    void                _rmtDnl_stop();

    bool                _rmtDnl_update(double tot, double cur, double rate, uint64_t hash);

    void                _rmtDnl_finish(uint64_t hash);

    static bool         _rmtDnl_download_cb(void* ptr, double tot, double cur, double rate, uint64_t data);

    unsigned            _rmtDnl_count;

    uint64_t            _rmtDnl_abort;

    bool                _thread_abort;

    void                _buildLvRep();

    void                _rsizeLvRep();

    unsigned            _buildLvRep_icSize;

    void                _buildCbLoc();

    void                _buildLvRmt();

    void                _rsizeLvRmt();

    unsigned            _buildLvRmt_icSize;

    void                _onCbLocSel();

    void                _onLvRmtHit();

    void                _onLvRepSel();

    void                _onLvRepHit();

    void                _onLvRmtRclk();

    void                _onLvRmtSel();

    void                _onBcChkRep();

    void                _onBcStopRep();

    void                _onBcNewRep();

    void                _onBcDelRep();

    void                _onBcAbort();

    void                _onInit();

    void                _onShow();

    void                _onHide();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMAINNET_H
