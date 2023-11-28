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
#ifndef OMBASEWIN_H
#define OMBASEWIN_H

#define WINVER          0x0600  //< _WIN32_WINNT_VISTA
#define NTDDI_VERSION   0x6000000  //< NTDDI_VISTA

#include <Windows.h>

/// \brief Thread run.
///
/// Generic thread process function to be called at thread start
///
/// \param[in]  ptr     : User data pointer
///
/// \return Result value.
///
typedef DWORD _stdcall (*Om_threadProc)(void* ptr);

/// \brief Wait thread callback.
///
/// Generic wait callback function for wait for thread end function.
///
/// \param[in]  ptr     : User data pointer
///
typedef void _stdcall (*Om_waitCb)(void* ptr, unsigned char fired);

/// \brief Create thread
///
/// Create a new thread managed by the system
///
/// \param[in] thread_proc  : Process function to be called at thread start.
/// \param[in] user_ptr     : Custom pointer to be passed to run function.
///
/// \return System Handle to thread.
///
inline HANDLE Om_createThread(Om_threadProc thread_proc, void* user_ptr)
{
  return CreateThread(NULL, 0, thread_proc, user_ptr, 0, NULL);
}

/// \brief Wait thread end
///
/// Register a wait for thread handle and call callback once thread ended.
///
/// \param[in] hthread    : Handle to thread to wait for end
/// \param[in] wait_cb    : Callback function called at thread end
/// \param[in] user_ptr   : Custom pointer to be passed to callback function.
///
/// \return System Handle to registered wait.
///
inline HANDLE Om_waitForThread(HANDLE hthread, Om_waitCb wait_cb, void* user_ptr)
{
  HANDLE hNewWaitObject;
  RegisterWaitForSingleObject(&hNewWaitObject, hthread, wait_cb, user_ptr,
                              INFINITE, WT_EXECUTEONLYONCE);
  return hNewWaitObject;
}

/// \brief Clear thread objects
///
/// Close the given thread object and unregister the given wait thread.
///
/// \param[in] hthread    : Handle to thread to wait for end
/// \param[in] hwait      : Handle to wait for thread process
///
inline void Om_clearThread(HANDLE hthread, HANDLE hwait)
{
  if(hthread)
    CloseHandle(hthread);

  if(hwait)
    UnregisterWait(hwait);
}

/// \brief Get thread exit code
///
/// Retrieve and returns the exit code of the given thread
///
/// \param[in] hthread    : Handle to thread to get exit code
///
/// \return Thread exit code
///
inline DWORD Om_threadExitCode(HANDLE hthread)
{
  DWORD exitCode;
  GetExitCodeThread(hthread, &exitCode);
  return exitCode;
}

#endif // OMBASEWIN_H
