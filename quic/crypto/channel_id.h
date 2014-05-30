// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_QUIC_CRYPTO_CHANNEL_ID_H_
#define NET_QUIC_CRYPTO_CHANNEL_ID_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/strings/string_piece.h"
#include "net/base/net_export.h"

namespace net {

// ChannelIDKey is an interface that supports signing with and serializing a
// ChannelID key.
class NET_EXPORT_PRIVATE ChannelIDKey {
 public:
  virtual ~ChannelIDKey() { }

  // Sign signs |signed_data| using the ChannelID private key and puts the
  // signature into |out_signature|. It returns true on success.
  virtual bool Sign(base::StringPiece signed_data,
                    std::string* out_signature) = 0;

  // SerializeKey returns the serialized ChannelID public key.
  virtual std::string SerializeKey() = 0;
};

// ChannelIDSource is an abstract interface by which a QUIC client can obtain
// a ChannelIDKey for a given hostname.
class NET_EXPORT_PRIVATE ChannelIDSource {
 public:
  virtual ~ChannelIDSource() {}

  // GetChannelIDKey looks up the ChannelIDKey for |hostname|. On success it
  // returns true and stores the ChannelIDKey in |*channel_id|.
  virtual bool GetChannelIDKey(const std::string& hostname,
                               scoped_ptr<ChannelIDKey>* channel_id_key) = 0;
};

// ChannelIDVerifier verifies ChannelID signatures.
class NET_EXPORT_PRIVATE ChannelIDVerifier {
 public:
  // kContextStr is prepended to the data to be signed in order to ensure that
  // a ChannelID signature cannot be used in a different context. (The
  // terminating NUL byte is inclued.)
  static const char kContextStr[];
  // kClientToServerStr follows kContextStr to specify that the ChannelID is
  // being used in the client to server direction. (The terminating NUL byte is
  // included.)
  static const char kClientToServerStr[];

  // Verify returns true iff |signature| is a valid signature of |signed_data|
  // by |key|.
  static bool Verify(base::StringPiece key,
                     base::StringPiece signed_data,
                     base::StringPiece signature);

  // FOR TESTING ONLY: VerifyRaw returns true iff |signature| is a valid
  // signature of |signed_data| by |key|. |is_channel_id_signature| indicates
  // whether |signature| is a ChannelID signature (with kContextStr prepended
  // to the data to be signed).
  static bool VerifyRaw(base::StringPiece key,
                        base::StringPiece signed_data,
                        base::StringPiece signature,
                        bool is_channel_id_signature);

 private:
  DISALLOW_COPY_AND_ASSIGN(ChannelIDVerifier);
};

}  // namespace net

#endif  // NET_QUIC_CRYPTO_CHANNEL_ID_H_
