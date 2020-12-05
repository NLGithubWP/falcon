// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: job.proto

#ifndef PROTOBUF_job_2eproto__INCLUDED
#define PROTOBUF_job_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/struct.pb.h>
// @@protoc_insertion_point(includes)

namespace com {
namespace nus {
namespace dbsystem {
namespace falcon {
namespace v0 {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_job_2eproto();
void protobuf_AssignDesc_job_2eproto();
void protobuf_ShutdownFile_job_2eproto();

class ForwardJobRequest;
class ForwardJobResponse;
class StopJobRequest;
class StopJobResponse;

// ===================================================================

class ForwardJobRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:com.nus.dbsystem.falcon.v0.ForwardJobRequest) */ {
 public:
  ForwardJobRequest();
  virtual ~ForwardJobRequest();

  ForwardJobRequest(const ForwardJobRequest& from);

  inline ForwardJobRequest& operator=(const ForwardJobRequest& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ForwardJobRequest& default_instance();

  void Swap(ForwardJobRequest* other);

  // implements Message ----------------------------------------------

  inline ForwardJobRequest* New() const { return New(NULL); }

  ForwardJobRequest* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ForwardJobRequest& from);
  void MergeFrom(const ForwardJobRequest& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ForwardJobRequest* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 job_id = 1;
  void clear_job_id();
  static const int kJobIdFieldNumber = 1;
  ::google::protobuf::int32 job_id() const;
  void set_job_id(::google::protobuf::int32 value);

  // optional string job_name = 2;
  void clear_job_name();
  static const int kJobNameFieldNumber = 2;
  const ::std::string& job_name() const;
  void set_job_name(const ::std::string& value);
  void set_job_name(const char* value);
  void set_job_name(const char* value, size_t size);
  ::std::string* mutable_job_name();
  ::std::string* release_job_name();
  void set_allocated_job_name(::std::string* job_name);

  // optional int32 task_num = 3;
  void clear_task_num();
  static const int kTaskNumFieldNumber = 3;
  ::google::protobuf::int32 task_num() const;
  void set_task_num(::google::protobuf::int32 value);

  // optional int32 party_num = 4;
  void clear_party_num();
  static const int kPartyNumFieldNumber = 4;
  ::google::protobuf::int32 party_num() const;
  void set_party_num(::google::protobuf::int32 value);

  // repeated int32 party_ids = 5;
  int party_ids_size() const;
  void clear_party_ids();
  static const int kPartyIdsFieldNumber = 5;
  ::google::protobuf::int32 party_ids(int index) const;
  void set_party_ids(int index, ::google::protobuf::int32 value);
  void add_party_ids(::google::protobuf::int32 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
      party_ids() const;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
      mutable_party_ids();

  // optional int32 fl_setting = 6;
  void clear_fl_setting();
  static const int kFlSettingFieldNumber = 6;
  ::google::protobuf::int32 fl_setting() const;
  void set_fl_setting(::google::protobuf::int32 value);

  // optional .google.protobuf.Value job_desc = 7;
  bool has_job_desc() const;
  void clear_job_desc();
  static const int kJobDescFieldNumber = 7;
  const ::google::protobuf::Value& job_desc() const;
  ::google::protobuf::Value* mutable_job_desc();
  ::google::protobuf::Value* release_job_desc();
  void set_allocated_job_desc(::google::protobuf::Value* job_desc);

  // @@protoc_insertion_point(class_scope:com.nus.dbsystem.falcon.v0.ForwardJobRequest)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr job_name_;
  ::google::protobuf::int32 job_id_;
  ::google::protobuf::int32 task_num_;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 > party_ids_;
  mutable int _party_ids_cached_byte_size_;
  ::google::protobuf::int32 party_num_;
  ::google::protobuf::int32 fl_setting_;
  ::google::protobuf::Value* job_desc_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_job_2eproto();
  friend void protobuf_AssignDesc_job_2eproto();
  friend void protobuf_ShutdownFile_job_2eproto();

  void InitAsDefaultInstance();
  static ForwardJobRequest* default_instance_;
};
// -------------------------------------------------------------------

class ForwardJobResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:com.nus.dbsystem.falcon.v0.ForwardJobResponse) */ {
 public:
  ForwardJobResponse();
  virtual ~ForwardJobResponse();

