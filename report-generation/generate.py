#!./virt/bin/python3
# -*- encoding: utf-8 -*-

import os
import shutil
import sys

blueprint_name = sys.argv[1]
output_directory = sys.argv[2]

# Copy the template
shutil.copytree("template", output_directory, )

# Rename the templates subfolder
shutil.move(os.path.join(output_directory, "templates", "example"),
            os.path.join(output_directory, "templates", blueprint_name))


# Read in the file
init_file = os.path.join(output_directory,"__init__.py")
with open(init_file, 'r') as file :
  filedata = file.read()

# Replace the target string
filedata = filedata.replace('###TEMPLATENAME###', blueprint_name)

# Write the file out again
with open(init_file, 'w') as file:
  file.write(filedata)

print("Generated a new blueprint template at ", output_directory)
