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
#include "settings.h"
#include "helpers.h"
#include "managers/user.h"
#include "managers/settings.h"
#include "managers/maps.h"
#include "subscription.h"

Settings::Settings() 
{

}

void Settings::SetDefaults()
{
       defaults.insert({ "allowchans", "true" });
       defaults.insert({ "syntaxhints", "true" });
       defaults.insert({ "autojoin", "false" });
       defaults.insert({ "chansync", "false" });

      for (std::map<std::string, std::string> ::iterator i = this->defaults.begin(); i != this->defaults.end(); i++)      
      {
             std::string key = i->first;
             std::string value = i->second;
             this->Set(key, value);
      }
}

void Settings::Load()
{
        bprint(INFO, "Loading settings.");
  
        Args confs = STHelper::HKeys("conf");
        
        for (Args::iterator i = confs.begin(); i != confs.end(); i++)
        {
             std::string key = *i;
             this->SetMap[key] = STHelper::Get("conf", key);
        }
}

void Settings::Set(const std::string& key, const std::string& value)
{
      this->SetMap[key] = value;
      STHelper::Set("conf", key, value);
}

std::string Settings::Get(const std::string& key)
{
        std::map<std::string, std::string>::iterator it = this->SetMap.find(key);

        if (it == this->SetMap.end())
        {
                return "";
        }
        
        return it->second;
}

bool Settings::AsBool(const std::string& key)
{
        return Helpers::as_bool(this->Get(key), false);
}

std::string Helpers::to_bool(bool boolean)
{
        if (boolean)
        {
                return "true";
        }
        else
        {
                return "false";
        }
}

bool Helpers::as_bool(const std::string &key, bool def)
{
        std::string result = key;

        if (key.empty())
        {
                return def;
        }

        if (stdhelpers::string::equalsci(result, "yes") || stdhelpers::string::equalsci(result, "true") || stdhelpers::string::equalsci(result, "on") || result == "1")
        {
                return true;
        }

        if (stdhelpers::string::equalsci(result, "no") || stdhelpers::string::equalsci(result, "false") || stdhelpers::string::equalsci(result, "off") || result == "0")
        {
                return false;
        }

        return def;
}

std::string Helpers::Format(const std::string& fmt)
{
    return Daemon::Format("\"%s\"", fmt.c_str());
}

void Helpers::make_query(User* user, std::shared_ptr<QueryBase> base, const std::string& key, bool allow)     
{
      base->user = user;
      base->database = user->GetDatabase();
      base->select_query = user->select;
      
      if (!key.empty())
      {
            base->key = key;
            
            if (!allow)
            {
                  if (!Kernel->Engine->ValidKey(key))
                  {
                      base->access_set(DBL_INVALID_FORMAT);
                  }
            }
      }
}

void Helpers::make_geo_query(User* user, std::shared_ptr<QueryBase> base, const std::string& key)
{
      base->user = user;
      base->database = user->GetDatabase();
      base->select_query = user->select;

      if (!key.empty())
      {
            base->key = key;
      }
}

void Helpers::make_map(User* user, std::shared_ptr<QueryBase> base, const std::string& key, const std::string& hesh, bool allow)
{
     base->user = user;
     base->database = user->GetDatabase();
     base->select_query = user->select;
     
     if (!key.empty() && !hesh.empty())
     {
            base->key = key;
            base->hesh = hesh;

            if (!allow)
            {
                  if (!Kernel->Engine->ValidKey(key))
                  {
                      base->access_set(DBL_INVALID_FORMAT);
                  }
            }

     }
}

void Helpers::make_cmap(std::shared_ptr<QueryBase> base, const std::string& key, const std::string& hesh)
{
       base->database = Kernel->Core->DB;
       base->select_query = "1";
       base->flags = QUERY_FLAGS_CORE;
     
       if (!key.empty() && !hesh.empty())
       {
            base->key = key;
            base->hesh = hesh;

            if (!Kernel->Engine->ValidKey(key))
            {
                  base->access_set(DBL_INVALID_FORMAT);
            }
       }
}

void Helpers::make_cquery(std::shared_ptr<QueryBase> base, const std::string& key)
{
      base->database = Kernel->Core->DB;
      base->select_query = "1";
      base->flags = QUERY_FLAGS_CORE;
      
      if (!key.empty())
      {
            base->key = key;
            
            if (!Kernel->Engine->ValidKey(key))
            {
                  base->access_set(DBL_INVALID_FORMAT);
            }
      }
}

void Helpers::make_mmap(User* user, std::shared_ptr<QueryBase> base, const std::string& key, bool allow)
{
     base->user = user;
     base->database = user->GetDatabase();
     base->select_query = user->select;
     
     if (!key.empty())
     {
            base->key = key;

            if (!allow)
            {
                  if (!Kernel->Engine->ValidKey(key))
                  {
                      base->access_set(DBL_INVALID_FORMAT);
                  }
            }
     }
}

void Helpers::make_list(User* user, std::shared_ptr<QueryBase> base, const std::string& key, bool allow)     
{
      base->user = user;
      base->database = user->GetDatabase();
      base->select_query = user->select;
      
      if (!key.empty())
      {
            base->key = key;

            if (!allow)
            {
                  if (!Kernel->Engine->ValidKey(key))
                  {
                      base->access_set(DBL_INVALID_FORMAT);
                  }
            }
      }
}

std::string Helpers::TypeString(const std::string& type)
{
     if (type.empty() || type == PROCESS_NULL)
     {
          return PROCESS_NULL;
     }
     
     if (type == INT_KEY)
     {
           return "KEY";
     }
     
     if (type == INT_GEO)
     {
          return "GEO";
     }
     
     if (type == INT_LIST)
     {
          return "LIST";
     }
     
     if (type == INT_MAP)
     {
          return "MAP";
     }
     
     if (type == INT_VECTOR)
     {
          return "VECTOR";
     }
     
     if (type == INT_MMAP)
     {
          return "MULTIMAP";
     }
     
     return "UNKNOWN";
}


