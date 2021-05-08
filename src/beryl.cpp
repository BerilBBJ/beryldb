/*
 * BerylDB - A modular database.
 * http://www.beryldb.com
 *
 * Copyright (C) 2015-2021 Carlos F. Ferry <cferry@beryldb.com>
 * 
 * This file is part of BerylDB. BerylDB is free software: you can
 * redistribute it and/or modify it under the terms of the BSD License
 * version 3.
 *
 * More information about our licensing can be found at https://docs.beryl.dev
 */

#include <signal.h>

#include "beryl.h"
#include "exit.h"

#include "brldb/dbmanager.h"
#include "brldb/dbflush.h"
#include "brldb/expires.h"
#include "managers/databases.h"

std::unique_ptr<Beryl> Kernel = nullptr;

/* Beryl's main. */
 
int main(int argc, char** argv)
{
	Kernel = std::make_unique<Beryl>(argc, argv);
	Kernel.reset();
	return 1;
}

Beryl::Beryl(int argc, char** argv) : ConfigFile(DEFAULT_CONFIG)
{
	/* Main link. */
	
	Kernel = std::unique_ptr<Beryl>(this);
	
	/* Refreshing this->TIME for the very first time. */
	
	this->Refresh();
	
	/* 
	 * Startup time. 
	 *
	 * time_t startup is used in order to calculate uptime.
	 * You may see your instance's uptime in Emerald
	 * by using the 'status u' command.
 	 */
	
	this->startup = this->Now();

	/* Improve core limits, thus leading to a better debugging experience. */
	
	this->Engine->SetCoreLimits();
	
	/* Generates a random base from TIME */
	
	this->RandomSeed();

	/* Creates epoll's fds */
	
	SocketPool::Start();

	/* Configuration class. This class will read and our config file. */
	
	this->Config = std::make_unique<Configuration>();
	
	/* Keep a copy of arguments from main() */
	
	this->Config->usercmd.argv = argv;
	
	/* We must pass both, argv and argc */
	
	this->Config->usercmd.argc = argc;

	/* Goes over user arguments by parsing with getopt. */
	
	this->CommandLine();

	{
		/* Service Providers for basic functions, such as join and publish. */
		
		ServiceProvider* provs[] =
		{
			&brldevents.numeric, &brldevents.join, &brldevents.part, &brldevents.quit, &brldevents.login,
			&brldevents.publish, &brldevents.ping, &brldevents.pong,
			&brldevents.error
		};

		this->Modules->AttachServices(provs, sizeof(provs)/sizeof(provs[0]));
	}

	/* Let's verify if provided configuration file exists. */
	
	if (!Daemon::ConfigFileExists(this->ConfigFile))
	{
		bprint(ERROR, "Unable to open config file: %s", Daemon::Welcome(this->ConfigFile).c_str());
		this->Exit(EXIT_CODE_CONFIG);
	}

        Kernel->Initialize();
}

