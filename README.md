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
