## ------------------------------------------------------------------------
## Find a file (or one of more files in a list of dirs)
## ------------------------------------------------------------------------

# serial 1

dnl mfx_FIND_FILE(FILES, DIRECTORIES, VAR-FOR-DIR)
AC_DEFUN(mfx_FIND_FILE,
[
$3=""
for mfx_ac_dir in $2; do
  for mfx_ac_file in $1; do
    if test -r "$mfx_ac_dir/$mfx_ac_file"; then
      $3="$mfx_ac_dir"
      break 2
    fi
  done
done
])
