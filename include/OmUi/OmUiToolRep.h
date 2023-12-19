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

    bool                _has_changes();

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
    bool                _title_unsaved;

    bool                _title_compare();

    void                _title_changed();

    // repository download path
    bool                _downpath_unsaved;

    bool                _downpath_compare();

    void                _downpath_changed();

    bool                _downpath_valid();

    // reference edition
    bool                _refs_unsaved;

    void                _refs_resize();

    void                _refs_populate();

    void                _refs_selchg(int32_t item = -1, bool selected = true);

    bool                _refs_add(const OmWString& path, bool select = false);

    bool                _refs_del();

    void                _refs_append_files();

    void                _refs_append_directory();

    // add reference Mod parsing thread
    OmWStringQueue      _append_queue;

    int32_t             _append_abort;

    void*               _append_hth;

    void*               _append_hwo;

    void                _append_start(const OmWStringArray&);

    void*               _append_hpd;

    static DWORD WINAPI _append_run_fn(void*);

    static VOID WINAPI  _append_end_fn(void*,uint8_t);

    // reference state
    bool                _ref_save(int32_t index = -1);

    bool                _ref_has_changes();

    // reference URL
    OmWString           _ref_url_cache;

    bool                _ref_url_unsaved;

    bool                _ref_url_compare();

    void                _ref_url_toggle();

    void                _ref_url_changed();

    bool                _ref_url_valid();

    void                _ref_url_alert();

    // reference thumbnail
    OmImage             _ref_thumb_cache;

    bool                _ref_thumb_unsaved;

    bool                _ref_thumb_compare();

    void                _ref_thumb_load();

    void                _ref_thumb_delete();

    OmImage             _ref_thumb_image;

    // reference description
    OmWString           _ref_desc_cache;

    bool                _ref_desc_unsaved;

    bool                _ref_desc_compare();

    void                _ref_desc_load();

    void                _ref_desc_changed();

    // reference depends
    void                _ref_deps_get(const OmWString&, OmWStringArray*);

    void                _ref_deps_check();

    // common dialog functions
    void                _onClose();

    void                _onInit();

    void                _onResize();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUITOOLREP_H
