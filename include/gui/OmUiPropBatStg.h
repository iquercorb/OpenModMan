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
#ifndef OMUIPROPBATSTG_H
#define OMUIPROPBATSTG_H

#include "OmDialog.h"

#define BAT_PROP_STG_TITLE  0
#define BAT_PROP_STG_INSLS  1

/// \brief Batch Properties / Setting tab child
///
/// OmDialog class derived for Batch Properties / Settings tab child dialog window
///
class OmUiPropBatStg : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropBatStg(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropBatStg();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Check changed parameter
    ///
    /// Checks whether the specified dialog related parameter has been
    /// marked as changed, meaning it was modified by user.
    ///
    /// \param[in]  i   : Dialog related parameter to check
    ///
    /// \return True if the specified parameter is marked as changed,
    ///         false otherwise
    ///
    bool hasChParam(unsigned i) const {
      return _chParam[i];
    }

    /// \brief Set changed parameter
    ///
    /// Marks the specified dialog related parameter as changed or unchanged.
    ///
    /// \param[in]  i   : Dialog related parameter to mark
    /// \param[in]  en  : Boolean value to enable or disable
    ///
    void setChParam(unsigned i, bool en);

    /// \brief Get Included list size.
    ///
    /// Get count of item in the Included List at index.
    ///
    /// \param[in]  l   : List index, corresponding to Location combo box.
    ///
    unsigned getIncLsSize(unsigned l) const {
      return this->_incLs[l].size();
    }

    /// \brief Get Included list size.
    ///
    /// Get count of item in the Included List at index.
    ///
    /// \param[in]  l   : List index, corresponding to Location combo box.
    /// \param[in]  v   : Value index to get in list.
    ///
    int getIncLsValue(unsigned l, unsigned v) const {
      return this->_incLs[l][v];
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    bool                _chParam[8];

    vector<vector<int>> _excLs;

    vector<vector<int>> _incLs;

    HBITMAP             _hBmBcUp;

    HBITMAP             _hBmBcDn;

    void                _rebuildPkgLb();

    void                _qucikFromCur();

    void                _upPkg();

    void                _dnPkg();

    void                _addPkg();

    void                _remPkg();

    void                _onInit();

    void                _onResize();

    void                _onRefresh();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPBATSTG_H
