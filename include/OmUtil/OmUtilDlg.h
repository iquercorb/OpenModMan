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
#ifndef OMUTILDLG_H
#define OMUTILDLG_H

#include "OmBase.h"
#include "OmBaseWin.h"

/// \brief Predefined filter string for dialogs.
///
/// Predefined common files filter string for Open dialog
///
#define OM_PKG_FILES_FILTER       L"Package file (*.ozp,*.zip)\0*.ozp;*.ZIP;\0Open Mod Pack (*.ozp)\0*.ozp;\0Zip file (*.zip)\0*.zip;\0All files (*.*)\0*.*;\0"
#define OM_BCK_FILES_FILTER       L"Backup file (*.ozb,*.zip)\0*.ZIP;*.ozb;\0Open Mod Backup (*.ozb)\0*.ozb;\0Zip file (*.zip)\0*.zip;\0All files (*.*)\0*.*;\0"
#define OM_IMG_FILES_FILTER       L"Image file (*.bmp,*.jpg,*.jpeg,*.gif,*.png)\0*.bmp;*.jpg;*.jpeg;*.gif;*.png;\0BMP file (*.bmp)\0*.bmp;\0JPEG file (*.jpg,*.jpeg)\0*.jpg;*.jpeg\0PNG file (*.png)\0*.png;\0CompuServe GIF (*.gif)\0*.gif;\0All files (*.*)\0*.*;\0"
#define OM_XML_FILES_FILTER       L"XML file (*.xml)\0*.xml;\0All files (*.*)\0*.*;\0"
#define OM_TXT_FILES_FILTER       L"Reamde file (*.txt,*.md,*.mkd)\0*.txt;*.md;*.mkd;\0Text file (*.txt)\0*.txt;\0Markdown file (*.md,*.mkd)\0*.md;*.mkd;\0All files (*.*)\0*.*;\0"
#define OM_ICO_FILES_FILTER       L"Icon files (*.ico,*.exe)\0*.ico;*.exe;\0Programs (*.exe)\0*.exe;\0Icons (*.ico)\0*.ico;\0All files (*.*)\0*.*;\0"
#define OM_HUB_FILES_FILTER       L"Definition file (*.omx,*.omc)\0*.omx;*.omc;\0Open Mod XML (*.omx)\0*.omx;\0Open Mod Context (*.omc)\0*.omc;\0All files (*.*)\0*.*;\0"
#define OM_REP_FILES_FILTER       L"Definition file (*.omx,*.xml)\0*.omx;*.xml;\0Open Mod XML (*.omx)\0*.omx;\0XML file (*.xml)\0*.xml;\0All files (*.*)\0*.*;\0"

typedef struct Om_filterSpec_s
{
  const wchar_t* name;
  const wchar_t* spec;
} Om_filterSpec_t;

/// \brief Message box parameters flags.
///
/// Buttons parameters flags for message box.
///
#define OM_DLGBOX_OK        0x0   //< OK
#define OM_DLGBOX_OC        0x1   //< OK - Cancel
#define OM_DLGBOX_YN        0x2   //< Yes - No
#define OM_DLGBOX_CA        0x4   //< Continue - Abort
#define OM_DLGBOX_YNC       0x8   //< Yes - No
#define OM_DLGBOX_DUAL_BARS 0x80  //< Progress dialog

/// \brief Custom message box dialog.
///
/// Show message box dialog according given parameters, parameters with
/// null value are ignored and not displayed.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Window caption.
/// \param[in] ico    : Optional icon resource ID.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
/// \param[in] lst    : Optional message item list.
/// \param[in] flags  : Buttons flags.
///
/// \return Zero if NO or CANCEL button was clicked, 1 if OK or YES button was clicked.
///
int Om_dlgBox(HINSTANCE hins, HWND hwnd, const wchar_t* cpt, uint16_t ico, const wchar_t* hdr, const wchar_t* msg, const wchar_t* lst, unsigned flags);

