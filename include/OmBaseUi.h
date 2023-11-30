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


/// \brief Main menu menu items positions
///
/// Positions constants for main menu
///
#define MNU_FILE            static_cast<unsigned>(0)
    #define MNU_FILE_NEW        0
        #define MNU_FILE_NEW_HUB    0
    // ------------------------ 1
    #define MNU_FILE_OPEN       2
    #define MNU_FILE_RECENT     3
    // ------------------------ 4
    #define MNU_FILE_CLOSE      5
    // ------------------------ 6
    #define MNU_FILE_QUIT       7
#define MNU_EDIT            1
    #define MNU_EDIT_MAN_PROP   0
#define MNU_HUB             2
    #define MNU_HUB_ADDCHN      0
    #define MNU_HUB_ADDPST      1
    // ------------------------ 2
    #define MNU_HUB_PROP        3
#define MNU_CHN             3
    #define MNU_CHN_ADDREP      0
    // ------------------------ 1
    #define MNU_CHN_QRYREP      2
    // ------------------------ 3
    #define MNU_CHN_PROP        4
#define MNU_TOOL            4
    #define MNU_TOOL_EDITPKG    0
    #define MNU_TOOL_EDITREP    1
#define MNU_HELP            5
    #define MNU_HELP_DEBUG      0
    #define MNU_HELP_ABOUT      1

/// \brief Context menu items positions
///
/// Positions constants for right-click context pop-up menus
///
#define POP_CHN             0
    #define POP_CHN_ADD         0
    #define POP_CHN_DEL         1
    // ------------------------ 2
    #define POP_CHN_PROP        3
#define POP_PST             1
    #define POP_PST_RUN         0
    // ------------------------ 1
    #define POP_PST_ADD         2
    #define POP_PST_DEL         3
    // ------------------------ 4
    #define POP_PST_PROP        5
#define POP_REP             2
    #define POP_REP_QRY         0
    // ------------------------ 1
    #define POP_REP_ADD         2
    #define POP_REP_DEL         3
#define POP_MOD             3
    #define POP_MOD_INST        0
    #define POP_MOD_UINS        1
    #define POP_MOD_CLNS        2
    // ------------------------ 3
    #define POP_MOD_DISC        4
    // ------------------------ 5
    #define POP_MOD_OPEN        6
    #define POP_MOD_TRSH        7
    // ------------------------ 8
    #define POP_MOD_EDIT        9
    // ------------------------ 10
    #define POP_MOD_INFO        11
#define POP_NET             4
    #define POP_NET_DNLD        0
    #define POP_NET_DNWS        1
    // ------------------------ 2
    #define POP_NET_STOP        3
    #define POP_NET_RVOK        4
    // ------------------------ 5
    #define POP_NET_FIXD        6
    // ------------------------ 7
    #define POP_NET_INFO        8


#endif // OMBASEUI_H
