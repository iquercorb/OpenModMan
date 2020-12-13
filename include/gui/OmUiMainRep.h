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

#ifndef OMUIMAINREP_H
#define OMUIMAINREP_H

#include "OmDialog.h"

/// \brief Main window Repositories Tab
///
/// OmDialog class derived for Main Window Repositories Tab_Control child
/// dialog window
///
class OmUiMainRep : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiMainRep(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiMainRep();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Select Location
    ///
    /// Select or unselect Location then refresh dialog.
    ///
    /// \param[in]  i  Index of Location to select or -1 to select none.
    ///
    void selLocation(int i);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    HFONT               _hFtTitle;

    HFONT               _hFtMonos;

    HBITMAP             _hBmBlank;

    HBITMAP             _hBmBcNew;

    HBITMAP             _hBmBcDel;

    HBITMAP             _hBmBcRef;

    void                _reloadLibEc();

    void                _reloadLocCb();

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMAINREP_H
