#!/usr/bin/env python

# Illustrates the creation of an HTTP request using X-OTP-Auth for testing a
# PourLogic server instance.

import sys
import hmac
import hashlib
import base64

if len(sys.argv) != 5:
    print >>sys.stderr, "Usage: {0} secret_key client_id drinker_id otp_count".format(sys.argv[0])
    exit(1)

# Parameters
secret_key = sys.argv[1] 
client_id = sys.argv[2]
drinker_id = sys.argv[3]
count = sys.argv[4] 

uri = '/pours/new'
method = 'GET'
query_string = 'k={0}&u={1}'.format(client_id, drinker_id)
message_body = ''

# This is the string that will be hashed
canonical_string = "{0}\n{1}\n{2}\n{3}".format(count, method, query_string, message_body)

# The key that is used is the hash of the plaintext secret (easier to store on
# Arduino and happens anyway with longer keys)
h = hashlib.new('sha256')
h.update(secret_key)
effective_key = h.digest()

auth_header = 'X-Otp-Auth'

hmac_result = hmac.new(effective_key, msg=canonical_string, digestmod=hashlib.sha256).hexdigest()

print '{0} {1}/?{2} HTTP/1.0'.format(method, uri, query_string)
print '{0}: {1}:{2}:{3}'.format(auth_header, client_id, count, hmac_result)
print
print
