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
#ifndef OMUIPROPCHNNET_H
#define OMUIPROPCHNNET_H

#include <OmDialogPropTab.h>

#define CHN_PROP_NET_ONUPGRADE   0
#define CHN_PROP_NET_WARNINGS    1

/// \brief Mod Channel Properties: "Network Repositories" tab
///
/// OmDialogPropTab class derived for "Network Repositories" tab child dialog window
///
class OmUiPropChnNet : public OmDialogPropTab
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropChnNet(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropChnNet();
    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _query_start(size_t);

    static void         _query_result_fn(void*, OmResult, uint64_t);

    void                _lb_rep_on_selchg();

    void                _bc_rpadd_pressed();

    void                _bc_rpqry_pressed();

    void                _bc_rpdel_pressed();

    void                _onTbInit();

    void                _onTbResize();

    void                _onTbRefresh();

    INT_PTR             _onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPCHNNET_H
