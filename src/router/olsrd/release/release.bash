#!/bin/bash

set -e
set -u


# ##############################################################################
# # Settings
# ##############################################################################

declare MODE_BRANCH="branch"
declare MODE_RELEASE="release"

declare MODE_BRANCH_TXT="Branch"
declare MODE_BRANCH_TXT_LOWER="branch"
declare MODE_RELEASE_TXT="Release"
declare MODE_RELEASE_TXT_LOWER="release"

declare MODE_TXT=""
declare MODE_TXT_LOWER=""


# The digit representation of a basic version can be in the format 0.6.4
declare versionRegexDigitsBasic="([[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+)"

# The digit representation of a release branch version can be in the format
# 0.6.4 or 0.6.4.1
declare versionRegexDigits="${versionRegexDigitsBasic}(\.[[:digit:]]+)?"

# The version for source code can be in the format:
# - 0.6.4 or 0.6.4.1 or pre-0.6.4 or pre-0.6.4.1
declare versionRegexSources="(|pre-)(${versionRegexDigits})"

# The version for a release tag is in the format v0.6.4 or v0.6.4.1
declare versionRegexReleaseTag="v(${versionRegexDigits})"

# A release branch is in the format release-0.6.4 or release-0.6.4.1
declare relBranchRegex="release-(${versionRegexDigits})"




# ##############################################################################
# # Functions
# ##############################################################################

#
# Print script usage
#
function usage() {
  echo ""
  echo "  $(basename "${script}") ${MODE_BRANCH} 0.6.4"
  echo "    - create the release branch for version 0.6.4"
  echo "  $(basename "${script}") ${MODE_RELEASE}"
  echo "    - release the (checked-out) release branch"
}


#
# Trim a string: remove spaces from the beginning and end of the string
#
# 1=string to trim
# return=trimmed string
function stringTrim() {
  if [[ -z "${1}" ]]; then
    return
  fi

  # remove leading whitespace characters
  local var="${1#${1%%[![:space:]]*}}"

  # remove trailing whitespace characters
  echo "${var%${var##*[![:space:]]}}"
}


#
# Get the canonical path of a file or directory
# This is the physical path without any links
#
# 1=the file or directory
function pathCanonicalPath() {
  local src="$(stringTrim "${1}")"

  if [[ -h "${src}" ]] && [[ -d "${src}" ]]; then
    # src is a link to a directory
    pushd . &> /dev/null
    cd -P "${src}" &> /dev/null
    pwd -P
    popd &> /dev/null
    return
  fi

  # we're not dealing with a directory here
  while [[ -h "${src}" ]]; do
    # keep getting the link target while src is a link
    src="$(ls -la "${src}" | \
           sed -r 's#^.*?[[:space:]]+->[[:space:]]+(.*)$#\1#')"
  done
  # src is no longer a link here

  pushd . &> /dev/null
  cd -P "$(dirname "${src}")" &> /dev/null
  echo "$(pwd -P)/$(basename "${src}")"
  popd &> /dev/null
}


#
# Determine whether a given directory is a git repository directory
#
# 1=directory
# return=0 not a git dir, 1 a git dir
function gitIsGitDirectory() {
  local place="$(stringTrim "${1}")"

  local -i result=1
  if [[ -d "${place}" ]]; then
    pushd "${place}" &> /dev/null
    set +e
    git rev-parse --git-dir &> /dev/null
    result=${?}
    set -e
    popd &> /dev/null
  fi

  if [[ ${result} -ne 0 ]]; then
    echo "0"
  else
    echo "1"
  fi
}


#
# Go into the root of the checkout and check some key files
#
function checkIsOlsrdGitCheckout() {
  if [[ "$(gitIsGitDirectory ".")" == "0" ]] || \
     [[ ! -r ./Makefile.inc ]] || \
     [[ ! -r ./files/olsrd.conf.default.full ]]; then
    echo "* You do not appear to be running the script from an olsrd git checkout"
    exit 1
  fi
}


