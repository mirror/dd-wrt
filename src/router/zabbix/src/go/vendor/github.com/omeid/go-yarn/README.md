[![GoDoc](https://godoc.org/github.com/omeid/go-yarn?status.svg)](https://godoc.org/github.com/omeid/go-yarn) [![Coverage Status](https://coveralls.io/repos/omeid/go-yarn/badge.png)](https://coveralls.io/r/omeid/go-yarn) [![Build Status](https://drone.io/github.com/omeid/go-yarn/status.png)](https://drone.io/github.com/omeid/go-yarn/latest)
# Yarn
Filesystem mapped key-string store. Ideal for embedding code like sql.

### Why?
Because programming in string literals sucks.

Lack of syntax highlighting, auto-completion, and other helpful tools when writing SQL or other programming languages make the task tedious and error prone, Yarn is here to help you pull your SQL strings into `*.sql` files and other code in appropriate files so you can use the right tools when coding.


### Documentation
See the [GoDoc](https://godoc.org/github.com/omeid/go-yarn)


### Example


```go
package main

import (
	"log"
	"net/http"

	"github.com/omeid/go-yarn"
)

var sqls = yarn.Must(http.Dir("./sqls"), "*.sql")

func main() {
	sqls.MustHave("users_table.sql", "query_all.sql", "query_user.sql") //Panics otherwise.

    //Or the following if you're into that sort of stuff.
    /*
  	err := sqls.Has("users_table.sql", "query_all.sql", "query_user.sql")
	if err != nil {
		log.Fatal(err)
	}
    */

	res, err := sql.Exec(sqls.Must("users_table.sql"), params...)
	//Just deal with it.
}
```

#### _Moar_ Files to ship, Smack That, you say?
Don't worry.  
I hate complicated stuff too and there is a reason Yarn is using a Virtual Filesystem, http.FileSystem namely, to allow embedding!   
You can simply use [go-resources](https://github.com/omeid/go-resources) to embed all your sql and other codes files,

Make sure you read the  _"Live" development of resources_ section for friction-free development.



### Catalog

If you prefer to use a single file with chunks of code (e.g sql statements, lua functions, or so forth), please see the [catalog](catalog/README.md) pacakge.

### Contributing
Please consider opening an issue first, or just send a pull request. :)

### Credits
See [Contributors](https://github.com/omeid/go-yarn/graphs/contributors).  
Inspired by [smotes/purse](https://github.com/smotes/purse).

### LICENSE
  [MIT](LICENSE).
