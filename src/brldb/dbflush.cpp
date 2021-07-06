/*
 * BerylDB - A lightweight database.
 * http://www.beryldb.com
 *
 * Copyright (C) 2021 - Carlos F. Ferry <cferry@beryldb.com>
 *
 * This file is part of BerylDB. BerylDB is free software: you can
 * redistribute it and/or modify it under the terms of the BSD License
 * version 3.
 *
 * More information about our licensing can be found at https://docs.beryl.dev
 */

#include "beryl.h"
#include "brldb/database.h"
#include "brldb/datathread.h"
#include "brldb/dbnumeric.h"
#include "brldb/dbflush.h"
#include "algo.h"

std::mutex DataFlush::query_mute;

std::mutex DataFlush::mute;

void DataFlush::Pause()
{
      this->running = false;
}

void DataFlush::Resume()
{
      this->running = true;
}

bool DataFlush::Status()
{
      return this->running;
}

DataFlush::DataFlush() : running(false), flushmute(false)
{

}

void DataFlush::AttachResult(std::shared_ptr<QueryBase> signal)
{
      std::lock_guard<std::mutex> lg(DataFlush::mute);
      signal->user->notifications.push_back(signal);
}

void DataFlush::GetResults()
{
      const UserMap& users = Kernel->Clients->GetInstances();
      
      if (!users.size())
      {
            return;
      }

      DataFlush::mute.lock();
      
      for (UserMap::const_iterator u = users.begin(); u != users.end(); ++u)
      {
                  User* user = u->second;

                  if (user == NULL || user->IsQuitting() || !user->notifications.size())
                  {
                        continue;
                  }
                  
                  /* Nothing to iterate if user has no pending notifications. */

                  if (user->notifications.size())
                  {
                        std::shared_ptr<QueryBase> signal = user->notifications.front();
                        
                        /* Removes signal from queue. */
                        
                        user->notifications.pop_front();
                        
                        if (!signal->GetStatus())
                        {
                             LocalUser* localuser = IS_LOCAL(user);
                             NOTIFY_MODS(OnQueryFailed, (signal->access, localuser, signal));
                        }
                        
                        switch (signal->access)
                        {
                             case DBL_NOT_FOUND:
                             
                                 DataFlush::NotFound(user, signal);

                             break;
                             
                             case DBL_INVALID_RANGE:
                                
                                DataFlush::InvalidRange(user, signal);
                             
                             break;
                             
                             case DBL_INVALID_FORMAT:
                             
                                  DataFlush::InvalidFormat(user, signal);
                                  
                             break;
                             
                             case DBL_MISS_ARGS:
                             
                                    DataFlush::MissArgs(user, signal);
                             
                             break;
                             
                             case DBL_INVALID_TYPE:
                              
                                     DataFlush::InvalidType(user, signal);
                              
                             break;                             
                             
                             case DBL_ENTRY_EXISTS:
        
                                     DataFlush::EntryExists(user, signal);
                             
                             break;
                             
                             case DBL_STATUS_BROKEN:
                             
                                     DataFlush::StatusFailed(user, signal);
                            break;
                            
                            case DBL_UNABLE_WRITE:
                            
                                    DataFlush::UnableWrite(user, signal);
                                    
                            break;
                            
                            case DBL_ENTRY_EXPIRES:

                                    DataFlush::EntryExpires(user, signal);

                            break;                            
                            
                            case DBL_NOT_NUM:
                                   
                                   DataFlush::NotNumeric(user, signal);
                                   
                            break;
                            
                            default:
                             
                               DataFlush::Flush(user, signal);
                        }

                        user->SetLock(false);
                }
      }
          
      DataFlush::mute.unlock();
}

void DataFlush::Process()
{
      /* We should not dispatch anything if we are paused. */
      
      if (!Kernel->Store->Flusher->Status())  
      {
            return;   
      }

      DataFlush::GetResults();
      DataFlush::GetPending();
}

void DataFlush::GetPending()
{
      /* Before processing user pendings, we must check global. */
      
      if (Kernel->Clients->Global->pending.size())
      {
               Process(Kernel->Clients->Global, Kernel->Clients->Global->pending.front());
               return;
      }

      const UserMap& users = Kernel->Clients->GetInstances();
      
      if (!users.size())
      {
            return;
      }
      
      for (UserMap::const_iterator u = users.begin(); u != users.end(); ++u)
      {
               User* user = u->second;

               unsigned int pending = user->pending.size();

               if ((user && !user->IsQuitting()) && (!pending || user->IsLocked()))
               {
                    continue;
               }
               
               user->SetLock(true);
               Process(user, user->pending.front());
      }
}

void DataThread::SetStatus(bool flag)
{
      this->busy = flag;
}

bool DataThread::IsBusy()
{
      return this->busy;
}