#
# Check that a signing key is configured
#
function checkGitSigningKeyIsConfigured() {
  local gpgKeyId="$(git config --get user.signingkey)"
  if [[ -z "${gpgKeyId}" ]]; then
    cat >&1 << EOF
* No signing key is setup for git, please run
    git config --global user.signingkey <key ID>

  You can get keys and IDs by running 'gpg --list-keys'
EOF
    exit 1
  fi

  #
  # Check that the signing key is present
  #
	set +e
	gpg --list-key "${gpgKeyId}" &> /dev/null
	local -i gpgKeyIdPresentResult=${?}
	set -e
	if [[ ${gpgKeyIdPresentResult} -ne 0 ]]; then
	  cat >&1 << EOF
* Your signing key with ID ${gpgKeyId} is not found, please run
    git config --global user.signingkey <key ID>
  to setup a valid key ID.

  You can get keys and IDs by running 'gpg --list-keys'
EOF
	  exit 1
	fi
}


#
# Get the version digits from a release tag version
#
# 1=release tag version
# return=version digits
function getVersionDigitsFromReleaseTag() {
  echo "$(stringTrim "${1}")" | sed -r "s/${versionRegexReleaseTag}/\1/"
}


#
# Get the previous release tag and check
#
declare prevRelTagVersion=""
function getPrevRelTag() {
  set +e
  prevRelTagVersion="$(git describe --abbrev=0 | \
                       grep -E "^${versionRegexReleaseTag}$")"
  set -e
  if [[ -z "${prevRelTagVersion}" ]]; then
    echo "* Could not find the previous release tag"
    exit 1
  fi
}


