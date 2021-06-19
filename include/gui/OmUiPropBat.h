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
#ifndef OMUIPROPBAT_H
#define OMUIPROPBAT_H

#include "OmDialogProp.h"

class OmBatch;

/// \brief Batch Properties parent dialog
///
/// OmDialogProp class derived for Batch Properties parent dialog window.
///
class OmUiPropBat : public OmDialogProp
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropBat(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropBat();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Batch.
    ///
    /// Returns associated Batch object previously defined
    /// via OmUiPropLoc.setContext
    ///
    /// \return Associated Batch or nullptr if none.
    ///
    OmBatch* batCur() const {
      return _pBat;
    }

    /// \brief Set associated Batch.
    ///
    /// Define the associated Batch, which the dialog is intended to
    /// work with and on. A valid Batch must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  bat   : Batch object to associate.
    ///
    void batSet(OmBatch* bat) {
      _pBat = bat;
    }

    /// \brief Check for properties changes
    ///
    /// Checks whether dialog's dedicated properties changed then perform
    /// proper operations, such as enabling or disabling the Apply button.
    ///
    bool checkChanges();

    /// \brief Apply properties changes
    ///
    /// Retrieve dialog's dedicated properties then apply changes.
    ///
    bool applyChanges();


  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmBatch*            _pBat;
};

#endif // OMUIPROPBAT_H
