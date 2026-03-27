# C-Menu ROADMAP

## 0.3.0 On the Horizon

### Asynchronous Communication and Serialization

- Currently, C-Menu can communicate via command-line arguments, pipes, or files, but a facility is needed for handling asynchronous submission over a network. One fairly simple way to get started would be to set up an MQTT broker.

- As C-Menu continues to employ more sophisticated technologies, the old text file configurations are wearing thin. I would like to move the information in the description files to a more structured format, such as JSON Forms. It looks like JSON is the standard, but I would like to have some input from users.

- Why not develop a C-Menu system to manage IOT devices. IOT uses M2M communication, generally over MQTT.

## 0.2.9 Wrap-Up

- Menu, Form, Pick, and View Pop-ups. View currently uses a Form pop-up for entering file names. That capability is being polished and expanded to be more flexible and user-friendly. The idea is to have simple function calls with minimal parameters to easily create pop-up's. Once we have that debugged and working well, we will expand the ability to create pop-ups in the description files.

- Menu, Form, Pick, View, and CKeys have been combined into a single executable. The individual executables were running about 150K, and the single executable is only 158K. The individual executables have been removed from the repository. You can create form, pick, view, and ckeys as symbolic links to "menu" and from the command line, they will work exactly as the individual executables did before.

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
