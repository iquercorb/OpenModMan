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
#ifndef OMUIMGRMAIN_H
#define OMUIMGRMAIN_H

#include "OmDialog.h"

class OmUiMgr;

/// \brief Window Main (top) Frame.
///
/// OmDialog class derived for the main window top frame which contain
/// main application tabs.
///
class OmUiMgrMain : public OmDialog
{
  public: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiMgrMain(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiMgrMain();

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
    /// or monitored elements such as deleting or moving Location in
    /// order to prevent conflicts or crash during process.
    ///
    /// \param[in]  enable  : Enable or disable safe mode.
    ///
    void safemode(bool enable);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiMgr*            _pUiMgr;

    vector<wstring>     _tabName;

    vector<OmDialog*>   _tabDial;

    void                _addTab(const wstring& name, OmDialog* dialog);

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMGRMAIN_H
