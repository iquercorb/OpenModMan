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
#ifndef OMUIMGR_H
#define OMUIMGR_H

#include "OmDialog.h"

/// \brief Base ID for Recent menu-items
///
/// Base resource ID for "Recent Files" menu-items dynamically manager
/// by the dialog.
///
#define IDM_FILE_RECENT_PATH      40990

/// \brief Custom "Mod Hub Changed" window Message
///
/// Custom window message to notify tab child dialogs that selected
/// Mod Hub in main window changed.
///
#define UWM_MAIN_HUB_CHANGED      (WM_APP+16)

/// \brief Custom "Request Abort" window Message
///
/// Custom window message to notify tab child dialogs that main dialog (user)
/// requested to quit, and must abort all running process
///
#define UWM_MAIN_ABORT_REQUEST    (WM_APP+17)

class OmUiManMain;
class OmUiManFoot;
class OmUiManMainLib;
class OmUiManMainNet;
class OmModPset;

/// \brief Application Main window
///
/// OmDialog class derived for the application Main dialog window
///
class OmUiMan : public OmDialog
{
  public: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiMan(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiMan();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    void enableLockMode(bool enable);

    bool lockMode() const {
      return this->_lock_mode;
    }

    void enableSafeMode(bool enable);

    bool safeMode() const {
      return this->_safe_mode;
    }

    /// \brief Open File
    ///
    /// Show Open Mod Hub file dialog then try to open selected file
    ///
    void openFile();

    /// \brief Open Recent file
    ///
    /// Open Mod Hub from saved Recent path at index
    ///
    /// \param[in] index  : Index in recent path list to open
    ///
    void openRecent(int32_t index);

    /// \brief Clear recent list
    ///
    /// Delete all path and menu-item from Recent List
    ///
    void clearRecents();

    /// \brief Open Mod Hub
    ///
    /// Try to load the specified Mod Hub file then refresh dialog.
    ///
    /// \param[in]  path  Mod Hub definition file path to load.
    ///
    void openHub(const OmWString& path);

    /// \brief Close current Mod Hub
    ///
    /// Close the current active Mod Hub file then refresh dialog.
    ///
    void closeHub();

    /// \brief Select Mod Hub
    ///
    /// Select or unselect Mod Hub then refresh dialog.
    ///
    /// \param[in] index  : Index of Mod Hub to select or -1 to select none.
    ///
    void selectHub(int32_t index);

    void hubProperties();

    /// \brief Select Mod Channel
    ///
    /// Select or unselect Mod Channel then refresh dialog.
    ///
    /// \param[in] index  : Index of Mod Channel to select or -1 to select none.
    ///
    void selectChannel(int32_t index);

    void createChannel();

    void deleteChannel(int32_t index = -1);

    void channelProperties();

    void createPreset();

    void deletePreset();

    void runPreset();

    void presetProperties();

    bool openArg(const OmWString& path);

    void openPkgEditor(const OmWString& path);

    void openRepEditor(const OmWString& path);

    /// \brief Get tab frame dialog
    ///
    /// Return pointer to main (top) frame dialog.
    ///
    /// \return Main (top) frame dialog.
    ///
    OmUiManMain* pUiMgrMain() const {
      return _UiManMain;
    }

    /// \brief Get footer frame dialog
    ///
    /// Return pointer to secondary (bottom) frame dialog.
    ///
    /// \return Secondary (bottom) frame dialog.
    ///
    OmUiManFoot* pUiMgrFoot() const {
      return _UiManFoot;
    }

    /// \brief Check whether in frame resize
    ///
    /// Checks whether dialog has captured the mouse to process frame
    /// resize by user.
    ///
    /// \return True if in frame resize process, false otherwise
    ///
    bool splitterCursorDragg() const {
      return _split_curs_dragg;
    }

    /// \brief Check whether mouse hover frame split
    ///
    /// Checks whether the mouse cursor is currently hovering between
    /// frames.
    ///
    /// \return True mouse cursor hovering between frames, false otherwise
    ///
    bool splitterCursorHover() const {
      return _split_curs_hover;
    }

    /// \brief List views ImageList handle
    ///
    /// Handle to shared ImageList for child dialogs ListView controls.
    ///
    /// \return ImagList handle
    ///
    HIMAGELIST listViewImgList() const {
      return this->_listview_himl;
    }

    /// \brief List views ImageList handle
    ///
    /// Handle to shared ImageList for child dialogs ListView controls.
    ///
    /// \return ImagList handle
    ///
    HIMAGELIST toolBarsImgList() const {
      return this->_toolbars_himl;
    }

    /// \brief Context menu handle
    ///
    /// Handle to hidden menu that hold right-click context popup
    ///
    /// \return Menu Handle
    ///
    HMENU contextMenu() const {
      return this->_context_menu;
    }

    /// \brief Context popup handle
    ///
    /// Handle to the specified context popup from the context menu
    ///
    /// \return Menu Handle
    ///
    HMENU getContextPopup(uint32_t index) const {
      return GetSubMenu(this->_context_menu, index);
    }

    bool checkTargetWrite(const OmWString& operation);

    bool checkLibraryRead(const OmWString& operation);

    bool checkLibraryWrite(const OmWString& operation);

    bool checkBackupWrite(const OmWString& operation);

    bool warnMissings(bool enabled, const OmWString& operation, const OmWStringArray& missings);

    bool warnOverlaps(bool enabled, const OmWString& operation, const OmWStringArray& overlaps);

    bool warnExtraInstalls(bool enabled, const OmWString& operation, const OmWStringArray& depends);

    bool warnExtraDownloads(bool enabled, const OmWString& operation, const OmWStringArray& depends);

    bool warnExtraRestores(bool enabled, const OmWString& operation, const OmWStringArray& overlappers, const OmWStringArray& dependents);

    bool warnBreakings(bool enabled, const OmWString& operation, const OmWStringArray& breakings);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiManMain*         _UiManMain;

    OmUiManFoot*         _UiManFoot;

    OmUiManMainLib*      _UiManMainLib;

    OmUiManMainNet*      _UiManMainNet;

    // freeze and safe mode
    bool                _safe_mode;

    bool                _lock_mode;

    int32_t             _lock_idch;

    // frame splitter
    bool                _split_curs_hover;

    bool                _split_curs_dragg;

    long                _split_move_param[3];

    // common elements
    HIMAGELIST          _listview_himl;

    uint32_t            _listview_himl_size;

    HIMAGELIST          _toolbars_himl;

    HMENU               _context_menu;

    // Preset setup processing
    int32_t             _psetup_count;

    int32_t             _psetup_idch;

    bool                _psetup_abort;

    void                _psetup_add(OmModPset*);

    static void         _psetup_begin_fn(void*, uint64_t);

    static bool         _psetup_progress_fn(void*, size_t, size_t, uint64_t);

    static void         _psetup_result_fn(void*, OmResult, uint64_t);

    // Delete channel processing
    void*               _delchan_hth;

    void*               _delchan_hwo;

    static DWORD WINAPI _delchan_run_fn(void*);

    static  VOID WINAPI _delchan_end_fn(void*,uint8_t);

    static bool         _delchan_progress_fn(void*, size_t, size_t, uint64_t);

    int32_t             _delchan_idch;

    void*               _delchan_hdlg;

    int32_t             _delchan_abort;

    // main window items
    void                _caption_populate();

    void                _status_populate();

    void                _menu_recent_populate();

    void                _menu_enable();

    // Mod Hub controls
    void                _sb_hub_populate();

    void                _cb_hub_populate();

    void                _cb_hub_on_selchange();

    // Mod Chan controls
    uint32_t            _lv_chn_icons_size;

    void                _lv_chn_populate();

    void                _lv_chn_on_resize();

    void                _lv_chn_on_selchg();

    void                _lv_chn_on_rclick();

    // Mod Presets controls
    uint32_t            _lv_pst_icons_size;

    void                _lv_pst_populate();

    void                _lv_pst_on_resize();

    void                _lv_pst_on_selchg();

    void                _lv_pst_on_rclick();

    // Dialog common functions
    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onClose();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMGR_H
