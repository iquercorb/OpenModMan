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
#include "OmBase.h"           //< string, vector, Om_alloc, OM_MAX_PATH, etc.

#include "OmUtilWin.h"   //< Om_getErrorStr

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errNotDir(const OmWString& item,  const OmWString& path)
{
  return item + L" \"" + path + L"\" does not exist or is not a directory.";
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errCreate(const OmWString& item,  const OmWString& path, int result)
{
  return item + L" \"" + path + L"\" creation error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errDelete(const OmWString& item,  const OmWString& path, int result)
{
  return item + L" \"" + path + L"\" delete error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errRename(const OmWString& item,  const OmWString& path, int result)
{
  return item + L" \"" + path + L"\" rename error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errMove(const OmWString& item,  const OmWString& path, int result)
{
  return item + L" \"" + path + L"\" move error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errCopy(const OmWString& item, const OmWString& path, int result)
{
  return item + L" \"" + path + L"\" copy error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errShell(const OmWString& item, const OmWString& path, int result)
{
  return item + L" \"" + path + L"\" shell error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errReadAccess(const OmWString& item, const OmWString& path)
{
  return item + L" \"" + path + L"\" read access denied";
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errWriteAccess(const OmWString& item, const OmWString& path)
{
  return item + L" \"" + path + L"\" write access denied";
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errInit(const OmWString& item, const OmWString& path, const OmWString& error)
{
  return item + L" \"" + path + L"\" cannot be created: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errOpen(const OmWString& item, const OmWString& path, const OmWString& error)
{
  return item + L" \"" + path + L"\" cannot be opened: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errLoad(const OmWString& item, const OmWString& path, const OmWString& error)
{
  return item + L" \"" + path + L"\" cannot be loaded: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errSave(const OmWString& item, const OmWString& path, const OmWString& error)
{
  return item + L" \"" + path + L"\" cannot be saved: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errParse(const OmWString& item, const OmWString& path, const OmWString& error)
{
  return item + L" \"" + path + L"\" parse error: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errZipComp(const OmWString& item, const OmWString& path, const OmWString& mesg)
{
  return item + L" \"" + path + L"\" ZIP compression error: " + mesg;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errZipExtr(const OmWString& item, const OmWString& path, const OmWString& mesg)
{
  return item + L" \"" + path + L"\" ZIP extraction error: " + mesg;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errExists(const OmWString& item, const OmWString& path)
{
  return item + L" \"" + path + L"\" already exists";
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_errBadAlloc(const OmWString& subject, const OmWString& name)
{
  return subject + L" \"" + name + L"\" error: out of memory.";
}
