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
#ifndef OMUIPROPPSTLST_H
#define OMUIPROPPSTLST_H

#include "OmDialogPropTab.h"

#define PST_PROP_STG_INSLS  2

/// \brief Batch Properties / Install List tab child
///
/// OmDialog class derived for Batch Properties / Install List tab child dialog window
///
class OmUiPropPstLst : public OmDialogPropTab
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropPstLst(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropPstLst();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get Included list size.
    ///
    /// Get count of item in the Included List at index.
    ///
    /// \param[in]  l   : List index, corresponding to Mod Channel combo box.
    ///
    unsigned instCount(unsigned l) const {
      return this->_included[l].size();
    }

    /// \brief Get Included list size.
    ///
    /// Get count of item in the Included List at index.
    ///
    /// \param[in]  l   : List index, corresponding to Mod Channel combo box.
    /// \param[in]  v   : Value index to get in list.
    ///
    int getSetupEntry(unsigned l, unsigned v) const {
      return this->_included[l][v];
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    std::vector<std::vector<int>> _excluded;

    std::vector<std::vector<int>> _included;

    void                _includePkg();

    void                _excludePkg();

    void                _buildLbs();

    void                _onLbExclsSel();

    void                _onLbInclsSel();

    void                _onBcUpPkg();

    void                _onBcDnPkg();

    void                _onTabInit();

    void                _onTabResize();

    void                _onTabRefresh();

    INT_PTR             _onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPPSTLST_H
