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
#include "OmBase.h"
#include "OmUtilAlg.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmDirNotify.h"

/// Routine to extract FILE_NOTIFY_INFORMATION Filename to proper
/// null-terminated string and indicating whether it is subtree item
static inline bool __get_item_name(wchar_t* dst, const wchar_t* src, size_t src_len)
{
  // supplied length is in bytes count
  size_t num_char = src_len / sizeof(wchar_t);

  size_t len;

  for(len = 0; len < num_char; ++len) {

    // we stop at the first '\' indicating this is subitem, we
    // keep only the direct child, no subitem
    if(src[len] == L'\\') {
      dst[len] = 0;
      return true;
    }

    dst[len] = src[len];
  }

  dst[len] = 0;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDirNotify::OmDirNotify() :
  _notify_cb(nullptr),
  _user_ptr(nullptr),
  _stop_hev(nullptr),
  _changes_hth(nullptr),
  _access_check_hth(nullptr),
  _access_check_hwo(nullptr)
{
  //ctor
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDirNotify::~OmDirNotify()
{
  this->stopMonitor();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDirNotify::setCallback(Om_notifyCb notify_cb, void* user_ptr)
{
  this->_notify_cb = notify_cb;
  this->_user_ptr = user_ptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDirNotify::stopMonitor()
{
  // set 'stop' event
  SetEvent(this->_stop_hev);

  if(this->_access_check_hth) {
    // wait for threads to quit
    WaitForSingleObject(this->_access_check_hth, INFINITE);
    this->_added_queue.clear();
  }

  if(this->_changes_hth) {
    // wait for threads to quit
    WaitForSingleObject(this->_changes_hth, INFINITE);
    CloseHandle(this->_changes_hth);
    this->_changes_hth = nullptr;
    this->_modif_queue.clear();
  }

  #ifdef DEBUG
  std::wcout << L"DEBUG => OmDirNotify::stopMonitor (" << this->_path << ")\n";
  #endif

  this->_path.clear();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDirNotify::startMonitor(const OmWString& path)
{
  this->_path = path;

  #ifdef DEBUG
  std::wcout << L"DEBUG => OmDirNotify::startMonitor (" << path << L")\n";
  #endif

  if(this->_access_check_hth || this->_changes_hth)
    this->stopMonitor();

  // create or reset custom 'stop' event
  if(this->_stop_hev) {
    ResetEvent(this->_stop_hev);
  } else {
    this->_stop_hev = CreateEvent(nullptr, true, false, nullptr);
  }

  this->_changes_hth = Om_threadCreate(OmDirNotify::_changes_run_fn, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmDirNotify::_changes_run_fn(void* ptr)
{
  OmDirNotify* self = static_cast<OmDirNotify*>(ptr);

  // get handle to library directory
  HANDLE hDirectory = CreateFileW(self->_path.c_str(),
                                  GENERIC_READ,
                                  FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,
                                  nullptr,
                                  OPEN_EXISTING,
                                  FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED, 0);


  // Overlapped structure with event
  OVERLAPPED Overlapped;
  Overlapped.hEvent = CreateEvent(nullptr, false, false, nullptr);

  // Array of events to be waited for, the custom 'stop' event at the second position
  HANDLE hEvents[] = {Overlapped.hEvent, self->_stop_hev};

  // notify filter for read changes
  DWORD NotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE;

  // buffer for received notifications
  uint8_t FileNotifyBuf[512000];

  // Request for changes in directory
  ReadDirectoryChangesW(hDirectory, FileNotifyBuf, 512000, true, NotifyFilter, nullptr, &Overlapped, nullptr);

  // Buffer for file name
  wchar_t FileName[OM_MAX_PATH];

  // Flag for subtree item notification
  bool is_subitem = false;

  OmWString FilePath;

  while(true) {

    DWORD result = WaitForMultipleObjects(2, hEvents, false, INFINITE);

    if(result == 1) //< custom stop object
      break;

    if(result == 0) { //< event received

      #ifdef DEBUG
      std::wcout << L"DEBUG => OmDirNotify : CHANGES\n";
      #endif

      DWORD bytesTransferred;

      GetOverlappedResult(hDirectory, &Overlapped, &bytesTransferred, false);

      FILE_NOTIFY_INFORMATION* Notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(FileNotifyBuf);

      while(true) {

        // We extract item filename from notify structure. We extract only the direct child
        // of the tracked folder, removing any subtree items from the path. If notification
        // is about subtree item the function return true, so we can make specific operations
        // for such case.
        is_subitem = __get_item_name(FileName, Notify->FileName, Notify->FileNameLength);

        Om_concatPaths(FilePath, self->_path, FileName);

        switch(Notify->Action)
        {
        case FILE_ACTION_ADDED:

          if(is_subitem) {

            // If notification is about subtree item, we treat it as modification of its parent
            Om_push_backUnique(self->_modif_queue, FilePath);

          } else {

            self->_access_check_add(FilePath);
          }

          break;

        case FILE_ACTION_REMOVED:

          #ifdef DEBUG
          std::wcout << L"DEBUG => OmDirNotify : FILE_ACTION_REMOVED (" << FilePath << L")\n";
          #endif // DEBUG

          if(is_subitem) {

            // If notification is about subtree item, we treat it as modification of its parent
            Om_push_backUnique(self->_modif_queue, FilePath);

          } else {

            if(self->_notify_cb)
              self->_notify_cb(self->_user_ptr, OM_NOTIFY_DELETED, reinterpret_cast<uint64_t>(FilePath.c_str()));
          }

          break;

        case FILE_ACTION_MODIFIED:

            if(!Om_arrayContain(self->_added_queue, FilePath))
              Om_push_backUnique(self->_modif_queue, FilePath);

          break;

        case FILE_ACTION_RENAMED_OLD_NAME:

          #ifdef DEBUG
          std::wcout << L"DEBUG => OmDirNotify : FILE_ACTION_RENAMED_OLD_NAME (" << FileName << L")\n";
          #endif // DEBUG

          if(is_subitem) {

            // If notification is about subtree item, we treat it as modification of its parent
            Om_push_backUnique(self->_modif_queue, FilePath);

          } else {

            if(self->_notify_cb)
              self->_notify_cb(self->_user_ptr, OM_NOTIFY_DELETED, reinterpret_cast<uint64_t>(FilePath.c_str()));
          }

          break;

        case FILE_ACTION_RENAMED_NEW_NAME:

          #ifdef DEBUG
          std::wcout << L"DEBUG => OmDirNotify : FILE_ACTION_RENAMED_NEW_NAME (" << FileName << L")\n";
          #endif // DEBUG

          // In case of file rename, the system send both FILE_ACTION_RENAMED_NEW_NAME
          // and FILE_ACTION_ADDED, so to prevent concurrency bug due to notification
          // called twice from two threads, we add to queue like another file creation,
          // verifying it is not already in queue.

          if(is_subitem) {

            // If notification is about subtree item, we treat it as modification of its parent
            Om_push_backUnique(self->_modif_queue, FilePath);

          } else {

            self->_access_check_add(FilePath);
          }

          break;
        }

        if(Notify->NextEntryOffset == 0)
          break;

        Notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<uint8_t*>(Notify) + Notify->NextEntryOffset);
      }

      // Flushes the modification queue, effectively sending notification to client
      size_t i = self->_modif_queue.size();
      while(i--) {

        #ifdef DEBUG
        std::wcout << L"DEBUG => OmDirNotify : File Mod (" << self->_modif_queue[i] << L")\n";
        #endif // DEBUG

        if(self->_notify_cb)
          self->_notify_cb(self->_user_ptr, OM_NOTIFY_ALTERED, reinterpret_cast<uint64_t>(self->_modif_queue[i].c_str()));

        self->_modif_queue.erase(self->_modif_queue.begin() + i);
      }
    }

    ReadDirectoryChangesW(hDirectory, FileNotifyBuf, 512000, false, NotifyFilter, nullptr, &Overlapped, nullptr);
  }

  // close handle to directory
  CloseHandle(hDirectory);

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDirNotify::_access_check_add(const OmWString& path)
{
  Om_push_backUnique(this->_added_queue, path);

  // start thread
  if(!this->_access_check_hth) {
    this->_access_check_hth = Om_threadCreate(OmDirNotify::_access_check_run_fn, this);
    this->_access_check_hwo = Om_threadWaitEnd(this->_access_check_hth, OmDirNotify::_access_check_end_fn, this);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmDirNotify::_access_check_run_fn(void* ptr)
{
  #ifdef DEBUG
  std::wcout << L"DEBUG => OmDirNotify::_access_check_run_fn : enter\n";
  #endif // DEBUG

  OmDirNotify* self = static_cast<OmDirNotify*>(ptr);

  HANDLE hFile;
  DWORD FileAttr;
  DWORD Flags;

  while(self->_added_queue.size()) {

    // test added files true availability, so we are sure once notification is sent the file
    // is available for read/write operation
    size_t i = self->_added_queue.size();
    while(i--) {

      FileAttr = GetFileAttributesW(self->_added_queue[i].c_str());

      // If file does not exist, remove it from queue
      if(FileAttr == INVALID_FILE_ATTRIBUTES) {
        #ifdef DEBUG
        std::wcout << L"DEBUG => OmDirNotify::_access_check_run_fn : removed from invalid file (" << self->_added_queue[i] << L")\n";
        #endif // DEBUG
        self->_added_queue.erase(self->_added_queue.begin() + i);
        continue;
      }

      if(FileAttr & FILE_ATTRIBUTE_DIRECTORY) {
        Flags = FILE_FLAG_BACKUP_SEMANTICS;
      } else {
        Flags = 0;
      }

      hFile = CreateFileW(self->_added_queue[i].c_str(),
                          GENERIC_READ|GENERIC_WRITE,
                          0, //< try to take exclusive control to ensure file is fully available
                          nullptr,OPEN_EXISTING,Flags,nullptr);

      if(hFile != INVALID_HANDLE_VALUE) {

        CloseHandle(hFile);

        #ifdef DEBUG
        std::wcout << L"DEBUG => OmDirNotify : File Add (" << self->_added_queue[i] << L")\n";
        #endif // DEBUG

        if(self->_notify_cb)
          self->_notify_cb(self->_user_ptr, OM_NOTIFY_CREATED, reinterpret_cast<uint64_t>(self->_added_queue[i].c_str()));

        self->_added_queue.erase(self->_added_queue.begin() + i);
      }
    }

    // Check for 'stop' event, wait 50 MS each loop to prevent flood of 'CreateFileW'
    if(0 == WaitForSingleObject(self->_stop_hev, 50))
      break;
  }

  #ifdef DEBUG
  std::wcout << L"DEBUG => OmDirNotify::_access_check_run_fn : leave\n";
  #endif // DEBUG

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmDirNotify::_access_check_end_fn(void* ptr, uint8_t fired)
{
  #ifdef DEBUG
  std::wcout << L"DEBUG => OmDirNotify::_access_check_end_fn\n";
  #endif // DEBUG

  OM_UNUSED(fired);

  OmDirNotify* self = static_cast<OmDirNotify*>(ptr);

  Om_threadClear(self->_access_check_hth, self->_access_check_hwo);

  // An item may be added in the queue after exiting the loop but before thread
  // closing so if item remain in queue, we restart the thread right now.
  if(self->_added_queue.size()) {
    self->_access_check_hth = Om_threadCreate(OmDirNotify::_access_check_run_fn, self);
    self->_access_check_hwo = Om_threadWaitEnd(self->_access_check_hth, OmDirNotify::_access_check_end_fn, self);
  } else {
    self->_access_check_hth = nullptr;
    self->_access_check_hwo = nullptr;
  }
}
