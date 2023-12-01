# Open Mod Mananager

Open source and generic Mod ("Modifications") manager.

## Version: 1.0

## Presentation

Open Mod Manager is an open source and generic Mod manager. Mod term come from
Modification and refers to one or several unofficial files which are added to, or
replace original editor's files of a software (usually a game) to change its behaviour,
add features, components or items.

While these Mods's files can be manually installed and origina files backed by the user,
Open Mod Manager provide a safe, automated and advanced environement to store, install,
back and restore original files, and create Mods as encapsuled packages.

### Modern package management

Cumulative installations, backup then restoration of original files are key feature,
Open Mod Manager embed safe and smart automatic backup mechanism to garantish user can
always and at any time restore editor's original files. Finally, as any modern package
manager, Open Mod Manager support versioning and dependencies mechanisms.

### Network oriented

Since most of mods are provided by users or communities, and downloaded by other, Open
Mod Manager provides an open online repository mechanism allowing any community or
individual to provides its encapsuled packages within public online repositories. Users
can configure repositories to check for updates or new availables packages from their
favorite communities.

### Generic and versatile

Open Mod Mod manager is not focused to work with a specific game or software, it is in
contrary designed to be generic, very flexible, and to adapt to the most configurations
as possible and even for what was not though in the start. The configuration
architecture is modular, stored as human readable XML files. Editing merging or
deleting configuration modules can be as simple as - litteraly - moving subfolders
from one location to another.

## Features

Open Mod Mananager currently implements the following key features:

- Flexible and modular XML and folder configuration logic.
- Multiple installation destinations per configuration context.
- Custom folders for packages (Mods) library and backup per installation destination.
- Support for both folders (legacy) Mods and advanced zipped Packages.
- Smart installation and backup mechanism allowing overlapped installed files.
- Packages installation dependencies mechanism (advanced packages).
- Backup data compressed as zip archives to save space (optional).
- Batch mechanism for automatic Package installation sequences.
- Network repositories for remote packages distribution and download.
- Automatic package dependencies download and fix system.

## Screenshot

![Open Mod Mananager Screenshot](screenshot.png)

## Want to support Open Mod Manager developper ?

<a href="https://www.patreon.com/bePatron?u=89686595" data-patreon-widget-type="become-patron-button">Become a member!</a><script async src="https://c6.patreon.com/becomePatronButton.bundle.js"></script>

## Version history

1.1.1 (2023-10-29)

- Fix inconsistent and buggy file size handling for 32 bit version
- Fix very slow XXHash digest computing, now way faster
- Fix application crash when remove package
- Removed Modding Hub "Uinstall All" feature
- Adding Per-Package "Discard backup data" feature
- Adding partial or aborted download resume mechanism

1.1 (2023-09-11)
 - Includes all cumulatives changes and fixes of 1.0.x hotfixes, including:
 - Fix application crash after download for package upgrade
 - Fix wrong behavior on package upgrade dialog message
 - Fix remote package update inconsistencies when refresh repositories
 - Fix critical bug in remote package refresh process
 - Fix Package Editor inconsistent prompts for unsaved changes
 - Fix Package Editor destination path set to default again even if not empty
 - Fix window geometry save and restore mechanism
 - Automatic README text now generated only if no description is provided
 - Changes Packages download and upgrade default behavior
 - Add support for detecting and skipping hidden files
 - Added contextual menu for Installation Batches List

1.0 (2022-05-16)
 - Includes all cumulatives changes and fixes of 0.9.9.x hotfixes, including:
 - Fix and optimize markdown rendering
 - Fix loading JPEG files may cause application to crash
 - Fix and optimized image processing
 - Fix deleting package may crash application
 - Fix Repository Package parse failing due to 'xxhsum' tag not properly handled
 - New support of paths with non-ANSI characters for ZIP file I/O
 - Fix download low performances on large files over high-speed network
 - Added bypass of access denied for network folders which access cannot be properly evaluated
 - Fixed newly created Software Context not properly selected after Wizard dialog closed
 - Fix potential memory leaks when closing software context
 - Update embedded libcurl library to version 7.83.1
 - Update embedded miniz library to version 2.2.0

