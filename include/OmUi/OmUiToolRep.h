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

    /// \brief Load repository
    ///
    /// Loads the specified repository definition
    ///
    /// \param[in] path : Path to repository definition
    ///
    /// \return true if succeed, false otherwise
    ///
    bool loadRepository(const OmWString& path) {
      return this->_repository_load(path);
    }

    /// \brief Set dialog init load
    ///
    /// Set path to repository definition to be loaded at dialog initialization.
    ///
    void setInitLoad(const OmWString& path) {
      this->_init_path = path;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // edited repository object
    OmNetRepo*          _NetRepo;

    OmWString           _init_path;

    // status bar populate/refresh
    void                _status_update_filename();

    void                _status_update_references();

    // unsaved changes managment
    bool                _has_unsaved;

    void                _set_unsaved(bool enable);

    int32_t             _ask_unsaved();

    // misc
    void                _reset_controls();

    // repository edition
    void                _repository_init();

    bool                _repository_load(const OmWString& path);

    bool                _repository_save(const OmWString& path);

    void                _repository_close();

    void                _repository_open();

    void                _repository_save();

    void                _repository_save_as();

    // repository title
    void                _repository_title_save();

    void                _repository_downpath_save();

    bool                _repository_reference_add(const OmWString& path, bool select = false);

    bool                _repository_reference_del();

    // reference edition
    void                _reflist_resize();

    void                _reflist_populate();

    void                _reflist_selchg(int32_t item = -1, bool selected = true);

    int32_t             _reflist_lastsel;

    void                _reflist_add_files();

    void                _reflist_add_directory();

    void                _reference_url_toggle();

    void                _reference_url_changed(int32_t item = -1);

    void                _reference_desc_load();

    void                _reference_desc_changed(int32_t item = -1);

    void                _reference_thumb_load();

    void                _reference_thumb_delete();

    void                _reference_deps_get(const OmWString&, OmWStringArray*);

    void                _reference_deps_check();


    // add reference Mod parsing thread
    OmWStringQueue      _reflist_add_queue;

    int32_t             _reflist_add_abort;

    void*               _reflist_add_hth;

    void*               _reflist_add_hwo;

    void                _reflist_add_start(const OmWStringArray&);

    void*               _reflist_add_hpd;

    static DWORD WINAPI _reflist_add_run_fn(void*);

    static VOID WINAPI  _reflist_add_end_fn(void*,uint8_t);

    // common dialog functions
    void                _onClose();

    void                _onInit();

    void                _onResize();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUITOOLREP_H
