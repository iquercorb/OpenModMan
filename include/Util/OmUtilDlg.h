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
#ifndef OMUTILDLG_H_INCLUDED
#define OMUTILDLG_H_INCLUDED

#include "OmBase.h"
#include "OmBaseWin.h"

/// \brief Predefined filter string for dialogs.
///
/// Predefined common files filter string for Open dialog
///
#define OMM_PKG_FILES_FILTER      L"Package File (*.zip,*.ozp)\0*.ZIP;*.OZP;\0"
#define OMM_BCK_FILES_FILTER      L"Backup File (*.zip,*.ozb)\0*.ZIP;*.OZB;\0"
#define OMM_IMG_FILES_FILTER      L"Image file (*.bmp,*.jpg,*.jpeg,*.gif,*.png)\0*.BMP;*.JPG;*.JPEG;*.GIF;*.PNG;\0BMP file (*.bmp)\0*.BMP;\0JPEG file (*.jpg,*.jpeg)\0*.JPG;*.JPEG\0PNG file (*.png)\0*.PNG;\0CompuServe GIF (*.gif)\0*.GIF;\0"
#define OMM_XML_FILES_FILTER      L"XML File (*.xml)\0*.XML;\0"
#define OMM_TXT_FILES_FILTER      L"Text file (*.txt)\0*.TXT\0"
#define OMM_ICO_FILES_FILTER      L"Icon files (*.ico,*.exe)\0*.ICO;*.EXE;\0Programs (*.exe)\0*.EXE;\0Icons (*.ico)\0*.ICO;\0"

#define OMM_CTX_DEF_FILE_FILER    L"Context definition (*.omc)\0*.OMC;\0"
#define OMM_LOC_DEF_FILE_FILER    L"Location definition (*.oml)\0*.OML;\0"
#define OMM_PKG_DEF_FILE_FILER    L"Package definition (*.omp)\0*.OMP;\0"
#define OMM_BCK_DEF_FILE_FILER    L"Backup definition (*.omk)\0*.OMK;\0"
#define OMM_BAT_DEF_FILE_FILER    L"Batch definition (*.omb)\0*.OMB;\0"

/// \brief Message box parameters flags.
///
/// Buttons parameters flags for message box.
///
#define OMM_DLGBOX_OK        0x0
#define OMM_DLGBOX_OC        0x1
#define OMM_DLGBOX_YN        0x2
#define OMM_DLGBOX_CA        0x4

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
void Om_dlgBox_ok(HWND hwnd, const wstring& cpt, uint16_t ico, const wstring& hdr, const wstring& msg);

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
void Om_dlgBox_okl(HWND hwnd, const wstring& cpt, uint16_t ico, const wstring& hdr, const wstring& msg, const wstring& lst);

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
bool Om_dlgBox_yn(HWND hwnd, const wstring& cpt, uint16_t ico, const wstring& hdr, const wstring& msg);

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
bool Om_dlgBox_ynl(HWND hwnd, const wstring& cpt, uint16_t ico, const wstring& hdr, const wstring& msg, const wstring& lst);

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
bool Om_dlgBox_ca(HWND hwnd, const wstring& cpt, uint16_t ico, const wstring& hdr, const wstring& msg);

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
bool Om_dlgBox_cal(HWND hwnd, const wstring& cpt, uint16_t ico, const wstring& hdr, const wstring& msg, const wstring& lst);

/// \brief Error message box.
///
/// Show basic error message box with standard stock icon.
///
/// \param[in] cpt    : Window caption.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
void Om_dlgBox_err(const wstring& cpt, const wstring& hdr, const wstring& msg);

/// \brief Warning message box.
///
/// Show basic warning message box with standard stock icon.
///
/// \param[in] cpt    : Window caption.
/// \param[in] hdr    : Optional message header.
/// \param[in] msg    : Optional message body.
///
void Om_dlgBox_wrn(const wstring& cpt, const wstring& hdr, const wstring& msg);

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
bool Om_dlgBrowseDir(wstring& result, HWND hwnd, const wchar_t* title, const wstring& start, bool captive = false);

/// \brief Open file dialog box.
///
/// Opens a file open selection dialog box.
///
/// \param[out] result  : Buffer to receive the selected folder path.
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  title   : Dialog window title.
/// \param[in]  filter  : File type and extension filter string.
/// \param[in]  start   : Path to default location where to begin browsing.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dlgOpenFile(wstring& result, HWND hwnd, const wchar_t* title, const wchar_t* filter, const wstring& start);

/// \brief Save file dialog box.
///
/// Opens a file save as selection dialog box.
///
/// \param[out] result  : Buffer to receive the selected folder path.
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  title   : Dialog window title.
/// \param[in]  filter  : File type and extension filter string.
/// \param[in]  start   : Path to default location where to begin browsing.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dlgSaveFile(wstring& result, HWND hwnd, const wchar_t* title, const wchar_t* filter, const wstring& start);

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
bool Om_dlgCreateFolder(HWND hwnd, const wstring& item, const wstring& path);

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
bool Om_dlgOverwriteFile(HWND hwnd, const wstring& path);

/// \brief Unsaved changes before closing dialog.
///
/// Generic dialog message to ask for saving unsaved changes before closing.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
///
/// \return True if user choose to close anyway, false otherwise
///
bool Om_dlgCloseUnsaved(HWND hwnd);

/// \brief Unsaved changes before reset dialog.
///
/// Generic dialog message to ask for saving unsaved changes before reset.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
///
/// \return True if user choose to close anyway, false otherwise
///
bool Om_dlgResetUnsaved(HWND hwnd);

/// \brief File save success dialog.
///
/// Generic dialog message for file save success.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  item    : Formated message item to set in success message.
///
void Om_dlgSaveSucces(HWND hwnd, const wstring& item);

/// \brief File save error dialog.
///
/// Generic dialog message for file save error.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  item    : Formated message item to set in error message.
/// \param[in]  error   : Formated message error to set in error message.
///
void Om_dlgSaveError(HWND hwnd, const wstring& item, const wstring& error);

/// \brief Valid name dialog process.
///
/// Checks whether specified name is valid and if not warn user with proper
/// dialog box.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  name    : Formated message item to set as invalid.
/// \param[in]  name    : Name to check.
///
/// \return True if name pass validation, false otherwise.
///
bool Om_dlgValidName(HWND hwnd, const wstring& item,  const wstring& name);

/// \brief Valid path dialog process.
///
/// Checks whether specified name is valid and if not warn user with proper
/// dialog box.
///
/// \param[in]  hwnd    : Dialog boxes parent window handle or nullptr to ignore.
/// \param[in]  name    : Formated message item to set as invalid.
/// \param[in]  path    : Path to check.
///
/// \return True if name pass validation, false otherwise.
///
bool Om_dlgValidPath(HWND hwnd, const wstring& item,  const wstring& path);

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
bool Om_dlgValidDir(HWND hwnd, const wstring& item,  const wstring& path);

#endif // OMUTILDLG_H_INCLUDED
