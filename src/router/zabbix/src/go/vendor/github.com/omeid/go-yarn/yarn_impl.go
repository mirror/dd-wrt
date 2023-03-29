package yarn

import (
	"fmt"
	"path/filepath"
	"strings"
)

// MissingYarn error format.
const MissingYarn = "Missing %s"

// Yarn is the implemention of yarn.Yarn
type yarn struct {
	prefix string
	yarns  map[string]string
}

// NewFromMap creats a new yarn from provided map of kv.
// it is intend for use by other libraries, for an example
// see yarn/catalog.
func NewFromMap(files map[string]string) Yarn {
	return &yarn{
		prefix: "",
		yarns:  files,
	}
}

// New returns a new Yarn
func newYarn() *yarn {
	return &yarn{
		prefix: "",
		yarns:  make(map[string]string),
	}
}

// Add adds a string to the yarn.
// Add is not thread-safe, so it must be only
// used before handing over impl.Yarn as yarn.Yarn
// which has no Add.
func (y *yarn) add(key, value string) {
	y.yarns[key] = value
}

// Has checks if a file for the provided list of keys exists, if not, returns an error.
func (y *yarn) Has(strings ...string) error {
	var (
		s       string
		ok      bool
		missing []string
	)

	for _, s = range strings {
		if y.prefix != "" {
			s = filepath.Join(y.prefix, s)
		}

		if _, ok = y.yarns[s]; !ok {
			missing = append(missing, s)
		}
	}

	if len(missing) > 0 {
		return fmt.Errorf(MissingYarn, missing)
	}
	return nil
}

// MustHave is like Has but panics on missing keys.
func (y *yarn) MustHave(strings ...string) {
	err := y.Has(strings...)
	if err != nil {
		panic(err.Error())
	}
}

// Get returns a loaded file's contents as string and if it exists by filename.
func (y *yarn) Get(key string) (string, bool) {
	if y.prefix != "" {
		key = filepath.Join(y.prefix, key)
	}

	content, ok := y.yarns[key]
	return content, ok
}

// Must returns a loaded file's contents as string, it panics if file doesn't exist.
func (y *yarn) Must(key string) string {
	content, ok := y.yarns[key]
	if !ok {
		panic(fmt.Sprintf(MissingYarn, key))
	}
	return content
}

// List returns all the files.
func (y *yarn) List() []string {

	files := []string{}

	for path := range y.yarns {
		// if no prefix or matching path prefix.
		if y.prefix == "" || strings.HasPrefix(path, y.prefix) {
			files = append(files, path)
		}
	}

	return files
}

// All returns every file and content in the yarn.
func (y *yarn) All() map[string]string {

	files := make(map[string]string, len(y.yarns))

	for path, content := range y.yarns {
		// if no prefix or matching path prefix.
		if y.prefix == "" || strings.HasPrefix(path, y.prefix) {
			files[path] = content
		}
	}

	return files
}

// Sub creats a subview of the yarn only covering the provided
// directory.
func (y *yarn) Sub(path string) Yarn {
	if y.prefix != "" {
		path = filepath.Join(y.prefix, path)
	}

	return &yarn{
		prefix: path,
		yarns:  y.yarns,
	}

}

// Walk calls the provided function for each file.
// Exactly `**` matches any file.
func (y *yarn) Walk(pattern string, visitor func(path string, content string) error) error {

	// we use all to take care of subviews.
	if pattern == "**" {
		for path, content := range y.All() {
			err := visitor(path, content)
			if err != nil {
				return err
			}
		}
		return nil
	}

	for path, content := range y.All() {
		match, _ := filepath.Match(pattern, path)
		if match {
			err := visitor(path, content)
			if err != nil {
				return err
			}
		}
	}

	return nil
}
