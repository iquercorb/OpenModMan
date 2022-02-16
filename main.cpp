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
#include "OmUiMgr.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
  // Check if another instance already running
  HANDLE hMutex = OpenMutexW(MUTEX_ALL_ACCESS, false, L"OpenModMan.Mutex");
  if(hMutex) {

    // search initial instance window
    HWND hWnd = FindWindow("OpenModMan.Class",nullptr);
    if(hWnd) {

      // if program is launched with argument, we pass it
      // to the already existing instance
      size_t len = strlen(lpCmdLine);
      if(len) {

        COPYDATASTRUCT cd;
        cd.dwData = 42;       //< special secret code
        cd.cbData = len + 1;  //< we need the null char
        cd.lpData = lpCmdLine;

        // send special message with argument string to initial instance
        SendMessage(hWnd, WM_COPYDATA, reinterpret_cast<WPARAM>(hWnd), reinterpret_cast<LPARAM>(&cd));
      }

      // Set foreground
      ShowWindow(hWnd, SW_SHOWNORMAL);
      SetForegroundWindow(hWnd);
    }

    // exit now
    return 0;
  }

  // Create new Mutex for single instance check
  hMutex = CreateMutexW(nullptr, true, L"OpenModMan.Mutex");

  OmManager manager;
  OmUiMgr dialog(hInst);

  if(manager.init(lpCmdLine)) {

    dialog.setData(&manager);
    // open main dialog with registered class name
    dialog.registered("OpenModMan.Class", true);

    // Window message loop
    dialog.loopMessage();

    manager.quit();
  }

  // Release "instance" Mutex to allow application to run again
  ReleaseMutex(hMutex);
  CloseHandle(hMutex);

  return 0;
}
