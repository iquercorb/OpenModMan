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
#ifndef OMUIMAIN_H
#define OMUIMAIN_H

#include "OmDialog.h"

/// \brief Base ID for Recent menu-items
///
/// Base resource ID for "Recent Files" menu-items dynamically manager
/// by the dialog.
///
#define IDM_FILE_RECENT_PATH      40990

/// \brief Custom "Context Changed" window Message
///
/// Custom window message to notify tab child dialogs that selected
/// Context in main window changed.
///
#define UWM_MAIN_CTX_CHANGED      (WM_APP+16)


/// \brief Application Main window
///
/// OmDialog class derived for the application Main dialog window
///
class OmUiMain : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiMain(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiMain();

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

    /// \brief Open Context
    ///
    /// Try to load the specified Context file then refresh dialog.
    ///
    /// \param[in]  path  Context definition file path to load.
    ///
    void ctxOpen(const wstring& path);

    /// \brief Select Context
    ///
    /// Select or unselect Context then refresh dialog.
    ///
    /// \param[in]  i  Index of Context to select or -1 to select none.
    ///
    void ctxSel(int i);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    vector<wstring>     _pageName;

    vector<OmDialog*>   _pageDial;

    void                _addPage(const wstring& name, OmDialog* dialog);

    bool                _freeze_mode;

    bool                _freeze_abort;

    void                _buildMnRct();

    void                _buildCbCtx();

    void                _onCbCtxSel();

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onClose();

    void                _onQuit();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMAIN_H
