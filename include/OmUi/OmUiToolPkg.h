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
#ifndef OMUITOOLPKG_H
#define OMUITOOLPKG_H

#include "OmDialog.h"
#include "OmModPack.h"

/// \brief Package Editor tool dialog
///
/// OmDialog class derived for Package Editor tool dialog window
///
class OmUiToolPkg : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiToolPkg(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiToolPkg();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Set initial source path.
    ///
    /// Set path to package source to be parsed at dialog initialization.
    ///
    /// \param[in]  path    : Path to source to parse.
    ///
    void setSource(const OmWString& path) {
      this->_initial_src = path;
    }

    /// \brief Select source path.
    ///
    /// Force to load the specified path as source.
    ///
    /// \param[in]  path    : Path to source to parse.
    ///
    void selectSource(const OmWString& path);

  protected:

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmModPack           _ModPack;

    bool                _unsaved;

    OmWString           _initial_src;

    bool                _parseSrc(const OmWString& path);

    void                _freeze(bool freeze);

    void                _save_init();

    void                _save_stop();

    void*               _save_hth;

    static bool         _save_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data);

    static DWORD WINAPI _save_fth(void*);

    bool                _save_abort;

    void                _onBcRadSrc();

    bool                _onBcBrwDir(const wchar_t* path = nullptr);

    bool                _onBcBrwPkg(const wchar_t* path = nullptr);

    void                _onNameChange();

    void                _onBcBrwDest();

    void                _onLbDpnlsSel();

    void                _onCbCatSel();

    void                _onCkBoxDep();

    void                _onBcAddDep();

    void                _onBcDelDep();

    void                _onCkBoxSnap();

    bool                _onBcBrwSnap();

    void                _onCkBoxDesc();

    bool                _onBcBrwDesc();

    void                _onBcSave();

    void                _onInit();

    void                _onResize();

    void                _onRefresh();

    void                _onClose();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUITOOLPKG_H