  ForwardJobResponse(const ForwardJobResponse& from);

  inline ForwardJobResponse& operator=(const ForwardJobResponse& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ForwardJobResponse& default_instance();

  void Swap(ForwardJobResponse* other);

  // implements Message ----------------------------------------------

  inline ForwardJobResponse* New() const { return New(NULL); }

  ForwardJobResponse* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ForwardJobResponse& from);
  void MergeFrom(const ForwardJobResponse& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ForwardJobResponse* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 job_id = 1;
  void clear_job_id();
  static const int kJobIdFieldNumber = 1;
  ::google::protobuf::int32 job_id() const;
  void set_job_id(::google::protobuf::int32 value);

  // optional int32 is_success = 2;
  void clear_is_success();
  static const int kIsSuccessFieldNumber = 2;
  ::google::protobuf::int32 is_success() const;
  void set_is_success(::google::protobuf::int32 value);

  // optional int32 error_code = 3;
  void clear_error_code();
  static const int kErrorCodeFieldNumber = 3;
  ::google::protobuf::int32 error_code() const;
  void set_error_code(::google::protobuf::int32 value);

  // optional string error_msg = 4;
  void clear_error_msg();
  static const int kErrorMsgFieldNumber = 4;
  const ::std::string& error_msg() const;
  void set_error_msg(const ::std::string& value);
  void set_error_msg(const char* value);
  void set_error_msg(const char* value, size_t size);
  ::std::string* mutable_error_msg();
  ::std::string* release_error_msg();
  void set_allocated_error_msg(::std::string* error_msg);

  // @@protoc_insertion_point(class_scope:com.nus.dbsystem.falcon.v0.ForwardJobResponse)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::int32 job_id_;
  ::google::protobuf::int32 is_success_;
  ::google::protobuf::internal::ArenaStringPtr error_msg_;
  ::google::protobuf::int32 error_code_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_job_2eproto();
  friend void protobuf_AssignDesc_job_2eproto();
  friend void protobuf_ShutdownFile_job_2eproto();

  void InitAsDefaultInstance();
  static ForwardJobResponse* default_instance_;
};
// -------------------------------------------------------------------

class StopJobRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:com.nus.dbsystem.falcon.v0.StopJobRequest) */ {
 public:
  StopJobRequest();
  virtual ~StopJobRequest();

  StopJobRequest(const StopJobRequest& from);

  inline StopJobRequest& operator=(const StopJobRequest& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const StopJobRequest& default_instance();

  void Swap(StopJobRequest* other);

  // implements Message ----------------------------------------------

  inline StopJobRequest* New() const { return New(NULL); }

  StopJobRequest* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const StopJobRequest& from);
  void MergeFrom(const StopJobRequest& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(StopJobRequest* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 job_id = 1;
  void clear_job_id();
  static const int kJobIdFieldNumber = 1;
  ::google::protobuf::int32 job_id() const;
  void set_job_id(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:com.nus.dbsystem.falcon.v0.StopJobRequest)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::int32 job_id_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_job_2eproto();
  friend void protobuf_AssignDesc_job_2eproto();
  friend void protobuf_ShutdownFile_job_2eproto();

  void InitAsDefaultInstance();
  static StopJobRequest* default_instance_;
};
// -------------------------------------------------------------------

class StopJobResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:com.nus.dbsystem.falcon.v0.StopJobResponse) */ {
 public:
  StopJobResponse();
  virtual ~StopJobResponse();

  StopJobResponse(const StopJobResponse& from);

