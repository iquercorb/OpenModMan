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
#ifndef OMUIMGRFOOTDSC_H
#define OMUIMGRFOOTDSC_H

#include "OmDialog.h"

class OmPackage;
class OmRemote;

/// \brief Main window - Description Tab child
///
/// OmDialog class derived for Package Description tab child dialog window of
/// the Main Window Foot frame.
///
class OmUiMgrFootDsc : public OmDialog
{
  public: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiMgrFootDsc(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiMgrFootDsc();

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

    /// \brief Set package preview
    ///
    /// Defines package preview informations to show
    ///
    /// \param[in]  pPkg    : Pointer to package to view.
    ///
    void setPreview(OmPackage* pPkg);

    /// \brief Set package preview
    ///
    /// Defines package preview informations to show
    ///
    /// \param[in]  pRmt    : Pointer to remote package to view.
    ///
    void setPreview(OmRemote* pRmt);

    /// \brief Clear package preview
    ///
    /// Clears package preview informations and hide controls
    ///
    void clearPreview();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiMgr*            _pUiMgr;

    OmPackage*          _pPkg;

    OmRemote*           _pRmt;

    uint8_t*            _rtfData;

    size_t              _rtfSize;

    size_t              _rtfRead;

    size_t              _rtfWrit;

    static void         _md2Rtf_cb(const uint8_t*, unsigned, void*);

    static DWORD CALLBACK _rtf2re_cb(DWORD_PTR, LPBYTE, LONG, PLONG);

    bool                _rawDesc;

    void                _showPreview(const wstring&, const OmVersion&, const OmImage&, const wstring&);

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMGRFOOTDSC_H
