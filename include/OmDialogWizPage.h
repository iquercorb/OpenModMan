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
#ifndef OMDIALOGWIZPAGE_H
#define OMDIALOGWIZPAGE_H

#include "OmDialog.h"

/// \brief Wizard Dialog Page.
///
/// Base class for Wizard dialog Page and child window.
///
/// This class inherit from OmDialog and is used as base class to create
/// specific Wizard dialog Page window to be integrated as Wizard Page
/// child dialogs within a main Dialog Wizard window (OmDialogWiz).
///
/// \see OmDialogWiz
///
class OmDialogWizPage : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmDialogWizPage(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    virtual ~OmDialogWizPage();

    /// \brief Check valid fields.
    ///
    /// Checks whether the page has proper fields filled, that is, the
    /// next button can be enabled.
    ///
    /// \return True if propers fields are filled, false otherwise.
    ///
    virtual bool validFields() const;

    /// \brief Check valid parameters.
    ///
    /// Checks whether the page has valid parameters, that is, going to
    /// the next page is allowed.
    ///
    /// \return True if parameters are valid, false otherwise.
    ///
    virtual bool validParams() const;

  protected: ///        - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Notify fields changed
    ///
    /// Perform checks and actions according page state then enable or disable
    /// the 'Next' button accordingly.
    ///
    /// This function should be called each time dialog detect changes in fields.
    ///
    void fieldsChanged();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void        _onPgInit();

    virtual void        _onPgShow();

    virtual void        _onPgResize();

    virtual void        _onPgRefresh();

    virtual INT_PTR     _onPgMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

/// \brief OmDialogWizPage pointer array
///
/// Typedef for an STL vector of OmDialogWizPage pointer type
///
typedef std::vector<OmDialogWizPage*> OmPWizPageArray;

#endif // OMDIALOGWIZPAGE_H
