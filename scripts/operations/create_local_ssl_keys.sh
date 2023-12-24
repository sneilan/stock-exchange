#!/bin/sh

openssl genpkey -algorithm RSA -out ${PRIVATE_KEY}
openssl rsa -pubout -in ${PRIVATE_KEY} -out ${PUBLIC_KEY}
