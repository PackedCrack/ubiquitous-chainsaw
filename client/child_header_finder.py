# This script checks whether or not any of the child headers of Windows.h has been included on its own.
# These child headers shouldn't be included as it can break dependencies - instead Windows.h should be included.
# See: https://en.wikipedia.org/wiki/Windows.h

import os
import sys
import re


EXIT_SUCCESS = 0
EXIT_FAILURE = 1

CHILD_HEADERS = {"stdarg.h", "windef.h", "winnt.h", "basetsd.h", "guiddef.h", "ctype.h", "string.h",
                 "winbase.h", "winerror.h", "wingdi.h", "winuser.h", "winnls.h", "wincon.h",
                 "winver.h", "winreg.h", "winnetwk.h", "winsvc.h", "imm.h", "cderr.h", "commdlg.h",
                 "dde.h", "ddeml.h", "dlgs.h", "lzexpand.h", "mmsystem.h", "nb30.h", "rpc.h",
                 "shellapi.h", "wincrypt.h", "winperf.h", "winresrc.h", "winsock.h", "winspool.h",
                 "winbgim.h", "ole2.h", "objbase.h", "oleauto.h", "olectlid.h"}

def extract_header_name(line: str) -> str:
    match = re.search(r'[<"]', line)
    if match:
        filepathIndex = match.start() + 1
        line = line[filepathIndex:]
        
        count = 0
        for i in range(len(line) - 1, -1, -1):
            if line[i] == '\n':
                count = count + 1
            elif line[i] == '>':
                count = count + 1
            elif line[i] == '"':
                count = count + 1
            else:
                break
                
        line = line[:-count]
        
        index = line.rfind('/')
        if index != -1:
            line = line[index + 1:]
        
        
        return line
    else:
        print("Failed to extract header name from string: \"{}\"".format(line))
        return line

def find_child_header_includes(directory) -> int:
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith((".hpp", ".cpp", ".c", ".h")):
                filePath = os.path.join(root, file)
                try:
                    with open(filePath, 'r') as file:
                        for line in file:
                            line = line.lower()
                            if line.startswith('#include'):
                                headerName = extract_header_name(line)
                                if headerName in CHILD_HEADERS:
                                    print("\033[31;1m" + "Found \"{}\" in file \"{}\"".format(headerName, filePath) + "\033[0m")
                                    return EXIT_FAILURE
                            elif line.startswith(("\n", "//", "#")): # Files might have comments, defines or new lines before any includes
                                continue	
                            else:
                                break
                except Exception as e:
                    print("An error occurred while trying to read the file {}: {}".format(filePath, e))
                    return EXIT_FAILURE
                
    return EXIT_SUCCESS

if __name__ == "__main__":
    print("### SCANNING SOURCE FILES FOR WINDOWS.H CHILD HEADER INCLUDES... ###")
    sys.exit(find_child_header_includes("src"))
