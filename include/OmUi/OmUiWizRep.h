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
#ifndef OMUIWIZREP_H
#define OMUIWIZREP_H

#include "OmDialogWiz.h"

class OmModChan;

/// \brief New Repository Wizard parent dialog
///
/// OmDialogWiz class derived for New Repository Wizard parent dialog window
///
class OmUiWizRep : public OmDialogWiz
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiWizRep(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiWizRep();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Set Mod Hub
    ///
    /// Set related Mod Hub where to create Channel.
    ///
    /// \param[in] ModHub : Pointer to Mod Hub object
    ///
    void setModChan(OmModChan* ModChan) {
      this->_ModChan = ModChan;
    }

    /// \brief Get Mod Hub
    ///
    /// Get related Mod Hub where to create Channel.
    ///
    /// \return Pointer to Mod Hub object.
    ///
    OmModChan* ModChan() const {
      return this->_ModChan;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmModChan*          _ModChan;

    void                _onWizInit();

    void                _onWizFinish();
};

#endif // OMUIWIZREP_H
