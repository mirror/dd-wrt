package dsn

// See https://github.com/dvyukov/go-fuzz
//
// (cd /tmp && go get -u github.com/dvyukov/go-fuzz/go-fuzz github.com/dvyukov/go-fuzz/go-fuzz-build)
// PATH=$HOME/sdk/go1.14.6/bin:$PATH GO111MODULE=on go-fuzz-build
// go-fuzz
func Fuzz(data []byte) int {
	P, err := Parse(string(data))
	if err != nil {
		return -1
	}
	s := P.StringWithPassword()
	Q, err := Parse(s)
	if err != nil {
		return 1
	}
	if s != Q.StringWithPassword() {
		return 1
	}
	return 0
}
