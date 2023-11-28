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

/// \brief Mod Channel Properties: "Mods Repositories" tab
///
/// OmDialogPropTab class derived for "Mods Repositories" tab child dialog window
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

    void                _query_start(size_t i);

    static void         _query_result_fn(void*, OmResult, uint64_t);

    void                _onLbReplsSel();

    void                _onBcAddRepo();

    void                _onBcQryRepo();

    void                _onBcDelRepo();

    void                _onBcRadUpg();

    void                _onCkBoxWrn();

    void                _onTabInit();

    void                _onTabResize();

    void                _onTabRefresh();

    INT_PTR             _onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPCHNNET_H
