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
#ifndef OMDIRNOTIFY_H
#define OMDIRNOTIFY_H

#include "OmBase.h"
#include "OmBaseWin.h"

/// \brief Directory monitoring class
///
/// Class to provide directory changes monitoring
///
class OmDirNotify
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmDirNotify();

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmDirNotify();

    /// \brief Set notification callback
    ///
    /// Set callback function and custom pointer for changes notifications
    ///
    /// In this context the \c param value passed to callback is a pointer to
    /// file or directory path string concerned by changes.
    ///
    /// \param[in] notify_cb  : Callback for changes notifications
    /// \param[in] user_ptr   : Custom pointer passed to callback
    ///
    void setCallback(Om_notifyCb notify_cb, void* user_ptr);

    /// \brief Start monitoring
    ///
    /// Starts the monitoring of the directory specified in path.
    ///
    /// \param[in] path       : Path to directory to monitor
    ///
    void startMonitor(const OmWString& path);

    /// \brief Stop monitoring
    ///
    /// Stop the currently running monitoring if any.
    ///
    void stopMonitor();

    /// \brief Check whether is monitoring
    ///
    /// Returns whether this instance is currently monitoring a directory
    ///
    /// \return True if directory monitoring, false otherwise
    ///
    bool isMonitoring() const {
      return (this->_changes_hth != nullptr);
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmWString             _path;

    Om_notifyCb           _notify_cb;

    void*                 _user_ptr;


    OmWStringArray        _added_queue;

    OmWStringArray        _modif_queue;


    void*                 _stop_hev;


    void*                 _changes_hth;

    static DWORD WINAPI   _changes_run_fn(void*);


    void*                 _access_check_hth;

    void*                 _access_check_hwo;

    void                  _access_check_add(const OmWString& path);

    static DWORD WINAPI   _access_check_run_fn(void*);

    static VOID WINAPI    _access_check_end_fn(void*,uint8_t);

};

#endif // OMDIRNOTIFY_H
