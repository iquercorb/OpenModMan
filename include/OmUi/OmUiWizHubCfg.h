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
#ifndef OMUIWIZHUBCFG_H
#define OMUIWIZHUBCFG_H

#include "OmDialogWizPage.h"

/// \brief New Mod Hub Wizard / Configuration page child
///
/// OmDialog class derived for New Mod Hub Wizard / Configuration page child dialog window
///
class OmUiWizHubCfg : public OmDialogWizPage
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiWizHubCfg(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiWizHubCfg();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Check valid fields.
    ///
    /// Checks whether the page has proper fields filled, that is, the
    /// next button can be enabled.
    ///
    /// \return True if propers fields are filled, false otherwise.
    ///
    bool validFields() const;

    /// \brief Check valid parameters.
    ///
    /// Checks whether the page has valid parameters, that is, going to
    /// the next page is allowed.
    ///
    /// \return True if parameters are valid, false otherwise.
    ///
    bool validParams() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _update_hub_path();

    void                _browse_dir_home();

    void                _onPgInit();

    void                _onPgResize();

    INT_PTR             _onPgMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIWIZHUBCFG_H
