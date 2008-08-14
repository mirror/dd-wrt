

                 How to setup a test Certificate Authority
                 =========================================



    1. Generate the CA certificate itself
    -------------------------------------

openssl req -sha1 -days 3653 -x509 -newkey rsa:2048 \
            -text -keyout test-ca.key -out test-ca.crt


    2. Generate the private keys and certificate requests
    -----------------------------------------------------

openssl genrsa -out server.key 2048
openssl genrsa -out client.key 2048

openssl req -new -key server.key -out server.req
openssl req -new -key client.key -out client.req


    3. Issue and sign the certificates
    ----------------------------------

openssl x509 -sha1 -days 365 -req -CA test-ca.crt -CAkey test-ca.key \
             -text -in server.req -out server.crt

openssl x509 -sha1 -days 365 -req -CA test-ca.crt -CAkey test-ca.key \
             -text -in client.req -out client.crt

cat server.crt server.key > server.pem
cat client.crt client.key > client.pem


    4. Verify the certificate signatures
    ------------------------------------

openssl verify -CAfile test-ca.crt server.crt
openssl verify -CAfile test-ca.crt client.crt


