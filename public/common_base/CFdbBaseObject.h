/*
 * Copyright (C) 2015   Jeremy Chen jeremy_cz@yahoo.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _CFDBBASEOBJECT_H_
#define _CFDBBASEOBJECT_H_

#include <map>
#include <set>
#include "CFdbMessage.h"
#include "CMethodJob.h"

enum EFdbEndpointRole
{
    FDB_OBJECT_ROLE_SERVER,
    FDB_OBJECT_ROLE_NS_SERVER,
    FDB_OBJECT_ROLE_CLIENT,
    FDB_OBJECT_ROLE_UNKNOWN
};

class CBaseWorker;
class CFdbMessage;
class CFdbSession;
class CBaseEndpoint;
struct CFdbSessionInfo;
namespace google
{
    namespace protobuf
    {
        template <typename Element> class RepeatedPtrField;
    }
}

typedef std::set<std::string> tFdbFilterSets;
typedef std::map<FdbMsgCode_t, tFdbFilterSets> tFdbSubscribeMsgTbl;
typedef std::set<CFdbSession *> tSubscribedSessionSets;
class CFdbBaseObject
{
public:
#define FDB_OBJ_ENABLED_MIGRATE (1 << 0)
#define FDB_OBJ_AUTO_REMOVE     (1 << 1)
#define FDB_OBJ_RECONNECT       (1 << 2)
#define FDB_OBJ_REGISTERED      (1 << 3)

    CFdbBaseObject(const char *name = 0, CBaseWorker *worker = 0, EFdbEndpointRole role = FDB_OBJECT_ROLE_UNKNOWN);
    virtual ~CFdbBaseObject();

    /*
     * invoke[1]
     * Invoke method call at receiver asynchronously. It can be called from client
     * to server or from server to client.
     * @iparam receiver: the receiver the request is sent to
     * @iparam code: the message code
     * @iparam data: the message data sent to receiver
     * @iparam timeout: timeout in unit of ms; 0 if no timer is specified
     *
     * CBaseServer::onInvoke() or CBaseClient::onInvoke() will be called to
     * process the request and reply will be sent after the request is
     * processed, which trigger invokation of onResponse() of the message;
     * If timeout is specified but receiver doesn't reply within specified time,
     * status is returned with error code being FDB_ST_TIMEOUT.
     */
    void invoke(FdbSessionId_t receiver
                , FdbMsgCode_t code
                , const CFdbBasePayload &data
                , int32_t timeout = 0);

    /*
     * invoke[1.1]
     * Similiar to invoke[1] but instead of passing message code, message
     * object is given. Usually the object is subclass of CBaseMessage so
     * that extra data can be attached.
     * In onInvoke(CBaseJob::Ptr &msg_ref), castToMessage() can be used to
     * convert msg_ref to subclass of CBaseMessage so that the extra data
     * can be retrieved.
     */
    void invoke(FdbSessionId_t receiver
                , CFdbMessage *msg
                , const CFdbBasePayload &data
                , int32_t timeout = 0);

    /*
     * invoke[2]
     * Similiar to invoke[1] without parameter 'receiver'. The method is called at
     * CBaseClient to invoke method upon connected server.
     */
    void invoke(FdbMsgCode_t code
                , const CFdbBasePayload &data
                , int32_t timeout = 0);
    /*
     * invoke[2.1]
     * Similiar to invoke[1.1] without parameter 'receiver'. The method is called at
     * CBaseClient to invoke method upon connected server.
     */
    void invoke(CFdbMessage *msg
                , const CFdbBasePayload &data
                , int32_t timeout = 0);

    /*
     * invoke[3]
     * Similiar to invoke[1] but is synchronous, which means the method blocks
     * until receiver reply. The added parameter 'msg_ref' is used to pass
     * (subclass of) CBaseMessage object in when called, and pass the same object
     * out, in which return data can be retrieved.
     *
     * @ioparam msg_ref: input - object of CBaseMessage or its subclass
     *                   output - the same object containing return data
     */
    void invoke(FdbSessionId_t receiver
                , CBaseJob::Ptr &msg_ref
                , const CFdbBasePayload &data
                , int32_t timeout = 0);

    /*
     * invoke[4]
     * Similiar to invoke[3] but 'receiver' is not specified. The method is
     * called by CBaseClient to invoke method upon connected server.
     */
    void invoke(CBaseJob::Ptr &msg_ref
                , const CFdbBasePayload &data
                , int32_t timeout = 0);

    /*
     * invoke[5]
     * Similiar to invoke[1] but raw data is sent rather than proto-buffer.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    void invoke(FdbSessionId_t receiver
                , FdbMsgCode_t code
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    void invoke(FdbSessionId_t receiver
                , CFdbMessage *msg
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    /*
     * invoke[6]
     * Similiar to invoke[2] but raw data is sent rather than proto-buffer.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    void invoke(FdbMsgCode_t code
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    void invoke(CFdbMessage *msg
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    /*
     * invoke[7]
     * Similiar to invoke[3] but raw data is sent rather than proto-buffer.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    void invoke(FdbSessionId_t receiver
                , CBaseJob::Ptr &msg_ref
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    /*
     * invoke[8]
     * Similiar to invoke[4] but raw data is sent rather than proto-buffer.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    void invoke(CBaseJob::Ptr &msg_ref
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    /*
     * send[1]
     * send protocol buffer to receiver. No reply is expected; just one-shot
     * @iparam receiver: the receiver to get the message
     * @iparam code: message code
     * @imaram data: message in protocol buffer format
     */
    void send(FdbSessionId_t receiver
              , FdbMsgCode_t code
              , const CFdbBasePayload &data);
    /*
     * send[2]
     * Similiar to send[1] without parameter 'receiver'. The method is called
     * from CBaseClient to send message to the connected server.
     */
    void send(FdbMsgCode_t code
              , const CFdbBasePayload &data);

    /*
     * send[3]
     * Similiar to send[1] but raw data is sent.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    void send(FdbSessionId_t receiver
              , FdbMsgCode_t code
              , const void *buffer = 0
              , int32_t size = 0);
    /*
     * send[4]
     * Similiar to send[2] but raw data is sent.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    void send(FdbMsgCode_t code
              , const void *buffer = 0
              , int32_t size = 0);

    /*
     * broadcast[1]
     * broadcast message of protocol buffer to all connected clients.
     * The clients should have already registered message 'code' with
     * optional filter.
     * @iparam code: code of the message to be broadcasted
     * @iparam data: message of protocol buffer
     * @iparam filter: the filter associated with the broadcasting
     */
    void broadcast(FdbMsgCode_t code
                   , const CFdbBasePayload &data
                   , const char *filter = 0);
    /*
     * broadcast[3]
     * Similiar to broadcast[1] except that raw data is broadcasted.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    void broadcast(FdbMsgCode_t code
                   , const char *filter = 0
                   , const void *buffer = 0
                   , int32_t size = 0);
    /*
     * Build subscribe list before calling subscribe().
     *
     * @oparam msg_list: the protobuf holding message sending subscribe
     *      request to server
     * @iparam msg_code: The message code to subscribe
     * @iparam filter: the filter associated with the message.
     */
    static void addNotifyItem(NFdbBase::FdbMsgSubscribe &msg_list
                              , FdbMsgCode_t msg_code
                              , const char *filter = 0);

    /*
     * subscribe[1]
     * Subscribe a list of messages so that the messages can be received
     *      when the server broadcast a message.
     * The method doesn't block and is asynchronous. Once completed, onStatus()
     *      will be called with CBaseMessage::isSubscribe() being true.
     *
     * @iparam msg_list: list of messages to be subscribed
     * @iparam timeout: optional timeout
     */
    void subscribe(NFdbBase::FdbMsgSubscribe &msg_list
                   , int32_t timeout = 0);

    /*
     * subscribe[1.1]
     * Similiar to subscribe[1] but additional parameter msg is given. The
     *      msg is object of subclass of CBaseMessage allowing extra data to be
     *      attached with the transaction.
     * in onStatus() with CBaseMessage::isSubscribe() being true, the object
     *      can be retrieved with castToMessage().
     *
     * @iparam msg_ist: list of messages to be retrieved
     * @iparam msg: (subclass of) CBaseMessage containing transaction-
     *      specific data
     * @iparam timeout: a timer expires if response is not received from
     *      server with the specified time.
     */
    void subscribe(NFdbBase::FdbMsgSubscribe &msg_list
                   , CFdbMessage *msg
                   , int32_t timeout = 0);

    /*
     * subscribe[2]
     * Similiar to subscribe[1] but is synchronous. It will block until the
     *      server has registered all messages in list.
     *
     * @ioparam msg_ref: if msg_ref doesn't hold anything, CBaseMessage is
     *      used by default; if msg_ref holds object of (subclass of)
     *      CBaseMessage, transaction-specific data can be attached.
     *
     * @iparam msg_ist: list of messages to be retrieved
     * @iparam timeout: timer of the transaction
     */
    void subscribe(CBaseJob::Ptr &msg_ref
                   , NFdbBase::FdbMsgSubscribe &msg_list
                   , int32_t timeout = 0);

    /*
     * Unsubscribe messages listed in msg_list
     */
    void unsubscribe(NFdbBase::FdbMsgSubscribe &msg_list);
    
    /*
     * Unsubscribe the whole object
     */
    void unsubscribe();

    FdbObjectId_t bind(CBaseEndpoint *endpoint, FdbObjectId_t oid = FDB_INVALID_ID);
    FdbObjectId_t connect(CBaseEndpoint *endpoint, FdbObjectId_t oid = FDB_INVALID_ID);

    void unbind();
    void disconnect();

    std::string &name()
    {
        return mName;
    }

    /*
     * The following methods migrate onXXX() callbacks to specified worker thread.
     *      CBaseEndpoint::onXXX() runs at thread CFdbContext if worker is not
     *      specified for a endpoint. You can manually migrate the callbacks to
     *      any worker and run within the context of the worker.
     */
    bool migrateOnSubscribeToWorker(CBaseJob::Ptr &msg_ref, CBaseWorker *worker = 0);
    bool migrateOnBroadcastToWorker(CBaseJob::Ptr &msg_ref, CBaseWorker *worker = 0);
    bool migrateOnInvokeToWorker(CBaseJob::Ptr &msg_ref, CBaseWorker *worker = 0);
    bool migrateOnOfflineToWorker(FdbSessionId_t sid, bool is_last, CBaseWorker *worker = 0);
    bool migrateOnOnlineToWorker(FdbSessionId_t sid, bool is_first, CBaseWorker *worker = 0);
    bool migrateOnReplyToWorker(CBaseJob::Ptr &msg_ref, CBaseWorker *worker = 0);
    bool migrateOnStatusToWorker(CBaseJob::Ptr &msg_ref
                                 , int32_t error_code
                                 , const char *description
                                 , CBaseWorker *worker = 0);

    /*
     * Get a list of subscribed messages.
     * Warning: not thread-safe!!! MUST be used within CONTEXT!!!
     */
    void getSubscribeTable(tFdbSubscribeMsgTbl &table);
    void getSubscribeTable(FdbMsgCode_t code, tFdbFilterSets &filters);
    void getSubscribeTable(FdbMsgCode_t code, const char *filter,
                            tSubscribedSessionSets &session_tbl);

    CBaseWorker *worker() const
    {
        return mWorker;
    }

    // Warning: Not thread safe and should be set before working!!
    void worker(CBaseWorker *worker)
    {
        mWorker = worker;
    }

    FdbObjectId_t objId() const
    {
        return mObjId;
    }

    EFdbEndpointRole role()
    {
        return mRole;
    }

    FdbEndpointId_t epid() const
    {
        return mEpid;
    }

    bool enableMigrate() const
    {
        return !!(mFlag & FDB_OBJ_ENABLED_MIGRATE);
    }

    void enableMigrate(bool enb)
    {
        if (enb)
        {
            mFlag |= FDB_OBJ_ENABLED_MIGRATE;
        }
        else
        {
            mFlag &= ~FDB_OBJ_ENABLED_MIGRATE;
        }
    }

    void autoRemove(bool enb)
    {
        if (enb)
        {
            mFlag |= FDB_OBJ_AUTO_REMOVE;
        }
        else
        {
            mFlag &= ~FDB_OBJ_AUTO_REMOVE;
        }
    }

    bool autoRemove()
    {
        return !!(mFlag & FDB_OBJ_AUTO_REMOVE);
    }
    
    void enableReconnect(bool active)
    {
        if (active)
        {
            mFlag |= FDB_OBJ_RECONNECT;
        }
        else
        {
            mFlag &= ~FDB_OBJ_RECONNECT;
        }
    }

    bool isReconnect()
    {
        return !!(mFlag & FDB_OBJ_RECONNECT);
    }

    void setDefaultSession(FdbSessionId_t sid = FDB_INVALID_ID)
    {
        mSid = sid;
    }

    FdbSessionId_t getDefaultSession()
    {
        return mSid;
    }

    // Internal use only!!!
    void broadcast(FdbSessionId_t sid
                  , FdbObjectId_t obj_id
                  , FdbMsgCode_t code
                  , const CFdbBasePayload &data
                  , const char *filter = 0);

    // Internal use only!!!
    void broadcast(FdbSessionId_t sid
                  , FdbObjectId_t obj_id
                  , FdbMsgCode_t code
                  , const char *filter = 0
                  , const void *buffer = 0
                  , int32_t size = 0);

    // Internal use only!!!
    bool broadcast(CFdbMessage *msg, CFdbSession *session);

