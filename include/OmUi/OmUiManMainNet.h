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
#ifndef OMUIMGRMAINNET_H
#define OMUIMGRMAINNET_H

#include "OmDialog.h"

class OmUiMan;

/// \brief Main window - Network Tab child
///
/// OmDialog class derived for Main window Network tab child dialog window
///
class OmUiManMainNet : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiManMainNet(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiManMainNet();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    void addRepository();

    void deleteRepository();

    void queryRepositories();

    /// \brief Stop downloads
    ///
    /// Stop/pause download of the selected Mods
    ///
    void stopDownloads();

    /// \brief Download selection
    ///
    /// Launch the download for the selected Mods.
    ///
    /// \param[in]  upgrade : Indicate the download process is an upgrade (remove superseded packages)
    ///
    void startDownloads(bool upgrade = false);

    /// \brief Fix selection dependencies
    ///
    /// Launch the download for the required dependencies of selected Mods.
    ///
    /// \param[in]  upgrade : Indicate the download process is an upgrade (remove superseded packages)
    ///
    void downloadDepends(bool upgrade = false);

    /// \brief Revoke downloads
    ///
    /// Revoke partial download data of the selected Mods
    ///
    void revokeDownloads();

    /// \brief Open Net Pack properties.
    ///
    /// Public function to open selected Network Mod Pack property dialog.
    ///
    void showProperties() const;


    bool splitCursorUpdate();

    bool splitCaptureCheck();

    bool splitMoving() const {
      return this->_split_moving;
    }

    bool splitCaptureRelease();

    bool splitMoveProcess();

    static LRESULT WINAPI splitSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);


  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiMan*            _UiMan;

    // library monitor callback
    static void         _netlib_notify_fn(void*, OmNotify, uint64_t);

    // thread and UI management
    void                _refresh_processing();

    void                _abort_processing();

    // repository query stuff
    void                _query_start(const OmPNetRepoArray&);

    void                _query_abort();

    static void         _query_begin_fn(void*, uint64_t);

    static void         _query_result_fn(void*, OmResult, uint64_t);

    static void         _query_ended_fn(void*, OmNotify, uint64_t);

    // Mods download stuff
    bool                _download_upgrd;

    void                _download_abort();

    void                _download_start(bool, const OmPNetPackArray&);

    static void         _download_begin_fn(void*, uint64_t);

    static bool         _download_download_fn(void*, int64_t, int64_t, int64_t, uint64_t);

    static void         _download_result_fn(void*, OmResult, uint64_t);

    static void         _download_ended_fn(void*, OmNotify, uint64_t);

    // Mods upgrade stuff
    bool                _upgrade_abort;

    void                _upgrade_start(OmModChan*, const OmPNetPackArray&);

    static void         _upgrade_begin_fn(void*, uint64_t);

    static bool         _upgrade_progress_fn(void*, size_t, size_t, uint64_t);

    static void         _upgrade_result_fn(void*, OmResult, uint64_t);

    static void         _upgrade_ended_fn(void*, OmNotify, uint64_t);

    // repositories ListView
    void                _lv_rep_populate();

    void                _lv_rep_on_resize();

    unsigned            _lv_rep_icons_size;

    void                _lv_rep_on_selchg();

    void                _lv_rep_on_dblclk();

    void                _lv_rep_on_rclick();

    int32_t             _lv_rep_get_status_icon(const OmNetRepo* NetRepo);

    int32_t             _lv_rep_span;

    // network library ListView
    uint32_t            _lv_net_icons_size;

    void                _lv_net_populate();

    void                _lv_net_alterate(OmNotify action, uint64_t param);

    void                _lv_net_on_resize();

    void                _lv_net_cdraw_progress(HDC hDc, uint64_t item, int32_t subitem);

    void*               _lv_net_cdraw_htheme;

    void                _lv_net_on_selchg();

    void                _lv_net_on_dblclk();

    int32_t             _lv_net_get_status_icon(const OmNetPack* NetPack);

    void                _lv_net_on_rclick();

    // main buttons
    void                _bc_stop_clicked();

    void                _bc_abort_clicked();

    // ListView splitter

    bool                _split_moving;

    bool                _split_hover_lvrep;

    long                _split_params[3];

    // Layout functions
    void                _layout_save();

    void                _layout_load();

    // common functions
    void                _onInit();

    void                _onShow();

    void                _onHide();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMGRMAINNET_H