  inline StopJobResponse& operator=(const StopJobResponse& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const StopJobResponse& default_instance();

  void Swap(StopJobResponse* other);

  // implements Message ----------------------------------------------

  inline StopJobResponse* New() const { return New(NULL); }

  StopJobResponse* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const StopJobResponse& from);
  void MergeFrom(const StopJobResponse& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(StopJobResponse* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 job_id = 1;
  void clear_job_id();
  static const int kJobIdFieldNumber = 1;
  ::google::protobuf::int32 job_id() const;
  void set_job_id(::google::protobuf::int32 value);

  // optional int32 is_success = 2;
  void clear_is_success();
  static const int kIsSuccessFieldNumber = 2;
  ::google::protobuf::int32 is_success() const;
  void set_is_success(::google::protobuf::int32 value);

  // optional int32 error_code = 3;
  void clear_error_code();
  static const int kErrorCodeFieldNumber = 3;
  ::google::protobuf::int32 error_code() const;
  void set_error_code(::google::protobuf::int32 value);

  // optional string error_msg = 4;
  void clear_error_msg();
  static const int kErrorMsgFieldNumber = 4;
  const ::std::string& error_msg() const;
  void set_error_msg(const ::std::string& value);
  void set_error_msg(const char* value);
  void set_error_msg(const char* value, size_t size);
  ::std::string* mutable_error_msg();
  ::std::string* release_error_msg();
  void set_allocated_error_msg(::std::string* error_msg);

  // @@protoc_insertion_point(class_scope:com.nus.dbsystem.falcon.v0.StopJobResponse)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::int32 job_id_;
  ::google::protobuf::int32 is_success_;
  ::google::protobuf::internal::ArenaStringPtr error_msg_;
  ::google::protobuf::int32 error_code_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_job_2eproto();
  friend void protobuf_AssignDesc_job_2eproto();
  friend void protobuf_ShutdownFile_job_2eproto();

  void InitAsDefaultInstance();
  static StopJobResponse* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// ForwardJobRequest

// optional int32 job_id = 1;
inline void ForwardJobRequest::clear_job_id() {
  job_id_ = 0;
}
inline ::google::protobuf::int32 ForwardJobRequest::job_id() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_id)
  return job_id_;
}
inline void ForwardJobRequest::set_job_id(::google::protobuf::int32 value) {
  
  job_id_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_id)
}

// optional string job_name = 2;
inline void ForwardJobRequest::clear_job_name() {
  job_name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& ForwardJobRequest::job_name() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_name)
  return job_name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ForwardJobRequest::set_job_name(const ::std::string& value) {
  
  job_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_name)
}
inline void ForwardJobRequest::set_job_name(const char* value) {
  
  job_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_name)
}
inline void ForwardJobRequest::set_job_name(const char* value, size_t size) {
  
  job_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_name)
}
inline ::std::string* ForwardJobRequest::mutable_job_name() {
  
  // @@protoc_insertion_point(field_mutable:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_name)
  return job_name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ForwardJobRequest::release_job_name() {
  // @@protoc_insertion_point(field_release:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_name)
  
  return job_name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ForwardJobRequest::set_allocated_job_name(::std::string* job_name) {
  if (job_name != NULL) {
    
  } else {
    
  }
  job_name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), job_name);
  // @@protoc_insertion_point(field_set_allocated:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_name)
}

// optional int32 task_num = 3;
inline void ForwardJobRequest::clear_task_num() {
  task_num_ = 0;
}
inline ::google::protobuf::int32 ForwardJobRequest::task_num() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobRequest.task_num)
  return task_num_;
}
inline void ForwardJobRequest::set_task_num(::google::protobuf::int32 value) {
  
  task_num_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobRequest.task_num)
}

// optional int32 party_num = 4;
inline void ForwardJobRequest::clear_party_num() {
  party_num_ = 0;
}
inline ::google::protobuf::int32 ForwardJobRequest::party_num() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobRequest.party_num)
  return party_num_;
}
inline void ForwardJobRequest::set_party_num(::google::protobuf::int32 value) {
  
  party_num_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobRequest.party_num)
}

