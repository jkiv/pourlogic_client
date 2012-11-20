#!/usr/bin/env python

# Illustrates the creation of an HTTP request using X-PourLogic-Auth for testing a
# PourLogic server instance.

import sys
import hmac
import hashlib
import base64

if __name__ == "__main__":
  if len(sys.argv) != 7:
      print >>sys.stderr, "Usage: {0} server_ip server_port secret_key bot_id drinker_id otp_count".format(sys.argv[0])
      exit(1)

  # Parameters
  server_ip = sys.argv[1]
  server_port = int(sys.argv[2])
  secret_key = sys.argv[3] 
  bot_id = sys.argv[4]
  drinker_id = sys.argv[5]
  count = sys.argv[6]

  uri = '/pours/new'
  message_body = ''
  
  query_parts = ["GET {uri}/?u={drinker_id} HTTP/1.0",
                 "X-PourLogic-Auth: {bot_id}:{count}:{hmac}",
                 "\r\n{message_body}"]

  # This is the string that will be hashed
  canonical_string = "{count}\n{request_line}\n{message_body}".format(count=count,
                                                                      request_line=query_parts[0].format(uri=uri, drinker_id=drinker_id),
                                                                      message_body=message_body)

  # The key that is used is the hash of the plaintext secret (easier to store on
  # Arduino and happens anyway with longer keys)
  h = hashlib.new('sha256')
  h.update(secret_key)
  effective_key = h.digest()

  hmac_result = hmac.new(effective_key, msg=canonical_string, digestmod=hashlib.sha256).hexdigest()

  # Connect to server
  sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  sock.connect((server_ip, server_port))
  
  # Send query
  sock.sendall(query_parts.join('\r\n').format(uri=uri,
                                               drinker_id=drinker_id,
                                               bot_id=bot_id,
                                               count=count,
                                               hmac=hmac,
                                               message_body=message_body)
                                        
  # Print result
  while True:
    data = sock.recv(1024)
    if not data:
      break
    print data

  sock.close()