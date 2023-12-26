#!/bin/sh

openssl genpkey -algorithm RSA -out ${PRIVATE_KEY}
openssl rsa -pubout -in ${PRIVATE_KEY} -out ${PUBLIC_KEY}

openssl req -new -key ${PRIVATE_KEY} -out ${CERTIFICATE_REQUEST} -subj "/C=US/ST=CA/L=San Francisco/O=My Organization/OU=My Department/CN=mydomain.com"
openssl x509 -req -in ${CERTIFICATE_REQUEST} -signkey ${PRIVATE_KEY} -out ${CERTIFICATE}