protected:
    std::string mName;
    /*
     * Implemented by server: called when client subscribe a message
     *      IMPORTANT: in the callback, current value must sent to the
     *      client represented by sid using unicast version of
     *      broadcast() so that client can be updated asap.
     * @iparam sid - the session subscribing the message
     * @iparam msg_code - the message subscribed
     * @iparam filter - the filter subscribed
     */
    virtual void onSubscribe(CBaseJob::Ptr &msg_ref)
    {}
    /*
     * Implemented by client: called when server broadcast a message.
     * @iparam msg_ref: reference to a message. Using castToMessage()
     *      to convert it to CBaseMessage.
     */
    virtual void onBroadcast(CBaseJob::Ptr &msg_ref)
    {}
    /*
     * Implemented by either client or server: called at receiver when
     *      the sender call invoke() or send() upon the receiver.
     */
    virtual void onInvoke(CBaseJob::Ptr &msg_ref)
    {}

    /*
     * Implemented by either client or server: called when a session
     *      is destroyed.
     * @iparam sid: id of the session being destroyed
     * @iparam is_last: true when it is the last session to be destroyed
     */
    virtual void onOffline(FdbSessionId_t sid, bool is_last)
    {}
    /*
     * Implemented by either client or server: called when a session
     *      is created.
     * @iparam sid: id of the session created
     * @iparam is_first: true when it is the first session to create
     */
    virtual void onOnline(FdbSessionId_t sid, bool is_first)
    {}
    /*
     * Implemented by either client or server: called when
     *      CBaseMessage::reply() is called by the sender
     */
    virtual void onReply(CBaseJob::Ptr &msg_ref)
    {}
    /*
     * Implemented by either client or server: called when
     *      CBaseMessage::status() is called by the sender
     */
    virtual void onStatus(CBaseJob::Ptr &msg_ref
                          , int32_t error_code
                          , const char *description)
    {}

    /*
     * Implemented by either client or server: called when
     *      an message (subscribe, invoke, send...) is
     *      received targeting an object which does not exist.
     *      The callback should create the associated object
     *      according to the message.
     *      =============== Warning!!! ===================
     *      1. It is running ONLY at CONTEXT thread even
     *      though worker thread is specified!!!
     *      2. If some tasks HAS to be done at other threads,
     *      use synchronous job. The synchronous job shall
     *      NOT call any FDBus API since the CONTEXT is
     *      blocked by the synchronous job.
     */
	virtual void onCreateObject(CBaseEndpoint *endpoint, CFdbMessage *msg)
    {}

    void role(EFdbEndpointRole role)
    {
        mRole = role;
    }

    bool isPrimary()
    {
        return mObjId == FDB_OBJECT_MAIN;
    }

    virtual bool authentication(CFdbSessionInfo const &session_info)
    {
        return true;
    }

    void objId(FdbObjectId_t obj_id)
    {
        mObjId = obj_id;
    }

    void epid(FdbEndpointId_t epid)
    {
        mEpid = epid;
    }

    bool registered()
    {
        return !!(mFlag & FDB_OBJ_REGISTERED);
    }

    void registered(bool active)
    {
        if (active)
        {
            mFlag |= FDB_OBJ_REGISTERED;
        }
        else
        {
            mFlag &= ~FDB_OBJ_REGISTERED;
        }
    }

    /*
     * request-reply through side band.
     * Note: Only used internally for FDBus!!!
     */
    void invokeSideband(FdbMsgCode_t code
                      , const CFdbBasePayload &data
                      , int32_t timeout = 0);
    void sendSideband(FdbMsgCode_t code, const CFdbBasePayload &data);
    virtual void onSidebandInvoke(CBaseJob::Ptr &msg_ref)
    {}
    virtual void onSidebandReply(CBaseJob::Ptr &msg_ref)
    {}

    virtual bool onMessageAuthentication(CFdbMessage *msg, CFdbSession *session)
    {
        return true;
    }
    virtual bool onEventAuthentication(CFdbMessage *msg, CFdbSession *session)
    {
        return true;
    }