#
# Get the next version digits by incrementing the micro digit
#
# 1=version in format 0.6.4 or 0.6.4.1
# return=incremented version in format 0.6.5
function getNextVersionDigitsMicro() {
  local version="$(stringTrim "${1}")"
  local -a versionDigits=( ${version//\./ } )
  local -i versionMicroNext=$(( ${versionDigits[2]} + 1 ))
  echo "${versionDigits[0]}.${versionDigits[1]}.${versionMicroNext}"
}


#
# Get the next version digits by incrementing the patchlevel digit
#
# 1=version in format 0.6.4 or 0.6.4.0
# return=incremented version in format 0.6.4.1
function getNextVersionDigitsPatchLevel() {
  local version="$(stringTrim "${1}")"
  local -a versionDigits=( ${version//\./ } )
  local -i versionPatchLevelNext=1
  if [[ ${#versionDigits[*]} -ne 3 ]]; then
    versionPatchLevelNext=$(( ${versionDigits[3]} + 1 ))
  fi
  echo "${versionDigits[0]}.${versionDigits[1]}.${versionDigits[2]}.${versionPatchLevelNext}"
}


#
# Adjust the branch name so that we can release 0.6.4.x from the
# release-0.6.4 branch
#
# prevTagVersionDigits	relBranchVersionDigits	relBranchVersionDigits (adjusted)
#	0.6.4			0.6.4			0.6.4.1
#	0.6.4			0.6.5			-
#	0.6.4			0.6.4.5			-
#	0.6.4			0.6.5.5			-
#	0.6.4.5			0.6.4			0.6.4.6
#	0.6.4.5			0.6.5			-
#	0.6.4.5			0.6.4.6			-
#	0.6.4.5			0.6.5.6			-
function adjustBranchName() {
  local -a prevTagVersionDigitsArray=( ${prevTagVersionDigits//\./ } )
  local -a relBranchVersionDigitsArray=( ${relBranchVersionDigits//\./ } )
  local -i prevTagVersionDigitsCount=${#prevTagVersionDigitsArray[*]}
  local -i relBranchVersionDigitsCount=${#relBranchVersionDigitsArray[*]}
  local prevTagVersionTrain="$(echo "$(stringTrim "${prevTagVersionDigits}")" | \
                               sed -r "s/${versionRegexDigits}/\1/")"

  if  [[ "${prevTagVersionDigits}" == "${relBranchVersionDigits}" ]] || \
     ([[ "${prevTagVersionTrain}"  == "${relBranchVersionDigits}" ]] && \
      [[ ${prevTagVersionDigitsCount}   -eq 4 ]] && \
      [[ ${relBranchVersionDigitsCount} -eq 3 ]]); then
    relBranchVersionDigits="$(getNextVersionDigitsPatchLevel "${prevTagVersionDigits}")"
  fi
}


#
# Check that the new version is incrementing
#
# 1=last version
# 2=new version
# 3= 0 to disallow equal versions as ok
# return (in checkVersionIncrementingResult) = 0 when new version > last version,
#                                              1 otherwise
declare -i checkVersionIncrementingResult=0
function checkVersionIncrementing() {
  checkVersionIncrementingResult=0
  local lastVersion="$(stringTrim "${1}")"
  local newVersion="$(stringTrim "${2}")"
  local -i allowEqualVersions=${3}

  local -a lastVersionDigits=( ${lastVersion//\./ } )
  local -a newVersionDigits=( ${newVersion//\./ } )

  # if the last version is in the format 0.6.4 then assume 0.6.4.0
  if [[ ${#lastVersionDigits[*]} -ne 4 ]]; then
    lastVersionDigits[3]=0
  fi

  # if the new version is in the format 0.6.4 then assume 0.6.4.0
  if [[ ${#newVersionDigits[*]} -ne 4 ]]; then
    newVersionDigits[3]=0
  fi

  # major
  if [[ ${newVersionDigits[0]} -lt ${lastVersionDigits[0]} ]]; then
    checkVersionIncrementingResult=1
    return
  fi
  if [[ ${newVersionDigits[0]} -gt ${lastVersionDigits[0]} ]]; then
    return
  fi

  # minor
  if [[ ${newVersionDigits[1]} -lt ${lastVersionDigits[1]} ]]; then
    checkVersionIncrementingResult=1
    return
  fi
  if [[ ${newVersionDigits[1]} -gt ${lastVersionDigits[1]} ]]; then
    return
  fi

  # micro
  if [[ ${newVersionDigits[2]} -lt ${lastVersionDigits[2]} ]]; then
    checkVersionIncrementingResult=1
    return
  fi
  if [[ ${newVersionDigits[2]} -gt ${lastVersionDigits[2]} ]]; then
    return
  fi

  # patch level
  if [[ ${newVersionDigits[3]} -lt ${lastVersionDigits[3]} ]]; then
    checkVersionIncrementingResult=1
    return
  fi
  if [[ ${newVersionDigits[3]} -gt ${lastVersionDigits[3]} ]]; then
    return
  fi

  # everything is equal
  if [[ ${allowEqualVersions} -eq 0 ]]; then
    checkVersionIncrementingResult=1
  fi
  return
}


#
# Commit the current changes, allow an empty commit, or amend (when the commit
# message is the same as that of the last commit)
#
# 1=non-zero to allow an empty commit
# 2=commit message
function commitChanges() {
  local -i allowEmpty=${1}
  local msg="$(stringTrim "${2}")"

  local lastMsg="$(git log -1 --format="%s")"
  lastMsg="$(stringTrim "${lastMsg}")"
  local extra=""
  if [[ ${allowEmpty} -ne 0 ]]; then
    extra="${extra} --allow-empty"
  fi
  if [[ "${msg}" == "${lastMsg}" ]]; then
    extra="${extra} --amend"
  fi
  set +e
  git commit -s -q ${extra} -m "${msg}" &> /dev/null
  set -e
}


#
# Get the version from the Makefile
#
function getVersionFromMakefile() {
  local src="Makefile"
  local regex="([[:space:]]*VERS[[:space:]]*=[[:space:]]*)${versionRegexSources}[[:space:]]*"
  grep -E "^${regex}\$" "${src}" | sed -r "s/^${regex}\$/\3/"
}


#
# Update the version in all relevant files
#
# 1=the new version (in the format of versionRegexSources)
function updateVersions() {
  local newVersion="$(stringTrim "${1}")"

  #
  # Adjust debug settings in Makefile.inc
  #
  local src="Makefile.inc"
  sed -ri \
   -e 's/^[[:space:]]*DEBUG[[:space:]]*?=.*$/DEBUG ?= 1/' \
   -e 's/^[[:space:]]*NO_DEBUG_MESSAGES[[:space:]]*?=.*$/NO_DEBUG_MESSAGES ?= 0/' \
   "${src}"
  set +e
  git add "${src}"
  set -e


  #
  # Adjust version in Makefile
  #
  local src="Makefile"
  sed -ri "s/^([[:space:]]*VERS[[:space:]]*=[[:space:]]*)${versionRegexSources}[[:space:]]*\$/\1${newVersion}/" "${src}"
  set +e
  git add "${src}"
  set -e


  #
  # Adjust version in win32 gui installer
  #
  local src="gui/win32/Inst/installer.nsi"
  local grepStr="^([[:space:]]*MessageBox[[:space:]]+MB_YESNO[[:space:]]+\".+?[[:space:]]+olsr\.org[[:space:]]+)${versionRegexSources}([[:space:]]+.+?\"[[:space:]]+IDYES[[:space:]]+NoAbort)[[:space:]]*$"
  local replStr="\1${newVersion}\6"
  sed -ri "s/${grepStr}/${replStr}/" "${src}"
  set +e
  git add "${src}"
  set -e


  #
  # Adjust version in win32 gui front-end
  #
  local src="gui/win32/Main/Frontend.rc"
  local grepStr="^([[:space:]]*CAPTION[[:space:]]+\"olsr\.org[[:space:]]+Switch[[:space:]]+)${versionRegexSources}([[:space:]]*\")[[:space:]]*\$"
  local replStr="\1${newVersion}\6"
  sed -ri "s/${grepStr}/${replStr}/" "${src}"
  set +e
  git add "${src}"
  set -e
}


#
# Sign a text file
#
# 1=the text file
function signTextFile() {
  local txtFile="$(stringTrim "${1}")"
  gpg -u "$(git config --get user.name)" --clearsign "${txtFile}"
  mv "${txtFile}.asc" "${txtFile}"
}




# ##############################################################################
# # Main
# ##############################################################################

declare script="$(pathCanonicalPath "${0}")"
declare scriptDir="$(dirname "${script}")"
declare baseDir="$(dirname "${scriptDir}")"

cd "${baseDir}"

declare -i masterWasUpdated=0

#
# Check the number of arguments
#
if [[ ${#} -lt 1 ]]; then
  echo "* Need at least 1 argument:"
  usage
  exit 1
fi


#
# Get the mode and check it
#
declare mode="$(stringTrim "${1}")"
shift 1
if [[ ! "${mode}" == "${MODE_BRANCH}" ]] && \
   [[ ! "${mode}" == "${MODE_RELEASE}" ]]; then
  echo "* Wrong mode: ${mode}"
  usage
  exit 1
fi


#
# Further mode/argument parsing
#
declare branchVersion=""
if [[ "${mode}" == "${MODE_BRANCH}" ]]; then
  MODE_TXT="${MODE_BRANCH_TXT}"
  MODE_TXT_LOWER="${MODE_BRANCH_TXT_LOWER}"

  #
  # Get the branch version to create
  #
  if [[ ${#} -ne 1 ]]; then
    echo "* Need the version to branch:"
    usage
    exit 1
  fi
  branchVersion="$(stringTrim "${1}")"
  shift 1

  #
  # Check branch version
  #
  if [[ -z "$(echo "${branchVersion}" | grep -E "^${versionRegexDigitsBasic}\$")" ]]; then
    echo "* Version to branch ${branchVersion} has invalid format"
    echo "  Expected format is: 0.6.4"
    exit 1
  fi
else
  MODE_TXT="${MODE_RELEASE_TXT}"
  MODE_TXT_LOWER="${MODE_RELEASE_TXT_LOWER}"

  if [[ ${#} -ne 0 ]]; then
    echo "* Need no additional arguments."
    usage
    exit 1
  fi
fi


checkIsOlsrdGitCheckout

if [[ "${mode}" == "${MODE_RELEASE}" ]]; then
  checkGitSigningKeyIsConfigured
fi

getPrevRelTag
declare prevTagVersionDigits="$(getVersionDigitsFromReleaseTag "${prevRelTagVersion}")"


#
# Get the current branch and check that we're on a release branch (for the
# release mode) or on the master branch (for the branch mode)
#
declare relBranch="$(git rev-parse --abbrev-ref HEAD)"
declare relBranch="$(stringTrim "${relBranch}")"
if [[ "${mode}" == "${MODE_BRANCH}" ]]; then
  if [[ -z "$(echo "${relBranch}" | grep -E "^master\$")" ]]; then
    echo "* You are not on the master branch"
    exit 1
  fi
  relBranch="release-${branchVersion}"

  # check that the branch does not yet exist
  declare -i branchTestResult=0
  set +e
  git rev-parse --abbrev-ref "${relBranch}" &> /dev/null
  branchTestResult=${?}
  set -e
  if [[ ${branchTestResult} -eq 0 ]]; then
    echo "* Branch ${relBranch} already exists"
    exit 1
  fi
else
  if [[ -z "$(echo "${relBranch}" | grep -E "^${relBranchRegex}\$")" ]]; then
    echo "* You are not on a release branch (format: release-0.6.4 or release-0.6.4.1)"
    exit 1
  fi
fi


#
# Get the version to release from the current branch
#
declare relBranchVersionDigits=""
if [[ "${mode}" == "${MODE_BRANCH}" ]]; then
  relBranchVersionDigits="${branchVersion}"
else
  relBranchVersionDigits="$(echo "${relBranch}" | \
                            sed -r "s/${relBranchRegex}/\1/")"
  adjustBranchName
fi

declare relTagVersion="v${relBranchVersionDigits}"
declare relBranchVersionDigitsNextMicro="$(getNextVersionDigitsMicro "${relBranchVersionDigits}")"
declare relBranchVersionDigitsNextPatchLevel="$(getNextVersionDigitsPatchLevel "${relBranchVersionDigits}")"


#
# Check that the version is incrementing
#
checkVersionIncrementing "${prevTagVersionDigits}" "${relBranchVersionDigits}" 0
if [[ ${checkVersionIncrementingResult} -ne 0 ]]; then
  echo "* The new version ${relBranchVersionDigits} is not greater than the previous version ${prevTagVersionDigits}"
  exit 1
fi


#
# When branching, check that the version is incrementing (allow equal versions),
# w.r.t. the version in the Makefile
#
if [[ "${mode}" == "${MODE_BRANCH}" ]]; then
  declare currentMasterVersion="$(getVersionFromMakefile)"
  checkVersionIncrementing "${currentMasterVersion}" "${relBranchVersionDigits}" 1
  if [[ ${checkVersionIncrementingResult} -ne 0 ]]; then
    echo "* The new version ${relBranchVersionDigits} is not greater than the current version ${currentMasterVersion}"
    exit 1
  fi
fi


#
# Confirm the branch/release
#
cat >&1 << EOF


* All checks pass, ready to ${MODE_TXT_LOWER} ${relBranchVersionDigits}.

  * The previous version found is: ${prevTagVersionDigits}
    Note: If this is not the version you were expecting, then maybe that
          version wasn't merged into this branch.
  * Continuing will DESTROY any modifications you currently have in your tree!

EOF
read -p "Press [enter] to continue or CTRL-C to exit..."
echo ""
echo ""


#
# Clean up the checkout
#
echo "Cleaning the git checkout..."
git clean -fdq
git reset -q --hard


if [[ "${mode}" == "${MODE_BRANCH}" ]]; then
  #
  # Update the versions for branch
  #
  echo "Updating the version to pre-${relBranchVersionDigits}..."
  updateVersions "pre-${relBranchVersionDigits}"
  commitChanges 1 "${MODE_TXT} ${relTagVersion}"
  masterWasUpdated=1

  # create release branch
  echo "Creating the release branch ${relBranch}..."
  git branch "${relBranch}"


  #
  # Update the version to the next release
  #
  echo "Updating the version to pre-${relBranchVersionDigitsNextMicro}..."
  updateVersions "pre-${relBranchVersionDigitsNextMicro}"
  commitChanges 0 "Update version after ${MODE_TXT_LOWER} of ${relTagVersion}"
else
  #
  # Update the versions for release
  #
  echo "Updating the version to ${relBranchVersionDigits}..."
  updateVersions "${relBranchVersionDigits}"
  commitChanges 1 "${MODE_TXT} ${relTagVersion}"


  #
  # Generate the changelog
  #
  echo "Generating the changelog..."
  declare src="CHANGELOG"
  declare dst="mktemp -q -p . -t "${src}.XXXXXXXXXX""
  cat > "${dst}" << EOF
${relBranchVersionDigits} -------------------------------------------------------------------

EOF
  git rev-list --pretty=short "${prevRelTagVersion}..HEAD" | \
    git shortlog -w80 -- >> "${dst}"
  cat "${src}" >> "${dst}"
  mv "${dst}" "${src}"
  set +e
  git add "${src}"
  set -e
  commitChanges 1 "${MODE_TXT} ${relTagVersion}"


  #
  # Tag the release
  #
  echo "Tagging ${relTagVersion}..."
  set +e
  git tag -d "${relTagVersion}" &> /dev/null
  set -e
  git tag -s -m "OLSRd release ${relBranchVersionDigits}" "${relTagVersion}"


  #
  # Update the version to the next release
  #
  echo "Updating the version to pre-${relBranchVersionDigitsNextPatchLevel}..."
  updateVersions "pre-${relBranchVersionDigitsNextPatchLevel}"
  commitChanges 1 "Update version after ${MODE_TXT_LOWER} of ${relTagVersion}"


  #
  # Update the version (on the master branch) to the next release
  #
  echo "Updating the version to pre-${relBranchVersionDigitsNextMicro} on the master branch..."
  git checkout -q master
  git clean -fdq
  git reset -q --hard

  declare oldMasterVersion="$(getVersionFromMakefile)"
  declare newMasterVersion="${relBranchVersionDigitsNextMicro}"
  checkVersionIncrementing "${oldMasterVersion}" "${newMasterVersion}" 0
  if [[ ${checkVersionIncrementingResult} -ne 0 ]]; then
    echo "* Skipped updating the version on the master branch:"
    echo "  The new version ${newMasterVersion} is not greater than the previous version ${oldMasterVersion}"
  else
    updateVersions "pre-${relBranchVersionDigitsNextMicro}"
    commitChanges 0 "Update version after ${MODE_TXT_LOWER} of ${relTagVersion}"
    masterWasUpdated=1
  fi

  git checkout -q "${relBranch}"
  git clean -fdq
  git reset -q --hard


  #
  # Make the release tarballs
  #
  echo "Generating the release tarballs..."
  declare tarFile="${scriptDir}/olsrd-${relBranchVersionDigits}.tar"
  declare tarGzFile="${tarFile}.gz"
  declare tarBz2File="${tarFile}.bz2"
  git archive --format=tar --prefix="olsrd-${relBranchVersionDigits}/" --output="${tarFile}" "${relTagVersion}"
  gzip   -c "${tarFile}" > "${tarGzFile}"
  bzip2  -c "${tarFile}" > "${tarBz2File}"
  rm -f "${tarFile}"
  echo "Generating the release tarball checksums..."
  declare md5File="${scriptDir}/MD5SUM-${relBranchVersionDigits}"
  declare sha256File="${scriptDir}/SHA256SUM-${relBranchVersionDigits}"
  md5sum    "${tarGzFile}" "${tarBz2File}" > "${md5File}"
  sha256sum "${tarGzFile}" "${tarBz2File}" > "${sha256File}"
  echo "Signing the release tarball checksums..."
  signTextFile "${md5File}"
  signTextFile "${sha256File}"
fi


echo "Done."


echo ""
echo ""
echo "==================="
echo "=   Git Updates   ="
echo "==================="
if [[ ${masterWasUpdated} -ne 0 ]]; then
  echo "Branch : master"
fi
echo "Branch : ${relBranch}"
if [[ "${mode}" == "${MODE_RELEASE}" ]]; then
  echo "Tag    : ${relTagVersion}"
  echo ""
  echo ""


  echo "==================="
  echo "= Generated Files ="
  echo "==================="
  cat >&1 << EOF
${tarGzFile}
${tarBz2File}
${md5File}
${sha256File}"
EOF
fi


echo ""
echo ""
echo "==================="
echo "= Manual Actions  ="
echo "==================="
if [[ "${mode}" == "${MODE_RELEASE}" ]]; then
  echo "1. Check that everything is in order. For example, run:"
  if [[ ${masterWasUpdated} -ne 0 ]]; then
    echo "     gitk master ${relBranch} ${relTagVersion}"
  else
    echo "     gitk ${relBranch} ${relTagVersion}"
  fi
  echo "2. Push. For example, run:"
  if [[ ${masterWasUpdated} -ne 0 ]]; then
    echo "     git push origin master ${relBranch} ${relTagVersion}"
  else
    echo "     git push origin ${relBranch} ${relTagVersion}"
  fi
  echo "3. Upload the generated files to"
  echo "     http://www.olsr.org/releases/${relBranchVersionDigits}"
  echo "4. Add a release article on olsr.org."
  echo ""
else
  echo "1. Check that everything is in order. For example, run:"
  if [[ ${masterWasUpdated} -ne 0 ]]; then
    echo "     gitk master ${relBranch}"
  else
    echo "     gitk ${relBranch}"
  fi
  echo "2. Push. For example, run:"
  if [[ ${masterWasUpdated} -ne 0 ]]; then
    echo "     git push origin master ${relBranch}"
  else
    echo "     git push origin ${relBranch}"
  fi
  echo "3. Send a 'release branch created' email to olsr-dev@lists.olsr.org."
  echo ""
fi

