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
#ifndef OMUIWIZREPQRY_H
#define OMUIWIZREPQRY_H

#include "OmDialogWizPage.h"

/// \brief Repository Config Wizard query page
///
/// OmDialogWizPage class derived for Repository Config Wizard query page dialog window
///
class OmUiWizRepQry : public OmDialogWizPage
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiWizRepQry(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiWizRepQry();

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

    /// \brief Last repository query result
    ///
    /// Returns the last repository query result
    ///
    /// \return Query result.
    ///
    OmResult queryResult() const {
      return this->_query_result;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmNetRepo*          _NetRepo;

    OmResult            _query_result;

    void                _query_abort();

    void                _query_start();

    void*               _query_hth;

    void*               _query_hwo;

    static DWORD WINAPI _query_run_fn(void*);

    static VOID WINAPI  _query_end_fn(void*,uint8_t);


    void                _onPgInit();

    void                _onPgShow();

    void                _onPgResize();

    INT_PTR             _onPgMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIWIZREPQRY_H
