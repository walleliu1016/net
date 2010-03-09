// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// HttpAlternateProtocols is an in-memory data structure used for keeping track
// of which HTTP HostPortPairs have an alternate protocol that can be used
// instead of HTTP on a different port.

#ifndef NET_HTTP_HTTP_ALTERNATE_PROTOCOLS_H_
#define NET_HTTP_HTTP_ALTERNATE_PROTOCOLS_H_

#include <map>
#include <utility>
#include "base/basictypes.h"
#include "net/base/host_port_pair.h"

namespace net {

class HttpAlternateProtocols {
 public:
  enum Protocol {
    BROKEN,  // The alternate protocol is known to be broken.
    SPDY,
    NUM_ALTERNATE_PROTOCOLS,
  };

  struct PortProtocolPair {
    bool Equals(const PortProtocolPair& other) const {
      return port == other.port && protocol == other.protocol;
    }

    uint16 port;
    Protocol protocol;
  };

  static const char kHeader[];
  static const char kSpdyProtocol[];

  HttpAlternateProtocols();
  ~HttpAlternateProtocols();

  // Reports whether or not we have received Alternate-Protocol for
  // |http_host_port_pair|.
  bool HasAlternateProtocolFor(const HostPortPair& http_host_port_pair) const;
  bool HasAlternateProtocolFor(const std::string& host, uint16 port) const;

  PortProtocolPair GetAlternateProtocolFor(
      const HostPortPair& http_host_port_pair) const;
  PortProtocolPair GetAlternateProtocolFor(
      const std::string& host, uint16 port) const;

  // SetAlternateProtocolFor() will ignore the request if the alternate protocol
  // has already been marked broken via MarkBrokenAlternateProtocolFor().
  void SetAlternateProtocolFor(const HostPortPair& http_host_port_pair,
                               uint16 alternate_port,
                               Protocol alternate_protocol);

  // Marks the alternate protocol as broken.  Once marked broken, any further
  // attempts to set the alternate protocol for |http_host_port_pair| will fail.
  void MarkBrokenAlternateProtocolFor(const HostPortPair& http_host_port_pair);

 private:
  typedef std::map<HostPortPair, PortProtocolPair> ProtocolMap;

  ProtocolMap protocol_map_;

  DISALLOW_COPY_AND_ASSIGN(HttpAlternateProtocols);
};

}  // namespace net

#endif  // NET_HTTP_HTTP_ALTERNATE_PROTOCOLS_H_
