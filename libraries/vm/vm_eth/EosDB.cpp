#include "EosDB.h"

Status EosDB::Open(const Options& options,
                     const std::string& name,
                     DB** dbptr) {
	  return Status::OK();
}

EosDB::EosDB() : DB() {

}

EosDB::~EosDB(){

}

Status EosDB::Put(const WriteOptions& options,
                     const Slice& key,
                     const Slice& value) {
	  return Status::OK();
  }

Status EosDB::Delete(const WriteOptions& options, const Slice& key) {
	return Status::OK();
}

Status EosDB::Write(const WriteOptions& options, WriteBatch* updates) {
	return Status::OK();
}

Status EosDB::Get(const ReadOptions& options,
                     const Slice& key, std::string* value) {
	return Status::OK();
}

Iterator* EosDB::NewIterator(const ReadOptions& options) {
	return nullptr;
}

const Snapshot* EosDB::GetSnapshot() {
	return nullptr;
}

void EosDB::ReleaseSnapshot(const Snapshot* snapshot) {

}

bool EosDB::GetProperty(const Slice& property, std::string* value) {
	return false;
}

void EosDB::GetApproximateSizes(const Range* range, int n,
                                   uint64_t* sizes) {

}

void EosDB::CompactRange(const Slice* begin, const Slice* end) {

}
