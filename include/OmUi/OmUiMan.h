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

/// \brief Main menu intems indices
///
/// Indices for main menu items
///
#define MNU_FILE             static_cast<unsigned>(0)
    #define MNU_FILE_NEW         0
        #define MNU_FILE_NEW_HUB    0
    // ------------------------- 1
    #define MNU_FILE_OPEN        2
    #define MNU_FILE_RECENT      3
    // ------------------------- 4
    #define MNU_FILE_CLOSE       5
    // ------------------------- 6
    #define MNU_FILE_QUIT        7

#define MNU_EDIT             1
    #define MNU_EDIT_HUB         0
        #define MNU_EDIT_ADDCHN      0
        // ------------------------- 1
        #define MNU_EDIT_PROPHUB     2

    // ------------------------- 1
    #define MNU_EDIT_CHN         2
        #define MNU_EDIT_PROPCHN     0

    // ------------------------- 3
    #define MNU_EDIT_MOD         4
        #define MNU_EDIT_MOD_INST    0
        #define MNU_EDIT_MOD_UINS    1
        #define MNU_EDIT_MOD_CLNS    2
        // ------------------------- 3
        #define MNU_EDIT_MOD_DISC    4
        // ------------------------- 5
        #define MNU_EDIT_MOD_OPEN    6
        #define MNU_EDIT_MOD_TRSH    7
        // ------------------------- 8
        #define MNU_EDIT_MOD_EDIT    9
        // ------------------------- 10
        #define MNU_EDIT_MOD_INFO    11

    #define MNU_EDIT_NET         5
        #define MNU_EDIT_NET_DNLD    0
        #define MNU_EDIT_NET_DNWS    1
        // ------------------------- 2
        #define MNU_EDIT_NET_ABRT    3
        #define MNU_EDIT_NET_RVOK    4
        // ------------------------- 5
        #define MNU_EDIT_NET_FIXD    6
        // ------------------------- 7
        #define MNU_EDIT_NET_INFO    8
    // ------------------------- 9
    #define MNU_EDIT_PROPMAN     6

#define MNU_TOOL             2
    #define MNU_TOOL_EDITPKG     0
    #define MNU_TOOL_EDITREP     1

#define MNU_HELP   3
    #define MNU_HELP_DEBUG       0
    #define MNU_HELP_ABOUT       1


class OmUiManMain;
class OmUiManFoot;
class OmUiManMainLib;
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

    /// \brief Set dialog install mode
    ///
    /// Enable or disable the dialog install mode.
    ///
    /// The install mode is a modal-kind emulation for threaded operations,
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
    /// or monitored elements such as deleting or moving Mod Channel in
    /// order to prevent conflicts or crash during process.
    ///
    /// \param[in]  enable  : Enable or disable safe mode.
    ///
    void safemode(bool enable);

    /// \brief Monitor Library
    ///
    /// Start or stop Library directory changes monitoring.
    ///
    /// \param[in]  enable  : Start or stop monitoring.
    ///
    void monitorLibrary(bool enable);

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
    /// \param[in]  i  Index of Mod Hub to select or -1 to select none.
    ///
    void selectHub(int32_t i);

    /// \brief Select Mod Channel
    ///
    /// Select or unselect Mod Channel then refresh dialog.
    ///
    /// \param[in]  i  Index of Mod Channel to select or -1 to select none.
    ///
    void selectChannel(int32_t id);

    void createPreset();

    void deletePreset();

    void runPreset();

    void presetProperties();

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
    void* listViewImgList() const {
      return this->_listview_himl;
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

    // freeze and safe mode
    bool                _freeze_mode;

    bool                _freeze_quit;

    // frame splitter
    bool                _split_curs_hover;

    bool                _split_curs_dragg;

    long                _split_move_param[3];

    // library directory monitor
    void*               _lib_monitor_hth;

    void*               _lib_monitor_hev[3];

    static DWORD WINAPI _lib_monitor_fth(void*);

    // common elements
    void*               _listview_himl;

    uint32_t            _listview_himl_size;

    // Preset setup processing
    int32_t             _setups_count;

    int32_t             _setups_chan_sel;

    void                _setups_abort();

    void                _setups_add(OmModPset*);

    static void         _setups_begin_fn(void*, uint64_t);

    static bool         _setups_progress_fn(void*, size_t, size_t, uint64_t);

    static void         _setups_result_fn(void*, OmResult, uint64_t);


    // main window items
    void                _caption_populate();

    void                _menu_recent_populate();

    // Mod Hub controls
    void                _sb_hub_populate();

    void                _cb_hub_populate();

    void                _cb_hub_on_selchange();

    // Mod Chan controls
    uint32_t            _lv_chn_icons_size;

    void                _lv_chn_populate();

    void                _lv_chn_on_resize();

    void                _lv_chn_on_selchg();

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
