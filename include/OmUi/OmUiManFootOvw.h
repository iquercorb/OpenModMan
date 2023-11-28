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
#ifndef OMUIMGRFOOTOVW_H
#define OMUIMGRFOOTOVW_H

#include "OmDialog.h"

class OmModPack;
class OmNetPack;
class OmVersion;
class OmImage;

/// \brief Main window - Description Tab child
///
/// OmDialog class derived for Package Description tab child dialog window of
/// the Main Window Foot frame.
///
class OmUiManFootOvw : public OmDialog
{
  public: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiManFootOvw(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiManFootOvw();

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

    /// \brief Set Mod preview
    ///
    /// Defines Mod preview informations to show
    ///
    /// \param[in]  ModPack    : Pointer to Mod Pack object.
    ///
    void setPreview(OmModPack* ModPack);

    /// \brief Set Network Mod preview
    ///
    /// Defines Network Mod preview informations to show
    ///
    /// \param[in]  NetPack    : Pointer to Network Mod Pack to view.
    ///
    void setPreview(OmNetPack* NetPack);

    /// \brief Clear package preview
    ///
    /// Clears package preview informations and hide controls
    ///
    void clearPreview();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmUiMan*            _UiMan;

    void                _overview_populate(const OmWString&, const OmVersion&, const OmImage&, const OmWString&, bool);

    void                _desc_set_text(const OmWString& text);

    void                _ft_desc_on_link(LPARAM lParam);

    void                _onInit();

    void                _onShow();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    INT_PTR             _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIMGRFOOTOVW_H
