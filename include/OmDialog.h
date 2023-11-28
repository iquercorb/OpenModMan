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
#ifndef OMDIALOG_H
#define OMDIALOG_H

#include "OmBase.h"
#include "OmBaseWin.h"

class OmModMan;

/// \brief Dialog window.
///
/// Base class for dialog window interface. This object is destined to serve as
/// base for a derived class with proper virtual functions to suits to and
/// interfaces a specific dialog window.
///
class OmDialog
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmDialog(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    virtual ~OmDialog();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    virtual long id() const;

    /// \brief Get Instance handle.
    ///
    /// Returns Instance WinAPI handle.
    ///
    /// \return Instance WinAPI handle.
    ///
    HINSTANCE hins() const {
      return _hins;
    }

    /// \brief Get window handle.
    ///
    /// Returns dialog window WinAPI handle.
    ///
    /// \return Window WinAPI Handle.
    ///
    HWND hwnd() const {
      return _hwnd;
    }

    /// \brief Get parent.
    ///
    /// Returns the parent dialog.
    ///
    /// \return Parent dialog.
    ///
    OmDialog* parent() const {
      return _parent;
    }

    /// \brief Get child dialog.
    ///
    /// Returns child dialog at specified index.
    ///
    /// \param[in]  i       : Index of child dialog to get.
    ///
    /// \return Child dialog.
    ///
    OmDialog* child(unsigned i) const {
      return _child[i];
    }

    /// \brief Get child dialog by resource id.
    ///
    /// Returns child dialog with the specified resource id.
    ///
    /// \param[in]  i       : Resource id of child dialog to get.
    ///
    /// \return Child dialog or nullptr if not found.
    ///
    OmDialog* childById(long id) const;

    /// \brief Get sibling dialog by resource id.
    ///
    /// Returns sibling dialog with the specified resource id.
    ///
    /// \param[in]  i       : Resource id of sibling dialog to get.
    ///
    /// \return Sibling dialog or nullptr if not found.
    ///
    OmDialog* siblingById(long id) const;

    /// \brief Get root.
    ///
    /// Returns the root parent dialog of the hierarchy.
    ///
    /// \return Root dialog.
    ///
    OmDialog* root();

    /// \brief Get custom data.
    ///
    /// Returns dialog custom data.
    ///
    /// \return Dialog custom data.
    ///
    void* data() const {
      return _data;
    }

    /// \brief Dialog menu.
    ///
    /// Returns dialog associated menu if exists.
    ///
    /// \return Dialog menu or nullptr.
    ///
    HMENU menu() const {
      return _menu;
    }

    /// \brief Defer resize handle.
    ///
    /// Returns handle dialog defer resize structure.
    ///
    /// \return Dialog defer resize handle.
    ///
    HDWP hdwp() const {
      return _hdwp;
    }

    /// \brief Open window.
    ///
    /// Create the dialog window.
    ///
    /// \param[in]  show    : Show the dialog window once created.
    ///
    void open(bool show = true);

    /// \brief Open modeless window.
    ///
    /// Create the dialog window as modeless.
    ///
    /// \param[in]  show    : Show the dialog window once created.
    ///
    void modeless(bool show = true);

    /// \brief Open registered window.
    ///
    /// Create the dialog window with registered class, the
    /// template must have the proper CLASS attributes defined.
    ///
    /// \param[in]  classname : Window class name as defined in template.
    /// \param[in]  show      : Show the dialog window once created.
    ///
    void registered(const char* classname, bool show = true);

    /// \brief Show window.
    ///
    /// Show the dialog window.
    ///
    void show() const {
      ShowWindow(_hwnd, SW_SHOW);
    }

    /// \brief Hide window.
    ///
    /// Hide the dialog window.
    ///
    void hide() const {
      ShowWindow(_hwnd, SW_HIDE);
    }

    /// \brief Check visibility.
    ///
    /// Checks whether dialog window is visible (shown)
    ///
    /// \return True if dialog is currently visible, false otherwise
    ///
    bool visible() const {
      return IsWindowVisible(_hwnd);
    }

    /// \brief Check enabled.
    ///
    /// Checks whether dialog window is enabled
    ///
    /// \return True if dialog is currently enabled, false otherwise
    ///
    bool enabled() const {
      return IsWindowEnabled(_hwnd);
    }

    /// \brief Check active.
    ///
    /// Checks whether dialog window is currently active
    ///
    /// \return True if dialog is currently active, false otherwise
    ///
    bool active() const {
      return _active;
    }

    /// \brief Check minimized.
    ///
    /// Checks whether dialog window is currently minimized.
    ///
    /// \return True if dialog is currently minimized, false otherwise
    ///
    bool minimized() const {
      return _minimized;
    }

    /// \brief Refresh window.
    ///
    /// Refresh the dialog window.
    ///
    void refresh();

    /// \brief Close window.
    ///
    /// Close and quit the dialog window.
    ///
    void quit();

    /// \brief Add child dialog.
    ///
    /// Add specified dialog as child.
    ///
    /// \param[in]  dialog  : Dialog to add as child.
    ///
    void addChild(OmDialog* dialog);

    /// \brief Set parent dialog.
    ///
    /// Set the specified dialog as parent.
    ///
    /// \param[in]  dialog  : Dialog to set as parent.
    ///
    void setParent(OmDialog* dialog);

    /// \brief Set dialog accelerator table.
    ///
    /// Defines dialog accelerator table.
    ///
    /// \param[in] id       : Accelerator table resource id
    ///
    void setAccel(long id);

    /// \brief Set dialog custom data.
    ///
    /// Defines dialog custom data, such as pointer to application object or
    /// structure. Custom data is set recursively to all children dialogs.
    ///
    /// \param[in] data     : Custom data to set.
    ///
    void setData(void* data);

    /// \brief Set dialog style.
    ///
    /// Defines dialog window standard and extended style.
    ///
    /// \param[in] style    : Style to apply
    /// \param[in] exstyle  : Extended style to apply
    ///
    void setStyle(long style, long exstyle);

    /// \brief Send message to dialog.
    ///
    /// Send the message to dialog, the message is transmitted recursively
    /// to all children dialogs.
    ///
    /// \param[in]  msg     : Pointer to Message to transmit.
    ///
    /// \return True if message was destined to this dialog window or one of its
    /// children, false otherwise.
    ///
    bool sendMessage(MSG* msg) const;

    /// \brief Post message to dialog.
    ///
    /// Places (posts) a message in the message queue of the dialog.
    ///
    /// \param[in]  msg     : Message to be posted
    /// \param[in]  wParam  : Additional message-specific information.
    /// \param[in]  lParam  : Additional message-specific information.
    ///
    /// \return True if succeed, false otherwise.
    ///
    bool postMessage(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0) const {
      return PostMessageW(_hwnd, msg, wParam, lParam);
    }

    /// \brief Enter thread message loop.
    ///
    /// Enters into window message loop.
    ///
    void loopMessage() const;

    /// \brief Dialog window X position.
    ///
    /// Returns dialog window X position in pixels.
    ///
    /// \return Window X position in pixels.
    ///
    long x() const {
      return this->_recw[0];
    }

    /// \brief Dialog window Y position.
    ///
    /// Returns dialog window Y position in pixels.
    ///
    /// \return Window X position in pixels.
    ///
    long y() const {
      return this->_recw[1];
    }

    /// \brief Dialog window width.
    ///
    /// Returns dialog window width in pixels.
    ///
    /// \return Window width in pixels.
    ///
    long width() const {
      return this->_size[0];
    }

    /// \brief Dialog window height.
    ///
    /// Returns dialog window height in pixels.
    ///
    /// \return Window height in pixels.
    ///
    long height() const {
      return this->_size[1];
    }

    /// \brief Dialog client unit width.
    ///
    /// Returns dialog client width in base unit.
    ///
    /// \return Window width in base unit.
    ///
    int cliUnitX() const {
      return this->_usize[0];
    }

    /// \brief Dialog client unit height.
    ///
    /// Returns dialog client height in base unit.
    ///
    /// \return Window height in base unit.
    ///
    int cliUnitY() const {
      return this->_usize[1];
    }

    /// \brief Dialog client width.
    ///
    /// Returns dialog client width in pixels.
    ///
    /// \return Client area width in pixels.
    ///
    long cliWidth() const {
      return this->_recc[2];
    }

    /// \brief Dialog client height.
    ///
    /// Returns dialog client height in pixels.
    ///
    /// \return Client area height in pixels.
    ///
    long cliHeight() const {
      return this->_recc[3];
    }

    /// \brief Dialog horizontal base unit.
    ///
    /// Returns dialog horizontal base unit value.
    ///
    /// \return Dialog horizontal base unit.
    ///
    int ubaseX() const {
      return this->_ubase[0];
    }

    /// \brief Dialog vertical base unit.
    ///
    /// Returns dialog vertical base unit value.
    ///
    /// \return Dialog vertical base unit.
    ///
    int ubaseY() const {
      return this->_ubase[1];
    }

    /// \brief Set dialog caption
    ///
    /// Sets the dialog title caption text.
    ///
    /// \param[in]  caption : Caption text to set.
    ///
    void setCaption(const OmWString& caption) const {
      SetWindowTextW(_hwnd, caption.c_str());
    }

    /// \brief Set dialog icon
    ///
    /// Sets the dialog title icon.
    ///
    /// \param[in]  big     : Big icon to set.
    /// \param[in]  small   : Small icon to set.
    ///
    void setIcon(HICON big, HICON small) const {
      SendMessageW(_hwnd,WM_SETICON,ICON_SMALL, reinterpret_cast<LPARAM>(small));
      SendMessageW(_hwnd,WM_SETICON,ICON_BIG,   reinterpret_cast<LPARAM>(big));
    }

    /// \brief Get control handle
    ///
    /// Returns dialog control handle with the specified ID.
    ///
    /// \param[in]  id  : Control resource ID to retrieve handle.
    ///
    /// \return Dialog control handle.
    ///
    HWND getItem(unsigned id) {
      return GetDlgItem(this->_hwnd, id);
    }

    /// \brief Redraw item region
    ///
    /// Send a redraw message to the specified item.
    ///
    /// \param[in]  id    : Control resource ID to set text for.
    /// \param[in]  rect  : Pointer to RECT struct for redraw region.
    /// \param[in]  falgs : Redraw flags.
    ///
    /// \return True if succeed, false otherwise
    ///
    bool redrawItem(unsigned id, RECT* rect, uint32_t falgs) const;

    /// \brief Set control text
    ///
    /// Define inner text for the specified dialog control.
    ///
    /// \param[in]  id    : Control resource ID to set text for.
    /// \param[in]  text  : Text to set.
    ///
    void setItemText(unsigned id, const OmWString& text) const;

    /// \brief Get control text
    ///
    /// Retrieve inner text for the specified dialog control.
    ///
    /// \param[in]  id    : Control resource ID to set text for.
    /// \param[in]  text  : Wide char string to get text.
    ///
    /// \return Count of written characters.
    ///
    size_t getItemText(unsigned id, OmWString& text) const;

    /// \brief Get ListBox control text
    ///
    /// Retrieve text at specified index of the specified ListBox control.
    ///
    /// \param[in]  id    : ListBox resource ID to set text for.
    /// \param[in]  i     : ListBox entry index to get text.
    /// \param[in]  text  : Wide char string to get text.
    ///
    /// \return Count of written characters.
    ///
    size_t getLbText(unsigned id, unsigned i, OmWString& text) const;

    /// \brief Find listView param
    ///
    /// Retrieve listView item index that has the specified param value.
    ///
    /// \param[in]  id    : listView resource ID to set text for.
    /// \param[in]  param : LPARAM value to search.
    ///
    /// \return Count of written characters.
    ///
    size_t findLvParam(unsigned id, uint64_t param) const;

    /// \brief Get listView subitem rectangle
    ///
    /// Get the listView bounding rectangle of the specified subitem of item.
    ///
    /// \param[in]  id      : listView resource ID to set text for.
    /// \param[in]  item    : ListView item (raw) index
    /// \param[in]  subitem : ListView subitem (col) index
    /// \param[out] rect    : Pointer to RECT struct to be filled.
    ///
    /// \return Nonzero if successful, or zero otherwise.
    ///
    size_t getLvSubRect(unsigned id, uint64_t item, uint32_t subitem, RECT* rect) const;

    /// \brief Set static control image
    ///
    /// Define image for the specified dialog static control.
    ///
    /// \param[in]  id    : Static control resource ID to set image to.
    /// \param[in]  image : Image to set.
    ///
    /// \return Previously associated image to control, or nullptr.
    ///
    HBITMAP setStImage(unsigned id, HBITMAP image) const;

    /// \brief Set static control icon
    ///
    /// Define icon for the specified dialog static control.
    ///
    /// \param[in]  id    : Static control resource ID to set image to.
    /// \param[in]  icon  : Icon to set.
    ///
    /// \return Previously associated icon to control, or nullptr.
    ///
    HICON setStIcon(unsigned id, HICON icon) const;

    /// \brief Set button control image
    ///
    /// Define image for the specified dialog button control.
    ///
    /// \param[in]  id    : Button control resource ID to set image to.
    /// \param[in]  image : Image to set.
    ///
    /// \return Previously associated image to control, or nullptr.
    ///
    HBITMAP setBmImage(unsigned id, HBITMAP image) const;

    /// \brief Set button control icon
    ///
    /// Define icon for the specified dialog button control.
    ///
    /// \param[in]  id    : Button control resource ID to set image to.
    /// \param[in]  icon  : Icon to set.
    ///
    /// \return Previously associated image to control, or nullptr.
    ///
    HICON setBmIcon(unsigned id, HICON icon) const;

    /// \brief Send message to control
    ///
    /// Send the message with parameters to the specified dialog control.
    ///
    /// \param[in]  id      : Control resource ID to set text for.
    /// \param[in]  uMsg    : The message to be sent.
    /// \param[in]  wParam  : Additional message-specific information.
    /// \param[in]  lParam  : Additional message-specific information.
    ///
    /// \return The return value specifies the result of the message processing.
    ///
    LRESULT msgItem(unsigned id, UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) const {
      return SendMessageW(GetDlgItem(_hwnd, id), uMsg, wParam, lParam);
    }

    /// \brief Enable or disable control
    ///
    /// Enable or disable the specified dialog control.
    ///
    /// \param[in]  id      : Control resource ID to enable or disable.
    /// \param[in]  enable  : Enable or disable.
    ///
    void enableItem(unsigned id, bool enable) const {
      EnableWindow(GetDlgItem(_hwnd, id), enable);
    }

    /// \brief Check whether control is enabled
    ///
    /// Check whether the specified dialog control is enabled.
    ///
    /// \param[in]  id      : Control resource ID to check.
    ///
    /// \return True if the control is enabled, false otherwise.
    ///
    bool itemEnabled(unsigned id) const {
      return IsWindowEnabled(GetDlgItem(_hwnd, id));
    }

    /// \brief Show or hide control
    ///
    /// Show or hide the specified dialog control.
    ///
    /// \param[in]  id      : Control resource ID to enable or disable.
    /// \param[in]  show    : Show or hide.
    ///
    void showItem(unsigned id, bool show) const {
      ShowWindow(GetDlgItem(_hwnd, id), show);
    }

    /// \brief Get menu pop-up
    ///
    /// Returns dialog main menu (menu bar) pop-up.
    ///
    /// \param[in]  pos     : Zero based position of the pop-up to get.
    ///
    /// \return Menu handle.
    ///
    HMENU getPopup(unsigned pos) const {
      return GetSubMenu(_menu, pos);
    }

    /// \brief Get pop-up menu-item
    ///
    /// Returns dialog menu pop-up menu-item.
    ///
    /// \param[in]  popup   : Zero based pop-up position to get menu-item from.
    /// \param[in]  pos     : Zero based menu-item position witin the popup.
    ///
    /// \return Menu handle.
    ///
    HMENU getPopupItem(unsigned popup, unsigned pos) const {
      return GetSubMenu(GetSubMenu(_menu, popup), pos);
    }

    /// \brief Get pop-up menu-item
    ///
    /// Returns dialog menu pop-up menu-item.
    ///
    /// \param[in]  popup   : Menu pop-up handle to get menu-item from.
    /// \param[in]  pos     : Zero based menu-item position within the pop-up.
    ///
    /// \return Menu handle.
    ///
    HMENU getPopupItem(HMENU popup, unsigned pos) const {
      return GetSubMenu(popup, pos);
    }

    /// \brief Enable or disable pop-up menu-item
    ///
    /// Enable, disable or grays dialog menu pop-up menu-item.
    ///
    /// \param[in]  popup   : Zero based pop-up position to get menu-item from.
    /// \param[in]  pos     : Zero based menu-item position within the pop-up.
    /// \param[in]  enable  : Enable flags to set.
    ///
    /// \return Menu handle.
    ///
    void setPopupItem(unsigned popup, unsigned pos, int enable) const {
      EnableMenuItem(GetSubMenu(_menu, popup), pos, MF_BYPOSITION|enable);
    }

    /// \brief Enable or disable pop-up menu-item
    ///
    /// Enable, disable or grays dialog menu pop-up menu-item.
    ///
    /// \param[in]  popup   : Menu pop-up handle to get menu-item from.
    /// \param[in]  pos     : Zero based menu-item position within the pop-up.
    /// \param[in]  enable  : Enable flags to set.
    ///
    /// \return Menu handle.
    ///
    void setPopupItem(HMENU popup, unsigned pos, int enable) const {
      EnableMenuItem(popup, pos, MF_BYPOSITION|enable);
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief On initialize virtual function.
    ///
    /// Virtual function to be defined by derived class, called when
    /// WM_INITIDALOG dialog message is received.
    ///
    virtual void _onInit();

    /// \brief On show virtual function.
    ///
    /// Virtual function to be defined by derived class, called when
    /// WM_SHOWWINDOW with True wParam dialog message is received.
    ///
    virtual void _onShow();

    /// \brief On hide virtual function.
    ///
    /// Virtual function to be defined by derived class, called when
    /// WM_SHOWWINDOW with False wParam dialog message is received.
    ///
    virtual void _onHide();

    /// \brief On show virtual function.
    ///
    /// \param[in] msg    Received window message.
    ///
    /// Virtual function to be defined by derived class, called when
    /// WM_SIZE dialog message is received.
    ///
    virtual void _onResize();

    /// \brief On refresh virtual function.
    ///
    /// Virtual function to be defined by derived class, called when
    /// dialog refresh is requested.
    ///
    virtual void _onRefresh();

    /// \brief On show virtual function.
    ///
    /// Virtual function to be defined by derived class, called when
    /// WM_CLOSE dialog message is received.
    ///
    virtual void _onClose();

    /// \brief On quit virtual function.
    ///
    /// Virtual function to be defined by derived class, called when
    /// dialog quit is requested.
    ///
    virtual void _onQuit();

    /// \brief On message virtual function.
    ///
    /// Virtual function to be defined by derived class, called when
    /// dialog receive a message.
    ///
    virtual INT_PTR _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

  protected: ///        - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    HINSTANCE           _hins;      //< Dialog HINSTANCE

    HWND                _hwnd;      //< Dialog window Handle

    OmDialog*           _parent;    //< Parent dialog

    std::vector<OmDialog*>   _child;     //< Children dialogs

    HACCEL              _accel;     //< Accelerator table handle for dialog

    HMENU               _menu;      //< Menu if exists or set

    void*               _data;      //< Dialog custom data

    void                _setItemPos(unsigned, long, long, long, long, bool pixel = false);

    void                _setChildPos(HWND, long, long, long, long, bool pixel = false);

    void                _getItemPos(unsigned, long*, bool pixel = false);

    void                _getItemSize(unsigned, long*, bool pixel = false);

    void                _createTooltip(unsigned, const OmWString&);

    void                _setSafe(bool safe);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    bool                _init;      //< Dialog in initialization

    bool                _modal;     //< Dialog was created as modal

    long                _recw[4];   //< Dialog window rect

    long                _recc[4];   //< Dialog client rect

    int                 _ubase[2];  //< Dialog window base unit

    long                _limit[2];  //< Dialog window size limits

    long                _size[2];   //< Dialog window size in pixel

    long                _usize[2];  //< Dialog window size in base unit

    bool                _active;    //< Dialog window tracked ACTIVE state

    bool                _minimized; //< Dialog window tracked MINIMIZED state

    bool                _safe;      //< Dialog is safe to quit

    bool                _wait_safe; //< Dialog wait safe for quit

    HDWP                _hdwp;      //< Defer window pos handle

    static INT_PTR CALLBACK  _wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMDIALOG_H
