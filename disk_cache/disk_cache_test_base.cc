// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/disk_cache/disk_cache_test_base.h"

#include "base/file_util.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"
#include "net/base/test_completion_callback.h"
#include "net/disk_cache/backend_impl.h"
#include "net/disk_cache/cache_util.h"
#include "net/disk_cache/disk_cache.h"
#include "net/disk_cache/disk_cache_test_util.h"
#include "net/disk_cache/mem_backend_impl.h"
#include "net/disk_cache/simple/simple_backend_impl.h"

DiskCacheTest::DiskCacheTest() {
  CHECK(temp_dir_.CreateUniqueTempDir());
  cache_path_ = temp_dir_.path();
  if (!MessageLoop::current())
    message_loop_.reset(new MessageLoopForIO());
}

DiskCacheTest::~DiskCacheTest() {
}

bool DiskCacheTest::CopyTestCache(const std::string& name) {
  base::FilePath path;
  PathService::Get(base::DIR_SOURCE_ROOT, &path);
  path = path.AppendASCII("net");
  path = path.AppendASCII("data");
  path = path.AppendASCII("cache_tests");
  path = path.AppendASCII(name);

  if (!CleanupCacheDir())
    return false;
  return file_util::CopyDirectory(path, cache_path_, false);
}

bool DiskCacheTest::CleanupCacheDir() {
  return DeleteCache(cache_path_);
}

void DiskCacheTest::TearDown() {
  base::RunLoop().RunUntilIdle();
}

DiskCacheTestWithCache::DiskCacheTestWithCache()
    : cache_(NULL),
      cache_impl_(NULL),
      mem_cache_(NULL),
      mask_(0),
      size_(0),
      type_(net::DISK_CACHE),
      memory_only_(false),
      simple_cache_mode_(false),
      force_creation_(false),
      new_eviction_(false),
      first_cleanup_(true),
      integrity_(true),
      use_current_thread_(false),
      cache_thread_("CacheThread") {
}

DiskCacheTestWithCache::~DiskCacheTestWithCache() {}

void DiskCacheTestWithCache::InitCache() {
  if (memory_only_)
    InitMemoryCache();
  else
    InitDiskCache();

  ASSERT_TRUE(NULL != cache_);
  if (first_cleanup_)
    ASSERT_EQ(0, cache_->GetEntryCount());
}

// We are expected to leak memory when simulating crashes.
void DiskCacheTestWithCache::SimulateCrash() {
  ASSERT_TRUE(!memory_only_);
  net::TestCompletionCallback cb;
  int rv = cache_impl_->FlushQueueForTest(cb.callback());
  ASSERT_EQ(net::OK, cb.GetResult(rv));
  cache_impl_->ClearRefCountForTest();

  delete cache_impl_;
  EXPECT_TRUE(CheckCacheIntegrity(cache_path_, new_eviction_, mask_));

  CreateBackend(disk_cache::kNoRandom, &cache_thread_);
}

void DiskCacheTestWithCache::SetTestMode() {
  ASSERT_TRUE(!memory_only_);
  cache_impl_->SetUnitTestMode();
}

void DiskCacheTestWithCache::SetMaxSize(int size) {
  size_ = size;
  if (cache_impl_)
    EXPECT_TRUE(cache_impl_->SetMaxSize(size));

  if (mem_cache_)
    EXPECT_TRUE(mem_cache_->SetMaxSize(size));
}

int DiskCacheTestWithCache::OpenEntry(const std::string& key,
                                      disk_cache::Entry** entry) {
  net::TestCompletionCallback cb;
  int rv = cache_->OpenEntry(key, entry, cb.callback());
  return cb.GetResult(rv);
}

int DiskCacheTestWithCache::CreateEntry(const std::string& key,
                                        disk_cache::Entry** entry) {
  net::TestCompletionCallback cb;
  int rv = cache_->CreateEntry(key, entry, cb.callback());
  return cb.GetResult(rv);
}

