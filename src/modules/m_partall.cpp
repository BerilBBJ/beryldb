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

class CommandPartAll : public Command
{
  public:
  
        CommandPartAll(Module* Creator) : Command(Creator, "PARTALL", 0, 0)
        {

        }
        
        COMMAND_RESULT Handle(User* user, const Params& parameters)
        {
               if (!user->chans.size())
               {
                      user->SendProtocol(ERR_INPUT, NO_CHANS); 
                      return FAILED;
               }
               
               user->SendProtocol(BRLD_OK, PROCESS_OK);

	       for (User::SubsList::iterator i = user->chans.begin(); i != user->chans.end(); )
	       {
		        Channel* chan = (*i)->chan;
			++i;
                        chan->PartUser(user);
               }
                                        
               return SUCCESS;
        }
};

class ModulePartAll : public Module
{
  private:
  
        CommandPartAll cmd;

 public:
 
        ModulePartAll() : cmd(this)
        {
        
        }

        Version GetDescription()
        {
                return Version("Provides PARTALL command.", VF_BERYLDB|VF_OPTCOMMON);
        }
};

MODULE_LOAD(ModulePartAll)
        
