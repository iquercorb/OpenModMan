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

#ifndef OMUIPROCPGRS_H
#define OMUIPROCPGRS_H

#include "OmDialog.h"

/// \brief Custom Process Progression dialog
///
/// OmDialog class derived for customizable process progression dialog
///
class OmUiProgress : public OmDialog
{
  public:

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

    /// \brief Set window Caption
    ///
    /// Set the window caption, which is the text in
    /// the title bar
    ///
    /// \param[in]  str   : Caption to set.
    ///
    void setCaption(const wchar_t* str) const;

    /// \brief Set dialog title
    ///
    /// Set the dialog title, which is the main text
    /// appearing above the Progress Bar control.
    ///
    /// \param[in]  str   : Title to set.
    ///
    void setTitle(const wchar_t* str) const;

    /// \brief Get abort pointer
    ///
    /// Returns a pointer to a boolean that indicate whether the
    /// abort button was clicked by user.
    ///
    /// \param[in]  str   : Title to set.
    ///
    const bool* getAbortPtr() const {
      return &this->_abort;
    }

    /// \brief Get progress bar control
    ///
    /// Returns the dialog progress bar control handle
    ///
    /// \return Handle (HWND) to dialog progress bar control
    ///
    void* getProgressBar() const;

    /// \brief Get static title control
    ///
    /// Returns the dialog title Static control handle
    ///
    /// \return Handle (HWND) to dialog title Static control
    ///
    void* getStaticTitle() const;

    /// \brief Get static comment control
    ///
    /// Returns the dialog comment Static control handle
    ///
    /// \return Handle (HWND) to dialog comment Static control
    ///
    void* getStaticComment() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _onInit();

    void                _onResize();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool                _abort;
};

#endif // OMUIPROCPGRS_H
