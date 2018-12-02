# Problem

If you are seeing these errors in the application output:

```
qt.network.ssl: QSslSocket: cannot call unresolved function SSLv23_client_method
qt.network.ssl: QSslSocket: cannot call unresolved function SSL_CTX_new
qt.network.ssl: QSslSocket: cannot call unresolved function SSL_library_init
qt.network.ssl: QSslSocket: cannot call unresolved function ERR_get_error
qt.network.ssl: QSslSocket: cannot call unresolved function ERR_get_error
```

it means your Qt installation does not provide SSL libraries.

# Solution

Copy libeay32.dll and ssleay32.dll from the bin/ folder to the same folder as
the executable and try again.

Source: http://www.blikoontech.com/networking/how-to-get-rid-of-ssl-warnings-in-qt-applications-qsslsocket-cannot-resolve-tlsv1_1_client_method-warnings
