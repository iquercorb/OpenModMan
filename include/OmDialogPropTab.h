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
#ifndef OMDIALOGPROPTAB_H
#define OMDIALOGPROPTAB_H

#include "OmBase.h"
#include "OmBaseWin.h"

#include "OmDialog.h"


#define OMDIALOGPROP_MAX_PARAM  16

/// \brief Property Dialog Tab.
///
/// Base class for Property dialog tab and child window.
/// This class inherit from OmDialog and is used as base class to create
/// specific dialog properties tab window to be integrated as properties
/// Tab child dialogs within a main Dialog properties window (OmDialogProp).
///
/// \see OmDialogProp
///
class OmDialogPropTab : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmDialogPropTab(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    virtual ~OmDialogPropTab();
    /// \brief Check changed parameter
    ///
    /// Checks whether the specified dialog related parameter has been
    /// marked as changed, meaning it was modified by user.
    ///
    /// \param[in]  i   : Dialog related parameter to check
    ///
    /// \return True if the specified parameter is marked as changed,
    ///         false otherwise
    ///
    bool paramChanged(unsigned i) const {
      return _chParam[i];
    }

    /// \brief Set parameter to be checked
    ///
    /// Marks the specified dialog related parameter to be checked for changes.
    ///
    /// \param[in]  i   : Dialog related parameter to mark
    ///
    void paramCheck(unsigned i);

    /// \brief Reset parameter as not changed
    ///
    /// Marks the specified dialog related parameter as not modified so it will
    /// not be checked again nor applied.
    ///
    /// \param[in]  i   : Dialog related parameter to mark
    ///
    void paramReset(unsigned i);

  protected:

    bool                _chParam[OMDIALOGPROP_MAX_PARAM];

    void                _paramResetAll();

  private:

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void        _onTabInit();

    virtual void        _onTabShow();

    virtual void        _onTabResize();

    virtual void        _onTabRefresh();

    virtual void        _onTabQuit();

    virtual INT_PTR     _onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMDIALOGPROPTAB_H
