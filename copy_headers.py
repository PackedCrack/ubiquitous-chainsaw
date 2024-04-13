import os
import sys
import shutil


EXIT_FAILURE = 1
EXIT_SUCCESS = 0

def delete_header_files(directory):
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(('.h', '.hpp')):
                os.remove(os.path.join(root, file))

def copy_headers(srcDir, destDir) -> int:
	if not os.path.exists(destDir):
		return EXIT_FAILURE

	for root, dirs, files in os.walk(srcDir):
		for file in files:
			if file.endswith(('.h', '.hpp')):
				if "detail" in file:
					continue

				srcFilepath = os.path.join(root, file)
				dstFilepath = os.path.join(destDir, file)
				shutil.copy2(srcFilepath, dstFilepath)

	return EXIT_SUCCESS

def main() -> int:
	DO_DELETE = False

	argc = len(sys.argv)
	if argc < 3:
		print("Usage: python3 copy_headers.py <source directory> <destination directory> <Optional: delete>")
		return EXIT_FAILURE


	srcDirectory = sys.argv[1]
	dstDirectory = sys.argv[2]
	if argc == 4:
		if sys.argv[3].lower() == "delete":
			DO_DELETE = True
		else:
			print("unknown third parameter: {}".format(sys.argv[3]))
			return EXIT_FAILURE

	if DO_DELETE:
		delete_header_files(dstDirectory)

	return copy_headers(srcDirectory, dstDirectory)


sys.exit(main())