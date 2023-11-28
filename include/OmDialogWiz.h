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
#ifndef OMDIALOGWIZ_H
#define OMDIALOGWIZ_H

#include "OmBase.h"
#include "OmBaseWin.h"

#include "OmDialog.h"

/// \brief Wizard Dialog window.
///
/// Base class for Wizard dialog window interface, inherited from Dialog base
/// class. This class is used as interface for specific dialog which use others
/// dialog as child to create a Wizard dialog sequence.
///
class OmDialogWiz : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmDialogWiz(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    virtual ~OmDialogWiz();

    /// \brief Enable Next button
    ///
    /// Enables or disables the dialog wizard Next button
    ///
    /// \param[in]  allow  : Boolean to enable or disable the Next button
    ///
    void setNextAllowed(bool allow);

  protected: ///        - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    std::vector<OmDialog*>   _pageDial;

    void                _addPage(OmDialog* dialog);

    void                _pagesOnCmd(int bc_back, int bc_next, WPARAM wParam);

    int                 _currPage;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    HBITMAP             _hBmSplash;

    void                _onInit();

    void                _onShow();

    void                _onResize();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void        _onWizInit();

    virtual void        _onWizShow();

    virtual void        _onWizResize();

    virtual bool        _onWizNext();

    virtual void        _onWizFinish();

    virtual INT_PTR     _onWizMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMDIALOGWIZ_H
