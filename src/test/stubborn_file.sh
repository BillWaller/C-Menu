#!/bin/bash

# Stubborn File Creator Script
# Use with caution!
# To stop the script, you may need to reboot your system.
# Run this script with root privileges.
# Example: sudo bash stubborn_file.sh
# To delete the file, you may need to boot into a live environment or use recovery mode.
# File name
STUBBORN_FILE="/tmp/stubborn_file.txt"
# Create the stubborn file
# Create the file
echo "This is a stubborn file. Deleting it will be a challenge!" >"$STUBBORN_FILE"
# Set immutable attribute
chattr +i "$STUBBORN_FILE"
# Create a loop to continuously recreate the file if deleted
(
    while true; do
        if [ ! -f "$STUBBORN_FILE" ]; then
            echo "Recreating the stubborn file..."
            echo "This is a stubborn file. Deleting it will be a challenge!" >"$STUBBORN_FILE"
            chattr +i "$STUBBORN_FILE"
        fi
        sleep 5
    done
) &
# Inform the user
echo "Stubborn file created at $STUBBORN_FILE"
echo "To delete it, you will need to remove the immutable attribute first:"
echo "sudo chattr -i $STUBBORN_FILE"
echo "Then you can delete the file:"
echo "sudo rm $STUBBORN_FILE"
echo "Note: You may need to stop the background process or reboot your system to"
echo "fully remove it."
echo "Use this script responsibly!"
# ------------------------------------------------------------------
# #!/bin/bash
# Cleanup Instructions
# to stop: reboot system
# to delete: sudo chattr -i /tmp/stubborn_file.txt && sudo rm /tmp/stubborn_file.txt
# be careful with this script!
#
# Let's see if we can automate the deletion process safely.
# We could create a cleanup script that removes the stubborn file and kills
# the background process. This cleanup script can be run in a recovery mode or
# live environment. Here's an example cleanup script:
#
# #!/bin/bash
# STUBBORN_FILE="/tmp/stubborn_file.txt"
# Remove immutable attribute
#  ╭───────────────────────────────────────────────────────────────────╮
#  │ xx or su -                                                        │
#  │ chattr -i "$STUBBORN_FILE"                                        │
#  │ rm "$STUBBORN_FILE                                                │
#  ╰───────────────────────────────────────────────────────────────────╯
# sudo ./cleanup_stubborn_file.sh
# This should safely remove the stubborn file without needing to reboot the system.
# Use this cleanup script responsibly!
# End of script
