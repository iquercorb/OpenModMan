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
#ifndef OMUIMGRMAINLIB_H
#define OMUIMGRMAINLIB_H

#include "OmDialog.h"

class OmUiMan;
//class OmModHub;

/// \brief Main window - Library tab child
///
/// OmDialog class derived for Main window Library tab child dialog window
///
class OmUiManMainLib : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiManMainLib(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiManMainLib();

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
    /// or monitored elements such as deleting or moving Mod Channel in
    /// order to prevent conflicts or crash during process.
    ///
    /// \param[in]  enable  : Enable or disable safe mode.
    ///
    void safemode(bool enable);

    /// \brief Refresh library
    ///
    /// Public function to refresh Library content. This function is
    /// typically called each time Library directory monitor thread
    /// detect changes in directory.
    ///
    void refreshLibrary();

    /// \brief Open Mod properties
    ///
    /// Opens currently selected Mod properties window.
    ///
    void showProperties();

    /// \brief Delete Mods Source
    ///
    /// Moves selected Mods (Sources) to recycle bin.
    ///
    void deleteSources();

    /// \brief Discard Mods Backup data
    ///
    /// Discard Backup data of the selected mods
    ///
    void discardBackups();

    /// \brief Open package in explorer.
    ///
    /// Opens the selected Mods in explorer or compatible application
    ///
    void exploreSources();

    /// \brief Open Mod in Mod Pack editor.
    ///
    /// Load the selected Mod in the Mod Pack editor, open Mod Pack
    /// editor window if necessary
    ///
    void editSource();

    /// \brief Add selected to queue for installation
    ///
    /// Add the currently selected Mods to Mods operations queue assuming this is
    /// for Mods installation
    ///
    /// \param[in] silent : Do not show warning messages and accept all
    ///
    void queueInstalls(bool silent = false);

    /// \brief Perform installations
    ///
    /// Perform installation of the given Mods in synchronous way (without thread).
    ///
    /// The Mod Pack in the supplied array must belong the current selected Mod Channel.
    ///
    /// \param[in] selection  : Array of Mods Pack object to install
    /// \param[in] silent     : Do not show warning messages and accept all
    ///
    void execInstalls(const OmPModPackArray& selection, bool silent);

    /// \brief Add selected to queue for restoration
    ///
    /// Add the currently selected Mods to Mods operations queue assuming this is
    /// for Mods restoration
    ///
    /// \param[in] silent : Do not show warning messages and accept all
    ///
    void queueRestores(bool silent = false);

    /// \brief Perform restorations
    ///
    /// Perform installation of the given Mods in synchronous way (without thread).
    ///
    /// The Mod Pack in the supplied array must belong the current selected Mod Channel.
    ///
    /// \param[in] selection  : Array of Mods Pack object to retore
    /// \param[in] silent     : Do not show warning messages and accept all
    ///
    void execRestores(const OmPModPackArray& selection, bool silent);

    /// \brief Add selected to queue for toggle
    ///
    /// Add the currently selected Mod to Mods operations queue and determine
    /// whether this is for install or uninstall according current Mod state.
    ///
    /// This function does not work with multiple selection.
    ///
    /// \param[in] silent : Do not show warning messages and accept all
    ///
    void queueAuto(bool silent = false);

    /// \brief Add selected to queue for cleaning
    ///
    /// Add the currently selected Mods to Mods operations queue assuming this is
    /// for Mods cleaning (restores selected and their unused dependencies).
    ///
    /// \param[in] silent : Do not show warning messages and accept all
    ///
    void queueCleaning(bool silent = false);

    /// \brief Abort all running operation
    ///
    /// Abort all the running operation and empty queues.
    ///
    void abortAll();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiMan*            _UiMan;

    // thread and UI management
    void                _update_safemode_status();

    // mods operation (install/uninstall)
    int32_t             _modops_count;

    OmPModPackQueue     _modops_ovr_queue;

    void                _modops_abort();

    void                _modops_add(const OmPModPackArray&);

    void                _modops_exec(const OmPModPackArray&);

    static void         _modops_begin_fn(void*, uint64_t);

    static bool         _modops_progress_fn(void*, size_t, size_t, uint64_t);

    static void         _modops_result_fn(void*, OmResult, uint64_t);

    // Mod Library Path Edit
    void                _ec_lib_populate();

    // Mod Library ListView
    uint32_t            _lv_mod_icons_size;

    void*               _lv_mod_cdraw_htheme;

    void                _lv_mod_populate();

    void                _lv_mod_on_resize();

    void                _lv_mod_on_selchg();

    void                _lv_mod_on_rclick();

    void                _lv_mod_cdraw_progress(HDC hDc, uint64_t item, int32_t subitem);

    int32_t             _lv_mod_get_status_icon(const OmModPack* ModPack);


    void                _bc_abort_hit();

    void                _onInit();

    void                _onShow();

    void                _onHide();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMGRMAINLIB_H
