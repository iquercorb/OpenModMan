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
#ifndef OMUIPROPLOCNET_H
#define OMUIPROPLOCNET_H

#include <OmDialog.h>

#define LOC_PROP_NET_ONUPGRADE   0
#define LOC_PROP_NET_WARNINGS    1

/// \brief Location Properties / Network tab child
///
/// OmDialog class derived for Location Properties / Network tab child dialog window
///
class OmUiPropLocNet : public OmDialog
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropLocNet(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropLocNet();
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

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    bool                _chParam[8];

    void                _onLbReplsSel();

    void                _onBcAddRepo();

    void                _onBcChkRepo();

    void                _onBcDelRepo();

    void                _onBcRadUpg();

    void                _onCkBoxWrn();

    void                _onInit();

    void                _onResize();

    void                _onRefresh();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPLOCNET_H
