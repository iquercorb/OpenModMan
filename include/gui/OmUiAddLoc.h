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
#ifndef OMUIADDLOC_H
#define OMUIADDLOC_H

#include "OmDialog.h"

class OmContext;

/// \brief Add Location dialog
///
/// OmDialog class derived for Add Location dialog window
///
class OmUiAddLoc : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiAddLoc(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiAddLoc();

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
    /// via OmUiAddLoc.setContext
    ///
    /// \return Associated Context or nullptr if none.
    ///
    OmContext* ctxCur() const {
      return _pCtx;
    }

    /// \brief Set associated Context.
    ///
    /// Define the associated Context, which the dialog is intended to
    /// work with and on. A valid Context must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  pCtx  : Context object to associate.
    ///
    void ctxSet(OmContext* pCtx) {
      _pCtx = pCtx;
    }

  protected:

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmContext*                _pCtx;

    void                      _onTitleChange();

    void                      _onBcBrwDst();

    void                      _onCkBoxLib();

    void                      _onBcBrwLib();

    void                      _onCkBoxBck();

    void                      _onBcBrwBck();

    bool                      _onBcOk();

    void                      _onInit();

    void                      _onResize();

    INT_PTR                   _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIADDLOC_H
