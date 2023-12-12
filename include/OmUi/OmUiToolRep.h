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
//#include "OmImage.h"
#include "OmXmlConf.h"

class OmNetRepo;

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

    // XML definition
    OmXmlConf           _xml;

    OmNetRepo*          _NetRepo;


    OmWString           _path;

    //
    void                _repo_init();

    void                _repo_open();

    void                _repo_close();

    void                _repo_save();

    void                _repo_save_as();

    void                _repo_save_title();

    void                _repo_save_downpath();

    bool                _repo_add_ref(const OmWString& path, bool select = false);

    bool                _repo_del_ref();

    void                _ref_selected();

    void                _ref_url_save();

    void                _ref_desc_load();

    void                _ref_desc_save();

    void                _ref_thumb_set();

    void                _ref_thumb_del();

    void                _ref_depends_get(const OmWString&, OmWStringArray*);

    void                _ref_depends_check();


    void                _status_update_filename();

    void                _status_update_references();


    void                _browse_add_files();

    void                _browse_add_directory();


    void                _set_unsaved(bool enable);

    int32_t             _ask_unsaved();

    bool                _has_unsaved;


    // add directory thread stuff
    OmWStringQueue      _repo_addlist_queue;

    int32_t             _repo_addlist_abort;

    void*               _repo_addlist_hth;

    void*               _repo_addlist_hwo;

    void                _repo_addlist_start(const OmWStringArray&);

    void*               _repo_addlist_hpd;

    static DWORD WINAPI _repo_addlist_run_fn(void*);

    static VOID WINAPI  _repo_addlist_end_fn(void*,uint8_t);


    void                _onClose();

    void                _onInit();

    void                _onResize();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUITOOLREP_H
