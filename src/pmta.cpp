/* Copyright (C) 2015  Dan Nielsen <dnielsen@reachmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <node.h>

#include "pmta.h"

using namespace v8;
using namespace pmta::submitter;

Persistent<FunctionTemplate> PMTAConnection::constructor;

void PMTAConnection::Init(Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  Local<String> name = String::NewSymbol("PMTAConnection");

  constructor = Persistent<FunctionTemplate>::New(tpl);
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(name);

  NODE_SET_PROTOTYPE_METHOD(constructor, "submit", Submit);

  target->Set(name, constructor->GetFunction());
}

PMTAConnection::PMTAConnection(const char* host, int port, 
    const char* name, const char* password)
  : ObjectWrap(),
    host_(host), port_(port), name_(name), password_(password)
    { connection_ = new pmta::submitter::Connection(host_, port_, name_, 
        password_); }

PMTAConnection::~PMTAConnection() { delete connection_; }

Handle<Value> PMTAConnection::New(const Arguments& args) {
  HandleScope scope;

  if (!args.IsConstructCall()) {
    return ThrowException(Exception::TypeError(
        String::New("Use the new operator to create instances of this object"))
    );
  }

  if (args.Length() < 2) {
    return ThrowException(Exception::TypeError(
        String::New("Connection(host, port, [name], [password])")));
  }

  v8::String::Utf8Value param1(args[0]->ToString());
  const char* host = strdup(*param1);
  int port = args[1]->ToInteger()->Value();

  const char* name;
  const char* password;

  if (args[2]->IsUndefined()) {
    name = "";
  } else {
    v8::String::Utf8Value param3(args[2]->ToString());
    name = strdup(*param3);
  }

  if (args[3]->IsUndefined()) {
    password = "";
  } else {
    v8::String::Utf8Value param4(args[3]->ToString());
    password = strdup(*param4);
  }

  PMTAConnection* obj = new PMTAConnection(host, port, name, password);
  obj->Wrap(args.This());

  return args.This();
}


Handle<Value> PMTAConnection::Submit (const Arguments& args) {
  HandleScope scope;

  PMTAConnection* obj = ObjectWrap::Unwrap<PMTAConnection>(args.This());
  PMTAMessage* mobj = ObjectWrap::Unwrap<PMTAMessage>(args[0]->ToObject());

  Local<Object> ret = Object::New();

  try {
    obj->connection_->submit(*mobj->message_);
    ret->Set(String::NewSymbol("submitted"), True());
  } catch (std::exception& e) {
    delete obj;
    ret->Set(String::NewSymbol("submitted"), False());
    ret->Set(String::NewSymbol("errorMessage"), String::New(e.what()));
  }

  return scope.Close(ret);
}

Persistent<FunctionTemplate> PMTAMessage::constructor;

void PMTAMessage::Init(Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  Local<String> name = String::NewSymbol("PMTAMessage");

  constructor = Persistent<FunctionTemplate>::New(tpl);
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(name);

  NODE_SET_PROTOTYPE_METHOD(constructor, "sender", Sender);
  NODE_SET_PROTOTYPE_METHOD(constructor, "addData", AddData);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setVerp", SetVerp);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setJobId", SetJobId);
  NODE_SET_PROTOTYPE_METHOD(constructor, "beginPart", BeginPart);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setEncoding", SetEncoding);
  NODE_SET_PROTOTYPE_METHOD(constructor, "addRecipient", AddRecipient);
  NODE_SET_PROTOTYPE_METHOD(constructor, "addMergeData", AddMergeData);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setReturnType", SetReturnType);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setEnvelopeId", SetEnvelopeId);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setVirtualMta", SetVirtualMta);
  NODE_SET_PROTOTYPE_METHOD(constructor, "addDateHeader", AddDateHeader);

  target->Set(name, constructor->GetFunction());
}

PMTAMessage::PMTAMessage(const char* sender)
  : ObjectWrap(),
    sender_(sender) { message_ = new pmta::submitter::Message(sender); }

PMTAMessage::~PMTAMessage() { delete message_; }

Handle<Value> PMTAMessage::New(const Arguments& args) {
  HandleScope scope;

  if (!args.IsConstructCall()) {
    return ThrowException(Exception::TypeError(
        String::New("Use the new operator to create instances of this object"))
    );
  }

  if (!args[0]->IsString()) {
    return ThrowException(Exception::TypeError(
        String::New("First argument must be a string")));
  }

  v8::String::Utf8Value param1(args[0]->ToString());
  const char* sender = strdup(*param1);

  try {
    PMTAMessage* obj = new PMTAMessage(sender);
    obj->Wrap(args.This());
  } catch (std::exception& e) {
    return ThrowException(Exception::TypeError(String::New(e.what())));
  }

  return args.This();
}

Handle<Value> PMTAMessage::Sender(const Arguments& args) {
  HandleScope scope;

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  return scope.Close(String::New(obj->sender_));
}

Handle<Value> PMTAMessage::SetVerp(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsBoolean()) {
    return ThrowException(Exception::TypeError(
        String::New("setVerp() argument 1 should be bool")));
  }

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  Local<Boolean> param1(args[0]->ToBoolean());
  bool verp = param1->BooleanValue();

  obj->message_->setVerp(verp);

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::SetEncoding(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsString()) {
    return ThrowException(Exception::TypeError(
        String::New("setEncoding: arg 1 requires PmtaMsgENCODING")));
  }

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  v8::String::Utf8Value param1(args[0]->ToString());
  char* cmp = strdup(*param1);

  PmtaMsgENCODING enc;

  if (strcmp(cmp, "ENCODING_7BIT") == 0) {
    enc = PmtaMsgENCODING_7BIT;
  } else if (strcmp(cmp, "ENCODING_8BIT") == 0) {
    enc = PmtaMsgENCODING_8BIT;
  } else if (strcmp(cmp, "ENCODING_BASE64") == 0) {
    enc = PmtaMsgENCODING_BASE64;
  } else {
    enc = PmtaMsgENCODING_7BIT;
  }

  obj->message_->setEncoding(enc);

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::SetJobId (const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsString()) {
    return ThrowException(Exception::TypeError(
        String::New("setJobId: arg 1 requires a string")));
  }

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  v8::String::Utf8Value param1(args[0]->ToString());
  const char* jobid = strdup(*param1);

  obj->message_->setJobId(jobid);

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::SetReturnType(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsString()) {
    return ThrowException(Exception::TypeError(
        String::New("setReturnType: arg 1 requires string")));
  }

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  v8::String::Utf8Value param1(args[0]->ToString());
  char* cmp = strdup(*param1);

  PmtaMsgRETURN ret;

  if (strcmp(cmp,"RETURN_FULL") == 0) {
    ret = PmtaMsgRETURN_FULL;
  } else if (strcmp(cmp,"RETURN_HEADERS") == 0) {
    ret = PmtaMsgRETURN_HEADERS;
  } else {
    ret = PmtaMsgRETURN_HEADERS;
  } 

  obj->message_->setReturnType(ret);

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::SetEnvelopeId(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsString()) {
    return ThrowException(Exception::TypeError(
        String::New("setEnvelopeId: string required for arg1")));
  }

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  v8::String::Utf8Value param1(args[0]->ToString());
  const char* eid = strdup(*param1);

  obj->message_->setEnvelopeId(eid);

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::SetVirtualMta(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsString()) {
    return ThrowException(Exception::TypeError(
        String::New("setVirtualMta: string required for arg 1")));
  }

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  v8::String::Utf8Value param1(args[0]->ToString());
  const char* vmta = strdup(*param1);

  obj->message_->setVirtualMta(vmta);

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::BeginPart(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsInt32()) {
    return ThrowException(Exception::TypeError(
        String::New("beginPart: integer required for arg 1")));
  }

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  int part = args[0]->ToInteger()->Value();

  if (part <= 1) {
    return ThrowException(Exception::TypeError(
        String::New("beginPart: arg 1 must be greater than 1")));
  }

  obj->message_->beginPart(part);

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::AddData (const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2) {
    return ThrowException(Exception::TypeError(
        String::New("addData(data, length): insufficient arguments")));
  }

  if (!args[0]->IsString()) {
    return ThrowException(Exception::TypeError(
        String::New("addData(data, length): arg 1 must be a string")));
  }

  if (!args[1]->IsInt32()) {
    return ThrowException(Exception::TypeError(
        String::New("addData(data, length): arg 2 must be an integer")));
  }

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  v8::String::Utf8Value param1(args[0]->ToString());
  const char* data = strdup(*param1);
  int length = args[1]->ToInteger()->Value();

  obj->message_->addData(data, length);

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::AddMergeData (const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2) {
    return ThrowException(Exception::TypeError(
        String::New("addData(data, length): insufficient arguments")));
  }

  if (!args[0]->IsString()) {
    return ThrowException(Exception::TypeError(
        String::New("addData(data, length): arg 1 must be a string")));
  }

  if (!args[1]->IsInt32()) {
    return ThrowException(Exception::TypeError(
        String::New("addData(data, length): arg 2 must be an integer")));
  }

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  v8::String::Utf8Value param1(args[0]->ToString());
  const char* data = strdup(*param1);
  int length = args[1]->ToInteger()->Value();

  obj->message_->addMergeData(data, length);

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::AddDateHeader (const Arguments& args) {
  HandleScope scope;

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  obj->message_->addDateHeader();

  return scope.Close(Undefined());
}

Handle<Value> PMTAMessage::AddRecipient (const Arguments& args) {
  HandleScope scope;

  PMTAMessage* obj = ObjectWrap::Unwrap<PMTAMessage>(args.This());
  PMTARecipient* robj = ObjectWrap::Unwrap<PMTARecipient>(args[0]->ToObject());

  obj->message_->addRecipient(*robj->recipient_);

  return scope.Close(Undefined());
}

Persistent<FunctionTemplate> PMTARecipient::constructor;

void PMTARecipient::Init(Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  Local<String> name = String::NewSymbol("PMTARecipient");

  constructor = Persistent<FunctionTemplate>::New(tpl);
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(name);

  NODE_SET_PROTOTYPE_METHOD(constructor, "address", Address);
  NODE_SET_PROTOTYPE_METHOD(constructor, "setNotify", SetNotify);
  NODE_SET_PROTOTYPE_METHOD(constructor, "defineVariable", DefineVariable);

  target->Set(name, constructor->GetFunction());
}

PMTARecipient::PMTARecipient(const char* address)
  : ObjectWrap(),
    address_(address) { recipient_ = new Recipient(address); }

PMTARecipient::~PMTARecipient() { delete recipient_; }

Handle<Value> PMTARecipient::New(const Arguments& args) {
  HandleScope scope;

  if (!args.IsConstructCall()) {
    return ThrowException(Exception::TypeError(
        String::New("Use the new operator to create instances of this object"))
    );
  }

  v8::String::Utf8Value param1(args[0]->ToString());
  const char* addr = strdup(*param1);

  try {
    PMTARecipient* obj = new PMTARecipient(addr);
    obj->recipient_->defineVariable("*parts", "1");
    obj->Wrap(args.This());
  } catch (std::exception& e) {
    return ThrowException(Exception::TypeError(String::New(e.what())));
  }

  return args.This();
}

Handle<Value> PMTARecipient::DefineVariable(const Arguments& args) {
  HandleScope scope;

  PMTARecipient* obj = ObjectWrap::Unwrap<PMTARecipient>(args.This());
  v8::String::Utf8Value p0(args[0]->ToString());
  v8::String::Utf8Value p1(args[1]->ToString());
  char* name = strdup(*p0);
  char* value = strdup(*p1);

  obj->recipient_->defineVariable(name, value);

  return scope.Close(Undefined());
}

Handle<Value> PMTARecipient::SetNotify(const Arguments& args) {
  HandleScope scope;

  PMTARecipient* obj = ObjectWrap::Unwrap<PMTARecipient>(args.This());
  int notifywhen = args[0]->ToInteger()->Value();

  obj->recipient_->setNotify(notifywhen);

  return scope.Close(Undefined());
}


Handle<Value> PMTARecipient::Address(const Arguments& args) {
  HandleScope scope;

  PMTARecipient* obj = ObjectWrap::Unwrap<PMTARecipient>(args.This());
  return scope.Close(String::New(obj->address_));
}

void RegisterModule(Handle<Object> target) {
  PMTAMessage::Init(target);
  PMTARecipient::Init(target);
  PMTAConnection::Init(target);
}

NODE_MODULE(pmta, RegisterModule);