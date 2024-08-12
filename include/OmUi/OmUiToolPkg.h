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

    /// \brief Set dialog init parse
    ///
    /// Set path to package to be parsed at dialog initialization.
    ///
    void setInitParse(const OmWString& path) {
      this->_init_path = path;
    }

    /// \brief Parse specified source
    ///
    /// Load and parse the specified source as Mod-package
    ///
    /// \param[in] path : Path to Mod-Package file or directory to parse
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool parseSource(const OmWString& path) {
      return this->_modpack_parse(path);
    }

  protected:

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // edited Mod Pack object
    OmModPack*          _ModPack;

    OmWString           _init_path;

    // status bar populate/refresh
    void                _status_update_filename();

    // unsaved changes managment
    bool                _has_unsaved;

    void                _set_unsaved(bool enable);

    int32_t             _ask_unsaved();

    bool                _has_changes();

    // misc
    void                _reset_controls();

    void                _ext_changed();

    void                _method_changed();

    int32_t             _method_cache;

    bool                _method_compare();

    bool                _method_unsaved;

    // package management
    void                _modpack_close();

    void                _modpack_new();

    void                _modpack_open();

    void                _modpack_build();

    void                _modpack_save();

    void                _modpack_save_as();

    bool                _modpack_parse(const OmWString& path);

    // package 'save' thread
    OmWString           _modpack_save_path;

    int32_t             _modpack_save_abort;

    void*               _modpack_save_hth;

    void*               _modpack_save_hwo;

    void*               _modpack_save_hdp;

    OmWString           _modpack_save_str;

    static DWORD WINAPI _modpack_save_run_fn(void*);

    static bool         _modpack_save_progress_fn(void*,size_t,size_t,uint64_t);

    static bool         _modpack_save_compress_fn(void*,size_t,size_t,uint64_t);

    static VOID WINAPI  _modpack_save_end_fn(void*,uint8_t);

    // filename management
    void                _name_compose();

    // content management
    void                _content_resize();

    void                _content_selchg();

    void                _content_populate();

    OmModEntryArray     _content_cache;

    // thumbnail management
    bool                _categ_compare();

    void                _categ_changed();

    void                _categ_select();

    bool                _categ_unsaved;

    // thumbnail management
    bool                _thumb_compare();

    void                _thumb_toggle();

    void                _thumb_load();

    bool                _thumb_unsaved;

    OmImage             _thumb_image;

    // description management
    bool                _desc_compare();

    void                _desc_changed();

    void                _desc_toggle();

    void                _desc_load();

    bool                _desc_unsaved;

    // dependencies management
    bool                _depend_compare();

    void                _depend_populate();

    void                _depend_toggle();

    void                _depend_selchg();

    void                _depend_del();

    void                _depend_browse();

    void                _depend_add_show(bool show);

    void                _depend_add_valid();

    bool                _depend_unsaved;

    // common dialog process
    void                _onInit();

    void                _onResize();

    void                _onRefresh();

    void                _onClose();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUITOOLPKG_H
