/*
** Copyright (C) 2001-2025 Zabbix SIA
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
** documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial portions
** of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
** WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/

// Package conf provides .conf file loading and unmarshalling
package conf

import (
	"bytes"
	"encoding/base64"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"regexp"
	"runtime"
	"strconv"
	"strings"
	"sync"
	"unicode/utf8"

	"golang.zabbix.com/sdk/errs"
	"golang.zabbix.com/sdk/log"
	"golang.zabbix.com/sdk/std"
)

var stdOs std.Os

var (
	rootConfigPath     string                                                  //nolint:gochecknoglobals
	openConfigPath     string                                                  //nolint:gochecknoglobals
	configEnvVarRegexp = regexp.MustCompile(`^\$\{[a-zA-Z_]+[a-zA-Z0-9_]*\}$`) //nolint:gochecknoglobals
	envVarsToUnset     = map[string]struct{}{}                                 //nolint:gochecknoglobals
	envVarsMutex       sync.Mutex                                              //nolint:gochecknoglobals
	isReloadUserParams = false                                                 //nolint:gochecknoglobals
)

// ErrInvalidRangeFormat is returned when option meta contains invalid range.
var ErrInvalidRangeFormat = errs.New("invalid range format")

// Errors returned during option value parsing, if value doesn't match limitations in meta.
var (
	ErrValueOutOfRange    = errs.New("value out of range")
	ErrValueCannotBeEmpty = errs.New("value cannot be empty")
)

// Meta structure is used to store the 'conf' tag metadata.
type Meta struct {
	name         string
	defaultValue *string
	optional     bool
	noEmptyValue bool
	min          int64
	max          int64
}

// Suffix describes duration suffixes (s - sec, m - min, h - hour, ...) and
// their factors (1, 60, 3600, ...).
type Suffix struct {
	suffix string
	factor int
}

func init() {
	stdOs = std.NewOs()
}

// setCurrentConfigPath sets a path of the root config file.
func setCurrentConfigPath(path string) {
	rootConfigPath = path
}

// GetCurrentConfigPath returns a path of the root config file.
func GetCurrentConfigPath() string {
	return rootConfigPath
}

func validateParameterName(key []byte) error {
	for i, b := range key {
		if ('A' > b || b > 'Z') && ('a' > b || b > 'z') && ('0' > b || b > '9') && b != '_' && b != '.' {
			return fmt.Errorf("invalid character '%c' at position %d", b, i+1)
		}
	}

	return nil
}

// parseLine parses parameter configuration line and returns key,value pair.
// The line must have format: <key>[ ]=[ ]<value> where whitespace surrounding
// '=' is optional.
func parseLine(line []byte) ([]byte, []byte, error) {
	valueStart := bytes.IndexByte(line, '=')
	if valueStart == -1 {
		return nil, nil, errors.New("missing assignment operator")
	}

	key := bytes.TrimSpace(line[:valueStart])
	if len(key) == 0 {
		return nil, nil, errors.New("missing variable name")
	}

	err := validateParameterName(key)
	if err != nil {
		return nil, nil, err
	}

	if valueStart+1 == len(line) {
		return key, []byte(""), nil
	} else {
		return key, bytes.TrimSpace(line[valueStart+1:]), nil
	}
}

// getMeta returns 'conf' tag metadata.
// The metadata has format [name=<name>,][optional,][range=<range>,][default=<default value>]
//
//	where:
//	<name> - the parameter name,
//	optional - set if the value is optional,
//	<range> - the allowed range <min>:<max>, where <min>, <max> values are optional,
//	<default value> - the default value. If specified it must always be the last tag.
func getMeta(field reflect.StructField) (meta *Meta, err error) {
	m := Meta{min: -1, max: -1}
	conf := field.Tag.Get("conf")

loop:
	for conf != "" {
		tags := strings.SplitN(conf, ",", 2)
		fields := strings.SplitN(tags[0], "=", 2)
		tag := strings.TrimSpace(fields[0])
		if len(fields) == 1 {
			// boolean tag
			switch tag {
			case "optional":
				m.optional = true
			case "nonempty":
				m.noEmptyValue = true
			default:
				return nil, errs.Wrapf(err, "invalid tag - %q", tag)
			}
		} else {
			// value tag
			switch tag {
			case "default":
				value := fields[1]
				if len(tags) == 2 {
					value += "," + tags[1]
				}
				m.defaultValue = &value

				break loop
			case "name":
				m.name = strings.TrimSpace(fields[1])
			case "range":
				limits := strings.Split(fields[1], ":")
				if len(limits) != 2 {
					return nil, ErrInvalidRangeFormat
				}
				if limits[0] != "" {
					m.min, _ = strconv.ParseInt(limits[0], 10, 64)
				}
				if limits[1] != "" {
					m.max, _ = strconv.ParseInt(limits[1], 10, 64)
				}
			default:
				return nil, errs.Wrapf(err, "invalid tag - %q", tag)
			}
		}

		if len(tags) == 1 {
			break
		}
		conf = tags[1]
	}

	if m.name == "" {
		m.name = field.Name
	}

	return &m, nil
}

func getTimeSuffix(str string) (string, int) {
	suffixes := []Suffix{
		{
			suffix: "s",
			factor: 1,
		},
		{
			suffix: "m",
			factor: 60,
		},
		{
			suffix: "h",
			factor: 3600,
		},
		{
			suffix: "d",
			factor: 86400,
		},
		{
			suffix: "w",
			factor: (7 * 86400),
		},
	}

	for _, s := range suffixes {
		if strings.HasSuffix(str, s.suffix) {
			str = strings.TrimSuffix(str, s.suffix)

			return str, s.factor
		}
	}

	return str, 1
}

func setBasicValue(value reflect.Value, meta *Meta, str *string) (err error) {
	if str == nil {
		return nil
	}

	if meta != nil && meta.noEmptyValue && *str == "" {
		return ErrValueCannotBeEmpty
	}

	switch value.Type().Kind() {
	case reflect.String:
		value.SetString(*str)
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		var v int64
		var r int

		handleEmpty(str)

		*str, r = getTimeSuffix(*str)
		if v, err = strconv.ParseInt(*str, 10, 64); err == nil {
			v = v * int64(r)
			if meta != nil {
				if meta.min != -1 && v < meta.min || meta.max != -1 && v > meta.max {
					return ErrValueOutOfRange
				}
			}
			value.SetInt(v)
		}
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
		var v uint64
		var r int
		*str, r = getTimeSuffix(*str)

		handleEmpty(str)

		if v, err = strconv.ParseUint(*str, 10, 64); err == nil {
			v = v * uint64(r)
			if meta != nil {
				if meta.min != -1 && v < uint64(meta.min) || meta.max != -1 && v > uint64(meta.max) {
					return ErrValueOutOfRange
				}
			}
			value.SetUint(v)
		}
	case reflect.Float32, reflect.Float64:
		handleEmpty(str)

		var v float64
		if v, err = strconv.ParseFloat(*str, 64); err == nil {
			if meta != nil {
				if meta.min != -1 && v < float64(meta.min) || meta.max != -1 && v > float64(meta.max) {
					return ErrValueOutOfRange
				}
			}
			value.SetFloat(v)
		}
	case reflect.Bool:
		var v bool

		switch *str {
		case "true":
			v = true
		case "false":
			v = false
		default:
			return errors.New("invalid boolean value")
		}

		value.SetBool(v)
	case reflect.Ptr:
		v := reflect.New(value.Type().Elem())

		value.Set(v)

		return setBasicValue(v.Elem(), meta, str)
	default:
		err = fmt.Errorf("unsupported variable type %v", value.Type().Kind())
	}

	return err
}

func handleEmpty(str *string) {
	if *str == "" {
		*str = "0"
	}
}

func setStructValue(value reflect.Value, node *Node) error {
	rt := value.Type()
	for i := 0; i < rt.NumField(); i++ {
		meta, err := getMeta(rt.Field(i))
		if err != nil {
			return err
		}

		child := node.get(meta.name)
		if child != nil || meta.defaultValue != nil {
			err := setValue(value.Field(i), meta, child)
			if err != nil {
				return err
			}

			continue
		}

		if !meta.optional {
			return errs.Errorf("cannot find mandatory parameter %s", meta.name)
		}
	}

	return nil
}

func setMapValue(value reflect.Value, node *Node) (err error) {
	m := reflect.MakeMap(reflect.MapOf(value.Type().Key(), value.Type().Elem()))

	for _, v := range node.Nodes {
		if child, ok := v.(*Node); ok {
			k := reflect.New(value.Type().Key())

			if err = setBasicValue(k.Elem(), nil, &child.Name); err != nil {
				return
			}
			v := reflect.New(value.Type().Elem())
			if err = setValue(v.Elem(), nil, child); err != nil {
				return
			}
			m.SetMapIndex(k.Elem(), v.Elem())
		}
	}

	value.Set(m)

	return
}

func setSliceValue(value reflect.Value, node *Node) (err error) {
	tmpValues := make([][]byte, 0)
	for _, v := range node.Nodes {
		if val, ok := v.(*Value); ok {
			tmpValues = append(tmpValues, val.Value)
		}
	}
	size := len(tmpValues)
	values := reflect.MakeSlice(reflect.SliceOf(value.Type().Elem()), 0, size)

	if len(tmpValues) > 0 {
		for _, data := range tmpValues {
			v := reflect.New(value.Type().Elem())
			str := string(data)
			if err = setBasicValue(v.Elem(), nil, &str); err != nil {
				return
			}
			values = reflect.Append(values, v.Elem())
		}
	} else {
		for _, n := range node.Nodes {
			if child, ok := n.(*Node); ok {
				v := reflect.New(value.Type().Elem())
				if err = setValue(v.Elem(), nil, child); err != nil {
					return
				}
				values = reflect.Append(values, v.Elem())
			}
		}
	}

	value.Set(values)

	return
}

func setValue(value reflect.Value, meta *Meta, node *Node) (err error) {
	var str *string

	if node != nil {
		node.used = true
	}

	switch value.Type().Kind() {
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64,
		reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64,
		reflect.Float32, reflect.Float64, reflect.Bool, reflect.String:
		if str, err = node.getValue(meta); err == nil {
			if err = setBasicValue(value, meta, str); err != nil {
				return node.newError("%s", err.Error())
			}
		}
	case reflect.Struct:
		if node != nil {
			return setStructValue(value, node)
		}
	case reflect.Map:
		if node != nil {
			return setMapValue(value, node)
		}
	case reflect.Slice:
		if node != nil {
			return setSliceValue(value, node)
		}
	case reflect.Ptr:
		v := reflect.New(value.Type().Elem())
		value.Set(v)
		return setValue(v.Elem(), meta, node)
	case reflect.Interface:
		value.Set(reflect.ValueOf(node))
		node.markUsed(true)
	}

	return nil
}

// assignValues assigns parsed nodes to the specified structure
func assignValues(v interface{}, root *Node) (err error) {
	rv := reflect.ValueOf(v)

	switch rv.Type().Kind() {
	case reflect.Ptr:
		rv = rv.Elem()
	default:
		return errors.New("output variable must be a pointer to a structure")
	}

	switch rv.Type().Kind() {
	case reflect.Struct:
		if err = setStructValue(rv, root); err != nil {
			return err
		}
	default:
		return errors.New("output variable must be a pointer to a structure")
	}

	return root.checkUsage()
}

func newIncludeError(root *Node, filename *string, errmsg string) (err error) {
	if root.includeFail {
		return errors.New(errmsg)
	}

	root.includeFail = true
	if filename != nil {
		return fmt.Errorf(`cannot include "%s": %s`, *filename, errmsg)
	}

	return fmt.Errorf(`cannot load file: %s`, errmsg)
}

func hasMeta(path string) bool {
	var metaChars string

	if runtime.GOOS != "windows" {
		metaChars = `*?[\`
	} else {
		metaChars = `*?[`
	}

	return strings.ContainsAny(path, metaChars)
}

func loadInclude(root *Node, path string) (err error) {
	path = filepath.Clean(path)
	if err := checkGlobPattern(path); err != nil {
		return newIncludeError(root, &path, err.Error())
	}

	absPath, err := filepath.Abs(path)
	if err != nil {
		return newIncludeError(root, &path, err.Error())
	}

	// If a path is relative, pad it with a directory of the current config file
	if path != absPath {
		confDir := filepath.Dir(GetCurrentConfigPath())
		path = filepath.Join(confDir, path)
	}

	if hasMeta(filepath.Dir(path)) {
		return newIncludeError(root, &path, "glob pattern is supported only in file names")
	}
	if !hasMeta(path) {
		var fi os.FileInfo
		if fi, err = stdOs.Stat(path); err != nil {
			return newIncludeError(root, &path, err.Error())
		}
		if fi.IsDir() {
			path = filepath.Join(path, "*")
		}
	} else {
		var fi os.FileInfo
		if fi, err = stdOs.Stat(filepath.Dir(path)); err != nil {
			return newIncludeError(root, &path, err.Error())
		}
		if !fi.IsDir() {
			return newIncludeError(root, &path, "base path is not a directory")
		}
	}

	var paths []string

	if hasMeta(path) {
		if paths, err = filepath.Glob(path); err != nil {
			return newIncludeError(root, nil, err.Error())
		}
	} else {
		paths = append(paths, path)
	}

	for _, path := range paths {
		// skip directories
		var fi os.FileInfo
		if fi, err = stdOs.Stat(path); err != nil {
			return newIncludeError(root, &path, err.Error())
		}
		if fi.IsDir() {
			continue
		}

		var file std.File
		if file, err = stdOs.Open(path); err != nil {
			return newIncludeError(root, &path, err.Error())
		}
		defer file.Close()

		buf := bytes.Buffer{}
		if _, err = buf.ReadFrom(file); err != nil {
			return newIncludeError(root, &path, err.Error())
		}

		openConfigPath = path

		if err = parseConfig(root, buf.Bytes()); err != nil {
			return newIncludeError(root, &path, err.Error())
		}
	}
	return
}

func checkGlobPattern(path string) error {
	if strings.HasPrefix(path, "*") {
		return errors.New("path should be absolute")
	}

	var isGlob, hasSepLeft, hasSepRight bool

	for _, p := range path {
		switch p {
		case '*':
			isGlob = true
		case filepath.Separator:
			switch isGlob {
			case true:
				hasSepRight = true
			case false:
				hasSepLeft = true
			}
		}
	}

	if (isGlob && !hasSepLeft && hasSepRight) || (isGlob && !hasSepLeft && !hasSepRight) {
		return errors.New("path should be absolute")
	}

	return nil
}

func lookupEnv(k, v []byte, line int) ([]byte, error) {
	if !configEnvVarRegexp.Match(v) {
		return v, nil
	}

	key := string(k)
	value := string(v)

	if isReloadUserParams && (key == "UserParameter" || key == "Include") {
		log.Warningf(
			"environment variables are not supported during user parameters reloading, skipped parameter"+
				" %q with value %q at line %d in config file %q",
			key,
			value,
			line,
			openConfigPath,
		)

		return nil, nil
	}

	envName := strings.TrimPrefix(value, "${")
	envName = strings.TrimSuffix(envName, "}")

	envValue, found := os.LookupEnv(envName)
	if !found {
		return nil, nil
	}

	if !utf8.ValidString(envValue) {
		return nil, errs.Errorf(
			"non-UTF-8 character in environment variable %q value %q at line %d in config file %q",
			value,
			envValue,
			line,
			openConfigPath,
		)
	}

	if strings.Contains(envValue, "\n") {
		return nil, errs.Errorf(
			"multi-line string in environment variable %q value %q at line %d in config file %q",
			value,
			envValue,
			line,
			openConfigPath,
		)
	}

	envVarsMutex.Lock()
	envVarsToUnset[envName] = struct{}{}
	envVarsMutex.Unlock()

	return []byte(envValue), nil
}

func parseConfig(root *Node, data []byte) (err error) {
	const maxStringLen = 2048
	var line []byte

	root.level++

	for offset, end, num := 0, 0, 1; end != -1; offset, num = offset+end+1, num+1 {
		if end = bytes.IndexByte(data[offset:], '\n'); end != -1 {
			line = bytes.TrimSpace(data[offset : offset+end])
		} else {
			line = bytes.TrimSpace(data[offset:])
		}

		if len(line) > maxStringLen {
			return fmt.Errorf("cannot parse configuration at line %d: limit of %d bytes is exceeded", num, maxStringLen)
		}

		if len(line) == 0 || line[0] == '#' {
			continue
		}

		if !utf8.ValidString(string(line)) {
			return fmt.Errorf("cannot parse configuration at line %d: not a valid UTF-8 character found", num)
		}

		var key, value []byte
		if key, value, err = parseLine(line); err != nil {
			return fmt.Errorf("cannot parse configuration at line %d: %s", num, err.Error())
		}

		value, err = lookupEnv(key, value, num)
		if err != nil {
			return errs.Wrap(err, "cannot lookup environment variable")
		}

		if value == nil {
			continue
		}

		if string(key) == "Include" {
			if root.level == 10 {
				return fmt.Errorf("include depth exceeded limits")
			}
			if err = loadInclude(root, string(value)); err != nil {
				return
			}
		} else {
			root.add(key, value, num)
		}
	}

	root.level--

	return nil
}

func addObject(parent *Node, v any) error {
	if attr, ok := v.(map[string]any); ok {
		if _, ok := attr["Nodes"]; ok {
			node := &Node{}
			if err := setObjectNode(node, attr); err != nil {
				return err
			}

			parent.Nodes = append(parent.Nodes, node)
		} else {
			value := &Value{}
			if err := setObjectValue(value, attr); err != nil {
				return err
			}
			parent.Nodes = append(parent.Nodes, value)
		}
	} else {
		return fmt.Errorf("invalid object type %T", v)
	}

	return nil
}

func setObjectValue(value *Value, attr map[string]any) error {
	var (
		line float64
		ok   bool
	)

	if line, ok = attr["Line"].(float64); !ok {
		return fmt.Errorf("invalid line attribute type %T", attr["Line"])
	}

	value.Line = int(line)

	var (
		err  error
		data string
	)

	if data, ok = attr["Value"].(string); !ok {
		return fmt.Errorf("invalid value type %T", attr["Value"])
	}

	if value.Value, err = base64.StdEncoding.DecodeString(data); err != nil {
		return err
	}

	return nil
}

func setObjectNode(node *Node, attr map[string]interface{}) error {
	var line float64
	var ok bool

	if line, ok = attr["Line"].(float64); !ok {
		return fmt.Errorf("invalid line attribute type %T", attr["Line"])
	}

	node.Line = int(line)

	if node.Name, ok = attr["Name"].(string); !ok {
		return fmt.Errorf("invalid node name type %T", attr["Name"])
	}

	var nodes []interface{}

	if nodes, ok = attr["Nodes"].([]interface{}); !ok {
		return fmt.Errorf("invalid node children type %T", attr["u"])
	}

	for _, a := range nodes {
		if err := addObject(node, a); err != nil {
			return err
		}
	}

	return nil
}

// Unmarshal unmarshals data into specified structure using non strict rules
// for more information read unmarshal function.
func Unmarshal(data, v any) error {
	return unmarshal(data, v, false)
}

// UnmarshalStrict unmarshals data into specified structure using strict rules
// for more information read unmarshal function.
func UnmarshalStrict(data, v any) error {
	return unmarshal(data, v, true)
}

// unmarshal unmarshals input data into specified structure. The input data can be either
// a byte array ([]byte) with configuration file or interface{} either returned by Marshal
// or a configuration file Unmarshaled into interface{} variable before.
// The third is optional 'strict' parameter that forces strict validation of configuration
// and structure fields (enabled by default). When disabled it will unmarshal part of
// configuration into incomplete target structures.
//
//nolint:gocyclo,cyclop
func unmarshal(data, v any, strict bool) error {
	rv := reflect.ValueOf(v)

	if rv.Kind() != reflect.Ptr || rv.IsNil() {
		return errs.New("invalid output parameter")
	}

	var root *Node

	switch u := data.(type) {
	case nil:
		root = &Node{
			Name:   "",
			used:   false,
			Nodes:  make([]interface{}, 0),
			parent: nil,
			Line:   0,
		}
	case []byte:
		root = &Node{
			Name:   "",
			used:   false,
			Nodes:  make([]interface{}, 0),
			parent: nil,
			Line:   0,
		}

		err := parseConfig(root, u)
		if err != nil {
			return errs.Wrap(err, "cannot read configuration")
		}
	case *Node:
		root = u
		root.markUsed(false)
	case map[string]interface{}: // JSON unmarshaling result
		root = &Node{}

		err := setObjectNode(root, u)
		if err != nil {
			return errs.Wrap(err, "cannot unmarshal JSON data")
		}
	default:
		return errs.Errorf("invalid input parameter of type %T", u)
	}

	if !strict {
		root.markUsed(true)
	}

	err := assignValues(v, root)
	if err != nil {
		return errs.Wrap(err, "cannot assign configuration")
	}

	return nil
}

func unsetConfEnvVars() {
	envVarsMutex.Lock()

	for k := range envVarsToUnset {
		err := os.Unsetenv(k)
		if err != nil {
			log.Warningf("failed to unset environment variable %s: %s", k, err.Error())
		}
	}

	envVarsToUnset = map[string]struct{}{}
	envVarsMutex.Unlock()
}

func Load(filename string, v interface{}) (err error) {
	var file std.File

	if file, err = stdOs.Open(filename); err != nil {
		return fmt.Errorf(`cannot open configuration file: %s`, err.Error())
	}
	defer file.Close()

	buf := bytes.Buffer{}
	if _, err = buf.ReadFrom(file); err != nil {
		return fmt.Errorf("cannot load configuration: %s", err.Error())
	}

	setCurrentConfigPath(filename)
	openConfigPath = filename

	defer unsetConfEnvVars()

	return UnmarshalStrict(buf.Bytes(), v)
}

func LoadUserParams(v interface{}) (err error) {
	var file std.File

	if file, err = stdOs.Open(rootConfigPath); err != nil {
		return fmt.Errorf(`cannot open configuration file: %s`, err.Error())
	}
	defer file.Close()

	buf := bytes.Buffer{}
	if _, err = buf.ReadFrom(file); err != nil {
		return fmt.Errorf("cannot load configuration: %s", err.Error())
	}

	isReloadUserParams = true
	openConfigPath = rootConfigPath

	return Unmarshal(buf.Bytes(), v)
}
