import sys as system


def remove_trailing_newlines(line: str) -> str:
	count = 0

	for i in range(len(line) - 1, -1, -1):
		if(line[i] == '\n' or line[i] == '\r'):
			count = count + 1
		else:
			break

	return line[:-count]


def remove_trailing_byte_as_str(line: str) -> str:
	count = 16
	for i in range(len(line) - count - 1, -1, -1):
		if(line[i] != ' '):
			break

		count = count + 1

	return line[:-count]

def remove_mem_address(line: str) -> str:
	count = 8
	for i in range(count, len(line)):
		if line[count] == ' ':
			count = count + 1
		else:
			break

	return line[count - 1:]	# keep one space for replace

def starts_with_number(line: str):
	if ord(line[0]) < 48 or ord(line[0]) > 57:
		return False

	return True

# Convert and print the C++ array initialization
#cpp_array = hex_dump_to_cpp_array(hex_dump)
#print(cpp_array)
EXIT_FAILURE = 1
EXIT_SUCCESS = 1
def main() -> int:
	#if len(system.argv) != 3:
	#	return system.exit(EXIT_FAILURE)
	
	#inputFilepath = "C:\\Users\\qwerty\\source\\repos\\ubiquitous-chainsaw\\client\\src\\gui\\font.txt" #system.argv[1]
	#ouputFilepath = "C:\\Users\\qwerty\\source\\repos\\ubiquitous-chainsaw\\client\\src\\gui\\out.txt" #system.argv[2]
	inputFilepath = system.argv[1]
	ouputFilepath = system.argv[2]

	numElements = 0
	arrayElements = str()
	with open(inputFilepath, 'r', encoding = 'utf-8') as ifstream:
		for line in ifstream:
			if not starts_with_number(line):	# valid line must start with a memory address
				continue

			line = remove_trailing_newlines(line)
			line = remove_trailing_byte_as_str(line)
			line = remove_mem_address(line)
			line = line.replace(' ', ", 0x")

			numElements = numElements + line.count(', ')
			if(numElements <= 16):
				# remove ', ' from first array element
				line = line[2:]

			arrayElements += line
			arrayElements += "\n"


	array = "static constexpr std::array<uint8_t, {}> data{{\n{} }};".format(numElements, arrayElements)

	with open(ouputFilepath, 'w', encoding = 'utf-8') as ofstream:
		ofstream.write(array)

	return system.exit(EXIT_SUCCESS)


system.exit(main())