int DiskCacheTestWithCache::DoomEntry(const std::string& key) {
  net::TestCompletionCallback cb;
  int rv = cache_->DoomEntry(key, cb.callback());
  return cb.GetResult(rv);
}

int DiskCacheTestWithCache::DoomAllEntries() {
  net::TestCompletionCallback cb;
  int rv = cache_->DoomAllEntries(cb.callback());
  return cb.GetResult(rv);
}

int DiskCacheTestWithCache::DoomEntriesBetween(const base::Time initial_time,
                                               const base::Time end_time) {
  net::TestCompletionCallback cb;
  int rv = cache_->DoomEntriesBetween(initial_time, end_time, cb.callback());
  return cb.GetResult(rv);
}

int DiskCacheTestWithCache::DoomEntriesSince(const base::Time initial_time) {
  net::TestCompletionCallback cb;
  int rv = cache_->DoomEntriesSince(initial_time, cb.callback());
  return cb.GetResult(rv);
}

int DiskCacheTestWithCache::OpenNextEntry(void** iter,
                                          disk_cache::Entry** next_entry) {
  net::TestCompletionCallback cb;
  int rv = cache_->OpenNextEntry(iter, next_entry, cb.callback());
  return cb.GetResult(rv);
}

void DiskCacheTestWithCache::FlushQueueForTest() {
  if (memory_only_ || !cache_impl_)
    return;

  net::TestCompletionCallback cb;
  int rv = cache_impl_->FlushQueueForTest(cb.callback());
  EXPECT_EQ(net::OK, cb.GetResult(rv));
}

void DiskCacheTestWithCache::RunTaskForTest(const base::Closure& closure) {
  if (memory_only_ || !cache_impl_) {
    closure.Run();
    return;
  }

  net::TestCompletionCallback cb;
  int rv = cache_impl_->RunTaskForTest(closure, cb.callback());
  EXPECT_EQ(net::OK, cb.GetResult(rv));
}

int DiskCacheTestWithCache::ReadData(disk_cache::Entry* entry, int index,
                                     int offset, net::IOBuffer* buf, int len) {
  net::TestCompletionCallback cb;
  int rv = entry->ReadData(index, offset, buf, len, cb.callback());
  return cb.GetResult(rv);
}

int DiskCacheTestWithCache::WriteData(disk_cache::Entry* entry, int index,
                                      int offset, net::IOBuffer* buf, int len,
                                      bool truncate) {
  net::TestCompletionCallback cb;
  int rv = entry->WriteData(index, offset, buf, len, cb.callback(), truncate);
  return cb.GetResult(rv);
}

int DiskCacheTestWithCache::ReadSparseData(disk_cache::Entry* entry,
                                           int64 offset, net::IOBuffer* buf,
                                           int len) {
  net::TestCompletionCallback cb;
  int rv = entry->ReadSparseData(offset, buf, len, cb.callback());
  return cb.GetResult(rv);
}

int DiskCacheTestWithCache::WriteSparseData(disk_cache::Entry* entry,
                                            int64 offset,
                                            net::IOBuffer* buf, int len) {
  net::TestCompletionCallback cb;
  int rv = entry->WriteSparseData(offset, buf, len, cb.callback());
  return cb.GetResult(rv);
}

void DiskCacheTestWithCache::TrimForTest(bool empty) {
  RunTaskForTest(base::Bind(&disk_cache::BackendImpl::TrimForTest,
                            base::Unretained(cache_impl_),
                            empty));
}

void DiskCacheTestWithCache::TrimDeletedListForTest(bool empty) {
  RunTaskForTest(base::Bind(&disk_cache::BackendImpl::TrimDeletedListForTest,
                            base::Unretained(cache_impl_),
                            empty));
}

void DiskCacheTestWithCache::AddDelay() {
  base::Time initial = base::Time::Now();
  while (base::Time::Now() <= initial) {
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(1));
  };
}

