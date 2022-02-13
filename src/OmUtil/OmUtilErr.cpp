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
#include "OmBase.h"           //< string, vector, Om_alloc, OMM_MAX_PATH, etc.

#include "OmUtilWin.h"   //< Om_getErrorStr

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errIsDir(const wstring& item,  const wstring& path)
{
  return item + L" \"" + path + L"\" does not exist or is not a directory.";
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errCreate(const wstring& item,  const wstring& path, int result)
{
  return item + L" \"" + path + L"\" creation error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errDelete(const wstring& item,  const wstring& path, int result)
{
  return item + L" \"" + path + L"\" delete error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errRename(const wstring& item,  const wstring& path, int result)
{
  return item + L" \"" + path + L"\" rename error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errMove(const wstring& item,  const wstring& path, int result)
{
  return item + L" \"" + path + L"\" move error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errCopy(const wstring& item, const wstring& path, int result)
{
  return item + L" \"" + path + L"\" copy error: " + Om_getErrorStr(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errReadAccess(const wstring& item, const wstring& path)
{
  return item + L" \"" + path + L"\" read access denied";
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errWriteAccess(const wstring& item, const wstring& path)
{
  return item + L" \"" + path + L"\" write access denied";
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errInit(const wstring& item, const wstring& path, const wstring& error)
{
  return item + L" \"" + path + L"\" cannot be created: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errOpen(const wstring& item, const wstring& path, const wstring& error)
{
  return item + L" \"" + path + L"\" cannot be opened: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errLoad(const wstring& item, const wstring& path, const wstring& error)
{
  return item + L" \"" + path + L"\" cannot be loaded: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errSave(const wstring& item, const wstring& path, const wstring& error)
{
  return item + L" \"" + path + L"\" cannot be saved: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errParse(const wstring& item, const wstring& path, const wstring& error)
{
  return item + L" \"" + path + L"\" parse error: " + error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errZipInit(const wstring& item, const wstring& path, const wstring& mesg)
{
  return item + L" \"" + path + L"\" ZIP create error: " + mesg;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errZipLoad(const wstring& item, const wstring& path, const wstring& mesg)
{
  return item + L" \"" + path + L"\" ZIP open error: " + mesg;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errZipDefl(const wstring& item, const wstring& path, const wstring& mesg)
{
  return item + L" \"" + path + L"\" ZIP compress error: " + mesg;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errZipInfl(const wstring& item, const wstring& path, const wstring& mesg)
{
  return item + L" \"" + path + L"\" ZIP extract error: " + mesg;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_errExists(const wstring& item, const wstring& path)
{
  return item + L" \"" + path + L"\" already exists";
}
