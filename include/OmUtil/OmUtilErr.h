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
#ifndef OMUTILERR_H_INCLUDED
#define OMUTILERR_H_INCLUDED

#include "OmBase.h"

/// \brief Invalid directory error message.
///
/// Compose error string for invalid or non existing directory.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
///
/// \return Formated error log message.
///
wstring Om_errIsDir(const wstring& item,  const wstring& path);

/// \brief Create error message.
///
/// Compose error string for unable to create.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
wstring Om_errCreate(const wstring& item,  const wstring& path, int result);

/// \brief Delete error message.
///
/// Compose error string for unable to delete.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
wstring Om_errDelete(const wstring& item,  const wstring& path, int result);

/// \brief Rename error message.
///
/// Compose error string for unable to rename.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
wstring Om_errRename(const wstring& item, const wstring& path, int result);

/// \brief Move error message.
///
/// Compose error string for unable to move.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
wstring Om_errMove(const wstring& item, const wstring& path, int result);

/// \brief Copy error message.
///
/// Compose error string for unable to copy.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
wstring Om_errCopy(const wstring& item, const wstring& path, int result);

/// \brief Shell File operation error message.
///
/// Compose error string for Shell File operation error.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
wstring Om_errShell(const wstring& item, const wstring& path, int result);

/// \brief Read access error message.
///
/// Compose error string for unable to read access.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
///
/// \return Formated error log message.
///
wstring Om_errReadAccess(const wstring& item, const wstring& path);

/// \brief Write access error message.
///
/// Compose error string for unable to write access.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
///
/// \return Formated error log message.
///
wstring Om_errWriteAccess(const wstring& item, const wstring& path);

/// \brief Init (create) error message.
///
/// Compose error string for unable to init (create).
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
wstring Om_errInit(const wstring& item, const wstring& path, const wstring& error);

/// \brief Open error message.
///
/// Compose error string for unable to open.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
wstring Om_errOpen(const wstring& item, const wstring& path, const wstring& error);

/// \brief Load error message.
///
/// Compose error string for unable to load.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
wstring Om_errLoad(const wstring& item, const wstring& path, const wstring& error);

/// \brief Save error message.
///
/// Compose error string for unable to save.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
wstring Om_errSave(const wstring& item, const wstring& path, const wstring& error);

/// \brief Save error message.
///
/// Compose error string for unable to save.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
wstring Om_errParse(const wstring& item, const wstring& path, const wstring& error);

/// \brief Zip deflate error message.
///
/// Compose error string for unable to deflate file.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  mesg    : Additional error message.
///
/// \return Formated error log message.
///
wstring Om_errZipDefl(const wstring& item, const wstring& path, const wstring& mesg);

/// \brief Zip inflate error message.
///
/// Compose error string for unable to inflate file.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  mesg    : Additional error message.
///
/// \return Formated error log message.
///
wstring Om_errZipInfl(const wstring& item, const wstring& path, const wstring& mesg);

/// \brief Already exists error message.
///
/// Compose error string for already existing item.
///
/// \param[in]  name    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
///
/// \return Formated error log message.
///
wstring Om_errExists(const wstring& item, const wstring& path);


#endif // OMUTILERR_H_INCLUDED
