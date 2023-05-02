/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: chat.proto */

#ifndef PROTOBUF_C_chat_2eproto__INCLUDED
#define PROTOBUF_C_chat_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _ChatSistOS__UserList ChatSistOS__UserList;
typedef struct _ChatSistOS__UsersOnline ChatSistOS__UsersOnline;
typedef struct _ChatSistOS__UserOption ChatSistOS__UserOption;
typedef struct _ChatSistOS__Answer ChatSistOS__Answer;
typedef struct _ChatSistOS__User ChatSistOS__User;
typedef struct _ChatSistOS__NewUser ChatSistOS__NewUser;
typedef struct _ChatSistOS__Status ChatSistOS__Status;
typedef struct _ChatSistOS__Message ChatSistOS__Message;


/* --- enums --- */


/* --- messages --- */

struct  _ChatSistOS__UserList
{
  ProtobufCMessage base;
  protobuf_c_boolean list;
  char *user_name;
};
#define CHAT_SIST_OS__USER_LIST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&chat_sist_os__user_list__descriptor) \
    , 0, (char *)protobuf_c_empty_string }


struct  _ChatSistOS__UsersOnline
{
  ProtobufCMessage base;
  size_t n_users;
  ChatSistOS__User **users;
};
#define CHAT_SIST_OS__USERS_ONLINE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&chat_sist_os__users_online__descriptor) \
    , 0,NULL }


struct  _ChatSistOS__UserOption
{
  ProtobufCMessage base;
  int32_t op;
  ChatSistOS__NewUser *createuser;
  ChatSistOS__UserList *userlist;
  ChatSistOS__Status *status;
  ChatSistOS__Message *message;
};
#define CHAT_SIST_OS__USER_OPTION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&chat_sist_os__user_option__descriptor) \
    , 0, NULL, NULL, NULL, NULL }


struct  _ChatSistOS__Answer
{
  ProtobufCMessage base;
  int32_t op;
  int32_t response_status_code;
  char *response_message;
  ChatSistOS__UsersOnline *users_online;
  ChatSistOS__Message *message;
  ChatSistOS__User *user;
  ChatSistOS__Status *status;
};
#define CHAT_SIST_OS__ANSWER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&chat_sist_os__answer__descriptor) \
    , 0, 0, (char *)protobuf_c_empty_string, NULL, NULL, NULL, NULL }


struct  _ChatSistOS__User
{
  ProtobufCMessage base;
  char *user_name;
  char *user_ip;
  int32_t user_state;
};
#define CHAT_SIST_OS__USER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&chat_sist_os__user__descriptor) \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0 }


struct  _ChatSistOS__NewUser
{
  ProtobufCMessage base;
  char *username;
  char *ip;
};
#define CHAT_SIST_OS__NEW_USER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&chat_sist_os__new_user__descriptor) \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string }


struct  _ChatSistOS__Status
{
  ProtobufCMessage base;
  char *user_name;
  int32_t user_state;
};
#define CHAT_SIST_OS__STATUS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&chat_sist_os__status__descriptor) \
    , (char *)protobuf_c_empty_string, 0 }


struct  _ChatSistOS__Message
{
  ProtobufCMessage base;
  protobuf_c_boolean message_private;
  char *message_destination;
  char *message_content;
  char *message_sender;
};
#define CHAT_SIST_OS__MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&chat_sist_os__message__descriptor) \
    , 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string }


/* ChatSistOS__UserList methods */
void   chat_sist_os__user_list__init
                     (ChatSistOS__UserList         *message);
size_t chat_sist_os__user_list__get_packed_size
                     (const ChatSistOS__UserList   *message);
size_t chat_sist_os__user_list__pack
                     (const ChatSistOS__UserList   *message,
                      uint8_t             *out);
size_t chat_sist_os__user_list__pack_to_buffer
                     (const ChatSistOS__UserList   *message,
                      ProtobufCBuffer     *buffer);
ChatSistOS__UserList *
       chat_sist_os__user_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   chat_sist_os__user_list__free_unpacked
                     (ChatSistOS__UserList *message,
                      ProtobufCAllocator *allocator);
/* ChatSistOS__UsersOnline methods */
void   chat_sist_os__users_online__init
                     (ChatSistOS__UsersOnline         *message);
size_t chat_sist_os__users_online__get_packed_size
                     (const ChatSistOS__UsersOnline   *message);
size_t chat_sist_os__users_online__pack
                     (const ChatSistOS__UsersOnline   *message,
                      uint8_t             *out);
size_t chat_sist_os__users_online__pack_to_buffer
                     (const ChatSistOS__UsersOnline   *message,
                      ProtobufCBuffer     *buffer);
ChatSistOS__UsersOnline *
       chat_sist_os__users_online__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   chat_sist_os__users_online__free_unpacked
                     (ChatSistOS__UsersOnline *message,
                      ProtobufCAllocator *allocator);
/* ChatSistOS__UserOption methods */
void   chat_sist_os__user_option__init
                     (ChatSistOS__UserOption         *message);
size_t chat_sist_os__user_option__get_packed_size
                     (const ChatSistOS__UserOption   *message);
size_t chat_sist_os__user_option__pack
                     (const ChatSistOS__UserOption   *message,
                      uint8_t             *out);
size_t chat_sist_os__user_option__pack_to_buffer
                     (const ChatSistOS__UserOption   *message,
                      ProtobufCBuffer     *buffer);
