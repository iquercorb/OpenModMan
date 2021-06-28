----------------------------------------------------------------------------------------
Open Mod Mananager
----------------------------------------------------------------------------------------

Open source and generic Mod ("Modifications") manager.


Version: 0.9.5 (beta)
----------------------------------------------------------------------------------------


Presentation
----------------------------------------------------------------------------------------
The purpose of Open Mod Manager is to provide an easy and safety way to organize,
install, uninstall and create generic packages of third-party tweaks or add-ons (also
called "mod") to be applied to any software (usually games).

It provide an advanced "mod" management using "packages" paradigm with support for
dependencies and advanced automatic backup logics to prevent software's original files
corruption.

Accent is made to flexibility and modularity by providing an open configuration logics
based on human-readable XML files and simple folders architecture witch can be easily
saved, deleted or altered.


Features
----------------------------------------------------------------------------------------
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

Screenshots
----------------------------------------------------------------------------------------
![Open Mod Mananager Screenshot](screenshot.png)

Version history
----------------------------------------------------------------------------------------

0.9.6 (not released)
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
