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

#include "OmBase.h"
#include "OmBaseWin.h"

#include "OmDialog.h"

/// \brief Property Dialog main frame.
///
/// Base class for Property dialog main frame and parent window.
/// This class inherit from OmDialog and is used as base class to create
/// specific dialog properties main/parent window that contain properties
/// Tab child dialogs (OmDialogPropTab).
///
/// \see OmDialogPropTab
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

    /// \brief Validate properties changes
    ///
    /// Checks whether dialog's changed properties are valid and suitable
    /// then alert user or perform required action in context.
    ///
    virtual bool validChanges();

    /// \brief Apply properties changes
    ///
    /// Retrieve dialog's dedicated properties then apply changes.
    ///
    virtual bool applyChanges();


  protected: ///        - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmWStringArray     _pageName;

    std::vector<OmDialog*>   _pageDial;

    void                _addPage(const OmWString& name, OmDialog* dialog);

    void                _setNoChange(bool enable);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    bool                _noChanges;

    HWND                _hTab;

    HWND                _hBcOk;

    HWND                _hBcApply;

    HWND                _hBcCancel;

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void        _onPropInit();

    virtual void        _onPropShow();

    virtual void        _onPropResize();

    virtual void        _onPropQuit();

    virtual INT_PTR     _onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMDIALOGPROP_H
