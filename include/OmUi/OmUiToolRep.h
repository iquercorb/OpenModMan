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
#ifndef OMUITOOLREP_H
#define OMUITOOLREP_H

#include "OmDialog.h"
#include "OmImage.h"
#include "OmXmlConf.h"

/// \brief Repository Editor tool dialog
///
/// OmDialog class derived for Repository Editor tool dialog window
///
class OmUiToolRep : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiToolRep(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiToolRep();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmXmlConf           _xmlconf;

    OmXmlNode           _rmtCur;

    bool                _unsaved;

    void                _repInit();

    bool                _repOpen(const OmWString& path);

    OmXmlNode           _rmtGet(const OmWString& ident);

    bool                _rmtAdd(const OmWString& path);

    bool                _rmtRem(const OmWString& ident);

    bool                _rmtSel(const OmWString& ident);

    int                 _rmtGetDeps(OmWStringArray& miss_list, const OmWString& ident);

    void                _addDir_init(const OmWString& path);

    void                _addDir_stop();

    void*               _addDir_hth;

    static DWORD WINAPI _addDir_fth(void*);

    OmWString           _addDir_path;

    void                _onLbPkglsSel();

    void                _onBcNew();

    void                _onBcOpen();

    bool                _onBcBrwPkg();

    bool                _onBcBrwDir();

    void                _onBcRemPkg();

    void                _onBcSavUrl();

    void                _onBcChkDeps();

    bool                _onBcBrwSnap();

    void                _onBcDelSnap();

    void                _onBcBrwDesc();

    void                _onBcSavDesc();

    void                _onBcSave();

    void                _onBcClose();

    void                _onInit();

    void                _onResize();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUITOOLREP_H