private:
    typedef std::map<std::string, bool> FilterTable_t;
    typedef std::map<FdbObjectId_t, FilterTable_t> ObjectTable_t;
    typedef std::map<CFdbSession *, ObjectTable_t> SessionTable_t;
    typedef std::map<FdbMsgCode_t, SessionTable_t> SubscribeTable_t;

    CBaseWorker *mWorker;
    SubscribeTable_t mSessionSubscribeTable;
    FdbEndpointId_t mEpid; // Which endpoint the object belongs to
    FdbObjectId_t mObjId;
    uint32_t mFlag;
    EFdbEndpointRole mRole;
    FdbSessionId_t mSid;

    void subscribe(CFdbSession *session, FdbMsgCode_t msg, FdbObjectId_t obj_id, const char *filter = 0);
    void unsubscribe(CFdbSession *session, FdbMsgCode_t msg, FdbObjectId_t obj_id, const char *filter = 0);
    void unsubscribe(CFdbSession *session);
    void unsubscribe(FdbObjectId_t obj_id);
    void broadcast(CFdbMessage *msg);
    void sendFdbLog(const CFdbBasePayload &data
                  , uint8_t *log_data
                  , int32_t size
                  , int32_t clipped_size = -1);
    void sendTraceLog(const CFdbBasePayload &data
                  , uint8_t *log_data
                  , int32_t size);
    void broadcastFdbLog(const CFdbBasePayload &data
                       , const uint8_t *log_data
                       , int32_t size);
    void broadcastTraceLog(const CFdbBasePayload &data
                         , const uint8_t *log_data
                         , int32_t size);

    void getSubscribeTable(SessionTable_t &sessions, tFdbFilterSets &filters);
    void getSubscribeTable(FdbMsgCode_t code, CFdbSession *session, tFdbFilterSets &filter_tbl);
    
    void callOnSubscribe(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callOnBroadcast(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callOnInvoke(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callOnOffline(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callOnOnline(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callOnReply(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callOnStatus(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callBindObject(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callConnectObject(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callUnbindObject(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callDisconnectObject(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);

    void doSubscribe(CBaseJob::Ptr &msg_ref);
    void doBroadcast(CBaseJob::Ptr &msg_ref);
    void doInvoke(CBaseJob::Ptr &msg_ref);
    void doReply(CBaseJob::Ptr &msg_ref);
    void doStatus(CBaseJob::Ptr &msg_ref);

    FdbObjectId_t addToEndpoint(CBaseEndpoint *endpoint, FdbObjectId_t obj_id);
    void removeFromEndpoint();
    FdbObjectId_t doBind(CBaseEndpoint *endpoint, FdbObjectId_t obj_id); // Server object
    FdbObjectId_t doConnect(CBaseEndpoint *endpoint, FdbObjectId_t obj_id); // Client object
    void doUnbind();
    void doDisconnect();
    void notifyOnline(CFdbSession *session, bool is_first);
    void notifyOffline(CFdbSession *session, bool is_last);

    friend class COnSubscribeJob;
    friend class COnBroadcastJob;
    friend class COnInvokeJob;
    friend class COnOfflineJob;
    friend class COnOnlineJob;
    friend class COnReplyJob;
    friend class COnStatusJob;
    friend class CBindObjectJob;
    friend class CConnectObjectJob;
    friend class CUnbindObjectJob;
    friend class CDisconnectObjectJob;

    friend class CBaseClient;
    friend class CBaseServer;
    friend class CFdbSession;
    friend class CFdbMessage;
    friend class CBaseEndpoint;
    friend class CLogProducer;
    friend class CLogServer;
};

template<typename T>
class CFdbMessageHandle
{
private:
    typedef void (T::*tCallbackFn)(CBaseJob::Ptr &msg_ref);
    typedef std::map<FdbMsgCode_t, tCallbackFn> tCallbackTbl;

public:
    void registerCallback(FdbMsgCode_t code, tCallbackFn callback)
    {
        mCallbackTbl[code] = callback;
    }

    bool processMessage(T *instance, CBaseJob::Ptr &msg_ref)
    {
        CFdbMessage *msg = castToMessage<CFdbMessage *>(msg_ref);
        typename tCallbackTbl::iterator it = mCallbackTbl.find(msg->code());
        if (it == mCallbackTbl.end())
        {
            return false;
        }
        (instance->*(it->second))(msg_ref);
        return true;
    }
private:
    tCallbackTbl mCallbackTbl;
};

template<typename T>
class CFdbSubscribeHandle
{
private:
    typedef void (T::*tCallbackFn)(CFdbMessage *msg, const ::NFdbBase::FdbMsgSubscribeItem *sub_item);
    typedef std::map<FdbMsgCode_t, tCallbackFn> tCallbackTbl;

public:
    void registerCallback(FdbMsgCode_t code, tCallbackFn callback)
    {
        mCallbackTbl[code] = callback;
    }

    bool processMessage(T *instance, CFdbMessage *msg, const ::NFdbBase::FdbMsgSubscribeItem *sub_item, FdbMsgCode_t code)
    {
        typename tCallbackTbl::iterator it = mCallbackTbl.find(code);
        if (it == mCallbackTbl.end())
        {
            return false;
        }
        (instance->*(it->second))(msg, sub_item);
        return true;
    }
private:
    tCallbackTbl mCallbackTbl;
};

#endif
