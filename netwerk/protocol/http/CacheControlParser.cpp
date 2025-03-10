/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 ts=8 et tw=80 : */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CacheControlParser.h"

namespace mozilla {
namespace net {

CacheControlParser::CacheControlParser(nsACString const& aHeader)
    : Tokenizer(aHeader, nullptr, "-_"),
      mMaxAgeSet(false),
      mMaxAge(0),
      mMaxStaleSet(false),
      mMaxStale(0),
      mMinFreshSet(false),
      mMinFresh(0),
      mStaleWhileRevalidateSet(false),
      mStaleWhileRevalidate(0),
      mNoCache(false),
      mNoStore(false),
      mPublic(false),
      mPrivate(false),
      mImmutable(false) {
  SkipWhites();
  if (!CheckEOF()) {
    Directive();
  }
}

void CacheControlParser::Directive() {
  nsAutoCString word;
  do {
    SkipWhites();
    if (!ReadWord(word)) {
      return;
    }

    ToLowerCase(word);
    if (word == "no-cache") {
      mNoCache = true;
      IgnoreDirective();  // ignore any optionally added values
    } else if (word == "no-store") {
      mNoStore = true;
    } else if (word == "max-age") {
      mMaxAgeSet = SecondsValue(&mMaxAge);
    } else if (word == "max-stale") {
      mMaxStaleSet = SecondsValue(&mMaxStale, PR_UINT32_MAX);
    } else if (word == "min-fresh") {
      mMinFreshSet = SecondsValue(&mMinFresh);
    } else if (word == "stale-while-revalidate") {
      mStaleWhileRevalidateSet = SecondsValue(&mStaleWhileRevalidate);
    } else if (word == "public") {
      mPublic = true;
    } else if (word == "private") {
      mPrivate = true;
    } else if (word == "immutable") {
      mImmutable = true;
    } else {
      IgnoreDirective();
    }

    SkipWhites();
    if (CheckEOF()) {
      return;
    }

  } while (CheckChar(','));

  NS_WARNING("Unexpected input in Cache-control header value");
}

bool CacheControlParser::SecondsValue(uint32_t* seconds, uint32_t defaultVal) {
  SkipWhites();
  if (!CheckChar('=')) {
    IgnoreDirective();
    *seconds = defaultVal;
    return !!defaultVal;
  }

  SkipWhites();
  if (!ReadInteger(seconds)) {
    NS_WARNING("Unexpected value in Cache-control header value");
    IgnoreDirective();
    *seconds = defaultVal;
    return !!defaultVal;
  }

  return true;
}

void CacheControlParser::IgnoreDirective() {
  Token t;
  while (Next(t)) {
    if (t.Equals(Token::Char(',')) || t.Equals(Token::EndOfFile())) {
      Rollback();
      break;
    }
    if (t.Equals(Token::Char('"'))) {
      SkipUntil(Token::Char('"'));
      if (!CheckChar('"')) {
        NS_WARNING(
            "Missing quoted string expansion in Cache-control header value");
        break;
      }
    }
  }
}

bool CacheControlParser::MaxAge(uint32_t* seconds) {
  *seconds = mMaxAge;
  return mMaxAgeSet;
}

bool CacheControlParser::MaxStale(uint32_t* seconds) {
  *seconds = mMaxStale;
  return mMaxStaleSet;
}

bool CacheControlParser::MinFresh(uint32_t* seconds) {
  *seconds = mMinFresh;
  return mMinFreshSet;
}

bool CacheControlParser::StaleWhileRevalidate(uint32_t* seconds) {
  *seconds = mStaleWhileRevalidate;
  return mStaleWhileRevalidateSet;
}

bool CacheControlParser::NoCache() { return mNoCache; }

bool CacheControlParser::NoStore() { return mNoStore; }

bool CacheControlParser::Public() { return mPublic; }

bool CacheControlParser::Private() { return mPrivate; }

bool CacheControlParser::Immutable() { return mImmutable; }

}  // namespace net
}  // namespace mozilla