// repeated int32 party_ids = 5;
inline int ForwardJobRequest::party_ids_size() const {
  return party_ids_.size();
}
inline void ForwardJobRequest::clear_party_ids() {
  party_ids_.Clear();
}
inline ::google::protobuf::int32 ForwardJobRequest::party_ids(int index) const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobRequest.party_ids)
  return party_ids_.Get(index);
}
inline void ForwardJobRequest::set_party_ids(int index, ::google::protobuf::int32 value) {
  party_ids_.Set(index, value);
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobRequest.party_ids)
}
inline void ForwardJobRequest::add_party_ids(::google::protobuf::int32 value) {
  party_ids_.Add(value);
  // @@protoc_insertion_point(field_add:com.nus.dbsystem.falcon.v0.ForwardJobRequest.party_ids)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
ForwardJobRequest::party_ids() const {
  // @@protoc_insertion_point(field_list:com.nus.dbsystem.falcon.v0.ForwardJobRequest.party_ids)
  return party_ids_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
ForwardJobRequest::mutable_party_ids() {
  // @@protoc_insertion_point(field_mutable_list:com.nus.dbsystem.falcon.v0.ForwardJobRequest.party_ids)
  return &party_ids_;
}

// optional int32 fl_setting = 6;
inline void ForwardJobRequest::clear_fl_setting() {
  fl_setting_ = 0;
}
inline ::google::protobuf::int32 ForwardJobRequest::fl_setting() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobRequest.fl_setting)
  return fl_setting_;
}
inline void ForwardJobRequest::set_fl_setting(::google::protobuf::int32 value) {
  
  fl_setting_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobRequest.fl_setting)
}

// optional .google.protobuf.Value job_desc = 7;
inline bool ForwardJobRequest::has_job_desc() const {
  return !_is_default_instance_ && job_desc_ != NULL;
}
inline void ForwardJobRequest::clear_job_desc() {
  if (GetArenaNoVirtual() == NULL && job_desc_ != NULL) delete job_desc_;
  job_desc_ = NULL;
}
inline const ::google::protobuf::Value& ForwardJobRequest::job_desc() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_desc)
  return job_desc_ != NULL ? *job_desc_ : *default_instance_->job_desc_;
}
inline ::google::protobuf::Value* ForwardJobRequest::mutable_job_desc() {
  
  if (job_desc_ == NULL) {
    job_desc_ = new ::google::protobuf::Value;
  }
  // @@protoc_insertion_point(field_mutable:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_desc)
  return job_desc_;
}
inline ::google::protobuf::Value* ForwardJobRequest::release_job_desc() {
  // @@protoc_insertion_point(field_release:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_desc)
  
  ::google::protobuf::Value* temp = job_desc_;
  job_desc_ = NULL;
  return temp;
}
inline void ForwardJobRequest::set_allocated_job_desc(::google::protobuf::Value* job_desc) {
  delete job_desc_;
  job_desc_ = job_desc;
  if (job_desc) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:com.nus.dbsystem.falcon.v0.ForwardJobRequest.job_desc)
}

// -------------------------------------------------------------------

// ForwardJobResponse

// optional int32 job_id = 1;
inline void ForwardJobResponse::clear_job_id() {
  job_id_ = 0;
}
inline ::google::protobuf::int32 ForwardJobResponse::job_id() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobResponse.job_id)
  return job_id_;
}
inline void ForwardJobResponse::set_job_id(::google::protobuf::int32 value) {
  
  job_id_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobResponse.job_id)
}

// optional int32 is_success = 2;
inline void ForwardJobResponse::clear_is_success() {
  is_success_ = 0;
}
inline ::google::protobuf::int32 ForwardJobResponse::is_success() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobResponse.is_success)
  return is_success_;
}
inline void ForwardJobResponse::set_is_success(::google::protobuf::int32 value) {
  
  is_success_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobResponse.is_success)
}

// optional int32 error_code = 3;
inline void ForwardJobResponse::clear_error_code() {
  error_code_ = 0;
}
inline ::google::protobuf::int32 ForwardJobResponse::error_code() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobResponse.error_code)
  return error_code_;
}
inline void ForwardJobResponse::set_error_code(::google::protobuf::int32 value) {
  
  error_code_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobResponse.error_code)
}

