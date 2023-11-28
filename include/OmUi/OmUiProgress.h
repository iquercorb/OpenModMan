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
#ifndef OMUIPROGRESS_H
#define OMUIPROGRESS_H

#include "OmDialog.h"

/// \brief Progression dialog
///
/// OmDialog class derived for Progression dialog
///
class OmUiProgress : public OmDialog
{
  public: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiProgress(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiProgress();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Set progress bar range
    ///
    /// Set progress bar control range.
    ///
    /// \param[in]  min   : Minimum or progression start value.
    /// \param[in]  min   : Maximum or progression finish value.
    ///
    void setPbRange(uint16_t min, uint16_t max) const;

    /// \brief Set progress bar position
    ///
    /// Set progress bar control position within previously defined range.
    ///
    /// \param[in]  pos   : Value to set position.
    ///
    void setPbPos(uint16_t pos) const;

    /// \brief Set header text
    ///
    /// Set progress dialog header text, this is the main
    /// title text at top of dialog.
    ///
    /// \param[in]  text   : Text to set.
    ///
    void setScHeadText(const OmWString& text) const;

    /// \brief Set item text
    ///
    /// Set progress dialog item or status text, this is the
    /// text right above the progress bar.
    ///
    /// \param[in]  text   : Text to set.
    ///
    void setScItemText(const OmWString& text) const;

    /// \brief Get progress bar control
    ///
    /// Returns the dialog progress bar control handle
    ///
    /// \return Handle (HWND) to dialog progress bar control
    ///
    HWND hPb() const;

    /// \brief Get head text handle
    ///
    /// Returns handle to head text Static control, this is the main
    /// title text at top of dialog.
    ///
    /// \return Handle (HWND) to dialog Static control.
    ///
    HWND hScHead() const;

    /// \brief Get item text handle
    ///
    /// Returns handle to item text Static control, this is the
    /// text right above the progress bar.
    ///
    /// \return Handle (HWND) to dialog Static control.
    ///
    HWND hScItem() const;

    /// \brief Get abort signal
    ///
    /// Returns current dialog abort signal flag.
    ///
    /// \return true if user clicked abort button, false otherwise.
    ///
    bool abortGet() const {
      return this->_abort;
    }

    /// \brief Set abort signal
    ///
    /// Sets abort signal flag then enable or disable button in accordance.
    ///
    /// \param[in]  enable    : Abort signal flag to set.
    ///
    void abortSet(bool enable);

    /// \brief Get abort signal pointer
    ///
    /// Returns a pointer to abort signal boolean.
    ///
    /// \return Pointer to abort clicked boolean value.
    ///
    const bool* abortPtr() const {
      return &this->_abort;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    bool                _abort;

    void                _bc_abort_hit();

    void                _onInit();

    void                _onResize();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROGRESS_H
