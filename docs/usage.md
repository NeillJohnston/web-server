# Usage

## Routing

Routing paths are specified by the DB. The server expects a table named `ROUTES` to exist, with columns `METHOD(VARCHAR)`, `ROUTE(VARCHAR)`, `PATH(VARCHAR)`, and `TYPE(INT)`.

Each row represents a route. `METHOD` is the request method the user sent and `ROUTE` is the request URI that a user tries to access. `PATH` is the file that the route maps to and `TYPE` is a magic number that specifies how the `PATH` should be interpreted:

-	`0`: Static file, return this in its entirety as the content of the HTTP response.
-	`1`: Database query, execute `PATH` as a SQL query to the database and return the result (if any). If the query requires parameters, they should be specified in the body of the HTTP request.
-	Any other values for `TYPE` will trigger a 500.

Example `ROUTES` table:

```
METHOD	ROUTE			PATH			TYPE	
------- --------------- --------------- ------- 
GET		/				/index.html		0
GET		/foo			/foo/index.html	0
GET		/api/bar		/query/bar.sql	1
POST	/api/baz		/query/baz.sql	1
ERROR	404				/404/index.html	0
```

If no row is found for the specified route and method, the server's response depends on the method:

-	For `GET` requests, attempt to reroute to the file specified by the request URI relative to the server root (i.e., treat the request as an attempt to get a file as normal).
-	For all other requests, trigger a 404.

### Errors

You can also use an HTTP error code as the value for `ROUTE`. If the server encounters an error, it will look for a row in `ROUTES` with `(METHOD=ERROR, ROUTE=<error code>)`. If a row is found, the server will attempt to reroute and use that as the response body, keeping the original reponse code. If that reroute fails, a 500 is triggered that will return the default response body. Otherwise, if no row is found, a default response is provided. Note the method name is "ERROR". This is chosen to avoid potential conflicts with real HTTP requests, and make it impossible to directly request for errors.