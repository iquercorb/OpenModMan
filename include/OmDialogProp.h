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
#ifndef OMDIALOGPROP_H
#define OMDIALOGPROP_H

#include "OmDialog.h"

/// \brief Property Dialog window.
///
/// Base class for Property dialog window interface, inherited from Dialog base
/// class. This class is used as interface for specific dialog which use others
/// dialog as child within a TabControl control.
///
class OmDialogProp : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmDialogProp(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    virtual ~OmDialogProp();

    /// \brief Check for properties changes
    ///
    /// Checks whether dialog's dedicated properties changed then perform
    /// proper operations, such as enabling or disabling the Apply button.
    ///
    virtual bool checkChanges();

    /// \brief Apply properties changes
    ///
    /// Retrieve dialog's dedicated properties then apply changes.
    ///
    virtual bool applyChanges();


  protected: ///        - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    vector<wstring>     _pageName;

    vector<OmDialog*>   _pageDial;

    void                _addPage(const wstring& name, OmDialog* dialog);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    HWND                _hTab;

    HWND                _hBcOk;

    HWND                _hBcApply;

    HWND                _hBcCancel;

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onQuit();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void        _onPropInit();

    virtual void        _onPropShow();

    virtual void        _onPropResize();

    virtual void        _onPropQuit();

    virtual bool        _onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMDIALOGPROP_H
