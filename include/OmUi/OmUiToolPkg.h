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
/*
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
*/
  protected:

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // edited Mod Pack object
    OmModPack*          _ModPack;

    // status bar populate/refresh
    void                _status_update_filename();

    // unsaved changes managment
    bool                _has_unsaved;

    void                _set_unsaved(bool enable);

    int32_t             _ask_unsaved();

    bool                _has_changes();

    // misc
    void                _reset_controls();

    void                _check_zip_method();

    int32_t             _method_cache;

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
    void                _categ_select();

    void                _categ_changed();

    OmWString           _categ_cache;

    // thumbnail management
    void                _thumb_toggle();

    void                _reference_thumb_load();

    OmImage             _thumb_cache;

    // description management
    void                _desc_toggle();

    void                _reference_desc_load();

    void                _reference_desc_changed();

    OmWString           _desc_cache;

    // dependencies management
    void                _depend_populate();

    void                _depend_toggle();

    void                _depend_sel_changed();

    void                _depend_delete();

    void                _depend_browse();

    void                _depend_add_show(bool show);

    void                _depend_add_valid();

    OmWStringArray      _depend_cache;

/*
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
*/
    void                _onInit();

    void                _onResize();

    void                _onRefresh();

    void                _onClose();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUITOOLPKG_H
