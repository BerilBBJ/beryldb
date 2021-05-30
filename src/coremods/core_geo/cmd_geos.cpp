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
#include "brldb/dbmanager.h"
#include "brldb/dbnumeric.h"
#include "extras.h"
#include "brldb/query.h"
#include "managers/geo.h"
#include "engine.h"
#include "core_geo.h"

CommandGeoAdd::CommandGeoAdd(Module* Creator) : Command(Creator, "GEOADD", 3, 3)
{
         syntax = "<name> <longitude> <latitude>";
}

COMMAND_RESULT CommandGeoAdd::Handle(User* user, const Params& parameters)
{
       const std::string& gname = parameters[0];
       const std::string& latitude = parameters[1];
       const std::string& longitude = parameters[2];
       
       if (!is_number(latitude, true) || !is_number(longitude, true))
       {
                 user->SendProtocol(ERR_USE, ERR_NOT_NUM, MUST_BE_NUMERIC.c_str());
                 return FAILED;
       }
       
       if (!ValidLong(convto_num<int>(longitude)))
       {
             Dispatcher::SmartCmd(user, ERR_USE, ERR_NOT_VALID_COORDINATE, INVALID_COORD);
             return FAILED;
       }
       
       if (!ValidLat(convto_num<int>(latitude)))
       {
             Dispatcher::SmartCmd(user, ERR_USE, ERR_NOT_VALID_COORDINATE, INVALID_COORD);
             return FAILED;
       }
       
       GeoHelper::Add(user, user->current_db, user->select, gname, latitude, longitude);
       return SUCCESS;
}

CommandGeoGet::CommandGeoGet(Module* Creator) : Command(Creator, "GEOGET", 1, 1)
{
         syntax = "<name>";
}

COMMAND_RESULT CommandGeoGet::Handle(User* user, const Params& parameters)
{
       const std::string& gname = parameters[0];
       
       GeoHelper::Get(user, user->current_db, user->select, gname);
       return SUCCESS;
}

CommandGeoDel::CommandGeoDel(Module* Creator) : Command(Creator, "GEODEL", 1, 1)
{
         syntax = "<name>";
}

COMMAND_RESULT CommandGeoDel::Handle(User* user, const Params& parameters)
{
       const std::string& gname = parameters[0];
       
       GeoHelper::Del(user, user->current_db, user->select, gname);
       return SUCCESS;
}

CommandGFind::CommandGFind(Module* Creator) : Command(Creator, "GFIND", 1, 3)
{
         syntax = "<\%key> <offset> <limit>";
}

COMMAND_RESULT CommandGFind::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];

       signed int offset;
       signed int limit;

       if (parameters.size() == 2)
       {
             if (!is_zero_or_great(parameters[1]))
             {
                   user->SendProtocol(ERR_USE, ERR_GREAT_ZERO, MUST_BE_GREAT_ZERO.c_str());
                   return FAILED;
             }

             limit = convto_num<signed int>(parameters[1]); 
             offset = 0;
       }
       else if (parameters.size() == 3)
       {
             limit = convto_num<signed int>(parameters[2]); 
             offset = convto_num<signed int>(parameters[1]);

             if (!is_zero_or_great(parameters[1]) || !is_zero_or_great(parameters[2]))
             {
                   user->SendProtocol(ERR_USE, ERR_GREAT_ZERO, MUST_BE_GREAT_ZERO.c_str());
                   return FAILED;
             }
       }
       else
       {
            limit = -1;
            offset = 0;
       }
       
       GeoHelper::Find(user, user->current_db, user->select, key, offset, limit);
       return SUCCESS;
}



