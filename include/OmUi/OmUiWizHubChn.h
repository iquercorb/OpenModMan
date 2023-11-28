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
#ifndef OMUIWIZHUBCHN_H
#define OMUIWIZHUBCHN_H

#include "OmDialog.h"

/// \brief New Mod Hub Wizard / Mod Channel page child
///
/// OmDialog class derived for New Mod Hub Wizard / Mod Channel page child dialog window
///
class OmUiWizHubChn : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiWizHubChn(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiWizHubChn();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Check valid parameters.
    ///
    /// Checks whether the dialog actually contain valid
    /// parameters set by user.
    ///
    /// \return True parameters set by user are valid, false otherwise.
    ///
    bool hasValidParams() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _onTitleChange();

    void                _onBcBrwDst();

    void                _onCkBoxLib();

    void                _onBcBrwLib();

    void                _onCkBoxBck();

    void                _onBcBrwBck();

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIWIZHUBCHN_H