ChatSistOS__UserOption *
       chat_sist_os__user_option__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   chat_sist_os__user_option__free_unpacked
                     (ChatSistOS__UserOption *message,
                      ProtobufCAllocator *allocator);
/* ChatSistOS__Answer methods */
void   chat_sist_os__answer__init
                     (ChatSistOS__Answer         *message);
size_t chat_sist_os__answer__get_packed_size
                     (const ChatSistOS__Answer   *message);
size_t chat_sist_os__answer__pack
                     (const ChatSistOS__Answer   *message,
                      uint8_t             *out);
size_t chat_sist_os__answer__pack_to_buffer
                     (const ChatSistOS__Answer   *message,
                      ProtobufCBuffer     *buffer);
ChatSistOS__Answer *
       chat_sist_os__answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   chat_sist_os__answer__free_unpacked
                     (ChatSistOS__Answer *message,
                      ProtobufCAllocator *allocator);
/* ChatSistOS__User methods */
void   chat_sist_os__user__init
                     (ChatSistOS__User         *message);
size_t chat_sist_os__user__get_packed_size
                     (const ChatSistOS__User   *message);
size_t chat_sist_os__user__pack
                     (const ChatSistOS__User   *message,
                      uint8_t             *out);
size_t chat_sist_os__user__pack_to_buffer
                     (const ChatSistOS__User   *message,
                      ProtobufCBuffer     *buffer);
ChatSistOS__User *
       chat_sist_os__user__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   chat_sist_os__user__free_unpacked
                     (ChatSistOS__User *message,
                      ProtobufCAllocator *allocator);
/* ChatSistOS__NewUser methods */
void   chat_sist_os__new_user__init
                     (ChatSistOS__NewUser         *message);
size_t chat_sist_os__new_user__get_packed_size
                     (const ChatSistOS__NewUser   *message);
size_t chat_sist_os__new_user__pack
                     (const ChatSistOS__NewUser   *message,
                      uint8_t             *out);
size_t chat_sist_os__new_user__pack_to_buffer
                     (const ChatSistOS__NewUser   *message,
                      ProtobufCBuffer     *buffer);
ChatSistOS__NewUser *
       chat_sist_os__new_user__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   chat_sist_os__new_user__free_unpacked
                     (ChatSistOS__NewUser *message,
                      ProtobufCAllocator *allocator);
/* ChatSistOS__Status methods */
void   chat_sist_os__status__init
                     (ChatSistOS__Status         *message);
size_t chat_sist_os__status__get_packed_size
                     (const ChatSistOS__Status   *message);
size_t chat_sist_os__status__pack
                     (const ChatSistOS__Status   *message,
                      uint8_t             *out);
size_t chat_sist_os__status__pack_to_buffer
                     (const ChatSistOS__Status   *message,
                      ProtobufCBuffer     *buffer);
ChatSistOS__Status *
       chat_sist_os__status__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   chat_sist_os__status__free_unpacked
                     (ChatSistOS__Status *message,
                      ProtobufCAllocator *allocator);
/* ChatSistOS__Message methods */
void   chat_sist_os__message__init
                     (ChatSistOS__Message         *message);
size_t chat_sist_os__message__get_packed_size
                     (const ChatSistOS__Message   *message);
size_t chat_sist_os__message__pack
                     (const ChatSistOS__Message   *message,
                      uint8_t             *out);
size_t chat_sist_os__message__pack_to_buffer
                     (const ChatSistOS__Message   *message,
                      ProtobufCBuffer     *buffer);
ChatSistOS__Message *
       chat_sist_os__message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   chat_sist_os__message__free_unpacked
                     (ChatSistOS__Message *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*ChatSistOS__UserList_Closure)
                 (const ChatSistOS__UserList *message,
                  void *closure_data);
typedef void (*ChatSistOS__UsersOnline_Closure)
                 (const ChatSistOS__UsersOnline *message,
                  void *closure_data);
typedef void (*ChatSistOS__UserOption_Closure)
                 (const ChatSistOS__UserOption *message,
                  void *closure_data);
typedef void (*ChatSistOS__Answer_Closure)
                 (const ChatSistOS__Answer *message,
                  void *closure_data);
typedef void (*ChatSistOS__User_Closure)
                 (const ChatSistOS__User *message,
                  void *closure_data);
typedef void (*ChatSistOS__NewUser_Closure)
                 (const ChatSistOS__NewUser *message,
                  void *closure_data);
typedef void (*ChatSistOS__Status_Closure)
                 (const ChatSistOS__Status *message,
                  void *closure_data);
typedef void (*ChatSistOS__Message_Closure)
                 (const ChatSistOS__Message *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor chat_sist_os__user_list__descriptor;
extern const ProtobufCMessageDescriptor chat_sist_os__users_online__descriptor;
extern const ProtobufCMessageDescriptor chat_sist_os__user_option__descriptor;
extern const ProtobufCMessageDescriptor chat_sist_os__answer__descriptor;
extern const ProtobufCMessageDescriptor chat_sist_os__user__descriptor;
extern const ProtobufCMessageDescriptor chat_sist_os__new_user__descriptor;
extern const ProtobufCMessageDescriptor chat_sist_os__status__descriptor;
extern const ProtobufCMessageDescriptor chat_sist_os__message__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_chat_2eproto__INCLUDED */
