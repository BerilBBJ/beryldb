/*
 * BerylDB - A modular database.
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

#include <mutex>

class ExpireEntry
{
    public:

        time_t schedule;
        time_t added;
        
        bool epoch;

        unsigned int secs;
        std::string select;
        std::string key;
        std::shared_ptr<Database> database;

        ExpireEntry()
        {

        }
};

typedef std::multimap<time_t, ExpireEntry> ExpireMap;

class Externalize ExpireManager : public safecast<ExpireManager>
{
    public:
        
        static std::mutex mute;
        
        /* Constructor */
        
        ExpireManager();

        /* Expires map */
        
        ExpireMap ExpireList;
        
        /* Submits all expires to main queue */
        
        void Flush(time_t TIME);
        
        signed int Add(signed int schedule, const std::string& key, const std::string& select, bool epoch = false);

        /* Returns triggering time for a timer. */
        
        static signed int TriggerTIME(const std::string& key, const std::string& select);

        /* Deletes an entry from the expire map. */
        
        static bool Delete(const std::string& key, const std::string& select);

        /* Finds an expiring entry. */
        
        ExpireEntry Find(const std::string& key, const std::string& select);
        
        /* Get all Expires */
        
        ExpireMap& GetExpires();        

        signed int GetTTL(const std::string& key, const std::string& select);
        
        /* Clears everything inside ExpireList. */
        
        static void Reset();
        
        /* Select reset */
        
        static unsigned int SReset(const std::string& select);

        /* Counts all timers. */
        
        unsigned int CountAll()
        {
            return this->ExpireList.size();
        }
        
        /* Counts items on a given select. */
        
        unsigned int Count(const std::string& select);
};