void DiskCacheTestWithCache::TearDown() {
  base::RunLoop().RunUntilIdle();
  delete cache_;
  if (cache_thread_.IsRunning())
    cache_thread_.Stop();

  if (!memory_only_ && integrity_) {
    EXPECT_TRUE(CheckCacheIntegrity(cache_path_, new_eviction_, mask_));
  }

  PlatformTest::TearDown();
}

void DiskCacheTestWithCache::InitMemoryCache() {
  mem_cache_ = new disk_cache::MemBackendImpl(NULL);
  cache_ = mem_cache_;
  ASSERT_TRUE(NULL != cache_);

  if (size_)
    EXPECT_TRUE(mem_cache_->SetMaxSize(size_));

  ASSERT_TRUE(mem_cache_->Init());
}

void DiskCacheTestWithCache::InitDiskCache() {
  if (first_cleanup_)
    ASSERT_TRUE(CleanupCacheDir());

  if (!cache_thread_.IsRunning()) {
    ASSERT_TRUE(cache_thread_.StartWithOptions(
                    base::Thread::Options(MessageLoop::TYPE_IO, 0)));
  }
  ASSERT_TRUE(cache_thread_.message_loop() != NULL);

  CreateBackend(disk_cache::kNoRandom, &cache_thread_);
}

// Testing backend creation retry logic is hard because the CacheCreator and
// cache backend(s) are tightly coupled. So we take the default backend often.
// Tests themselves need to be adjusted for platforms where the BackendImpl is
// not the default backend.
void DiskCacheTestWithCache::InitDefaultCacheViaCreator() {
  if (!cache_thread_.IsRunning()) {
    ASSERT_TRUE(cache_thread_.StartWithOptions(
                    base::Thread::Options(MessageLoop::TYPE_IO, 0)));
  }
  ASSERT_TRUE(cache_thread_.message_loop() != NULL);

  net::TestCompletionCallback cb;
  disk_cache::CacheCreator* creator = new disk_cache::CacheCreator(
      cache_path_, true, 0, net::DISK_CACHE, disk_cache::kNoRandom,
      cache_thread_.message_loop_proxy(), NULL, &cache_, cb.callback());
  int rv = creator->Run();
  ASSERT_EQ(net::OK, cb.GetResult(rv));
}

void DiskCacheTestWithCache::CreateBackend(uint32 flags, base::Thread* thread) {
  base::MessageLoopProxy* runner;
  if (use_current_thread_)
    runner = base::MessageLoopProxy::current();
  else
    runner = thread->message_loop_proxy();

  if (simple_cache_mode_) {
    net::TestCompletionCallback cb;
    disk_cache::Backend* simple_backend;
    // TODO(pasko): split Simple Backend construction from initialization.
    int rv = disk_cache::SimpleBackendImpl::CreateBackend(cache_path_, size_,
        type_, disk_cache::kNone, make_scoped_refptr(runner), NULL,
        &simple_backend, cb.callback());
    ASSERT_EQ(net::OK, cb.GetResult(rv));
    cache_ = simple_backend;
    return;
  }

  if (mask_)
    cache_impl_ = new disk_cache::BackendImpl(cache_path_, mask_, runner, NULL);
  else
    cache_impl_ = new disk_cache::BackendImpl(cache_path_, runner, NULL);
  cache_ = cache_impl_;
  ASSERT_TRUE(NULL != cache_);
  if (size_)
    EXPECT_TRUE(cache_impl_->SetMaxSize(size_));
  if (new_eviction_)
    cache_impl_->SetNewEviction();
  cache_impl_->SetType(type_);
  cache_impl_->SetFlags(flags);
  net::TestCompletionCallback cb;
  int rv = cache_impl_->Init(cb.callback());
  ASSERT_EQ(net::OK, cb.GetResult(rv));
  cache_ = cache_impl_;
}