void Beryl::Initialize()
{
	/* Signals to recognize. */
	
	Daemon::AssignSignals();

	/* Checks whether we are allowed to be running as root. */
	
	if (!this->Config->usercmd.asroot)
	{
		this->Engine->CheckRoot();
	}

	/* 
	 * Checks whether the --nofork arg has been provided, and if so,
         * move to the background .. or not. 
	 */
	  
	if (!this->Config->usercmd.nofork)
	{
		this->Engine->DaemonFork();
	}
	else
	{
	        bprint(INFO, "This instance will %s detach.", Daemon::Welcome("NOT").c_str());
        	do_newline;
	}
	
	/* Load configuration file. */

	this->Config->Load();
	
	/* Once our configuration file is loaded, we must set internal variables. */
	
	this->Config->SetAll();

	/* Opens core database. */
	
	this->Core->Open();
		
	/* Checks if this instance has ever been ran. */
	
	this->Core->CheckDefaults();
	
	/* Configuration file is valid and ready. */
	
	if (this->Config->Ready)
	{
		bprint(DONE, "Configuration file: %s", Daemon::Welcome(this->ConfigFile).c_str());
	}
	
	/* Opens all logs as provided in the configuration file. */
	
	this->Logs->OpenLogs();

	/* We must initialize our UID handler at this point. */
	
	this->Config->sid = UIDHandler::Create();

	/* Sets up the very first unique ID for future clients. */

	this->UID.Initialize();

	/* User that acts as a server. */
	
	this->Clients->Global = new GlobalUser(Kernel->Config->sid, Kernel->Config->ServerName);

	/* We are ready to start listening for connections. */
	
	this->StartListening();

        /* 
         * Resets all pending flushes. 
         * Given that ResetAll also removes timers, this function
         * should be called before loading all modules, which loads 
         * unflushed timers at startup.
         */
        
        DataFlush::ResetAll();

	/* Opens database processing threads. */

	this->Store->OpenAll();

        /* Checks whether flushdb argument was provided by user. */

        if (this->Config->usercmd.flushdb)
        {
                DBHelper::FlushDB(true);
                
                /* Flushes and exists. */
                
                this->Exit(EXIT_CODE_OK, false, true);
        }

        /* Loads all modules (both, core and modules will be loaded). */
        
        this->Modules->LoadAll();

	/* server name in bold. */
	
	bprint(DONE, "Beryl is now running as '%s'", Daemon::Welcome(this->Config->ServerName).c_str());
	
	/* If applies, Detach() sends BerylDB to the background. */
	
	this->Detach();

	/* Saves beryldb.pid file so we can later make calls to our bin */
	
	this->Engine->SavePID();

	/* Run as different user. */
	
	this->Engine->RunAs();

	slog("STARTUP", LOG_DEFAULT, "BerylDB running as %s [%s], with max. sockets: %lu ", this->Config->ServerName.c_str(), Config->GetSID().c_str(), SocketPool::GetMaxFds());

	/* Checks whether user wants to run TestOffice. */
	
	this->CheckOffice();
	
	/* We may now begin processing queries. */
	
	this->Store->Flusher.Resume();

	/* Start mainloop */

	this->Dispatcher();
}

void Beryl::Dispatcher()
{
	/* We wait 1 second before start processing requests. */

	usleep(PRELOOP_WAIT);

	/* Refresh timer before entering mainloop. */
	
	this->Refresh();
	
	time_t PREV_TIME = this->TIME.tv_sec;

	/* Main loop */

	do
	{
                /* this->TIME should be updated in every loop. */

                this->Refresh();

		if (this->TIME.tv_sec != PREV_TIME)
		{	
		    	PREV_TIME = TIME.tv_sec;
			
		    	/* Run functions that are meant to run every 1 second. */
		    	
			this->RunTimed(this->TIME.tv_sec);
		}

	        /* Delivers pending data to clients. */

	        this->Clients->Flush(this->TIME.tv_sec);
		
		/* Main processes. */
		
		this->Loop();
 	
		/* Signal detector. */
		
 	        if (s_signal)
 	        {
			this->SignalManager(s_signal);
			Beryl::s_signal = 0;
                }
	}		
	while (true);
}

void Beryl::Loop()
{
        /*
         * Calls our socket pool to wait on all active file descriptors.
         * This pool handles all socket-related writes,
         * from modules to clients.
         * In other words, these two functions below will read and/or write
         * events that are later dispatched by their fds.
         */

        SocketPool::Writes();
        SocketPool::Events();
      
	/* Removes all quitting clients. */
	
        this->Reducer->Apply();

        /* Dispatches both, pending queries and notifications. */
        
        DataFlush::DispatchAll();
        
        /* 
         * Runs functions meant to be run outside current 
         * (at time of addition) loop. 
         */
        
        this->Atomics->Run();
}

void Beryl::RunTimed(time_t current)
{
 	/* We call all pending timers to be delivered. */

        this->Tickers->Flush(current);
        
        /* Only works every 2 secs */
        
        if ((current % 2) == 0)
        {
	           /* Forces the exit of users with quitting status. */
	           
	           this->Clients->ForceExits();

 	           /* Function called every two seconds. */
 	           
                   NOTIFY_MODS(OnEveryTwoSeconds, (current));

                   if ((current % 30) == 0)
                   {
                             NOTIFY_MODS(OnEveryHalfMinute, (current));
                   }

                   if ((current % 3600) == 0)
                   {
			     /* 
			      * Login cache should only be in memory for
			      * an hour. There is no need to store ACL
			      * data for weeks.
			      */
			     
			     this->Logins->ResetCache();
			     
			     /* Runs every one hour. */
			     
			     NOTIFY_MODS(OnEveryHour, (current));
                   }
                   
         }
         
        /* Removes expiring entries. */
 	
  	this->Store->Expires->Flush(current);        
        
        /* 
         * CPU stats, we need this information to handle
         * overall usage of resources. 
         */
                    
        this->Engine->SnapshotStats();
}

