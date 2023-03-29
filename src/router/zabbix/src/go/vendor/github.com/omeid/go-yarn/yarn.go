// Package yarn is a filesystem mapped key-string store. Ideal for embedding code like sql.
package yarn

import (
	"io/ioutil"
	"net/http"
	"os"
	"path/filepath"
)

// Must is like New, but panics on error.
func Must(fs http.FileSystem, patterns ...string) Yarn {
	y, e := New(fs, patterns...)
	if e != nil {
		panic(e)
	}
	return y
}

// Yarn is script store.
type Yarn interface {
	// Has checks if a file for the provided list of keys exists, if not, returns an error.
	Has(strings ...string) error
	// MustHave is like Has but panics on missing keys.
	MustHave(strings ...string)
	// Get returns a loaded file's contents as string and if it exists by filename.
	Get(key string) (string, bool)
	// Must returns a loaded file's contents as string, it panics if file doesn't exist.
	Must(key string) string

	// A sub-directory view of the Yarn
	Sub(dir string) Yarn

	// List all the files.
	List() []string

	// All get every file in the as path -> content
	All() map[string]string

	// Walk calls the provided function with each file, it will stop as soon
	// as an error is returned.
	Walk(pattern string, visitor func(path string, content string) error) error
}

// New creates a new Yarn from provided filesystem's files that match the pattern,
func New(fs http.FileSystem, patterns ...string) (Yarn, error) {

	//Check the pattern.
	for _, pattern := range patterns {
		_, err := filepath.Match(pattern, "")
		if err != nil {
			return nil, err
		}
	}
	dir, err := fs.Open("/")
	if err != nil {
		return nil, err
	}

	files, err := dir.Readdir(-1)
	if err != nil {
		return nil, err
	}

	y := newYarn()

	err = addFiles("", y, fs, files, patterns)
	if err != nil {
		return nil, err
	}

	return y, nil
}

func addFiles(prefix string, y *yarn, fs http.FileSystem, files []os.FileInfo, patterns []string) error {

	for _, file := range files {

		if file.IsDir() {
			path := file.Name()

			if prefix != "" {
				path = filepath.Join(prefix, path)
			}

			dir, err := fs.Open(path)
			if err != nil {
				return nil
			}

			files, err := dir.Readdir(-1)
			if err != nil {
				return err
			}

			err = addFiles(path, y, fs, files, patterns)
			if err != nil {
				return err
			}

			continue
		}

		path := file.Name()
		if prefix != "" {
			path = filepath.Join(prefix, path)
		}

		var (
			err error
			ok  bool
		)

		for _, pattern := range patterns {
			ok, err = filepath.Match(pattern, path)
			if err != nil {
				return err
			}
			if ok {
				break
			}
		}
		if !ok {
			continue
		}
		file, err := fs.Open(path)
		if err != nil {
			return err
		}

		content, err := ioutil.ReadAll(file)
		if err != nil {
			return err
		}

		y.add(path, string(content))

	}
	return nil
}