// optional string error_msg = 4;
inline void ForwardJobResponse::clear_error_msg() {
  error_msg_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& ForwardJobResponse::error_msg() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.ForwardJobResponse.error_msg)
  return error_msg_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ForwardJobResponse::set_error_msg(const ::std::string& value) {
  
  error_msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.ForwardJobResponse.error_msg)
}
inline void ForwardJobResponse::set_error_msg(const char* value) {
  
  error_msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:com.nus.dbsystem.falcon.v0.ForwardJobResponse.error_msg)
}
inline void ForwardJobResponse::set_error_msg(const char* value, size_t size) {
  
  error_msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:com.nus.dbsystem.falcon.v0.ForwardJobResponse.error_msg)
}
inline ::std::string* ForwardJobResponse::mutable_error_msg() {
  
  // @@protoc_insertion_point(field_mutable:com.nus.dbsystem.falcon.v0.ForwardJobResponse.error_msg)
  return error_msg_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ForwardJobResponse::release_error_msg() {
  // @@protoc_insertion_point(field_release:com.nus.dbsystem.falcon.v0.ForwardJobResponse.error_msg)
  
  return error_msg_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ForwardJobResponse::set_allocated_error_msg(::std::string* error_msg) {
  if (error_msg != NULL) {
    
  } else {
    
  }
  error_msg_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), error_msg);
  // @@protoc_insertion_point(field_set_allocated:com.nus.dbsystem.falcon.v0.ForwardJobResponse.error_msg)
}

// -------------------------------------------------------------------

// StopJobRequest

// optional int32 job_id = 1;
inline void StopJobRequest::clear_job_id() {
  job_id_ = 0;
}
inline ::google::protobuf::int32 StopJobRequest::job_id() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.StopJobRequest.job_id)
  return job_id_;
}
inline void StopJobRequest::set_job_id(::google::protobuf::int32 value) {
  
  job_id_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.StopJobRequest.job_id)
}

// -------------------------------------------------------------------

// StopJobResponse

// optional int32 job_id = 1;
inline void StopJobResponse::clear_job_id() {
  job_id_ = 0;
}
inline ::google::protobuf::int32 StopJobResponse::job_id() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.StopJobResponse.job_id)
  return job_id_;
}
inline void StopJobResponse::set_job_id(::google::protobuf::int32 value) {
  
  job_id_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.StopJobResponse.job_id)
}

// optional int32 is_success = 2;
inline void StopJobResponse::clear_is_success() {
  is_success_ = 0;
}
inline ::google::protobuf::int32 StopJobResponse::is_success() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.StopJobResponse.is_success)
  return is_success_;
}
inline void StopJobResponse::set_is_success(::google::protobuf::int32 value) {
  
  is_success_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.StopJobResponse.is_success)
}

// optional int32 error_code = 3;
inline void StopJobResponse::clear_error_code() {
  error_code_ = 0;
}
inline ::google::protobuf::int32 StopJobResponse::error_code() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.StopJobResponse.error_code)
  return error_code_;
}
inline void StopJobResponse::set_error_code(::google::protobuf::int32 value) {
  
  error_code_ = value;
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.StopJobResponse.error_code)
}

// optional string error_msg = 4;
inline void StopJobResponse::clear_error_msg() {
  error_msg_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& StopJobResponse::error_msg() const {
  // @@protoc_insertion_point(field_get:com.nus.dbsystem.falcon.v0.StopJobResponse.error_msg)
  return error_msg_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void StopJobResponse::set_error_msg(const ::std::string& value) {
  
  error_msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:com.nus.dbsystem.falcon.v0.StopJobResponse.error_msg)
}
inline void StopJobResponse::set_error_msg(const char* value) {
  
  error_msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:com.nus.dbsystem.falcon.v0.StopJobResponse.error_msg)
}
inline void StopJobResponse::set_error_msg(const char* value, size_t size) {
  
  error_msg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:com.nus.dbsystem.falcon.v0.StopJobResponse.error_msg)
}
inline ::std::string* StopJobResponse::mutable_error_msg() {
  
  // @@protoc_insertion_point(field_mutable:com.nus.dbsystem.falcon.v0.StopJobResponse.error_msg)
  return error_msg_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* StopJobResponse::release_error_msg() {
  // @@protoc_insertion_point(field_release:com.nus.dbsystem.falcon.v0.StopJobResponse.error_msg)
  
  return error_msg_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void StopJobResponse::set_allocated_error_msg(::std::string* error_msg) {
  if (error_msg != NULL) {
    
  } else {
    
  }
  error_msg_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), error_msg);
  // @@protoc_insertion_point(field_set_allocated:com.nus.dbsystem.falcon.v0.StopJobResponse.error_msg)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace v0
}  // namespace falcon
}  // namespace dbsystem
}  // namespace nus
}  // namespace com

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_job_2eproto__INCLUDED