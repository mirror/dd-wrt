import sys
import string
import random

__all__ = ['convert']

def xpy_comment(line):
  return '##XPY## %s ##' % line

def convert(source):
  # Convert newlines and split
  lines = source.replace('\r\n', '\n').split('\n')

  # Iterate over lines
  i = 0
  while i < len(lines):
    unmod_line = lines[i]
    line = unmod_line.strip()

    if len(line) > 0:
      
      """
      Extended decorators
      -------------------

      Format: @name a, b <x, y>:

      Output: @name(a, b):
              def X__name_ABCDE123(x=x, y=y):

      Decorator args (a, b) and method args (<x, y>) are both optional.

      """
      if line[0] == "@" and line[-1] == ":":
        # Get the prefix (number of spaces before the @), then remove the @ and : chars
        prefix = unmod_line[:unmod_line.find('@')]
        line = line[1:-1]

        # Split out any method arguments first
        parts = line.split('<')
        method_args = [x.strip() for x in parts[1][:-1].split(',')] if len(parts) == 2 else []
        
        # Get the first part of the string (decorator name + args)
        first_part = parts[0].strip()

        # If there's a space, see if we have any args after it
        pos = first_part.find(' ')
        if pos > -1:
          name = first_part[:pos]
          decorator_args = [x.strip() for x in first_part[pos:].split(',')]
        else:
          name = first_part.strip()
          decorator_args = []

        # Convert the args into regular Python code
        new_line = prefix + '@' + name
        if len(decorator_args) > 0:
          new_line += '(' + ','.join(decorator_args) + ')'
        new_line += '\n'
        new_line += prefix + 'def X__' + name + '_' + ''.join(random.choice(string.ascii_uppercase + string.digits) for x in range(8))
        new_line += '(' + ','.join(['%(x)s=%(x)s' % dict(x=x) for x in method_args]) + ')'
        new_line += ': ' + xpy_comment(unmod_line.strip())
        lines[i] = new_line

    i += 1

  return '\n'.join(lines)

if __name__ == "__main__" and len(sys.argv) > 1:
  f = open(sys.argv[1], 'r')
  source = f.read()
  f.close()

  new_source = convert(source)

  if len(sys.argv) > 2:
    f = open(sys.argv[2], 'w')
    f.write(new_source)
    f.close()
  else:
    print new_source
