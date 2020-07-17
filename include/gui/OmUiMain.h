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

// Base resource ID for "File > Recent Files" popup path list
#define IDM_FILE_RECENT_PATH      40990


/// \brief Application main window
///
/// OmDialog class derived for the application main window
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

    /// \brief Set on-process state
    ///
    /// Enables or disable the On-Process state of the dialog. This is
    /// intended to provide a modal kind behavior.
    ///
    /// \param[in]  enable    Enable or disable On-Process state of dialog.
    ///
    void setOnProcess(bool enable);

    /// \brief Set dialog safe-edit mode
    ///
    /// Enables or disable the dialog safe-edit mode.
    ///
    /// This unselect the current Context Location to disable automated
    /// refresh process preventing application crash during hard modification
    /// of Location or Context environment.
    ///
    /// \param[in]  enable    Enable or disable Safe-Edit state of dialog.
    ///
    void setSafeEdit(bool enable);

    /// \brief Get File menu sub-item
    ///
    /// Returns handle to the specified sub-item in File menu
    ///
    /// \param[in]  pos       Zero based Sub-item position in menu.
    ///
    HMENU getMenuFile(unsigned pos);

    /// \brief Get Edit menu sub-item
    ///
    /// Returns handle to the specified sub-item in Edit menu
    ///
    /// \param[in]  pos       Zero based Sub-item position in menu.
    ///
    HMENU getMenuEdit(unsigned pos);

    /// \brief Get Help menu sub-item
    ///
    /// Returns handle to the specified sub-item in Help menu
    ///
    /// \param[in]  pos       Zero based Sub-item position in menu.
    ///
    HMENU getMenuHelp(unsigned pos);

    /// \brief Enable or disable File menu sub-item
    ///
    /// Set File menu item enable state. The function is a shortcut for
    /// EnableMenuItem WINAPI function.
    ///
    /// \param[in]  item      Menu item position or command ID.
    /// \param[in]  enable    Enable flag to set.
    ///
    void setMenuFile(unsigned item, unsigned enable);

    /// \brief Enable or disable Edit menu sub-item
    ///
    /// Set Edit menu item enable state. The function is a shortcut for
    /// EnableMenuItem WINAPI function.
    ///
    /// \param[in]  item      Menu item position or command ID.
    /// \param[in]  enable    Enable flag to set.
    ///
    void setMenuEdit(unsigned item, unsigned enable);

    /// \brief Enable or disable Help menu sub-item
    ///
    /// Set Help menu item enable state. The function is a shortcut for
    /// EnableMenuItem WINAPI function.
    ///
    /// \param[in]  item      Menu item position or command ID.
    /// \param[in]  enable    Enable flag to set.
    ///
    void setMenuHelp(unsigned item, unsigned enable);

    /// \brief Open Context
    ///
    /// Try to load the specified Context file then refresh dialog.
    ///
    /// \param[in]  path  Context definition file path to load.
    ///
    void openContext(const wstring& path);

    /// \brief Select Context
    ///
    /// Select or unselect Context then refresh dialog.
    ///
    /// \param[in]  i  Index of Context to select or -1 to select none.
    ///
    void selContext(int i);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    vector<wstring>     _pageName;

    vector<OmDialog*>   _pageDial;

    void                _addPage(const wstring& name, OmDialog* dialog);

    bool                _quitPending;

    bool                _onProcess;

    bool                _safeEdit;

    HMENU               _hMenuFile;

    HMENU               _hMenuEdit;

    HMENU               _hMenuHelp;

    void                _reloadCtxCb();

    void                _reloadMenu();

    void                _reloadCtxIcon();

    void                _reloadCaption();

    void                _onInit();

    void                _onResize();

    void                _onRefresh();

    void                _onClose();

    void                _onQuit();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMAIN_H
