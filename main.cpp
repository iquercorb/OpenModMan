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
#include "OmManager.h"
#include "gui/OmUiMain.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
  HANDLE hMutex = OpenMutexW(MUTEX_ALL_ACCESS, false, L"OpenModMan.Instance");

  // Check if another instance already running
  if(hMutex) {

    wstring msg = L"Only one instance of Open Mod Manager is allowed to run.";
    Om_dialogBoxWarn(nullptr, L"Open Mod Manager already running", msg);

    // search initial instance window
    HWND hWnd = FindWindow(nullptr,"Open Mod Manager");
    if(hWnd) {

      // Set foreground
      ShowWindow(hWnd, SW_SHOWNORMAL);
      SetForegroundWindow(hWnd);
    }

    // exit now
    return 0;

  }

  // Create new Mutex for single instance check
  hMutex = CreateMutexW(nullptr, true, L"OpenModMan.Instance");


  InitCommonControls();

  curl_global_init(CURL_GLOBAL_ALL);

  OmManager manager;
  OmUiMain dialog(hInst);

  if(manager.init(lpCmdLine)) {

    dialog.setData(&manager);
    dialog.open(true);

    // Window message loop
    dialog.loopMessage();

    manager.quit();
  }

  // Release Mutex as application end
  ReleaseMutex(hMutex);
  CloseHandle(hMutex);

  return 0;
}
