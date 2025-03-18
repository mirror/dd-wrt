import os, re, fnmatch, glob
import Utils

IGNORE_DIRS = ['@eaDir', '.*_UNPACK_.*', '.*_FAILED_.*', '\..*', 'lost\+found', '.AppleDouble', '.*\.itlp$', '@Recycle', '.*\.photoslibrary', '#recycle', '@Recently-Snapshot']
ROOT_IGNORE_DIRS = ['\$Recycle.Bin', 'System Volume Information', 'Temporary Items', 'Network Trash Folder']

# Parse a .plexignore file, append patterns to the plexignore lists.
def ParsePlexIgnore(filename, plexignore_files, plexignore_dirs, cwd=None):
  try:
    f = open(filename,'r')
    for pattern in f:
      pattern = pattern.strip()
      if pattern != '' and pattern[0] != '#':

        # Check for backslash escaped #'s and trim if necessary.
        if pattern[:2] == '\\#': 
          pattern = pattern[1:]

        if '/' not in pattern:
          # Match filenames using regex.
          plexignore_files.append(fnmatch.translate(pattern))
        else:
          # Match directories using glob.  Leading slashes screw things up;
          # these should always be relative to the .plexignore file.
          if pattern.strip()[0] != '/':
            dir_root = os.path.dirname(cwd) if cwd else os.path.dirname(filename)
            plexignore_dirs.append(os.path.join(dir_root, pattern))
    f.close()
  except:
    return


# Remove files and directories that don't make sense to scan.
def Scan(path, files, mediaList, subdirs, exts, root=None):

  files_to_whack = []
  plexignore_files = []
  plexignore_dirs = []
  use_unicode = os.path.supports_unicode_filenames

  # Build a list of things to ignore based on a .plexignore file in this dir.
  if root and Utils.ContainsFile(files, '.plexignore'):
    ParsePlexIgnore(os.path.join(root,path,'.plexignore'), plexignore_files, plexignore_dirs)

  # Also look for a .plexignore in the 'root' for this source.
  if root and (not files or (files and root != os.path.dirname(files[0]))):
    if Utils.ContainsFile(os.listdir(root), '.plexignore'):
      ParsePlexIgnore(os.path.join(root,'.plexignore'), plexignore_files, plexignore_dirs, os.path.join(root, path))

  for f in files:
    # Only use unicode if it's supported, which it is on Windows and OS X,
    # but not Linux. This allows things to work with non-ASCII characters
    # without having to go through a bunch of work to ensure the Linux 
    # filesystem is UTF-8 "clean".
    #
    if use_unicode:
      try: filename = unicode(f.decode('utf-8'))
      except: files_to_whack.append(f)
    else:
      filename = f
      
    (basename, ext) = os.path.splitext(f)
    basename = os.path.basename(basename)
    
    # If extension is wrong, don't include.
    if not ext.lower()[1:] in exts:
      files_to_whack.append(f)
    
    # Broken symlinks and zero byte files need not apply.
    if os.path.exists(filename) == False or os.path.getsize(filename) == 0:
      files_to_whack.append(f)

    # Remove unreadable files.
    if not os.access(filename, os.R_OK):
      # If access() claims the file is unreadable, try to read a byte just to be sure.
      try:
        read_file = open(filename,'rb')
        read_file.read(1)
        read_file.close()
      except:
        files_to_whack.append(f)
      
    # Remove hidden files.
    if len(basename) == 0 or basename[0] == '.':
      files_to_whack.append(f)

    # Remove .plexignore file regex matches.
    for rx in plexignore_files:
      if re.match(rx, os.path.basename(f), re.IGNORECASE):
        Utils.Log('Removing file due to plexignore rule (%s): %s' % (rx, f))
        files_to_whack.append(f)

  # See what directories to ignore.
  ignore_dirs_total = IGNORE_DIRS
  if len(path) == 0:
    ignore_dirs_total += ROOT_IGNORE_DIRS

  dirs_to_whack = []
  for dir in subdirs:
    # See which directories to get rid of.
    baseDir = os.path.basename(dir)
    for rx in ignore_dirs_total:
      if re.match(rx, baseDir, re.IGNORECASE):
        dirs_to_whack.append(dir)
        break

  # Add glob matches from .plexignore before whacking.
  for pattern in plexignore_dirs:
    ignore_list = glob.glob(pattern)
    ignore_list.extend(glob.glob(os.path.dirname(pattern)))
    for match in ignore_list:
      if os.path.isdir(match):
        dirs_to_whack.append(match)
        Utils.Log('Removing directory due to plexignore rule (%s): %s' % (pattern, match))
      else:
        files_to_whack.append(match)
        Utils.Log('Removing file due to plexignore rule (%s): %s' % (pattern, match))

  # Whack files.
  files_to_whack = list(set(files_to_whack))
  for f in files_to_whack:
    if f in files:
      files.remove(f)

  # Remove the directories.
  dirs_to_whack = list(set(dirs_to_whack))
  for f in dirs_to_whack:
    if f in subdirs:
      subdirs.remove(f)
