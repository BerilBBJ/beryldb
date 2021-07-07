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

#pragma once

#include <thread>

class ExportAPI Daemon : public safecast<Daemon>
{
    friend class Beryl;

    private:

        /* 
         * Records statistical information.
         * This function is called during every loop.
         */    
         
        void SnapshotStats();

        void RunAs();
        
        /* 
         * Checks whether configuration file exists.
         * 
         * @parameters:
	 *
	 *         · path: Path to check.
	 * 
         * @return:
 	 *
         *         · True: File OK.
         *         · False: Unable to locate files.
         */            

        static bool ConfigFileExists(std::string& path);
        
        void SetCoreLimits();
        
    public:
    
    	/* Main thread */
    	
    	std::thread::id main_threadid;
        
        /* Constructor */
        
        Daemon();  

        /* Prints a new line */
                
        static void print_newline(unsigned int loops);

        /* Prints text in bold */
        
        static std::string Welcome(const std::string& msg);

        static std::string Welcome(std::string& msg);
        
        /* Deletes PID file. */
        
        void DeletePID();

        /* 
         * Saves PID.
         * 
         * @parameters:
         *
         *         · exitonfail: Will Exit Beryl if unable to write.
         */

        void SavePID(bool exitonfail = true);

        STR1::function<bool(const std::string&)> ValidChannel;

        /* 
         * Validates a channel.
         * 
         * @parameters:
	 *
	 *         · channel: Channel to validate.
	 * 
         * @return:
 	 *
         *         · True: Valid channel.
         *         · False: Unable to validate channel.
         */    
         
        static bool ChannelValidator(const std::string& channel);

        /* 
         * Generates random string.
         * 
         * @parameters:
	 *         
	 *         · output: Where to return.
	 *         · max: String limit.
         */    
         
        static void DefaultGenRandom(char* output, size_t max);
        
        STR1::function<void(char*, size_t)> GenRandom;

        /* 
         * Checks for a valid key.
         * 
         * @parameters:
         *
         *         · Key to check.
         * 
         * @return:
         *
         *         · true: Valid Key, false: invalid.
         */

        static bool KeyValidator(const std::string& key);
        
        STR1::function<bool(const std::string&)> ValidKey;

        /* 
         * Validates provided login.
         * 
         * @parameters:
	 *
	 *         · login to check.
	 * 
         * @return:
 	 *
         *         · True: Login has a correct format.
         *         · False: Poorly-formated login.
         */            

        static bool LoginValidator(const std::string& login);

        STR1::function<bool(const std::string&)> ValidLogin;

        /* 
         * Checks whether a valid agent has been provided.
         * 
         * @parameters:
	 *
	 *         · agent to check.
	 * 
         * @return:
 	 *
         *         · True: Valid agent.
         *         · False: Invalid agent.
         */    
        

        static bool AgentValidator(const std::string& agent);

        STR1::function<bool(const std::string&)> IsAgent;

        /* 
         * Checks whether provided database name is valid.
         * 
         * @parameters:
	 *
	 *         · Database name to check.
	 * 
         * @return:
 	 *
         *         · True: Valid database.
         *         · False: Invalid name.
         */    
        
        static bool DBValidator(const std::string& name);

        STR1::function<bool(const std::string&)> IsDatabase;
        
        static bool ValidHost(const std::string& host);
        
        static bool Valid_SID(const std::string& sid);
        
        std::string generate_random_str(unsigned int length, bool printable = true);

        
        unsigned long generate_random_int(unsigned long max);

        /* 
         * Checks if BerylDB had been started as root.
         *
         * @exit:
         *
         *       · This function will exit if started using root, unless
         *         --asoot argument was provided.
         */

        
        void CheckRoot();

        std::string get_real_path(const char* path);

        /* 
         * Formats an string.
         * 
         * @parameters:
	 *
	 *         · printf like string to format.
	 * 
         * @return:
 	 *
         *         · string: Formatted string.
         */    
       
        static std::string Format(const char* TempString, ...) BERYL_PRINTF(1, 2);
        
        static std::string Format(va_list& vaList, const char* TempString) BERYL_PRINTF(2, 0);

