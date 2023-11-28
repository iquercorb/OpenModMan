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
#ifndef OMBASEAPP_H
#define OMBASEAPP_H

#define OM_APP_NAME              L"Open Mod Manager"
#define OM_APP_SHORT_NAME        L"OpenModMan"

#define OM_APP_MAJ               1
#define OM_APP_MIN               2
#define OM_APP_REV               0

#ifdef _WIN64
  #define OM_APP_ARCH            L"x64"
#else
  #define OM_APP_ARCH            L"x86"
#endif

#define OM_APP_DATE              L"" __DATE__
#define OM_APP_AUTHOR            L"Eric M."
#define OM_APP_CONTRIB           L""
#define OM_APP_C_YEAR            L"2023"
#define OM_APP_GIT               L"https://github.com/sedenion/OpenModMan"
#define OM_APP_URL               L"https://github.com/sedenion/OpenModMan"

#define OM_XMAGIC_APP            L"Open_Mod_Manager_Main"
#define OM_XMAGIC_HUB            L"Open_Mod_Manager_ModHub"
#define OM_XMAGIC_CHN            L"Open_Mod_Manager_ModChan"
#define OM_XMAGIC_SPT            L"Open_Mod_Manager_Script"
#define OM_XMAGIC_PKG            L"Open_Mod_Manager_Package"
#define OM_XMAGIC_BCK            L"Open_Mod_Manager_Backup"

#define OM_XMAGIC_REP            L"Open_Mod_Manager_Repository"

#define OM_MODHUB_FILENAME       L"ModHub.xml"
#define OM_MODCHN_FILENAME       L"ModChan.xml"

#define OM_MODHUB_MODPSET_DIR    L".Presets"

#define OM_MODCHAN_BACKUP_DIR    L"\\Backup"
#define OM_MODCHAN_MODLIB_DIR    L"\\Library"

#define OM_MODPACK_THUMB_SIZE    128

// old signatures, used only for migration to new standard
#define OM_XMAGIC_CTX            L"Open_Mod_Manager_Context"
#define OM_XMAGIC_LOC            L"Open_Mod_Manager_Location"
#define OM_XMAGIC_BAT            L"Open_Mod_Manager_Batch"

// old definition files extension, used only for migration to new standard
#define OM_CTX_DEF_FILE_EXT      L"omc"
#define OM_LOC_DEF_FILE_EXT      L"oml"
#define OM_BAT_DEF_FILE_EXT      L"omb"
#define OM_PKG_DEF_FILE_EXT      L"omp"
#define OM_BCK_DEF_FILE_EXT      L"omk"
#define OM_PKG_FILE_EXT          L"ozp"
#define OM_BCK_FILE_EXT          L"ozb"

#endif // OMBASEAPP_H
