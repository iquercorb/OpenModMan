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
#ifndef OMUIADDREP_H
#define OMUIADDREP_H

#include "OmDialog.h"
#include "OmConnect.h"

class OmModChan;

/// \brief Add Repository dialog
///
/// OmDialog class derived for Add Repository dialog window
///
class OmUiAddRep : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiAddRep(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiAddRep();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Mod Channel.
    ///
    /// Returns associated Mod Channel object
    ///
    /// \return Associated Mod Channel or nullptr if none.
    ///
    OmModChan* ModChan() const {
      return this->_ModChan;
    }

    /// \brief Set associated Mod Channel.
    ///
    /// Define the associated Mod Channel, which the dialog is intended to
    /// work with and on. A valid Mod Channel must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  ModChan  : Mod Channel object to associate.
    ///
    void setModChan(OmModChan* ModChan) {
      this->_ModChan = ModChan;
    }

  protected:

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmModChan*          _ModChan;

    OmConnect           _connect;

    OmResult            _qry_result;

    void                _qry_addlog(const OmWString& log);

    static void         _qry_reponse_fn(void* ptr, uint8_t* buf, uint64_t len, uint64_t code);

    void                _onBcChk();

    bool                _onBcOk();

    void                _onInit();

    void                _onResize();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIADDREP_H
