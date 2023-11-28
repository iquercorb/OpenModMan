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
#ifndef OMUIMGRFOOTDET_H
#define OMUIMGRFOOTDET_H

#include "OmDialog.h"

class OmModPack;
class OmNetPack;

/// \brief Main window - Description Tab child
///
/// OmDialog class derived for Package Description tab child dialog window of
/// the Main Window Foot frame.
///
class OmUiManFootDet : public OmDialog
{
  public: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiManFootDet(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiManFootDet();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Set dialog install mode
    ///
    /// Enable or disable the dialog install mode.
    ///
    /// The install mode is a modal-kind emulation for threaded operations,
    /// it disables (almost) all controls of the dialog and its children
    /// to prevent user to interact with elements while a threaded process
    /// is running.
    ///
    /// \param[in]  enable  : Enable or disable freeze mode.
    ///
    void freeze(bool enable);

    /// \brief Set dialog safe mode
    ///
    /// Enables or disable the dialog safe mode.
    ///
    /// The safe mode is used to operate modifications on sensitive
    /// or monitored elements such as deleting or moving Mod Channel in
    /// order to prevent conflicts or crash during process.
    ///
    /// \param[in]  enable  : Enable or disable safe mode.
    ///
    void safemode(bool enable);

    /// \brief Display Mod details
    ///
    /// Displays details of the specified Mod.
    ///
    /// \param[in]  ModPack  : Pointer to Mod Pack object
    ///
    void setDetails(OmModPack* ModPack);

    /// \brief Display network Mod details
    ///
    /// Displays details of the specified network Mod.
    ///
    /// \param[in]  NetPack  : Pointer to Network Mod Pack object
    ///
    void setDetails(OmNetPack* NetPack);

    /// \brief Clear details view
    ///
    /// Clears the detail view
    ///
    void clearDetails();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiMan*            _UiMan;

    void                _desc_set_text(const OmWString& text);

    void                _ft_desc_on_link(LPARAM lParam);

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMGRFOOTDET_H
