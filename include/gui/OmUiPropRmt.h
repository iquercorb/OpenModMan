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
#ifndef OMUIPROPRMT_H
#define OMUIPROPRMT_H

#include "OmDialogProp.h"

class OmRemote;

/// \brief Remote package Details dialog
///
/// OmDialogProp class derived for Remote package Details dialog window
///
class OmUiPropRmt : public OmDialogProp
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropRmt(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropRmt();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Package.
    ///
    /// Returns associated Package object previously defined
    /// via OmUiPropLoc.setPackage
    ///
    /// \return Associated Package or nullptr if none.
    ///
    OmRemote* rmtCur() const {
      return _pRmt;
    }
    /// \brief Set associated Package.
    ///
    /// Define the associated Package, which the dialog is intended to
    /// work with and on. A valid Package must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  pRmt  : Package object to associate.
    ///
    void rmtSet(OmRemote* pRmt) {
      _pRmt = pRmt;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmRemote*          _pRmt;
};

#endif // OMUIPROPRMT_H
