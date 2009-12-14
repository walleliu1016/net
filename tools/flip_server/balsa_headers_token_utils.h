// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Utility class that performs basic operations on header value tokens: parsing
// them out, checking for presense of certain tokens, and removing them.

#ifndef NET_TOOLS_FLIP_SERVER_BALSA_HEADERS_TOKEN_UTILS_H_
#define NET_TOOLS_FLIP_SERVER_BALSA_HEADERS_TOKEN_UTILS_H_

#include "net/tools/flip_server/balsa_headers.h"
#include "strings/stringpiece.h"

namespace net {

class BalsaHeadersTokenUtils {
 public:
  // All the functions below respect multiple header lines with the same key.

  // Checks whether the last header token matches a given value. Useful to
  // check the outer-most content or transfer-encoding, for example. In the
  // presence of multiple header lines with given key, the last token of the
  // last line is compared.
  static bool CheckHeaderForLastToken(const BalsaHeaders& headers,
                                      const StringPiece& key,
                                      const StringPiece& token);

  // Tokenizes header value for a given key. In the presence of multiple lines
  // with that key, all of them will be tokenized and tokens will be added to
  // the list in the order in which they are encountered.
  static void TokenizeHeaderValue(const BalsaHeaders& headers,
                                  const StringPiece& key,
                                  BalsaHeaders::HeaderTokenList* tokens);

  // Removes the last token from the header value. In the presence of multiple
  // header lines with given key, will remove the last token of the last line.
  // Can be useful if the last encoding has to be removed.
  static void RemoveLastTokenFromHeaderValue(const StringPiece& key,
                                             BalsaHeaders* headers);

  // Given a pointer to the beginning and the end of the header value
  // in some buffer, populates tokens list with beginning and end indices
  // of all tokens present in the value string.
  static void ParseTokenList(const char* start,
                             const char* end,
                             BalsaHeaders::HeaderTokenList* tokens);

 private:
  // Helper function to tokenize a header line once we have its description.
  static void TokenizeHeaderLine(
      const BalsaHeaders& headers,
      const BalsaHeaders::HeaderLineDescription& line,
      BalsaHeaders::HeaderTokenList* tokens);

  BalsaHeadersTokenUtils();  // Prohibit instantiation
};

}  // namespace net

#endif  // NET_TOOLS_FLIP_SERVER_BALSA_HEADERS_TOKEN_UTILS_H_

