# C-Menu ROADMAP

## 0.2.9 - Pre-Release

Menu, Form, Pick, View, and General features are still being developed and de-bugged.When the features are ready, the version will be updated to 1.0.0 and the roadmap will be updated with the new features and enhancements.

There are a few more features that will be added prior to the 1.0.0 release, but they are not yet finalized and may change based on user feedback and testing.

### Form

- Form Pop-ups. View currently uses a Form pop-up for entering file names, but that capability needs to be polished and expanded to be more flexible and user-friendly. The idea is to have a simple function call with minimal parameters to create a Form pop-up. The function should default to sane values, but allow styling and layout to be customizable.

- Currently, data can be communicated via command-line arguments, pipes, or files, but a facility is needed for handling asynchronous submission over a network. This will be added in a future release.

### Pick

- Pick pop-ups and drop-downs. Pic currently supports communication via command-line arguments, pipes, and files, but it needs a more user-friendly interface for selecting options. The idea is to have a simple function call with minimal parameters to create a Pick pop-up or drop-down. The function should default to sane values, but allow styling and layout to be customizable.

### Exercises - Example Use Cases

- This may be more of a learning than a teaching tool. The idea is to have a set of exercises that demonstrate how to use the various features of C-Menu in different scenarios. These exercises will be designed to be simple and easy to follow, and will cover a range of use cases from basic to advanced. While developing these exercises, We, I and any collaborators who volunteer to help, will be looking for feedback from users to ensure that they are useful and relevant. There is no doubt that developing these exercises will expose areas of C-Menu that need improvement, and we will be using that feedback to make C-Menu better for everyone.

Ideas for exercises include:

- UI for sqlite3 General Ledger, A/R, or Payables

- UI for NetworkManager like nmtui

- UI for systemctl, journalctl, and other systemd utilities

- UI for System Troubleshooting and Maintenance

- UI for Update-Alternatives

- UI for Rescue CD

- UI for Linux From Scratch Installation

- Plugins for Neovim, Vim, Emacs, and other text editors

- UI for Server and Embedded System Management

- UI for managing IOT devices

### View

- View is looking pretty good at this point. The line number table was a major improvement, and not just for the pretty display. In conjunction with memory mapped file I-O, the line number table dramatically simplifies internal navigation and boosts performance to obscene levels.

There is one improvement I would like to make, and that is to add multi-threading support to populate the line number table in the background while the user is viewing the file. That would involve creating a separate thread with a separate buffer that won't interfere with the main thread's use of it's buffer. That should be no problem as both buffers would be read-only. No one has complained, so this may be vanity work, but it would be a fun challenge.

## 1.0.0 - Initial Release

### General

- JSON - Look at MaterialUI

## 1.1.0 - Feature Enhancements

### General

- Consider a snap-on GUI front end.