        /* 
         * Converts a given time_t timestamp into human-readable time.
         * 
         * @parameters:
	 *
	 *         · time_t: Time to convert.
	 *         · format: Optional format to use.
	 *         · utc: Whether using utc or not.
	 * 
         * @return:
 	 *
         *         · string: Formated string.
         */    
         
        static std::string HumanEpochTime(time_t curtime, const char* format = NULL, bool utc = false);

        static bool TimingSafeCompare(const std::string& one, const std::string& two);

        ProtocolInterface DefaultProtocolInterface;
        
        ProtocolInterface* PI = NULL;

        /* 
         * Matches an string with a given mask.
         * 
         * @parameters:
	 *
	 *         · str: String to check.
	 *         · mask: Mask to check against.
	 * 
         * @return:
 	 *
         *         · True: Matches.
         *         · False: String did not match.
         */    
        
        static bool Match(const std::string& str, const std::string& mask, unsigned const char* map = NULL);
        
        static bool Match(const char* str, const char* mask, unsigned const char* map = NULL);
        
        static bool MatchCompactIP(const std::string& str, const std::string& mask, unsigned const char* map = NULL);

        static bool MatchCompactIP(const char* str, const char* mask, unsigned const char* map = NULL);
        
        static bool MatchMask(const std::string& masks, const std::string& hostname, const std::string& ipaddr);
        
        static unsigned long Duration(const std::string& str);
        
        static bool Duration(const std::string& str, unsigned long& duration);
        
        static bool IsValidDuration(const std::string& str);
        
        static std::string duration_as_string(time_t duration);

        /* 
         * Checks the format of a string (ie "string query").
         * 
         * @parameters:
	 *
	 *         · value: String to check.
	 *         · notify: Whether this function notifies to the requesting user.
         *
         * @return:
 	 *
         *         · True: Valid query.
         *         · False: Invalid query.
         */    
         
        static bool CheckFormat(User* user, const std::string& value, bool notify = true);
        
        static bool CheckRange(User* user, const std::string& value, const std::string& reason, int min, int max);
        
        bool PassCompare(Expandable* ex, const std::string& data, const std::string& input, const std::string& hashtype);

        /* 
         * Prints a colored-formated string.
         * 
         * @parameters:
	 *
	 *         · type: String type (INFO, ERROR, DONE)
	 *         · buff: Buffer to print.
         */    
         
         void printb(const int type, const std::string& buff);

         void printb(const int type, const char *fmt, ...) BERYL_PRINTF(3, 4);
         
        /* 
         * Prints an colored notification with a counter.
         * 
         * @parameters:
	 *
	 *         · number: Counter to print.
	 *         · buff: Buffer to print.
         */            
        
         void iprintb(int number, const std::string& buff);

         void iprintb(int number, const char *fmt, ...) BERYL_PRINTF(3, 4);
        
        /* 
         * Formats uptime into a string.
         * 
         * @parameters:
	 *
	 *         · msg: Msg to incorporate.
	 *         · up: uptime in seconds.
	 * 
         * @return:
 	 *
         *         · Formatted string.
         */    
        
        static std::string Uptime(const std::string& msg, unsigned int up);
};

class ExportAPI Dispatcher : public safecast<Dispatcher>
{
   public:
    
        static void SmartDiv(User* user, BRLD_PROTOCOL brld, const std::string& key, const std::string& msg, const std::string& div);

        static void SmartCmd(User* user, BRLD_PROTOCOL brld, BRLD_PROTOCOL brld2, const std::string& msg);
     
        static void TellThat(std::string& who, const std::string& msg, int rpl);      
     
        /* 
         * JustAPI transmits given data only if end-user is connected
         * via an API (not Emerald).
         *
         * @parameters:
	 *
	 *         · user: User to send data to.
	 *         · brld: Protocol.
         */    
        
        static void JustAPI(User* user, BRLD_PROTOCOL brld);
     
        /*  
         * Formats an item inside a list.
         * 
         * @parameters:
	 *
	 *         · User: User to dispatch data to.
	 *         · brld: Protocol to send.
	 *         · one_api: First string.
	 *         · second_api: Second string.
         */    
         
        static void CondList(User* user, BRLD_PROTOCOL brld, const std::string& one_api, const std::string& second_api, bool comillas = false);
};
