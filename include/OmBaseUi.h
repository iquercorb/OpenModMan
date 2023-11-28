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
#ifndef OMBASEUI_H
#define OMBASEUI_H

#include "../res/resource.h"

enum OmLvIconsId : int
{
  ICON_NONE = -1,
  ICON_STS_CHK = 0,
  ICON_STS_ERR = 1,
  ICON_STS_WRN = 2,
  ICON_STS_WIP = 3,
  ICON_STS_OVR = 4,
  ICON_STS_NEW = 5,
  ICON_STS_UPG = 6,
  ICON_STS_DNG = 7,
  ICON_STS_DNL = 8,
  ICON_STS_RES = 9,
  ICON_STS_QUE = 10,
  ICON_STS_RB0 = 11,
  ICON_STS_RB1 = 12,
  ICON_MOD_DIR = 13,
  ICON_MOD_PKG = 14,
  ICON_MOD_DEP = 15,
  ICON_MOD_ERR = 16,
  ICON_CHN = 17,
  ICON_PST = 18,
  ICON_REP = 19
};

#define OM_LISTVIEW_ICON_COUNT       20

inline void Om_setupLvIconsDb(unsigned* icon_db)
{
  icon_db[ICON_STS_CHK] = IDB_STS_CHK_16;
  icon_db[ICON_STS_ERR] = IDB_STS_ERR_16;
  icon_db[ICON_STS_WRN] = IDB_STS_WRN_16;
  icon_db[ICON_STS_WIP] = IDB_STS_WIP_16;
  icon_db[ICON_STS_OVR] = IDB_STS_OVR_16;
  icon_db[ICON_STS_NEW] = IDB_STS_NEW_16;
  icon_db[ICON_STS_UPG] = IDB_STS_UPG_16;
  icon_db[ICON_STS_DNG] = IDB_STS_DNG_16;
  icon_db[ICON_STS_DNL] = IDB_STS_DNL_16;
  icon_db[ICON_STS_RES] = IDB_STS_RES_16;
  icon_db[ICON_STS_QUE] = IDB_STS_QUE_16;
  icon_db[ICON_STS_RB0] = IDB_STS_RB0_16;
  icon_db[ICON_STS_RB1] = IDB_STS_RB1_16;
  icon_db[ICON_MOD_DIR] = IDB_PKG_DIR_16;
  icon_db[ICON_MOD_PKG] = IDB_PKG_ZIP_16;
  icon_db[ICON_MOD_DEP] = IDB_PKG_DPN_16;
  icon_db[ICON_MOD_ERR] = IDB_PKG_ERR_16;
  icon_db[ICON_CHN]     = IDB_CHN_16;
  icon_db[ICON_PST]     = IDB_PST_16;
  icon_db[ICON_REP]     = IDB_REP_16;
}

#endif // OMBASEUI_H