void DataFlush::Process(User* user, std::shared_ptr<QueryBase> signal)
{
      if (!signal || !Kernel->Store->Flusher->Status())
      {
            return;
      }

      DataThreadVector& Threads = Kernel->Store->Flusher->GetThreads();
      
      /* Thread to use. */
      
      DataThread *ToUse = NULL;
      
      for (DataThreadVector::iterator iter = Threads.begin(); iter != Threads.end(); ++iter)
      {
           DataThread* thread = *iter;
           
           /* First, we attempt to find an unused thread. */
           
           if (!thread->IsBusy())
           {
                 ToUse = thread;
                 
                 /* No need to continue looking for a thread. */
                 
                 break; 
           }
      }
      
      /* Checks whether we found a not-busy thread. */
      
      if (!ToUse)
      {
            /* We find a random thread to attach this signal to. */
      
            ToUse = *random_item(Threads.begin(), Threads.end());
      }

      /* Unable to find any thread at all. */
      
      if (!ToUse)
      {
            user->SetLock(false);
            user->pending.clear();
            return;
      }

      user->pending.pop_front();

      /* Let's start processing this signal. */
      
      ToUse->Post(signal);
}

void DataFlush::ResetAll()
{
      Kernel->Store->Flusher->Pause();

      if (Kernel->Clients->Global)
      {
           Kernel->Clients->Global->pending.clear();
           Kernel->Clients->Global->notifications.clear();
      }
      
      /* Clear up pending notifications. */

      const UserMap& users = Kernel->Clients->GetInstances();
      
      DataFlush::mute.lock();
      
      for (UserMap::const_iterator u = users.begin(); u != users.end(); ++u)
      {           
              User* user = u->second;
            
              if (user == NULL || user->IsQuitting())
              {
                     continue;
              }

              user->notifications.clear();
              user->pending.clear();
      }

      DataFlush::mute.unlock();

      Kernel->Store->Expires->Reset();
      Kernel->Store->Flusher->Resume();
}

DataThread::DataThread() : m_thread(nullptr), busy(false)
{

}

void DataThread::Exit()
{
      if (!m_thread)
      {
           return;
      }

      std::shared_ptr<ThreadMsg> Input(new ThreadMsg(PROC_EXIT_THREAD, 0));
      {
                std::lock_guard<std::mutex> lock(m_mutex);
                queue.push(Input);
                m_cv.notify_one();
      }

      m_thread->join();
      m_thread = nullptr;
}

void DataThread::Post(std::shared_ptr<QueryBase> query)
{	
      if (!m_thread)
      {
            return;
      }

      if (!query || !query->database || query->database->IsClosing())
      {
              query->user->SendProtocol(ERR_QUERY, DB_NULL);
              query->user->SetLock(false);
              return;
      }
           
      std::shared_ptr<ThreadMsg> Input(new ThreadMsg(PROC_SIGNAL, query));
      std::unique_lock<std::mutex> lk(m_mutex);
      queue.push(Input);
      m_cv.notify_one();
}

std::thread::id DataThread::Create()
{
      if (!m_thread)
      {
           m_thread = std::unique_ptr<std::thread>(new std::thread(&DataThread::Process, this)); 
      }

      return m_thread->get_id();
}

void DataThread::Process()
{
        while (true)
        {
              std::shared_ptr<ThreadMsg> signal;
              {
                      std::unique_lock<std::mutex> lk(m_mutex);

                      while (queue.empty())
                      {
                            m_cv.wait(lk);
                      }

                      if (queue.empty())
                      {
                            continue;
                      }

                      signal = queue.front();
              }

              /* Indicates this thread is busy. */

              this->SetStatus(true);

              if (!Kernel->Store->Flusher->Status())
              {
                    continue;
              }

              switch (signal->id)
              {
                    case PROC_EXIT_THREAD:
                    {
                          this->clear();
                          return;
                    }
                    
                    case PROC_SIGNAL:
                    {
                          std::shared_ptr<QueryBase> request = signal->msg;
                          
                          /* Let's release the locked user. */

                          if (request->user && !request->user->IsQuitting())
                          {
                                /* OK */
                                
                          }
                          else
                          {
                               /* In case lock status gets locked. */
                               
                               if (request->user)
                               {
                                     request->user->SetLock(false);
                               }
                               
                               queue.pop();
                               
                               signal.reset();
                               signal = nullptr;
                               
                               continue;
                          }
                          
                          queue.pop();
                          
                          DataFlush::query_mute.lock();
                          
                          if (request->access != DBL_INVALID_FORMAT)
                          {
                                request->Prepare();
                          }

                          DataFlush::query_mute.unlock();

                          if (request->flags == QUERY_FLAGS_QUIET)
                          {
                                break;
                          }
                          
                          if (request->access != DBL_INTERRUPT)
                          {
                                /* Adds result to the pending notification list. */

                               DataFlush::AttachResult(request);
                          }
                        
                        break;
                    }

                    default:
                    {
                           break;
                    }
              }
              
              /* Thread is no longer busy. */

              this->SetStatus(false);
        }
}

DataThread::~DataThread()
{

}

void DataFlush::CloseThreads()
{
      DataThreadVector& Threads = Kernel->Store->Flusher->GetThreads();

      unsigned int counter = 0;
      
      for (DataThreadVector::iterator iter = Threads.begin(); iter != Threads.end(); ++iter)
      {
             DataThread* thread = *iter;
             thread->Exit();
             thread = NULL;
             counter++;
      }     
      
      Kernel->Store->Flusher->EraseAll();
      iprint(counter, "Thread%s closed.", counter > 1 ? "s" : "");
      slog("DATABASE", LOG_VERBOSE, "Thread%s closed.", counter > 1 ? "s" : "");
}