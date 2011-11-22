#!/usr/bin/env python
# (c) 2011 jkiv.ca/client

# Illustrates the creation of an HTTP request using X-OTP-Auth for testing a
# PourLogic server instance.

import sys
import hmac
import hashlib
import base64

if len(sys.argv) != 6:
    print >>sys.stderr, "Usage: {0} secret_key client_id drinker_id otp_count poured_volume".format(sys.argv[0])
    exit(1)

# Parameters
secret_key = sys.argv[1] 
client_id = sys.argv[2]
drinker_id = sys.argv[3]
count = sys.argv[4]
volume = sys.argv[5]

uri = '/pours/create/'
method = 'POST'
query_string = ''
message_body = 'k={0}&u={1}&v={2}'.format(client_id, drinker_id, volume)

canonical_string = "{0}\n{1}\n{2}\n{3}".format(count, method, query_string, message_body)

# Hash secret key string
h = hashlib.new('sha256')
h.update(secret_key)
effective_key = h.digest()

auth_header = 'X-Otp-Auth'

hmac_result = hmac.new(effective_key, msg=canonical_string, digestmod=hashlib.sha256).hexdigest()

print '{0} {1} HTTP/1.0'.format(method, uri, query_string)
print 'Content-Length: {0}'.format(len(message_body))
print '{0}: {1}:{2}:{3}'.format(auth_header, client_id, count, hmac_result)
print
print message_body
