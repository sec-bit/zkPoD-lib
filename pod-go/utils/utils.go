package utils

import (
	"fmt"
	"os"
	"path/filepath"
)

// CheckRegularFileReadPerm checks following characters of the
// specified file:
//  1. the file exists
//  2. the file is a regular file
//  3. the file is readable
func CheckRegularFileReadPerm(fname string) error {
	st, err := os.Stat(fname)
	if err != nil {
		return err
	}

	if !st.Mode().IsRegular() {
		return fmt.Errorf("%s is not regular file", fname)
	}

	f, err := os.OpenFile(fname, os.O_RDONLY, 0666)
	if err != nil {
		return err
	}
	_ = f.Close()

	return nil
}

// CheckDirReadPerm checks following characters of the specified
// directory:
//  1. the directory exists
//  2. the directory is a directory
//  3. the directory is readable
func CheckDirReadPerm(dname string) error {
	st, err := os.Stat(dname)
	if err != nil {
		return err
	}

	if !st.Mode().IsDir() {
		return fmt.Errorf("%s is not directory", dname)
	}

	d, err := os.OpenFile(dname, os.O_RDONLY, 1777)
	if err != nil {
		return err
	}
	_ = d.Close()

	return nil
}

// CheckDirOfPathExistence checks following characters of all but the
// last element of the specified path:
//  1. it exists
//  2. it is a directory
func CheckDirOfPathExistence(path string) error {
	return CheckDirReadPerm(filepath.Dir(path))
}
