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
#include "brldb/query.h"
#include "managers/databases.h"
#include "converter.h"
#include "engine.h"
#include "core_dbmanager.h"

CommandUsing::CommandUsing(Module* Creator) : Command(Creator, "USING", 1, 1)
{	
         requires = 'm';
         syntax = "<instance>";
}

COMMAND_RESULT CommandUsing::Handle(User* user, const Params& parameters)
{  
       const std::string& instance = parameters[0];
       
       User* found = Kernel->Clients->FindInstance(instance);
       
       if (!found)
       {
              user->SendProtocol(ERR_NO_INSTANCE, "Instance not found.");
              return FAILED;
       }
       
       user->SendProtocol(BRLD_USING, found->select, found->select.c_str());
       return SUCCESS;
}

CommandUse::CommandUse(Module* Creator) : Command(Creator, "USE", 1)
{          
         no_hint_until_reg = true;
         
         /* Users may provide use before registering. */
         
         pre_reg_ok = true;
         syntax = "<id between 1 and 100>";
}

COMMAND_RESULT CommandUse::Handle(User* user, const Params& parameters)
{  
       const std::string& use = parameters[0];

       if (!is_number(use))
       {
                 user->SendProtocol(ERR_USE, MUST_BE_NUMERIC);
                 return FAILED;
       }

       if (!is_positive_number(use))
       {
                user->SendProtocol(ERR_USE, MUST_BE_POSIT);
                return FAILED;
       }
       
       if (!Daemon::CheckRange(user, use, "Must be a value between 1 and 100", 1, 100))
       {
               return FAILED;
       }
       
       if (user->select == use)
       {
             user->SendProtocol(ERR_USE, use, PROCESS_ALREADY);
             return FAILED;
       }
      
       user->select = use;
       user->SendProtocol(BRLD_NEW_USE, use, PROCESS_OK);
       
       return SUCCESS;
}

CommandCurrent::CommandCurrent(Module* Creator) : Command(Creator, "CURRENT", 0)
{

}

COMMAND_RESULT CommandCurrent::Handle(User* user, const Params& parameters)
{  
       const std::string use = user->select;
       user->SendProtocol(BRLD_CURRENT_USE, use, use.c_str());
       return SUCCESS;
}

CommandDBReset::CommandDBReset(Module* Creator) : Command(Creator, "DBRESET", 0)
{

}

COMMAND_RESULT CommandDBReset::Handle(User* user, const Params& parameters)
{  
       if (!Kernel->Store->Flusher->Status())
       {
              user->SendProtocol(ERR_UNABLE_DBRESET, "Already paused.");
              return FAILED;
       }
       
       DataFlush::ResetAll();
       
       user->SendProtocol(BRLD_QUERY_OK, PROCESS_OK);
       
       return SUCCESS;
}

CommandDBSize::CommandDBSize(Module* Creator) : Command(Creator, "DBSIZE", 0)
{

}

COMMAND_RESULT CommandDBSize::Handle(User* user, const Params& parameters)
{  
       DBHelper::DBSize(user);
       return SUCCESS;
}