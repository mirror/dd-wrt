# Rough release process

## Update `CHANGES`

Go through `git log` and ensure that each relevant change is documented in
the `CHANGES file.

## Ensure version consistency

The version identifier needs to be consistent amongst the `CHANGES` file, the
`configure.ac` file, and the git tag. First, determine the currently-configured
version identifier, such as by running:

```
grep AC_INIT configure.ac | cut -d[ -f 2 | cut -d] -f 1
grep Release CHANGES | head -1
```

When preparing a release candidate build, the version string should end with
`_rcN`, where N is the candidate build number.

Conventionally, the `CHANGES` file will contain a string in the format
`v<version>`, such as `v2.20_rc1` or `v2.20`, while the git tag and
`configure.ac` file will contain a string in the format `<version>`, such as
`2.20_rc1` or `2.20.`

Edit the `CHANGES` file and note the new version identifier.

## Update `configure.ac`

After manually updating the `CHANGES` file, update the `configure.ac` file
with a matching version identifier, such as:

```
export VERSION="$(grep Release CHANGES | head -1 | sed s/'.*Release v'//g)"
echo "New version identifier is: $VERSION"
sed -i -e "/^AC_INIT/s,\[.*\],[$VERSION],g" configure.ac
```

## Validate, commit, and tag

Next, examine the changes to ensure accuracy:

```
git diff CHANGES configure.ac
```

If everything looks good, commit the changes and create the tag. Note that
this will create a signed tag, so ensure that you have GPG configured
appropriately.

```
git commit -s -m "Release ${VERSION}" CHANGES configure.ac
git tag -s v${VERSION} -m "$VERSION"
```

## Build release archives

### Clean up Docker environment

To build the release archives, first delete the container manually to ensure
the build works with a clean container (this command may fail if hte container
does not exist):

```
docker rmi radvd-autogen:latest
```

### Perform a package build

The `autogen-container.sh` script will run `autoreconf` in a clean environment.
Afterward, the `./configure` script can be run in order to configure the build
environment. Finally, `make packages` will create package archives suitable
for release.

```
./autogen-container.sh
./configure
make packages
```

### Release the new version on GitHub

To perform this step, first install and configure the
[GitHub CLI](https://cli.github.com/).

```
gh release create v${VERSION} radvd-${VERSION}.tar.{xz,gz}{,.asc,.sha256,.sha512}`
```
