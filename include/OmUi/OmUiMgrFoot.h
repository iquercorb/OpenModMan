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
#ifndef OMUIMGRFOOT_H
#define OMUIMGRFOOT_H

#include "OmDialog.h"

class OmUiMgr;
class OmPackage;
class OmRemote;

/// \brief Window Secondary (bottom) Frame.
///
/// OmDialog class derived for the main window bottom frame which contain
/// package snapshot and description view.
///
class OmUiMgrFoot : public OmDialog
{
  public: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiMgrFoot(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiMgrFoot();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Set dialog install mode
    ///
    /// Enable or disable the dialog install mode.
    ///
    /// The install mode is a modal-kind emulation for threaded operations,
    /// it disables (almost) all controls of the dialog and its children
    /// to prevent user to interact with elements while a threaded process
    /// is running.
    ///
    /// \param[in]  enable  : Enable or disable freeze mode.
    ///
    void freeze(bool enable);

    /// \brief Set dialog safe mode
    ///
    /// Enables or disable the dialog safe mode.
    ///
    /// The safe mode is used to operate modifications on sensitive
    /// or monitored elements such as deleting or moving Location in
    /// order to prevent conflicts or crash during process.
    ///
    /// \param[in]  enable  : Enable or disable safe mode.
    ///
    void safemode(bool enable);

    void viewPackage(const wstring& name, const OmVersion& vers, const OmImage& snap, const wstring& desc);

    void viewReset();


  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiMgr*            _pUiMgr;

    uint8_t*            _rtfData;

    size_t              _rtfSize;

    size_t              _rtfRead;

    size_t              _rtfWrit;

    static void         _md2Rtf_cb(const uint8_t*, unsigned, void*);

    static DWORD CALLBACK _rtf2re_cb(DWORD_PTR, LPBYTE, LONG, PLONG);

    bool                _rawDesc;

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMGRFOOT_H
