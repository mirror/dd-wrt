//go:build never
// +build never

// Copyright 2021 The Godror Authors

package main

import (
	"archive/zip"
	"bufio"
	"bytes"
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"time"
)

func main() {
	if err := Main(); err != nil {
		log.Fatalf("%+v", err)
	}
}

func Main() error {
	// flagPackage := flag.String("pkg", "godror", "package name to use")
	flagOut := flag.String("o", "", "output file name")
	flag.Parse()
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
	defer cancel()
	b, err := exec.CommandContext(ctx, "go", "env", "GOROOT").Output()
	if err != nil {
		return err
	}
	goroot := string(bytes.TrimSpace(b))
	fn := filepath.Join(goroot, "lib", "time", "zoneinfo.zip")
	log.Printf("go env GOROOT: %s -> fn=%q", b, fn)
	zr, err := zip.OpenReader(fn)
	if err != nil {
		fn = filepath.Join(goroot, "src", "time", "tzdata", "zzipdata.go")
		b, err := os.ReadFile(fn)
		if err != nil {
			return err
		}
		const prefix = "const zipdata = "
		if i := bytes.Index(b, []byte(prefix)); i < 0 {
			return fmt.Errorf("no const zipdata in %q", string(b[:128]))
		} else {
			b = b[i+len(prefix):]
			if i = bytes.LastIndexByte(b[1:], b[0]); i < 0 {
				return fmt.Errorf("no string end %c in %q", b[0], string(b[len(b)-128:]))
			}
			s, err := strconv.Unquote(string(b[:i+2]))
			if err != nil {
				return err
			}
			br := strings.NewReader(s)
			if r, err := zip.NewReader(br, br.Size()); err != nil {
				return fmt.Errorf("%q %q ... %q: %w", fn, string(b[:128]), string(b[len(b)-128:]), err)
			} else {
				zr = &zip.ReadCloser{Reader: *r}
			}
		}
	}
	defer zr.Close()
	names := make([]string, 0, len(zr.File))
	for _, f := range zr.File {
		if f.Mode().IsDir() {
			continue
		}
		names = append(names, strings.ToLower(f.Name)+f.Name)
	}
	sort.Strings(names)
	fh := os.Stdout
	if !(*flagOut == "" || *flagOut == "-") {
		if fh, err = os.Create(*flagOut); err != nil {
			return err
		}
	}
	defer fh.Close()
	bw := bufio.NewWriter(fh)
	for _, name := range names {
		bw.WriteString(name)
		bw.WriteByte('\n')
	}
	return fh.Close()
}
