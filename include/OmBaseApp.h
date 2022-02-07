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
#ifndef OMBASEAPP_H_INCLUDED
#define OMBASEAPP_H_INCLUDED

#define OMM_APP_NAME              L"Open Mod Manager"
#define OMM_APP_SHORT_NAME        L"OpenModMan"

#define OMM_APP_MAJ               0
#define OMM_APP_MIN               9
#define OMM_APP_REV               9

#ifdef _WIN64
  #define OMM_APP_ARCH            L"x64"
#else
  #define OMM_APP_ARCH            L"x86"
#endif

#define OMM_APP_DATE              L"Feburary 2022"
#define OMM_APP_AUTHOR            L"Eric M."
#define OMM_APP_CONTRIB           L""
#define OMM_APP_C_YEAR            L"2022"
#define OMM_APP_GIT               L"https://github.com/sedenion/OpenModMan"
#define OMM_APP_URL               L"https://github.com/sedenion/OpenModMan"

#define OMM_XMAGIC_APP    L"Open_Mod_Manager_Main"
#define OMM_XMAGIC_CTX    L"Open_Mod_Manager_Context"
#define OMM_XMAGIC_LOC    L"Open_Mod_Manager_Location"
#define OMM_XMAGIC_PKG    L"Open_Mod_Manager_Package"
#define OMM_XMAGIC_BCK    L"Open_Mod_Manager_Backup"
#define OMM_XMAGIC_BAT    L"Open_Mod_Manager_Batch"
#define OMM_XMAGIC_REP    L"Open_Mod_Manager_Repository"

#define OMM_CTX_DEF_FILE_EXT      L"omc"
#define OMM_LOC_DEF_FILE_EXT      L"oml"
#define OMM_BAT_DEF_FILE_EXT      L"omb"
#define OMM_PKG_DEF_FILE_EXT      L"omp"
#define OMM_BCK_DEF_FILE_EXT      L"omk"
#define OMM_PKG_FILE_EXT          L"ozp"
#define OMM_BCK_FILE_EXT          L"ozb"

#define OMM_THUMB_SIZE    128

#endif // OMBASEAPP_H_INCLUDED
