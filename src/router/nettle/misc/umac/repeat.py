import sys

if len(sys.argv) < 3:
	sys.stderr.write('Usage: repeat [string] [length]\n')
	sys.exit(1)

s = sys.argv[1]
length = int(sys.argv[2])
if length > 0:
	if length > 1024:
		while len(s) < 1024:
			s = s + s;
	while length > len(s):
		sys.stdout.write(s)
		length -= len(s)

	sys.stdout.write(s[:length])