/// \brief Custom progression dialog.
///
/// Create progression dialog according given parameters, parameters with
/// null value are ignored and not displayed.
///
/// \param[in] hwnd     : Handle to parent/owner window.
/// \param[in] cpt      : Window caption.
/// \param[in] ico      : Optional icon resource ID.
/// \param[in] hdr      : Optional message header.
/// \param[out] result  : Integer to be set as dialog quit result (1 if abort).
/// \param[in] flags    : Option flags.
///
/// \return Handle to progression dialog
///
HWND Om_dlgProgress(HWND hwnd, const wchar_t* cpt, uint16_t ico, const wchar_t* hdr, int* result = nullptr, unsigned flags = 0);

/// \brief Update progression dialog.
///
/// Update an existing progression dialog with supplied parameters. Integer parameter
/// less than 0 and null pointer are ignored and not updated.
///
/// \param[in] hwnd     : Handle to existing progress dialog
/// \param[in] tot      : Progress bar range.
/// \param[in] cur      : Progress bar current position.
/// \param[in] msg      : Progression message (item name) to set.
/// \param[in] bar      : In case of multiple progress bars, the progress bar index
///
void Om_dlgProgressUpdate(HWND hwnd, int tot = -1, int cur = -1, const wchar_t* msg = nullptr, uint8_t bar = 0);

/// \brief Close progression dialog.
///
/// Close an existing progression dialog.
///
/// \param[in] hwnd     : Handle to existing progress dialog
///
void Om_dlgProgressClose(HWND hwnd);

/// \brief OK message box.
///
/// Show a message box dialog with OK button.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Window caption.
/// \param[in] ico    : Optional icon resource ID.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
void Om_dlgBox_ok(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg);

/// \brief OK message box with List.
///
/// Show a message box dialog with OK button and an item list.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Window caption.
/// \param[in] ico    : Optional icon resource ID.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
/// \param[in] lst    : Optional message item list.
///
void Om_dlgBox_okl(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg, const OmWString& lst);

/// \brief Yes/No message box.
///
/// Show a message box dialog with Yes and No button which return
/// value corresponding to clicked button.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Window caption.
/// \param[in] ico    : Optional icon resource ID.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
/// \return True if user clicked 'Yes' button, false otherwise.
///
bool Om_dlgBox_yn(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg);

/// \brief Yes/No message box with List.
///
/// Show a message box dialog with Yes and No button and an item list
/// which return value corresponding to clicked button.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Window caption.
/// \param[in] ico    : Optional icon resource ID.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
/// \return True if user clicked 'Yes' button, false otherwise.
///
bool Om_dlgBox_ynl(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg, const OmWString& lst);

/// \brief Continue/Abort message box.
///
/// Show a message box dialog with Continue and Abort button
/// which return value corresponding to clicked button.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Window caption.
/// \param[in] ico    : Optional icon resource ID.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
/// \return True if user clicked 'Continue' button, false otherwise.
///
bool Om_dlgBox_ca(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg);

/// \brief Continue/Abort message box with List.
///
/// Show a message box dialog with Continue and Abort button and an item list
/// which return value corresponding to clicked button.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Window caption.
/// \param[in] ico    : Optional icon resource ID.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
/// \return True if user clicked 'Continue' button, false otherwise.
///
bool Om_dlgBox_cal(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg, const OmWString& lst);

/// \brief Yes/No/Cancel message box with List.
///
/// Show a message box dialog with Yes and No button and an item list
/// which return value corresponding to clicked button.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Window caption.
/// \param[in] ico    : Optional icon resource ID.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
/// \return 1 if user clicked 'Yes', 0 if user clicker 'No' and -1 for 'Cancel'
///
int32_t Om_dlgBox_ync(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg);

/// \brief Yes/No/Cancel message box with List.
///
/// Show a message box dialog with Yes and No button and an item list
/// which return value corresponding to clicked button.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Window caption.
/// \param[in] ico    : Optional icon resource ID.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
/// \return 1 if user clicked 'Yes', 0 if user clicker 'No' and -1 for 'Cancel'
///
int32_t Om_dlgBox_yncl(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg, const OmWString& lst);

