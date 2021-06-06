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
#ifndef OMUIADDREP_H
#define OMUIADDREP_H

#include "OmDialog.h"

class OmLocation;

/// \brief Add Repository dialog
///
/// OmDialog class derived for Add Repository dialog window
///
class OmUiAddRep : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiAddRep(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiAddRep();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Location.
    ///
    /// Returns associated Location object previously defined
    /// via OmUiAddLoc.setLocation
    ///
    /// \return Associated Location or nullptr if none.
    ///
    OmLocation* location() const {
      return _location;
    }

    /// \brief Set associated Location.
    ///
    /// Define the associated Location, which the dialog is intended to
    /// work with and on. A valid Location must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  pCtx  : Location object to associate.
    ///
    void setLocation(OmLocation* pLoc) {
      _location = pLoc;
    }

  protected:

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmLocation*         _location;

    HFONT               _hFtMonos;

    HFONT               _hFtHeavy;

    int                 _check;

    void                _log(const wstring& log);

    void                _repoChk();

    bool                _apply();

    void                _onInit();

    void                _onResize();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIADDREP_H
