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

/// \brief Mod Preset Properties / Install List tab child
///
/// OmDialog class derived for Mod Preset Properties / Install List tab child dialog window
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

    void                _install_include();

    void                _install_exclude();

    void                _lb_populate();

    void                _lb_exc_on_selchg();

    void                _lb_inc_on_selchg();

    void                _install_list_up();

    void                _install_list_dn();

    void                _onTbInit();

    void                _onTbResize();

    void                _onTbRefresh();

    INT_PTR             _onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPPSTLST_H