/// \brief Error message box.
///
/// Show basic error message box with standard stock icon.
///
/// \param[in] cpt    : Window caption.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
void Om_dlgBox_err(const OmWString& cpt, const OmWString& hdr, const OmWString& msg);

/// \brief Warning message box.
///
/// Show basic warning message box with standard stock icon.
///
/// \param[in] cpt    : Window caption.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
void Om_dlgBox_wrn(const OmWString& cpt, const OmWString& hdr, const OmWString& msg);
/*
/// \brief Select folder dialog box.
///
/// Opens a folder selection dialog box.
///
/// \param[out] result  : Buffer to receive the selected folder path.
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  title   : Dialog window title.
/// \param[in]  start   : Path to default location where to begin browsing.
/// \param[in]  captive : If true, the default start location is set as root
///                       and user cannot browse up to parent folder.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dlgBrowseDir(OmWString& result, HWND hwnd, const wchar_t* title, const OmWString& start, bool captive = false, bool nonew = false);
*/
/// \brief Select directory dialog box (explorer style).
///
/// Opens a single directory selection dialog box using modern explorer style.
///
/// \param[out] result  : Wide string that receive the selected directory path.
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  title   : Dialog window title.
/// \param[in]  start   : Default directory path where to begin browsing.
/// \param[in]  force   : Force dialog to use defined start path.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dlgOpenDir(OmWString& result, HWND hWnd, const wchar_t* title, const OmWString& start, bool force = false);

/// \brief Select multiple directories dialog box (explorer style).
///
/// Opens a multiple directories selection dialog box using modern explorer style.
///
/// \param[out] result  : Array of wide string that to receive the selected directories path.
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  title   : Dialog window title.
/// \param[in]  start   : Default directory path where to begin browsing.
/// \param[in]  force   : Force dialog to use defined start path.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dlgOpenDirMultiple(OmWStringArray& result, HWND hWnd, const wchar_t* title, const OmWString& start, bool force = false);

/// \brief Open file dialog box (explorer style).
///
/// Opens a file open selection dialog box using modern API.
///
/// \param[out] result  : Wide string that receive the selected file path
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore
/// \param[in]  title   : Dialog window title
/// \param[in]  filter  : File type and extension filter string
/// \param[in]  start   : Default directory path where to begin browsing.
/// \param[in]  force   : Force dialog to use defined start path.
///
/// \return True if succeed, false if user canceled or an error occurred
///
bool Om_dlgOpenFile(OmWString& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const OmWString& start, bool force = false);

/// \brief Open multiple files dialog box (explorer style).
///
/// Opens a file open selection dialog box that allow multiple selection using modern API.
///
/// \param[out] result  : Array of wide string that receive selected files path
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore
/// \param[in]  title   : Dialog window title
/// \param[in]  filter  : File type and extension filter string
/// \param[in]  start   : Default directory path where to begin browsing.
/// \param[in]  force   : Force dialog to use defined start path.
///
/// \return True if succeed, false if user canceled or an error occurred
///
bool Om_dlgOpenFileMultiple(OmWStringArray& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const OmWString& start, bool force = false);

/// \brief Save file dialog box.
///
/// Opens a file save as selection dialog box using modern API.
///
/// \param[out] result  : Buffer to receive the selected folder path.
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  title   : Dialog window title.
/// \param[in]  filter  : File type and extension filter string.
/// \param[in]  ext     : Default file extension to add to filename if required
/// \param[in]  name    : Default file name provided at dialog open.
/// \param[in]  start   : Path to default location where to begin browsing.
/// \param[in]  force   : Force dialog to use defined start path.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dlgSaveFile(OmWString& result, HWND hwnd, const wchar_t* title, const wchar_t* filter, const wchar_t* ext, const wchar_t* name, const OmWString& start, bool force = false);

