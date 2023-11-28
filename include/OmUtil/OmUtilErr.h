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
#ifndef OMUTILERR_H
#define OMUTILERR_H

#include "OmBase.h"

/// \brief Invalid directory error message.
///
/// Compose error string for invalid or non existing directory.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
///
/// \return Formated error log message.
///
OmWString Om_errNotDir(const OmWString& item,  const OmWString& path);

/// \brief Create error message.
///
/// Compose error string for unable to create.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
OmWString Om_errCreate(const OmWString& item,  const OmWString& path, int result);

/// \brief Delete error message.
///
/// Compose error string for unable to delete.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
OmWString Om_errDelete(const OmWString& item,  const OmWString& path, int result);

/// \brief Rename error message.
///
/// Compose error string for unable to rename.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
OmWString Om_errRename(const OmWString& item, const OmWString& path, int result);

/// \brief Move error message.
///
/// Compose error string for unable to move.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
OmWString Om_errMove(const OmWString& item, const OmWString& path, int result);

/// \brief Copy error message.
///
/// Compose error string for unable to copy.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
OmWString Om_errCopy(const OmWString& item, const OmWString& path, int result);

/// \brief Shell File operation error message.
///
/// Compose error string for Shell File operation error.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  result  : WinAPI standard result error code.
///
/// \return Formated error log message.
///
OmWString Om_errShell(const OmWString& item, const OmWString& path, int result);

/// \brief Read access error message.
///
/// Compose error string for unable to read access.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
///
/// \return Formated error log message.
///
OmWString Om_errReadAccess(const OmWString& item, const OmWString& path);

/// \brief Write access error message.
///
/// Compose error string for unable to write access.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
///
/// \return Formated error log message.
///
OmWString Om_errWriteAccess(const OmWString& item, const OmWString& path);

/// \brief Init (create) error message.
///
/// Compose error string for unable to init (create).
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
OmWString Om_errInit(const OmWString& item, const OmWString& path, const OmWString& error);

/// \brief Open error message.
///
/// Compose error string for unable to open.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
OmWString Om_errOpen(const OmWString& item, const OmWString& path, const OmWString& error);

/// \brief Load error message.
///
/// Compose error string for unable to load.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
OmWString Om_errLoad(const OmWString& item, const OmWString& path, const OmWString& error);

/// \brief Save error message.
///
/// Compose error string for unable to save.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
OmWString Om_errSave(const OmWString& item, const OmWString& path, const OmWString& error);

/// \brief Save error message.
///
/// Compose error string for unable to save.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  error   : Additional error message details.
///
/// \return Formated error log message.
///
OmWString Om_errParse(const OmWString& item, const OmWString& path, const OmWString& error);

/// \brief Zip deflate error message.
///
/// Compose error string for unable to deflate file.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  mesg    : Additional error message.
///
/// \return Formated error log message.
///
OmWString Om_errZipComp(const OmWString& item, const OmWString& path, const OmWString& mesg);

/// \brief Zip inflate error message.
///
/// Compose error string for unable to inflate file.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
/// \param[in]  mesg    : Additional error message.
///
/// \return Formated error log message.
///
OmWString Om_errZipExtr(const OmWString& item, const OmWString& path, const OmWString& mesg);

/// \brief Already exists error message.
///
/// Compose error string for already existing item.
///
/// \param[in]  item    : Formated message item designation.
/// \param[in]  path    : Formated message item path.
///
/// \return Formated error log message.
///
OmWString Om_errExists(const OmWString& item, const OmWString& path);

/// \brief Bad Alloc error message.
///
/// Compose error string for unable to allocate memory.
///
/// \param[in]  subject : Formated message allocation subject/reason.
/// \param[in]  name    : Formated message item name or path.
///
/// \return Formated error log message.
///
OmWString Om_errBadAlloc(const OmWString& subject, const OmWString& name);

#endif // OMUTILERR_H