0.9.9 (2022-02-19)
 - Fix command shortcuts still working when main dialog is not active.
 - Fix scrolling for Overlap and Dependencies in Package Properties Dialog.
 - Fix missing dialogs for package download errors.
 - Fix Package Editor bad encoding of loaded/saved description text.
 - Fix Package Editor not detected file extension when open an existing package.
 - Fix Network Repository description never saved locally.
 - Update embedded XXhash algorithm to the 0.8.1 version.
 - Update embedded Pugixml library to the 1.12.1 version.
 - New Uninstall tree - uninstall with dependencies - feature.
 - New MD5 (md5sum) alogirthm support for repository files checksum.
 - New Category field for Packages.
 - New custom path or URL mechanism for repository files download.
 - New improved message boxes and icons.
 - New Installation Batch "Install-Only" execution mode.
 - New ability to associate description and thumbnail for dev/folder packages.
 - New improved UI with resizable frames.
 - New Markdown parsing support for package description.

0.9.8 (2022-01-04)
 - Add support and automatic follow of HTTP redirect responses.
 - Fix Network ListView columns click wrongly pops up contextual menu.
 - Fix Network ListView not properly redrawn after item sorting.
 - Fix Network Abort button not properly enabled at download start.
 - Fix Package Editor not properly clear properties of modified existing package.
 - New menu and shortcut to load seleted package in Package Editor.
 - Fix not working delete repository button of Network tab.
 - Fix overly long warning messages with huge packages list.

0.9.7 (2021-07-18)
 - Developer mode back as enabled by default.
 - Fix undo install not working with archive files backup.
 - Package created folders no longer generate overlapping.

0.9.6 (2021-07-07)
 - Overall optimization and various debug.
 - Repaired broken Context Wizard dialog.
 - Fix Load Contexts At Startup option dialog not saving properly.
 - New Tools menu for editors.
 - New improved Package editor dialog (moved in Tools menu).
 - New support for .omp extension file as Open Mod Manager Package.
 - Fix Batch properties crash if package no longer available.
 - Batch properties now ask user to clean Batch in case of invalid package reference.
 - Automatic fix Batch inconsistency in case of Context's Location changes.
 - Fix Batch execution thread not properly terminated.
 - Consolidated package algorithms for robust dependencies and overlaps management.
 - Setup now configure the system-wide application files and icons association.
 - Now only one running instance allowed to prevent conflicting usages.
 - New ability to run application and open Context files (.omc) by double-click on it.
 - Moved dev mode and warnings parameters from Manager to Location.
 - New Network repositories feature.
   - New Repository editor to create server side repository definition files.
   - New Location's Network tab to setup client side repositories.
   - New Main Network tab to query online repositories and download remote packages.
   - Automatic dependencies download and fix system.

0.9.5 (2021-05-22)
 - Fix Library list-view scroll position reset to top on refresh.
 - New buttons and dialogs for Batches edition
 - New Discard backups data feature for emergency situations
 - Edit menu modified for quicker access properties

0.9.4 (2020-11-26)
 - Modify Package creation function to add picture file as provided.
 - Fix Package creation dialog crash if no Context loaded.
 - Fix various typo for "Uninstall".
 - Fix Load Contexts At Startup option dialog not saving properly.

0.9.3 (2020-07-21)
 - Update pugixml API to version 1.10
 - Fixed new Context Wizzard custom library and bakcup folder incorrectly created.
 - Fixed multiple bugs in various dialogs.

0.9.2 (2020-07-15)
 - Rewritten Package building algorithm.
 - Fixed Package "Save As" button never enabling.
 - Replace GPL text by credits in the "About" dialog.
 - Rewritten file/directory Access-Control check mechanisms.
 - Improved Unicode and non-ASCII characters support.
 - Fixed incorrect packages list refresh after move to trash.

0.9.1 (2020-07-11)
 - Fixed random string bug and crash caused by buffer overflow.
 - Fixed Context creation wizard not properly reseted after cancel by user.
 - Fixed missing application informations and icons within Windows Application list.
 - Algorithm to check file and folder access permissions slightly modified.

0.9.0 (2020-07-08)
 - First public BETA release