sig_atomic_t Beryl::s_signal = 0;

void Beryl::AssignSignal(int signal)
{
	s_signal = signal;
}

void Beryl::Exit(int status, bool nline, bool skip, const std::string& exitmsg)
{
        if (nline)
        {
                do_newline;
        }
        
        if (!skip)
        {
        	if (!exitmsg.empty())
        	{
        		this->PrepareExit(status, exitmsg);
		}
		else
		{
			this->PrepareExit(status, Daemon::Format("%s: %s", SERVER_EXITING, ExitMap[status]).c_str());
		}
	}
	
        exit (status);
}

void Beryl::PrepareExit(int status, const std::string& quitmsg)
{
	bprint(INFO, "Preparing exit: %s (code %i)", ExitMap[status], status);
	
	/* Always a great idea to keep track of exiting date. */
	
	slog("STARTUP", LOG_DEFAULT, "Preparing exit: %s (code %i)", ExitMap[status], status);
	
	/* Login cache will not be needed anymore. */
	
	this->Logins->ResetCache();

	/* 
	 * ResetAll removes all pending flushes. 
	 *
	 * Flushes can be either notifications or pending 
	 * queries.
	 */
	
	DataFlush::ResetAll();

	/* No more requests will be processed. */
	
	Kernel->Store->Flusher->Pause();
	
	/* Stop listening in all ports. */
	
	for (unsigned int i = 0; i < binds.size(); i++)
	{
		binds[i]->discard();
		delete binds[i];
	}
	
	binds.clear();

	/* We delete all pending expires. */
	
	this->Store->Expires->Reset();
	
	/* 
	 * Removes PID file (if any). Keep in mind that this function
	 * will not have any effect when running using --nofork.
	 *
	 * Just to recap, --nofork prevents a pid file to be written.
	 */
	
	this->Engine->DeletePID();
			
	/* Modules should remove their local data. */
	
	NOTIFY_MODS(OnHalt, (SERVER_EXITING));

	/* Let's disconnect all active clients. */
	
	const ClientManager::LocalList& clients = this->Clients->GetLocals();
	
	/* Disconnecting counter. */
	
	unsigned int csize = 0;
	
	/* We disconnect all connected clients. */
	
	while (!clients.empty())
	{
		/* NOTE: SERVER_EXITING is defined in constants.h */
		
		this->Clients->Disconnect(clients.front(), quitmsg);
		csize++;
	}
	
	/* Displays counter only if it is greater than zero. */
	
	if (csize > 0)
	{
		bprint(DONE, "%i client%s disconnected.", csize, csize > 1 ? "s" : "");
                slog("STARTUP", LOG_DEFAULT, "%i client%s disconnected.", csize, csize > 1 ? "s" : "");
	}
	else
	{
		bprint(INFO, "No clients have been disconnected.");
                slog("STARTUP", LOG_DEFAULT, "No clients have been disconnected.");
	}

	/* Reset channels. */
	
	this->Channels->Reset();	
	
	/* Runs pending reducers. */
	
	this->Reducer->Apply();

	/* It is now safe to unload all modules. */
	
	this->Modules->UnloadAll();

	/* Shuts down socket pool. */
	
	SocketPool::CloseAll();

	/* Beryl should stop logging at this point. */
	
	this->Logs->CloseLogs();
	
	/* Info message. */
	
	bprint(INFO, "Exit finished.");

        /* Configuration class is not needed anymore, so we reset it. */

        this->Config.reset();
        
	/* Cleans up config file. */
	
        this->ConfigFile.clear();
        
        /* No need to block anymore. */
        
        this->Lock = false;
}
