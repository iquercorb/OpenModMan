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

#ifndef OMUIPROPCTXLOCS_H
#define OMUIPROPCTXLOCS_H

#include "OmDialog.h"

#define CTX_PROP_LOC_ORDER   0

/// \brief Context Properties Locations Tab
///
/// OmDialog class derived for Context Properties Locations Tab_Control child
/// dialog window
///
class OmUiPropCtxLoc : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropCtxLoc(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropCtxLoc();

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

    void                _locationUp();

    void                _locationDn();

    void                _remLocation_init();

    void                _remLocation_stop();

    void*               _remLocation_hth;

    static DWORD WINAPI _remLocation_fth(void*);

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPCTXLOCS_H
