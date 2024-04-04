import subprocess
import os


class Color:
    PURPLE = '\033[95m'
    CYAN = '\033[96m'
    DARKCYAN = '\033[36m'
    BLUE = '\033[94m'
    YELLOW = '\033[93m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    END = '\033[0m'


def run_command(command):
    try:
        subprocess.run(command, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running command '{command}': {e}")

def file_exists(fileName):
    if os.path.exists(fileName):
        return True
    else:
        return False

def file_delete(fileName):
    if os.path.exists(fileName):
        print(Color.GREEN + "LOG INFO: " + Color.END + "Deleting file " + Color.BOLD + f"'{fileName}'" + Color.END)
        os.remove(fileName)
    else:
        print(Color.RED + "LOG ERROR: " + Color.END + "Failed deleting file " + Color.BOLD + f"'{fileName}'" + Color.END + " which doesn't exist!")
        
def main():

    PROJECT_TARGET = "esp32s3"
    DEPENDENCIES = ["\"wolfssl/wolfssl^5.7.0\""]
    SDK_DEFAULTS = "sdkconfig.defaults"

    print(Color.GREEN + "LOG INFO: " + Color.END + "Configuring Ubiquitous-Chainsaw Server..")
    if file_exists(SDK_DEFAULTS):
        print(Color.GREEN + "LOG INFO: " + Color.END + "Custom SDK settings has been found")
    else:
        print(Color.YELLOW + "LOG WARNING: " + Color.END + "Custom SDK settings could not be found. Default SDK settings will be configured")
    
    print(Color.GREEN + "LOG INFO: " + Color.END + "Setting target.." + Color.END)
    SET_TARGET_CMD = f"idf.py set-target {PROJECT_TARGET}" # Will also clean the build
    run_command(SET_TARGET_CMD)

    print(Color.GREEN + "LOG INFO: " + Color.END + "Adding dependencies.." + Color.END)

    for dep in DEPENDENCIES:
        ADD_DEP_CMD = f"idf.py add-dependency {dep}"
        run_command(ADD_DEP_CMD)

    print(Color.GREEN + "LOG INFO: " + Color.END + "Building project.." + Color.END)
    BUILD_PROJECT_CMD = "idf.py build"
    run_command(BUILD_PROJECT_CMD)
    print()

if __name__ == "__main__":
    main()