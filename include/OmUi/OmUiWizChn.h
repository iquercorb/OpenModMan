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
#ifndef OMUIWIZCHN_H
#define OMUIWIZCHN_H

#include "OmDialogWiz.h"

class OmModHub;

/// \brief New Mod Channel Wizard parent dialog
///
/// OmDialogWiz class derived for New Mod Channel Wizard parent dialog window
///
class OmUiWizChn : public OmDialogWiz
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiWizChn(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiWizChn();

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
    void setModHub(OmModHub* ModHub) {
      this->_ModHub = ModHub;
    }

    /// \brief Get Mod Hub
    ///
    /// Get related Mod Hub where to create Channel.
    ///
    /// \return Pointer to Mod Hub object.
    ///
    OmModHub* ModHub() const {
      return this->_ModHub;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmModHub*           _ModHub;

    void                _onWizFinish();
};

#endif // OMUIWIZCHN_H