/// \brief Create folder dialog process.
///
/// Checks whether specified path exists, if not ask the user to create
/// it and warn user if creation failed.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  item    : Formated message item to set in question message.
/// \param[in]  path    : Path to check and create if required.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dlgCreateFolder(HWND hwnd, const OmWString& item, const OmWString& path);

/// \brief Overwrite file dialog process.
///
/// Checks whether specified file exists, if yes, asks user to overwrite
/// existing file.
///
/// \param[in]  hwnd      : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  path      : File path to check.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dlgOverwriteFile(HWND hwnd, const OmWString& path);

/// \brief Unsaved changes before closing dialog.
///
/// Generic dialog message to ask for saving unsaved changes before closing.
///
/// \param[in] cpt    : Window caption.
/// \param[in] hwnd    : Dialog boxes parent window handle or nullptr to ignore.
///
/// \return True if user choose to close anyway, false otherwise
///
bool Om_dlgCloseUnsaved(HWND hwnd, const OmWString& cpt);

/// \brief Unsaved changes before reset dialog.
///
/// Generic dialog message to ask for saving unsaved changes before reset.
///
/// \param[in] cpt    : Window caption.
/// \param[in] hwnd    : Dialog boxes parent window handle or nullptr to ignore.
///
/// \return True if user choose to close anyway, false otherwise
///
bool Om_dlgResetUnsaved(HWND hwnd, const OmWString& cpt);

/// \brief File save success dialog.
///
/// Generic dialog message for file save success.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  item    : Formated message item to set in success message.
///
void Om_dlgSaveSucces(HWND hwnd, const OmWString& cpt, const OmWString& hdr, const OmWString& item);

/// \brief File save error dialog.
///
/// Generic dialog message for file save error.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  item    : Formated message item to set in error message.
/// \param[in]  error   : Formated message error to set in error message.
///
void Om_dlgSaveError(HWND hwnd, const OmWString& cpt, const OmWString& hdr, const OmWString& item, const OmWString& error);

/// \brief Valid name dialog process.
///
/// Checks whether specified name is valid and if not warn user with proper
/// dialog box.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  item    : Formated message item to set as invalid.
/// \param[in]  name    : Name to check.
///
/// \return True if name pass validation, false otherwise.
///
bool Om_dlgValidName(HWND hwnd, const OmWString& item,  const OmWString& name);

/// \brief Valid path dialog process.
///
/// Checks whether specified path is valid and if not warn user with proper
/// dialog box.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  item    : Formated message item to set as invalid.
/// \param[in]  path    : Path to check.
///
/// \return True if path pass validation, false otherwise.
///
bool Om_dlgValidPath(HWND hwnd, const OmWString& item,  const OmWString& path);

/// \brief Valid URL dialog process.
///
/// Checks whether specified URL is valid and if not warn user with proper
/// dialog box.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  item    : Formated message item to set as invalid.
/// \param[in]  url     : Path to check.
///
/// \return True if url pass validation, false otherwise.
///
bool Om_dlgValidUrl(HWND hwnd, const OmWString& item, const OmWString& url);

/// \brief Valid URL name dialog process.
///
/// Checks whether specified URL name is valid and if not warn user with proper
/// dialog box.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  item    : Formated message item to set as invalid.
/// \param[in]  name    : Path to check.
///
/// \return True if URL name pass validation, false otherwise.
///
bool Om_dlgValidUrlPath(HWND hwnd, const OmWString& item, const OmWString& name);

/// \brief Valid directory dialog process.
///
/// Checks whether specified path is an existing directory and if not
/// warn user with proper dialog box.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  name    : Formated message item to set as invalid.
/// \param[in]  path    : Path to check.
///
/// \return True if path pass validation, false otherwise.
///
bool Om_dlgValidDir(HWND hwnd, const OmWString& item,  const OmWString& path);

#endif // OMUTILDLG_H
