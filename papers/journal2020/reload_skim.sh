#!/bin/bash

/usr/bin/osascript << EOF
  set theFile to POSIX file "dsd20_main.pdf" as alias
  tell application "Skim"
  activate
  set theDocs to get documents whose path is (get POSIX path of theFile)
  if (count of theDocs) > 0 then revert theDocs
  open theFile
  end tell
EOF
