#include <atomic>
#include <mutex>
#include <condition_variable>
#include "sockex.h"
#include "tlv.h"

struct _threadsynheader {
  INT32 synid;
  INT32 synseq;
};

sockthread* freethreads = nullptr;
sockthread* allthreads[THREADPOOL_THREAD_MAXNUM] = {0};

atomic_int sockthreadnum(0); //main thread
mutex cvmtx; // for condition-variable

sockthread::sockthread(bool hasthread) {
  if (!hasthread) {
    if (CreateThread(NULL, 0, sockRoutine, this, 0, &tid) == NULL) {
      throw std::logic_error("pthread_create ex");
    }
  } else {
    tid = GetCurrentThreadId();

    WSADATA data;
    WORD w = MAKEWORD(2, 2);
    WSAStartup(w, &data);
  }

  type = LISTENING;
  msg = nullptr;

  allthreads[sockthreadnum] = this;
  sockthreadnum += 1;
}

int sockthread::getselfid() {
  for (int i = 0; i < sockthreadnum; i++) {
    if (allthreads[i]->tid == GetCurrentThreadId()) {
      return i;
    }
  }

  return 0xFFFFFFFF;
}

//refrence to article, https://www.cnblogs.com/haippy/p/3301408.html 
void sockthread::withdraw() { // actually is interlockedpush
  if (type == LISTENING) {
    return;
  }
  type = FREE;
  synseq++; //avoid after withdrawed, an previous msg wakeup this thread
  
  // suspend-self
  std::unique_lock <std::mutex> lck(cvmtx);
  this->next = freethreads;
  freethreads = this;
  sig.wait(lck);
}

sockthread* sockthread::getfree() { //actually is interlockedpop
  if (freethreads == nullptr) {
    if (sockthreadnum < THREADPOOL_THREAD_MAXNUM) {
      sockthread *th = new sockthread(false);
      return th;
    }
    return nullptr; //must has a thread to process packets, orelse maybe all threads are waiting for packet but no one process incoming data, causing dead-lock
  }

  sockthread* oldHead = freethreads;
  freethreads = oldHead->next;
  oldHead->type = LISTENING;
  oldHead->wakeup();
  return oldHead;
  /* wtf: I am a hacker? there is a simple method which using mutex to implement list
  while (!freethreads.compare_exchange_weak(oldHead, newHead)) {
    newHead = oldHead->next; //MUST consider the oldHead is nullptr, so using recursive maybe express simple
  } */
}

char* sockthread::wait(int timeout) {
  sockthread* cur = allthreads[getselfid()];
  if (cur->msg != nullptr) {
    delete cur->msg;
    cur->msg = nullptr;
  }

  std::unique_lock <std::mutex> lck(cvmtx);
  if (cur->type == LISTENING) {
    cur->type = WORKING;
    //LOG("cur threadid:%d, timeout:%d", cur->tid, timeout);
    //scan exist thread, if any is listen, don't get free thread
    /*sockthread* listenthread = nullptr;
    for (int i = 0; i < sockthreadnum; i++) {
      if (allthreads[i]->type == LISTENING) {
        listenthread = allthreads[i];
        break;
      }
    }

    if (listenthread == nullptr) {
      listenthread = getfree();
    }
    if (listenthread == nullptr) {*/
    if (getfree() == nullptr) {
      cur->type = LISTENING;
      return nullptr;
    }
  } else {
    // cur is WORKING indicate there is a LISTENING thread;
  }

  if (timeout == 0) {
    return (char*)-1;
  }

  if (cur->sig.wait_for(lck, chrono::seconds(timeout)) == cv_status::timeout) {
    //dbgOut("waitfor timeout:%d", timeout);
  }
  cur->synseq++;
  return cur->msg;
}

void sockthread::wakeup(char* buf, int buflen) {
/*  if (this->type == LISTENING) { //avoid when free, be waked up incorrectly
    return;
  }
*/
  if (buf != nullptr) {
    msg = new char[buflen];
    memcpy(msg, buf, buflen);
  }

//  std::unique_lock <std::mutex> lck(cvmtx);
  sig.notify_one();
}

void sockthread::pack_synheader(StarTlv* msg) {
  _threadsynheader synheader;
  synheader.synid = getselfid();
  synheader.synseq = allthreads[synheader.synid]->synseq;
  msg->pack_atom(TLV_RES_SYNHEAD, sizeof(_threadsynheader), (char*)&synheader);
}

int TlvSynReplyCb(SockEx* sockex, StarTlv& tlvs) {
  _threadsynheader* synheader = (_threadsynheader*)tlvs.get_tlv(TLV_RES_SYNHEAD);
  if (synheader == nullptr) {
    return 0;
  }

  if (synheader->synid >= sockthreadnum) {
    return 0;
  }
  sockthread* synthread = allthreads[synheader->synid];
  if (synthread->type != sockthread::WORKING) {
    return 0;
  }

  if (synthread->synseq == synheader->synseq) {
    synthread->wakeup(tlvs.in - sizeof(struct _tlv), tlvs.total + sizeof(struct _tlv));
  }

  return 0;
}

