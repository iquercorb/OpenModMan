/*
  This file is part of Open Mod Manager.

  void Mod OmManager is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  void Mod OmManager is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Open Mod Manager. If not, see <http://www.gnu.org/licenses/>.
*/

#include "OmGlobal.h"
#include "OmVersion.h"
#include "OmManager.h"
#include "gui/OmUiMain.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, char* argv, int nShowCmd)
{
  InitCommonControls();

  OmManager manager;
  OmUiMain dialog(hInst);

  if(manager.init()) {

    dialog.setData(&manager);
    dialog.open(true);

    // Window message loop
    dialog.loopMessage();

    manager.quit();
  }

  return 0;
}
