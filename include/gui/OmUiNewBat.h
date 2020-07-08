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
#ifndef OMUIPSTMAKE_H
#define OMUIPSTMAKE_H

#include "OmDialog.h"

class OmContext;

/// \brief New Batch dialog
///
/// OmDialog class derived for Preset Builder dialog window
///
class OmUiNewBat : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiNewBat(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiNewBat();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Context.
    ///
    /// Returns associated Context object previously defined
    /// via OmUiNewLoc.setContext
    ///
    /// \return Associated Context or nullptr if none.
    ///
    OmContext* context() const {
      return _context;
    }

    /// \brief Set associated Context.
    ///
    /// Define the associated Context, which the dialog is intended to
    /// work with and on. A valid Context must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  ctx   : Context object to associate.
    ///
    void setContext(OmContext* ctx) {
      _context = ctx;
    }

  protected:

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmContext*                _context;

    vector<vector<int>>       _excLs;

    vector<vector<int>>       _incLs;

    void                      _rebuildPkgLb();

    void                      _qucikFromCur();

    void                      _upPkg();

    void                      _dnPkg();

    void                      _addPkg();

    void                      _remPkg();

    bool                      _apply();

    void                      _onShow();

    void                      _onResize();

    void                      _onRefresh();

    void                      _onQuit();

    bool                      _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPSTMAKE_H
