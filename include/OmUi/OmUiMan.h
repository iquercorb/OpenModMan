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
    OmUiManMain* pUiManMain() const {
      return this->_UiManMain;
    }

    /// \brief Get footer frame dialog
    ///
    /// Return pointer to secondary (bottom) frame dialog.
    ///
    /// \return Secondary (bottom) frame dialog.
    ///
    OmUiManFoot* pUiManFoot() const {
      return this->_UiManFoot;
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

    /// \brief Check Target directory read/write access
    ///
    /// Test read/write access for current Channel's Target directory
    /// and open a custom error message if access is denied.
    ///
    /// \param[in]  operation : Custom context string for error message.
    ///
    /// \return true if access granted, false otherwise
    ///
    bool checkTargetWrite(const OmWString& operation);

    /// \brief Check Mod Library directory read (only) access
    ///
    /// Test read (only) access for current Channel's Mod Library directory
    /// and open a custom error message if access is denied.
    ///
    /// \param[in]  operation : Custom context string for error message.
    ///
    /// \return true if access granted, false otherwise
    ///
    bool checkLibraryRead(const OmWString& operation);

    /// \brief Check Mod Library directory read/write access
    ///
    /// Test read/write access for current Channel's Mod Library directory
    /// and open a custom error message if access is denied.
    ///
    /// \param[in]  operation : Custom context string for error message.
    ///
    /// \return true if access granted, false otherwise
    ///
    bool checkLibraryWrite(const OmWString& operation);

    /// \brief Check Backup Data directory read/write access
    ///
    /// Test read/write access for current Channel's Backup Data directory
    /// and open a custom error message dialog if access is denied.
    ///
    /// \param[in]  operation : Custom context string for error message.
    ///
    /// \return true if access granted, false otherwise
    ///
    bool checkBackupWrite(const OmWString& operation);

    /// \brief Automatic Missing Mod dependencies dialog
    ///
    /// Quick check against supplied parameters for missing Mod dependencies
    /// then open the required custom dialog box to ask user to Continue or Abort.
    ///
    /// \param[in]  enabled   : Channel's option value for this warning.
    /// \param[in]  operation : Custom context string for dialog message.
    /// \param[in]  missings  : Produced missing dependencies list (can be empty).
    ///
    /// \return true if user clicked Continue, false otherwise.
    ///
    bool warnMissings(bool enabled, const OmWString& operation, const OmWStringArray& missings);

    /// \brief Automatic Missing Mod overlapping dialog
    ///
    /// Quick check against supplied parameters for Mod overlapping
    /// then open the required custom dialog box to ask user to Continue or Abort.
    ///
    /// \param[in]  enabled   : Channel's option value for this warning.
    /// \param[in]  operation : Custom context string for dialog message.
    /// \param[in]  overlaps  : Produced overlapping list (can be empty).
    ///
    /// \return true if user clicked Continue, false otherwise.
    ///
    bool warnOverlaps(bool enabled, const OmWString& operation, const OmWStringArray& overlaps);

    /// \brief Automatic Mod Additional Installs dialog
    ///
    /// Quick check against supplied parameters for Mod Additional Installs
    /// then open the required custom dialog box to ask user to Continue or Abort.
    ///
    /// \param[in]  enabled   : Channel's option value for this warning.
    /// \param[in]  operation : Custom context string for dialog message.
    /// \param[in]  depends   : Produced additional dependencies list (can be empty).
    ///
    /// \return true if user clicked Continue, false otherwise.
    ///
    bool warnExtraInstalls(bool enabled, const OmWString& operation, const OmWStringArray& depends);

    /// \brief Automatic Mod Additional Downloads dialog
    ///
    /// Quick check against supplied parameters for Mod Additional Downloads
    /// then open the required custom dialog box to ask user to Continue or Abort.
    ///
    /// \param[in]  enabled   : Channel's option value for this warning.
    /// \param[in]  operation : Custom context string for dialog message.
    /// \param[in]  depends   : Produced additional dependencies list (can be empty).
    ///
    /// \return true if user clicked Continue, false otherwise.
    ///
    bool warnExtraDownloads(bool enabled, const OmWString& operation, const OmWStringArray& depends);

    /// \brief Automatic Mod Additional Uninstalls dialog
    ///
    /// Quick check against supplied parameters for Mod Additional Uninstalls
    /// then open the required custom dialog box to ask user to Continue or Abort.
    ///
    /// \param[in]  enabled     : Channel's option value for this warning.
    /// \param[in]  operation   : Custom context string for dialog message.
    /// \param[in]  overlappers : Produced additional overlappers list (can be empty).
    /// \param[in]  dependents  : Produced additional dependents list (can be empty).
    ///
    /// \return true if user clicked Continue, false otherwise.
    ///
    bool warnExtraRestores(bool enabled, const OmWString& operation, const OmWStringArray& overlappers, const OmWStringArray& dependents);

    /// \brief Automatic Mod Breaking dependencies dialog
    ///
    /// Quick check against supplied parameters for Mod Uninstall that break dependencies
    /// then open the required custom dialog box to ask user to Continue or Abort.
    ///
    /// \param[in]  enabled     : Channel's option value for this warning.
    /// \param[in]  operation   : Custom context string for dialog message.
    /// \param[in]  breakings   : Produced additional broken dependencies list (can be empty).
    ///
    /// \return true if user clicked Continue, false otherwise.
    ///
    bool warnBreakings(bool enabled, const OmWString& operation, const OmWStringArray& breakings);

    /// \brief UI Layout split Cursor update
    ///
    /// Public function to proceed proper mouse cursor changes (usually on WM_SETCURSOR message)
    /// in response to mouse hovering layout split zone.
    ///
    /// \return true if mouse cursor changed from default (hovering split zone)
    ///
    bool splitCursorUpdate();

    /// \brief UI Layout split cursor capture
    ///
    /// Public function to test and capture mouse cursor (usually on WM_LBUTTONDOWN message)
    /// in order to enter Layout split move process.
    ///
    /// \return true if mouse cursor was captured, false otherwise
    ///
    bool splitCaptureCheck();

    /// \brief UI Layout split moving (captured) state
    ///
    /// Returns whether mouse cursor is currently captured by main window
    /// for Layout split move process.
    ///
    /// \return true if mouse cursor is captured, false otherwise
    ///
    bool splitMoving() const {
      return this->_split_moving;
    }

    /// \brief UI Layout split capture release
    ///
    /// Public function to release mouse cursor capture (usually on WM_LBUTTONUP message)
    /// in order to exit Layout split move process.
    ///
    /// \return true if mouse cursor was released from a capture state, false otherwise
    ///
    bool splitCaptureRelease();

    /// \brief UI Layout split move process
    ///
    /// Public function to proceed (usually on WM_MOUSEMOVE message) computation and proper resizes
    /// according mouse cursor position, or to check for mouse cursor hovering split zone.
    ///
    /// \return true if mouse cursor is captured or is hovering split zone, false otherwise.
    ///
    bool splitMoveProcess();

    /// \brief UI Layout split subclassing procedure
    ///
    /// Public static function to be used as subclass procedure for the various child controls
    /// to properly handle mouse cursors and motions for Layout split mechanism.
    ///
    static LRESULT WINAPI splitSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiManMain*         _UiManMain;

    OmUiManFoot*         _UiManFoot;

    OmUiManMainLib*      _UiManMainLib;

    OmUiManMainNet*      _UiManMainNet;

    // freeze and safe mode
    bool                _safe_mode;

    bool                _lock_mode;

    int32_t             _lock_idch;

    // layout split parameters
    bool                _split_hover_head;

    bool                _split_hover_foot;

    bool                _split_hover_side;

    bool                _split_moving;

    long                _split_params[3];

    // common elements
    HIMAGELIST          _listview_himl;

    uint32_t            _listview_himl_size;

    HIMAGELIST          _toolbars_himl;

    HMENU               _context_menu;

    // Preset setup processing
    int32_t             _psexec_count;

    int32_t             _psexec_idch;

    bool                _psexec_abort;

    void                _psexec_add(OmModPset*);

    static void         _psexec_begin_fn(void*, uint64_t);

    static bool         _psexec_progress_fn(void*, size_t, size_t, uint64_t);

    static void         _psexec_result_fn(void*, OmResult, uint64_t);

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

    bool                _lv_chn_show;

    int32_t             _lv_chn_span;

    // Mod Presets controls
    uint32_t            _lv_pst_icons_size;

    void                _lv_pst_populate();

    void                _lv_pst_on_resize();

    void                _lv_pst_on_selchg();

    void                _lv_pst_on_rclick();

    bool                _lv_pst_show;

    int32_t             _lv_pst_span;

    // Mod Overview controls

    bool                _ui_ovw_show;

    int32_t             _ui_ovw_span;

    // Layout functions
    void                _layout_save();

    void                _layout_load();

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
